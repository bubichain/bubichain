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

#include "system.h"
#include "file.h"
#include "strings.h"

#include <sys/stat.h>

utils::PhysicalMemory::PhysicalMemory() {
	total_bytes_ = 0;
	free_bytes_ = 0;
	buffers_bytes_ = 0;
	cached_bytes_ = 0;
	available_bytes_ = 0;
	usage_percent_ = 0;
}

utils::PhysicalDisk::PhysicalDisk() {
	total_bytes_ = 0;
	free_bytes_ = 0;
	available_bytes_ = 0;
	usage_percent_ = 0;
}

utils::PhysicalHDD::PhysicalHDD() {
	total_bytes_ = 0;
}

utils::PhysicalPartition::PhysicalPartition() {
	usage_percent_ = 0;
	total_bytes_ = free_bytes_ = available_bytes_ = 0;
}

utils::SystemProcessor::SystemProcessor() {
	core_count_ = 0;
	cpu_type_ = "";
	user_time_ = 0;
	nice_time_ = 0;
	system_time_ = 0;
	idle_time_ = 0;
	io_wait_time_ = 0;
	irq_time_ = 0;
	soft_irq_time_ = 0;
	usage_percent_ = 0;
}
utils::SystemProcessor::~SystemProcessor() {

}

uint64_t utils::SystemProcessor::GetTotalTime() {
#ifdef WIN32
	return user_time_ + system_time_;
#else
	return user_time_ + nice_time_ + system_time_ + idle_time_ + io_wait_time_ + irq_time_ + soft_irq_time_;
#endif
}
uint64_t utils::SystemProcessor::GetUsageTime() {
#ifdef WIN32
	return user_time_ + system_time_ - idle_time_;
#else
	return user_time_ + nice_time_ + system_time_;
#endif
}

utils::System::System(bool with_processors) {
	with_processors_ = with_processors;
	processor_list_ = NULL;

#if WIN32
	if (with_processors) {
		HMODULE hmodule = LoadLibrary("Ntdll.dll");
		pfn_nt_query_system_information_ = NULL;
		if (hmodule != NULL) {
			pfn_nt_query_system_information_ = (PROCNTQSI)GetProcAddress(hmodule, "NtQuerySystemInformation");
		}
	}
#endif
}

utils::System::~System() {
	delete processor_list_;
}

const utils::SystemProcessor &utils::System::GetProcessor() const { 
	return processor_;
};

bool utils::System::UpdateProcessor() {
	SystemProcessor nold_processer = processor_;
	SystemProcessorVector old_process_list;

	if (with_processors_) {
		if (NULL != processor_list_) {
			old_process_list.assign(processor_list_->begin(), processor_list_->end());
			processor_list_->clear();
		}
		else {
			processor_list_ = new SystemProcessorVector();
		}
	}

#ifdef WIN32
	//get cpu information
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);

	processor_.core_count_ = system_info.dwNumberOfProcessors;
	switch (system_info.wProcessorArchitecture) {
	case PROCESSOR_ARCHITECTURE_ALPHA:
		processor_.cpu_type_ = "Intel";
		break;
	case PROCESSOR_ARCHITECTURE_MIPS:
		processor_.cpu_type_ = "MIPS";
		break;
	case PROCESSOR_ARCHITECTURE_ARM:
		processor_.cpu_type_ = "ARM";
		break;
	case PROCESSOR_ARCHITECTURE_AMD64:
		processor_.cpu_type_ = "AMD64";
	}


	FILETIME idle_time_, nkernel_time, user_time;
	if (!GetSystemTimes(&idle_time_, &nkernel_time, &user_time))
		return false;

	processor_.user_time_ = (int64_t)((((uint64_t)user_time.dwHighDateTime) << 32) + (uint64_t)user_time.dwLowDateTime);
	processor_.system_time_ = (int64_t)((((uint64_t)nkernel_time.dwHighDateTime) << 32) + (uint64_t)nkernel_time.dwLowDateTime);
	processor_.idle_time_ = (int64_t)((((uint64_t)idle_time_.dwHighDateTime) << 32) + (uint64_t)idle_time_.dwLowDateTime);
	processor_.nice_time_ = 0;
	processor_.io_wait_time_ = 0;
	processor_.irq_time_ = 0;
	processor_.soft_irq_time_ = 0;

	if (with_processors_ && NULL != pfn_nt_query_system_information_) {
		ULONG uOutLength = 0;
		ULONG uInBufferLength = sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)* processor_.core_count_;

		unsigned char *pBuffer = (unsigned char*)malloc(uInBufferLength);
		DWORD dwCode = (pfn_nt_query_system_information_)(SystemProcessorPerformanceInformation, pBuffer, uInBufferLength, &uOutLength);
		if (dwCode == 0) {
			SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *pInfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*)(pBuffer);
			for (size_t n = 0; n < processor_.core_count_; ++n) {
				SystemProcessor processorItem;
				processorItem.idle_time_ = pInfo->IdleTime.QuadPart;
				processorItem.user_time_ = pInfo->UserTime.QuadPart;
				processorItem.system_time_ = pInfo->KernelTime.QuadPart;
				++pInfo;
			}
		}
		free(pBuffer);
	}
