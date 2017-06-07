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
/* Author: fengruiming                                                  */
/* Date: 2016.3.28                                                      */
/* Function: connect and communication width thd agent server           */
/************************************************************************/
#ifndef MONITOR_MANAGER_H_
#define MONITOR_MANAGER_H_

#include <utils/thread.h>
#include <utils/singleton.h>
#include <common/general.h>
#include "processmessage.h"
#include "monitor.h"

namespace bubi {

	class MonitorManager : public utils::Runnable,
		public utils::Singleton<MonitorManager>, public bubi::TimerNotify {
		class WebSocketClient {
			friend class MonitorAgent;
			typedef websocketpp::lib::shared_ptr<asio::ssl::context> context_ptr;
			typedef websocketpp::config::asio_tls_client::message_type::ptr message_ptr;
			typedef ProcessMessage::client::connection_ptr connection_ptr;
		public:
			WebSocketClient(ProcessMessage& process_message_);
			~WebSocketClient();
			int32_t Initialize();
			bool IsConnected();
			bool Exit();
		private:
			void on_open(websocketpp::connection_hdl);
			void on_message(websocketpp::connection_hdl, message_ptr);
			void on_close(websocketpp::connection_hdl);

#ifdef AGENT_TLS
			context_ptr on_tls_init(websocketpp::connection_hdl);
#endif
		private:
			ProcessMessage::client	endpoint_;
			ProcessMessage& process_message_;
			bool bis_connected_;
		};
	public:
		MonitorManager();
		~MonitorManager();
		bool Initialize();
		void OnTimer(int64_t current_time);
		void OnSlowTimer(int64_t current_time);
		bool Exit();
	private:
		virtual void Run(utils::Thread *thread);
	private:
		WebSocketClient socket_client_;
		ProcessMessage process_message_;
		utils::Thread* thread_ptr_;
		bool bexit_;
	};
}

#endif
