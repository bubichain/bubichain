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

/************************************************************************/
/* Author: fengruiming                                                  */
/* Date: 2016.3.28                                                      */
/* Function: connect to bubi with shared memory and get bubi info       */
/************************************************************************/

#include "alert.h"
#include "monitor.h"
#include "processmessage.h"
#include <utils/timestamp.h>
#include <utils/logger.h>

namespace bubi {
	Alert::Alert() {
		ResetState();
		cpu_criticality_ = 100;
		memory_criticality_ = 100;
		disk_criticality_ = 100;
		consensus_time_ = 360;
		bubi_crack_time_ = 60;
		alert_interval_ = 8 * utils::MICRO_UNITS_PER_SEC;
	}

	Alert::~Alert() {}

	bool Alert::Initialize() {
		return true;
	}

	bool Alert::Exit() {
		return true;
	}

	void Alert::ResetState() {
		cpu_state_.clear();
		memory_state_.clear();
		disk_state_.clear();
		consensus_state_.clear();
		bubi_state_.clear();
		consensus_last_time_ = utils::Timestamp::HighResolution();
		bubi_last_time_ = consensus_last_time_;
		alert_last_time_ = bubi_last_time_;
	}

	std::string Alert::GetId() {
		utils::MutexGuard guard_id(mutex_id_);
		return id_;
	}

	bool Alert::GetBubiCrackState() {
		bool bret = false;
		if (!bubi_state_["state"].empty() && 0 == bubi_state_["state"].asUInt()) {
			bret = true;
		}
		return bret;
	}

	void Alert::SetBuffer(const std::string buffer) {
		utils::MutexGuard guard_buffer(mutex_buffer_);
		buffer_ = buffer;
	}

	void Alert::SetCpuCriticality(const double cpu_criticality) {
		cpu_criticality_ = cpu_criticality;
	}

	void Alert::SetMemoryCriticality(const double memory_criticality) {
		memory_criticality_ = memory_criticality;
	}

	void Alert::SetDiskCriticality(const double disk_criticality) {
		disk_criticality_ = disk_criticality;
	}

	void Alert::SetConsensusTime(const uint64_t consensus_time) {
		consensus_time_ = consensus_time;
	}

	void Alert::SetBubiCrackTime(const uint64_t bubi_crack_time) {
		bubi_crack_time_ = bubi_crack_time;
	}

	void Alert::SetBubiLastTime(uint64_t bubi_last_time) {
		bubi_last_time_ = bubi_last_time;
	}

	bool Alert::CheckBubiState() {
		bool bwarning = false;
		uint64_t high_resolution_time = utils::Timestamp::HighResolution();
		std::string buffer;
		{
			utils::MutexGuard guard_buffer(mutex_buffer_);
			buffer = buffer_;
		}
		if ((bubi_state_["state"].asUInt() == General::NOWARNING || bubi_state_["state"].empty()) &&
			(high_resolution_time - bubi_last_time_ > bubi_crack_time_ * utils::MICRO_UNITS_PER_SEC)) {
			bubi_state_["state"] = General::WARNING;
			bwarning = true;
		}
		else if ((bubi_state_["state"].asUInt() == General::WARNING ||
			bubi_state_["state"].empty()) &&
			(high_resolution_time - bubi_last_time_ < bubi_crack_time_ * utils::MICRO_UNITS_PER_SEC) &&
			buffer.length() > 0) {
			bubi_state_["state"] = General::NOWARNING;
			bwarning = true;
		}
		return bwarning;
	}

	bool Alert::CheckConsensusState(const uint64_t& ledger_sequence) {
		bool bwarning = false;
		do {
			uint64_t high_resolution_time = utils::Timestamp::HighResolution();
			std::string buffer;
			{
				utils::MutexGuard guard_buffer(mutex_buffer_);
				buffer = buffer_;
			}
			if ((consensus_state_["state"].empty() || consensus_state_["state"].asUInt() == General::NOWARNING) &&
				(high_resolution_time - consensus_last_time_ > consensus_time_ * utils::MICRO_UNITS_PER_SEC) &&
				(consensus_state_["value"] >= ledger_sequence) && buffer.length() > 0) {
				consensus_state_["value"] = ledger_sequence;
				consensus_state_["state"] = General::WARNING;
				consensus_last_time_ = high_resolution_time;
				bwarning = true;
			}
			else if ((consensus_state_["state"].empty() || consensus_state_["state"].asUInt() == General::WARNING) &&
				(consensus_state_["value"] < ledger_sequence) && buffer.length() > 0) {
				consensus_state_["value"] = ledger_sequence;
				consensus_state_["state"] = General::NOWARNING;
				consensus_last_time_ = high_resolution_time;
				bwarning = true;
			}
		} while (false);
		return bwarning;
	}

