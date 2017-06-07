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
/* Date: 2016.10.20                                                     */
/* Function: process the message of web socket                          */
/************************************************************************/
#ifndef PROCESS_MESSAGE_H_
#define PROCESS_MESSAGE_H_

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <list>
#include <zmq.h>
#include <common/general.h>
#include <common/zmq_helper.h>
#include <monitor/upgrade.h>
#include <monitor/alert.h>
#include <monitor/notice.h>

namespace bubi {
	class ProcessMessage : public utils::Runnable {
		friend class MonitorManager;
		friend void Upgrade::Run(utils::Thread *thread);

		typedef struct MonitorMessage_ {
			MonitorMessage_() : type_(ZMQ_MONITOR_NULL), buffer_("") {}
			ZMQTaskType type_;
			std::string buffer_;
		} MonitorMessage;

		class MonitorMessages : public utils::Runnable {
			friend class ProcessMessage;
			typedef std::map<ZMQTaskType, int> MonitorErrorCode;
		public:
			MonitorMessages(ProcessMessage* pprocess_message);
			~MonitorMessages();
			bool Initialize();
			bool Exit();
		private:
			virtual void Run(utils::Thread *thread);
			void MonitorError(ZMQTaskType type);
			void ResetState();
			void PushBackMonitorMessage(const MonitorMessage& message);
		private:
			ProcessMessage* pprocess_message_;
			utils::Thread* thread_ptr_;
			std::list<MonitorMessage> message_lists_;
			utils::Mutex message_lists_mutex_;
			MonitorErrorCode monitor_error_;
			bool bexit_;
		};

#ifdef AGENT_TLS
		typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
#else
		typedef websocketpp::client<websocketpp::config::asio_client> client;
#endif

		typedef void (ProcessMessage::*MonitorMethodProc)(const char* msg, const std::string& method);
		typedef std::map<std::string, MonitorMethodProc> AgentMethodMap;
		typedef std::map<std::string, ZMQTaskType> MonitorType;
		typedef std::map<int, std::string> ErrorType;

	public:
		ProcessMessage();
		~ProcessMessage();
		bool Initialize();
		bool Exit();
	private:
		void Run(utils::Thread *thread);
		void OnTimer();
		void OnSlowTimer();
		void ResetState();
		bool ProcessRequestMessages(const std::string& msg);
		bool ProcessResponseMessages(const std::string& msg);
		void PushBackRequestMessages(const std::string& msg);
		void PushBackResponseMessages(const std::string& msg);
		void SendRequestMessage(const std::string& method, const bool& request, const Json::Value& parameter);
		void SendResponseMessage(const std::string& method, const bool& request, const int& error_code, const Json::Value& result);
		// request server and ask for response
		void on_common_method(const char* msg, const std::string& method);
		void on_upgrade_method(const char* msg, const std::string& method);
		void on_get_configure_method(const char* msg, const std::string& method);
		void on_set_configure_method(const char* msg, const std::string& method);
		void on_logout_method(const char* msg, const std::string& method);
		// result from zero message queue
		void on_common_monitor(const char* msg, int len, std::string &reply);
		void on_alert_monitor(const char* msg, int len, std::string &reply);
		void on_notice_monitor(const char* msg, int len, std::string &reply);
		void on_account_exception_monitor(const char* msg, int len, std::string &reply);
		// request server and not ask for response
		void on_response_hello(const char* msg, const std::string& method);
		void on_response_register(const char* msg, const std::string& method);
		void on_response_ledger(const char* msg, const std::string& method);
		void on_response_common(const char* msg, const std::string& method);
		void on_response_heartbeat(const char* msg, const std::string& method);
		void on_response_upgrade(const char* msg, const std::string& method);
		void on_response_error(const char* msg, const std::string& method);
	private:
		MonitorMessages monitor_message_;
		Upgrade upgrade_;
		Alert alert_;
		Notice notice_;
		utils::Thread* thread_ptr_;
		client* pendpoint_;						// client socket handle
		websocketpp::connection_hdl connection_hdl_;				// handle to connect server
		std::string monitor_id_;					// agent id
		std::string random_key_;					// session id
		std::string log_path_;						// log file path
		std::list<std::string> request_message_lists_;			// request message lists
		utils::Mutex request_message_lists_mutex_;
		std::list<std::string> response_message_lists_;		// response message lists
		utils::Mutex response_message_lists_mutex_;
		int64_t heartbeat_interval_;			// heartbeat interval
		int64_t connection_timeout_;			// timeout
		int64_t	heartbeat_last_time_;			// lastest time of sending hearbeat
		bool bheartbeated_;					// to prevent the heartbeat for sending frequently
		bool bclosebeated_;					// to prevent the close for sending frequentyly
		AgentMethodMap request_methods_;				// request methods
		AgentMethodMap response_methods_;				// response methods
		MonitorType monitor_type_;
		ErrorType error_type_;
		int64_t check_exception_interval_;		// interval time of checking exception
		int64_t last_check_exception_time_;		// last time of checking exception
		bool blogin_;
		bool bupgrading_;
	};
}

#endif
