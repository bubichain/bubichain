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

#ifndef CONSENSUS_RECEIVER_H_
#define CONSENSUS_RECEIVER_H_

#include <utils/headers.h>
#include <utils/lrucache.hpp>
#include <json/json.h>
#include <common/general.h>
#include "transaction_set_frm.h"
#include "txset_fetcher.h"

namespace bubi {
	const int64_t g_creceiver_deloffset_ = 15;

	class ConsensusReceiver {
	public:
		ConsensusReceiver(ITxSetFetcherNotify *notify);
		~ConsensusReceiver();
		bool Initialize();
		void Recv(const ConsensusMsg &msg);
		void RecvTx(TransactionFrm::pointer &tx_frm);
		bool WantedTxSet(const std::string txset_hash);
		bool WantedTx(const std::string txset_hash, const std::string tx_hash);
		void RecvTxHashSet(const protocol::TxHashSet &txHashSet, std::unordered_set<std::string> &missing_txs);
		bool ConsensusHasRecv(const ConsensusMsg &msg);
		//receive transaction set
		void RecvTxSet(TransactionSetFrmPtr txset);
		void PeerDontHave(uint16_t type, std::string const& itemID, int64_t peer_id);
		//if have fetched,then return true
		bool HaveFetched(const ConsensusMsg &msg);
		bool StartFetch(const ConsensusMsg &msg, std::string &not_exist_hash);
		//transaction set that about consensus have fetched, then ready to go
		void ConsensusReady(const ConsensusMsg &msg);
		//pop back the consensus message
		bool PopBack(int64_t sequence, ConsensusMsg &msg);
		//clear obejct below the sequence
		void EraseLow(int64_t sequence);
		//clear object equal the sequence
		void Erase(int64_t sequence);
		std::vector<uint64_t> readySlots();
		//Get transaction set by hash
		TransactionSetFrmPtr GetTxSet(const std::string &hash);
		void GetModuleStatus(Json::Value &data);
		void OnTimer(int64_t current_time);
		//store the transaction set
		bool SaveTxSet(TransactionSetFrmPtr txset);
		bool LoadAllTxSet();
	private:
		//the list that have received
		std::map<int64_t, std::vector<ConsensusMsg>> received_;
		//the list that need to be request
		std::map<int64_t, std::set<ConsensusMsg>> fetching_;
		//the list than we need to process
		std::map<int64_t, std::vector<ConsensusMsg>> processing_;
		TxSetFetcher txset_fetcher_;
		cache::lru_cache<std::string, TransactionSetFrmPtr> txset_cache_;
		utils::Mutex lock_;
	};
}

#endif