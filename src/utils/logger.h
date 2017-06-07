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

#ifndef UTILS_LOGGER_H_
#define UTILS_LOGGER_H_

#include "common.h"
#include "singleton.h"
#include "thread.h"

#define LOG_TRACE(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_TRACE,__FILE__,__func__, __LINE__ , fmt , ## __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_DEBUG,__FILE__,__func__, __LINE__ , fmt , ## __VA_ARGS__)
#define LOG_INFO(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_INFO,__FILE__,__func__, __LINE__ , fmt , ## __VA_ARGS__)
#define LOG_WARN(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_WARN,__FILE__,__func__, __LINE__ , fmt , ## __VA_ARGS__)
#define LOG_ERROR(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_ERROR,__FILE__,__func__, __LINE__ , fmt , ## __VA_ARGS__)
#define LOG_FATAL(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_FATAL,__FILE__,__func__, __LINE__ , fmt , ## __VA_ARGS__)
#define LOG_TRACE_ERRNO(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_TRACE,__FILE__,__func__, __LINE__ , fmt" (%u:%s)" , ## __VA_ARGS__)
#define LOG_DEBUG_ERRNO(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_DEBUG,__FILE__,__func__, __LINE__ , fmt" (%u:%s)" , ## __VA_ARGS__)
#define LOG_INFO_ERRNO(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_INFO,__FILE__,__func__, __LINE__ , fmt" (%u:%s)" , ## __VA_ARGS__)
#define LOG_WARN_ERRNO(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_WARN,__FILE__,__func__, __LINE__ , fmt" (%u:%s)" , ## __VA_ARGS__)
#define LOG_ERROR_ERRNO(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_ERROR,__FILE__,__func__, __LINE__ , fmt" (%u:%s)" , ## __VA_ARGS__)
#define LOG_FATAL_ERRNO(fmt, ...) utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_FATAL,__FILE__,__func__, __LINE__ , fmt" (%u:%s)" , ## __VA_ARGS__)
#define LOG_STD_ERR(fmt, ...) utils::Logger::Instance().LogStubVmError(__FILE__,__func__, __LINE__ , fmt , ## __VA_ARGS__)
#define LOG_STD_ERRNO(fmt, ...) utils::Logger::Instance().LogStubVmError(__FILE__,__func__, __LINE__ , fmt" (%u:%s)" , ## __VA_ARGS__)
#define STD_ERR_CODE utils::error_code() 
#define STD_ERR_DESC utils::error_desc().c_str() 
#define BUBI_EXIT(fmt, ...) { utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_ERROR,__FILE__,__func__, __LINE__ , fmt , ## __VA_ARGS__); exit(-1); }
#define BUBI_EXIT_ERRNO(fmt, ...) { utils::Logger::Instance().LogStubVm(utils::LOG_LEVEL_ERROR,__FILE__,__func__, __LINE__ , fmt" (%u:%s)" , ## __VA_ARGS__); exit(-1); }

namespace utils {
	typedef enum tagLogDest {
		/** Indicates that logging should be done to file. */
		LOG_DEST_NONE = 0x00,
		LOG_DEST_FILE = 0x01,
		/** Log to Console. (stdout) */
		LOG_DEST_OUT = 0x02,
		LOG_DEST_ERR = 0x04,
		LOG_DEST_ALL = 0xFF,
		LOG_DEST_FILE_OUT_ID = 0,
		LOG_DEST_FILE_ERR_ID = 1,
		LOG_DEST_OUT_ID = 2,
		LOG_DEST_ERR_ID = 3,
		LOG_DEST_COUNT = 4,
	}LogDest;
	typedef enum tagLogLevel {
		LOG_LEVEL_NONE = 0x00,
		LOG_LEVEL_TRACE = 0x01,
		LOG_LEVEL_DEBUG = 0x02,
		LOG_LEVEL_INFO = 0x04,
		LOG_LEVEL_WARN = 0x08,
		LOG_LEVEL_ERROR = 0x10,
		LOG_LEVEL_FATAL = 0x20,
		LOG_LEVEL_ALL = 0xFF
	}LogLevel;
	class Logger;
	class LogWriter {
	public:
		LogWriter();
		~LogWriter();
		bool Init(LogDest dest, const std::string &file_name, bool open_mode);
		bool Write(Logger *logger, const LogLevel logLevel,
			const char *current_time,
			const char* file, const char* funcName, const int lineNum,
			const char* fmt, va_list ap);
		bool Close();
		LogDest log_dest();
		static std::string GetLogPrefix(const LogLevel logLevel);
	private:
		LogDest dest_;
		FILE *file_ptr_;
		uint64_t size_;
		time_t begin_time_;
		std::string file_name_;
	};
	class Logger : public Singleton<Logger> {
		friend class Singleton<Logger>;
		friend class LogWriter;
	private:
		Logger();
		~Logger();
	public:
		bool Initialize(utils::LogDest log_dest, utils::LogLevel log_level, const std::string &file_name, bool open_mode);
		bool Exit();
		int LogStubVm(LogLevel logLevel,
			const char* file,
			const char* funcName, const int lineNum,
			const char* fmt, ...);
		int LogStubVmError(
			const char* file,
			const char* funcName, const int lineNum,
			const char* fmt, ...);
		int LogStub(utils::LogLevel log_Level,
			const char* file,
			const char* funcName, const int lineNum,
			const char* fmt, va_list ap);
		void SetCapacity(uint32_t time_cap, uint64_t size_cap);
		void SetExpireDays(uint32_t expire_days);
		void SetLogLevel(LogLevel log_level);
		void CheckExpiredLog();
		bool GetBackupNameTime(const std::string &strBackupName, time_t &nTimeFrom, time_t &nTimeTo);
	private:
		LogWriter log_writers_[LOG_DEST_COUNT];
		LogLevel log_level_;
		LogDest log_dest_;
		utils::Mutex mutex_;
		time_t time_capacity_;
		uint64_t size_capacity_;
		uint32_t expire_days_;
		std::string log_path_;
		int64_t m_nCheckRunLogTime_;
	};
}
#endif
