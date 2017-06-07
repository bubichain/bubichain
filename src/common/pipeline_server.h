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

#ifndef PIPELINE_SERVER_H_
#define PIPELINE_SERVER_H_

#include <utils/thread.h>
#include "zmq_helper.h"

namespace bubi {
	class PipelineServer :
		public bubi::TimerNotify,
		public bubi::StatusModule,
		public utils::Runnable {
	private:
		class  PipelineTask : public utils::Runnable {
		public:
			PipelineTask(PipelineServer* pipeline_server, char type, const char* msg, int len);
			~PipelineTask();
			virtual void Run(utils::Thread *this_thread);
		public:
			unsigned char type_;
			std::string message_;
			PipelineServer* pipeline_server_;
		};

		class  PipelineWorker :
			public utils::Runnable {
			friend class PipelineTask;
		public:
			PipelineWorker(PipelineServer*	pipeline_server);
			~PipelineWorker();
			PipelineWorker(const PipelineTask& s) = delete;
			virtual void Run(utils::Thread *thread) override;
		public:
			PipelineServer* pipeline_server_;
		};
	public:
		enum PipelineType { ROUND_ROBIN, BROADCAST };
		PipelineServer(PipelineType pipe);
		~PipelineServer();
		bool Initialize(std::string name, std::string send_address, std::string pull_address, int32_t worker_count);
		bool Exit();
		bool Send(const ZMQTaskType type, const std::string& buf);
		bool Bind();
		void Recv(const ZMQTaskType type, const std::string& buf);
		virtual void GetModuleStatus(Json::Value &data);
	protected:
		void AddHandler(const ZMQTaskType type, DynamicHandler callback);
		void AddHandler(const ZMQTaskType type, StaticHandler callback);
		virtual void OnTimer(int64_t current_time) override;
		virtual void OnSlowTimer(int64_t current_time) override;
	private:
		virtual void Run(utils::Thread *thread) override;
	protected:
		void* context_;
		std::string name_;
		void* pull_socket_;
		void* pull_socket_mon_;
		std::string pull_address_;
		int16_t pull_num_;
		int recv_buffer_size_;
		char* recv_buffer_;
		PipelineType pipe_type_;
		void* send_socket_;
		void* send_socket_mon_;
		std::string send_address_;
		int16_t send_num_;
	private:
		StaticHandler static_handlers_[256];
		DynamicHandler dynamic_handlers_[256];
		utils::ThreadTaskQueue task_queue_;
		std::vector<PipelineWorker*> pipeline_workers_;
		std::vector<utils::Thread *> worker_thread_ptrs_;
		utils::Thread *thread_ptr_;
	};
}

#endif