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
#include <common/configure.h>
#include <ledger/ledger_manager.h>
#include "monitor_master.h"

namespace bubi {

	MonitorMaster::MonitorMaster() : MonitorAgent(MonitorAgent::ROUND_ROBZOUT), peer_id_("") {

	}

	MonitorMaster::~MonitorMaster() {

	}

	bool MonitorMaster::Initialize() {
		bool bret = false;
		do {
			AddHandler(ZMQ_MONITOR_BUBI, std::bind(&MonitorMaster::on_bubi_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			AddHandler(ZMQ_MONITOR_LEDGER, std::bind(&MonitorMaster::on_ledger_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			AddHandler(ZMQ_MONITOR_ALERT, std::bind(&MonitorMaster::on_alert_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			AddHandler(ZMQ_MONITOR_ACCOUNT_EXCEPTION, std::bind(&MonitorMaster::on_account_exception_monitor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			std::string log_path = Configure::Instance().logger_configure_.path_;
			if (!utils::File::IsAbsolute(log_path)) {
				log_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), log_path.c_str());
			}

			PipelineConfigure& monitor_configure = Configure::Instance().monitor_configure_.pipeline_configure_;
			if (!MonitorAgent::Initialize(monitor_configure.send_address_, monitor_configure.recv_address_, log_path))
				break;

			bret = true;
		} while (false);
		return bret;
	}

	bool MonitorMaster::Exit() {
		return MonitorAgent::Exit();
	}

	void MonitorMaster::on_bubi_monitor(const char* msg, int len, std::string &reply) {
		Json::Value modules_status;
		{
			utils::ReadLockGuard guard(bubi::StatusModule::status_lock_);
			if (bubi::StatusModule::modules_status_ != NULL && !bubi::StatusModule::modules_status_->empty()) {
				modules_status.fromString(bubi::StatusModule::modules_status_->toFastString());
			}
		}
		if (bubi::StatusModule::modules_status_->empty()) {
			LOG_ERROR("on_bubi_method -- bubi info is empty");
		}
		else {
			Json::Value result;
			Json::Value& glue_manager = result["glue_manager"];
			glue_manager["system_current_time"] = modules_status.get("glue_manager", Json::Value::null).get("system", Json::Value::null).get("current_time", Json::Value::null);
			glue_manager["process_uptime"] = modules_status.get("glue_manager", Json::Value::null).get("system", Json::Value::null).get("process_uptime", Json::Value::null);
			glue_manager["system_uptime"] = modules_status.get("glue_manager", Json::Value::null).get("system", Json::Value::null).get("uptime", Json::Value::null);
			Json::Value& peer_manager = result["peer_manager"];
			peer_manager["peer_id"] = modules_status.get("peer_manager", Json::Value::null).get("peer_node_address", Json::Value::null);
			peer_manager["peers"] = modules_status.get("peer_manager", Json::Value::null).get("peers", Json::Value::null);
			reply = result.toFastString();
		}
		reply.push_back(8);
	}

	void MonitorMaster::on_ledger_monitor(const char* msg, int len, std::string &reply) {
		Json::Value receive;
		std::string seq = "";
		std::string num = "";
		receive.fromCString(msg);

		if (receive["parameter"]["seq"].empty() != true) {
			seq = receive["parameter"]["seq"].asString();
		}
		if (receive["parameter"]["num"].empty() != true) {
			num = receive["parameter"]["num"].asString();
		}

		// get ledger info
		LedgerFrm frm;
		Json::Value results;
		if (false == frm.LoadFromDb(seq, num, results)) {
			switch (results["error_code"].asUInt()) {
			case protocol::ERRORCODE::ERRCODE_NOT_EXIST:
				LOG_ERROR("on_ledger_method -- ledger %s does not exist", seq.c_str());
				break;
			case protocol::ERRORCODE::ERRCODE_INTERNAL_ERROR:
				LOG_ERROR("on_ledger_method -- ledger %s  internal error", seq.c_str());
				break;
			}
		}
		else {
			results.removeMember("error_code");
			reply = results.toFastString();
		}
		reply.push_back(20);
	}

	void MonitorMaster::on_alert_monitor(const char* msg, int len, std::string &reply) {
		do {
			Json::Value value;
			{
				utils::ReadLockGuard guard(bubi::StatusModule::status_lock_);
				if (bubi::StatusModule::modules_status_->empty()) {
					LOG_ERROR("OnSlowTimer -- bubi info is empty");
					break;
				}
				else {
					std::string modules_status = bubi::StatusModule::modules_status_->toFastString();
					Json::Value ledger_seqence = bubi::StatusModule::modules_status_->get("ledger_manager", Json::Value::null).get("ledger_sequence", Json::Value::null);
					if (!ledger_seqence.empty()) {
						value["ledger_sequence"] = ledger_seqence;
					}
					Json::Value sync_completed = bubi::StatusModule::modules_status_->get("ledger_manager", Json::Value::null).get("sync_completed", Json::Value::null);
					if (!sync_completed.empty()) {
						value["sync_completed"] = sync_completed;
					}
					if (0 == peer_id_.length()) {
						Json::Value id = bubi::StatusModule::modules_status_->get("peer_manager", Json::Value::null).get("peer_node_address", Json::Value::null);
						if (!id.isNull()) {
							value["id"] = id;
						}
					}
					else {
						value["id"] = peer_id_;
					}


				}
			}

			Json::Value system_data;
			if (system_manager_.GetSystemMonitor(log_path_, system_data)) {
				value["memory"] = system_data["result"]["memory"];
				value["cpu"] = system_data["result"]["cpu"];
				value["disk"] = system_data["result"]["disk"];
			}

			reply = value.toFastString();
		} while (false);
	}

	void MonitorMaster::on_account_exception_monitor(const char* msg, int len, std::string &reply) {
		do {
			{
				LOG_INFO("on_account_exception_monitor -- account_exception method is not completed.");
				break;
			}

			Json::Value result;
			result["type"] = 6;
			if (0 == peer_id_.length()) {
				utils::ReadLockGuard guard(bubi::StatusModule::status_lock_);
				Json::Value id = bubi::StatusModule::modules_status_->get("peer_manager", Json::Value::null).get("peer_node_address", Json::Value::null);
				if (!id.empty()) {
					result["id"] = id;
				}
			}
			else {
				result["id"] = peer_id_;
			}
		} while (false);
		reply.push_back(22);
	}
}