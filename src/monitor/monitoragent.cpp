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

/************************************************************************/
/* Author: FengRuiMing                                                  */
/* Date: 2016.3.28                                                      */
/* Function: connect and communication width the agent server           */
/************************************************************************/

#include <utils/logger.h>
#include <utils/crypto.h>
#include <utils/file.h>
#include <common/configure.h>
#include "monitoragent.h"

namespace bubi {
	const char* MONITOR_MESSAGE = "monitor message";
	const char* MONITOR_AGENT_NAME = "monitor agent";

	MonitorAgent::MonitorMessages::MonitorMessages(MonitorAgent* monitor_agent) :
		thread_ptr_(NULL),
		monitor_agent_(monitor_agent){
		message_lists_.clear();
	}

	MonitorAgent::MonitorMessages::~MonitorMessages() {

	}

	bool MonitorAgent::MonitorMessages::Initialize() {
		bool bret = false;
		do {
			monitor_agent_->AddHandler(ZMQ_MONITOR_SYSTEM, std::bind(&MonitorAgent::MonitorMessages::on_system_method,
				this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			monitor_agent_->AddHandler(ZMQ_MONITOR_NOTICE, std::bind(&MonitorAgent::MonitorMessages::on_notice_method,
				this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			thread_ptr_ = new utils::Thread(this);
			if (false == thread_ptr_->Start(MONITOR_MESSAGE)) {
				LOG_ERROR("Initialize -- Start failed.");
				break;
			}
			bret = true;
		} while (false);

		return bret;
	}

	bool MonitorAgent::MonitorMessages::Exit() {
		try {
			if (thread_ptr_) {
				thread_ptr_->JoinWithStop();
				delete thread_ptr_;
				thread_ptr_ = NULL;
			}
		}
		catch (std::exception& e) {
			LOG_ERROR("Exit -- %s", e.what());
		}
		return true;
	}

	void MonitorAgent::MonitorMessages::Run(utils::Thread *thread) {
		monitor_agent_->Connect();
		do {
			if (message_lists_.empty()) {
				utils::Sleep(1);
				continue;
			}
			MonitorMessage msg;
			{
				utils::MutexGuard guard(message_lists_mutex_);
				msg = message_lists_.front();
			}
			std::string reply;
			if (monitor_agent_->m_dynamicHandlers_[msg.type_]) {
				monitor_agent_->m_dynamicHandlers_[msg.type_](msg.buffer_.c_str(), msg.buffer_.length(), reply);
			}
			else if (monitor_agent_->m_staticHandlers_[msg.type_]) {
				monitor_agent_->m_staticHandlers_[msg.type_](msg.buffer_.c_str(), msg.buffer_.length(), reply);
			}

			int64_t p = (int64_t)this;
			reply.append((char*)&p, sizeof(int64_t));
			if (!monitor_agent_->SendToMonitor(msg.type_, reply)) {
				LOG_ERROR("Run -- SendToMonitor failed");
				utils::Sleep(2000);
				continue;
			}

			{
				utils::MutexGuard guard(message_lists_mutex_);
				message_lists_.pop_front();
			}
			utils::Sleep(0);
		} while (thread->enabled());
	}

	void MonitorAgent::MonitorMessages::on_system_method(const char* msg, int len, std::string &reply) {
		Json::Value value;
		if (monitor_agent_->system_manager_.GetSystemMonitor(monitor_agent_->log_path_, value)) {
			reply = value["result"].toFastString();
		}
		else {
			LOG_ERROR("on_system_method -- get system info failed");
		}
		reply.push_back(6);
	}

	void MonitorAgent::MonitorMessages::on_notice_method(const char* msg, int len, std::string &reply) {
		reply = utils::String::Format("%s", msg);
	}

	MonitorAgent::MonitorAgent(MONITORTYPE monitor_type) :
		thread_ptr_(NULL),
		context_(NULL),
		pull_socket_(NULL),
		push_socket_(NULL),
		monitor_type_(monitor_type),
		push_address_(""),
		pull_address_(""),
		monitor_messages_(this)
		{
		memset(m_staticHandlers_, 0, sizeof(m_staticHandlers_));
		memset(m_dynamicHandlers_, 0, sizeof(m_dynamicHandlers_));
		timer_name_ = "Moniter Agent";
	}

	MonitorAgent::~MonitorAgent() {}

	bool MonitorAgent::Exit() {
		try {
			zmq_ctx_shutdown(context_);
			if (thread_ptr_) {
				thread_ptr_->JoinWithStop();
				delete thread_ptr_;
				thread_ptr_ = NULL;
			}
			monitor_messages_.Exit();

			zmq_close(pull_socket_);
			zmq_close(push_socket_);
			zmq_ctx_term(context_);
		}
		catch (std::exception& e) {
			LOG_ERROR("Exit -- %s", e.what());
		}

		return true;
	}

	bool MonitorAgent::Initialize(const std::string& send_address, const std::string& recv_address, const std::string& log_path) {

		bool bret = false;

		do {
			if (monitor_type_ != ROUND_ROBZOUT && monitor_type_ != SUBSCRIBE) {
				break;
			}

			push_address_ = send_address;
			pull_address_ = recv_address;
			log_path_ = log_path;

			context_ = zmq_ctx_new();

			if (!monitor_messages_.Initialize()) {
				LOG_ERROR("Initialize -- monitor messages initialize failed");
				break;
			}

			thread_ptr_ = new utils::Thread(this);
			if (false == thread_ptr_->Start(MONITOR_AGENT_NAME)) {
				LOG_ERROR("Initialize -- Start failed.");
				break;
			}

			// register OnTimer
			bubi::TimerNotify::RegisterModule(this);
			bret = true;
		} while (false);

		return bret;
	}

	void MonitorAgent::Run(utils::Thread *thread) {
		if (!pull_socket_) {
			if (monitor_type_ == ROUND_ROBZOUT) {
				pull_socket_ = zmq_socket(context_, ZMQ_PULL);
			}
			else if (monitor_type_ == SUBSCRIBE) {
				pull_socket_ = zmq_socket(context_, ZMQ_SUB);
				zmq_setsockopt(pull_socket_, ZMQ_SUBSCRIBE, "", 0);
			}
		}

		zmq_connect(pull_socket_, pull_address_.c_str());
		char* recv_buffer_ = new char[ZMQ_RECV_BUFFER_SIZE];

		while (thread->enabled()) {
			memset(recv_buffer_, 0, ZMQ_RECV_BUFFER_SIZE);

			char* buffer = recv_buffer_;
			unsigned char	type = 0;

			int len = zmq_recv(pull_socket_, recv_buffer_, ZMQ_RECV_BUFFER_SIZE, 0);
			if (len < 2) //at 2 bytes
			{
				continue;
			}
			type = recv_buffer_[len - 1];

			if (len == 5 && type == ZMQ_LARGE_MSG) {
				//lager message than 10M, rarely happen
				LOG_WARN("slave worker is receiving a large transaction of size %d", len);

				//4 bytes big-endian integer + 1 byte type
				int expectLen = (recv_buffer_[0] << 24) + (recv_buffer_[1] << 16) + (recv_buffer_[2] << 8) + recv_buffer_[3];
				char* newBuffer = new char[expectLen];
				len = zmq_recv(pull_socket_, newBuffer, expectLen, 0);
				if (len != expectLen) {
					LOG_ERROR("drop messsage, actual length %d != expect length %d ", len, expectLen);
					delete[] newBuffer;
					continue;
				}
				buffer = newBuffer;
				type = newBuffer[len - 1];
			}
			if (ZMQ_MONITOR_NULL == (ZMQTaskType)type)
				continue;
			buffer[len - 1 - sizeof(int64_t)] = '\0';
			MonitorMessage monitor_message;
			monitor_message.buffer_ = buffer;
			monitor_message.type_ = (ZMQTaskType)type;
			{
				utils::MutexGuard guard(monitor_messages_.message_lists_mutex_);
				monitor_messages_.message_lists_.push_back(monitor_message);
			}

			if (buffer != recv_buffer_ && buffer != NULL) {
				delete[] buffer;
				buffer = NULL;
			}
		}

		if (recv_buffer_ != NULL) {
			delete[] recv_buffer_;
			recv_buffer_ = NULL;
		}
	}

	void MonitorAgent::OnSlowTimer(int64_t current_time) {
		system_manager_.OnSlowTimer();
	}

	void MonitorAgent::OnTimer(int64_t current_time) {}

	bool MonitorAgent::SendToMonitor(const ZMQTaskType type, std::string& buf) {
		buf.push_back(type);
		return zmq_send_ex_nonblock(push_socket_, buf.c_str(), buf.length()) > 0;
	}

	bool MonitorAgent::Connect() {
		bool bret = false;
		do {
			if (!push_socket_) {
				push_socket_ = zmq_socket(context_, ZMQ_PUSH);
			}

			zmq_connect(push_socket_, push_address_.c_str());
			bret = true;
		} while (false);

		return bret;
	}

	bool MonitorAgent::NoticeMonitor(const std::string& buffer, const ZMQTaskType& type) {
		bool bret = false;
		try {
			MonitorMessage monitor_message;
			monitor_message.type_ = type;
			monitor_message.buffer_ = buffer;
			int64_t p = (int64_t)this;
			monitor_message.buffer_.append((char*)&p, sizeof(int64_t));
			utils::MutexGuard guard(monitor_messages_.message_lists_mutex_);
			monitor_messages_.message_lists_.push_front(monitor_message);
			bret = true;
		}
		catch (std::exception& e) {
			LOG_ERROR("AddMessageList -- %s", e.what());
		}

		return bret;
	}

	void MonitorAgent::AddHandler(const ZMQTaskType type, DynamicHandler callback) {
		if (m_staticHandlers_[type] || m_dynamicHandlers_[type] != nullptr) {
			LOG_WARN("duplicated handlers for type %d", type);
		}

		m_dynamicHandlers_[type] = callback;
	}

	void MonitorAgent::AddHandler(const ZMQTaskType type, StaticHandler callback) {
		if (m_staticHandlers_[type] || m_dynamicHandlers_[type] != nullptr) {
			LOG_WARN("duplicated handlers for type %d", type);
		}

		m_staticHandlers_[type] = callback;
	}

}