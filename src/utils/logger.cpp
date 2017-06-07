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
#include "thread.h"
#include "timestamp.h"
#include "file.h"
#include "strings.h"
#include "logger.h"

utils::LogWriter::LogWriter() {
	file_ptr_ = NULL;
}

utils::LogWriter::~LogWriter() {}

bool utils::LogWriter::Init(LogDest dest, const std::string &file_name, bool open_mode) {
	Close();
	dest_ = dest;
	file_name_ = file_name;

	if (dest_ == LOG_DEST_FILE) {
		do {
			file_ptr_ = fopen(file_name.c_str(), open_mode ? "a" : "w");
			if (file_ptr_ == NULL) {
				break;
			}

			// seek to end
			fseek64(file_ptr_, 0, SEEK_END);

			begin_time_ = File::GetAttribue(file_name.c_str()).create_time_;
			size_ = ftell64(file_ptr_);

			return true;
		} while (false);

		return false;
	}
	else if (dest_ == LOG_DEST_OUT) {
		file_ptr_ = stdout;
	}
	else if (dest_ == LOG_DEST_ERR) {
		file_ptr_ = stderr;
	}

	return true;
}

std::string utils::LogWriter::GetLogPrefix(const LogLevel logLevel) {
	switch (logLevel) {
	case LOG_LEVEL_TRACE:	return "TRC";
	case LOG_LEVEL_DEBUG:	return "DBG";
	case LOG_LEVEL_INFO:	return "INF";
	case LOG_LEVEL_WARN:	return "WRN";
	case LOG_LEVEL_ERROR:	return "ERR";
	case LOG_LEVEL_FATAL:	return "FTL";
	default:	return "";
	}
}

utils::LogDest utils::LogWriter::log_dest() {
	return dest_;
}

bool utils::LogWriter::Write(
	Logger *logger,
	const LogLevel logLevel,
	const char *current_time,
	const char* file, const char* funcName, const int lineNum,
	const char* fmt, va_list ap) {

	if (file_ptr_ == NULL) {
		utils::set_error_code(ERROR_NOT_READY);
		return false;
	}

	if (dest_ == LOG_DEST_FILE) {
		size_ = ftell64(file_ptr_);

		if (size_ > logger->size_capacity_ || begin_time_ + logger->time_capacity_ <= time(NULL)) {

			utils::Timestamp begintime((int64_t)begin_time_ * utils::Timestamp::kMicroSecondsPerSecond);

			std::string strBeginTime = begintime.ToFormatString(false);
			std::string strEndTime = utils::Timestamp::Now().ToFormatString(false);

			utils::String::Replace(strBeginTime, "-", ".");
			utils::String::Replace(strBeginTime, "/", ".");
			utils::String::Replace(strBeginTime, ":", ".");
			utils::String::Replace(strBeginTime, " ", "-");

			utils::String::Replace(strEndTime, "-", ".");
			utils::String::Replace(strEndTime, "/", ".");
			utils::String::Replace(strEndTime, ":", ".");
			utils::String::Replace(strEndTime, " ", "-");

			std::string backup_file_name = utils::String::Format("%s-(%s~%s)", file_name_.c_str(), strBeginTime.c_str(), strEndTime.c_str());

			fclose(file_ptr_);
			file_ptr_ = NULL;

			if (!utils::File::Move(file_name_, backup_file_name)) {
				// fatal error on auto-process
				//fprintf(stderr, "Fatal error - backup log file(%s=>%s) failed (%u:%s)\r\n", 
				//	m_strFile.c_str(), strBackupFile.c_str(), __UERR_CODE, __UERR_STR);
				//fflush(stderr);

				// try copy file if move failed
				// because of some program may is still opening the log file
				// such as using #tail -f xxx.log
				utils::File::Copy(file_name_, backup_file_name);
			}

			file_ptr_ = fopen(file_name_.c_str(), "wb");
			if (NULL == file_ptr_) {
				// fatal error on auto-process
				fprintf(stderr, "Fatal error - reopen log file(%s) failed (%u:%s)\n", file_name_.c_str(), STD_ERR_CODE, STD_ERR_DESC);
				fflush(stderr);
			}
			else {
#ifdef WIN32
				// fix Windows file's create time
				// the new log file's create time still keeps the original file time
				// we don't use copy() file because of move() is more quickly
				HANDLE hWin32Handle = (HANDLE)_get_osfhandle(fileno(file_ptr_));
				if (INVALID_HANDLE_VALUE != hWin32Handle) {
					FILETIME nCurrentTime = { 0 };
					::GetSystemTimeAsFileTime(&nCurrentTime);
					::SetFileTime(hWin32Handle, &nCurrentTime, &nCurrentTime, &nCurrentTime);
				}
#endif
				// seek to end
				fseek64(file_ptr_, 0, SEEK_END);

				fprintf(file_ptr_, "[%s - %s] Log::New file start, previous backup file is %s\r\n",
					current_time, GetLogPrefix(utils::LOG_LEVEL_INFO).c_str(), backup_file_name.c_str());
				fflush(file_ptr_);

				begin_time_ = time(NULL);
				size_ = ftell64(file_ptr_);
			}
		}
	}

	fprintf(file_ptr_, "[%s - %s] <%lX> ", current_time, GetLogPrefix(logLevel).c_str(), utils::Thread::current_thread_id());
	fprintf(file_ptr_, "%s(%d):", file, lineNum);
	//fprintf(file_ptr_, "%s(%s:%d):", file, funcName, lineNum);
	// under linux, va_list can't been reused
#ifdef WIN32
	vfprintf(file_ptr_, fmt, ap);
#else
	va_list copy_ap;
	va_copy(copy_ap, ap);
	vfprintf(file_ptr_, fmt, copy_ap);
	va_end(copy_ap);
#endif
	fprintf(file_ptr_, "\n");
	fflush(file_ptr_);
	return true;
}

