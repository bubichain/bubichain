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

#include <utils/utils.h>
#include <utils/file.h>
#include <utils/strings.h>
#include <utils/logger.h>
#include "configure.h"
#include "general.h"

namespace bubi {
	void Configure::GetValue(const Json::Value &object, const std::string &key, std::string &value) {
		if (object.isMember(key)) {
			value = object[key].asString();
		}
	}

	void Configure::GetValue(const Json::Value &object, const std::string &key, uint32_t &value) {
		if (object.isMember(key)) {
			value = object[key].asUInt();
		}
	}

	void Configure::GetValue(const Json::Value &object, const std::string &key, int32_t &value) {
		if (object.isMember(key)) {
			value = object[key].asInt();
		}
	}

	void Configure::GetValue(const Json::Value &object, const std::string &key, int64_t &value) {
		if (object.isMember(key)) {
			value = object[key].asInt64();
		}
	}

	void Configure::GetValue(const Json::Value &object, const std::string &key, utils::StringList &list) {
		if (object.isMember(key)) {
			const Json::Value &array_value = object[key];
			for (size_t i = 0; i < array_value.size(); i++) {
				list.push_back(array_value[i].asString());
			}
		}
	}

	void Configure::GetValue(const Json::Value &object, const std::string &key, bool &value) {
		if (object.isMember(key)) {
			value = object[key].asBool();
		}
	}

	bubi::DbConfigure::DbConfigure() {
		keyvalue_db_path_ = bubi::General::DEFAULT_KEYVALUE_DB_PATH;
		rational_db_path_ = bubi::General::DEFAULT_RATIONAL_DB_PATH;
		tmp_path_ = "tmp";
		async_write_sql_ = false; //default sync write sql
		async_write_kv_ = false; //default sync write kv
	}

	bubi::DbConfigure::~DbConfigure() {}

