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
/* Function: connect and communication width the agent server           */
/************************************************************************/

#include "monitor_manager.h"
#include <common/configure.h>
#include <utils/logger.h>
#include <utils/crypto.h>
#include <utils/file.h>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

namespace bubi {
	MonitorManager::WebSocketClient::WebSocketClient(ProcessMessage& process_message_) :
		process_message_(process_message_),
		bis_connected_(false) {
		// Set Log Access
		//endpoint_.set_access_channels(websocketpp::log::alevel::all);
		//endpoint_.set_error_channels(websocketpp::log::elevel::all);
		endpoint_.clear_access_channels(websocketpp::log::alevel::all);
		// Initialize ASIO
		endpoint_.init_asio();
		// Register our handlers
#ifdef AGENT_TLS
		endpoint_.set_tls_init_handler(bind(&MonitorManager::WebSocketClient::on_tls_init, this, ::_1));
#endif
		endpoint_.set_message_handler(bind(&MonitorManager::WebSocketClient::on_message, this, ::_1, ::_2));
		endpoint_.set_open_handler(bind(&MonitorManager::WebSocketClient::on_open, this, ::_1));
		endpoint_.set_close_handler(bind(&MonitorManager::WebSocketClient::on_close, this, ::_1));
		endpoint_.set_fail_handler(bind(&MonitorManager::WebSocketClient::on_close, this, ::_1));
		endpoint_.set_interrupt_handler(bind(&MonitorManager::WebSocketClient::on_close, this, ::_1));
	}

	MonitorManager::WebSocketClient::~WebSocketClient() {}

	bool MonitorManager::WebSocketClient::Exit() {
		bool bret = false;
		try {
			if (bis_connected_) {
				LOG_INFO("Exit -- disconnecting");
				endpoint_.close(process_message_.connection_hdl_, websocketpp::close::status::going_away, process_message_.random_key_);
			}
			bret = true;
		}
		catch (std::exception& e) {
			LOG_ERROR("Exit -- %s", e.what());
		}
		return bret;
	}

	int32_t MonitorManager::WebSocketClient::Initialize() {
		int32_t iret = 0;

		do {
			WebSocketClientConfigure& web_socket_client_configure = MonitorConfigure::Instance().web_socket_client_configure_;
			process_message_.monitor_id_ = web_socket_client_configure.monitor_id_;
#ifdef AGENT_TLS
			std::string http_state = "https://";
#else
			std::string http_state = "http://";
#endif
			std::string url = http_state + web_socket_client_configure.connect_address_;

			try {
				endpoint_.reset();
				websocketpp::lib::error_code ec;
				ProcessMessage::client::connection_ptr con = endpoint_.get_connection(url, ec);
				if (ec) {
					LOG_ERROR("Initialize -- %d -- %s", ec.value(), ec.message().c_str());
					endpoint_.get_alog().write(websocketpp::log::alevel::app, ec.message());
					break;
				}
				endpoint_.connect(con);
				process_message_.pendpoint_ = &endpoint_;
				LOG_INFO("Initialize -- Initialize successful");
				// Start the ASIO io_service run loop
				endpoint_.run();
				iret = con->get_ec().value();
				if (iret > 0) {
					LOG_ERROR("Initialize -- %d -- %s", iret, con->get_ec().message().c_str());
				}
			}
			catch (std::exception& e) {
				LOG_ERROR("Run -- %s", e.what());
			}
		} while (false);

		return iret;
	}

	bool MonitorManager::WebSocketClient::IsConnected() {
		return bis_connected_;
	}

#ifdef AGENT_TLS
	MonitorManager::WebSocketClient::context_ptr MonitorManager::WebSocketClient::on_tls_init(websocketpp::connection_hdl) {
		WebSocketClientConfigure& web_socket_client_configure = MonitorConfigure::Instance().web_socket_client_configure_;
		std::string strHome = utils::File::GetBinHome();
		context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

		try {
			ctx->set_options(asio::ssl::context::default_workarounds |
				asio::ssl::context::no_sslv2 |
				asio::ssl::context::no_sslv3 |
				asio::ssl::context::single_dh_use);

			asio::error_code a;
			ctx->load_verify_file(utils::String::Format("%s/%s", strHome.c_str(), web_socket_client_configure.chain_file_.c_str()), a);
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}
		return ctx;
	}
#endif

	void MonitorManager::WebSocketClient::on_open(websocketpp::connection_hdl hdl) {
		try {
			bis_connected_ = true;
			Json::Value parameter;
			process_message_.connection_hdl_ = hdl;
			std::string md5_id = utils::MD5::GenerateMD5((unsigned char*)process_message_.monitor_id_.c_str(), process_message_.monitor_id_.length());
			parameter["id_md"] = utils::MD5::GenerateMD5((unsigned char*)process_message_.monitor_id_.c_str(), process_message_.monitor_id_.length());
			process_message_.SendRequestMessage("hello", true, parameter);
		}
		catch (std::exception& e) {
			LOG_ERROR("on_open -- %s", e.what());
		}
	}

	void MonitorManager::WebSocketClient::on_message(websocketpp::connection_hdl, message_ptr msg) {
		process_message_.PushBackRequestMessages(msg->get_payload());
	}

	void MonitorManager::WebSocketClient::on_close(websocketpp::connection_hdl hdl) {
		process_message_.ResetState();
		bis_connected_ = false;
		LOG_INFO("on_close -- disconnected");
	}

	MonitorManager::MonitorManager() :
		socket_client_(process_message_),
		thread_ptr_(NULL),
		bexit_(true) {}

	MonitorManager::~MonitorManager() {}

	bool MonitorManager::Exit() {
		try {
			bexit_ = true;
			socket_client_.Exit();
			if (thread_ptr_) {
				thread_ptr_->JoinWithStop();
				delete thread_ptr_;
				thread_ptr_ = NULL;
			}
			process_message_.Exit();
		}
		catch (std::exception& e) {
			LOG_ERROR("Exit -- %s", e.what());
		}

		return true;
	}

	bool MonitorManager::Initialize() {
		bool bret = false;
		do {
			if (!process_message_.Initialize())
				break;
			thread_ptr_ = new utils::Thread(this);
			if (false == thread_ptr_->Start("WebSocket Agent"))
				break;
			// register OnTimer
			bubi::TimerNotify::RegisterModule(this);
			bret = true;
		} while (false);

		return bret;
	}

	void MonitorManager::Run(utils::Thread *thread) {
		bexit_ = false;
		do {
			if (!socket_client_.IsConnected()) {
				socket_client_.Initialize();
			}
			socket_client_.Exit();
			if (!bexit_) utils::Sleep(5000);
		} while (thread->enabled());
	}

	void MonitorManager::OnSlowTimer(int64_t current_time) {
		process_message_.OnSlowTimer();
	}

	void MonitorManager::OnTimer(int64_t current_time) {
		process_message_.OnTimer();
	}
}