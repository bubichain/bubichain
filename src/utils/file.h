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

#ifndef _UTILS_FILE_H_
#define _UTILS_FILE_H_

namespace utils {
	class FileAttribute {
	public:
		FileAttribute();
	public:
		bool is_directory_;
		time_t create_time_;
		time_t modify_time_;
		time_t access_time_;
		int64_t  size_;
	};
	typedef std::map<std::string, utils::FileAttribute> FileAttributes;
	class File {
	public:
		typedef enum FILE_OPEN_MODE_ {
			FILE_M_NONE = 0x00,
			FILE_M_READ = 0x01,
			FILE_M_WRITE = 0x02,
			FILE_M_APPEND = 0x04,
			FILE_M_TEXT = 0x08,
			FILE_M_BINARY = 0x10,
			FILE_M_LOCK = 0x20,
		}FILE_OPEN_MODE;
		typedef enum FILE_SEEK_MODE_ {
			FILE_S_BEGIN = 0x00,
			FILE_S_CURRENT = 0x01,
			FILE_S_END = 0x02,
		}FILE_SEEK_MODE;
		static const char  *PATH_SEPARATOR;
		static const char   PATH_CHAR;
		static const uint64_t INVALID_SIZE = ((uint64_t)-1);
		static const size_t MAX_PATH_LEN = 256;
	public:
		File();
		~File();
		inline bool IsOpened() { return NULL != handle_; }
		bool   Open(const std::string &file_name, int nMode);
		bool   Close();
		bool   Flush();
		size_t ReadData(std::string &data, size_t max_size);
		size_t Read(void *pBuffer, size_t nChunkSize, size_t nCount);
		bool   ReadLine(std::string &strLine, size_t nMaxCount);
		size_t Write(const void *pBuffer, size_t nChunkSize, size_t nCount);
		static std::string RegularPath(const std::string &path);
		static std::string GetFileFromPath(const std::string &path);
		static bool IsAbsolute(const std::string &path);
		static std::string GetBinPath();
		static std::string GetBinDirecotry();
		static std::string GetBinHome();
		static std::string GetUpLevelPath(const std::string &path);
		static bool GetAttribue(const std::string &strFile0, FileAttribute &nAttr);
		static bool GetFileList(const std::string &strDirectory, utils::FileAttributes &nFiles, bool bFillAttr = true, size_t nMaxCount = 0);
		static bool GetFileList(const std::string &strDirectory, const std::string &strPattern, utils::FileAttributes &nFiles, bool bFillAttr = true, size_t nMaxCount = 0);
		static utils::FileAttribute GetAttribue(const std::string &strFile);
		static bool   Move(const std::string &source, const std::string &dest, bool over_write = false);
		static bool   Copy(const std::string &source, const std::string &dest, bool over_write = true);
		static bool   IsExist(const std::string &strFile);
		static bool   Delete(const std::string &strFile);
		static bool   DeleteFolder(const std::string &path);
		static bool   CreateDir(const std::string &path);
		static std::string GetExtension(const std::string &strPath);
		static std::string GetTempDirectory();
		bool   LockRange(uint64_t offset, uint64_t size, bool try_lock = false);
		bool   UnlockRange(uint64_t offset, uint64_t size);
		uint64_t  GetPosition();
		bool Seek(uint64_t offset, FILE_SEEK_MODE nMode);
	public:
		FILE *handle_;
		int open_mode_;
		std::string file_name_;
	};
}
#endif