bool utils::LogWriter::Close() {
	if (file_ptr_ != NULL && dest_ == LOG_DEST_FILE) {
		fclose(file_ptr_);
	}

	file_ptr_ = NULL;
	return true;
}

utils::Logger::Logger() {
	log_dest_ = (LogDest)(utils::LOG_DEST_OUT | utils::LOG_DEST_ERR);
	log_level_ = (LogLevel)(utils::LOG_LEVEL_ALL & ~utils::LOG_LEVEL_TRACE);
	time_capacity_ = utils::SECOND_UNITS_PER_DAY;
	size_capacity_ = 10 * utils::BYTES_PER_MEGA;
	expire_days_ = 10;
	m_nCheckRunLogTime_ = 0;
}

utils::Logger::~Logger() {}

bool utils::Logger::Initialize(utils::LogDest log_dest, utils::LogLevel log_level, const std::string &file_name, bool open_mode) {
	log_level_ = log_level;
	log_dest_ = log_dest;

	if (log_dest & LOG_DEST_FILE && !file_name.empty()) {
		std::string out_file_name = file_name + "-out";
		std::string err_file_name = file_name + "-err";
		std::string file_ext = utils::File::GetExtension(file_name);

		if (file_ext.size() > 0 && (file_ext.size() + 1) < file_name.size()) {
			std::string strSubPath = file_name.substr(0, file_name.size() - file_ext.size() - 1);

			out_file_name = utils::String::Format("%s-out.%s", strSubPath.c_str(), file_ext.c_str());
			err_file_name = utils::String::Format("%s-err.%s", strSubPath.c_str(), file_ext.c_str());
		}

		log_writers_[LOG_DEST_FILE_OUT_ID].Init(LOG_DEST_FILE, out_file_name, open_mode);
		log_writers_[LOG_DEST_FILE_ERR_ID].Init(LOG_DEST_FILE, err_file_name, open_mode);
	}

	if (log_dest & LOG_DEST_OUT) {
		log_writers_[LOG_DEST_OUT_ID].Init(LOG_DEST_OUT, "", false);
	}

	if (log_dest & LOG_DEST_ERR) {
		log_writers_[LOG_DEST_ERR_ID].Init(LOG_DEST_ERR, "", false);
	}

	if (!file_name.empty()) {
		log_path_ = utils::File::GetUpLevelPath(file_name);
	}
	return true;
}

bool utils::Logger::Exit() {
	for (size_t i = 0; i < utils::LOG_DEST_COUNT; i++) {
		log_writers_[i].Close();
	}
	return true;
}

void utils::Logger::SetCapacity(uint32_t time_cap, uint64_t size_cap) {
	time_capacity_ = time_cap;
	size_capacity_ = size_cap;
}

void utils::Logger::SetExpireDays(uint32_t expire_days) {
	expire_days_ = expire_days;
}

void utils::Logger::SetLogLevel(LogLevel log_level) {
	log_level_ = log_level;
}

int utils::Logger::LogStubVm(LogLevel log_Level,
	const char* file,
	const char* funcName, const int lineNum,
	const char* fmt, ...) {

	va_list ap;
	int retVal = 0;
	va_start(ap, fmt);
	retVal = LogStub(log_Level, file, funcName, lineNum, fmt, ap);
	va_end(ap);
	return retVal;
}


int utils::Logger::LogStubVmError(
	const char* file,
	const char* funcName, const int lineNum,
	const char* fmt, ...) {
	utils::MutexGuard _access_(mutex_);

	std::string time_string = utils::Timestamp::Now().Format(true);
	std::string file_name = utils::File::GetFileFromPath(file);

	va_list ap;
	int retVal = 0;
	va_start(ap, fmt);

	utils::LogWriter write_err;
	write_err.Init(LOG_DEST_ERR, "", 0);
	write_err.Write(this, LOG_LEVEL_ERROR, time_string.c_str(),
		file_name.c_str(), funcName, lineNum,
		fmt, ap);


	va_end(ap);

	return 0;
}

