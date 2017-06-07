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
#include "strings.h"
#include "file.h"

#ifdef WIN32
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <fnmatch.h>
#endif

#ifdef WIN32
const char *utils::File::PATH_SEPARATOR = "\\";
const char  utils::File::PATH_CHAR = '\\';
#else
const char *utils::File::PATH_SEPARATOR = "/";
const char  utils::File::PATH_CHAR = '/';
#endif

utils::File::File() {
	handle_ = NULL;
}

utils::File::~File() {
	if (IsOpened()) Close();
}

std::string utils::File::RegularPath(const std::string &path) {

	std::string new_path = path;
#ifdef WIN32
	if (new_path.size() > 1 && new_path.at(0) == '/') {
		new_path.erase(0, 1);
		new_path.insert(1, ":");
	}

	new_path = utils::String::Replace(new_path, "/", File::PATH_SEPARATOR);

#else
	new_path = utils::String::Replace(new_path, "\\", File::PATH_SEPARATOR);
#endif

	return new_path;
}

std::string utils::File::GetFileFromPath(const std::string &path) {

	std::string regular_path = path;
	regular_path = File::RegularPath(regular_path);

	size_t nPos = regular_path.rfind(File::PATH_CHAR);
	if (std::string::npos == nPos) {
		return regular_path;
	}
	else if (nPos + 1 >= regular_path.size()) {
		return std::string("");
	}
	else {
		return regular_path.substr(nPos + 1, regular_path.size() - nPos - 1);
	}
}

bool utils::File::Open(const std::string &strFile0, int nMode) {
	CHECK_ERROR_RET(IsOpened(), ERROR_ALREADY_EXISTS, false);

	file_name_ = File::RegularPath(strFile0);

	// read or write
	char szMode[64] = { 0 };
	if (nMode & FILE_M_READ)	strcpy(szMode, (nMode & FILE_M_WRITE) ? "r+" : "r");
	else if (nMode & FILE_M_WRITE) strcpy(szMode, "w");
	else if (nMode & FILE_M_APPEND) strcpy(szMode, "a");
	else strcpy(szMode, "r+");

	// text or binary
	if (nMode & FILE_M_BINARY) strcat(szMode, "b");
	else if (nMode & FILE_M_TEXT) strcat(szMode, "t");
	else strcat(szMode, "b"); // default as binary

	handle_ = fopen(file_name_.c_str(), szMode);
	if (NULL == handle_) {
		return false;
	}

	open_mode_ = nMode;
	if (nMode & FILE_M_LOCK && !LockRange(0, utils::LOW32_BITS_MASK, true)) {
		uint32_t error_code = utils::error_code();

		fclose(handle_);
		handle_ = NULL;

		utils::set_error_code(error_code);
		return false;
	}

	return NULL != handle_;
}

bool utils::File::Close() {
	CHECK_ERROR_RET(!IsOpened(), ERROR_NOT_READY, false);

	if (open_mode_ & FILE_M_LOCK) {
		UnlockRange(0, utils::LOW32_BITS_MASK);
	}

	fclose(handle_);
	handle_ = NULL;

	return true;
}

bool utils::File::Flush() {
	CHECK_ERROR_RET(!IsOpened(), ERROR_NOT_READY, false);

	return fflush(handle_) == 0;
}

