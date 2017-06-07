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

#include <overlay/peer_manager.h>
#include <glue/glue_manager.h>
#include "consensus_receiver.h"

//one ledger of 10000Tx could generate about 2M/4M txset
//   4M * 1000 = 4G
//   4M * 10000 = 40G
#define TXSET_CACHE_SIZE 100
//it'd better to store those consented txset only, need to drop local proposed txset
#define TXSET_CACHE_STORAGE_SIZE 100

namespace bubi {
	ConsensusReceiver::ConsensusReceiver(ITxSetFetcherNotify *notify) :
		txset_fetcher_(notify),
		txset_cache_(TXSET_CACHE_SIZE) {}

	ConsensusReceiver::~ConsensusReceiver() {}

	bool ConsensusReceiver::Initialize() {
		return LoadAllTxSet();
	}

	void ConsensusReceiver::PeerDontHave(uint16_t type, std::string const& itemID, int64_t peer_id) {
		utils::MutexGuard guard(lock_);
		if (type == PeerMessage::PEER_MESSAGE_TXSET) {
			txset_fetcher_.DontHave(itemID, peer_id);
		}
		else if (type == PeerMessage::PEER_MESSAGE_TXHASHSET) {
			txset_fetcher_.DontHave(itemID, peer_id);
		}
		else {
			LOG_ERROR("Unknown Type in peerDoesntHave: %u", type);
		}
	}

	void ConsensusReceiver::RecvTxSet(TransactionSetFrmPtr txset) {
		utils::MutexGuard guard(lock_);
		std::string hash = txset->GetContentsHash();
		LOG_INFO("Recv tx set(%s)", utils::String::Bin4ToHexString(hash).c_str());
		txset_cache_.put(hash, txset);
		SaveTxSet(txset);
		txset_fetcher_.Recv(hash);
	}

	void ConsensusReceiver::RecvTxHashSet(const protocol::TxHashSet &txHashSet, std::unordered_set<std::string> &missing_txs) {
		utils::MutexGuard guard(lock_);
		TransactionSetFrmPtr txset = txset_fetcher_.RecvTxHashSet(txHashSet, missing_txs);

		//all the transactions have been received
		if (txset) {
			LOG_INFO("Recv tx set(%s)", utils::String::Bin4ToHexString(txHashSet.hash()).c_str());
			txset_cache_.put(txHashSet.hash(), txset);
			SaveTxSet(txset);
		}
	}

	void ConsensusReceiver::RecvTx(TransactionFrm::pointer &tx_frm) {
		utils::MutexGuard guard(lock_);

		TransactionSetFrmPtr txset = txset_fetcher_.RecvTx(tx_frm);

		if (txset) {
			LOG_INFO("Recv tx set(%s)", utils::String::Bin4ToHexString(tx_frm->txset_hash_).c_str());
			txset_cache_.put(tx_frm->txset_hash_, txset);
			SaveTxSet(txset);
		}
	}

	// called from Peer and when an Item tracker completes
	void ConsensusReceiver::Recv(const ConsensusMsg &msg) {
		utils::MutexGuard guard(lock_);

		std::set<ConsensusMsg> &set = fetching_[msg.GetSeq()];
		do {
			std::set<ConsensusMsg>::const_iterator iter = set.find(msg);
			if (iter != set.end()) {
				//Check if we have fetch the consensus message
				if (HaveFetched(msg)) {
					ConsensusReady(msg);
				}
				break;
			}

			//not found in request
			bool found_receive = false;
			std::vector<ConsensusMsg> &received_list_ = received_[msg.GetSeq()];
			for (size_t i = 0; i < received_list_.size(); i++) {
				if (received_list_[i] == msg) {
					found_receive = true;
				}
			}
			if (found_receive) {
				break;
			}

			received_list_.push_back(msg);

			std::string set_hash;
			//Check if we have fetch the consensus message, if true then return
			if (StartFetch(msg, set_hash)) { // fully fetched
				LOG_INFO("Receive consensus message's hash have fetched");
				ConsensusReady(msg);
			}
			else {
				LOG_INFO("Recv consensus message's hash not exist set hash(%s)", utils::String::Bin4ToHexString(set_hash).c_str());
				set.insert(msg);
			}

		} while (false);
	}

