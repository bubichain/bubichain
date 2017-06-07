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

#ifndef CONFIGURE_H_
#define CONFIGURE_H_

#include <json/json.h>
#include <utils/singleton.h>
#include <utils/strings.h>
#include <utils/net.h>

namespace bubi {
	class DbConfigure {
	public:
		DbConfigure();
		~DbConfigure();
		bool Load(const Json::Value &value);
	public:
		std::string keyvalue_db_path_;
		std::string rational_db_path_;
		std::string rational_db_type_;
		std::string tmp_path_;
		std::string soci_dbname_;
		std::string soci_user_;
		std::string soci_password_;
		std::string soci_connection_;
		bool async_write_sql_;
		bool async_write_kv_;
	};

	class SSLConfigure {
	public:
		SSLConfigure();
		~SSLConfigure();
		bool Load(const Json::Value &value);
	public:
		std::string chain_file_;
		std::string private_key_file_;
		std::string dhparam_file_;
		std::string verify_file_;
	};

	class P2pNetwork {
	public:
		enum P2pType { CONSENSUS, TRANSACTION };
		P2pNetwork(P2pType type);
		~P2pNetwork();
		bool Load(const Json::Value &value);
	public:
		size_t target_peer_connection_;
		int64_t connect_timeout_;
		int64_t heartbeat_interval_;
		int32_t listen_port_;
		utils::StringList known_peer_list_;
	};

	class P2pConfigure {
	public:
		P2pConfigure();
		~P2pConfigure();
		bool Load(const Json::Value &value);
	public:
		std::string node_private_key_;
		SSLConfigure ssl_configure_;
		P2pNetwork consensus_network_configure_;
		P2pNetwork transaction_network_configure_;
	};

	class LoggerConfigure {
	public:
		LoggerConfigure();
		~LoggerConfigure();
		bool Load(const Json::Value &value);
	public:
		std::string path_;
		std::string dest_str_;
		std::string level_str_;
		int32_t time_capacity_;
		int64_t size_capacity_;
		uint32_t dest_;
		uint32_t level_;
		int32_t expire_days_;
	};

	class PipelineConfigure {
	public:
		enum PipelineConfigType {
			BUBI_SLAVE_T,
			BUBI_MONITOR_T,
			BUBI_MQSERVER_T,
			MONITOR_BUBI_T,
			MONITOR_SLAVE_T,
			SLAVE_BUBI_T,
			SLAVE_MONITOR_T
		};
		PipelineConfigure(PipelineConfigType type);
		~PipelineConfigure();
		bool Load(const Json::Value &value);
	public:
		std::string recv_address_;
		std::string send_address_;
		uint32_t workers_count_;
		uint32_t hash_type_;
	private:
		PipelineConfigType type_;
	};

	class MqServerConfigure {
	public:
		MqServerConfigure();
		~MqServerConfigure();
		bool Load(const Json::Value &value);
	public:
		PipelineConfigure pipeline_configure_;
		bool tx_status_;	
	};
	
	class WsServerConfigure {
	public:
		WsServerConfigure();
		~WsServerConfigure();

		utils::InetAddress listen_address_;
		bool listen_tx_status_;

		bool Load(const Json::Value &value);
	};
	
	class WebServerConfigure {
	public:
		WebServerConfigure();
		~WebServerConfigure();
		bool Load(const Json::Value &value);
	public:
		utils::InetAddressList listen_addresses_;
		SSLConfigure ssl_configure_;
		bool ssl_enable_;
		uint32_t query_limit_;
		uint32_t multiquery_limit_;
		bool remote_authorized_;
        uint32_t thread_count_;
	};

	class LedgerConfigure {
	public:
		LedgerConfigure();
		~LedgerConfigure();
		bool Load(const Json::Value &value);
	public:
		uint32_t base_fee_;
		uint32_t base_reserve_;
		uint32_t hash_type_;
		uint32_t max_trans_per_ledger_;
		uint32_t max_ledger_per_message_;
		uint32_t max_trans_in_memory_;
		uint32_t max_apply_ledger_per_round_;
		bool test_model_;
		std::string genesis_account_;
	};

