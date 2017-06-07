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

#include <utils/logger.h>
#include "general.h"
#include "utils/strings.h"
#include "proto/message.pb.h"

namespace bubi {
	const uint32_t General::OVERLAY_VERSION = 1000;
	const uint32_t General::OVERLAY_MIN_VERSION = 1000;
	const uint32_t General::LEDGER_VERSION = 2000;
	const uint32_t General::LEDGER_MIN_VERSION = 1000;
	const char *General::BUBI_VERSION = "2.0.0.0";
	const char *General::CONSENSUS_NET_MAGICWORD = "consensus network";
	const char *General::TRANSACTION_NET_MAGICWORD = "transaction network";

#ifdef WIN32
	const char *General::DEFAULT_KEYVALUE_DB_PATH = "data/keyvalue.db";
	const char *General::DEFAULT_RATIONAL_DB_PATH = "data/rational.db";
	const char *General::CONFIG_FILE = "config/bubi.json";
	const char *General::SLAVE_CONFIG_FILE = "config/slave.json";
	const char *General::MONITOR_CONFIG_FILE = "config/monitor.json";
	const char *General::LOGGER_FILE = "log/bubi.log";

#else
	const char *General::DEFAULT_KEYVALUE_DB_PATH = "data/keyvalue.db";
	const char *General::DEFAULT_RATIONAL_DB_PATH = "bubidata/rational.db";
	const char *General::CONFIG_FILE = "config/bubi.json";
	const char *General::SLAVE_CONFIG_FILE = "config/slave.json";
	const char *General::MONITOR_CONFIG_FILE = "config/monitor.json";
	const char *General::LOGGER_FILE = "log/bubi.log";
#endif

	const char *General::PEERS_C_TABLE_NAME = "peers_c";
	const char *General::PEERS_C_CREATE_SQL =
		"CREATE TABLE peers_c("
		"ip VARCHAR(64),"
		"port INT, "
		"rank INT,"
		"num_failures INT,"
		"next_attempt_time BIGINT,"
		"active_time BIGINT"
		");"
		;

	const char *General::PEERS_T_TABLE_NAME = "peers_t";
	const char *General::PEERS_T_CREATE_SQL =
		"CREATE TABLE peers_t("
		"ip VARCHAR(64),"
		"port INT, "
		"rank INT,"
		"num_failures INT,"
		"next_attempt_time BIGINT,"
		"active_time BIGINT"
		");"
		;

	const char* General::PEERS_TABLE_NAME[2] =
	{
		General::PEERS_C_TABLE_NAME,
		General::PEERS_T_TABLE_NAME
	};

	const char* General::PEERS_CREATE_SQL[2] =
	{
		General::PEERS_C_CREATE_SQL,
		General::PEERS_T_CREATE_SQL
	};

	const char *General::LEDGER_TABLE_NAME = "ledger";
	const char *General::LEDGER_CREATE_SQL =
		"CREATE TABLE ledger("
		"ledger_seq	      INT  NOT NULL,"
		"hash			  VARCHAR(70)   NOT NULL DEFAULT '', "
		"phash			  VARCHAR(70)   NOT NULL DEFAULT '',"
		"txhash			  VARCHAR(70)   NOT NULL DEFAULT '',"
		"account_hash	  VARCHAR(70)   NOT NULL DEFAULT '',"
		"total_coins	  BIGINT        NOT NULL DEFAULT 0,"
		"close_time		  BIGINT        NOT NULL DEFAULT 0,"
		"consensus_value  VARCHAR(1024) NOT NULL DEFAULT '',"
		"base_fee		  INT           NOT NULL DEFAULT 0,"
		"base_reserve	  INT           NOT NULL DEFAULT 0,"
		"ledger_version	  INT           NOT NULL DEFAULT 1,"
		"tx_count         BIGINT        NOT NULL DEFAULT 1,"
		"state            INT           NOT NULL DEFAULT 0,"
		"PRIMARY KEY   (ledger_seq)                        "
		");"
		;


