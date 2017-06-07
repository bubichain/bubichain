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

#include <common/configure.h>
#include <utils/headers.h>
#include "processmessage.h"
#include "monitor.h"

namespace bubi {
	const char* MONITOR_MESSAGE = "monitor message";
	const char* MONITOR_PROCESS = "monitor process";

	ProcessMessage::MonitorMessages::MonitorMessages(ProcessMessage* pprocess_message) : pprocess_message_(pprocess_message), bexit_(true) {}

	ProcessMessage::MonitorMessages::~MonitorMessages() {}

	bool ProcessMessage::MonitorMessages::Initialize() {
		bool bret = false;
		do {
			message_lists_.clear();
			monitor_error_[ZMQ_MONITOR_SYSTEM] = 6;
			monitor_error_[ZMQ_MONITOR_BUBI] = 8;
			monitor_error_[ZMQ_MONITOR_LEDGER] = 20;
			monitor_error_[ZMQ_MONITOR_ACCOUNT_EXCEPTION] = 22;

			thread_ptr_ = new utils::Thread(this);
			if (false == thread_ptr_->Start(MONITOR_MESSAGE)) {
				LOG_ERROR("Initialize -- Start failed.");
				break;
			}
			bret = true;
		} while (false);
		return bret;
	}

	bool ProcessMessage::MonitorMessages::Exit() {
		try {
			bexit_ = true;
			if (thread_ptr_) {
				thread_ptr_->JoinWithStop();
				delete thread_ptr_;
				thread_ptr_ = NULL;
			}
		}
		catch (std::exception& e) {
			LOG_ERROR("Exit -- %s", e.what());
		}
		return true;
	}

	void ProcessMessage::MonitorMessages::Run(utils::Thread *thread) {
		Monitor& monitor = Monitor::Instance();
		monitor.monitor_master_.Bind();
		monitor.monitor_slave_.Bind();
		bexit_ = false;
		do {
			if (message_lists_.empty()) {
				if (!bexit_) utils::Sleep(3000);
				continue;
			}
			MonitorMessage	msg;
			{
				utils::MutexGuard guard(message_lists_mutex_);
				msg = message_lists_.front();
			}
			if (!monitor.monitor_master_.Send(msg.type_, msg.buffer_)) {
				//if (!monitor.monitor_slave_.Send(msg.type_, msg.buffer_)) {
				MonitorError(msg.type_);
			}
			{
				utils::MutexGuard guard(message_lists_mutex_);
				message_lists_.pop_front();
			}
			utils::Sleep(10);
		} while (thread->enabled());
	}

	void ProcessMessage::MonitorMessages::MonitorError(ZMQTaskType type) {
		do {
			if (ZMQ_MONITOR_ALERT == type) {
				break;
			}

			bool type_is_exists = true;
			if (monitor_error_.find(type) == monitor_error_.end()) {
				LOG_ERROR("MonitorError -- error code for ZMQTaskType %d does not exist.", type);
				break;
			}
			std::string message = "";
			message.push_back(monitor_error_.at(type));
			int64_t p = (int64_t)this;
			message.append((char*)&p, sizeof(int64_t));
			if (pprocess_message_ != NULL) {
				std::string reply;
				pprocess_message_->on_common_monitor(message.c_str(), message.length(), reply);
			}
		} while (false);
	}

	void ProcessMessage::MonitorMessages::ResetState() {}

	void ProcessMessage::MonitorMessages::PushBackMonitorMessage(const MonitorMessage& message) {
		utils::MutexGuard guard(message_lists_mutex_);
		message_lists_.push_back(message);
	}

	ProcessMessage::ProcessMessage() :
		monitor_message_(this),
		upgrade_(this),
		check_exception_interval_(5 * utils::MICRO_UNITS_PER_SEC),
		connection_timeout_(120 * utils::MICRO_UNITS_PER_SEC) {
		ResetState();
	}

