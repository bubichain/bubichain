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

#include <proto/message.pb.h>
#include <overlay/peer_manager.h>
#include "txset_fetcher.h"
#include "glue_manager.h"

namespace bubi {
	TxSetFetcherItem::TxSetFetcherItem(const std::string &hash) :
		hash_(hash),
		consensus_messages_(),
		peer_ids_(),
		last_send_time_(0),
		fetched_(false),
		sync_txset_(NULL),
		missing_txhash_set_() {}

	TxSetFetcherItem::~TxSetFetcherItem() {}

	void TxSetFetcherItem::InsertConsensusMsg(const ConsensusMsg &consensus_msg) {
		bool found = false;
		for (size_t i = 0; i < consensus_messages_.size(); i++) {
			if (consensus_messages_[i] == consensus_msg) {
				found = true;
				break;
			}
		}

		if (!found) {
			consensus_messages_.push_back(consensus_msg);
		}
	}

	TxSetFetcher::TxSetFetcher(ITxSetFetcherNotify *notify) : notify_(notify) {}

	TxSetFetcher::~TxSetFetcher() {}

	void TxSetFetcher::Fetch(const std::string &hash, const ConsensusMsg &consensus_msg) {
		TxSetFetcherMap::iterator iter = items_.find(hash);
		if (iter != items_.end()) {
			TxSetFetcherItem &item = iter->second;
			item.InsertConsensusMsg(consensus_msg);
		}
		else {
			TxSetFetcherItem item(hash);
			item.InsertConsensusMsg(consensus_msg);
			items_.insert(std::make_pair(hash, item));
		}
	}

	void TxSetFetcher::Recv(const std::string &hash) {
		TxSetFetcherMap::iterator iter = items_.find(hash);
		if (iter != items_.end()) {
			TxSetFetcherItem &item = iter->second;
			item.fetched_ = true;
			for (size_t i = 0; i < item.consensus_messages_.size(); i++) {
				ConsensusMsg msg = item.consensus_messages_[i];
				Global::Instance().GetIoService().post([msg, this]() {
					notify_->OnTxSetFetched(msg);
				});
			}
			item.consensus_messages_.clear();
		}
	}

	void TxSetFetcher::DontHave(const std::string &hash, int64_t peer_id) {
		TxSetFetcherMap::iterator iter = items_.find(hash);
		if (iter != items_.end()) {
			TxSetFetcherItem &item = iter->second;
			if (item.peer_ids_.empty()) {
				return;
			}

			int64_t cur_id = item.peer_ids_.back();
			item.peer_ids_.pop_back();

			item.last_send_time_ = utils::Timestamp::HighResolution();
			auto msg = PeerMessage::NewGetTxHashSet();
			((protocol::GetTxHashSet *)msg->data_)->set_hash(hash);
			PeerManager::Instance().SendMessage(cur_id, msg);
		}
	}

	void TxSetFetcher::StopFetch(int64_t sequence) {
		for (TxSetFetcherMap::iterator iter = items_.begin();
			iter != items_.end();) {
			TxSetFetcherItem &item = iter->second;

			for (std::vector<ConsensusMsg>::iterator iter_msg = item.consensus_messages_.begin();
				iter_msg != item.consensus_messages_.end();) {
				const ConsensusMsg &msg = *iter_msg;
				if (msg.GetSeq() < sequence) {
					iter_msg = item.consensus_messages_.erase(iter_msg);
				}
				else {
					iter_msg++;
				}
			}

			if (item.consensus_messages_.empty()) {
				iter = items_.erase(iter);
			}
			else {
				iter++;
			}
		}
	}