	bool Alert::CheckCpuWarning(const double& cpu_used_percent) {
		bool bwarning = false;
		if (cpu_state_["state"].empty() || cpu_state_["state"].asUInt() == General::NOWARNING) {
			if (cpu_used_percent >= cpu_criticality_) {
				cpu_state_["value"] = cpu_used_percent;
				cpu_state_["state"] = General::WARNING;
				bwarning = true;
			}
		}
		else if (cpu_state_["state"].empty() || cpu_state_["state"].asUInt() == General::WARNING) {
			if (cpu_used_percent < cpu_criticality_) {
				cpu_state_["value"] = cpu_used_percent;
				cpu_state_["state"] = General::NOWARNING;
				bwarning = true;
			}
		}
		return bwarning;
	}

	bool Alert::CheckMemoryWarning(const double& usage_percent) {
		bool bwarning = false;
		if (memory_state_["state"].empty() || memory_state_["state"].asUInt() == General::NOWARNING) {
			if (usage_percent >= memory_criticality_) {
				memory_state_["value"] = usage_percent;
				memory_state_["state"] = General::WARNING;
				bwarning = true;
			}
		}
		else if (memory_state_["state"].empty() || memory_state_["state"].asUInt() == General::WARNING) {
			if (usage_percent < memory_criticality_) {
				memory_state_["value"] = usage_percent;
				memory_state_["state"] = General::NOWARNING;
				bwarning = true;
			}
		}
		return bwarning;
	}


	bool Alert::CheckDiskWarning(const double& usage_percent) {
		bool bwarning = false;
		if (disk_state_["state"].empty() || disk_state_["state"].asUInt() == General::NOWARNING) {
			if (usage_percent >= disk_criticality_) {
				disk_state_["value"] = usage_percent;
				disk_state_["state"] = General::WARNING;
				bwarning = true;
			}
		}
		else if (disk_state_["state"].empty() || disk_state_["state"].asUInt() == General::WARNING) {
			if (usage_percent < disk_criticality_) {
				disk_state_["value"] = usage_percent;
				disk_state_["state"] = General::NOWARNING;
				bwarning = true;
			}
		}
		return bwarning;
	}

	bool Alert::CheckAlert(Json::Value& items) {
		bool bwarning = false;
		int64_t high_resolution_time = utils::Timestamp::HighResolution();
		if (high_resolution_time - alert_interval_ > alert_last_time_) {
			do {
				Json::Value message;
				uint64_t ledger_sequence = 0;
				bool sync_completed = false;
				double cpu_used_percent = 0;
				double memory_used_percent = 0;
				double disk_used_percent = 0;
				std::string buffer;
				{
					utils::MutexGuard guard_buffer(mutex_buffer_);
					buffer = buffer_;
				}
				if (buffer.length() > 0) {
					if (!message.fromString(buffer)) {
						LOG_ERROR("Alert on_alert_method : msg's format is invalid");
						break;
					}
					if (message["id"].empty()) {
						LOG_ERROR("Alert on_alert_method : bubi version is low");
						break;
					}
				{
					utils::MutexGuard guard_id(mutex_id_);
					id_ = message["id"].asString();
				}

				if (message["ledger_sequence"].empty()) {
					LOG_ERROR("Alert on_alert_method : bubi version is low");
					break;
				}
				sync_completed = message["sync_completed"].asBool();
				ledger_sequence = message["ledger_sequence"].asUInt64();
				cpu_used_percent = message["cpu"]["usedPercent"].asDouble();
				memory_used_percent = message["memory"]["usedPercent"].asDouble();
				disk_used_percent = message["disk"]["usedPercent"].asDouble();
				}

				if (CheckBubiState()) {
					items["bubi_crack"] = bubi_state_;
					bwarning = true;
				}

				if (CheckConsensusState(ledger_sequence) && sync_completed) {
					items["consensus"] = consensus_state_;
					bwarning = true;
				}

				if (CheckCpuWarning(cpu_used_percent)) {
					items["cpu"] = cpu_state_;
					bwarning = true;
				}

				if (CheckMemoryWarning(memory_used_percent)) {
					LOG_INFO("memory General::WARNING  total=%lld, avail=%lld", message["memory"]["total"].asUInt64(), message["memory"]["available"].asUInt64());
					items["memory"] = memory_state_;
					bwarning = true;
				}

				if (CheckDiskWarning(disk_used_percent)) {
					LOG_INFO("disk General::WARNING  total=%lld, avail=%lld", message["disk"]["total"].asUInt64(), message["disk"]["available"].asUInt64());
					items["disk"] = disk_state_;
					bwarning = true;
				}

			} while (false);
			alert_last_time_ = high_resolution_time;
		}

		return bwarning;
	}

}