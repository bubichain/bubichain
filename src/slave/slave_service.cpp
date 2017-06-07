/*
Copyright © Bubi Technologies Co., Ltd. 2017 All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
		 http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <utils/headers.h>
#include <common/general.h>
#include <common/configure.h>
#include <common/private_key.h>
#include "slave_service.h"
#include "slave_executor.h"
#include "overlay/peer.h"

//define this through CXXFLAGS when make
//#define BUBI_TEST

namespace bubi {

	SlaveService::SlaveService() :
		m_pushersMapRWLock_(),
        worker_thread_ptrs_(),
		m_pushersMap_()
	{
		memset(m_staticHandlers_, 0, sizeof(m_staticHandlers_));
		memset(m_dynamicHandlers_, 0, sizeof(m_dynamicHandlers_));
	}

	SlaveService::~SlaveService() 
    {        
        if (m_worker_)
        {
            delete m_worker_;
        }

        zmq_ctx_term(m_context_);
	}

    bool SlaveService::Initialize(PipelineConfigure &config)
	{
        AddHandler(ZMQ_BROADCAST_TX, SlaveExecutor::ProcessBroadcastTransaction);
        AddHandler(ZMQ_TXSET_TX, SlaveExecutor::ProcessTxSetTransaction);

        m_pullAddress_ = config.recv_address_;
        m_pushAddress_ = config.send_address_;

        m_context_ = zmq_ctx_new();

        uint32_t workers_count = config.workers_count_;

        for (uint32_t i = 0; i < workers_count; i++)
        {
            m_worker_ = new SlaveWorker();
            utils::Thread* worker_thread = new utils::Thread(m_worker_);
            if (!worker_thread->Start(utils::String::Format("slave-worker-%d", i)))
            {
                //process will exit, is it necessary to clean up? 
                return false;
            }

            worker_thread_ptrs_.push_back(worker_thread);
        }
        
		return true;
	}

	bool SlaveService::Exit() 
	{
        //all the blocking zmq_xxx will return immediately
        zmq_ctx_shutdown(m_context_);

        for (uint32_t i = 0; i < worker_thread_ptrs_.size(); i++)
        {
            //join after stop
            worker_thread_ptrs_[i]->JoinWithStop();
            delete worker_thread_ptrs_[i];
        }

        worker_thread_ptrs_.clear();
        
		return true;
	}

	void SlaveService::AddHandler(const ZMQTaskType type, StaticHandler callback)
	{
        if (m_staticHandlers_[type] || m_dynamicHandlers_[type] != nullptr)
		{
			LOG_WARN("duplicated handlers for type %d", type);
		}

		m_staticHandlers_[type] = callback;
	}


	bool SlaveService::SendToMaster(const ZMQTaskType type, std::string& buf)
    {
		void* pushSocket  = NULL;

		size_t	currentTid = utils::Thread::current_thread_id();

		m_pushersMapRWLock_.ReadLock();
		std::map<size_t, void*>::iterator it = m_pushersMap_.find(currentTid);
		if (it != m_pushersMap_.end())
		{
			pushSocket  = it->second;
			m_pushersMapRWLock_.ReadUnlock();
		}
		else
		{
			m_pushersMapRWLock_.ReadUnlock();
			m_pushersMapRWLock_.WriteLock();
			pushSocket  = zmq_socket(m_context_, ZMQ_PUSH);
			m_pushersMap_[currentTid] = pushSocket;
			m_pushersMapRWLock_.WriteUnlock();

			zmq_connect(pushSocket, m_pushAddress_.c_str());
            LOG_INFO("ZMQ of webserver connects to bubi %s", m_pushAddress_.c_str());
		}

		buf.push_back(type);
        return zmq_send_ex(pushSocket, buf.c_str(), buf.length()) > 0;
	}

	//SlaveWorker
	SlaveWorker::SlaveWorker() :
		m_pushSocket_(NULL),
		m_pullSocket_(NULL),
		m_recvBufferSize_(ZMQ_RECV_BUFFER_SIZE),
		m_recvBuffer_(new char[m_recvBufferSize_])//default 10M
	{

	}

	SlaveWorker::~SlaveWorker()
	{
		if (m_recvBuffer_)
		{
			delete[] m_recvBuffer_;
		}
	}

    void SlaveWorker::Run(utils::Thread *thread)
	{
		m_pushSocket_ = zmq_socket(SlaveService::GetInstance()->m_context_, ZMQ_PUSH);
        zmq_connect(m_pushSocket_, SlaveService::GetInstance()->m_pushAddress_.c_str());

        m_pullSocket_ = zmq_socket(SlaveService::GetInstance()->m_context_, ZMQ_PULL);
        zmq_connect(m_pullSocket_, SlaveService::GetInstance()->m_pullAddress_.c_str());

		//poll and update pull_connected or execute task
		while (thread->enabled())
		{
			char*			buffer	= m_recvBuffer_;
			unsigned char	type	= 0;

			int len = zmq_recv(m_pullSocket_, m_recvBuffer_, m_recvBufferSize_, 0);
			if (len < 2) //at 2 bytes
			{
				continue;
			}
			type = m_recvBuffer_[len - 1];

			if (len == 5 && type == ZMQ_LARGE_MSG)
			{
				//lager message than 10M, rarely happen
				LOG_WARN("slave worker is receiving a large transaction of size %d", len);

				//4 bytes big-endian integer + 1 byte type
				int expectLen = (m_recvBuffer_[0] << 24) + (m_recvBuffer_[1] << 16) + (m_recvBuffer_[2] << 8) + m_recvBuffer_[3];
				char* newBuffer = new char[expectLen];
				len = zmq_recv(m_pullSocket_, newBuffer, expectLen, 0);
				if (len != expectLen)
				{
					LOG_ERROR("drop messsage, actual length %d != expect length %d ", len, expectLen);
					delete[] newBuffer;
					continue;
				}
				buffer = newBuffer;
				type = m_recvBuffer_[len - 1];
			}

			std::string reply;
			if (SlaveService::GetInstance()->m_dynamicHandlers_[type])
			{
                SlaveService::GetInstance()->m_dynamicHandlers_[type](buffer, len - 1, reply);
			}
            else if (SlaveService::GetInstance()->m_staticHandlers_[type])
			{
                SlaveService::GetInstance()->m_staticHandlers_[type](buffer, len - 1, reply);
			}

			if (reply.length() > 0)
			{
				reply.push_back(type);

				//reply is updated to buffer + 1, send result to master with type
				zmq_send_ex(m_pushSocket_, reply.c_str(), reply.length());
			}

			if (buffer != m_recvBuffer_)
			{
				delete[] buffer;
			}

			//give other threads a chance
			utils::Sleep(0);
		}

		zmq_close(m_pushSocket_);
		zmq_close(m_pullSocket_);
	}
}

