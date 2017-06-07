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

#include "pipeline_server.h"
#include <utils/strings.h>
#include <utils/logger.h>
#include <utils/net.h>

namespace bubi {
	PipelineServer::PipelineWorker::PipelineWorker(PipelineServer*	pipeline_server)
		:pipeline_server_(pipeline_server) {

	}

	PipelineServer::PipelineWorker::~PipelineWorker() {}

	void PipelineServer::PipelineWorker::Run(utils::Thread *thread) {
		while (thread->enabled()) {
			utils::Runnable* task = pipeline_server_->task_queue_.Get();
			if (task) {
				task->Run(thread);
				delete task;
			}
			else
				utils::Sleep(1);
		}
	}

	PipelineServer::PipelineTask::PipelineTask(PipelineServer* pipeline_server, char type, const char* msg, int len)
		:pipeline_server_(pipeline_server), type_(type), message_(msg, len) {}
	PipelineServer::PipelineTask::~PipelineTask() {}

	void PipelineServer::PipelineTask::Run(utils::Thread *this_thread) {
		std::string reply;
		if (pipeline_server_->dynamic_handlers_[type_]) {
			pipeline_server_->dynamic_handlers_[type_](message_.c_str(), message_.length(), reply);
		}
		else if (pipeline_server_->static_handlers_[type_]) {
			pipeline_server_->static_handlers_[type_](message_.c_str(), message_.length(), reply);
		}
	}

	PipelineServer::PipelineServer(PipelineType pipe_type) :
		context_(NULL),
		pull_socket_(NULL),
		pull_socket_mon_(NULL),
		pull_address_(),
		pull_num_(0),
		pipe_type_(pipe_type),
		send_socket_(NULL),
		send_socket_mon_(NULL),
		send_num_(0),
		recv_buffer_size_(ZMQ_RECV_BUFFER_SIZE),
		recv_buffer_(new char[recv_buffer_size_]),
		task_queue_(),
		worker_thread_ptrs_() {
		memset(static_handlers_, 0, sizeof(static_handlers_));
		memset(dynamic_handlers_, 0, sizeof(dynamic_handlers_));
		timer_name_ = "PiperLine Server";
	}

	PipelineServer::~PipelineServer() {
		delete[] recv_buffer_;
	}

	bool PipelineServer::Initialize(std::string name, std::string send_address, std::string pull_address, int32_t works) {
		send_address_ = send_address;
		pull_address_ = pull_address;
		name_ = timer_name_ = name;
		context_ = zmq_ctx_new();

		for (uint32_t i = 0; i < works; i++) {
			PipelineWorker *pipeline_worker = new PipelineWorker(this);

			pipeline_workers_.push_back(pipeline_worker);

			utils::Thread* worker_thread = new utils::Thread(pipeline_worker);
			std::string work_name = name_ + "-work-" + utils::String::ToString(i);
			if (!worker_thread->Start(utils::String::Format(work_name.c_str()))) {
				//process will exit, is it necessary to clean up? 
				LOG_ERROR("Pipeline work thread start failed");
				return false;
			}
			worker_thread_ptrs_.push_back(worker_thread);
		}

		thread_ptr_ = new utils::Thread(this);
		std::string recv_name = name_ + "-recv";
		if (!thread_ptr_->Start(recv_name.c_str())) {
			LOG_ERROR("Pipeline work thread start failed");
			return false;
		}

		TimerNotify::RegisterModule(this);
		StatusModule::RegisterModule(this);

		return true;
	}

	bool PipelineServer::Exit() {
		zmq_ctx_shutdown(context_);


		while (worker_thread_ptrs_.size() > 0) {
			delete pipeline_workers_.back();
			pipeline_workers_.pop_back();
			//join after stop
			worker_thread_ptrs_.back()->JoinWithStop();
			delete worker_thread_ptrs_.back();
			worker_thread_ptrs_.pop_back();

		}

		if (thread_ptr_) {
			thread_ptr_->JoinWithStop();
			delete thread_ptr_;
			thread_ptr_ = NULL;
		}
		zmq_close(send_socket_);
		zmq_close(send_socket_mon_);
		zmq_close(pull_socket_mon_);
		zmq_ctx_term(context_);

		return true;
	}

