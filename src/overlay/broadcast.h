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

#ifndef BROADCAST_H_
#define BROADCAST_H_

#include "peer.h"

namespace bubi {
	class IBroadcastDriver {
	public:
		IBroadcastDriver() {};
		virtual ~IBroadcastDriver() {};
		virtual bool SendMessage(int64_t peer_id, PeerMessagePointer msg) = 0;
		virtual std::set<int64_t> GetActivePeerIds() = 0;
	};
	class BroadcastRecord {
	public:
		typedef std::shared_ptr<BroadcastRecord> pointer;
		BroadcastRecord(const PeerMessagePointer &peer_msg, int64_t);
		~BroadcastRecord();
	public:
		PeerMessagePointer peer_msg_;
		int64_t time_stamp_;
		std::set<int64_t> peers_;
	};
	typedef std::map<int64_t, std::string> BroadcastRecordCoupleMap;
	typedef std::map<std::string, BroadcastRecord::pointer> BroadcastRecordMap;
	class Broadcast {
	public:
		Broadcast(IBroadcastDriver *driver);
		~Broadcast();
		bool Add(const PeerMessagePointer &msg, int64_t peer_id);
		void Send(const PeerMessagePointer &msg);
		void OnTimer();
		size_t GetRecordSize() const { return records_.size(); };
	private:
		BroadcastRecordCoupleMap records_couple_;
		BroadcastRecordMap records_;
		std::list<PeerMessagePointer> msg_sending_;
		utils::Mutex mutex_msg_sending_;
		IBroadcastDriver *driver_;
	};
};

#endif