int utils::Logger::LogStub(utils::LogLevel log_Level,
	const char* file,
	const char* funcName, const int lineNum,
	const char* fmt, va_list ap) {
	int ret_val = 0;

	if (log_Level != LOG_LEVEL_ALL && ((log_level_ & log_Level) == LOG_LEVEL_NONE)) {
		return 0;
	}

	utils::MutexGuard _access_(mutex_);

	std::string time_string = utils::Timestamp::Now().Format(true);
	std::string file_name = utils::File::GetFileFromPath(file);

	if (log_dest_ & LOG_DEST_FILE) {
		log_writers_[log_Level <= LOG_LEVEL_INFO ? LOG_DEST_FILE_OUT_ID : LOG_DEST_FILE_ERR_ID].Write(this, log_Level, time_string.c_str(),
			file_name.c_str(), funcName, lineNum,
			fmt, ap);
	}

	if (log_Level <= LOG_LEVEL_WARN && log_dest_ & LOG_DEST_OUT) {
		log_writers_[LOG_DEST_OUT_ID].Write(this, log_Level, time_string.c_str(),
			file_name.c_str(), funcName, lineNum,
			fmt, ap);
	}

	if (log_Level > LOG_LEVEL_WARN && log_dest_ & LOG_DEST_ERR) {
		log_writers_[LOG_DEST_ERR_ID].Write(this, log_Level, time_string.c_str(),
			file_name.c_str(), funcName, lineNum,
			fmt, ap);
	}
	return ret_val;
}

void utils::Logger::CheckExpiredLog() {
	int64_t nNextCheckTime = m_nCheckRunLogTime_ + int64_t(utils::SECOND_UNITS_PER_HOUR) * utils::MICRO_UNITS_PER_SEC;
	if (nNextCheckTime > utils::Timestamp::HighResolution()) {
		return;
	}

	time_t nExpiredTime = expire_days_ * utils::SECOND_UNITS_PER_DAY;
	utils::FileAttributes nFiles;
	utils::File::GetFileList(log_path_, nFiles, true);

	for (utils::FileAttributes::iterator itr = nFiles.begin(); itr != nFiles.end(); itr++) {
		const std::string &strName = itr->first;
		utils::FileAttribute &nAttr = itr->second;

		if (nAttr.is_directory_) {
			continue;
		}

		time_t nTimeFrom = 0, nTimeTo = 0;
		if (!utils::Logger::GetBackupNameTime(strName, nTimeFrom, nTimeTo)) {
			nTimeTo = std::max<time_t>(nAttr.create_time_, nAttr.modify_time_);
		}

		if (nTimeTo + nExpiredTime < time(NULL)) {
			utils::Timestamp fromtime((int64_t)nTimeFrom * utils::MICRO_UNITS_PER_SEC);
			utils::Timestamp totime((int64_t)nTimeTo * utils::MICRO_UNITS_PER_SEC);
			LOG_INFO("Run log(%s) expired, days(%u), time(%s => %s), delete it",
				strName.c_str(), expire_days_,
				fromtime.ToFormatString(false).c_str(), totime.ToFormatString(false).c_str());

			std::string strPath = utils::String::Format("%s/%s", log_path_.c_str(), strName.c_str());
			if (!utils::File::Delete(strPath)) {
				LOG_ERROR_ERRNO("Delete run log(%s) failed", strPath.c_str(), STD_ERR_CODE, STD_ERR_DESC);
			}
		}
	}

	m_nCheckRunLogTime_ = utils::Timestamp::HighResolution();
}

bool utils::Logger::GetBackupNameTime(const std::string &strBackupName, time_t &nTimeFrom, time_t &nTimeTo) {
	size_t nBeginPos = strBackupName.find('(');
	size_t nMidPos = strBackupName.find('~');
	size_t nEndPos = strBackupName.rfind(')');

	if (nBeginPos == std::string::npos || nEndPos == std::string::npos || nMidPos == std::string::npos ||
		!(nMidPos > nBeginPos && nMidPos < nEndPos)) {
		return false;
	}

	std::string strLogTimeFrom = strBackupName.substr(nBeginPos + 1, nMidPos - nBeginPos - 1);
	std::string strLogTimeTo = strBackupName.substr(nMidPos + 1, nEndPos - nMidPos - 1);

	nTimeFrom = utils::String::ToTimestamp(strLogTimeFrom);
	nTimeTo = utils::String::ToTimestamp(strLogTimeTo);
	return true;
}
