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

#ifndef TXSET_FETCHER_H_
#define TXSET_FETCHER_H_

#include <utils/headers.h>
#include <common/general.h>
#include <consensus/consensus_msg.h>
#include "transaction_set_frm.h"

namespace bubi {
	class TxSetFetcherItem {
	public:
		TxSetFetcherItem(const std::string &hash);
		~TxSetFetcherItem();
		void InsertConsensusMsg(const ConsensusMsg &consensus_msg);
	public:
		std::string hash_;
		std::vector<ConsensusMsg> consensus_messages_;
		std::vector<int64_t> peer_ids_;
		int64_t last_send_time_;
		bool fetched_;
		TransactionSetFrmPtr sync_txset_;
		std::unordered_set<std::string> missing_txhash_set_;
	};

	typedef std::map<std::string, TxSetFetcherItem> TxSetFetcherMap;

	class ITxSetFetcherNotify {
	public:
		ITxSetFetcherNotify() {};
		~ITxSetFetcherNotify() {};
		virtual void OnTxSetFetched(const ConsensusMsg consensus_msg) = 0;
	};

	class TxSetFetcher : public utils::NonCopyable {
	public:
		TxSetFetcher(ITxSetFetcherNotify *notify);
		~TxSetFetcher();
		void Fetch(const std::string &hash, const ConsensusMsg &consensus_msg);
		void Recv(const std::string &hash);
		void DontHave(const std::string &hash, int64_t peer_id);
		void StopFetch(int64_t sequence);
		void GetModuleStatus(Json::Value &data) const;
		void OnTimer(int64_t current_time);
		bool Fetching(const std::string &hash);
		bool Fetching(const std::string &txset_hash, const std::string tx_hash);
		TransactionSetFrmPtr RecvTx(TransactionFrm::pointer &tx_frm);
		TransactionSetFrmPtr RecvTxHashSet(const protocol::TxHashSet &txHashSet, std::unordered_set<std::string> &missing_txs);
	private:
		TxSetFetcherMap items_;
		ITxSetFetcherNotify *notify_;
	};
}

#endif //TXSET_FETCHER_H_