#else
	utils::File proce_file;

	if (!proce_file.Open("/proc/stat", utils::File::FILE_M_READ))
		return false;

	std::string strline;
	utils::StringVector values;

	if (!proce_file.ReadLine(strline, 1024)) {
		proce_file.Close();
		return false;
	}

	values = utils::String::split(strline, " ");
	if (values.size() < 8) {
		proce_file.Close();
		return false;
	}

	processor_.user_time_ = utils::String::Stoi64(values[1]);
	processor_.nice_time_ = utils::String::Stoi64(values[2]);
	processor_.system_time_ = utils::String::Stoi64(values[2]);
	processor_.idle_time_ = utils::String::Stoi64(values[2]);
	processor_.io_wait_time_ = utils::String::Stoi64(values[2]);
	processor_.irq_time_ = utils::String::Stoi64(values[2]);
	processor_.soft_irq_time_ = utils::String::Stoi64(values[2]);
	processor_.core_count_ = 0;

	while (proce_file.ReadLine(strline, 1024)) {
		values = utils::String::split(strline, " ");
		if (values.size() < 8)
			break;
		if (std::string::npos == values[0].find("cpu"))
			break;

		processor_.core_count_++;
	}
	proce_file.Close();
#endif
	if (nold_processer.system_time_ > 0) {
		int64_t totalTime1 = nold_processer.GetTotalTime();
		int64_t usageTime1 = nold_processer.GetUsageTime();
		int64_t totalTime2 = processor_.GetTotalTime();
		int64_t usageTime2 = processor_.GetUsageTime();
		if (totalTime2 > totalTime1 && usageTime2 > usageTime1) {
			processor_.usage_percent_ = double(usageTime2 - usageTime1) / double(totalTime2 - totalTime1)*100.0;
		}
		else {
			processor_.usage_percent_ = 0;
		}
	}
	else {
		processor_.usage_percent_ = double(processor_.GetUsageTime()) / double(processor_.GetTotalTime()) * 100;
	}

	return true;
}

bool utils::System::GetPhysicalDisk(const std::string &path, utils::PhysicalDisk &disk) {
#ifdef WIN32
	ULARGE_INTEGER available, total, free;
	if (!GetDiskFreeSpaceExA(path.c_str(), &available, &total, &free)) {
		return false;
	}

	disk.total_bytes_ = total.QuadPart;
	disk.free_bytes_ = free.QuadPart;
	disk.available_bytes_ = available.QuadPart;
#else
	struct statfs ndisk_stat;

	if (statfs(path.c_str(), &ndisk_stat) != 0) {
		return false;
	}

	disk.total_bytes_ = (uint64_t)(ndisk_stat.f_blocks) * (uint64_t)(ndisk_stat.f_frsize);
	disk.available_bytes_ = (uint64_t)(ndisk_stat.f_bavail) * (uint64_t)(ndisk_stat.f_bsize);
	// default as root
	disk.free_bytes_ = disk.available_bytes_;
#endif
	if (disk.total_bytes_ > disk.free_bytes_) {
		disk.usage_percent_ = double(disk.total_bytes_ - disk.free_bytes_) / double(disk.total_bytes_) * 100.0;
	}

	return true;
}

