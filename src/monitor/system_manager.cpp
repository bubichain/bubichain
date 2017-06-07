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
/* Function: get system or bubi info                                    */
/************************************************************************/

#include <utils/headers.h>
#include <json/json.h>
#include "system_manager.h"

namespace bubi {
	SystemManager::SystemManager()
		: system_(true) {
		cpu_used_percent_ = 0.0;
		check_interval_ = 5 * utils::MICRO_UNITS_PER_SEC;
		last_check_time_ = utils::Timestamp::HighResolution();
	}

	SystemManager::~SystemManager() {

	}

	void SystemManager::OnSlowTimer() {
		int64_t high_resolution_time = utils::Timestamp::HighResolution();
		if (high_resolution_time - last_check_time_ > check_interval_) {
			if (!system_.UpdateProcessor()) {
				return;
			}
			last_check_time_ = high_resolution_time;
		}
		const utils::SystemProcessor processor = system_.GetProcessor();
		cpu_used_percent_ = processor.usage_percent_;
	}

	bool SystemManager::GetSystemMonitor(const std::string path, Json::Value &data) {

		Json::Value& result = data["result"];

		Json::Value& property = result["property"];
		// property
		property["host_name"] = system_.GetHostName();
		property["os_version"] = system_.GetOsVersion();
		property["startup_time"] = system_.GetStartupTime(0);
		property["os_bit"] = system_.GetOsBits();
		property["log_size"] = system_.GetLogsSize(path);

		Json::Value& memory = result["memory"];

		// memory info
		utils::PhysicalMemory physical_memory;
		if (false == system_.GetPhysicalMemory(physical_memory)) {
			LOG_STD_ERR("Common::SystemManager , Get physical memory status failed");
		}
		else {
			memory["available"] = physical_memory.available_bytes_;
			memory["total"] = physical_memory.total_bytes_;
			memory["usedPercent"] = physical_memory.usage_percent_;
		}

		Json::Value& cpu = result["cpu"];

		// cpu info
		if (cpu_used_percent_ > 1e-6) {
			cpu["usedPercent"] = cpu_used_percent_;
		}

		Json::Value& disk = result["disk"];

		// disk info
		utils::PhysicalDisk physical_disk;
		if (false == system_.GetPhysicalDisk(utils::File::GetBinHome(), physical_disk)) {
			LOG_STD_ERR("Common::SystemManager , Get physical disk(/) status failed");
		}
		else {
			disk["available"] = physical_disk.available_bytes_;
			disk["total"] = physical_disk.total_bytes_;
			disk["usedPercent"] = physical_disk.usage_percent_;
		}

		return true;
	}
}