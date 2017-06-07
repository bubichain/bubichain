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

#ifndef TRANSACTION_SET_FRM_H_
#define TRANSACTION_SET_FRM_H_

#include <unordered_set>
#include <ledger/transaction_frm.h>

#define LATEST_BASE_RESERVE (int64_t)bubi::Configure::Instance().ledger_configure_.base_reserve_

namespace bubi {
	struct TxLess {
	public:
		bool operator()(const TransactionFrm::pointer left, const TransactionFrm::pointer right) {
			const std::string &leftAddr = left->GetSourceAddress();
			const std::string &rightAddr = right->GetSourceAddress();
			return  leftAddr == rightAddr ? left->GetSequnce() < right->GetSequnce() : leftAddr < rightAddr;
		}
	};

	struct TxHashCode {
	public:
		size_t operator()(const TransactionFrm::pointer tx) const {
			return tx->GetFullHash()[0];
		}
	};

	struct TxCmp {
	public:
		bool operator()(const TransactionFrm::pointer left, const TransactionFrm::pointer right) {
			return  left->GetFullHash() == right->GetFullHash();
		}
	};

	class TransactionSetFrm {
		DISALLOW_COPY_AND_ASSIGN(TransactionSetFrm);
	public:
		typedef std::set<TransactionFrm::pointer, TxLess> TxSetByAccountSeq;
		typedef std::unordered_map<std::string, TransactionFrm::pointer> TxMapByHash;
		TransactionSetFrm(std::string const &previous_ledger_hash);
		TransactionSetFrm(protocol::TxSet const &tran_envs);
		virtual ~TransactionSetFrm();
		std::string GetContentsHash();
		static bool CheckTxSetHash(const protocol::TxHashSet *hashSet);
		std::string& PreviousLedgerHash();
		std::string const& PreviousLedgerHash() const;
		int32_t CheckValid();
		void TrimInvalid(std::vector<TransactionFrm::pointer>& trimmed);
		void Add(TransactionFrm::pointer tx);
		size_t size();
		void ToProtobuf(protocol::TxSet& set);
	public:
		TxSetByAccountSeq txset_by_acc_seq_;
		TxMapByHash txmap_by_hash_;
	private:
		bool hash_is_valid_;
		bool tx_is_valid_;
		std::string hash_;
		std::string previous_ledger_hash_;
	};
	typedef std::shared_ptr<TransactionSetFrm> TransactionSetFrmPtr;
};

#endif