bool utils::System::GetPhysicalMemory(utils::PhysicalMemory &memory) {
#ifdef WIN32
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(MEMORYSTATUSEX);

	::GlobalMemoryStatusEx(&status);

	memory.total_bytes_ = status.ullTotalPhys;
	memory.free_bytes_ = status.ullAvailPhys;
	memory.available_bytes_ = status.ullTotalPhys - status.ullAvailPhys;
	memory.cached_bytes_ = 0;
	memory.buffers_bytes_ = 0;
#else
	utils::File proc_file;

	if (!proc_file.Open("/proc/meminfo", utils::File::FILE_M_READ)) {
		return false;
	}

	std::string strline;
	while (proc_file.ReadLine(strline, 1024)) {
		utils::StringVector values;
		values = utils::String::split(strline, ":");
		utils::String::Trim(values[0]);
		utils::String::Trim(values[1]);
		const std::string &strkey = values[0];
		const std::string &strvalue = values[1];
		if (strkey.compare("MemTotal") == 0) {
			memory.total_bytes_ = utils::String::Stoui64(values[1]) * 1024;
		}
		else if (strkey.compare("MemFree") == 0) {
			memory.free_bytes_ = utils::String::Stoui64(values[1]) * 1024;
		}
		else if (strkey.compare("Buffers") == 0) {
			memory.buffers_bytes_ = utils::String::Stoui64(values[1]) * 1024;
		}
		else if (strkey.compare("Cached") == 0) {
			memory.cached_bytes_ = utils::String::Stoui64(values[1]) * 1024;
		}
	}
	proc_file.Close();

	memory.available_bytes_ = memory.free_bytes_ + memory.buffers_bytes_ + memory.cached_bytes_;
#endif
	if (memory.total_bytes_ > memory.available_bytes_) {
		memory.usage_percent_ = double(memory.total_bytes_ - memory.available_bytes_) / double(memory.total_bytes_) * 100.0;
	}

	return true;
}

time_t utils::System::GetStartupTime(time_t time_now) {
	time_t startup_time = 0;
	if (0 == time_now) {
		time_now = time(NULL);
	}
#ifdef WIN32
	LARGE_INTEGER count, freq;

	if (!QueryPerformanceCounter(&count) || !QueryPerformanceFrequency(&freq) || 0 == freq.QuadPart) {
		return 0;
	}
	startup_time = time_now - (time_t)(count.QuadPart / freq.QuadPart);
#else
	struct sysinfo info;

	memset(&info, 0, sizeof(info));
	sysinfo(&info);
	startup_time = time_now - (time_t)info.uptime;
#endif
	return startup_time;

}

std::string utils::System::GetHostName() {
	char host_name[128];
	if (gethostname(host_name, 128) != 0)
		host_name[0] = '\0';
	return std::string(host_name);
}