bool utils::File::LockRange(uint64_t offset, uint64_t size, bool try_lock /* = false */) {
	CHECK_ERROR_RET(!IsOpened(), ERROR_NOT_READY, false);

	bool result = false;
	int file_no = fileno(handle_);

#ifdef WIN32
	OVERLAPPED nOverlapped;
	DWORD dwSizeLow = (DWORD)(size & utils::LOW32_BITS_MASK);
	DWORD dwSizeHigh = (DWORD)((size >> 32) & utils::LOW32_BITS_MASK);
	DWORD dwFlags = try_lock ? (LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY) : LOCKFILE_EXCLUSIVE_LOCK;

	memset(&nOverlapped, 0, sizeof(nOverlapped));
	nOverlapped.Offset = (DWORD)(offset & utils::LOW32_BITS_MASK);
	nOverlapped.OffsetHigh = (DWORD)((offset >> 32) & utils::LOW32_BITS_MASK);

	result = (::LockFileEx((HANDLE)_get_osfhandle(file_no), dwFlags, 0, dwSizeLow, dwSizeHigh, &nOverlapped) == TRUE);
#else


	uint64_t nOldPosition = GetPosition();
	if (nOldPosition != offset && !Seek(offset, File::FILE_S_BEGIN)) {
		return false;
	}

	int nFlags = try_lock ? F_TLOCK : F_LOCK;
	off_t nUnlockSize = (utils::LOW32_BITS_MASK == size) ? 0 : (off_t)size;
	result = (lockf(file_no, nFlags, nUnlockSize) == 0);

	if (nOldPosition != offset && !Seek(nOldPosition, File::FILE_S_BEGIN)) {
		uint32_t nErrorCode = utils::error_code();

		// unlock
		//result = false;
		result = (lockf(file_no, F_ULOCK, nUnlockSize) == 0);

		utils::set_error_code(nErrorCode);
	}

#endif

	return result;
}

bool utils::File::UnlockRange(uint64_t offset, uint64_t size) {
	CHECK_ERROR_RET(!IsOpened(), ERROR_NOT_READY, false);

	bool result = false;
	int file_no = fileno(handle_);

#ifdef WIN32
	OVERLAPPED nOverlapped;
	DWORD dwSizeLow = (DWORD)(size & utils::LOW32_BITS_MASK);
	DWORD dwSizeHigh = (DWORD)((size >> 32) & utils::LOW32_BITS_MASK);

	memset(&nOverlapped, 0, sizeof(nOverlapped));
	nOverlapped.Offset = (DWORD)(offset & utils::LOW32_BITS_MASK);
	nOverlapped.OffsetHigh = (DWORD)((offset >> 32) & utils::LOW32_BITS_MASK);

	result = (::UnlockFileEx((HANDLE)_get_osfhandle(file_no), 0, dwSizeLow, dwSizeHigh, &nOverlapped) == TRUE);
#else

#ifndef ANDROID
	uint64_t nOldPosition = GetPosition();

	if (nOldPosition != offset && !Seek(offset, File::FILE_S_BEGIN)) {
		return false;
	}

	off_t nUnlockSize = (utils::LOW32_BITS_MASK == size) ? 0 : (off_t)size;
	bool bResult = (lockf(file_no, F_ULOCK, nUnlockSize) == 0);

	if (nOldPosition != offset && !Seek(nOldPosition, File::FILE_S_BEGIN)) {
		bResult = false;
	}
#endif

#endif

	return result;
}

size_t utils::File::ReadData(std::string &data, size_t max_count) {
	CHECK_ERROR_RET(!IsOpened(), ERROR_NOT_READY, 0);

	const size_t buffer_size = 1024 * 1024;
	size_t read_bytes_size = 0;
	static char nTmpBuffer[buffer_size];

	while (read_bytes_size < max_count) {
		size_t nCount = MIN(max_count - read_bytes_size, buffer_size);
		size_t nRetBytes = fread(nTmpBuffer, 1, nCount, handle_);

		if (nRetBytes > 0) {
			read_bytes_size += nRetBytes;
			data.append(nTmpBuffer, nRetBytes);
		}
		else {
			break;
		}
	}

	return read_bytes_size;
}

size_t utils::File::Read(void *pBuffer, size_t nChunkSize, size_t nCount) {
	CHECK_ERROR_RET(!IsOpened(), ERROR_NOT_READY, 0);

	return fread(pBuffer, nChunkSize, nCount, handle_);
}