	ProcessMessage::~ProcessMessage() {}

	bool ProcessMessage::Exit() {
		try {
			blogin_ = false;
			if (thread_ptr_) {
				thread_ptr_->JoinWithStop();
				delete thread_ptr_;
				thread_ptr_ = NULL;
			}
			monitor_message_.Exit();
			notice_.Exit();
			upgrade_.Exit();
			alert_.Exit();
		}
		catch (std::exception& e) {
			LOG_ERROR("Exit -- %s", e.what());
		}
		return true;
	}

	bool ProcessMessage::Initialize() {
		bool bret = false;
		do {
			request_methods_["bubi"] = &ProcessMessage::on_common_method;
			request_methods_["ledger"] = &ProcessMessage::on_common_method;
			request_methods_["system"] = &ProcessMessage::on_common_method;
			request_methods_["account_exception"] = &ProcessMessage::on_common_method;
			request_methods_["upgrade"] = &ProcessMessage::on_upgrade_method;
			request_methods_["logout"] = &ProcessMessage::on_logout_method;
			request_methods_["get_configure"] = &ProcessMessage::on_get_configure_method;
			request_methods_["set_configure"] = &ProcessMessage::on_set_configure_method;

			response_methods_["hello"] = &ProcessMessage::on_response_hello;
			response_methods_["register"] = &ProcessMessage::on_response_register;
			response_methods_["error"] = &ProcessMessage::on_response_error;
			response_methods_["system"] = &ProcessMessage::on_response_common;
			response_methods_["bubi"] = &ProcessMessage::on_response_common;
			response_methods_["ledger"] = &ProcessMessage::on_response_ledger;
			response_methods_["upgrade"] = &ProcessMessage::on_response_upgrade;
			response_methods_["get_configure"] = &ProcessMessage::on_response_common;
			response_methods_["set_configure"] = &ProcessMessage::on_response_common;
			response_methods_["account_exception"] = &ProcessMessage::on_response_common;
			response_methods_["block_status"] = &ProcessMessage::on_response_common;
			response_methods_["heartbeat"] = &ProcessMessage::on_response_heartbeat;
			response_methods_["warning"] = &ProcessMessage::on_response_common;

			monitor_type_["bubi"] = ZMQ_MONITOR_BUBI;
			monitor_type_["ledger"] = ZMQ_MONITOR_LEDGER;
			monitor_type_["system"] = ZMQ_MONITOR_SYSTEM;
			monitor_type_["alert"] = ZMQ_MONITOR_ALERT;
			monitor_type_["account_exception"] = ZMQ_MONITOR_ACCOUNT_EXCEPTION;

			error_type_[6] = "system";
			error_type_[8] = "bubi";
			error_type_[20] = "ledger";
			error_type_[22] = "account_exception";

			Monitor& monitor = Monitor::Instance();
			monitor.monitor_master_.AddHandler(ZMQ_MONITOR_BUBI,
				std::bind(&ProcessMessage::on_common_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			monitor.monitor_master_.AddHandler(ZMQ_MONITOR_LEDGER,
				std::bind(&ProcessMessage::on_common_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			monitor.monitor_master_.AddHandler(ZMQ_MONITOR_SYSTEM,
				std::bind(&ProcessMessage::on_common_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			monitor.monitor_master_.AddHandler(ZMQ_MONITOR_ACCOUNT_EXCEPTION,
				std::bind(&ProcessMessage::on_common_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			monitor.monitor_master_.AddHandler(ZMQ_MONITOR_ALERT,
				std::bind(&ProcessMessage::on_alert_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			monitor.monitor_master_.AddHandler(ZMQ_MONITOR_NOTICE,
				std::bind(&ProcessMessage::on_notice_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			if (!monitor_message_.Initialize() ||
				!notice_.Initialize() ||
				!upgrade_.Initialize() ||
				!alert_.Initialize()) {
				break;
			}

			LoggerConfigure &config = MonitorConfigure::Instance().logger_configure_;
			log_path_ = config.path_;
			if (!utils::File::IsAbsolute(log_path_)) {
				log_path_ = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), log_path_.c_str());
			}
			thread_ptr_ = new utils::Thread(this);
			if (!thread_ptr_->Start(MONITOR_PROCESS)) {
				LOG_ERROR("Initialize -- start thread failed");
				break;
			}
			bret = true;
		} while (false);
		return bret;
	}

	void ProcessMessage::ResetState() {
		blogin_ = false;
		bheartbeated_ = false;
		bclosebeated_ = false;
		bupgrading_ = false;
		pendpoint_ = NULL;
		random_key_ = "";
		heartbeat_last_time_ = utils::Timestamp::HighResolution();
		heartbeat_interval_ = 30 * utils::MICRO_UNITS_PER_SEC;
		monitor_message_.ResetState();
		alert_.ResetState();
		notice_.ResetState();
	}

	void ProcessMessage::Run(utils::Thread *thread) {
		while (thread->enabled()) {
			// process request message
			do {
				if (request_message_lists_.empty()) {
					utils::Sleep(100);
					continue;
				}
				std::string	msg;
				{
					utils::MutexGuard guard(request_message_lists_mutex_);
					msg = request_message_lists_.front();
				}
				ProcessRequestMessages(msg);
				utils::MutexGuard guard(request_message_lists_mutex_);
				request_message_lists_.pop_front();
			} while (false);

			// process response message
			do {
				if (response_message_lists_.empty()) {
					utils::Sleep(100);
					continue;
				}
				std::string	msg;
				{
					utils::MutexGuard guard(response_message_lists_mutex_);
					msg = response_message_lists_.front();
				}
				ProcessResponseMessages(msg);
				{
					utils::MutexGuard guard(response_message_lists_mutex_);
					response_message_lists_.pop_front();
				}
				utils::Sleep(10);
			} while (false);
		}
	}

	void ProcessMessage::PushBackRequestMessages(const std::string& msg) {
		utils::MutexGuard guard(request_message_lists_mutex_);
		request_message_lists_.push_back(msg);
	}

	void ProcessMessage::PushBackResponseMessages(const std::string& msg) {
		utils::MutexGuard guard(response_message_lists_mutex_);
		response_message_lists_.push_back(msg);
	}

	bool ProcessMessage::ProcessResponseMessages(const std::string& msg) {
		bool bret = false;
		try {
			do {
				if (NULL == pendpoint_) break;
				pendpoint_->send(connection_hdl_, msg, websocketpp::frame::opcode::text);

				bret = true;
			} while (false);
		}
		catch (std::exception&) {
			LOG_ERROR("ProcessResponseMessages -- server has disconnected");
		}
		return bret;
	}

	bool ProcessMessage::ProcessRequestMessages(const std::string& msg) {
		bool bret = false;

		do {
			try {
				Json::Value receive;

				if (!receive.fromString(msg)) {
					// disconnect
					LOG_ERROR("ProcessRequestMessages : message is not json style");
					if (NULL == pendpoint_) break;
					pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
					break;
				}
				// check the message's items
				if (receive["method"].empty()) {
					LOG_ERROR("ProcessRequestMessages : method item is empty");
					if (NULL == pendpoint_) break;
					pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
					break;
				}
				if (receive["request"].empty()) {
					LOG_ERROR("ProcessRequestMessages -- request item is empty");
					if (NULL == pendpoint_) break;
					pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
					break;
				}
				if (!receive["request"].asBool() && receive["error_code"].empty()) {
					LOG_ERROR("ProcessRequestMessages : response message's error_code item is empty");
					if (NULL == pendpoint_) break;
					pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
					break;
				}

				const std::string strmethod = receive["method"].asString();
				// do or not have session_id
				if (strmethod.compare("hello") != 0 && strmethod.compare("logout") != 0) {
					if (receive["parameter"].empty() && receive["result"].empty()) {
						LOG_ERROR("ProcessRequestMessages : parameter and result item is empty");
						if (NULL == pendpoint_) break;
						pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
						break;
					}
					bool bsession_id = false;
					Json::Value& value = receive["parameter"].isNull() ? receive["result"] : receive["parameter"];
					std::string session_id = value["session_id"].asString();
					if (false == value["session_id"].isNull() && session_id.compare(random_key_) == 0) {
						bsession_id = true;
					}

					if (false == bsession_id) {
						LOG_ERROR("ProcessRequestMessages -- session_id does not exist");
						if (NULL == pendpoint_) break;
						pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
						break;
					}
				}
				AgentMethodMap *pmethods = receive["request"].asBool() ? &request_methods_ : &response_methods_;
				AgentMethodMap::iterator itr_method = pmethods->find(strmethod);
				// method not exist
				if (itr_method == pmethods->end()) {
					std::string strMessageBody = receive.toFastString();
					std::string strerror = "unknown method(" + strmethod;
					if (receive["request"].asBool()) {
						Json::Value result;
						strerror += ") request";
						int error_code = 4;
						result["session_id"] = random_key_;

						LOG_ERROR("ProcessRequestMessages -- %s", strerror.c_str());
						SendResponseMessage("error", true, error_code, result);
						break;
					}
					else {
						strerror += ") response";
						LOG_ERROR("ProcessRequestMessages -- %s", strerror.c_str());
						break;
					}
				}
				// run method
				MonitorMethodProc agent_method = itr_method->second;
				(this->*agent_method)(msg.c_str(), strmethod);
				bret = true;
			}
			catch (std::exception&) {
				LOG_ERROR("ProcessRequestMessages -- server has disconnected");
			}
		} while (false);
		return bret;
	}

	void ProcessMessage::on_common_method(const char* msg, const std::string& method) {
		try {
			do {
				MonitorMessage message;
				if (monitor_type_.find(method) != monitor_type_.end()) {
					message.type_ = monitor_type_.at(method);
				}
				else {
					LOG_ERROR("on_common_method -- monitor_type for this method %s is not exist", method.c_str());
					break;
				}
				if (ZMQ_MONITOR_LEDGER == message.type_) {
					Json::Value receive;
					Json::Value result;
					int32_t error_code = 0;
					int32_t seq = 1;
					int32_t num = 1;
					receive.fromCString(msg);
					if (receive["parameter"]["seq"].empty() != true) {
						seq = receive["parameter"]["seq"].asInt();
					}
					if (receive["parameter"]["num"].empty() != true) {
						num = receive["parameter"]["num"].asInt();
					}
					if (seq <= 0 || num <= 0) {
						error_code = 14;
						result["session_id"] = random_key_;
						SendResponseMessage(method, true, error_code, result);
						break;
					}
				}

				message.buffer_ = msg;
				int64_t p = (int64_t)this;
				message.buffer_.append((char*)&p, sizeof(int64_t));
				monitor_message_.PushBackMonitorMessage(message);
			} while (false);
		}
		catch (std::exception& e) {
			LOG_ERROR("on_common_method -- %s", e.what());
		}
	}

	void ProcessMessage::on_upgrade_method(const char* msg, const std::string& method) {
		Json::Value receive;
		Json::Value result;
		int error_code = 0;
		receive.fromCString(msg);
		do {
			if (receive["parameter"]["items"].empty()) {
				LOG_ERROR("on_upgrade_method : items is empty");
				error_code = 14;
				break;
			}
			if (bupgrading_) {
				LOG_ERROR("on_upgrade_method : upgrading");
				result["state"] = 1;
				error_code = 12;
				break;
			}

			bupgrading_ = true;
			result["state"] = 1;
			Json::Value& items = receive["parameter"]["items"];
			upgrade_.SetItems(items);
		} while (false);
		if (error_code) {
			result["session_id"] = random_key_;
			SendResponseMessage(method, true, error_code, result);
		}
	}

	void ProcessMessage::on_get_configure_method(const char* msg, const std::string& method) {
		Json::Value result;
		int error_code = 0;

		bubi::MonitorConfigure& configure = bubi::MonitorConfigure::Instance();
		if (false == configure.GetConfigure(result)) {
			LOG_ERROR("on_get_configure_method -- get configure failed");
			error_code = 7;
		}
		result["session_id"] = random_key_;
		SendResponseMessage(method, true, error_code, result);
	}

	void ProcessMessage::on_set_configure_method(const char* msg, const std::string& method) {
		Json::Value receive;
		receive.fromCString(msg);

		Json::Value result;
		int error_code = 0;
		bool brequest = true;

		do {
			Json::Value file_config;
			if (receive["parameter"]["config_file"].isNull() ||
				false == file_config.fromString(receive["parameter"]["config_file"].asString())) {
				LOG_ERROR("on_set_configure_method -- configure content error");
				error_code = 1;
				break;
			}

			bubi::MonitorConfigure& configure = bubi::MonitorConfigure::Instance();
			std::string json_style = file_config.toStyledString();
			if (false == configure.SetConfigure(json_style)) {
				LOG_ERROR("on_set_configure_method -- write configure file failed");
				error_code = 18;
				break;
			}
		} while (false);
		result["session_id"] = random_key_;
		SendResponseMessage(method, true, error_code, result);
	}

	void ProcessMessage::on_logout_method(const char* msg, const std::string& method) {
		do {
			try {
				LOG_INFO("on_logout_method -- logout & close");
				if (NULL == pendpoint_) break;
				pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, random_key_);
			}
			catch (std::exception&) {
				std::string method = "error";
				int error_code = 17;
				Json::Value result;
				result["session_id"] = random_key_;

				SendResponseMessage(method, true, error_code, result);
			}
		} while (false);
	}

	void ProcessMessage::on_response_hello(const char* msg, const std::string& method) {
		do {
			Json::Value receive;
			receive.fromCString(msg);
			uint32_t error_code = receive["error_code"].asUInt();
			if (error_code) {
				switch (error_code) {
				case 2:
					LOG_ERROR("on_response_hello -- agent id does not exist");
					break;
				case 16:
					LOG_ERROR("on_response_hello -- user already exists");
					break;
				default:
					LOG_ERROR("on_response_hello -- illegal error_code");
					break;
				}
				if (16 == error_code || 2 == error_code) {
					if (NULL == pendpoint_) break;
					pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
					break;
				}
				else {
					LOG_ERROR("on_response_hello -- illegal error_code");
					if (NULL == pendpoint_) break;
					pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
					break;
				}
			}
			Json::Value& result = receive["result"];
			if (result["random_key"].empty() || result["rand_id_md"].empty() || result["threshold"].empty()) {
				LOG_ERROR("on_response_hello -- illegal parameter");
				if (NULL == pendpoint_) break;
				pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
				break;
			}
			random_key_ = result["random_key"].asString();
			std::string md_unsign = monitor_id_ + random_key_;
			if (result["rand_id_md"] != utils::MD5::GenerateMD5((unsigned char*)md_unsign.c_str(), md_unsign.length())) {
				LOG_ERROR("on_response_hello -- server is invalid and close");
				if (NULL == pendpoint_) break;
				pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, random_key_);
				break;
			}
			// configure info
			if (!result["threshold"]["cpu"].empty())
				alert_.SetCpuCriticality(result["threshold"]["cpu"].asDouble());
			if (!result["threshold"]["memory"].empty())
				alert_.SetMemoryCriticality(result["threshold"]["memory"].asDouble());
			if (!result["threshold"]["disk"].empty())
				alert_.SetDiskCriticality(result["threshold"]["disk"].asDouble());
			if (!result["threshold"]["consensus"].empty())
				alert_.SetConsensusTime(result["threshold"]["consensus"].asInt64());
			if (!result["threshold"]["bubi_crack"].empty())
				alert_.SetBubiCrackTime(result["threshold"]["bubi_crack"].asInt64());
			if (!result["threshold"]["bubi_attack_time"].empty())
				notice_.SetBubiAttackTime(result["threshold"]["bubi_attack_time"].asInt64());
			if (!result["threshold"]["bubi_attack_counts"].empty())
				notice_.SetBubiAttackTime(result["threshold"]["bubi_attack_counts"].asInt64());
			if (!result["connection_timout"].empty())
				connection_timeout_ = result["connection_timout"].asInt64() * utils::MICRO_UNITS_PER_SEC;
			heartbeat_interval_ = connection_timeout_ / 4;
			// reply
			Json::Value parameter;
			parameter["session_id"] = random_key_;
			SendRequestMessage("register", true, parameter);
		} while (false);
	}

	void ProcessMessage::on_response_register(const char* msg, const std::string& method) {
		Json::Value receive;
		receive.fromCString(msg);
		uint32_t error_code = receive["error_code"].asUInt();
		if (error_code != 0) {
			LOG_ERROR("on_response_register -- illegal error_code");
			if (NULL == pendpoint_) return;
			pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
			return;
		}
		heartbeat_last_time_ = utils::Timestamp::HighResolution();
		last_check_exception_time_ = heartbeat_last_time_;
		blogin_ = true;
		bclosebeated_ = false;
		bheartbeated_ = false;
		LOG_INFO("websocketclient on_response_register : register successful");
	}

	void ProcessMessage::on_response_ledger(const char* msg, const std::string& method) {
		Json::Value receive;
		receive.fromCString(msg);
		uint32_t error_code = receive["error_code"].asInt();
		if (error_code) {
			switch (error_code) {
			case 14:
				LOG_ERROR("on_response_ledger -- illegal parameter");
				break;
			case 20:
				LOG_ERROR("on_response_ledger -- send ledger info failed");
				break;
			case 21:
				LOG_ERROR("on_response_ledger -- internal error");
				break;
			default:
				LOG_ERROR("on_response_ledger -- illegal error_code");
				if (NULL == pendpoint_) break;
				pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
				break;
			}
		}
	}

	void ProcessMessage::on_response_common(const char* msg, const std::string& method) {
		Json::Value receive;
		receive.fromCString(msg);
		uint32_t error_code = receive["error_code"].asInt();
		if (error_code) {
			switch (error_code) {
			case 6:
			case 7:
			case 8:
			case 18:
			case 22:
				LOG_ERROR("on_response_common : send %s failed", receive["method"].asCString());
				break;
			default:
				LOG_ERROR("on_response_common %s -- illegal error_code", receive["method"].asCString());
				if (NULL == pendpoint_) break;
				pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
				break;
			}
		}
	}

	void ProcessMessage::on_response_heartbeat(const char* msg, const std::string& method) {
		Json::Value receive;
		receive.fromCString(msg);
		if (receive["error_code"].asInt()) {
			LOG_ERROR("on_response_heartbeat -- illegal error_code");
			if (NULL == pendpoint_) return;
			pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
			return;
		}
		heartbeat_last_time_ = utils::Timestamp::HighResolution();
		bheartbeated_ = false;
	}

	void ProcessMessage::on_response_upgrade(const char* msg, const std::string& method) {
		Json::Value receive;
		receive.fromCString(msg);
		uint32_t error_code = receive["error_code"].asInt();
		if (error_code) {
			switch (error_code) {
			case 9:
			case 10:
			case 11:
			case 12:
			case 14:
			case 19:
			case 21:
				LOG_ERROR("on_response_upgrade : send %s failed", receive["method"].asCString());
				break;
			default:
				LOG_ERROR("on_response_upgrade -- illegal error_code");
				if (NULL == pendpoint_) break;
				pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
				break;
			}
		}

	}

	void ProcessMessage::on_response_error(const char* msg, const std::string& method) {
		Json::Value receive;
		receive.fromCString(msg);
		uint32_t error_code = receive["error_code"].asInt();
		if (error_code) {
			switch (error_code) {
			case 1:
				LOG_ERROR("on_response_error -- configure content error");
				break;
			case 3:
				LOG_ERROR("on_response_error -- service signature error");
				break;
			case 4:
				LOG_ERROR("on_response_error -- method does not exist");
				break;
			case 6:
				LOG_ERROR("on_response_error -- system info error");
				break;
			case 8:
				LOG_ERROR("on_response_error -- bubi info failed");
				break;
			case 9:
				LOG_ERROR("on_response_error -- system user permission denied");
				break;
			case 10:
				LOG_ERROR("on_response_error -- file download failed");
				break;
			case 11:
				LOG_ERROR("on_response_error -- file md5 error");
				break;
			case 12:
				LOG_ERROR("on_response_error -- is upgrading");
				break;
			case 13:
				LOG_ERROR("on_response_error -- session id error");
				break;
			case 14:
				LOG_ERROR("on_response_error -- illegal parameter");
				break;
			case 17:
				LOG_ERROR("on_response_error -- logout failed");
				break;
			case 18:
				LOG_ERROR("on_response_error -- set configure file failed");
				break;
			case 19:
				LOG_ERROR("on_response_error -- upgrade a file is not this program");
				break;
			default:
				LOG_ERROR("on_response_error -- illegal error_code");
				if (NULL == pendpoint_) break;
				pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
				break;
			}
		}
	}

	void ProcessMessage::on_common_monitor(const char* msg, int len, std::string &reply) {
		try {
			Json::Value result;
			int error_code = msg[len - sizeof(int64_t) - 1];
			std::string method = "";
			do {
				if (error_type_.find(error_code) == error_type_.end()) {
					LOG_ERROR("on_common_monitor -- this method %s for monitor is not exist", method.c_str());
					break;
				}

				method = error_type_.at(error_code);
				// check the bubi state
				if (alert_.GetBubiCrackState()) {
					break;
				}
				std::string message(msg, len - sizeof(int64_t) - 1);
				if (len <= sizeof(int64_t) + 1) {
					LOG_ERROR("on_common_monitor -- %s's buffer is empty", method.c_str());
					break;
				}
				Json::Value value;
				if (!value.fromString(message)) {
					LOG_ERROR("on_common_monitor -- %s'buffer's format is invalid", method.c_str());
					break;
				}
				error_code = 0;
				result = value;
			} while (false);
			result["session_id"] = random_key_;
			SendResponseMessage(method.c_str(), true, error_code, result);
		}
		catch (std::exception& e) {
			LOG_ERROR("on_common_monitor -- %s", e.what());
		}
	}

	void ProcessMessage::on_alert_monitor(const char* msg, int len, std::string &reply) {
		do {
			std::string message(msg, len - sizeof(int64_t));
			alert_.SetBubiLastTime(utils::Timestamp::HighResolution());
			alert_.SetBuffer(message);
		} while (false);
	}

	void ProcessMessage::on_notice_monitor(const char* msg, int len, std::string &reply) {
		std::string message(msg, len - sizeof(int64_t));
		notice_.SetBuffer(message);
		Json::Value result;
		if (notice_.ProcessNotice(result)) {
			try {
				if (!alert_.GetId().empty()) {
					result["id"] = alert_.GetId();
					result["session_id"] = random_key_;
					SendRequestMessage("block_status", true, result);
					LOG_INFO("on_notice_monitor -- start send notice: %s", result.toFastString().c_str());
				}
			}
			catch (std::exception& e) {
				LOG_ERROR("on_notice_monitor -- %s", e.what());
			}
		}
	}

	void ProcessMessage::on_account_exception_monitor(const char* msg, int len, std::string &reply) {
		Json::Value result;
		int error_code = 0;
		do {
			std::string message(msg, len - sizeof(int64_t));
			Json::Value value;
			if (!value.fromString(message)) {
				LOG_ERROR("on_account_exception_monitor -- buffer's format is invalid");
				break;
			}
			result = value;
		} while (false);

		result["session_id"] = random_key_;
		SendResponseMessage("account_exception", true, error_code, result);
	}

	void ProcessMessage::SendRequestMessage(const std::string& method, const bool& request, const Json::Value& parameter) {
		do {
			Json::Value message;
			message["method"] = method;
			message["request"] = request;
			message["parameter"] = parameter;
			PushBackResponseMessages(message.toFastString());
		} while (false);
	}

	void ProcessMessage::SendResponseMessage(const std::string& method, const bool& request, const int& error_code, const Json::Value& result) {
		do {
			Json::Value message;
			message["method"] = method;
			message["request"] = request;
			message["error_code"] = error_code;
			message["result"] = result;
			PushBackResponseMessages(message.toFastString());
		} while (false);
	}

	void ProcessMessage::OnTimer() {
		do {
			if (!blogin_) {
				int64_t high_resolution_time = utils::Timestamp::HighResolution();
				if (high_resolution_time - heartbeat_last_time_ > heartbeat_interval_) {
					try {
						LOG_ERROR("OnTimer -- server is refused to be connected, now connecting time out.");
						if (pendpoint_ != NULL)
							pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
					}
					catch (std::exception& e) {
						LOG_ERROR("OnTimer -- no login -- %s", e.what());
					}
					heartbeat_last_time_ = high_resolution_time;
				}
				break;
			}

			int64_t high_resolution_time = utils::Timestamp::HighResolution();
			if (high_resolution_time - heartbeat_last_time_ > heartbeat_interval_) {
				try {
					// timeout
					if (high_resolution_time - heartbeat_last_time_ >= connection_timeout_ && false == bclosebeated_) {
						if (NULL == pendpoint_) break;
						LOG_ERROR("OnTimer -- send heartbeat failed & closing");
						pendpoint_->close(connection_hdl_, websocketpp::close::status::going_away, "");
						bclosebeated_ = true;
					}
					else if (false == bheartbeated_) {
						Json::Value parameter;
						parameter["session_id"] = random_key_;
						SendRequestMessage("heartbeat", true, parameter);
						bheartbeated_ = true;
					}
				}
				catch (std::exception&) {
					LOG_ERROR("OnTimer -- send heartbeat failed");
				}
			}
		} while (false);
	}

	void ProcessMessage::OnSlowTimer() {
		do {
			if (!blogin_) break;

			// send alert request
			int64_t high_resolution_time = utils::Timestamp::HighResolution();
			if (high_resolution_time - check_exception_interval_ > last_check_exception_time_) {
				on_common_method("", "alert");
				last_check_exception_time_ = high_resolution_time;
			}

			// check alert
			Json::Value result;
			if (alert_.CheckAlert(result)) {
				try {
					result["session_id"] = random_key_;
					SendRequestMessage("warning", true, result);
					LOG_INFO("OnSlowTimer -- start send warning: %s", result.toFastString().c_str());
				}
				catch (std::exception& e) {
					LOG_ERROR("OnSlowTimer -- %s", e.what());
				}
			}

			// check transactions exceptions
			result.clear();
			if (notice_.CheckTxException(result)) {
				try {
					result["session_id"] = random_key_;
					SendRequestMessage("block_status", true, result);
					LOG_INFO("OnSlowTimer -- start send warning: %s", result.toFastString().c_str());
				}
				catch (std::exception& e) {
					LOG_ERROR("OnSlowTimer -- %s", e.what());
				}
			}
		} while (false);
	}
}
