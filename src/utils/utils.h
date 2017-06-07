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

#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#include <stack>
#include "common.h"
#include "basen.h"

#ifndef WIN32
#include <sys/sysinfo.h>
#endif

namespace utils {
	// seconds
	static const int64_t  MILLI_UNITS_PER_SEC = 1000;
	static const int64_t  MICRO_UNITS_PER_MILLI = 1000;
	static const int64_t  NANO_UNITS_PER_MICRO = 1000;
	static const int64_t  MICRO_UNITS_PER_SEC = MICRO_UNITS_PER_MILLI * MILLI_UNITS_PER_SEC;
	static const int64_t  NANO_UNITS_PER_SEC = NANO_UNITS_PER_MICRO * MICRO_UNITS_PER_SEC;
	static const time_t SECOND_UNITS_PER_MINUTE = 60;
	static const time_t MINUTE_UNITS_PER_HOUR = 60;
	static const time_t HOUR_UNITS_PER_DAY = 24;
	static const time_t SECOND_UNITS_PER_HOUR = SECOND_UNITS_PER_MINUTE * MINUTE_UNITS_PER_HOUR;
	static const time_t SECOND_UNITS_PER_DAY = SECOND_UNITS_PER_HOUR * HOUR_UNITS_PER_DAY;
	static const size_t  BYTES_PER_KILO = 1024;
	static const size_t  KILO_PER_MEGA = 1024;
	static const size_t  BYTES_PER_MEGA = BYTES_PER_KILO * KILO_PER_MEGA;
	static const uint16_t  MAX_UINT16 = 0xFFFF;
	static const uint32_t  MAX_UINT32 = 0xFFFFFFFF;
	static const int32_t   MAX_INT32 = 0x7FFFFFFF;
	static const int64_t   MAX_INT64 = 0x7FFFFFFFFFFFFFFF;
	// low-high
	static const uint64_t LOW32_BITS_MASK = 0xffffffffULL;
	static const uint64_t HIGH32_BITS_MASK = 0xffffffff00000000ULL;
	static const size_t ETH_MAX_PACKET_SIZE = 1600;
	uint32_t error_code();
	void set_error_code(uint32_t code);
	std::string error_desc(uint32_t code = -1);

#ifdef WIN32
#define BUBI_CAS(mem, with, cmp) InterlockedCompareExchange(mem, with, cmp)
#define BUBI_YIELD()             SwitchToThread()
#else
#define BUBI_CAS(mem, with, cmp) __sync_val_compare_and_swap(mem, cmp, with)
#define BUBI_YIELD()             pthread_yield();
#endif

#ifdef WIN32
	inline LONG AtomicInc(volatile LONG *value) {
		return InterlockedIncrement(value);
	}
#else
	inline int32_t AtomicInc(volatile int32_t *value) {
		__sync_fetch_and_add(value, 1);
		return *value;
	}
#endif

#ifdef WIN32
	inline LONGLONG AtomicInc(volatile LONGLONG *value) {
		return InterlockedIncrement64(value);
	}
#else
	inline int64_t AtomicInc(volatile int64_t *value) {
		__sync_fetch_and_add(value, 1);
		return *value;
	}
#endif

#ifdef WIN32
	inline LONG AtomicDec(volatile LONG *value) {
		return InterlockedDecrement(value);
	}
#else
	inline int32_t AtomicDec(volatile int32_t *value) {
		__sync_fetch_and_sub(value, 1);
		return *value;
	}
#endif

#ifdef WIN32
	inline LONGLONG AtomicDec(volatile LONGLONG *value) {
		return InterlockedDecrement64(value);
	}
#else
	inline int64_t AtomicDec(volatile int64_t *value) {
		__sync_fetch_and_sub(value, 1);
		return *value;
	}
#endif

	template<typename T>
	class AtomicInteger {
	public:
		AtomicInteger()
			: value_(0) {}
		T Inc() {
			return AtomicInc(&value_);
		}
		T Dec() {
			return AtomicDec(&value_);
		}
		T value() const {
			return value_;
		}
	private:
		T value_;
	};

#ifdef WIN32
	typedef AtomicInteger<LONG> AtomicInt32;
	typedef AtomicInteger<LONGLONG> AtomicInt64;
#else
	typedef AtomicInteger<int32_t> AtomicInt32;
	typedef AtomicInteger<int64_t> AtomicInt64;
#endif

	void Sleep(int nMillSecs);
	size_t GetCpuCoreCount();
	time_t GetStartupTime(time_t time_now = 0);
#if __cplusplus >= 201402L || (defined(_MSC_VER) && _MSC_VER >= 1900)
	using std::make_unique;
#else
	template <typename T, typename... Args>
	std::unique_ptr<T>
		make_unique(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
#endif

	class BubiAtExit {
	public:
		typedef std::function<bool()> ExitHandler;
		BubiAtExit();
		~BubiAtExit();
		void Push(ExitHandler fun);
	private:
		std::stack<ExitHandler> s_;
	};
}

#endif

