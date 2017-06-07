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
#include <common/general.h>
#include <common/configure.h>
#include <common/private_key.h>
#include <glue/glue_manager.h>
#include <ledger/transaction_frm.h>
#ifndef  BUBI_SLAVE
#include <ledger/ledger_manager.h>
#endif //BUBI_SLAVE
#include <overlay/peer_manager.h>
#include <ledger/production_ope_frm.h>
#include "slave/master_service.h"
#include "slave/slave_service.h"
#include "web_server.h"

#include "monitor/monitor_master.h"

namespace bubi {
	WebServer::WebServer() :
		async_io_ptr_(NULL),
		server_ptr_(NULL),
		context_(NULL),
		rational_db_(NULL),
		running_(NULL),
		thread_count_(0){}

	WebServer::~WebServer() {}

	bool WebServer::Initialize(WebServerConfigure &webserver_config) {

		if (webserver_config.listen_addresses_.size() == 0) {
			LOG_INFO("Listen address not set, ignore");
			return true;
		}
#ifndef  BUBI_SLAVE
		rational_db_ = Storage::Instance().NewRationalDb();
		if (!rational_db_) {
			return false;
		}
#endif
		if (webserver_config.ssl_enable_) {
			std::string strHome = utils::File::GetBinHome();
			context_ = new asio::ssl::context(asio::ssl::context::sslv23);
			context_->set_options(
				asio::ssl::context::default_workarounds
				| asio::ssl::context::no_sslv2
				| asio::ssl::context::single_dh_use);
			context_->set_password_callback(std::bind(&WebServer::GetCertPassword, this, std::placeholders::_1, std::placeholders::_2));
			context_->use_certificate_chain_file(utils::String::Format("%s/%s", strHome.c_str(), webserver_config.ssl_configure_.chain_file_.c_str()));
			asio::error_code ignore_code;
			context_->use_private_key_file(utils::String::Format("%s/%s", strHome.c_str(), webserver_config.ssl_configure_.private_key_file_.c_str()),
				asio::ssl::context::pem,
				ignore_code);
			context_->use_tmp_dh_file(utils::String::Format("%s/%s", strHome.c_str(), webserver_config.ssl_configure_.dhparam_file_.c_str()));
		}

		thread_count_ = webserver_config.thread_count_;
		if (thread_count_ == 0){
			thread_count_ = utils::GetCpuCoreCount() * 8;
		}

		utils::InetAddress address = webserver_config.listen_addresses_.front();
		server_ptr_ = new http::server::server(address.ToIp(), address.GetPort(), context_, thread_count_);
		server_ptr_->add404(std::bind(&WebServer::FileNotFound, this, std::placeholders::_1, std::placeholders::_2));

#ifndef  BUBI_SLAVE
		server_ptr_->addRoute("hello", std::bind(&WebServer::Hello, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("createAccount", std::bind(&WebServer::CreateAccount, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getAccount", std::bind(&WebServer::GetAccount, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getTransactionBlob", std::bind(&WebServer::GetTransactionBlob, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getTransactionHistory", std::bind(&WebServer::GetTransactionHistory, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getRecord", std::bind(&WebServer::GetRecord, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getUniqueAsset", std::bind(&WebServer::GetUniqueAsset, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getStatus", std::bind(&WebServer::GetStatus, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getLedger", std::bind(&WebServer::GetLedger, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getModulesStatus", std::bind(&WebServer::GetModulesStatus, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getConsensusInfo", std::bind(&WebServer::GetConsensusInfo, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("updateLogLevel", std::bind(&WebServer::UpdateLogLevel, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getAddress", std::bind(&WebServer::GetAddress, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getSources", std::bind(&WebServer::getSources, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getTransactionFromBlob", std::bind(&WebServer::GetTransactionFromBlob, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getAssetRank", std::bind(&WebServer::GetAssetRank, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("getPeerNodeAddress", std::bind(&WebServer::GetPeerNodeAddress, this, std::placeholders::_1, std::placeholders::_2));
		server_ptr_->addRoute("multiQuery", std::bind(&WebServer::MultiQuery, this, std::placeholders::_1, std::placeholders::_2));
#endif

		server_ptr_->addRoute("submitTransaction", std::bind(&WebServer::SubmitTransaction, this, std::placeholders::_1, std::placeholders::_2));

		server_ptr_->Run();
		running_ = true;

		StatusModule::RegisterModule(this);
		LOG_INFO("Webserver started, thread count(" FMT_SIZE ") listen at %s", thread_count_, address.ToIpPort().c_str());
		return true;
	}

	bool WebServer::Exit() {
		LOG_INFO("WebServer stoping...");
		running_ = false;
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

#ifndef  BUBI_SLAVE
	void WebServer::Hello(const http::server::request &request, std::string &reply) {
		Json::Value reply_json = Json::Value(Json::objectValue);
		reply_json["bubi_version"] = General::BUBI_VERSION;
		reply_json["ledger_version"] = utils::String::ToString(General::LEDGER_VERSION);
		reply_json["overlay_version"] = utils::String::ToString(General::OVERLAY_VERSION);
		reply_json["current_time"] = utils::Timestamp::Now().ToFormatString(true);
		reply = reply_json.toFastString();
	}

	void WebServer::CreateAccount(const http::server::request &request, std::string &reply) {
		std::string error_desc;
		int32_t error_code = protocol::ERRCODE_SUCCESS;
		Json::Value reply_json = Json::Value(Json::objectValue);
		do {
			PrivateKey priv_key(ED25519SIG);
			std::string public_key = priv_key.GetBase58PublicKey();
			std::string private_key = priv_key.GetBase58PrivateKey();
			std::string public_address = priv_key.GetBase58Address();

			LOG_TRACE("Creating account address:%s", public_address.c_str());

			Json::Value &result = reply_json["result"];
			result["public_key"] = public_key;
			result["private_key"] = private_key;
			result["address"] = public_address;
			result["public_key_raw"] = utils::String::BinToHexString(priv_key.GetPublicKey());

		} while (false);
		reply_json["error_code"] = error_code;
		reply = reply_json.toFastString();
	}

	void WebServer::GetAccount(const http::server::request &request, std::string &reply) {
		std::string address = request.GetParamValue("address");

		int32_t error_code = protocol::ERRCODE_SUCCESS;
		AccountFrm::pointer acc = NULL;
		int64_t balance = 0;
		Json::Value reply_json = Json::Value(Json::objectValue);
		Json::Value record = Json::Value(Json::arrayValue);
		Json::Value &result = reply_json["result"];

		if (!LedgerManager::Instance().GetAccountEntry(address, acc)) {
			error_code = protocol::ERRCODE_NOT_EXIST;
			LOG_TRACE("GetAccount fail, account(%s) not exist", address.c_str());
		}
		else {
			acc->ToJson(result);
		}

		reply_json["error_code"] = error_code;
		reply = reply_json.toStyledString();
	}

	void WebServer::GetTransactionBlob(const http::server::request &request, std::string &reply) {
		Result result;
		Json::Value reply_json = Json::Value(Json::objectValue);
		Json::Value &js_result = reply_json["result"];
		do {
			Json::Value body;
			if (!body.fromString(request.body)) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("");
				break;
			}

			protocol::Transaction tran;
			if (!WebServer::Instance().MakeTransactionHelper(body, &tran, result)) {
				break;
			}

			std::string SerializeString;
			if (!tran.SerializeToString(&SerializeString)) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("");
				LOG_INFO("SerializeToString Transaction Failed");
				break;
			}

			std::string crypto = utils::encode_b16(SerializeString);
			js_result["transaction_blob"] = crypto;
			js_result["hash"] = utils::encode_b16(utils::Sha256::Crypto(SerializeString));
		} while (false);

		reply_json["error_code"] = result.code();
		reply_json["error_desc"] = result.desc();
		reply = reply_json.toStyledString();
	}

	void WebServer::GetTransactionFromBlob(const http::server::request &request, std::string &reply) {
		Result result_e;
		result_e.set_code(protocol::ERRCODE_SUCCESS);
		Json::Value reply_json = Json::Value(Json::objectValue);
		Json::Value &result = reply_json["result"];

		std::string blob = request.GetParamValue("blob");
		std::string env = request.GetParamValue("env");
		do {
			std::string decodeblob;
			if (!utils::String::HexStringToBin(blob, decodeblob)) {
				result_e.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result_e.set_desc("'transaction_blob' value must be Hex");
				break;
			}

			protocol::TransactionEnv tran_env;
			if (env == "true") {
				if (!tran_env.ParseFromString(decodeblob)) {
					result_e.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result_e.set_desc("Parse From env String from decodeblob invalid");
					LOG_ERROR("ParseFromString from decodeblob invalid");
					break;
				}
			}
			else {
				protocol::Transaction *tran = tran_env.mutable_transaction();
				if (!tran->ParseFromString(decodeblob)) {
					result_e.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result_e.set_desc("Parse From String from decodeblob invalid");
					LOG_ERROR("ParseFromString from decodeblob invalid");
					break;
				}
			}

			TransactionFrm frm(tran_env);
			frm.ToJson(reply_json);

		} while (false);

		reply_json["error_code"] = result_e.code();
		reply_json["error_desc"] = result_e.desc();
		reply = reply_json.toStyledString();
	}
	void WebServer::GetTransactionHistory(const http::server::request &request, std::string &reply) {
		WebServerConfigure &web_config = Configure::Instance().webserver_configure_;

		std::string address = request.GetParamValue("address");
		std::string seq = request.GetParamValue("ledger_seq");
		std::string hash = request.GetParamValue("hash");
		std::string str_order = request.GetParamValue("order");
		std::string start_str = request.GetParamValue("start");
		std::string limit_str = request.GetParamValue("limit");
		std::string with_ledger = request.GetParamValue("with_ledger");

		if (str_order == "DESC" ||
			str_order == "desc" ||
			str_order == "asc" ||
			str_order == "ASC") {
		}
		else {
			str_order = "DESC";
		}

		int32_t error_code = protocol::ERRCODE_SUCCESS;
		Json::Value reply_json = Json::Value(Json::objectValue);

		Json::Value &result = reply_json["result"];
		Json::Value &txs = result["transactions"];
		txs = Json::Value(Json::arrayValue);
		do {
			if (start_str.empty()) start_str = "0";
			if (!utils::String::is_number(start_str) == 1) {
				error_code = protocol::ERRCODE_INVALID_PARAMETER;
				break;
			}
			uint32_t start = utils::String::Stoui(start_str);


			if (limit_str.empty()) limit_str = "20";
			if (!utils::String::is_number(limit_str) == 1) {
				error_code = protocol::ERRCODE_INVALID_PARAMETER;
				break;
			}
			uint32_t limit = utils::String::Stoui(limit_str);
			limit = MIN(limit, web_config.query_limit_);


			//if (start_str.empty()) start_str = "0";
			//uint32_t start = utils::String::Stoui(start_str);
			//if (limit_str.empty()) limit_str = "20";
			//uint32_t limit = utils::String::Stoui(limit_str);
			//limit = MIN(limit, web_config.query_limit_);

			std::string table_name = General::TX_TABLE_NAME;
			std::string hash_name = "hash";
			std::string condition = "WHERE 1=1 ";
			size_t condition_size = condition.size();

			if (!address.empty()) {
				condition += utils::String::Format(" AND account='%s' ", rational_db_->Format(address).c_str());
				table_name = General::ACCOUNT_TX_NAME;
				hash_name = "trans_id";
			}

			if (!seq.empty() || !hash.empty()) {
				if (!hash.empty()) {
					condition += utils::String::Format("AND %s='%s'", hash_name.c_str(), hash.c_str());
				}
				else if (!seq.empty()) {
					condition += utils::String::Format("AND ledger_seq=%s", seq.c_str());
				}
			}

			std::string sql = utils::String::Format("SELECT %s, seq_in_global FROM %s %s ORDER BY seq_in_global %s OFFSET %u LIMIT %u",
				hash_name.c_str(),
				table_name.c_str(),
				condition.c_str(),
				str_order.c_str(),
				start, limit);
			
			//avoid scan the whole table
			 protocol::LedgerHeader header = LedgerManager::Instance().GetLastClosedLedger();
			if (condition.size() == condition_size && table_name == General::TX_TABLE_NAME) {
				result["total_count"] = header.tx_count();
			}
			else {
				Json::Value ret;
				int32_t nret = rational_db_->QueryRecord(utils::String::Format("SELECT COUNT(1) AS count FROM %s %s", table_name.c_str(), condition.c_str()), ret);
				if (nret < 0) {
					LOG_ERROR_ERRNO("excute query failed",
						rational_db_->error_code(), rational_db_->error_desc());
					error_code = protocol::ERRCODE_INTERNAL_ERROR;
					break;
				}
				result["total_count"] = ret["count"].asInt64();
			}
			
			if (with_ledger == "true") {
				LedgerFrm ledger_frm(header);
				result["last_ledger"] = ledger_frm.ToJson();
			}

			Json::Value record;
			if (rational_db_->Query(sql, record) < 0) {
				LOG_ERROR_ERRNO("query sql(%s) failed", sql.c_str(), rational_db_->error_code(), rational_db_->error_desc());
				error_code = protocol::ERRCODE_INTERNAL_ERROR;
				break;
			}

			if (record.size() == 0) {
				error_code = protocol::ERRCODE_NOT_EXIST;
				break;
			}

			for (size_t i = 0; i < record.size() && error_code == protocol::ERRCODE_SUCCESS; i++) {
				Json::Value &item = record[i];
				TransactionFrm txfrm;
				txfrm.LoadFromDb(item[hash_name].asString());
				Json::Value m;
				txfrm.ToJson(m);
				txs[(Json::UInt) i] = m;
			}
		} while (false);

		reply_json["error_code"] = error_code;
		reply = reply_json.toStyledString();
	}

	void WebServer::GetRecord(const http::server::request &request, std::string &reply) {
		WebServerConfigure &web_config = Configure::Instance().webserver_configure_;
		std::string id = request.GetParamValue("id");


		std::string record_participant = request.GetParamValue("record_participant");
		std::string record_address = request.GetParamValue("record_address");

		std::string str_order = request.GetParamValue("order");
		std::string start_str = request.GetParamValue("start");
		std::string limit_str = request.GetParamValue("limit");

		if (str_order == "DESC" ||
			str_order == "desc" ||
			str_order == "asc" ||
			str_order == "ASC") {
		}
		else {
			str_order = "DESC";
		}

		if (start_str.empty()) start_str = "0";
		uint32_t start = utils::String::Stoui(start_str);


		if (limit_str.empty()) limit_str = "20";
		uint32_t limit = utils::String::Stoui(limit_str);
		limit = MIN(limit, web_config.query_limit_);


		int32_t error_code = protocol::ERRCODE_SUCCESS;
		Json::Value reply_json = Json::Value(Json::objectValue);

		std::string condition = "WHERE 1=1";
		if (!id.empty()) {
			condition += utils::String::Format(" AND %s.record_id='%s' ",
				General::RECORD_TABLE_NAME, rational_db_->Format(id).c_str());
		}

		// find record
		if (!record_address.empty()) {
			condition += utils::String::Format(" AND ( (record_address='' AND record_participant='%s')",
				rational_db_->Format(record_address).c_str());

			if (!record_participant.empty()) {
				//find additional record
				if (record_participant == "all") {
					condition += utils::String::Format(" OR (record_address='%s') ",
						rational_db_->Format(record_address).c_str());
				}
				//find addrdss additional record
				else {
					condition += utils::String::Format(" OR (record_address='%s' AND record_participant='%s') ",
						rational_db_->Format(record_address).c_str(), rational_db_->Format(record_participant).c_str());
				}
			}
			condition += utils::String::Format(")");
		}




		std::string sql = "";
		sql = utils::String::Format("SELECT * FROM %s %s  ORDER BY ledger_seq  %s OFFSET %u LIMIT %u",
			General::RECORD_TABLE_NAME,
			condition.c_str(),
			str_order.c_str(),
			start, limit);



		Json::Value &result = reply_json["result"];

		do {
			Json::Value record;
			if (rational_db_->Query(sql, record) < 0) {
				LOG_ERROR("Query sql(%s) failed", sql.c_str());
				error_code = protocol::ERRCODE_INTERNAL_ERROR;
				break;
			}

			if (record.size() == 0) {
				error_code = protocol::ERRCODE_NOT_EXIST;
				break;
			}

			for (size_t i = 0; i < record.size(); i++) {
				Json::Value &item = record[i];
				item["record_ext"] = item["record_ext"].asString();
				result.append(item);
			}

		} while (false);

		reply_json["error_code"] = error_code;
		reply = reply_json.toStyledString();
	}

	void WebServer::GetUniqueAsset(const http::server::request &request, std::string &reply) {
		WebServerConfigure &web_config = Configure::Instance().webserver_configure_;

		std::string asset_issuer = request.GetParamValue("asset_issuer");
		std::string asset_code = request.GetParamValue("asset_code");

		std::string str_order = request.GetParamValue("order");
		std::string start_str = request.GetParamValue("start");
		std::string limit_str = request.GetParamValue("limit");

		if (str_order == "DESC" ||
			str_order == "desc" ||
			str_order == "asc" ||
			str_order == "ASC") {
		}
		else {
			str_order = "DESC";
		}

		if (start_str.empty()) start_str = "0";
		uint32_t start = utils::String::Stoui(start_str);


		if (limit_str.empty()) limit_str = "20";
		uint32_t limit = utils::String::Stoui(limit_str);
		limit = MIN(limit, web_config.query_limit_);

		int32_t error_code = protocol::ERRCODE_SUCCESS;
		Json::Value reply_json = Json::Value(Json::objectValue);

		std::string condition = "WHERE 1=1";
		if (!asset_issuer.empty()) {
			condition += utils::String::Format(" AND %s.asset_issuer='%s' ",
				General::UNIQUE_ASSET_NAME, rational_db_->Format(asset_issuer).c_str());
		}

		if (!asset_code.empty()) {
			condition += utils::String::Format(" AND %s.asset_code='%s' ",
				General::UNIQUE_ASSET_NAME, rational_db_->Format(asset_code).c_str());
		}

		std::string sql = "";
		sql = utils::String::Format("SELECT * FROM %s %s  ORDER BY ledger_seq  %s OFFSET %u LIMIT %u",
			General::UNIQUE_ASSET_NAME,
			condition.c_str(),
			str_order.c_str(),
			start, limit);

		Json::Value &result = reply_json["result"];

		do {
			Json::Value record;
			if (rational_db_->Query(sql, record) < 0) {
				LOG_ERROR("Query sql(%s) failed", sql.c_str());
				error_code = protocol::ERRCODE_INTERNAL_ERROR;
				break;
			}

			if (record.size() == 0) {
				error_code = protocol::ERRCODE_NOT_EXIST;
				break;
			}

			for (size_t i = 0; i < record.size(); i++) {
				Json::Value &item = record[i];
				result.append(item);
			}


		} while (false);
		reply_json["error_code"] = error_code;
		reply = reply_json.toStyledString();
	}

	void WebServer::GetStatus(const http::server::request &request, std::string &reply) {
		uint32_t error_code = protocol::ERRCODE_SUCCESS;
		Json::Value reply_json = Json::Value(Json::objectValue);
		Json::Value &result = reply_json["result"];

		const protocol::LedgerHeader &ledger = LedgerManager::Instance().GetLastClosedLedger();
		result["transaction_count"] = ledger.tx_count();
		result["account_count"] = LedgerManager::Instance().GetAccountNum();

		reply_json["error_code"] = error_code;
		reply = reply_json.toStyledString();
	}


	void WebServer::GetModulesStatus(const http::server::request &request, std::string &reply) {
		utils::ReadLockGuard guard(bubi::StatusModule::status_lock_);
		Json::Value reply_json = *bubi::StatusModule::modules_status_;

		reply_json["keyvalue_db"] = Json::Value(Json::objectValue);
		bubi::Storage::Instance().keyvalue_db()->GetOptions(reply_json["keyvalue_db"]);

		reply = reply_json.toStyledString();
	}

	void WebServer::GetLedger(const http::server::request &request, std::string &reply) {
		std::string ledger_seq = request.GetParamValue("seq");

		/// default last closed ledger
		if (ledger_seq.empty())
			ledger_seq = utils::String::ToString(LedgerManager::Instance().GetLastClosedLedger().ledger_sequence());


		int32_t error_code = protocol::ERRCODE_SUCCESS;
		Json::Value reply_json = Json::Value(Json::objectValue);
		Json::Value record = Json::Value(Json::arrayValue);
		Json::Value &result = reply_json["result"];

		ledger_seq = rational_db_->Format(ledger_seq);
		LedgerFrm frm;
		do {
			if (!frm.LoadFromDb(utils::String::Stoi64(ledger_seq))) {
				error_code = protocol::ERRCODE_NOT_EXIST;
				break;
			}
			result = frm.ToJson();
		} while (false);

		reply_json["error_code"] = error_code;
		reply = reply_json.toStyledString();
	}

	void WebServer::GetConsensusInfo(const http::server::request &request, std::string &reply) {

		Json::Value root;
		ConsensusManager::Instance().GetConsensus()->GetModuleStatus(root);
		reply = root.toStyledString();
	}

	void WebServer::GetAddress(const http::server::request &request, std::string &reply) {
		std::string private_key = request.GetParamValue("private_key");
		std::string public_key = request.GetParamValue("public_key");
		Json::Value reply_json = Json::Value(Json::objectValue);

		if (!private_key.empty()) {
			PrivateKey key(private_key);
			if (key.IsValid()) {
				reply_json["error_code"] = protocol::ERRCODE_SUCCESS;
				Json::Value &result = reply_json["result"];
				result["public_key"] = key.GetBase58PublicKey();
				result["private_key"] = key.GetBase58PrivateKey();
				result["address"] = key.GetBase58Address();
				result["private_raw"] = key.GetRawPrivateKey();
				result["public_key_raw"] = utils::String::BinToHexString(key.GetPublicKey());
			}
			else {
				reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
			}
		}
		else if (!public_key.empty()) {
			PublicKey key(public_key);
			if (key.IsValid()) {
				reply_json["error_code"] = protocol::ERRCODE_SUCCESS;
				Json::Value &result = reply_json["result"];
				result["public_key"] = key.GetBase58PublicKey();
				result["address"] = key.GetBase58Address();
			}
			else {
				reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
			}
		}
		else {
			reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
		}

		reply = reply_json.toStyledString();
	}

	void WebServer::GetPeerNodeAddress(const http::server::request &request, std::string &reply) {
		std::string token = request.GetParamValue("token");
		if (token != "bubiokqwer") {
			reply = "Access is not valid";
			return;
		}

		bubi::KeyValueDb *db = bubi::Storage::Instance().keyvalue_db();
		std::string key = utils::String::Format("%s_nodeprivkey", bubi::General::OVERLAY_PREFIX);
		std::string name;
		bubi::PrivateKey priv_key(bubi::ED25519SIG);
		if (db->Get(key, name) && priv_key.From(name)) {
			reply = utils::String::Format("%s", priv_key.GetBase58Address().c_str());
		}
		else {
			reply = "address not exist";
		}
	}

	static bool AssetAmountSorter(std::pair < std::string, int64_t> const& ac1, std::pair < std::string, int64_t> const& ac2) {
		// need to use the hash of whole tx here since multiple txs could have
		// the same Contents
		return ac1.second > ac2.second;
	}

	void WebServer::GetAssetRank(const http::server::request &request, std::string &reply) {
		if (!Configure::Instance().webserver_configure_.remote_authorized_ && !request.peer_address_.IsLoopback()) {
			reply = "Not authorized";
			return;
		}

		Json::Value reply_json = Json::Value(Json::objectValue);
		do {
			reply_json["error_code"] = protocol::ERRCODE_SUCCESS;
			Json::Value &result = reply_json["result"];

			//Check request
			Json::Value request_json = Json::Value(Json::objectValue);
			if (!request_json.fromString(request.body)) {
				reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
				break;
			}

			if (!request_json.isMember("asset_issuer") ||
				!bubi::PublicKey::IsAddressValid(request_json["asset_issuer"].asString())) {
				reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
				break;
			}
			std::string asset_issuer = request_json["asset_issuer"].asString();

			if (!request_json.isMember("asset_code") ||
				request_json["asset_code"].asString().length() > ASSET_CODE_MAX_SIZE ||
				request_json["asset_code"].asString().length() == 0
				) {
				reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
				break;
			}
			std::string asset_code = request_json["asset_code"].asString();

			int64_t count = -1;
			if (request_json.isMember("count")) {
				if (!request_json["count"].isInt64()
					|| request_json["count"].asInt64() < 0) {
					reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
					break;
				}
				count = request_json["count"].asInt64();
			}

			std::vector<std::string > filter_addresss;
			if (request_json.isMember("filter_address") && request_json["filter_address"].size() > 0) {
				for (size_t i = 0; i < request_json["filter_address"].size(); i++) {
					if (!bubi::PublicKey::IsAddressValid(request_json["filter_address"][i].asString())) {
						reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
						break;
					}
					filter_addresss.push_back(request_json["filter_address"][i].asString());
				}

			}
			filter_addresss.push_back(asset_issuer);

			//  have asset address list
			std::vector < std::pair < std::string, int64_t>>  address_list;

			//asset
			protocol::AssetProperty asset_property;
			asset_property.set_type(protocol::AssetProperty_Type_IOU);
			asset_property.set_code(asset_code);
			asset_property.set_issuer(asset_issuer);

			//load from DB
			bubi::KeyValueDb* db = Storage::Instance().keyvalue_db();
			KVDB::Iterator* it = (KVDB::Iterator*)(db->NewIterator());

			for (it->SeekToFirst(); it->Valid(); it->Next()) {

				if (it->key() == ACCOUNT_TREE_ROOT_KEY) {
					//skip root node
					continue;
				}

				if (it->key().size() != 32) {
					//skip non-account record
					continue;
				}

				AccountFrm::pointer acc = std::make_shared<AccountFrm>();
				if (!acc->UnSerializer(it->value().ToString()))
					continue;

				//  filter address
				if (std::find(filter_addresss.begin(), filter_addresss.end(), acc->GetAccountAddress()) != filter_addresss.end())
					continue;

				int64_t asset_amount = acc->GetAssetAmount(asset_property);
				// have asset
				if (asset_amount > 0) {
					address_list.push_back(std::make_pair(acc->GetAccountAddress(), asset_amount));
				}
			}

			// sort 
			std::sort(address_list.begin(), address_list.end(), AssetAmountSorter);

			for (int64_t i = 0; i < address_list.size() && (count == -1 || i < count); i++) {
				Json::Value address_js;
				address_js["number"] = i + 1;
				address_js["address"] = address_list[i].first;
				address_js["amount"] = address_list[i].second;
				result["addresss"].append(address_js);
			}
		} while (false);
		reply = reply_json.toStyledString();
	}

	void WebServer::UpdateLogLevel(const http::server::request &request, std::string &reply) {
		std::string levelreq = request.GetParamValue("level");
		utils::LogLevel loglevel = utils::LOG_LEVEL_ALL;
		std::string loglevel_info = "LOG_LEVEL_ALL";
		if (levelreq == "1") {
			loglevel = (utils::LogLevel)(utils::LOG_LEVEL_ALL & ~utils::LOG_LEVEL_TRACE);
			loglevel_info = "LOG_LEVEL_ALL & ~utils::LOG_LEVEL_TRACE";
		}

		utils::Logger::Instance().SetLogLevel(loglevel);
		reply = utils::String::Format("set log level to %s", loglevel_info.c_str());

	}

	void WebServer::GetSources(const std::string &hash, int64_t depth, Json::Value& js) {
		js["hash"] = hash;

		TransactionFrm tx;
		uint32_t load_code = tx.LoadFromDb(hash);
		js["error_code"] = load_code;

		if (load_code != protocol::ERRCODE_SUCCESS) {
			return;
		}
		if (tx.GetResult().code() != protocol::ERRCODE_SUCCESS) {
			js["error_code"] = tx.GetResult().code();
			return;
		}

		if (tx.GetTransactionEnv().transaction().operations().size() != 1) {
			js["error_code"] = protocol::ERRCODE_INTERNAL_ERROR;
			return;
		}

		protocol::Operation ope = tx.GetTransactionEnv().transaction().operations(0);
		if (!ope.has_production()) {
			js["error_code"] = protocol::ERRCODE_INTERNAL_ERROR;
			return;
		}
		protocol::OperationProduction product = ope.production();
		js["address"] = tx.GetSourceAddress();

		if (js.isMember("index")) {
			int32_t nindex = js["index"].asInt();
			if (nindex < product.outputs_size() && nindex >= 0) {
				if (product.outputs(nindex).has_metadata()) {
					js["output_meta"] = utils::String::BinToHexString(product.outputs(nindex).metadata());
				}
			}
		}

		if (depth == 0) {
			return;
		}


		Json::Value &js_s = js["from"];
		for (int i = 0; i < product.inputs_size(); i++) {
			std::string hash_pre = utils::String::BinToHexString(product.inputs(i).hash());
			js_s[i]["index"] = product.inputs(i).index();
			if (product.inputs(i).has_metadata())
				js_s[i]["input_meta"] = utils::String::BinToHexString(product.inputs(i).metadata());
			GetSources(hash_pre, depth - 1, js_s[i]);
		}
	}

	void WebServer::getSources(const http::server::request &request, std::string &reply) {
		std::string depth = request.GetParamValue("depth");
		std::string hash = request.GetParamValue("hash");
		int64_t ndepth = utils::String::Stoui64(depth);

		Json::Value js;
		Json::Value rep;
		do {
			TransactionFrm tx;
			uint32_t load_code = tx.LoadFromDb(hash);
			if (load_code != protocol::ERRCODE_SUCCESS) {
				rep["error_code"] = load_code;
				break;
			}

			if (tx.GetTransactionEnv().transaction().operations().size() != 1) {
				rep["error_code"] = protocol::ERRCODE_NOT_SUPPLY;
				break;
			}

			protocol::Operation ope = tx.GetTransactionEnv().transaction().operations(0);
			if (!ope.has_production()) {
				rep["error_code"] = protocol::ERRCODE_NOT_SUPPLY;
				break;
			}

			if (tx.GetResult().code() != protocol::ERRCODE_SUCCESS) {
				rep["error_code"] = tx.GetResult().code();
				break;
			}
			WebServer::GetSources(hash, ndepth, js);
			rep["error_code"] = js["error_code"];
		} while (false);

		rep["result"] = js;
		reply = rep.toStyledString();
	}

	void WebServer::MultiQuery(const http::server::request &request, std::string &reply) {
		WebServerConfigure &web_config = Configure::Instance().webserver_configure_;
		Json::Value reply_json = Json::Value(Json::objectValue);
		Json::Value &results = reply_json["results"];

		do {
			Json::Value req;
			if (!req.fromString(request.body)) {
				LOG_ERROR("Parse request body json failed");
				reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
				break;
			}

			const Json::Value &items = req["items"];

			if (items.size() > web_config.multiquery_limit_) {
				LOG_ERROR("MultiQuery size is too larger than %u", web_config.multiquery_limit_);
				reply_json["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
				break;
			}

			for (uint32_t i = 0; i < items.size(); i++) {
				const Json::Value &item = items[i];
				Json::Value &result = results[i];
				std::string url = item["url"].asString();
				std::string method = item["method"].asString();

				http::server::request request_inner;
				if (item.isMember("jsonData")) {
					const Json::Value &nRequestJsonData = item["jsonData"];
					if (nRequestJsonData.isString()) {
						request_inner.body = nRequestJsonData.asString();
					}
					else {
						request_inner.body = nRequestJsonData.toFastString();
					}
				}

				std::string reply_inner;
				request_inner.uri = url;
				request_inner.method = method;
				request_inner.Update();

				http::server::server::routeHandler *handle = server_ptr_->getRoute(request_inner.command);
				if (handle) {
					(*handle)(request_inner, reply_inner);
				}

				result.fromString(reply_inner);
			}

			reply_json["error_code"] = 0;
		} while (false);

		reply = reply_json.toStyledString();
	}

#endif //BUBI_SLAVE

	bool WebServer::CreateAccountOpeFrm_FromJson(protocol::OperationCreateAccount* ope_create_account, const Json::Value& js, Result &result) {
		if (!js.isMember("dest_address") ||
			!bubi::PublicKey::IsAddressValid(js["dest_address"].asString())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'dest_address' value not exist or parameter error");
			return false;
		}
		ope_create_account->set_dest_address(js["dest_address"].asString());

		if (!js.isMember("init_balance") ||
			!js["init_balance"].isInt64() ||
			js["init_balance"].asInt64() < 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'init_balance' not exist or parameter error,'init_balance' value must be greater than 0");
			return false;
		}
		ope_create_account->set_init_balance(js["init_balance"].asInt64());

		if (js.isMember("account_metadata")) {
			std::string meta_bin;
			if (!utils::String::HexStringToBin(js["account_metadata"].asString(), meta_bin) ||
				meta_bin.size() > METADATA_MAXSIZE) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'account_metadata' value must be Hex,'account_metadata' value is in the range of 0 and %d", (METADATA_MAXSIZE / 2)));
				return false;
			}
			ope_create_account->set_account_metadata(meta_bin);
		}

		if (js.isMember("threshold")) {
			protocol::AccountThreshold *acc_threshold = ope_create_account->mutable_thresholds();
			const Json::Value &json_threshold = js["threshold"];
			if (json_threshold.isMember("master_weight")) {
				if (!json_threshold["master_weight"].isInt64() ||
					json_threshold["master_weight"].asInt() < 0 ||
					json_threshold["master_weight"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'master_weight' parameter error,'master_weight' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				acc_threshold->set_master_weight(json_threshold["master_weight"].asInt());
			}

			if (json_threshold.isMember("low_threshold")) {
				if (!json_threshold["low_threshold"].isInt64() ||
					json_threshold["low_threshold"].asInt() < 0 ||
					json_threshold["low_threshold"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'low_threshold' not exist or parameter error,'low_threshold' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				acc_threshold->set_low_threshold(json_threshold["low_threshold"].asInt());
			}

			if (json_threshold.isMember("med_threshold")) {
				if (!json_threshold["med_threshold"].isInt64() ||
					json_threshold["med_threshold"].asInt() < 0 ||
					json_threshold["med_threshold"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'med_threshold' not exist or parameter error,'med_threshold' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				acc_threshold->set_med_threshold(json_threshold["med_threshold"].asInt());
			}

			if (json_threshold.isMember("high_threshold")) {
				if (!json_threshold["high_threshold"].isInt64() ||
					json_threshold["high_threshold"].asInt() < 0 ||
					json_threshold["high_threshold"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'high_threshold' not exist or parameter error,'high_threshold' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				acc_threshold->set_high_threshold(json_threshold["high_threshold"].asInt());
			}
		}

		if (js.isMember("signers")) {
			const Json::Value &json_signers = js["signers"];
			for (size_t i = 0; i < json_signers.size(); i++) {
				const Json::Value &json_signer = json_signers[i];
				protocol::Signer *signer = ope_create_account->add_signers();

				if (!json_signer.isMember("address") ||
					!bubi::PublicKey::IsAddressValid(json_signer["address"].asString())) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'address' value not exist or parameter error");
					return false;
				}
				signer->set_address(json_signer["address"].asString());

				if (!json_signer.isMember("weight") ||
					!json_signer["weight"].isInt64() ||
					json_signer["weight"].asInt() < 0 ||
					json_signer["weight"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'weight' not exist or parameter error,'weight' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				signer->set_weight(json_signer["weight"].asInt());
			}
		}
		return true;
	}

	bool WebServer::PaymentOpeFrm_FromJson(protocol::OperationPayment* ope_payment, const Json::Value& js, Result &result) {

		if (!js.isMember("dest_address") ||
			!bubi::PublicKey::IsAddressValid(js["dest_address"].asString())
			) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'dest_address' value not exist or parameter error");
			return false;
		}
		ope_payment->set_destaddress(js["dest_address"].asString());

		if (!js.isMember("asset_amount") ||
			!js["asset_amount"].isInt64() ||
			js["asset_amount"].asInt64() <= 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_amount' not exist or parameter error,'asset_amount' value must be greater than 0");
			return false;
		}
		ope_payment->mutable_asset()->set_amount(js["asset_amount"].asInt64());


		protocol::AssetProperty *property = ope_payment->mutable_asset()->mutable_property();

		if (!js.isMember("asset_type") ||
			!js["asset_type"].isInt64() ||
			!protocol::AssetProperty_Type_IsValid(js["asset_type"].asInt()) ||
			js["asset_type"].asInt() == protocol::AssetProperty_Type_UNIQUE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' not exist or parameter error");
			return false;
		}
		property->set_type((protocol::AssetProperty_Type)js["asset_type"].asInt());

		std::string issuer;
		std::string asset_code;
		if (js["asset_type"].asInt() == protocol::AssetProperty_Type_IOU) {
			if (!js.isMember("asset_issuer") ||
				!bubi::PublicKey::IsAddressValid(js["asset_issuer"].asString())
				) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'asset_issuer' value not exist or parameter error");
				return false;
			}
			issuer = js["asset_issuer"].asString();

			if (!js.isMember("asset_code") ||
				js["asset_code"].asString().size() == 0 ||
				js["asset_code"].asString().size() > ASSET_CODE_MAX_SIZE) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'asset_code' not exist or parameter error,'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
				return false;
			}
			asset_code = js["asset_code"].asString();
		}
		property->set_issuer(issuer);
		property->set_code(asset_code);

		if (js.isMember("details")) {
			const Json::Value &details = js["details"];
			int64_t cont = 0;
			for (Json::UInt i = 0; i < details.size(); i++) {
				const Json::Value item = details[i];
				protocol::Detail* d = ope_payment->mutable_asset()->add_details();

				if (!item.isMember("amount") ||
					!item["amount"].isInt64() ||
					item["amount"].asInt64() <= 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'amount' not exist or parameter error,'amount' value must be greater than 0");
					return false;
				}
				d->set_amount(item["amount"].asInt64());

				if (!item.isMember("ext") ||
					item["ext"].asString().size() > 64) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'ext' not exist or parameter error,'ext' value is in the range of 0 and 64");
					return false;
				}
				d->set_ext(item["ext"].asString());

				if (!item.isMember("start") ||
					!item["start"].isInt64() ||
					item["start"].asInt64() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'start' not exist or parameter error,'start' value must be greater than or equal to  0");
					return false;
				}
				d->set_start(item["start"].asInt64());

				if (!item.isMember("length") ||
					!item["length"].isInt64() ||
					item["length"].asInt64() < -1) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'length' not exist or parameter error,'length' value must be greater than or equal to  -1");
					return false;
				}
				d->set_length(item["length"].asInt64());

				cont += item["amount"].asInt64();
			}

			if (cont != js["asset_amount"].asInt64()) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("sum of details 'amount' must be equal to 'asset_amount'");
				return false;
			}
		}
		return true;
	}

	bool WebServer::IssueOpeFrm_FromJson(protocol::OperationIssueAsset* ope_issue_asset, const Json::Value& js, Result &result) {
		if (!js.isMember("asset_amount") ||
			!js["asset_amount"].isInt64() ||
			js["asset_amount"].asInt64() <= 0
			) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_amount' not exist or parameter error,'asset_amount' value must be greater than 0");
			return false;
		}
		ope_issue_asset->mutable_asset()->set_amount(js["asset_amount"].asInt64());

		protocol::AssetProperty *property = ope_issue_asset->mutable_asset()->mutable_property();

		if (js.isMember("asset_type") &&
			(!js["asset_type"].isInt64() || js["asset_type"].asInt() != protocol::AssetProperty_Type_IOU)) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' not exist or parameter error");
			return false;
		}
		property->set_type(protocol::AssetProperty_Type_IOU);

		if (!js.isMember("asset_issuer") ||
			!bubi::PublicKey::IsAddressValid(js["asset_issuer"].asString())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_issuer' value not exist or parameter error");
			return false;
		}
		property->set_issuer(js["asset_issuer"].asString());

		if (!js.isMember("asset_code") ||
			js["asset_code"].asString().size() > ASSET_CODE_MAX_SIZE ||
			js["asset_code"].asString().size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_code' not exist or parameter error,'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
			return false;
		}
		property->set_code(js["asset_code"].asString());

		if (js.isMember("details")) {
			const Json::Value &details = js["details"];
			int64_t cont = 0;
			for (Json::UInt i = 0; i < details.size(); i++) {
				const Json::Value item = details[i];
				protocol::Detail* d = ope_issue_asset->mutable_asset()->add_details();

				if (!item.isMember("amount") ||
					!item["amount"].isInt64() ||
					item["amount"].asInt64() <= 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'amount' not exist or parameter error,'amount' value must be greater than 0");
					return false;
				}
				d->set_amount(item["amount"].asInt64());

				if (!item.isMember("ext") ||
					item["ext"].asString().size() > 64) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("details 'ext' not exist or parameter error,'ext' value is in the range of 0 and 64"));
					return false;
				}
				d->set_ext(item["ext"].asString());

				if (!item.isMember("start") ||
					!item["start"].isInt64() ||
					item["start"].asInt64() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'start' not exist or parameter error,'start' value must be greater than or equal to  0");
					return false;
				}
				d->set_start(item["start"].asInt64());

				if (!item.isMember("length") ||
					!item["length"].isInt64() ||
					item["length"].asInt64() < -1) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'length' not exist or parameter error,'length' value must be greater than or equal to  -1");
					return false;
				}
				d->set_length(item["length"].asInt64());

				cont += item["amount"].asInt64();
			}

			if (cont != js["asset_amount"].asInt64()) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("sum of details 'amount' must be equal to 'asset_amount'");
				return false;
			}
		}
		return true;
	}

	bool WebServer::IssueUniqueAssetOpeFrm_FromJson(protocol::OperationIssueUniqueAsset* ope_issue_unique_asset, const Json::Value& js, Result &result) {
		std::string detailed_bin;
		if (!js.isMember("asset_detailed") ||
			!utils::String::HexStringToBin(js["asset_detailed"].asString(), detailed_bin) ||
			detailed_bin.size() > DETAILED_MAX_SIZE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_detailed' not exist or parameter error,'asset_detailed' value is in the range of 0 and %d", DETAILED_MAX_SIZE));
			return false;
		}
		ope_issue_unique_asset->mutable_asset()->set_detailed(detailed_bin);

		protocol::AssetProperty *property = ope_issue_unique_asset->mutable_asset()->mutable_property();

		if (js.isMember("asset_type") &&
			(!js["asset_type"].isInt64() || js["asset_type"].asInt() != protocol::AssetProperty_Type_UNIQUE)) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' must be UNIQUE");
			return false;
		}
		property->set_type(protocol::AssetProperty_Type_UNIQUE);

		if (!js.isMember("asset_issuer") ||
			!bubi::PublicKey::IsAddressValid(js["asset_issuer"].asString())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_issuer' value not exist or parameter error");
			return false;
		}
		property->set_issuer(js["asset_issuer"].asString());


		if (!js.isMember("asset_code") ||
			js["asset_code"].asString().size() > ASSET_CODE_MAX_SIZE ||
			js["asset_code"].asString().size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_code' not exist or parameter error,'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
			return false;
		}
		property->set_code(js["asset_code"].asString());

		return true;
	}

	bool WebServer::PaymentUniqueAssetOpeFrm_FromJson(protocol::OperationPaymentUniqueAsset* ope_payment, const Json::Value& js, Result &result) {
		if (!js.isMember("dest_address") ||
			!bubi::PublicKey::IsAddressValid(js["dest_address"].asString())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'dest_address' value not exist or parameter error");
			return false;
		}
		ope_payment->set_destaddress(js["dest_address"].asString());

		protocol::AssetProperty *property = ope_payment->mutable_asset_pro();

		if (js.isMember("asset_type") &&
			(!js["asset_type"].isInt64() || js["asset_type"].asInt() != protocol::AssetProperty_Type_UNIQUE)) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' must be UNIQUE");
			return false;
		}
		property->set_type(protocol::AssetProperty_Type_UNIQUE);

		if (!js.isMember("asset_issuer") ||
			!bubi::PublicKey::IsAddressValid(js["asset_issuer"].asString())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_issuer' value not exist or parameter error");
			return false;
		}
		property->set_issuer(js["asset_issuer"].asString());

		if (!js.isMember("asset_code") ||
			js["asset_code"].asString().size() > ASSET_CODE_MAX_SIZE ||
			js["asset_code"].asString().size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_code' not exist or parameter error,'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
			return false;
		}
		property->set_code(js["asset_code"].asString());

		return true;
	}

	bool WebServer::SetOptionsOpeFrm_FromJson(protocol::OperationSetOptions* ope_set_opetions, const Json::Value& js, Result &result) {

		if (js.isMember("threshold")) {
			const Json::Value &json_threshold = js["threshold"];
			if (json_threshold.isMember("master_weight")) {
				if (!json_threshold["master_weight"].isInt64() ||
					json_threshold["master_weight"].asInt() < 0 ||
					json_threshold["master_weight"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'master_weight' parameter error,'master_weight' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				ope_set_opetions->set_master_weight(json_threshold["master_weight"].asInt());
			}

			if (json_threshold.isMember("low_threshold")) {
				if (!json_threshold["low_threshold"].isInt64() ||
					json_threshold["low_threshold"].asInt() < 0 ||
					json_threshold["low_threshold"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'low_threshold' parameter error,'low_threshold' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				ope_set_opetions->set_low_threshold(json_threshold["low_threshold"].asInt());
			}

			if (json_threshold.isMember("med_threshold")) {
				if (!json_threshold["med_threshold"].isInt64() ||
					json_threshold["med_threshold"].asInt() < 0 ||
					json_threshold["med_threshold"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'med_threshold' parameter error,'med_threshold' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				ope_set_opetions->set_med_threshold(json_threshold["med_threshold"].asInt());
			}

			if (json_threshold.isMember("high_threshold")) {
				if (!json_threshold["high_threshold"].isInt64() ||
					json_threshold["high_threshold"].asInt() < 0 ||
					json_threshold["high_threshold"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'high_threshold' parameter error,'high_threshold' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				ope_set_opetions->set_high_threshold(json_threshold["high_threshold"].asInt());
			}


			if (json_threshold.isMember("metadata_version")) {
				if (!json_threshold["metadata_version"].isInt64() ||
					json_threshold["metadata_version"].asInt64() < 0 ||
					!json_threshold.isMember("metadata")) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'metadata_version'  parameter error");
					return false;
				}
				ope_set_opetions->set_account_metadata_version(json_threshold["metadata_version"].asInt64());
			}

			if (json_threshold.isMember("metadata")) {
				std::string str_decode;
				if (!utils::String::HexStringToBin(json_threshold["metadata"].asString(), str_decode) ||
					str_decode.size() > METADATA_MAXSIZE) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'metadata' value must be Hex,'metadata' value is in the range of 0 and %d", (METADATA_MAXSIZE / 2)));
					return false;
				}
				ope_set_opetions->set_account_metadata(str_decode);
			}
		}

		if (js.isMember("signers")) {
			const Json::Value &json_signers = js["signers"];
			for (size_t i = 0; i < json_signers.size(); i++) {
				const Json::Value &json_signer = json_signers[i];
				protocol::Signer *signer = ope_set_opetions->add_signers();

				if (!json_signer.isMember("address") ||
					!bubi::PublicKey::IsAddressValid(json_signer["address"].asString())) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'address' value not exist or parameter error");
					return false;
				}
				signer->set_address(json_signer["address"].asString());

				if (!json_signer.isMember("weight") ||
					!json_signer["weight"].isInt64() ||
					json_signer["weight"].asInt() < 0 ||
					json_signer["weight"].asInt() > UINT8_MAX) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'weight' not exist or parameter error,'weight' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
				signer->set_weight(json_signer["weight"].asInt());
			}
		}
		return true;
	}

	bool WebServer::ProductionFrm_FromJson(protocol::OperationProduction *production, const Json::Value &js, Result &result) {
		if (js.isMember("inputs")) {
			const Json::Value &inputs = js["inputs"];
			for (Json::UInt i = 0; i < inputs.size(); i++) {
				protocol::Input *input_add = production->add_inputs();

				std::string hash_b16;
				if (!inputs[i].isMember("hash") ||
					!utils::String::HexStringToBin(inputs[i]["hash"].asString(), hash_b16) ||
					hash_b16.size() != 32) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'hash' not exist or parameter error");
					return false;
				}
				input_add->set_hash(hash_b16);

				if (!inputs[i].isMember("index") ||
					!inputs[i]["index"].isInt64() ||
					inputs[i]["index"].asInt64() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'index' not exist or parameter error,'index' value must be greater than or equal to  0");
					return false;
				}
				input_add->set_index(inputs[i]["index"].asInt64());

				if (inputs[i].isMember("metadata")) {
					std::string metadata_16;
					if (!utils::String::HexStringToBin(inputs[i]["metadata"].asString(), metadata_16) ||
						metadata_16.size() > METADATA_MAXSIZE) {
						result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
						result.set_desc(utils::String::Format("'metadata' value must be Hex,value is in the range of 0 and %d", (METADATA_MAXSIZE / 2)));
						return false;
					}
					input_add->set_metadata(metadata_16);
				}
			}
		}

		if (!js.isMember("outputs") ||
			js["outputs"].size() <= 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'outputs' not exist");
			return false;
		}

		const Json::Value &outputs = js["outputs"];
		for (Json::UInt i = 0; i < outputs.size(); i++) {
			protocol::Output *output_add = production->add_outputs();
			if (!outputs[i].isMember("address") ||
				!bubi::PublicKey::IsAddressValid(outputs[i]["address"].asString())) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'address' not exist or parameter error");
				return false;
			}
			output_add->set_address(outputs[i]["address"].asString());


			if (outputs[i].isMember("metadata")) {
				std::string metadata_16;
				if (!utils::String::HexStringToBin(outputs[i]["metadata"].asString(), metadata_16) ||
					metadata_16.size() > METADATA_MAXSIZE) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'metadata' value must be Hex");
					return false;
				}
				output_add->set_metadata(metadata_16);
			}
		}
		return true;
	}

	bool WebServer::InitPaymentOpeFrm_FromJson(protocol::OperationInitPayment *ope_init_payment, const Json::Value &js, Result &result) {
		if (!js.isMember("dest_address") ||
			!bubi::PublicKey::IsAddressValid(js["dest_address"].asString())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'dest_address' value not exist or parameter error");
			return false;
		}
		ope_init_payment->set_destaddress(js["dest_address"].asString());

		if (!js.isMember("asset_amount") ||
			!js["asset_amount"].isInt64() ||
			js["asset_amount"].asInt64() <= 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_amount' not exist or parameter error,'asset_amount' value must be greater than 0");
			return false;
		}
		ope_init_payment->mutable_asset()->set_amount(js["asset_amount"].asInt64());


		protocol::AssetProperty *property = ope_init_payment->mutable_asset()->mutable_property();
		if (js.isMember("asset_type") &&
			(!js["asset_type"].isInt64() || js["asset_type"].asInt() != protocol::AssetProperty_Type_IOU)) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' must be IOU");
			return false;
		}
		property->set_type(protocol::AssetProperty_Type_IOU);

		if (!js.isMember("asset_issuer") ||
			!bubi::PublicKey::IsAddressValid(js["asset_issuer"].asString())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_issuer' value not exist or parameter error");
			return false;
		}
		property->set_issuer(js["asset_issuer"].asString());

		if (!js.isMember("asset_code") ||
			js["asset_code"].asString().size() > ASSET_CODE_MAX_SIZE ||
			js["asset_code"].asString().size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_code' not exist or parameter error,'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
			return false;
		}
		property->set_code(js["asset_code"].asString());

		if (js.isMember("details")) {
			const Json::Value &details = js["details"];
			int64_t cont = 0;
			for (Json::UInt i = 0; i < details.size(); i++) {
				const Json::Value item = details[i];
				protocol::Detail* d = ope_init_payment->mutable_asset()->add_details();

				if (!item.isMember("amount") ||
					!item["amount"].isInt64() ||
					item["amount"].asInt64() <= 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'amount' not exist or parameter error,'amount' value must be greater than 0");
					return false;
				}
				d->set_amount(item["amount"].asInt64());

				if (!item.isMember("ext") ||
					item["ext"].asString().size() > 64) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'ext' not exist or parameter error,'ext' value is in the range of 0 and 64");
					return false;
				}
				d->set_ext(item["ext"].asString());

				if (!item.isMember("start") ||
					!item["start"].isInt64() ||
					item["start"].asInt64() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'start' not exist or parameter error,'start' value must be greater than or equal to  0");
					return false;
				}
				d->set_start(item["start"].asInt64());

				if (!item.isMember("length") ||
					!item["length"].isInt64() ||
					item["length"].asInt64() < -1) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'length' not exist or parameter error,'length' value must be greater than or equal to  -1");
					return false;
				}
				d->set_length(item["length"].asInt64());

				cont += item["amount"].asInt64();
			}

			if (cont != js["asset_amount"].asInt64()) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("sum of details 'amount' must be equal to 'asset_amount'");
				return false;
			}
		}
		return true;
	}

	bool WebServer::RecordOpeFrm_FromJson(protocol::OperationRecord *ope_record, const Json::Value &js, Result &result) {

		if (!js.isMember("record_id") ||
			js["record_id"].asString().size() > RECORD_ID_MAX_SIZE ||
			js["record_id"].asString().size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'record_id' not exist or parameter error,'record_id' value is in the range of 1 and %d", RECORD_ID_MAX_SIZE));
			return false;
		}
		ope_record->set_id(js["record_id"].asString());

		std::string meta_bin;
		if (!js.isMember("record_ext") ||
			!utils::String::HexStringToBin(js["record_ext"].asString(), meta_bin) ||
			meta_bin.size() > METADATA_MAXSIZE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'record_ext' value must be Hex");
			return false;
		}
		ope_record->set_ext(meta_bin);

		if (js.isMember("record_address")) {
			if (!bubi::PublicKey::IsAddressValid(js["record_address"].asString())) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'record_address' value not exist or parameter error");
				return false;
			}
			ope_record->set_address(js["record_address"].asString());
		}
		return true;

	}

	bool WebServer::MakeTransactionHelper(const Json::Value &object, protocol::Transaction *tran, Result& result) {
		result.set_code(protocol::ERRCODE_SUCCESS);
		do {
			if (!object.isMember("source_address") || !bubi::PublicKey::IsAddressValid(object["source_address"].asString())) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("transaction 'source_address' value not exist or parameter error");
				break;
			}
			tran->set_source_address(object["source_address"].asString());

			if (!object.isMember("fee") || !object["fee"].isInt64() || object["fee"].asInt() < 0) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'fee' not exist or parameter error,'fee' value must be greater than 0");
				break;
			}
			tran->set_fee(object["fee"].asInt());


			if (!object.isMember("sequence_number") || !object["sequence_number"].isInt64() || object["sequence_number"].asInt64() <= 0) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'sequence_number' not exist or parameter error,'sequence_number' value must be greater than 0");
				break;
			}
			tran->set_sequence_number(object["sequence_number"].asInt64());