std::string utils::System::GetOsVersion() {
	std::string os_version;
#ifdef WIN32
	OSVERSIONINFOEX osvi;
	BOOL os_version_info_ex = false;
	const DWORD product_buffer_size = 1024;

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!(os_version_info_ex = GetVersionEx((OSVERSIONINFO*)&osvi))) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO*)&osvi)) {
			return os_version;
		}
	}

	switch (osvi.dwPlatformId) {
		// Test for the Windows NT product family.
	case VER_PLATFORM_WIN32_NT:

		// Test for the specific product.
		if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0 && osvi.wProductType == VER_NT_WORKSTATION)
			os_version = "Windows 10 ";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3) {
			if (osvi.wProductType == VER_NT_WORKSTATION)
				os_version = "Windows 8.1 ";
			else
				os_version = "Windows Server 2012 r2 ";
		}
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2) {
			if (osvi.wProductType == VER_NT_WORKSTATION)
				os_version = "Windows 8 ";
			else
				os_version = "Windows Server 2012 ";
		}
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
			if (osvi.wProductType == VER_NT_WORKSTATION)
				os_version = "Windows 7 ";
			else
				os_version = "Windows Server 2008 R2 ";
		}
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
			if (osvi.wProductType == VER_NT_WORKSTATION)
				os_version = "Windows Vista ";
			else
				os_version = "Windows Server 2008 ";
		}
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
			if (osvi.wProductType == VER_NT_WORKSTATION && info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				os_version = "Windows xp Pro x64 Edition ";
			else if (GetSystemMetrics(SM_SERVERR2) == 0)
				os_version = "Windows Server 2003 ";
			else if (GetSystemMetrics(SM_SERVERR2) != 0)
				os_version = "Windows Server 2003 R2 ";
		}
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
			if (osvi.wSuiteMask & VER_SUITE_EMBEDDEDNT)
				os_version = "Windows XP Embedded ";
			else
				os_version = "Windows XP ";
		}

		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			os_version = "Windows 2000 ";
		else
			os_version = utils::String::Format("Windows NT(%u.%u)", osvi.dwMajorVersion, osvi.dwMinorVersion);

		// Test for specific product on Windows NT 4.0 SP6 and later.
		if (os_version_info_ex) {
			// Test for the workstation type.
			switch (osvi.dwMajorVersion) {
			case 4:
				if (osvi.wProductType == VER_NT_WORKSTATION)
					os_version += "Workstation 4.0 ";
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					os_version += "Server 4.0 Enterprise Edition ";
				else
					os_version += "Server 4.0 ";
			case 5:
				switch (osvi.dwMinorVersion) {
				case 0:                  //Windows 2000 
					if (osvi.wSuiteMask == VER_SUITE_ENTERPRISE)
						os_version += "Advanced Server ";
					break;
				case 1:                  //Windows XP 
					if (osvi.wSuiteMask == VER_SUITE_EMBEDDEDNT)
						os_version += "Embedded ";
					else if (osvi.wSuiteMask == VER_SUITE_PERSONAL)
						os_version += "Home Edition ";
					else
						os_version += "Professional ";
					break;
				case 2:
					if (GetSystemMetrics(SM_SERVERR2) == 0 && osvi.wSuiteMask == VER_SUITE_BLADE)  //Windows Server 2003 
						os_version += "Web Edition ";
					else if (GetSystemMetrics(SM_SERVERR2) == 0 && osvi.wSuiteMask == VER_SUITE_COMPUTE_SERVER)
						os_version += "Compute Cluster Edition ";
					else if (GetSystemMetrics(SM_SERVERR2) == 0 && osvi.wSuiteMask == VER_SUITE_STORAGE_SERVER)
						os_version += "Storage Server ";
					else if (GetSystemMetrics(SM_SERVERR2) == 0 && osvi.wSuiteMask == VER_SUITE_DATACENTER)
						os_version += "DataCenter Edition ";
					else if (GetSystemMetrics(SM_SERVERR2) == 0 && osvi.wSuiteMask == VER_SUITE_ENTERPRISE)
						os_version += "Enterprise Edition ";
					else if (GetSystemMetrics(SM_SERVERR2) != 0 && osvi.wSuiteMask == VER_SUITE_STORAGE_SERVER)  // Windows Server 2003 R2
						os_version += "Storage Server ";
					break;
				}
				break;
			case 6:
				if (osvi.wProductType != VER_NT_WORKSTATION && osvi.wSuiteMask == VER_SUITE_DATACENTER)
					os_version += "DataCenter Server ";
				else if (osvi.wProductType != VER_NT_WORKSTATION && osvi.wSuiteMask == VER_SUITE_ENTERPRISE)
					os_version += "Enterprise ";
				else if (osvi.wProductType == VER_NT_WORKSTATION && osvi.wSuiteMask != VER_SUITE_ENTERPRISE)
					os_version += "Home Edition ";
				else if (osvi.wProductType == VER_NT_WORKSTATION && osvi.wSuiteMask == VER_SUITE_ENTERPRISE)
					os_version += "Enterprise ";
				break;
			case 10:
				if (osvi.wProductType == VER_NT_WORKSTATION && osvi.wSuiteMask == VER_SUITE_ENTERPRISE)
					os_version += "Enterprise ";
				else if (osvi.wProductType == VER_NT_WORKSTATION && osvi.wSuiteMask != VER_SUITE_ENTERPRISE)
					os_version += "Home Edition ";
				break;
			default:
				os_version += "";
			}
		}
		// Test for specific product on Windows NT 4.0 SP5 and earlier
		else {
			HKEY hkey;
			CHAR product_type[product_buffer_size];
			DWORD dwbuf_len = product_buffer_size;
			LONG lRet = 0;

			memset(product_type, 0, sizeof(product_type));

			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
				0, KEY_QUERY_VALUE, &hkey)) {
				if (ERROR_SUCCESS == RegQueryValueExA(hkey, "ProductType", NULL, NULL,
					(LPBYTE)product_type, &dwbuf_len)) {
					if (stricmp("WINNT", product_type) == 0)
						os_version += "Workstation ";
					if (stricmp("LANMANNT", product_type) == 0)
						os_version += "Server ";
					if (stricmp("SERVERNT", product_type) == 0)
						os_version += "Advanced Server ";
				}
				RegCloseKey(hkey);
			}

			utils::String::AppendFormat(os_version, "%u.%u", osvi.dwMajorVersion, osvi.dwMinorVersion);
		}

		// Display service pack (if any) and build number.
		if (osvi.dwMajorVersion == 4 &&
			stricmp(osvi.szCSDVersion, "Service Pack 6") == 0) {
			HKEY hkey = NULL;
			LONG lRet = 0;

			// Test for SP6 versus SP6a.
			if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,
				"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
				0, KEY_QUERY_VALUE, &hkey)) {
				utils::String::AppendFormat(os_version, "Service Pack 6a (Build %d)", osvi.dwBuildNumber & 0xFFFF);
				RegCloseKey(hkey);
			}
			else {// Windows NT 4.0 prior to SP6a
				utils::String::AppendFormat(os_version, "%s (Build %d)",
					osvi.szCSDVersion,
					osvi.dwBuildNumber & 0xFFFF);
			}
		}
		else {// not Windows NT 4.0 
			utils::String::AppendFormat(os_version, "%s (Build %d)",
				osvi.szCSDVersion,
				osvi.dwBuildNumber & 0xFFFF);
		}
		break;
	}