	bool ConsensusReceiver::ConsensusHasRecv(const ConsensusMsg &msg) {
		utils::MutexGuard guard(lock_);
		std::vector<ConsensusMsg> &receivedList = received_[msg.GetSeq()];
		for (size_t i = 0; i < receivedList.size(); i++) {
			if (receivedList[i] == msg) {
				return true;
			}
		}

		return false;
	}

	void ConsensusReceiver::ConsensusReady(const ConsensusMsg &msg) {
		PeerMessagePointer msg_ptr = msg.ComposeNewPeerMessage();
		PeerManager::Instance().Broadcast(msg_ptr);

		utils::MutexGuard guard(lock_);
		processing_[msg.GetSeq()].push_back(msg);

		Global::Instance().GetIoService().post([this]() {
			GlueManager::Instance().ProcessConsensusQueue();
		});
	}

	bool ConsensusReceiver::HaveFetched(const ConsensusMsg &msg) {
		utils::MutexGuard guard(lock_);
		std::vector<protocol::Value> values = msg.GetValues();
		for (std::vector<protocol::Value>::const_iterator iter = values.begin();
			iter != values.end();
			iter++) {
			if (!txset_cache_.exists(iter->hash_set()))
				return false;
		}

		return true;
	}

	// returns true if already fetched
	bool ConsensusReceiver::StartFetch(const ConsensusMsg &msg, std::string &not_exist_hash) {
		bool ret = true;
		utils::MutexGuard guard(lock_);

		std::vector<protocol::Value> values = msg.GetValues();
		for (std::vector<protocol::Value>::const_iterator iter = values.begin();
			iter != values.end();
			iter++) {
			if (!txset_cache_.exists(iter->hash_set())) {
				txset_fetcher_.Fetch(iter->hash_set(), msg);
				not_exist_hash = iter->hash_set();
				ret = false;
			}
		}

		return ret;
	}

	bool ConsensusReceiver::PopBack(int64_t sequence, ConsensusMsg &msg) {
		utils::MutexGuard guard(lock_);
		if (processing_.find(sequence) == processing_.end()) {
			return false;
		}
		else {
			if (!processing_[sequence].size())
				return false;
			msg = processing_[sequence].back();
			processing_[sequence].pop_back();

			return true;
		}
	}

	std::vector<uint64_t> ConsensusReceiver::readySlots() {
		utils::MutexGuard guard(lock_);
		std::vector<uint64_t> result;
		for (auto const& entry : processing_) {
			if (!entry.second.empty())
				result.push_back(entry.first);
		}
		return result;
	}

	void ConsensusReceiver::EraseLow(int64_t sequence) {
		LOG_TRACE("Erase all below  of the sequence(" FMT_I64 ")", sequence);
		utils::MutexGuard guard(lock_);
		for (auto iter = processing_.begin();
			iter != processing_.end();) {
			if (iter->first == 0) {
				iter++;
				continue;
			}
			if (iter->first < sequence && iter->first != 0) {
				iter = processing_.erase(iter);
			}
			else
				break;
		}

		for (auto iter = fetching_.begin();
			iter != fetching_.end();) {
			if (iter->first == 0) {
				iter++;
				continue;
			}

			if (iter->first < sequence) {
				iter = fetching_.erase(iter);
			}
			else
				break;
		}
	}

	void ConsensusReceiver::Erase(int64_t sequence) {
		LOG_TRACE("Erase the sequence(" FMT_I64 ")", sequence);
		utils::MutexGuard guard(lock_);

		processing_.erase(sequence);

		// keep the last few ledgers worth of messages around to give to people
		received_.erase(sequence - g_creceiver_deloffset_);
		fetching_.erase(sequence);

		txset_fetcher_.StopFetch(sequence + 1);
	}

	TransactionSetFrmPtr ConsensusReceiver::GetTxSet(const std::string &hash) {
		utils::MutexGuard guard(lock_);
		if (txset_cache_.exists(hash)) {
			return txset_cache_.get(hash);
		}

		return NULL;
	}