			// Optional metadata
			if (object.isMember("metadata")) {
				std::string tmp;
				if (!utils::String::HexStringToBin(object["metadata"].asString(), tmp) ||
					tmp.size() > METADATA_MAXSIZE) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("tx 'metadata' value must be Hex,'metadata' value is in the range of 0 and %d", (METADATA_MAXSIZE / 2)));
					break;
				}
				tran->set_metadata(tmp);
			}

			// Optional time_range
			if (object.isMember("time_range")) {
				const Json::Value &timeRange = object["time_range"];
				protocol::CloseTimeRange *time_range = tran->mutable_close_time_range();

				if (!timeRange.isMember("min_time") ||
					!timeRange["min_time"].isInt64() ||
					timeRange["min_time"].asInt64() < 0
					) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'min_time' value not exist or parameter error");
					break;
				}
				time_range->set_mintime(timeRange["min_time"].asInt64());
				if (!timeRange.isMember("max_time") ||
					!timeRange["max_time"].isInt64() ||
					timeRange["max_time"].asInt64() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'max_time' value not exist or parameter error");
					break;
				}
				time_range->set_maxtime(timeRange["max_time"].asInt64());
			}

			LOG_TRACE("make new transaction: account=%s, seq_number=" FMT_I64, tran->source_address().c_str(), tran->sequence_number());

			const Json::Value &operations = object["operations"];
			for (size_t i = 0; i < operations.size(); i++) {
				protocol::Operation *ope = NULL;
				ope = tran->add_operations();

				const Json::Value &operation = operations[i];
				if (!operation.isMember("type") ||
					!operation["type"].isInt64() ||
					!protocol::Operation_Type_IsValid(operation["type"].asInt())) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'type' value not exist or parameter error");
					break;
				}
				ope->set_type((protocol::Operation_Type)operation["type"].asInt());

				//  Optional metadata
				if (operation.isMember("metadata")) {
					std::string str_bin_meta;
					if (!utils::String::HexStringToBin(operation["metadata"].asString(), str_bin_meta) ||
						str_bin_meta.size() > METADATA_MAXSIZE) {
						result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
						result.set_desc(utils::String::Format("operation 'metadata' value must be Hex,'metadata' value is in the range of 0 and %d", (METADATA_MAXSIZE / 2)));
						break;
					}
					ope->set_metadata(str_bin_meta);
				}

				// Optional source_address ,default Tx source address
				if (operation.isMember("source_address")) {
					std::string str = operation["source_address"].asString();
					if (!bubi::PublicKey::IsAddressValid(str)) {
						result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
						result.set_desc("operation 'source_address' parameter error");
						break;
					}
					ope->set_source_address(str);
				}

				switch (operation["type"].asInt()) {
				case protocol::Operation_Type_CREATE_ACCOUNT:
				{
					protocol::OperationCreateAccount *create = ope->mutable_create_account();
					CreateAccountOpeFrm_FromJson(create, operation, result);
					break;
				}
				case protocol::Operation_Type_INIT_PAYMENT:
				{
					protocol::OperationInitPayment *init_payment = ope->mutable_init_payment();
					InitPaymentOpeFrm_FromJson(init_payment, operation, result);
					break;
				}
				case protocol::Operation_Type_PAYMENT:
				{
					protocol::OperationPayment *payment = ope->mutable_payment();
					PaymentOpeFrm_FromJson(payment, operation, result);
					break;
				}
				case protocol::Operation_Type_ISSUE_ASSET:
				{
					protocol::OperationIssueAsset *issue_asset = ope->mutable_issue_asset();
					IssueOpeFrm_FromJson(issue_asset, operation, result);
					break;
				}
				case protocol::Operation_Type_ISSUE_UNIQUE_ASSET:
				{
					protocol::OperationIssueUniqueAsset *issue_unique_asset = ope->mutable_issue_unique_asset();
					IssueUniqueAssetOpeFrm_FromJson(issue_unique_asset, operation, result);
					break;
				}
				case protocol::Operation_Type_PAYMENT_UNIQUE_ASSET:
				{
					protocol::OperationPaymentUniqueAsset *payment_unique_asset = ope->mutable_payment_unique_asset();
					PaymentUniqueAssetOpeFrm_FromJson(payment_unique_asset, operation, result);
					break;
				}
				case protocol::Operation_Type_SET_OPTIONS:
				{
					protocol::OperationSetOptions *setoptions = ope->mutable_setoptions();
					SetOptionsOpeFrm_FromJson(setoptions, operation, result);
					break;
				}
				case protocol::Operation_Type_PRODUCTION:
				{
					protocol::OperationProduction *production = ope->mutable_production();
					ProductionFrm_FromJson(production, operation, result);
					break;
				}
				case protocol::Operation_Type_RECORD:
				{
					protocol::OperationRecord *record = ope->mutable_record();
					RecordOpeFrm_FromJson(record, operation, result);
					break;
				}
				default:
				{
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("");
					break;
				}
				}
			}
		} while (false);
		return result.code() == protocol::ERRCODE_SUCCESS;
	}

	void WebServer::SubmitTransaction(const http::server::request &request, std::string &reply) {

		Json::Value body;
		if (!body.fromString(request.body)) {
			LOG_ERROR("Parse request body json failed");
			Json::Value reply_json;
			reply_json["results"][Json::UInt(0)]["error_code"] = protocol::ERRCODE_INVALID_PARAMETER;
			reply_json["results"][Json::UInt(0)]["error_desc"] = "request must being json format";
			reply_json["success_count"] = Json::UInt(0);
			reply = reply_json.toStyledString();
			return;
		}

		Json::Value reply_json = Json::Value(Json::objectValue);
		Json::Value &results = reply_json["results"];
		results = Json::Value(Json::arrayValue);
		uint32_t success_count = 0;

		int64_t begin_time = utils::Timestamp::HighResolution();
		const Json::Value &json_items = body["items"];
		for (size_t j = 0; j < json_items.size() && running_; j++) {
			const Json::Value &json_item = json_items[j];
			Json::Value &result_item = results[results.size()];

			int64_t active_time = utils::Timestamp::HighResolution();
			Result result;
			result.set_code(protocol::ERRCODE_SUCCESS);
			result.set_desc("");
			do {
				protocol::TransactionEnvWrapper tran_env_wrapper;
				protocol::TransactionEnv &tran_env = *tran_env_wrapper.mutable_transaction_env();
				if (json_item.isMember("transaction_blob")) {
					if (!json_item.isMember("signatures")) {
						result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
						result.set_desc("'signatures' value not exist");
						break;
					}

					std::string decodeblob;
					std::string decodesig;
					if (!utils::String::HexStringToBin(json_item["transaction_blob"].asString(), decodeblob)) {
						result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
						result.set_desc("'transaction_blob' value must be Hex");
						break;
					}
					const Json::Value &signatures = json_item["signatures"];
					for (uint32_t i = 0; i < signatures.size(); i++) {
						const Json::Value &signa = signatures[i];
						protocol::Signature *signpro = tran_env.add_signatures();

						decodesig = "";
						if (!utils::String::HexStringToBin(signa["sign_data"].asString(), decodesig)) {
							result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
							result.set_desc("'sign_data' value must be Hex");
							break;
						}

						PublicKey pubkey(signa["public_key"].asString());
						if (!pubkey.IsValid()) {
							result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
							result.set_desc("'public_key' value not exist or parameter error");
							LOG_ERROR("invalid publickey (%s)", signa["public_key"].asString().c_str());
							break;
						}

						signpro->set_sign_data(decodesig);
						signpro->set_public_key(pubkey.GetBase58PublicKey());
					}

					protocol::Transaction *tran = tran_env.mutable_transaction();
					if (!tran->ParseFromString(decodeblob)) {
						result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
						result.set_desc("ParseFromString from 'sign_data' invalid");
						LOG_ERROR("ParseFromString from decodeblob invalid");
						break;
					}

				}
				else {
					protocol::Transaction *tran = tran_env.mutable_transaction();
					if (!WebServer::Instance().MakeTransactionHelper(json_item["transaction_json"], tran, result)) {
						break;
					}

					if (!tran->IsInitialized()) {
						result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
						result.set_desc("transaction SerializeToString failed");
						LOG_ERROR("transaction SerializeToString failed(%s)", json_item.toFastString().c_str());
						break;
					}
					std::string content = tran->SerializeAsString();
					const Json::Value &private_keys = json_item["private_keys"];
					for (uint32_t i = 0; i < private_keys.size(); i++) {
						const std::string &private_key = private_keys[i].asString();

						PrivateKey privateKey(private_key);
						if (!privateKey.IsValid()) {
							result.set_code(protocol::ERRCODE_INVALID_PRIKEY);
							result.set_desc("signature failed");
							break;
						}

						std::string sign = privateKey.Sign(content);
						protocol::Signature *signpro = tran_env.add_signatures();
						signpro->set_sign_data(sign);
						signpro->set_public_key(privateKey.GetBase58PublicKey());
					}
				}

				if (result.code() == protocol::ERRCODE_SUCCESS) {
					success_count++;

					bubi::PeerMessage  msg;
					msg.header_.type = PeerMessage::PEER_MESSAGE_TRANSACTION;

					msg.data_ = &tran_env_wrapper;
					std::string peerMessage = msg.ToString();

					std::string transStr = tran_env.transaction().SerializeAsString();
					std::string transEvnStr = tran_env.SerializeAsString();

					protocol::SlaveVerifyResponse sv_rsp;
					sv_rsp.set_peer_message(peerMessage);
					sv_rsp.set_peer_message_hash(utils::Sha256::Crypto(peerMessage));
					sv_rsp.set_transaction_hash(utils::Sha256::Crypto(transStr));
					sv_rsp.set_transaction_env_hash(utils::Sha256::Crypto(transEvnStr));

					result_item["hash"] = utils::encode_b16(sv_rsp.transaction_hash());

					for (int32_t i = 0; i < tran_env.signatures_size(); i++) {
						const protocol::Signature &signature = tran_env.signatures(i);

						if (!PublicKey::Verify(transStr, signature.sign_data(), signature.public_key())) {
							LOG_ERROR("Verify signature failed");
							continue;
						}
						bubi::PublicKey pub(signature.public_key());
						sv_rsp.add_address(pub.GetBase58Address());
					}

					std::string slaveTransMsg = sv_rsp.SerializeAsString();
#ifndef BUBI_SLAVE
					//Send MasterSlave
					bubi::MasterService::GetInstance()->Recv(ZMQ_NEW_TX, slaveTransMsg);


					if (bubi::Configure::Instance().monitor_configure_.real_time_status_) {
						//notice monitor Tx state
						std::shared_ptr<Json::Value> tx_status = std::make_shared<Json::Value>();
						(*tx_status)["type"] = 1;
						(*tx_status)["tx_hash"] = Json::Value(utils::encode_b16(sv_rsp.transaction_hash()));
						(*tx_status)["active_time"] = Json::Value(active_time);
						bubi::MonitorMaster::Instance().NoticeMonitor(tx_status->toStyledString());
					}
#else
					if (!bubi::SlaveService::GetInstance()->SendToMaster(ZMQ_NEW_TX, slaveTransMsg)) {
						LOG_ERROR("SendToMaster ZMQ_NEW_TX fail!");
					}
					if (bubi::SlaveConfigure::Instance().monitor_configure_.real_time_status_) {
						//notice monitor Tx state
						std::shared_ptr<Json::Value> tx_status = std::make_shared<Json::Value>();
						(*tx_status)["type"] = 1;
						(*tx_status)["tx_hash"] = Json::Value(utils::encode_b16(sv_rsp.transaction_hash()));
						(*tx_status)["active_time"] = Json::Value(active_time);
						bubi::MonitorMaster::Instance().NoticeMonitor(tx_status->toStyledString());
					}
#endif
					msg.data_ = NULL;
				}
			} while (false);
			result_item["error_code"] = result.code();
			result_item["error_desc"] = result.desc();
		}
		LOG_TRACE("Create %u transaction use " FMT_I64 "(ms)", json_items.size(),
			(utils::Timestamp::HighResolution() - begin_time) / utils::MICRO_UNITS_PER_MILLI);


		reply_json["success_count"] = success_count;
		reply = reply_json.toStyledString();
	}

	void WebServer::AssetToJson(const protocol::Asset &asset, Json::Value &js) {
		js["asset_amount"] = asset.amount();
		js["asset_issuer"] = asset.property().issuer();
		js["asset_code"] = asset.property().code();
		js["asset_type"] = asset.property().type();
		Json::Value &js_detail = js["details"];
		for (int i = 0; i < asset.details_size(); i++) {
			Json::Value detailitem;
			detailitem["amount"] = asset.details(i).amount();
			detailitem["start"] = asset.details(i).start();
			detailitem["length"] = asset.details(i).length();
			detailitem["ext"] = asset.details(i).ext();
			js_detail[i] = detailitem;
		}
	}

	void WebServer::GetModuleStatus(Json::Value &data) {
		data["name"] = "web_server";
		data["context"] = (context_ != NULL);
		data["thread_count"] = thread_count_;
	}
}