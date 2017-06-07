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
#include "web_server.h"
#include "monitor_manager.h"


namespace bubi {

    WebServer::WebServer() :
        async_io_ptr_(NULL),
        server_ptr_(NULL),
        context_(NULL),
		running(NULL)
    {
		bubi_info_ = ledger_info_ = system_info_ = "please require again,the first require is empty";
    }

    WebServer::~WebServer() {
    }

	bool WebServer::Initialize(WebServerConfigure &webserver_config) {

        if (webserver_config.listen_addresses_.size() == 0) {
            LOG_INFO("Listen address not set, ignore");
            return true;
        }

        if (webserver_config.ssl_enable_) {
            std::string strHome = utils::File::GetBinHome();
            context_ = new asio::ssl::context(asio::ssl::context::sslv23);
            context_->set_options(
                asio::ssl::context::default_workarounds
                | asio::ssl::context::no_sslv2
                | asio::ssl::context::single_dh_use);
            context_->set_password_callback(std::bind(&WebServer::GetCertPassword, this, std::placeholders::_1, std::placeholders::_2));
            context_->use_certificate_chain_file(utils::String::Format("%s/%s", strHome.c_str(), webserver_config.chain_file_.c_str()));
            asio::error_code ignore_code;
            context_->use_private_key_file(utils::String::Format("%s/%s", strHome.c_str(), webserver_config.private_key_file_.c_str()),
                asio::ssl::context::pem,
                ignore_code);
            context_->use_tmp_dh_file(utils::String::Format("%s/%s", strHome.c_str(), webserver_config.dhparam_file_.c_str()));
        }

        utils::InetAddress address = webserver_config.listen_addresses_.front();
        server_ptr_ = new http::server::server(address.ToIp(), address.GetPort(), context_, 8);

        server_ptr_->add404(std::bind(&WebServer::FileNotFound, this, std::placeholders::_1, std::placeholders::_2));
        server_ptr_->addRoute("hello", std::bind(&WebServer::Hello, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("bubi", std::bind(&WebServer::GetBubi, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("ledger", std::bind(&WebServer::GetLedger, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("system", std::bind(&WebServer::GetSystem, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("warning", std::bind(&WebServer::GetWarning, this, std::placeholders::_1, std::placeholders::_2));

        server_ptr_->Run();
		running = true;
        LOG_INFO("Webserver started, listen at %s", address.ToIpPort().c_str());
        return true;
    }

    bool WebServer::Exit() {
        LOG_INFO("WebServer stoping...");
		running = false;
        if (server_ptr_) {
            server_ptr_->Stop();
            delete server_ptr_;
            server_ptr_ = NULL;
        }

        if (context_) {
            delete context_;
            context_ = NULL;
        }
        LOG_INFO("WebServer stop [OK]");
        return true;
    }

    std::string WebServer::GetCertPassword(std::size_t, asio::ssl::context_base::password_purpose purpose) {
        return "bubi";
    }

    void WebServer::FileNotFound(const http::server::request &request, std::string &reply) {
        reply = "File not found";
    }

    void WebServer::Hello(const http::server::request &request, std::string &reply) {
        Json::Value reply_json = Json::Value(Json::objectValue);
        reply_json["monitor_version"] = General::BUBI_VERSION;
        reply_json["current_time"] = utils::Timestamp::Now().ToFormatString(true);
        reply = reply_json.toFastString();
    }

	void WebServer::GetBubi(const http::server::request &request, std::string &reply) {
		do {
			std::string token = request.GetParamValue("token");
			if (token != "monitor056800") {
				reply = "illegal access";
				break;
			}
			bubi::MonitorManager& monitor_messages = bubi::MonitorManager::Instance();
			monitor_messages.SendHttpRequest("", "bubi");
			reply = bubi_info_;
		} while (false);
	}

	void WebServer::GetLedger(const http::server::request &request, std::string &reply) {
		do {
			std::string token = request.GetParamValue("token");
			if (token != "monitor056800") {
				reply = "illegal access";
				break;
			}
			Json::Value result;
			Json::Value parameter;
			if (!parameter.fromString(request.body) && request.body.length() > 0) {
				result["error_code"] = 14;
				result["description"] = "parameter is invalid";
				reply = result.toFastString();
				break;
			}
			int32_t seq = 1;
			int32_t num = 1;
			if (parameter["seq"].empty() != true) {
				seq = parameter["seq"].asInt();
			}
			if (parameter["num"].empty() != true) {
				num = parameter["num"].asInt();
			}
			if (seq <= 0 || num <= 0) {
				result["error_code"] = 14;
				result["description"] = "parameter is invalid";
				reply = result.toFastString();
				break;
			}
			bubi::MonitorManager& monitor_messages = bubi::MonitorManager::Instance();
			monitor_messages.SendHttpRequest("", "ledger");
			reply = ledger_info_;
		} while (false);
	}

	void WebServer::GetSystem(const http::server::request &request, std::string &reply) {
		do {
			std::string token = request.GetParamValue("token");
			if (token != "monitor056800") {
				reply = "illegal access";
				break;
			}
			bubi::MonitorManager& monitor_messages = bubi::MonitorManager::Instance();
			monitor_messages.SendHttpRequest("", "system");
			reply = system_info_;
		} while (false);
	}

	void WebServer::GetWarning(const http::server::request &request, std::string &reply) {
		do {
			std::string token = request.GetParamValue("token");
			if (token != "monitor056800") {
				reply = "illegal access";
				break;
			}
			Json::Value result;
			bubi::MonitorManager& monitor_messages = bubi::MonitorManager::Instance();
			monitor_messages.GetAlert(result);
			if (result.empty()) {
				reply = "alert is empty, no warning";
			}
			else {
				reply = result.toStyledString();
			}
		} while (false);
	}

	void WebServer::SetBubi(const std::string& bubi_info) {
		bubi_info_ = bubi_info;
	}

	void WebServer::SetLedger(const std::string& ledger_info) {
		ledger_info_ = ledger_info;
	}

	void WebServer::SetSystem(const std::string& system_info) {
		system_info_ = system_info;
	}
}