	void TxSetFetcher::OnTimer(int64_t current_time) {
		for (TxSetFetcherMap::iterator iter = items_.begin();
			iter != items_.end();
			iter++) {
			TxSetFetcherItem &item = iter->second;
			if ((current_time - item.last_send_time_ < 5 * utils::MICRO_UNITS_PER_SEC) || item.fetched_) {
				continue;
			}

			//get the peer ids from peer manager
			if (item.peer_ids_.empty()) {

				PeerNetwork &network = PeerManager::Instance().TransactionNetwork();
				Json::Value peers = Json::Value(Json::arrayValue);
				network.GetPeers(peers);
				for (size_t i = 0; i < peers.size(); i++) {
					const Json::Value &peer_item = peers[i];
					if (peer_item["is_active"].asBool()) {
						item.peer_ids_.push_back(peer_item["id"].asInt64());
					}
				}

				std::random_shuffle(item.peer_ids_.begin(), item.peer_ids_.end());
			}

			if (item.peer_ids_.empty()) continue;

			//get the first id then send
			int64_t cur_id = item.peer_ids_.back();
			item.peer_ids_.pop_back();
			item.last_send_time_ = utils::Timestamp::HighResolution();
			auto msg = PeerMessage::NewGetTxHashSet();
			((protocol::GetTxHashSet *)msg->data_)->set_hash(item.hash_);
			PeerManager::Instance().SendMessage(cur_id, msg);
		}
	}

	void TxSetFetcher::GetModuleStatus(Json::Value &data) const {
		data["tracker_size"] = items_.size();

		size_t waiting_envsize = 0;
		for (auto &item : items_) {
			waiting_envsize += item.second.consensus_messages_.size();
		}
		data["waitingenv_size"] = waiting_envsize;
	}

	bool TxSetFetcher::Fetching(const std::string &hash) {
		return items_.find(hash) != items_.end();
	}

	bool TxSetFetcher::Fetching(const std::string &txset_hash, const std::string tx_hash) {
		TxSetFetcherMap::iterator it = items_.find(txset_hash);

		if (it != items_.end()
			&& it->second.missing_txhash_set_.find(tx_hash) == it->second.missing_txhash_set_.end()) {
			return true;
		}
		return false;
	}

	TransactionSetFrmPtr TxSetFetcher::RecvTxHashSet(const protocol::TxHashSet &txHashSet, std::unordered_set<std::string> &missing_txs) {
		std::map<std::string, TxSetFetcherItem>::iterator it = items_.find(txHashSet.hash());
		if (it == items_.end()) {
			return NULL;
		}

		auto &sync_txset = it->second.sync_txset_;
		auto &missing_txhash_set = it->second.missing_txhash_set_;

		if (missing_txhash_set.size() > 0) {
			missing_txs = missing_txhash_set;
			return NULL;
		}

		sync_txset = std::make_shared<TransactionSetFrm>(txHashSet.previous_ledger_hash());
		for (::google::protobuf::RepeatedPtrField< ::std::string>::const_iterator it = txHashSet.hashs().begin();
			it != txHashSet.hashs().end(); it++) {
			TransactionFrm::pointer tx = bubi::GlueManager::Instance().GetTx(*it);
			if (tx) {
				sync_txset->Add(tx);
			}
			else {
				missing_txhash_set.insert(*it);
			}
		}

		if (missing_txhash_set.size() == 0) {
			std::string hash = sync_txset->GetContentsHash();
			LOG_INFO("Done with recv tx set hash(%s), %d tx", utils::String::Bin4ToHexString(hash).c_str(), sync_txset->size());

			Recv(hash);
			return sync_txset;
		}

		missing_txs = missing_txhash_set;

		return NULL;
	}

	TransactionSetFrmPtr TxSetFetcher::RecvTx(TransactionFrm::pointer &tx_frm) {
		std::map<std::string, TxSetFetcherItem>::iterator it = items_.find(tx_frm->txset_hash_);
		if (it == items_.end()) {
			return NULL;
		}

		auto &sync_txset = it->second.sync_txset_;
		auto &missing_txhash_set = it->second.missing_txhash_set_;

		std::unordered_set<std::string>::iterator itr = missing_txhash_set.find(tx_frm->GetFullHash());
		if (itr != missing_txhash_set.end()) {
			missing_txhash_set.erase(itr);
			sync_txset->Add(tx_frm);
		}

		if (missing_txhash_set.size() == 0) {
			std::string hash = sync_txset->GetContentsHash();
			LOG_INFO("Done with recv tx set hash(%s)", utils::String::Bin4ToHexString(hash).c_str());

			Recv(hash);
			return sync_txset;
		}
		return NULL;
	}
}