	const char *General::TX_TABLE_NAME = "transaction_history";
	const char *General::TX_CREATE_SQL =
		"CREATE TABLE transaction_history("
		"hash			 VARCHAR(64)  NOT NULL DEFAULT '', "
		"from_account	 VARCHAR(64)  NOT NULL DEFAULT '', "
		"ledger_seq		 BIGINT       NOT NULL DEFAULT 0,  "
		"seq			 BIGINT       NOT NULL DEFAULT 0,  "
		"body			 TEXT         NOT NULL , "
		"result			 TEXT		  NOT NULL , "
		"error_code		 INT          NOT NULL DEFAULT 0,  "
		"seq_in_global   BIGINT       NOT NULL DEFAULT 0,  "
		"apply_time      BIGINT       NOT NULL DEFAULT 0,   "
		"PRIMARY KEY   (hash)"
		");"
		"CREATE INDEX  index_ledger_seq ON transaction_history(ledger_seq);"
		"CREATE INDEX  index_seq_in_global ON transaction_history(seq_in_global);"
		;

	const char *General::ACCOUNT_TX_NAME = "account_transactions";
	const char *General::ACCOUNT_TX_CREATE_SQL =
		"CREATE TABLE account_transactions ( "
		"trans_id	   VARCHAR(64) NOT NULL DEFAULT '',"
		"account       VARCHAR(64) NOT NULL DEFAULT '',"
		"ledger_seq    BIGINT NOT NULL DEFAULT 0,      "
		"txn_seq       BIGINT NOT NULL DEFAULT 0,      "
		"apply_time    BIGINT NOT NULL DEFAULT 0,      "
		"seq_in_global BIGINT NOT NULL DEFAULT 0 ,     "
		"PRIMARY KEY   (account,trans_id)				"
		");                                            "
		;


	const char *General::UNIQUE_ASSET_NAME = "table_unique_asset";
	const char *General::UNIQUE_ASSET_CREATE_SQL =
		"CREATE TABLE table_unique_asset ( "
		"id				 VARCHAR(256) NOT NULL DEFAULT '', "
		"ledger_seq		 BIGINT       NOT NULL DEFAULT 0,  "
		"tx_hash		 VARCHAR(64)  NOT NULL DEFAULT '', "
		"asset_issuer	 VARCHAR(64)  NOT NULL DEFAULT '', "
		"asset_code		 VARCHAR(64)  NOT NULL DEFAULT '', "
		"asset_detailed  TEXT	  NOT NULL, "
		"from_address	 VARCHAR(64)  NOT NULL DEFAULT '', "
		"to_address		 VARCHAR(64)  NOT NULL DEFAULT '', "
		"PRIMARY KEY   (id)                                "
		");"
		;

	const char *General::RECORD_TABLE_NAME = "table_record";
	const char *General::RECORD_SQL =
		"CREATE TABLE table_record ( "
		"id						VARCHAR(256) NOT NULL DEFAULT '', "
		"ledger_seq				BIGINT       NOT NULL DEFAULT 0, "
		"tx_hash				VARCHAR(64)  NOT NULL DEFAULT '', "
		"record_participant		VARCHAR(64)  NOT NULL DEFAULT '', "
		"record_address			VARCHAR(64)  NOT NULL DEFAULT '', "
		"record_id				VARCHAR(64)  NOT NULL DEFAULT '', "
		"record_ext				TEXT		 NOT NULL           , "
		"PRIMARY KEY   (id)                                       "
		");"
		;

	const char* General::ORDER_NAME = "table_order";
	const char* General::ORDER_CREATE_SQL =
		"CREATE TABLE table_order( "
		"id                   BIGINT          NOT NULL DEFAULT  (0 ), "
		"seller               VARCHAR( 64 )   NOT NULL DEFAULT (''),  "
		"selling_asset_type   INT             NOT NULL DEFAULT (0 ),  "
		"selling_asset_issuer VARCHAR( 64 )   NOT NULL DEFAULT (''),  "
		"selling_asset_code   VARCHAR( 64 )   NOT NULL DEFAULT (''),  "
		"selling_amount       BIGINT          NOT NULL DEFAULT (0 ),  "
		"buying_asset_type    INT             NOT NULL DEFAULT (0 ),  "
		"buying_asset_issuer  VARCHAR( 64 )   NOT NULL DEFAULT (''),  "
		"buying_asset_code    VARCHAR( 64 )   NOT NULL DEFAULT (''),  "
		"buying_amount        BIGINT          NOT NULL DEFAULT (0 )   "
		");"
		;


	const char *General::TABLE_GLOBAL = "global_data";
	const char *General::TABLE_GLOBAL_CREATE_SQL =
		"CREATE TABLE global_data("
		"name	      VARCHAR(255)      NOT NULL DEFAULT '',"
		"value		  TEXT              NOT NULL,"
		"PRIMARY KEY(name)                                 "
		");";


