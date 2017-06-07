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

#ifndef  LEDGER_FETCH_H_
#define  LEDGER_FETCH_H_

#include <common/general.h>
#include <common/storage.h>
#include "ledger_frm.h"
#include <set>

#define  GET_LEDGER_TIMEOUT 30000000

namespace bubi {
	class LedgerManager;
	class LedgerFetch {
		enum BlockState {
			MISSING = 1,
			REQUESTING = 2,
			RECEIVED = 3,
			CONFIRMED = 4,
			CLOSED = 5
		};
		struct FetchWorker;
		struct Segment {
			int64_t	begin_;
			int64_t	end_;
			BlockState state_;
			protocol::Ledgers rec_buf_;
			int64_t pid_;

			Segment(int64_t b, int64_t e, BlockState s) :begin_(b), end_(e), state_(s) {
				assert(begin_ >= 1 && end_ >= begin_);
				pid_ = 0;
			}

			void SetMissing() {
				state_ = MISSING;
				rec_buf_.Clear();
				pid_ = 0;
			}
		};

		struct FetchWorker {
			int64_t pid_;
			int64_t	last_send_time_;
			int64_t delay_;
			int64_t score_;
			int64_t max_ledger_size_;
			int64_t disable_time_;

			FetchWorker(int64_t pid) {
				pid_ = pid;
				last_send_time_ = 0;
				delay_ = 0;
				score_ = 0;
				disable_time_ = 0;
				max_ledger_size_ = Configure::Instance().ledger_configure_.max_ledger_per_message_;
			}
			Json::Value ToJson() const;
			void Disable();
		};
		static std::string BlockStateName(BlockState s);
	public:
		LedgerFetch();
		~LedgerFetch();
		bool Initialize();
		void OnTimer();
		void OnReceiveLedgers(const PeerMessagePointer &message, int64_t peer_id);
		void OnConsent(LedgerFrm::pointer ledger);
		void GetModuleStatus(Json::Value &data) const;
		void ManualFetch(int64_t begin, int64_t end, std::string source);
		bool ReceiveGetLedgers(const PeerMessagePointer &message, int64_t peer_id);
		bool ToDbBuffer(const protocol::Ledger &ledger);
		LedgerFrm::pointer FromDbBuffer(int64_t seq);
		void CreateSnapShot(int64_t ledger_seq);
		bool FromSnapShot(const std::string &filename);
	public:
		LedgerFrm::pointer max_consensus_ledger_;
	private:
		void GetActivePeer();
		void ApplyValidLedger();
		void DeleteTimeOutMsg();
		void RequestMissingLedger(std::shared_ptr<FetchWorker> worker);
		void MergeSegment();
		bool CheckLedgerValidation(const protocol::Ledgers *ledger, std::vector<protocol::Ledger> &frms);
		bool IsReadyApply(int64_t seq);
		int64_t LocalMaxSnapSeq();
		void SnapFethc();
	private:
		std::list<Segment> block_seg_;
		std::map <int64_t, std::shared_ptr<FetchWorker>> workers_;
		std::map<int64_t, std::string> confirm_hash_;
		int64_t last_ontimer_;
		int32_t manual_peer_;
		bool manual_enabled_;
		std::pair<int64_t, int64_t> manual_seq_;
	};
}

#endif 
