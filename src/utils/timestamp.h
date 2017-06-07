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

#ifndef UTILS_TIME_H_
#define UTILS_TIME_H_

#include "common.h"

namespace utils {
	class Timestamp {
	public:
		Timestamp();
		Timestamp(int64_t usTimestamp);
		Timestamp(const Timestamp &ts);
		std::string ToString() const;
		std::string ToFormatString(bool with_micro) const;
		std::string Format(bool milli_second) const;
		time_t ToUnixTimestamp() const;
		int64_t timestamp() const;
		bool Valid() const;
#ifdef WIN32
		LARGE_INTEGER toLargeInt() const {
			LARGE_INTEGER li;
			li.QuadPart = (timestamp_ * 10) + 116444736000000000;
			return li;
		}
#endif
		static Timestamp Now();
		static int64_t HighResolution();
		static bool GetLocalTimestamp(time_t timestamp, struct tm &timevalue);
		static Timestamp Invalid() { return Timestamp(); }
		static const int kMicroSecondsPerSecond = 1000 * 1000;
	private:
		int64_t timestamp_;
	};
	inline bool operator<(Timestamp lhs, Timestamp rhs);
	inline bool operator<=(Timestamp lhs, Timestamp rhs);
	inline bool operator==(Timestamp lhs, Timestamp rhs);
}
#endif