	const char *General::TABLE_LEDGER_BUFFER = "ledger_buffer";
	const char *General::TABLE_LEDGER_BUFFER_CREATE_SQL =
		"CREATE TABLE ledger_buffer("
		"seq	BIGINT		NOT NULL DEFAULT 0,"
		"value	TEXT	NOT NULL,"
		"PRIMARY KEY(seq)                                 "
		");";
	volatile long General::tx_new_count = 0;
	volatile long General::tx_delete_count = 0;
	volatile long General::txset_new_count = 0;
	volatile long General::txset_delete_count = 0;
	volatile long General::peermsg_new_count = 0;
	volatile long General::peermsg_delete_count = 0;
	volatile long General::account_new_count = 0;
	volatile long General::account_delete_count = 0;
	volatile long General::trans_low_new_count = 0;
	volatile long General::trans_low_delete_count = 0;

	const char *General::KEY_LEDGER_SEQ = "max_seq";

	const char *General::CONSENSUS_PREFIX = "consensus";
	const char *General::TXSET_PREFIX = "txset";
	const char *General::OVERLAY_PREFIX = "overlay";
	const char *General::RATIONAL_TMPDB_POSTFIX = ".tmpdb";
	const char *General::KEYVALUE_TMPDB_POSTFIX = ".tmpkv";

	TimerNotify::TimerNotify(){
		last_check_time_ = 0;
		last_slow_check_time_ = 0;
		check_interval_ = 0;
	}

	TimerNotify::~TimerNotify(){
	}

	bool TimerNotify::RegisterModule(TimerNotify *module) {
		notifys_.push_back(module); return true; 
	}

	void TimerNotify::TimerWrapper(int64_t current_time) {
		last_execute_complete_time_ = 0; //clear first
		if (current_time > last_check_time_ + check_interval_) {
			last_check_time_ = current_time;
			OnTimer(current_time);
			last_execute_complete_time_ = utils::Timestamp::HighResolution();
		}
	}

	void TimerNotify::SlowTimerWrapper(int64_t current_time) {
		last_slow_execute_complete_time_ = 0;//clear first
		if (current_time > last_slow_check_time_ + check_interval_) {
			last_slow_check_time_ = current_time;
			OnSlowTimer(current_time);
			last_slow_execute_complete_time_ = utils::Timestamp::HighResolution();
		}
	}

	bool TimerNotify::IsSlowExpire(int64_t time_out) {
		return last_slow_execute_complete_time_ - last_slow_check_time_ > time_out;
	}

	bool TimerNotify::IsExpire(int64_t time_out) {
		return last_execute_complete_time_ - last_check_time_ > time_out;
	}

	int64_t TimerNotify::GetSlowLastExecuteTime() {
		return last_slow_execute_complete_time_ - last_slow_check_time_;
	}

	int64_t TimerNotify::GetLastExecuteTime() {
		return last_execute_complete_time_ - last_check_time_;
	}

	std::string TimerNotify::GetTimerName() {
		return timer_name_;
	}

	StatusModule::StatusModule() {
	}

	StatusModule::~StatusModule() {
	}

	bool StatusModule::RegisterModule(StatusModule *module) {
		modules_.push_back(module);
		return true; 
	}

	Result::Result() {
		code_ = protocol::ERRCODE_SUCCESS;
	}

	Result::~Result() {};

	int32_t Result::code() const {
		return code_;
	}

	std::string Result::desc() const {
		return desc_;
	}

	void Result::set_code(int32_t code) {
		code_ = code;
	}

	void Result::set_desc(const std::string desc) {
		desc_ = desc;
	}

	bool Result::operator=(const Result &result) {
		code_ = result.code();
		desc_ = result.desc();
		return true;
	}

	std::list<StatusModule *> StatusModule::modules_;
	Json::Value *StatusModule::modules_status_ = NULL;
	utils::ReadWriteLock StatusModule::status_lock_;

	void StatusModule::GetModulesStatus(Json::Value &nData) {
		for (auto &item : modules_) {
			Json::Value json_item = Json::Value(Json::objectValue);
			int64_t begin_time = utils::Timestamp::HighResolution();
			item->GetModuleStatus(json_item);
			json_item["time"] = utils::String::Format(FMT_I64 " ms", (utils::Timestamp::HighResolution() - begin_time) / utils::MICRO_UNITS_PER_MILLI);
			std::string key = json_item["name"].asString();
			json_item.removeMember("name");
			nData[key] = json_item;
		}
	}