bool utils::File::ReadLine(std::string &strLine, size_t nMaxCount) {
	CHECK_ERROR_RET(!IsOpened(), ERROR_NOT_READY, 0);

	strLine.resize(nMaxCount + 1, 0);

	char *pszBuffer = (char *)strLine.c_str();
	pszBuffer[nMaxCount] = 0;
	if (fgets(pszBuffer, nMaxCount, handle_) == NULL) {
		strLine.clear();
		return false;
	}

	size_t nNewLen = strlen(pszBuffer);
	strLine.resize(nNewLen);
	return true;
}

size_t utils::File::Write(const void *pBuffer, size_t nChunkSize, size_t nCount) {
	CHECK_ERROR_RET(!IsOpened(), ERROR_NOT_READY, 0);

	return fwrite(pBuffer, nChunkSize, nCount, handle_);
}

uint64_t  utils::File::GetPosition() {
	return 0;
}

bool utils::File::Seek(uint64_t offset, FILE_SEEK_MODE nMode) {
	return true;
}

bool utils::File::IsAbsolute(const std::string &path) {
	std::string regular_path = File::RegularPath(path);

#ifdef WIN32
	return regular_path.size() > 0 && regular_path.find(':') != std::string::npos;
#else
	return regular_path.size() > 0 && '/' == regular_path[0];
#endif
}

std::string utils::File::GetBinPath() {
	std::string path;
	char szpath[File::MAX_PATH_LEN * 4] = { 0 };

#ifdef WIN32
	DWORD path_len = ::GetModuleFileNameA(NULL, szpath, File::MAX_PATH_LEN * 4 - 1);
	if (path_len >= 0) {
		szpath[path_len] = '\0';
		path = szpath;
	}
#else
	ssize_t path_len = readlink("/proc/self/exe", szpath, File::MAX_PATH_LEN * 4 - 1);
	if (path_len >= 0) {
		szpath[path_len] = '\0';
		path = szpath;
	}
#endif

	return path;
}

utils::FileAttribute::FileAttribute() {
	is_directory_ = false;
	create_time_ = 0;
	modify_time_ = 0;
	access_time_ = 0;
	size_ = 0;
}

std::string utils::File::GetBinDirecotry() {
	std::string path = File::GetBinPath();

	return GetUpLevelPath(path);
}

std::string utils::File::GetBinHome() {
	return GetUpLevelPath(GetBinDirecotry());
}

std::string utils::File::GetUpLevelPath(const std::string &path) {
	std::string normal_path = File::RegularPath(path);

	size_t nPos = normal_path.rfind(File::PATH_CHAR);
	if (std::string::npos == nPos) {
#ifdef WIN32
		nPos = normal_path.rfind(':');
		if (std::string::npos == nPos) {
			return std::string("");
		}
		else {
			nPos++;
		}
#else
		return std::string("");
#endif
	}

	if (0 == nPos && normal_path.size() > 0 && normal_path[0] == File::PATH_CHAR) {
		nPos++;
	}

	return normal_path.substr(0, nPos);
}

#ifdef WIN32
time_t __WinFileTime2UnixTime(const FILETIME &nTime) {
	LONGLONG nTimeTick = (((LONGLONG)nTime.dwHighDateTime) << 32) + (LONGLONG)(nTime.dwLowDateTime);
	return (time_t)((nTimeTick - 116444736000000000L) / 10000000);
}
#endif