	void PipelineServer::AddHandler(const ZMQTaskType type, DynamicHandler callback) {
		if (static_handlers_[type] || dynamic_handlers_[type] != nullptr) {
			LOG_WARN("Duplicated handlers for type %d", type);
		}

		dynamic_handlers_[type] = callback;
	}

	void PipelineServer::AddHandler(const ZMQTaskType type, StaticHandler callback) {
		if (static_handlers_[type] || dynamic_handlers_[type] != nullptr) {
			LOG_WARN("Duplicated handlers for type %d", type);
		}

		static_handlers_[type] = callback;
	}
	bool PipelineServer::Send(const ZMQTaskType type, const std::string& buf) {
		std::string buf_with_type = buf + char(type);
		return zmq_send_ex_nonblock(send_socket_, buf_with_type.c_str(), buf_with_type.length()) > 0;
	}

	void PipelineServer::Recv(const ZMQTaskType type, const std::string& buf) {
		task_queue_.Put(new PipelineTask(this, type, buf.c_str(), buf.length()));
	}

	bool PipelineServer::Bind() {
		if (!send_socket_) {
			if (pipe_type_ == ROUND_ROBIN) {
				send_socket_ = zmq_socket(context_, ZMQ_PUSH);
				int sndhwm = 0;
				zmq_setsockopt(send_socket_, ZMQ_SNDHWM, &sndhwm, sizeof(int));
				int immediate = 1;
				zmq_setsockopt(send_socket_, ZMQ_IMMEDIATE, &immediate, sizeof(int));
			}
			else if (pipe_type_ == BROADCAST) {
				send_socket_ = zmq_socket(context_, ZMQ_PUB);
				int sndhwm = 0;
				zmq_setsockopt(send_socket_, ZMQ_SNDHWM, &sndhwm, sizeof(int));
				int immediate = 1;
				zmq_setsockopt(send_socket_, ZMQ_IMMEDIATE, &immediate, sizeof(int));
			}
			else {
				return false;
			}

			zmq_socket_monitor(send_socket_, utils::String::Format("inproc://send_mon-%s", name_.c_str()).c_str(), ZMQ_EVENT_ALL);
			send_socket_mon_ = zmq_socket(context_, ZMQ_PAIR);
			zmq_connect(send_socket_mon_, utils::String::Format("inproc://send_mon-%s", name_.c_str()).c_str());

			zmq_bind(send_socket_, send_address_.c_str());
			return true;
		}
		return false;
	}
	void PipelineServer::Run(utils::Thread *thread) {

		pull_socket_ = zmq_socket(context_, ZMQ_PULL);

		zmq_socket_monitor(pull_socket_, utils::String::Format("inproc://recv_mon-%s", name_.c_str()).c_str(), ZMQ_EVENT_ALL);
		pull_socket_mon_ = zmq_socket(context_, ZMQ_PAIR);
		zmq_connect(pull_socket_mon_, utils::String::Format("inproc://recv_mon-%s", name_.c_str()).c_str());

		int recvBufferSize = 1 * utils::BYTES_PER_MEGA;
		zmq_setsockopt(pull_socket_, ZMQ_RCVBUF, &recvBufferSize, sizeof(int));
		zmq_bind(pull_socket_, pull_address_.c_str());

		while (thread->enabled()) {
			char*			buffer = recv_buffer_;
			unsigned char	type = 0;

			int len = zmq_recv(pull_socket_, recv_buffer_, recv_buffer_size_, 0);
			if (len < 2) //at 2 bytes
			{
				continue;
			}
			type = recv_buffer_[len - 1];

			if (len == 5 && type == ZMQ_LARGE_MSG) {
				//lager message than 10M, rarely happen
				LOG_WARN("Slave worker is receiving a large transaction of size %d", len);

				//4 bytes big-endian integer + 1 byte type
				int expectLen = (recv_buffer_[0] << 24) + (recv_buffer_[1] << 16) + (recv_buffer_[2] << 8) + recv_buffer_[3];
				char* newBuffer = new char[expectLen];
				len = zmq_recv(pull_socket_, newBuffer, expectLen, 0);
				if (len != expectLen) {
					LOG_ERROR("Drop messsage, actual length %d != expect length %d ", len, expectLen);
					delete[] newBuffer;
					continue;
				}
				buffer = newBuffer;
				type = newBuffer[len - 1];
			}

			task_queue_.Put(new PipelineTask(this, type, buffer, len - 1));

			if (buffer != recv_buffer_) {
				delete[] buffer;
			}
		}

		zmq_close(pull_socket_);
	}
	static uint16_t get_monitor_event(void *monitor, uint32_t& value, std::string& address) {
		//  First frame in message contains event number and value
		zmq_msg_t msg;
		zmq_msg_init(&msg);
		if (zmq_msg_recv(&msg, monitor, ZMQ_DONTWAIT) == -1)
			return 0;              //  Interrupted, presumably
		assert(zmq_msg_more(&msg));

		char *data = (char *)zmq_msg_data(&msg);
		uint16_t event = *(uint16_t *)(data);
		value = *(uint32_t *)(data + 2);

		//  Second frame in message contains event address
		zmq_msg_init(&msg);
		if (zmq_msg_recv(&msg, monitor, ZMQ_DONTWAIT) == -1)
			return -1;              //  Interrupted, presumably
		assert(!zmq_msg_more(&msg));


		data = (char *)zmq_msg_data(&msg);
		size_t size = zmq_msg_size(&msg);
		address.assign(data, size);

		return event;
	}

