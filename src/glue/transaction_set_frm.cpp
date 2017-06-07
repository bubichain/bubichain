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

#include <common/configure.h>
#include <ledger/ledger_manager.h>
#include "transaction_set_frm.h"

namespace bubi {
	TransactionSetFrm::TransactionSetFrm(std::string const& previous_ledger_hash)
		: hash_is_valid_(false), previous_ledger_hash_(previous_ledger_hash) {
		utils::AtomicInc(&bubi::General::txset_new_count);
	}

	TransactionSetFrm::TransactionSetFrm(protocol::TxSet const& tran_envs) :
		hash_is_valid_(false) {
		for (int i = 0; i < tran_envs.tran_envs_size(); i++) {
			auto const& envelope = tran_envs.tran_envs(i);
			TransactionFrm::pointer tx = std::make_shared<TransactionFrm>(envelope);
			Add(tx);
		}
		previous_ledger_hash_ = tran_envs.previous_ledger_hash();
		utils::AtomicInc(&bubi::General::txset_new_count);
	}

	TransactionSetFrm::~TransactionSetFrm() {
		utils::AtomicInc(&bubi::General::txset_delete_count);
	}

	void TransactionSetFrm::Add(TransactionFrm::pointer tx) {
		txset_by_acc_seq_.insert(tx);
		txmap_by_hash_[tx->GetFullHash()] = tx;
		hash_is_valid_ = false;
	}

	size_t TransactionSetFrm::size() {
		return txset_by_acc_seq_.size();
	}

	// TODO.3 this and checkValid share a lot of code
	void TransactionSetFrm::TrimInvalid(std::vector<TransactionFrm::pointer>& trimmed) {
		std::string acc;
		bubi::AccountFrm::pointer account = NULL;
		int64_t seq = 0;
		int64_t totalFee = 0;

		for (TxSetByAccountSeq::iterator it = txset_by_acc_seq_.begin();
			it != txset_by_acc_seq_.end();) {
			TransactionFrm::pointer tx = *it;
			if (acc != tx->GetSourceAddress()) {
				if (bubi::LedgerManager::Instance().GetAccountEntry(tx->GetSourceAddress(), account)) {
					seq = account->GetAccountTxSeq();
					totalFee = 0;
					acc = tx->GetSourceAddress();
				}
				else {
					tx->result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
					trimmed.push_back(tx);
					txmap_by_hash_.erase(tx->GetFullHash());
					txset_by_acc_seq_.erase(it++);
					hash_is_valid_ = false;
					continue;
				}
			}

			if (totalFee + tx->GetFee() + LATEST_BASE_RESERVE > account->GetAccountBalance()) {
				tx->result_.set_code(protocol::ERRCODE_ACCOUNT_LOW_RESERVE);
				trimmed.push_back(tx);
				txmap_by_hash_.erase(tx->GetFullHash());
				txset_by_acc_seq_.erase(it++);
				hash_is_valid_ = false;
			}
			else {
				totalFee += tx->GetFee();
				it++;
			}
		}
	}

	// need to make sure every account that is submitting a tx has enough to pay
	// the fees of all the tx it has submitted in this set
	int32_t TransactionSetFrm::CheckValid() {
		protocol::LedgerHeader lcl = bubi::LedgerManager::Instance().GetLastClosedLedger();
		if (lcl.hash() != previous_ledger_hash_) {
			LOG_WARN("Got bad txSet: %s ; expected: %s", utils::String::Bin4ToHexString(previous_ledger_hash_).c_str(),
				utils::String::Bin4ToHexString(lcl.hash()).c_str());
			return 0;
		}

		return 1;
	}

	std::string TransactionSetFrm::GetContentsHash() {
		if (!hash_is_valid_) {
			utils::Sha256 sha256;
			sha256.Update(previous_ledger_hash_);
			for (auto const& tx : txset_by_acc_seq_) {
				sha256.Update(tx->GetFullHash());
			}
			hash_ = sha256.Final();
			hash_is_valid_ = true;
		}
		return hash_;
	}

	bool TransactionSetFrm::CheckTxSetHash(const protocol::TxHashSet *hashSet) {
		if (hashSet) {
			utils::Sha256 sha256;
			sha256.Update(hashSet->previous_ledger_hash());
			for (auto const& txFullHash : hashSet->hashs()) {
				sha256.Update(txFullHash);
			}

			std::string res = sha256.Final();

			LOG_INFO("<hash=%s,phash=%s,count=%d,Final=%s>", utils::String::Bin4ToHexString(hashSet->hash()).c_str(),
				utils::String::Bin4ToHexString(hashSet->previous_ledger_hash()).c_str(), hashSet->hashs_size(), utils::String::Bin4ToHexString(res).c_str());

			return hashSet->hash() == res;
		}
		return false;
	}

	std::string &TransactionSetFrm::PreviousLedgerHash() {
		hash_is_valid_ = false;
		return previous_ledger_hash_;
	}

	std::string const& TransactionSetFrm::PreviousLedgerHash() const {
		return previous_ledger_hash_;
	}

	void TransactionSetFrm::ToProtobuf(protocol::TxSet &txSet) {
		for (auto const& tx : txset_by_acc_seq_) {
			*txSet.add_tran_envs() = tx->GetTransactionEnv();
		}
		txSet.set_previous_ledger_hash(previous_ledger_hash_);
		txSet.set_hash(GetContentsHash());
	}
}