bool utils::File::GetAttribue(const std::string &strFile0, FileAttribute &nAttr) {
	std::string file1 = RegularPath(strFile0);


#ifdef WIN32
	WIN32_FILE_ATTRIBUTE_DATA nFileAttr;
	if (GetFileAttributesExA(file1.c_str(), GetFileExInfoStandard, &nFileAttr)) {
		nAttr.is_directory_ = (nFileAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		nAttr.create_time_ = __WinFileTime2UnixTime(nFileAttr.ftCreationTime);
		nAttr.modify_time_ = __WinFileTime2UnixTime(nFileAttr.ftLastWriteTime);
		nAttr.access_time_ = __WinFileTime2UnixTime(nFileAttr.ftLastAccessTime);
		nAttr.size_ = (((uint64_t)(nFileAttr.nFileSizeHigh)) << 32) + (uint64_t)(nFileAttr.nFileSizeLow);

		return true;
	}
#else
	struct stat nFileStat;
	if (stat(file1.c_str(), &nFileStat) == 0) {
		nAttr.is_directory_ = (nFileStat.st_mode & S_IFDIR) != 0;
		nAttr.create_time_ = nFileStat.st_ctime;
		nAttr.modify_time_ = nFileStat.st_mtime;
		nAttr.access_time_ = nFileStat.st_atime;
		nAttr.size_ = nFileStat.st_size;

		return true;
	}
#endif
	else {
		return false;
	}
}

#ifdef WIN32
size_t __WinGetDriveInfo(utils::FileAttributes &nFiles) {
	DWORD dwDrv = GetLogicalDrives();
	for (int i = 0; dwDrv > 0; i++, dwDrv >>= 1) {
		if ((dwDrv & 0x00000001) == 0) continue;
		std::string strName = utils::String::Format("%c", 'A' + i);
		utils::FileAttribute &nAttr = nFiles[strName];
		nAttr.is_directory_ = true;
	}
	return nFiles.size();
}
#endif

bool utils::File::GetFileList(const std::string &strDirectory0, const std::string &strPattern, utils::FileAttributes &nFiles, bool bFillAttr, size_t nMaxCount) {
	std::string strNormalPath = File::RegularPath(strDirectory0);

#ifdef WIN32
	if (strDirectory0 == "/") {
		__WinGetDriveInfo(nFiles);
		return true;
	}

	WIN32_FIND_DATAA nFindData;
	HANDLE hFindFile = INVALID_HANDLE_VALUE;

	std::string strFindName = utils::String::Format("%s\\%s", strNormalPath.c_str(), strPattern.empty() ? "*" : strPattern.c_str());
	hFindFile = FindFirstFileA(strFindName.c_str(), &nFindData);
	if (INVALID_HANDLE_VALUE == hFindFile) {
		return false;
	}

	// clean old files
	nFiles.clear();

	do {
		if (strcmp(nFindData.cFileName, ".") == 0 ||
			strcmp(nFindData.cFileName, "..") == 0) {
			// self or parent
			continue;
		}

		std::string strName(nFindData.cFileName);
		utils::FileAttribute &nAttr = nFiles[strName];

		nAttr.is_directory_ = (nFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		nAttr.create_time_ = __WinFileTime2UnixTime(nFindData.ftCreationTime);
		nAttr.modify_time_ = __WinFileTime2UnixTime(nFindData.ftLastWriteTime);
		nAttr.access_time_ = __WinFileTime2UnixTime(nFindData.ftLastAccessTime);
		nAttr.size_ = (((uint64_t)(nFindData.nFileSizeHigh)) << 32) + (uint64_t)(nFindData.nFileSizeLow);

		if (nMaxCount > 0 && nFiles.size() >= nMaxCount) {
			break;
		}
	} while (FindNextFileA(hFindFile, &nFindData));

	FindClose(hFindFile);
	hFindFile = INVALID_HANDLE_VALUE;
#else
	DIR *pDir = opendir(strNormalPath.c_str());
	if (NULL == pDir) {
		return false;
	}

	// clean old files
	nFiles.clear();

	struct dirent *pItem = NULL;
	while ((pItem = readdir(pDir)) != NULL) {
		if (strcmp(pItem->d_name, ".") == 0 ||
			strcmp(pItem->d_name, "..") == 0) {
			// self or parent
			continue;
		}

		if (!strPattern.empty() && fnmatch(strPattern.c_str(), pItem->d_name, FNM_FILE_NAME | FNM_PERIOD) != 0) {
			// not match the pattern
			continue;
		}

		std::string strName(pItem->d_name);
		utils::FileAttribute &nAttr = nFiles[strName];

		if (bFillAttr) {
			std::string strFilePath = utils::String::Format("%s/%s", strNormalPath.c_str(), strName.c_str());
			File::GetAttribue(strFilePath, nAttr);
		}

		if (nMaxCount > 0 && nFiles.size() >= nMaxCount) {
			break;
		}
	}

	closedir(pDir);
#endif

	return true;
}

bool utils::File::GetFileList(const std::string &strDirectory0, utils::FileAttributes &nFiles, bool bFillAttr, size_t nMaxCount) {
	return utils::File::GetFileList(strDirectory0, std::string(""), nFiles, bFillAttr, nMaxCount);
}

utils::FileAttribute utils::File::GetAttribue(const std::string &strFile0) {
	utils::FileAttribute nAttr;
	std::string strNormalFile = File::RegularPath(strFile0);

	File::GetAttribue(strNormalFile, nAttr);
	return nAttr;
}

bool utils::File::Move(const std::string &strSource, const std::string &strDest, bool bOverwrite) {
	std::string strNormalSource = File::RegularPath(strSource);
	std::string strNormalDest = File::RegularPath(strDest);

	if (bOverwrite && utils::File::IsExist(strDest)) {
		utils::File::Delete(strDest);
	}

#ifdef WIN32
	return ::MoveFileA(strNormalSource.c_str(), strNormalDest.c_str()) == TRUE;
#else
	return rename(strNormalSource.c_str(), strNormalDest.c_str()) == 0;
#endif
}

bool utils::File::Copy(const std::string &strSource, const std::string &strDest, bool bOverwrite) {
	std::string strNormalSource = File::RegularPath(strSource);
	std::string strNormalDest = File::RegularPath(strDest);

#ifdef WIN32
	return ::CopyFileA(strNormalSource.c_str(), strNormalDest.c_str(), !bOverwrite) == TRUE;
#else
	if (strNormalSource == strNormalDest) {
		utils::set_error_code(ERROR_ALREADY_EXISTS);
		return false;
	}
	else if (!bOverwrite && utils::File::IsExist(strNormalDest)) {
		utils::set_error_code(ERROR_ALREADY_EXISTS);
		return false;
	}

	utils::File nSource, nDest;
	uint32_t nErrorCode = ERROR_SUCCESS;
	bool bSuccess = false;
	char *pDataBuffer = NULL;
	const size_t nBufferSize = 102400;

	do {
		pDataBuffer = (char *)malloc(nBufferSize);
		if (NULL == pDataBuffer) {
			nErrorCode = utils::error_code();
			break;
		}

		if (!nSource.Open(strNormalSource, utils::File::FILE_M_READ | utils::File::FILE_M_BINARY) ||
			!nDest.Open(strNormalDest, utils::File::FILE_M_WRITE | utils::File::FILE_M_BINARY)) {
			nErrorCode = utils::error_code();
			break;
		}

		while (true) {
			size_t nSizeRead = nSource.Read(pDataBuffer, 1, nBufferSize);
			if (nSizeRead == 0) {
				bSuccess = true;
				break;
			}

			if (nDest.Write(pDataBuffer, 1, nSizeRead) != nSizeRead) {
				nErrorCode = utils::error_code();
				break;
			}
		}
	} while (false);

	if (NULL != pDataBuffer) free(pDataBuffer);
	if (nSource.IsOpened()) nSource.Close();
	if (nDest.IsOpened()) nDest.Close();

	if (ERROR_SUCCESS != nErrorCode) {
		utils::set_error_code(nErrorCode);
	}

	return bSuccess;
#endif
}

bool utils::File::IsExist(const std::string &strFile) {
	std::string strNormalFile = File::RegularPath(strFile);

#ifdef WIN32
	return ::PathFileExistsA(strNormalFile.c_str()) == TRUE;
#else
	struct stat nFileStat;
	return stat(strNormalFile.c_str(), &nFileStat) == 0 || errno != ENOENT;
#endif
}

bool utils::File::Delete(const std::string &strFile) {
	std::string strNormalFile = File::RegularPath(strFile);

#ifdef WIN32
	return ::DeleteFileA(strNormalFile.c_str()) == TRUE;
#else
	return unlink(strNormalFile.c_str()) == 0;
#endif
}

#ifdef WIN32
#else
void dfs_remove_dir() {
	DIR* cur_dir = opendir(".");
	struct dirent *ent = NULL;
	struct stat st;
	if (!cur_dir) {
		//	LOG_ERROR("");
		return;
	}
	while ((ent = readdir(cur_dir)) != NULL) {
		stat(ent->d_name, &st);
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
			continue;
		}

		if (S_ISDIR(st.st_mode)) {
			chdir(ent->d_name);
			chdir("..");
		}
		remove(ent->d_name);
	}
	closedir(cur_dir);
}
#endif // WIN32


bool utils::File::DeleteFolder(const std::string &path) {
	std::string strNormalFile = File::RegularPath(path);

#ifdef WIN32
	SHFILEOPSTRUCT  FileOp;
	FileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
	FileOp.hNameMappings = NULL;
	FileOp.hwnd = NULL;
	FileOp.lpszProgressTitle = NULL;
	strNormalFile += '\0'; //sxf add 20170302 meet the FileOp.pFrom input of double null
	FileOp.pFrom = strNormalFile.c_str();
	FileOp.pTo = NULL;
	FileOp.wFunc = FO_DELETE;
	int n = SHFileOperation(&FileOp);
	return n == 0;
#else
	char old_path[100];

	if (!path.c_str()) {
		return false;
	}

	getcwd(old_path, 100);

	if (chdir(path.c_str()) == -1) {
		//LOG_ERROR("not a dir or access error\n");
		return false;
	}

	//LOG_INFO("path_raw : %s\n", path_raw);
	dfs_remove_dir();
	chdir(old_path);
	unlink(old_path);
	return rmdir(strNormalFile.c_str()) == 0;
#endif
}

std::string utils::File::GetExtension(const std::string &path) {
	std::string normal_path = RegularPath(path);

	// check whether url is like "*****?url=*.*.*.*"
	size_t end_pos = normal_path.find('?');
	if (end_pos != std::string::npos) {
		normal_path = normal_path.substr(0, end_pos);
	}

	size_t nPos = normal_path.rfind('.');
	if (std::string::npos == nPos || (nPos + 1) == normal_path.size()) {
		return std::string("");
	}

	return normal_path.substr(nPos + 1);
}

std::string utils::File::GetTempDirectory() {
	std::string strPath;

#ifdef WIN32
	const size_t nMaxPathSize = 10240;
	strPath.resize(nMaxPathSize, 0);
	DWORD dwRetVal = GetTempPathA((DWORD)nMaxPathSize - 1, (char *)strPath.c_str());

	if (dwRetVal > 0) {
		strPath.resize(dwRetVal);
	}
	else {
		strPath.clear();
	}
#else
	strPath = std::string("/tmp");
#endif

	while (strPath.size() > 0 && File::PATH_CHAR == strPath[strPath.size() - 1]) {
		strPath.erase(strPath.size() - 1, 1);
	}

	return strPath;
}

bool utils::File::CreateDir(const std::string &path) {
	std::string strNormalFile = File::RegularPath(path);
#ifdef WIN32
	return CreateDirectoryA(strNormalFile.c_str(), NULL) != 0;
#else
	return mkdir(strNormalFile.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
#endif
}