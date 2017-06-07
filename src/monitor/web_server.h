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

#pragma once
#include <3rd/http/server.hpp>
#include <common/general.h>
#include <common/configure.h>
#include <utils/singleton.h>
#include <utils/net.h>

namespace bubi {

	class WebServer :public utils::Singleton<bubi::WebServer>
	{
		friend class utils::Singleton<bubi::WebServer>;
	public:
		WebServer();
		~WebServer();
	private:
		std::string bubi_info_;
		std::string ledger_info_;
		std::string system_info_;
		utils::AsyncIo *async_io_ptr_;
		http::server::server *server_ptr_;
		asio::ssl::context *context_;
		bool running;

        void FileNotFound(const http::server::request &request, std::string &reply);
		void Hello(const http::server::request &request, std::string &reply);
		void GetBubi(const http::server::request &request, std::string &reply);
		void GetLedger(const http::server::request &request, std::string &reply);
		void GetSystem(const http::server::request &request, std::string &reply);
		void GetWarning(const http::server::request &request, std::string &reply);
        std::string GetCertPassword(std::size_t, asio::ssl::context_base::password_purpose purpose);

	public:
		bool Initialize(WebServerConfigure &webserver_configure);
		bool Exit();
		void SetBubi(const std::string& bubi_info);
		void SetLedger(const std::string& ledger_info);
		void SetSystem(const std::string& system_info);
	};
}