	void ConsensusReceiver::GetModuleStatus(Json::Value &data) {
		utils::MutexGuard guard(lock_);
		data["recv_size"] = received_.size();
		data["fetch_size"] = fetching_.size();

		size_t processing_size = 0;
		for (auto &item : processing_) {
			processing_size += item.second.size();
		}
		data["processing_size"] = processing_size;
		data["txset_cache_size"] = txset_cache_.size();

		size_t txset_txcatchesize = 0;
		for (auto &item : txset_cache_.GetList()) {
			txset_txcatchesize += item.second->size();
		}
		data["txset_cachetx_size"] = txset_txcatchesize;

		txset_fetcher_.GetModuleStatus(data["txset_fetch"]);
	}

	void ConsensusReceiver::OnTimer(int64_t current_time) {
		utils::MutexGuard guard(lock_);
		txset_fetcher_.OnTimer(current_time);
	}

	bool ConsensusReceiver::LoadAllTxSet() {
		KeyValueDb *db = Storage::Instance().keyvalue_db();
		std::string key_list_key = utils::String::Format("%s_keys", bubi::General::TXSET_PREFIX);
		do {
			std::string key_list;
			if (!db->Get(key_list_key, key_list)) {
				LOG_INFO("Load tx set key(%s) from db nothing", key_list_key.c_str());
				return true;
			}

			protocol::EntryList key_list_proto;
			if (!key_list_proto.ParseFromString(key_list)) {
				LOG_ERROR("Parse form key(%s) from db failed", key_list_key.c_str());
				break;
			}

			LOG_INFO("Loading all tx set, size(%d)", key_list_proto.entry_size());

			for (int32_t i = 0; i < key_list_proto.entry_size(); i++) {
				std::string key = utils::String::Format("%s_%s", bubi::General::TXSET_PREFIX,
					utils::String::BinToHexString(key_list_proto.entry(i)).c_str());
				std::string value;

				if (!db->Get(key, value)) {
					LOG_ERROR("Get hash set key(%s) from db failed when load all txset", key.c_str());
					continue;
				}

				protocol::TxSet txset;
				if (!txset.ParseFromString(value)) {
					LOG_ERROR("Parse from key(%s) failed when load all txset", key.c_str());
					continue;
				}

				TransactionSetFrmPtr ptr = std::make_shared<TransactionSetFrm>(txset);
				txset_cache_.put(ptr->GetContentsHash(), ptr);
				LOG_TRACE("Loaded tx set hash(%s)", utils::String::Bin4ToHexString(ptr->GetContentsHash()).c_str());
			}

			return true;
		} while (false);

		return false;
	}

	bool ConsensusReceiver::SaveTxSet(TransactionSetFrmPtr txset) {
		KeyValueDb *db = Storage::Instance().keyvalue_db();
		std::string key_list_key = utils::String::Format("%s_keys", bubi::General::TXSET_PREFIX);
		do {
			std::string key_list;
			protocol::EntryList key_list_proto;

			if (db->Get(key_list_key, key_list)) {
				if (!key_list_proto.ParseFromString(key_list)) {
					LOG_ERROR("Parse form key(%s) from db failed", key_list_key.c_str());
					break;
				}
			}

			if (key_list_proto.entry_size() > TXSET_CACHE_STORAGE_SIZE + 100) {
				protocol::EntryList new_entry;
				 
				for (int32_t i = 0; i < 100; i++) {
					db->Delete(key_list_proto.entry(i));
				}

				for (int32_t i = 100; i < key_list_proto.entry_size(); i++) {
					new_entry.add_entry(key_list_proto.entry(i));
				}

				key_list_proto = new_entry;
			}

			key_list_proto.add_entry(txset->GetContentsHash());
			db->Put(key_list_key, key_list_proto.SerializeAsString());

			std::string key = utils::String::Format("%s_%s", bubi::General::TXSET_PREFIX,
				utils::String::BinToHexString(txset->GetContentsHash()).c_str());

			protocol::TxSet txset_proto;
			txset->ToProtobuf(txset_proto);
			db->Put(key, txset_proto.SerializeAsString());

			return true;
		} while (false);

		return false;
	}

	bool ConsensusReceiver::WantedTxSet(const std::string txset_hash) {
		return txset_fetcher_.Fetching(txset_hash);
	}

	bool ConsensusReceiver::WantedTx(const std::string txset_hash, const std::string tx_hash) {
		return txset_fetcher_.Fetching(txset_hash, tx_hash);
	}
}
