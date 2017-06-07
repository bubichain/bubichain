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

/**
* the common header which need to be included in others utils files
*/

#ifndef UTILS_COMMON_H_
#define UTILS_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory>
#include <functional>
#include <errno.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <string>
#include <string.h>
#include <list>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <algorithm>

#ifndef NDEBUG
#include <assert.h>
#endif

#define MSG_NOSIGNAL_ 0
#define SIGHUP       1
#define SIGQUIT      3
#define SIGKILL      9

#ifdef WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <shlwapi.h>
#include <io.h>
#include <process.h>

#define fseek64  _fseeki64
#define ftell64  _ftelli64

typedef CRITICAL_SECTION pthread_mutex_t;

#define __func__ __FUNCTION__
#pragma warning(disable:4819)
#pragma warning(disable:4996)
#define snprintf _snprintf

#if _MSC_VER < 1500 // MSVC 2008
#define vsnprintf _vsnprintf
#endif
#else

#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ifaddrs.h>
#include <dirent.h>

#define ERROR_SUCCESS            0
#define ERROR_ALREADY_EXISTS     EEXIST
#define ERROR_NOT_READY          ENOENT
#define ERROR_ACCESS_DENIED      EPERM
#define ERROR_NO_DATA            ENOENT
#define ERROR_TIMEOUT            ETIMEDOUT
#define ERROR_CONNECTION_REFUSED ECONNREFUSED
#define ERROR_INVALID_PARAMETER  ERANGE
#define ERROR_NOT_SUPPORTED      ENODATA
#define ERROR_INTERNAL_ERROR     EFAULT
#define ERROR_IO_DEVICE          EIO

#define fseek64     fseeko
#define ftell64     ftello

#endif

#ifdef WIN32
#define FMT_I64 "%I64d"
#define FMT_I64_EX(fmt) "%" #fmt "I64d"
#define FMT_U64 "%I64u"
#define FMT_X64 "%I64X"
#define FMT_SIZE "%lu"
#else
#ifdef __x86_64__
#define FMT_I64 "%ld"
#define FMT_I64_EX(fmt) "%" #fmt "ld"
#define FMT_U64 "%lu"
#define FMT_X64 "%lX"
#define FMT_SIZE "%lu"
#else
#define FMT_I64 "%lld"
#define FMT_I64_EX(fmt) "%" #fmt "lld"
#define FMT_U64 "%llu"
#define FMT_X64 "%llX"
#define FMT_SIZE "%u"
#endif

#endif

namespace utils {
	using namespace std::placeholders; // for std::bind
	// disallow copy ctor and assign opt
#undef UTILS_DISALLOW_EVIL_CONSTRUCTORS
#define UTILS_DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
    TypeName(const TypeName&);                         \
    void operator=(const TypeName&)
	// delete object safe
#define SAFE_DELETE(p)        \
    if (NULL != p) {          \
        delete p;             \
        p = NULL;             \
	    }
	// delete object array safe
#define SAFE_DELETE_ARRAY(p)  \
    if (NULL != p) {          \
        delete []p;           \
        p = NULL;             \
	    }
#ifndef MIN
#define MIN(a,b) ((a)<(b)) ? (a) : (b)
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)) ? (a) : (b)
#endif
#define CHECK_ERROR_RET(func, ecode, ret) \
	if( func )\
		{\
	utils::set_error_code(ecode); \
	return ret; \
		}
#define DISALLOW_COPY_AND_ASSIGN(cls) private:\
	cls( const cls & );\
	cls & operator =( const cls & )
}
#endif 