	std::list<TimerNotify *> TimerNotify::notifys_;

	SlowTimer::SlowTimer() {}

	SlowTimer::~SlowTimer() {}

	bool SlowTimer::Initialize(size_t thread_count) {
		for (size_t i = 0; i < thread_count; i++) {
			utils::Thread *thread_p = new utils::Thread(this);
			if (!thread_p->Start(utils::String::Format("slowtimer-%d", i))) {
				return false;
			}

			thread_ptrs_.push_back(thread_p);
		}

		return true;
	}

	bool SlowTimer::Exit() {
		LOG_INFO("SlowTimer stoping...");
		io_service_.stop();
		for (size_t i = 0; i < thread_ptrs_.size(); i++) {
			utils::Thread *thread_p = thread_ptrs_[i];
			if (thread_p) {
				thread_p->JoinWithStop();
				delete thread_p;
				thread_p = NULL;
			}
		}
		LOG_INFO("SlowTimer stop [OK]");
		return true;
	}

	void SlowTimer::Run(utils::Thread *thread) {
		asio::io_service::work work(io_service_);
		while (!io_service_.stopped()) {
			asio::error_code err;
			io_service_.poll(err);

			for (auto item : TimerNotify::notifys_) {
				item->SlowTimerWrapper(utils::Timestamp::HighResolution());

				if (item->IsSlowExpire(5 * utils::MICRO_UNITS_PER_SEC)) {
					LOG_WARN("The timer(%s) execute time(" FMT_I64 " us) is expire than 5s", item->GetTimerName().c_str(), item->GetSlowLastExecuteTime());
				}
			}

			utils::Sleep(1);
		}
	}

	Global::Global() : work_(io_service_), main_thread_id_(0) {}

	Global::~Global() {}

	bool Global::Initialize() {
		timer_name_ = "Global";
		main_thread_id_ = utils::Thread::current_thread_id();
		TimerNotify::RegisterModule(this);
		return true;
	}

	bool Global::Exit() {
		LOG_INFO("Global stoping...");
		LOG_INFO("Global stop [OK]");
		return true;
	}

	void Global::OnTimer(int64_t current_time) {
		//clock_.crank(false);
		asio::error_code err;
		io_service_.poll(err);
	}

	asio::io_service &Global::GetIoService() {
		return io_service_;
	}

	int64_t Global::GetMainThreadId() {
		return main_thread_id_;
	}

	static int32_t ledger_type_ = HashWrapper::HASH_TYPE_SHA256;
	HashWrapper::HashWrapper() {
		type_ = ledger_type_;
		if (type_ == HASH_TYPE_SM3) {
			hash_ = new utils::Sm3();
		}
		else {
			hash_ = new utils::Sha256();
		}
	}

	HashWrapper::HashWrapper(int32_t type) {
		type_ = type;
		if (type_ == HASH_TYPE_SM3) {
			hash_ = new utils::Sm3();
		}
		else {
			hash_ = new utils::Sha256();
		}
	}

	HashWrapper::~HashWrapper() {
		if (hash_) {
			delete hash_;
		}
	}

	void HashWrapper::Update(const std::string &input) {
		hash_->Update(input);
	}

	void HashWrapper::Update(const void *buffer, size_t len) {
		hash_->Update(buffer, len);
	}

	std::string HashWrapper::Final() {
		return hash_->Final();
	}

	void HashWrapper::SetLedgerHashType(int32_t type_) {
		ledger_type_ = type_;
	}

	int32_t HashWrapper::GetLedgerHashType() {
		return ledger_type_;
	}

	std::string HashWrapper::Crypto(const std::string &input) {
		if (ledger_type_ == HASH_TYPE_SM3) {
			return utils::Sm3::Crypto(input);
		}
		else {
			return utils::Sha256::Crypto(input);
		}
	}

	void HashWrapper::Crypto(unsigned char* str, int len, unsigned char *buf) {
		if (ledger_type_ == HASH_TYPE_SM3) {
			utils::Sm3::Crypto(str, len, buf);
		}
		else {
			utils::Sha256::Crypto(str, len, buf);
		}
	}

	void HashWrapper::Crypto(const std::string &input, std::string &str) {
		if (ledger_type_ == HASH_TYPE_SM3) {
			utils::Sm3::Crypto(input, str);
		}
		else {
			utils::Sha256::Crypto(input, str);
		}
	}
}
