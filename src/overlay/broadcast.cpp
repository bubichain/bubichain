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

#include <json/value.h>
#include <utils/headers.h>
#include "broadcast.h"

namespace bubi {
	BroadcastRecord::BroadcastRecord(const PeerMessagePointer &peer_msg, int64_t peer_id) {
		peer_msg_ = peer_msg;
		peers_.insert(peer_id);
		time_stamp_ = utils::Timestamp::HighResolution();
	}

	BroadcastRecord::~BroadcastRecord() {}

	Broadcast::Broadcast(IBroadcastDriver *driver)
		:driver_(driver) {}

	Broadcast::~Broadcast() {}

	bool Broadcast::Add(const PeerMessagePointer &msg, int64_t peer_id) {
		utils::MutexGuard guard(mutex_msg_sending_);
		if (msg->hash_.empty()) msg->hash_ = utils::Sha256::Crypto(msg->GetString());
		BroadcastRecordMap::iterator result = records_.find(msg->hash_);
		if (result == records_.end()) { // we have never seen this message
			BroadcastRecord::pointer record = std::make_shared<BroadcastRecord>(msg, peer_id);
			records_[msg->hash_] = record;
			records_couple_[record->time_stamp_] = msg->hash_;
			return true;
		}
		else {
			result->second->peers_.insert(peer_id);
			return false;
		}

		return true;
	}

	void Broadcast::Send(const std::shared_ptr<PeerMessage> &msg) {
		if (msg->hash_.empty()) msg->hash_ = utils::Sha256::Crypto(msg->GetString());
		utils::MutexGuard guard(mutex_msg_sending_);
		BroadcastRecordMap::iterator result = records_.find(msg->hash_);
		if (result == records_.end()) { // no one has sent us this message
			BroadcastRecord::pointer record = std::make_shared<BroadcastRecord>(
				msg, 0);

			records_[msg->hash_] = record;
			records_couple_[record->time_stamp_] = msg->hash_;
			std::set<int64_t> peer_ids = driver_->GetActivePeerIds();
			for (const auto peer_id : peer_ids) {
				driver_->SendMessage(peer_id, msg);
				record->peers_.insert(peer_id);
			}
		}
		else { // send it to people that haven't sent it to us
			std::set<int64_t>& peersTold = result->second->peers_;
			for (const auto peer : driver_->GetActivePeerIds()) {
				if (peersTold.find(peer) == peersTold.end()) {
					driver_->SendMessage(peer, msg);
					result->second->peers_.insert(peer);
				}
			}
		}
	}

	void Broadcast::OnTimer() {
		utils::MutexGuard guard(mutex_msg_sending_);
		int64_t current_time = utils::Timestamp::HighResolution();

		for (auto it = records_couple_.begin(); it != records_couple_.end();) {
			// give one ledger of leeway
			if (it->first + 120 * utils::MICRO_UNITS_PER_SEC < current_time) {
				records_.erase(it->second);
				records_couple_.erase(it++);
			}
			else {
				break;
			}
		}

		for (auto i = msg_sending_.begin(); i != msg_sending_.end(); i++) {
			Send(*i);
		}
		msg_sending_.clear();
	}
}
