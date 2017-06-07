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

#ifndef UTILS_SYSTEM_H_
#define UTILS_SYSTEM_H_

#include "utils.h"
#ifdef WIN32
#include <tlhelp32.h>
#include <psapi.h>
#include <Winternl.h>
#include <Winnls.h>
#include <windows.h> 
#include <lm.h>
#else
#include <sys/statvfs.h>
#include <sys/statfs.h>
#include <sys/utsname.h>
#include <shadow.h>
#endif

namespace utils {
	class PhysicalMemory {
	public:
		PhysicalMemory();
	public:
		uint64_t total_bytes_;
		uint64_t free_bytes_;
		uint64_t buffers_bytes_;
		uint64_t cached_bytes_;
		uint64_t available_bytes_;
		double usage_percent_;
	};
	class PhysicalDisk {
	public:
		PhysicalDisk();
	public:
		uint64_t total_bytes_;
		uint64_t free_bytes_;
		uint64_t available_bytes_;
		double usage_percent_;
	};
	class  PhysicalHDD {
	public:
		PhysicalHDD();
	public:
		uint64_t total_bytes_;
		std::string describe_;
	};
	typedef std::vector<PhysicalHDD> PhysicalHDDVector;
	class PhysicalPartition {
	public:
		PhysicalPartition();
	public:
		uint64_t total_bytes_;
		uint64_t free_bytes_;
		uint64_t available_bytes_;
		double usage_percent_;
		std::string describe_;
	};
	typedef std::vector<PhysicalPartition> PhysicalPartitionVector;
	class SystemProcessor {
	public:
		SystemProcessor();
		~SystemProcessor();
		uint64_t GetTotalTime();
		uint64_t GetUsageTime();
	public:
		size_t core_count_;
		std::string cpu_type_;
		int64_t user_time_;
		int64_t nice_time_;
		int64_t system_time_;
		int64_t idle_time_;
		int64_t io_wait_time_;
		int64_t irq_time_;
		int64_t soft_irq_time_;
		double  usage_percent_;
	};
	class System {
	public:
		System(bool with_processors);
		virtual ~System();
		bool UpdateProcessor();
		const SystemProcessor &GetProcessor() const;
		static bool	GetPhysicalDisk(const std::string &str_path, utils::PhysicalDisk &disk);
		static bool	GetPhysicalMemory(utils::PhysicalMemory &memory);
		static time_t GetStartupTime(time_t time_now);
		static std::string GetHostName();
		static std::string GetOsVersion();
		static std::string GetOsBits();
		static uint64_t GetLogsSize(const std::string path);
	private:
		static uint64_t GetLogSize(const char* path);
	protected:
		typedef std::vector<SystemProcessor> SystemProcessorVector;
	protected:
		SystemProcessor  processor_;
		SystemProcessorVector *processor_list_;
	private:
		bool with_processors_;
#ifdef WIN32
		typedef NTSTATUS(WINAPI *PROCNTQSI)(UINT, PVOID, ULONG, PULONG);
		PROCNTQSI pfn_nt_query_system_information_;
#endif
	};
}
#endif