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

#ifndef WEB_SERVER_H_
#define WEB_SERVER_H_

#include <http/server.hpp>
#include <common/general.h>
#include <common/storage.h>
#include <utils/singleton.h>
#include <proto/message.pb.h>
#include <utils/net.h>

namespace bubi {
	class WebServer :public utils::Singleton<bubi::WebServer>, public bubi::StatusModule {
		friend class utils::Singleton<bubi::WebServer>;
	public:
		WebServer();
        ~WebServer();
        bool Initialize(WebServerConfigure &webserver_configure);
        bool Exit();
        void GetModuleStatus(Json::Value &data);
        static void AssetToJson(const protocol::Asset &asset, Json::Value &js);
        static void GetSources(const std::string &hash, int64_t depth, Json::Value &js);
	private:
		void FileNotFound(const http::server::request &request, std::string &reply);
#ifndef  BUBI_SLAVE
		void Hello(const http::server::request &request, std::string &reply);
		void CreateAccount(const http::server::request &request, std::string &reply);
		void GetAccount(const http::server::request &request, std::string &reply);
		void CreateTransaction(const http::server::request &request, std::string &reply);
		void GetTransactionBlob(const http::server::request &request, std::string &reply);
		void UpdateLogLevel(const http::server::request &request, std::string &reply);
		void GetTransactionHistory(const http::server::request &request, std::string &reply);
		void GetRecord(const http::server::request &request, std::string &reply);
		void GetUniqueAsset(const http::server::request &request, std::string &reply);
		void GetStatus(const http::server::request &request, std::string &reply);
		void GetModulesStatus(const http::server::request &request, std::string &reply);
		void GetLedger(const http::server::request &request, std::string &reply);
		void GetAddress(const http::server::request &request, std::string &reply);
		void GetPeerNodeAddress(const http::server::request &request, std::string &reply);
		void GetAssetRank(const http::server::request &request, std::string &reply);
		void GetTransactionFromBlob(const http::server::request &request, std::string &reply);
		void GetConsensusInfo(const http::server::request &request, std::string &reply);
		void getSources(const http::server::request &request, std::string &reply);
		void MultiQuery(const http::server::request &request, std::string &reply);
#endif
		std::string GetCertPassword(std::size_t, asio::ssl::context_base::password_purpose purpose);
		void SubmitTransaction(const http::server::request &request, std::string &reply);
		//operations
		bool CreateAccountOpeFrm_FromJson(protocol::OperationCreateAccount* ope_create_account, const Json::Value& js, Result &result);
		bool PaymentOpeFrm_FromJson(protocol::OperationPayment* ope_payment, const Json::Value& js, Result &result);
		bool IssueOpeFrm_FromJson(protocol::OperationIssueAsset* ope_issue_asset, const Json::Value& js, Result &result);
		bool IssueUniqueAssetOpeFrm_FromJson(protocol::OperationIssueUniqueAsset* ope_issue_unique_asset, const Json::Value& js, Result &result);
		bool PaymentUniqueAssetOpeFrm_FromJson(protocol::OperationPaymentUniqueAsset* ope_payment, const Json::Value& js, Result &result);
		bool SetOptionsOpeFrm_FromJson(protocol::OperationSetOptions* ope_set_opetions, const Json::Value& js, Result &result);
		bool ProductionFrm_FromJson(protocol::OperationProduction *ope_production, const Json::Value &js, Result &result);
		bool InitPaymentOpeFrm_FromJson(protocol::OperationInitPayment *ope_init_payment, const Json::Value &js, Result &result);
		bool RecordOpeFrm_FromJson(protocol::OperationRecord *ope_record, const Json::Value &js, Result &result);
		bool MakeTransactionHelper(const Json::Value &object, protocol::Transaction *tran, Result& result);
	private:
        utils::AsyncIo *async_io_ptr_;
        http::server::server *server_ptr_;
        asio::ssl::context *context_;
        std::shared_ptr<RationalDb> rational_db_;
        bool running_;
		size_t thread_count_;
	};
}

#endif
