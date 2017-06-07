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

#include "utils.h"
#include "file.h"
#include "strings.h"
#include "base_int.h"

uint32_t utils::error_code() {
#ifdef WIN32
	return (uint32_t)GetLastError();
#else
	return (uint32_t)errno;
#endif //WIN32
}

void utils::set_error_code(uint32_t code) {
#ifdef WIN32
	SetLastError(code);
#else
	errno = code;
#endif //WIN32
}

void utils::Sleep(int time_milli) {
#ifdef WIN32
	::Sleep(time_milli);
#else
	::usleep(((__useconds_t)time_milli) * 1000);
#endif //WIN32
}

std::string utils::error_desc(uint32_t code) {
	uint32_t real_code = (((uint32_t)-1) == code) ? error_code() : code;
#ifdef WIN32
	LPVOID msg_buffer = NULL;
	if (!FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		real_code,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // Default language
		(LPSTR)&msg_buffer,
		0,
		NULL)) {
		// Handle the error.
		return std::string("");
	}

	// trim end blank characters
	char *msg_data = (char *)msg_buffer;
	int msg_len = (int)strlen(msg_data);
	while (msg_len > 0 && isspace(msg_data[msg_len - 1])) {
		msg_data[msg_len - 1] = 0;
		msg_len--;
	}

	std::string strError((const char *)msg_buffer);

	LocalFree(msg_buffer);

	return strError;

#else
	return std::string(strerror(real_code));
#endif
}

size_t utils::GetCpuCoreCount() {
	size_t core_count = 1;
#if defined(WIN32)
	SYSTEM_INFO nSystemInfo;
	GetSystemInfo(&nSystemInfo);
	core_count = nSystemInfo.dwNumberOfProcessors;
#else
	do {
		utils::File nProcFile;

		if (!nProcFile.Open("/proc/stat", utils::File::FILE_M_READ)) {
			break;
		}

		std::string strLine;

		if (!nProcFile.ReadLine(strLine, 1024)) {
			nProcFile.Close();
			break;
		}
		utils::StringVector nValues = utils::String::Strtok(strLine, ' ');
		if (nValues.size() < 8) {
			break;
		}

		core_count = nValues.size();
	} while (false);
#endif
	return core_count;
}

time_t utils::GetStartupTime(time_t time_now) {
	time_t nStartupTime = 0;

	if (0 == time_now) {
		time_now = time(NULL);
	}

#if defined(WIN32)
	LARGE_INTEGER nCount, nFreq;

	if (!QueryPerformanceCounter(&nCount) || !QueryPerformanceFrequency(&nFreq) || 0 == nFreq.QuadPart) {
		return 0;
	}

	nStartupTime = time_now - (time_t)(nCount.QuadPart / nFreq.QuadPart);
#else
	struct sysinfo nInfo;

	memset(&nInfo, 0, sizeof(nInfo));
	sysinfo(&nInfo);
	nStartupTime = time_now - (time_t)nInfo.uptime;
#endif

	return nStartupTime;
}

utils::BubiAtExit::BubiAtExit() {
}

utils::BubiAtExit::~BubiAtExit() {
	while (!s_.empty()) {
		s_.top()();
		s_.pop();
	}
}

void utils::BubiAtExit::Push(ExitHandler fun) { 
	s_.push(fun);
}