	class ValidationConfigure {
	public:
		ValidationConfigure();
		~ValidationConfigure();
		bool Load(const Json::Value &value);
	public:
		std::string type_;
		bool is_validator_;
		std::string node_privatekey_;
		utils::StringList validators_;
		int32_t threshold_;
		int64_t close_interval_;
	};

	class SystemConfig {
	public:
		SystemConfig();
		~SystemConfig();
		bool Load(const Json::Value &value);
	public:
		//the number of thread when multi-thread used
		std::string local_address_;
		uint32_t thread_count_;
		uint32_t delay_consensus_;
	};

	class WebSocketClientConfigure {
	public:
		WebSocketClientConfigure();
		~WebSocketClientConfigure();
		bool Load(const Json::Value &value);
	public:
		std::string monitor_id_;
		std::string connect_address_;
		std::string bubi_config_path_;
		std::string chain_file_;
		std::string private_key_file_;
		std::string dhparam_file_;
		bool ssl_enable_;
		utils::InetAddressList listen_addresses_;
	};

	class CMonitorConfigure {
	public:
		CMonitorConfigure(PipelineConfigure::PipelineConfigType type);
		~CMonitorConfigure();
		bool Load(const Json::Value &value);
	public:
		PipelineConfigure pipeline_configure_;
		bool real_time_status_;
	};

	class Configure : public utils::Singleton<Configure> {
		friend class utils::Singleton<Configure>;
	private:
		Configure();
		~Configure();
	public:
		bool Load(const std::string &config_file_path);
		static void GetValue(const Json::Value &object, const std::string &key, std::string &value);
		static void GetValue(const Json::Value &object, const std::string &key, int32_t &value);
		static void GetValue(const Json::Value &object, const std::string &key, uint32_t &value);
		static void GetValue(const Json::Value &object, const std::string &key, int64_t &value);
		static void GetValue(const Json::Value &object, const std::string &key, utils::StringList &list);
		static void GetValue(const Json::Value &object, const std::string &key, bool &value);
	public:
		DbConfigure db_configure_;
		SystemConfig system_config_;
		LoggerConfigure logger_configure_;
		P2pConfigure p2p_configure_;
		WebServerConfigure webserver_configure_;
		LedgerConfigure ledger_configure_;
		ValidationConfigure validation_configure_;
		PipelineConfigure slave_configure_;
		CMonitorConfigure monitor_configure_;
		MqServerConfigure mqserver_configure_;
        WsServerConfigure wsserver_configure_;
	};

	class SlaveConfigure : public utils::Singleton<SlaveConfigure> {
		friend class utils::Singleton<SlaveConfigure>;
	private:
		SlaveConfigure();
		~SlaveConfigure();
	public:
		bool Load(const std::string &config_file_path);
	public:
		LoggerConfigure logger_configure_;
		WebServerConfigure webserver_configure_;
		PipelineConfigure bubi_configure_;
		CMonitorConfigure monitor_configure_;
	};

	class MonitorConfigure : public utils::Singleton < MonitorConfigure > {
		friend class utils::Singleton < MonitorConfigure >;
	private:
		MonitorConfigure();
		~MonitorConfigure();
		bool ReadConfigure(const std::string &file_path, Json::Value &object);
	public:
		bool Load(const std::string &config_file_path);
		bool GetConfigure(Json::Value &object);
		bool SetConfigure(std::string &config);
	public:
		LoggerConfigure logger_configure_;
		PipelineConfigure slave_pipeline_configure_;
		PipelineConfigure bubi_pipeline_configure_;
		WebSocketClientConfigure web_socket_client_configure_;
	private:
		std::string mutable_config_file_path_;
		std::string immutable_config_file_path_;
	};
}

#endif