#else
	struct utsname unix_name;

	memset(&unix_name, 0, sizeof(unix_name));
	if (uname(&unix_name) != 0) {
		os_version = "Unknown";
	}
	else {
		os_version = utils::String::Format("%s %s %s %s",
			unix_name.sysname,
			unix_name.release,
			unix_name.machine,
			unix_name.version);
	}
#endif
	return os_version;
}

std::string utils::System::GetOsBits() {
	std::string os_bit = "";

#ifdef WIN32
	BOOL bis_wow64 = FALSE;

	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fn_is_wow64_process;

	fn_is_wow64_process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle("kernel32"), "IsWow64Process");
	if (NULL != fn_is_wow64_process) {
		fn_is_wow64_process(GetCurrentProcess(), &bis_wow64);
	}

	if (bis_wow64) {
		os_bit = "64";
	}
	else {
		os_bit = "32";
	}
#else
	FILE *fstream = NULL;
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	if (NULL == (fstream = popen("getconf LONG_BIT", "r"))) {
		os_bit = "error";
		return os_bit;
	}
	if (NULL != fgets(buff, sizeof(buff), fstream)) {
		buff[2] = '\0';
		os_bit = buff;
	}
	else {
		os_bit = "error";
	}
	pclose(fstream);

#endif
	return os_bit;
}

uint64_t utils::System::GetLogsSize(const std::string path) {

	uint64_t log_size = 0;

	std::string out_file_name = path + "-out";
	std::string err_file_name = path + "-err";

	std::string file_ext = utils::File::GetExtension(path);
	if (file_ext.size() > 0 && (file_ext.size() + 1) < path.size()) {
		std::string sub_path = path.substr(0, path.size() - file_ext.size() - 1);

		out_file_name = utils::String::Format("%s-out.%s", sub_path.c_str(), file_ext.c_str());
		err_file_name = utils::String::Format("%s-err.%s", sub_path.c_str(), file_ext.c_str());
	}

	log_size = GetLogSize(out_file_name.c_str());
	log_size += GetLogSize(err_file_name.c_str());

	return log_size;
}

uint64_t utils::System::GetLogSize(const char* path) {

	uint64_t log_size = 0;

	struct stat buff;
	if (stat(path, &buff) < 0) {
		return log_size;
	}
	else {
		log_size = buff.st_size;
	}

	return log_size;
}