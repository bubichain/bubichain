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

#include "notice.h"
#include <utils/timestamp.h>
#include <utils/logger.h>

namespace bubi {
	Notice::Notice() {
		buffer_ = "";
		bubi_attack_time_ = 60;
		bubi_attack_counts_ = 200;
		txs_erase_interval_ = 180 * utils::MICRO_UNITS_PER_SEC;
		ResetState();
	}

	Notice::~Notice() {

	}

	void Notice::ResetState() {
		txs_last_time_ = utils::Timestamp::HighResolution();
	}

	bool Notice::Initialize() {
		txs_exception_.clear();
		return true;
	}

	bool Notice::Exit() {
		txs_exception_.clear();
		return true;
	}

	void Notice::SetBuffer(const std::string buffer) {
		utils::MutexGuard guard_buffer(mutex_buffer_);
		buffer_ = buffer;
	}

	void Notice::SetBubiAttackTime(const uint64_t bubi_attack_time) {
		bubi_attack_time_ = bubi_attack_time;
	}

	void Notice::SetBubiAttackCounts(const uint64_t bubi_attack_counts) {
		bubi_attack_counts_ = bubi_attack_counts;
	}

	bool Notice::ProcessNotice(Json::Value& result) {
		bool bret = false;
		do {
			std::string buffer;
			{
				utils::MutexGuard guard_buffer(mutex_buffer_);
				buffer = buffer_;
			}
			if (0 == buffer.length()) {
				LOG_ERROR("ProcessNotice -- notice is empty");
				break;
			}
			if (!result.fromString(buffer)) {
				LOG_ERROR("ProcessNotice -- msg's format is invalid");
				break;
			}
			if (!result.isMember("type")) {
				LOG_ERROR("ProcessNotice -- notice does not have type");
				break;
			}
			if (0 == result["type"].asInt()) {
				utils::MutexGuard guard(txs_exception_mutex_);
				txs_exception_[result["error_code"].asInt()].insert(std::make_pair(utils::Timestamp::HighResolution(), result["tx_hash"].asString()));
				break;
			}
			int type = result["type"].asInt();
			//  id of sending to
			if (2 == type) {
				result["from_id"] = result["id"].asString();
			}
			bret = true;
		} while (false);

		return bret;
	}

	bool Notice::CheckTxException(Json::Value& result) {
		bool bret = false;
		do {
			if (0 == bubi_attack_time_) {
				break;
			}

			int64_t now_time = utils::Timestamp::HighResolution();
			if (now_time - txs_last_time_ <= bubi_attack_time_ * utils::MICRO_UNITS_PER_SEC / 2) {
				break;
			}

			bool btx_exception = false;
			utils::MutexGuard guard(txs_exception_mutex_);
			for (auto i = txs_exception_.begin(); i != txs_exception_.end(); i++) {
				auto& tx_exception = i->second;
				if (tx_exception.size() < bubi_attack_counts_) {
					continue;
				}

				std::map<std::string, int> txs_count; // all txs for one exception
				txs_count.clear();
				for (auto j = tx_exception.begin(); j != tx_exception.end(); j++) {
					if (now_time - j->first >= txs_erase_interval_) {
						tx_exception.erase(j);
						j--;
					}
					else {
						txs_count[j->second]++;
					}
				}

				// bigger than 90, this is one attacking
				Json::Value item;
				if (tx_exception.size() > bubi_attack_counts_) {
					Json::Value& txs = item["txs"];
					for (auto k = txs_count.begin(); k != txs_count.end(); k++) {

						Json::Value tx;
						tx["tx_hash"] = k->first;
						tx["counts"] = k->second;
						txs.append(tx);
					}

					item["exception_code"] = i->first;
					result["tx_exceptions"].append(item);
					btx_exception = true;
				}
			}
			if (btx_exception) {
				result["type"] = 0;
				bret = true;
			}
			txs_last_time_ = now_time;
		} while (false);

		return bret;
	}

}

