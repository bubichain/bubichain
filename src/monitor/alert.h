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
#ifndef ALERT_H_
#define  ALERT_H_

#include <utils/singleton.h>
#include <json/json.h>
#include <common/general.h>

namespace bubi {
	class Alert {
	public:
		Alert();
		~Alert();
		bool Initialize();
		bool Exit();
		bool CheckAlert(Json::Value& items);
		void ResetState();
		std::string GetId();
		bool GetBubiCrackState();
		void SetBuffer(const std::string buffer);
		void SetCpuCriticality(const double cpu_criticality);
		void SetMemoryCriticality(const double memory_criticality);
		void SetDiskCriticality(const double disk_criticality);
		void SetConsensusTime(const uint64_t consensus_time);
		void SetBubiCrackTime(const uint64_t bubi_crack_time);
		void SetBubiLastTime(uint64_t bubi_last_time);
	private:
		bool CheckBubiState();
		bool CheckConsensusState(const uint64_t& ledger_sequence);
		bool CheckCpuWarning(const double& cpu_used_percent);
		bool CheckMemoryWarning(const double& usage_percent);
		bool CheckDiskWarning(const double& usage_percent);
	private:
		std::string id_;
		utils::Mutex mutex_id_;
		std::string buffer_;
		utils::Mutex mutex_buffer_;
		double cpu_criticality_;						// cpu's threshold
		double memory_criticality_;					// memory's threshold
		double disk_criticality_;						// disk's threshold
		uint64_t consensus_time_;						// consensus's threshold
		uint64_t bubi_crack_time_;						// bubi_crack's threshold
		uint64_t bubi_last_time_;
		uint64_t consensus_last_time_;
		uint64_t alert_interval_;
		uint64_t alert_last_time_;
		Json::Value consensus_state_;
		Json::Value bubi_state_;
		Json::Value cpu_state_;
		Json::Value memory_state_;
		Json::Value disk_state_;
	};
}

#endif
