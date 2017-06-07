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

#ifndef NOTICE_H_
#define NOTICE_H_

#include <utils/thread.h>
#include <json/json.h>

namespace bubi {
	class Notice {
	public:
		Notice();
		~Notice();
		bool Initialize();
		void ResetState();
		bool ProcessNotice(Json::Value& result);
		bool CheckTxException(Json::Value& result);
		void SetBuffer(const std::string buffer);
		void SetBubiAttackTime(const uint64_t bubi_attack_time);
		void SetBubiAttackCounts(const uint64_t bubi_attack_counts);
		bool Exit();
	private:
		utils::Mutex mutex_buffer_;
		std::string buffer_;
		uint64_t bubi_attack_time_;		// bubi_attack's threshold
		uint64_t bubi_attack_counts_;	// bubi_attack's counts
		uint64_t txs_last_time_;
		int64_t txs_erase_interval_;
		std::map<uint32_t, std::map<int64_t, std::string>> txs_exception_;
		utils::Mutex txs_exception_mutex_;	// lock exs_exception_
	};
}

#endif
