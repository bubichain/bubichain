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
#ifndef GENERAL_H_
#define GENERAL_H_

#include <asio.hpp>
#include <utils/common.h>
#include <utils/timer.h>
#include <utils/thread.h>
#include <utils/singleton.h>
#include <utils/crypto.h>
#include <utils/noncopyable.h>
#include <json/value.h>

namespace bubi {
	class General {
	public:
		const static uint32_t OVERLAY_VERSION;
		const static uint32_t OVERLAY_MIN_VERSION;
		const static uint32_t LEDGER_VERSION;
		const static uint32_t LEDGER_MIN_VERSION;
		const static char *BUBI_VERSION;
		const static char *CONSENSUS_NET_MAGICWORD;
		const static char *TRANSACTION_NET_MAGICWORD;
		const static int CONSENSUS_PORT = 9333;
		const static int TRANSACTION_PORT = 9334;
		const static int WEBSERVER_PORT = 19333;
		const static int BUBI_SLAVE_PORT1 = 3053;
		const static int BUBI_SLAVE_PORT2 = 3054;
		const static int BUBI_MONITOR_PORT1 = 4053;
		const static int BUBI_MONITOR_PORT2 = 4054;
		const static int SLAVE_MONITOR_PORT1 = 5053;
		const static int SLAVE_MONITOR_PORT2 = 5054;
		const static int MQSERVER_PORT1 = 6053;
		const static int MQSERVER_PORT2 = 6054;
		const static char *DEFAULT_KEYVALUE_DB_PATH;
		const static char *DEFAULT_RATIONAL_DB_PATH;
		const static char *PEERS_TABLE_NAME[2];
		const static char *PEERS_CREATE_SQL[2];
		const static char *PEERS_C_TABLE_NAME;
		const static char *PEERS_C_CREATE_SQL;
		const static char *PEERS_T_TABLE_NAME;
		const static char *PEERS_T_CREATE_SQL;
		const static char *LEDGER_TABLE_NAME;
		const static char *LEDGER_CREATE_SQL;
		const static char *TX_TABLE_NAME;
		const static char *TX_CREATE_SQL;
		const static char *ACCOUNT_TX_NAME;
		const static char *ACCOUNT_TX_CREATE_SQL;
		const static char *UNIQUE_ASSET_NAME;
		const static char *UNIQUE_ASSET_CREATE_SQL;
		const static char *RECORD_TABLE_NAME;
		const static char *RECORD_SQL;
		const static char *ORDER_NAME;
		const static char *ORDER_CREATE_SQL;
		const static char *TABLE_GLOBAL;
		const static char *TABLE_GLOBAL_CREATE_SQL;
		const static char *TABLE_LEDGER_BUFFER;
		const static char *TABLE_LEDGER_BUFFER_CREATE_SQL;
		const static char *CONFIG_FILE;
		const static char *SLAVE_CONFIG_FILE;
		const static char *MONITOR_CONFIG_FILE;
		const static char *LOGGER_FILE;
		const static char *CONSENSUS_PREFIX;
		const static char *TXSET_PREFIX;
		const static char *OVERLAY_PREFIX;
		const static char *RATIONAL_TMPDB_POSTFIX;
		const static char *KEYVALUE_TMPDB_POSTFIX;
		const static int ACCOUNT_LENGTH_MAX = 40;
		const static char *KEY_LEDGER_SEQ;
		typedef enum WARNINGCODE_ {
			WARNING,
			NOWARNING
		} WARNINGCODE;
		volatile static long tx_new_count;
		volatile static long tx_delete_count;
		volatile static long txset_new_count;
		volatile static long txset_delete_count;
		volatile static long peermsg_new_count;
		volatile static long peermsg_delete_count;
		volatile static long account_new_count;
		volatile static long account_delete_count;
		volatile static long trans_low_new_count;
		volatile static long trans_low_delete_count;
	};

	class Result {
	public:
		Result();
		~Result();
		int32_t code() const;
		std::string desc() const;
		void set_code(int32_t code);
		void set_desc(const std::string desc);
		bool operator=(const Result &result);
	public:
		int64_t ledger_seq_;
		int64_t close_time_;
	private:
		int32_t code_;
		std::string desc_;
	};

	class TimerNotify {
	protected:
		int64_t last_check_time_;
		int64_t last_slow_check_time_;
		int64_t check_interval_;
		int64_t last_execute_complete_time_;
		int64_t last_slow_execute_complete_time_;
		std::string timer_name_;
	public:
		static std::list<TimerNotify *> notifys_;
	public:
		static bool RegisterModule(TimerNotify *module);
		TimerNotify();
		~TimerNotify();
		void TimerWrapper(int64_t current_time);
		void SlowTimerWrapper(int64_t current_time);
		bool IsSlowExpire(int64_t time_out);
		bool IsExpire(int64_t time_out);
		int64_t GetSlowLastExecuteTime();
		int64_t GetLastExecuteTime();
		std::string GetTimerName();
		virtual void OnTimer(int64_t current_time) = 0;
		virtual void OnSlowTimer(int64_t current_time) = 0;
	};

	class StatusModule {
	public:
		StatusModule();
		~StatusModule();
		virtual void GetModuleStatus(Json::Value &nData) = 0;
		static bool RegisterModule(StatusModule *module);
		static void GetModulesStatus(Json::Value &nData);
	public:
		static std::list<StatusModule *> modules_;
		static Json::Value *modules_status_;
		static utils::ReadWriteLock status_lock_;
	};

	class SlowTimer : public utils::Singleton<bubi::SlowTimer>, public utils::Runnable {
	public:
		SlowTimer();
		~SlowTimer();
		bool Initialize(size_t thread_count);
		bool Exit();
		virtual void Run(utils::Thread *thread) override;
		void Stop();
	public:
		asio::io_service io_service_;
		std::vector<utils::Thread *> thread_ptrs_;
	};

	class Global : public utils::Singleton<bubi::Global>, public TimerNotify {
	public:
		Global();
		~Global();
		bool Initialize();
		bool Exit();
		virtual void OnTimer(int64_t current_time) override;
		virtual void OnSlowTimer(int64_t current_time) override {};
		asio::io_service &GetIoService();
		int64_t GetMainThreadId();
	private:
		asio::io_service io_service_;
		asio::io_service::work work_;
		int64_t main_thread_id_;
	};

#define  ASSERT_MAIN_THREAD assert(utils::Thread::current_thread_id() == Global::Instance().GetMainThreadId());

	class HashWrapper : public utils::NonCopyable {
	public:
		enum HashType {
			HASH_TYPE_SHA256 = 0,
			HASH_TYPE_SM3 = 1,
			HASH_TYPE_MAX = 2
		};
		HashWrapper();
		HashWrapper(int32_t type);
		~HashWrapper();
		void Update(const std::string &input);
		void Update(const void *buffer, size_t len);
		std::string Final();
		static void SetLedgerHashType(int32_t type);
		static int32_t GetLedgerHashType();
		static std::string Crypto(const std::string &input);
		static void Crypto(unsigned char* str, int len, unsigned char *buf);
		static void Crypto(const std::string &input, std::string &str);
	private:
		int32_t type_;// 0 : protocol::LedgerUpgrade::SHA256, 1: protocol::LedgerUpgrade::SM3
		utils::Hash *hash_;
	};
}

#endif