	bool bubi::DbConfigure::Load(const Json::Value &value) {
		Configure::GetValue(value, "keyvalue_path", keyvalue_db_path_);
		Configure::GetValue(value, "rational_path", rational_db_path_);
		Configure::GetValue(value, "rational_string", soci_connection_);
		Configure::GetValue(value, "rational_db_type", rational_db_type_);
		Configure::GetValue(value, "tmp_path", tmp_path_);
		Configure::GetValue(value, "async_write_sql", async_write_sql_);
		Configure::GetValue(value, "async_write_kv", async_write_kv_);

		if (!utils::File::IsAbsolute(keyvalue_db_path_)) {
			keyvalue_db_path_ = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), keyvalue_db_path_.c_str());
		}
		if (!utils::File::IsAbsolute(rational_db_path_)) {
			rational_db_path_ = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), rational_db_path_.c_str());
		}
		if (!utils::File::IsAbsolute(tmp_path_)) {
			tmp_path_ = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), tmp_path_.c_str());
		}
		return true;
	}

	bubi::SSLConfigure::SSLConfigure() {}

	bubi::SSLConfigure::~SSLConfigure() {}

	bool bubi::SSLConfigure::Load(const Json::Value &value) {
		Configure::GetValue(value, "chain_file", chain_file_);
		Configure::GetValue(value, "private_key_file", private_key_file_);
		Configure::GetValue(value, "dhparam_file", dhparam_file_);
		Configure::GetValue(value, "verify_file", verify_file_);
		return true;
	}

	bubi::P2pNetwork::P2pNetwork(P2pType type) :
		target_peer_connection_(50),
		connect_timeout_(5),// second
		heartbeat_interval_(1800) {// second
		switch (type) {
		case bubi::P2pNetwork::CONSENSUS:
			listen_port_ = bubi::General::CONSENSUS_PORT;
			break;
		case bubi::P2pNetwork::TRANSACTION:
			listen_port_ = bubi::General::TRANSACTION_PORT;
			break;
		default:
			break;
		}
	}

	bubi::P2pNetwork::~P2pNetwork() {}

	bubi::P2pConfigure::P2pConfigure() :
		consensus_network_configure_(P2pNetwork::P2pType::CONSENSUS),
		transaction_network_configure_(P2pNetwork::P2pType::TRANSACTION) {}

	bubi::P2pConfigure::~P2pConfigure() {}

	bool bubi::P2pConfigure::Load(const Json::Value &value) {
		Configure::GetValue(value, "node_private_key", node_private_key_);
		ssl_configure_.Load(value["ssl"]);
		consensus_network_configure_.Load(value["consensus_network"]);
		transaction_network_configure_.Load(value["transaction_network"]);
		return true;
	}

	bool bubi::P2pNetwork::Load(const Json::Value &value) {
		int32_t temp;
		Configure::GetValue(value, "target_peer_connection", temp);
		target_peer_connection_ = temp;
		Configure::GetValue(value, "known_peers", known_peer_list_);
		Configure::GetValue(value, "connect_timeout", connect_timeout_);
		Configure::GetValue(value, "heartbeat_interval", heartbeat_interval_);
		Configure::GetValue(value, "listen_port", listen_port_);

		connect_timeout_ = connect_timeout_ * utils::MICRO_UNITS_PER_SEC; //micro second
		heartbeat_interval_ = heartbeat_interval_ * utils::MICRO_UNITS_PER_SEC; //micro second
		return true;
	}

	bubi::LoggerConfigure::LoggerConfigure() {
		path_ = bubi::General::LOGGER_FILE;
		dest_ = utils::LOG_DEST_OUT | utils::LOG_DEST_FILE;
		level_ = utils::LOG_LEVEL_ALL;
		time_capacity_ = 30;
		size_capacity_ = 100;
		expire_days_ = 10;
	}

	bubi::LoggerConfigure::~LoggerConfigure() {}

	bool bubi::LoggerConfigure::Load(const Json::Value &value) {
		Configure::GetValue(value, "path", path_);
		Configure::GetValue(value, "dest", dest_str_);
		Configure::GetValue(value, "level", level_str_);
		Configure::GetValue(value, "time_capacity", time_capacity_);
		Configure::GetValue(value, "size_capacity", size_capacity_);
		Configure::GetValue(value, "expire_days", expire_days_);

		time_capacity_ *= (3600 * 24);
		size_capacity_ *= utils::BYTES_PER_MEGA;

		// parse type string
		utils::StringVector dests, levels;
		dest_ = utils::LOG_DEST_NONE;
		dests = utils::String::Strtok(dest_str_, '|');

		for (auto &dest : dests) {
			std::string destitem = utils::String::ToUpper(dest);

			if (destitem == "ALL")         dest_ = utils::LOG_DEST_ALL;
			else if (destitem == "STDOUT") dest_ |= utils::LOG_DEST_OUT;
			else if (destitem == "STDERR") dest_ |= utils::LOG_DEST_ERR;
			else if (destitem == "FILE")   dest_ |= utils::LOG_DEST_FILE;
		}

		// parse level string
		level_ = utils::LOG_LEVEL_NONE;
		levels = utils::String::Strtok(level_str_, '|');

		for (auto &level : levels) {
			std::string levelitem = utils::String::ToUpper(level);

			if (levelitem == "ALL")          level_ = utils::LOG_LEVEL_ALL;
			else if (levelitem == "TRACE")   level_ |= utils::LOG_LEVEL_TRACE;
			else if (levelitem == "INFO")    level_ |= utils::LOG_LEVEL_INFO;
			else if (levelitem == "WARNING") level_ |= utils::LOG_LEVEL_WARN;
			else if (levelitem == "ERROR")   level_ |= utils::LOG_LEVEL_ERROR;
			else if (levelitem == "FATAL")   level_ |= utils::LOG_LEVEL_FATAL;
		}

		return true;
	}

	bubi::PipelineConfigure::PipelineConfigure(PipelineConfigType type) :workers_count_(1), type_(type) {}

	bubi::PipelineConfigure::~PipelineConfigure() {}

	bool bubi::PipelineConfigure::Load(const Json::Value &value) {
		int32_t recv_port;
		int32_t send_port;
		std::string address = "127.0.0.1";// 

		switch (type_) {
		case BUBI_SLAVE_T:
			recv_port = bubi::General::BUBI_SLAVE_PORT1;
			send_port = bubi::General::BUBI_SLAVE_PORT2;
			address = bubi::Configure::Instance().system_config_.local_address_;
			break;
		case BUBI_MONITOR_T:
			recv_port = bubi::General::BUBI_MONITOR_PORT1;
			send_port = bubi::General::BUBI_MONITOR_PORT2;
			address = bubi::Configure::Instance().system_config_.local_address_;
			break;
		case BUBI_MQSERVER_T:
			recv_port = bubi::General::MQSERVER_PORT1;
			send_port = bubi::General::MQSERVER_PORT2;
			address = bubi::Configure::Instance().system_config_.local_address_;
			break;
		case MONITOR_BUBI_T:
			recv_port = bubi::General::BUBI_MONITOR_PORT2;
			send_port = bubi::General::BUBI_MONITOR_PORT1;
			break;
		case MONITOR_SLAVE_T:
			recv_port = bubi::General::SLAVE_MONITOR_PORT1;
			send_port = bubi::General::SLAVE_MONITOR_PORT2;
			break;
		case SLAVE_BUBI_T:
			recv_port = bubi::General::BUBI_SLAVE_PORT2;
			send_port = bubi::General::BUBI_SLAVE_PORT1;
			break;
		case SLAVE_MONITOR_T:
			recv_port = bubi::General::SLAVE_MONITOR_PORT2;
			send_port = bubi::General::SLAVE_MONITOR_PORT1;
			break;
		default:
			break;
		}

		Configure::GetValue(value, "address", address);
		Configure::GetValue(value, "recv_port", recv_port);
		Configure::GetValue(value, "send_port", send_port);
		Configure::GetValue(value, "workers_count", workers_count_);
		Configure::GetValue(value, "hash_type", hash_type_);

		recv_address_ = utils::String::Format("tcp://%s:%d", address.c_str(), recv_port);
		send_address_ = utils::String::Format("tcp://%s:%d", address.c_str(), send_port);
		return true;
	}

	bubi::MqServerConfigure::MqServerConfigure() :
		tx_status_(false),
		pipeline_configure_(PipelineConfigure::PipelineConfigType::BUBI_MQSERVER_T) {}

	bubi::MqServerConfigure::~MqServerConfigure() {}

	bool bubi::MqServerConfigure::Load(const Json::Value &value) {
		pipeline_configure_.Load(value);
		Configure::GetValue(value, "listen_tx_status", tx_status_);

		return true;
	}


	bubi::WebServerConfigure::WebServerConfigure() {
		ssl_enable_ = false;
		query_limit_ = 1000;
		multiquery_limit_ = 100;
		remote_authorized_ = false;
        thread_count_ = 0;
	}

	bubi::WsServerConfigure::~WsServerConfigure() {}


	bool bubi::WsServerConfigure::Load(const Json::Value &value) {
		std::string address;
		Configure::GetValue(value, "listen_address", address);
		listen_address_ = utils::InetAddress(address);
		Configure::GetValue(value, "listen_tx_status", listen_tx_status_);

		return true;
	}


	bubi::WsServerConfigure::WsServerConfigure() {
		listen_tx_status_ = false;
	}

	bubi::WebServerConfigure::~WebServerConfigure() {}

	bool bubi::WebServerConfigure::Load(const Json::Value &value) {
		std::string listen_address_value;
		Configure::GetValue(value, "listen_addresses", listen_address_value);
		utils::StringVector address_array = utils::String::Strtok(listen_address_value, ',');
		for (size_t i = 0; i < address_array.size(); i++) {
			listen_addresses_.push_back(utils::InetAddress(address_array[i]));
		}
		Configure::GetValue(value, "ssl_enable", ssl_enable_);
		Configure::GetValue(value, "query_limit", query_limit_);
		Configure::GetValue(value, "multiquery_limit", multiquery_limit_);
		Configure::GetValue(value, "remote_authorized", remote_authorized_);
		Configure::GetValue(value, "thread_count", thread_count_);
		if (ssl_enable_)
			ssl_configure_.Load(value["ssl"]);
		return true;
	}

	bubi::LedgerConfigure::LedgerConfigure() {
		max_trans_per_ledger_ = 10000;
		max_trans_in_memory_ = 100000;
		max_ledger_per_message_ = 5;
		max_apply_ledger_per_round_ = 3;
		test_model_ = false;
	}

	bubi::LedgerConfigure::~LedgerConfigure() {}

	bool bubi::LedgerConfigure::Load(const Json::Value &value) {
		Configure::GetValue(value, "base_fee", base_fee_);
		Configure::GetValue(value, "base_reserve", base_reserve_);
		Configure::GetValue(value, "hash_type", hash_type_);
		Configure::GetValue(value, "max_trans_per_ledger", max_trans_per_ledger_);
		Configure::GetValue(value, "max_ledger_per_message", max_ledger_per_message_);
		Configure::GetValue(value, "max_apply_ledger_per_round", max_apply_ledger_per_round_);
		Configure::GetValue(value, "max_trans_in_memory", max_trans_in_memory_);
		Configure::GetValue(value, "test_model", test_model_);
		Configure::GetValue(value, "genesis_account", genesis_account_);

		if (max_apply_ledger_per_round_ == 0
			|| max_trans_in_memory_ / max_apply_ledger_per_round_ == 0) {
			return false;
		}
		return true;
	}

	bubi::ValidationConfigure::ValidationConfigure() {
		close_interval_ = 3;
		threshold_ = 1;
		is_validator_ = false;
	}

	bubi::ValidationConfigure::~ValidationConfigure() {}

	bool bubi::ValidationConfigure::Load(const Json::Value &value) {

		Configure::GetValue(value, "type", type_);
		Configure::GetValue(value, "is_validator", is_validator_);
		Configure::GetValue(value, "node_private_key", node_privatekey_);
		Configure::GetValue(value, "validators", validators_);
		Configure::GetValue(value, "close_interval", close_interval_);
		Configure::GetValue(value, "threshold", threshold_);
		if ((int32_t)validators_.size() < threshold_
			|| validators_.empty()) {
			return false;
		}
		close_interval_ = close_interval_ * utils::MICRO_UNITS_PER_SEC; //micro second
		return true;
	}

	bubi::SystemConfig::SystemConfig() {
		thread_count_ = 0;
		local_address_ = "127.0.0.1";
		delay_consensus_ = 2;
	}


	bubi::SystemConfig::~SystemConfig() {}

	bool bubi::SystemConfig::Load(const Json::Value &value) {
		Configure::GetValue(value, "local_address", local_address_);
		Configure::GetValue(value, "thread_count", thread_count_);
		Configure::GetValue(value, "delay_consensus", delay_consensus_);
		if (delay_consensus_ > 5) {
			return false;
		}
		return true;
	}

	bubi::CMonitorConfigure::CMonitorConfigure(PipelineConfigure::PipelineConfigType type) :
		real_time_status_(false),
		pipeline_configure_(type) {}

	bubi::CMonitorConfigure::~CMonitorConfigure() {}

	bool bubi::CMonitorConfigure::Load(const Json::Value &value) {
		pipeline_configure_.Load(value);
		Configure::GetValue(value, "real_time_status", real_time_status_);
		return true;
	}


	bubi::Configure::Configure() :
		slave_configure_(PipelineConfigure::PipelineConfigType::BUBI_SLAVE_T),
		monitor_configure_(PipelineConfigure::PipelineConfigType::BUBI_MONITOR_T) {}

	bubi::Configure::~Configure() {}

	bool bubi::Configure::Load(const std::string &config_file_path) {

		do {
			utils::File config_file;
			if (!config_file.Open(config_file_path, utils::File::FILE_M_READ)) {
				break;
			}

			std::string data;
			config_file.ReadData(data, utils::BYTES_PER_MEGA);

			Json::Reader reader;
			Json::Value values;
			if (!reader.parse(data, values)) {
				break;
			}
			if (!system_config_.Load(values["system"])) {
				break;
			}
			if (!validation_configure_.Load(values["validation"])) {
				break;
			}

			if (values.isMember("db")) db_configure_.Load(values["db"]);
			if (values.isMember("p2p")) p2p_configure_.Load(values["p2p"]);
			if (values.isMember("logger")) logger_configure_.Load(values["logger"]);
			if (values.isMember("webserver")) webserver_configure_.Load(values["webserver"]);
			if (values.isMember("ledger") && !ledger_configure_.Load(values["ledger"])) {
				break;
			}

			if (values.isMember("slave")) slave_configure_.Load(values["slave"]);
			if (values.isMember("monitor")) monitor_configure_.Load(values["monitor"]);
			if (values.isMember("mqserver")) mqserver_configure_.Load(values["mqserver"]);
			if (values.isMember("wsserver")) wsserver_configure_.Load(values["wsserver"]);

			return true;
		} while (false);

		return false;
	}

	bubi::SlaveConfigure::SlaveConfigure() :
		bubi_configure_(PipelineConfigure::PipelineConfigType::SLAVE_BUBI_T),
		monitor_configure_(PipelineConfigure::PipelineConfigType::SLAVE_MONITOR_T) {}

	bubi::SlaveConfigure::~SlaveConfigure() {}

	bool bubi::SlaveConfigure::Load(const std::string &config_file_path) {
		do {
			utils::File config_file;
			if (!config_file.Open(config_file_path, utils::File::FILE_M_READ)) {
				break;
			}

			std::string data;
			config_file.ReadData(data, utils::BYTES_PER_MEGA);

			Json::Reader reader;
			Json::Value values;
			if (!reader.parse(data, values)) {
				break;
			}

			bubi_configure_.Load(values["bubi"]);
			logger_configure_.Load(values["logger"]);
			webserver_configure_.Load(values["webserver"]);
			monitor_configure_.Load(values["monitor"]);
			return true;
		} while (false);

		return false;
	}

	bubi::WebSocketClientConfigure::WebSocketClientConfigure() {}

	bubi::WebSocketClientConfigure::~WebSocketClientConfigure() {}

	bool bubi::WebSocketClientConfigure::Load(const Json::Value &value) {
		do {
			Configure::GetValue(value, "monitor_id", monitor_id_);
			Configure::GetValue(value, "connect_address", connect_address_);
			Configure::GetValue(value, "bubi_config_path", bubi_config_path_);
			Configure::GetValue(value, "ssl_enable", ssl_enable_);
			Configure::GetValue(value, "chain_file", chain_file_);
			Configure::GetValue(value, "private_key_file", private_key_file_);
			Configure::GetValue(value, "dhparam_file", dhparam_file_);
			return true;
		} while (false);

		return false;
	}

	bubi::MonitorConfigure::MonitorConfigure() :
		slave_pipeline_configure_(PipelineConfigure::PipelineConfigType::MONITOR_SLAVE_T),
		bubi_pipeline_configure_(PipelineConfigure::PipelineConfigType::MONITOR_BUBI_T) {}

	bubi::MonitorConfigure::~MonitorConfigure() {}

	bool bubi::MonitorConfigure::Load(const std::string &config_file_path) {
		do {
			utils::File config_file;
			if (!config_file.Open(config_file_path, utils::File::FILE_M_READ)) {
				break;
			}

			std::string data;
			config_file.ReadData(data, utils::BYTES_PER_MEGA);

			Json::Reader reader;
			Json::Value values;
			if (!reader.parse(data, values)) {
				break;
			}

			web_socket_client_configure_.Load(values["web_socket_client"]);
			logger_configure_.Load(values["logger"]);
			slave_pipeline_configure_.Load(values["slave"]);
			bubi_pipeline_configure_.Load(values["bubi"]);

			immutable_config_file_path_ = web_socket_client_configure_.bubi_config_path_;
			if (!utils::File::IsAbsolute(immutable_config_file_path_)) {
				immutable_config_file_path_ = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), immutable_config_file_path_.c_str());
			}

			mutable_config_file_path_ = immutable_config_file_path_ + ".tmp";
			if (!utils::File::IsAbsolute(mutable_config_file_path_)) {
				mutable_config_file_path_ = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), mutable_config_file_path_.c_str());
			}
			return true;
		} while (false);

		return false;
	}

	bool MonitorConfigure::GetConfigure(Json::Value &object) {

		bool bret = false;
		do {
			Json::Value immutable_file_config;
			if (false == ReadConfigure(immutable_config_file_path_, immutable_file_config)) {
				LOG_ERROR("configure GetConfigure : get bubi.json failed");
				break;
			}
			if (!immutable_file_config.empty())
				object["immutable_config"] = immutable_file_config;

			Json::Value mutable_file_config;
			if (ReadConfigure(mutable_config_file_path_, mutable_file_config)) {
				object["mutable_config"] = mutable_file_config;
			}
			bret = true;
		} while (false);
		return bret;
	}

	bool MonitorConfigure::ReadConfigure(const std::string &file_path, Json::Value &object) {
		bool bret = false;
		do {
			utils::File config_file;
			if (!config_file.Open(file_path, utils::File::FILE_M_READ)) {
				break;
			}
			std::string data;
			config_file.ReadData(data, utils::BYTES_PER_MEGA);
			Json::Reader reader;
			if (!reader.parse(data, object)) {
				break;
			}
			config_file.Close();
			bret = true;
		} while (false);
		return bret;
	}

	bool MonitorConfigure::SetConfigure(std::string &config) {
		bool bret = false;
		do {
			utils::File config_file;
			if (!config_file.Open(mutable_config_file_path_, utils::File::FILE_M_WRITE)) {
				break;
			}
			if (config_file.Write(config.c_str(), 1, config.length()) < config.length()) {
				break;
			}
			config_file.Close();
			bret = true;
		} while (false);
		return bret;
	}
}