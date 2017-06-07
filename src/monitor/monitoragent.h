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
/* Function: connect and communication width thd agent server           */
/************************************************************************/
#ifndef MONITOR_AGENT_H_
#define MONITOR_AGENT_H_

#include "system_manager.h"
#include <utils/thread.h>
#include <common/general.h>
#include <common/zmq_helper.h>

namespace bubi {
	class MonitorAgent : public utils::Runnable, public bubi::TimerNotify {
	protected:
		typedef struct MonitorMessage_ {
			MonitorMessage_() : type_(ZMQ_MONITOR_NULL), buffer_("") {}
			ZMQTaskType type_;
			std::string buffer_;
		} MonitorMessage;
		enum MONITORTYPE { ROUND_ROBZOUT, SUBSCRIBE };
		void AddHandler(const ZMQTaskType type, DynamicHandler callback);
		void AddHandler(const ZMQTaskType type, StaticHandler callback);
	private:
		class MonitorMessages : public utils::Runnable {
			friend class MonitorAgent;
		public:
			MonitorMessages(MonitorAgent* monitor_agent);
			~MonitorMessages();
		public:
			bool Initialize();
			bool Exit();
		private:
			virtual void Run(utils::Thread *thread);
			void on_system_method(const char* msg, int len, std::string &reply);
			void on_notice_method(const char* msg, int len, std::string &reply);
		private:
			utils::Thread* thread_ptr_;
			MonitorAgent* monitor_agent_;
			std::list<MonitorMessage> message_lists_;
			utils::Mutex message_lists_mutex_;
		};
	public:
		MonitorAgent(MONITORTYPE monitor_type);
		~MonitorAgent();
	public:
		bool Initialize(const std::string& send_address, const std::string& recv_address, const std::string& log_path);
		bool NoticeMonitor(const std::string& buffer, const ZMQTaskType& type = ZMQ_MONITOR_NOTICE);
		void OnTimer(int64_t current_time);
		void OnSlowTimer(int64_t current_time);
		bool Exit();
	private:
		virtual void Run(utils::Thread *thread);
		bool SendToMonitor(const ZMQTaskType type, std::string& buf);
		bool Connect();
	protected:
		SystemManager system_manager_;
		std::string log_path_;
	private:
		StaticHandler m_staticHandlers_[256];
		DynamicHandler m_dynamicHandlers_[256];
		utils::Thread* thread_ptr_;
		void* context_;
		void* pull_socket_;
		void* push_socket_;
		MONITORTYPE monitor_type_;
		std::string push_address_;
		std::string pull_address_;
		MonitorMessages monitor_messages_;
	};
}

#endif