	void PipelineServer::OnTimer(int64_t current_time) {
		std::string address = "";
		uint32_t value = 0;
		uint16_t event = get_monitor_event(send_socket_mon_, value, address);
		if (event != 0) {
			if ((event & ZMQ_EVENT_LISTENING) == ZMQ_EVENT_LISTENING) {
				send_num_ = 0;
				LOG_INFO("(%s) 'send' listening address(%s)", name_.c_str(), send_address_.c_str());
			}
			else if ((event & ZMQ_EVENT_ACCEPTED) == ZMQ_EVENT_ACCEPTED) {
				send_num_++;
				LOG_INFO("(%s) 'send' accepted address(%s) count(%d)", name_.c_str(), utils::GetPeerName(value).c_str(), send_num_);
			}
			else if ((event & ZMQ_EVENT_DISCONNECTED) == ZMQ_EVENT_DISCONNECTED) {
				send_num_--;
				if (send_num_ < 0)
					LOG_ERROR("(%s) 'send' count < 0", name_.c_str());
				LOG_INFO("(%s) 'send' disconnected address(%s) count(%d)", name_.c_str(), utils::GetPeerName(value).c_str(), send_num_);
			}
			else if ((event & ZMQ_EVENT_MONITOR_STOPPED) == ZMQ_EVENT_MONITOR_STOPPED) {
				send_num_ = 0;
				LOG_INFO("(%s) 'send' monitor stopped", name_.c_str());
			}
			else {
				LOG_INFO("(%s) 'send' monitor event %x", name_.c_str(), event);
			}
		}

		event = get_monitor_event(pull_socket_mon_, value, address);

		if (event != 0) {
			if ((event & ZMQ_EVENT_LISTENING) == ZMQ_EVENT_LISTENING) {
				pull_num_ = 0;
				LOG_INFO("(%s) 'recv' listening address(%s)", name_.c_str(), pull_address_.c_str());
			}
			else if ((event & ZMQ_EVENT_ACCEPTED) == ZMQ_EVENT_ACCEPTED) {
				pull_num_++;
				LOG_INFO("(%s) 'recv' accepted address(%s) count(%d)", name_.c_str(), utils::GetPeerName(value).c_str(), pull_num_);
			}
			else if ((event & ZMQ_EVENT_DISCONNECTED) == ZMQ_EVENT_DISCONNECTED) {
				pull_num_--;
				if (pull_num_ < 0)
					LOG_ERROR("(%s) 'recv' count < 0", name_.c_str());
				LOG_INFO("(%s) 'recv' disconnected address(%s) count(%d)", name_.c_str(), utils::GetPeerName(value).c_str(), pull_num_);
			}
			else if ((event & ZMQ_EVENT_MONITOR_STOPPED) == ZMQ_EVENT_MONITOR_STOPPED) {
				pull_num_ = 0;
				LOG_INFO("(%s) 'recv' monitor stopped", name_.c_str());
			}
			else {
				LOG_INFO("(%s) 'recv' monitor event %x", name_.c_str(), event);
			}
		}
	}


	void PipelineServer::OnSlowTimer(int64_t current_time) {

	}

	void PipelineServer::GetModuleStatus(Json::Value &data) {
		data["name"] = name_;
		Json::Value &item = data[name_];
		item["pending_size"] = task_queue_.Size();
	}
}