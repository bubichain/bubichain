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

#include <utils/headers.h>
#include <common/configure.h>
#include <common/general.h>
#include <overlay/peer_manager.h>
#include <ledger/ledger_manager.h>
#include "glue_manager.h"
//notice
#include "monitor/monitor_master.h"
#include "api/mq_server.h"
#include "api/websocket_server.h"

namespace bubi {
	TransactionFrmLow::TransactionFrmLow() : max_seq_(0), total_fees_(0) {
		utils::AtomicInc(&bubi::General::trans_low_new_count);
	}

	TransactionFrmLow::~TransactionFrmLow() {
		utils::AtomicInc(&bubi::General::trans_low_delete_count);
	}

	static size_t add_hash_map = 0;
	static size_t del_hash_map = 0;
	bool TransactionFrmLow::Add(const TransactionFrm::pointer &tran) {
		auto const& h = tran->GetSequnce();
		if (txmap_by_seq_.find(h) != txmap_by_seq_.end()) {
			return false;
		}
		txmap_by_seq_.insert(std::make_pair(h, tran));
		max_seq_ = MAX(tran->GetSequnce(), max_seq_);
		total_fees_ += tran->GetFee();

		//add to hash map
		if (GlueManager::Instance().transactions_by_hash_.find(tran->GetFullHash()) == GlueManager::Instance().transactions_by_hash_.end()) {
			LOG_TRACE("Add transaction(%s)", utils::String::BinToHexString(tran->GetContentHash()).c_str());
			GlueManager::Instance().transactions_by_hash_[tran->GetFullHash()] = tran;
			add_hash_map++;
		}

		return true;
	}

	bool TransactionFrmLow::Del(const TransactionFrm::pointer &tran) {
		std::map<int64_t, TransactionFrm::pointer>::iterator iter = txmap_by_seq_.find(tran->GetSequnce());
		if (iter != txmap_by_seq_.end()) {
			LOG_TRACE("Remove transaction(%s)", utils::String::BinToHexString(tran->GetContentHash()).c_str());
			//erase hash map
			TransactionSetFrm::TxMapByHash::iterator del = GlueManager::Instance().transactions_by_hash_.find(tran->GetFullHash());
			if (del != GlueManager::Instance().transactions_by_hash_.end()) {
				GlueManager::Instance().transactions_by_hash_.erase(tran->GetFullHash());
				del_hash_map++;
			}

			txmap_by_seq_.erase(iter);
			return true;
		}

		return false;
	}

	bool TransactionFrmLow::Empty() const {
		return txmap_by_seq_.empty();
	}

	void TransactionFrmLow::Recalculate() {
		max_seq_ = 0;
		total_fees_ = 0;
		for (auto const& pair : txmap_by_seq_) {
			max_seq_ = MAX(pair.second->GetSequnce(), max_seq_);
			total_fees_ += pair.second->GetFee();
		}
	}

	TransactionFrm::pointer TransactionFrmLow::GetTransaction(int64_t sequence) {
		std::map<int64_t, TransactionFrm::pointer>::iterator iter = txmap_by_seq_.find(sequence);
		if (iter != txmap_by_seq_.end()) {
			return iter->second;
		}

		return NULL;
	}

	std::map<int64_t, TransactionFrm::pointer> TransactionFrmLow::RemoveTimeout(int64_t expire_time) {
		std::map<int64_t, TransactionFrm::pointer> ret;
		for (std::map<int64_t, TransactionFrm::pointer>::iterator iter = txmap_by_seq_.begin(); iter != txmap_by_seq_.end();) {
			if (iter->second->CheckTimeout(expire_time)) {
				LOG_ERROR("Remove timeout transaction(%s), account(%s) sequence(" FMT_I64 ")",
					utils::String::Bin4ToHexString(iter->second->GetContentHash()).c_str(),
					iter->second->GetSourceAddress().c_str(), iter->second->GetSequnce());
				//erase hash map
				TransactionSetFrm::TxMapByHash::iterator del = GlueManager::Instance().transactions_by_hash_.find(iter->second->GetFullHash());
				if (del != GlueManager::Instance().transactions_by_hash_.end()) {
					GlueManager::Instance().transactions_by_hash_.erase(iter->second->GetFullHash());
					del_hash_map++;
				}

				ret.insert(*iter);
				iter = txmap_by_seq_.erase(iter);
			}
			else {
				iter++;
			}
		}

		return ret;
	}

	void TransactionFrmLow::MoveFrom(TransactionFrmLow &low) {
		for (auto const& tx : low.txmap_by_seq_) {
			Add(tx.second);
		}
		low.txmap_by_seq_.clear();
	}

	void TransactionFrmLow::GetProposeTxs(int64_t sequence, TransactionSetFrmPtr &proposed_set) {
		for (std::map<int64_t, TransactionFrm::pointer>::const_iterator iter = txmap_by_seq_.begin();
			iter != txmap_by_seq_.end();) {
			if (sequence > iter->second->GetSequnce() - 1) { //erase

				LOG_ERROR("Remove error seq transaction(%s), account(%s) sequence(" FMT_I64 ")",
					utils::String::Bin4ToHexString(iter->second->GetContentHash()).c_str(),
					iter->second->GetSourceAddress().c_str(), iter->second->GetSequnce());
				//erase hash map
				TransactionSetFrm::TxMapByHash::iterator del = GlueManager::Instance().transactions_by_hash_.find(iter->second->GetFullHash());
				if (del != GlueManager::Instance().transactions_by_hash_.end()) {
					GlueManager::Instance().transactions_by_hash_.erase(iter->second->GetFullHash());
					del_hash_map++;
				}

				iter = txmap_by_seq_.erase(iter);
			}
			else if (sequence == iter->second->GetSequnce() - 1) {
				proposed_set->Add(iter->second);

				iter++;
				sequence++;
			}
			else {
				LOG_WARN("Get tx from account(%s) stopped as tx sequence(" FMT_I64 ") is not increase one",
					iter->second->GetSourceAddress().c_str(), iter->second->GetSequnce());
				break;
			}
		}
	}

	GlueConsensusData::GlueConsensusData(int64_t index, const protocol::Value &value){
		block_seq_ = index;
		value_ = value;
		next_ledger_state_ = State::NONE;
		sequence_ = 0;
	}

	GlueConsensusData::~GlueConsensusData(){
	}

	int64_t const  GlueManager::MAX_LEDGER_TIMESPAN_SECONDS = 120 * utils::MICRO_UNITS_PER_SEC;
	uint64_t const  GlueManager::MAX_TIME_SLIP_MICROSECONDS = 120 * utils::MICRO_UNITS_PER_SEC;
	uint64_t const  GlueManager::MAX_TRANSACTION_KEEPTIME_MICROSECONDS = 300 * utils::MICRO_UNITS_PER_SEC;

	GlueManager::GlueManager() :
		valid_txset_cache_(50),
		ledgerclose_check_timer_(0),
		transactions_(),
		transactions_by_hash_() {
		delay_consensus_ = 0;

		next_ledger_num_ = 2;
		next_consensus_can_trigger_ = false;
		empty_transaction_times_ = 0;
		trigger_timer_ = 0;
		check_interval_ = 500 * utils::MICRO_UNITS_PER_MILLI;
		timer_name_ = "Glue Manager";
	}

	GlueManager::~GlueManager() {}

	bool GlueManager::Initialize() {
		const ValidationConfigure &validation_config = Configure::Instance().validation_configure_;
		process_uptime_ = time(NULL);
		do {
			consensus_ = ConsensusManager::Instance().GetConsensus();
			consensus_->SetNotify(this);
			consensus_receiver_ = std::make_shared<ConsensusReceiver>(this);
			if (!consensus_receiver_->Initialize()) {
				LOG_ERROR("Initialize consensus receiver failed");
				break;
			}

			next_ledger_num_ = LedgerManager::Instance().GetLastClosedLedger().ledger_sequence() + 1;

			delay_consensus_ = Configure::Instance().system_config_.delay_consensus_;
			transactions_.resize(delay_consensus_ + Configure::Instance().ledger_configure_.max_trans_in_memory_ / Configure::Instance().ledger_configure_.max_trans_per_ledger_);
			for (size_t i = 0; i < transactions_.size(); i++) {
				transactions_[i] = new TransactionFrmMid();
			}

			auto const& lcl = LedgerManager::Instance().GetLastClosedLedger();
			tracking_consensus_ = utils::make_unique<GlueConsensusData>(lcl.ledger_sequence(), lcl.consensus_value());

			if (consensus_->IsValidator()) {
				LOG_INFO("Start consensus,node address(%s) ", consensus_->GetNodeAddress().c_str());
				last_trigger_time_ = utils::Timestamp::HighResolution();
				if (consensus_->RepairStatus())ScheduleNextLedger(true);
			}

			TimerNotify::RegisterModule(this);
			StatusModule::RegisterModule(this);

			return true;
		} while (false);
		return false;
	}

	bool GlueManager::Exit() {
		LOG_INFO("GlueManager stoping...");
		for (size_t i = 0; i < transactions_.size(); i++) {
			delete transactions_[i];
		}
		LOG_INFO("GlueManager stop [OK]");
		return true;
	}

	bool GlueManager::ReceiveTransaction(TransactionFrm::pointer &tx_frm, int64_t* ledger_seq) {
		std::string hash_value = tx_frm->GetContentHash();
		std::string address = tx_frm->GetSourceAddress();
		Result result;
		bool status = false;
		
		do {

			if (Configure::Instance().ledger_configure_.max_trans_in_memory_ <=
				GetCachedTxSize()) {
				result.set_code(protocol::ERRCODE_OUT_OF_TXCACHE);
				result.set_desc("too much transactions");
				LOG_ERROR("Too much transactions,transaction hash(%s)", utils::String::Bin4ToHexString(hash_value).c_str());
				break;
			}

			int64_t totFee = tx_frm->GetFee();

			bool duplicate = false;
			do {
				utils::ReadLockGuard guard(trans_mutex_);

				for (auto& map : transactions_) {
					TransactionFrmMid::iterator i = map->find(address);
					if (i != map->end()) {
						std::shared_ptr<TransactionFrmLow> &txmap = i->second;
						if (txmap->GetTransaction(tx_frm->GetSequnce())) {
							result.set_code(protocol::ERRCODE_ALREADY_EXIST);
							result.set_desc(utils::String::Format("Receive duplicate transaction, source address(%s) hash(%s)", address.c_str(), utils::String::Bin4ToHexString(hash_value).c_str()));
							LOG_ERROR("Receive duplicate transaction, source address(%s) hash(%s)", address.c_str(), utils::String::Bin4ToHexString(hash_value).c_str());
							duplicate = true;
							break;
						}
						totFee += txmap->GetTotalFees();
					}
				}
			} while (false);

			if (duplicate) break;

			if (!tx_frm->CheckValid(-1)) {
				result = tx_frm->GetResult();
				Json::Value js;
				js["action"] = "apply";
				js["error_code"] = result.code();
				js["desc"] = result.desc();
				LOG_ERROR("Check transaction failed, source address(%s) hash(%s), return(%s)",
					address.c_str(), utils::String::Bin4ToHexString(hash_value).c_str(), js.toFastString().c_str());
				break;
			}

			int64_t t3 = utils::Timestamp::HighResolution();

			if (tx_frm->GetSourceAccount()->GetAccountBalance() - totFee < LATEST_BASE_RESERVE) {
				LOG_ERROR("Account reserve ballance not enough for transaction fee and base reserve:" FMT_I64 "- " FMT_I64 " < %d,last transaction hash(%s)",
					tx_frm->GetSourceAccount()->GetAccountBalance(), totFee, LATEST_BASE_RESERVE, utils::String::Bin4ToHexString(hash_value).c_str());
				result.set_code(protocol::ERRCODE_ACCOUNT_LOW_RESERVE);
				result.set_desc("source account balance is not enough");
				break;
			}

			{
				utils::WriteLockGuard guard(trans_mutex_);
				if (*ledger_seq == 0) {
					size_t i = delay_consensus_;
					for (; i < transactions_.size(); i++) {
						if (transactions_[i]->size() < Configure::Instance().ledger_configure_.max_trans_per_ledger_) {
							break;
						}
					}

					if (i == transactions_.size()) {
						LOG_TRACE("too many ledger pending consensus,transaction hash(%s) abandon", utils::String::Bin4ToHexString(hash_value).c_str());
						result.set_code(protocol::ERRCODE_OUT_OF_TXCACHE);
						result.set_desc("too many ledger pending consensus");
						break;
					}

					*ledger_seq = next_ledger_num_ + i;

					tx_frm->suggestion_seq_ = *ledger_seq;

					std::shared_ptr<TransactionFrmLow> low = GetTranLow(*transactions_[(size_t)(*ledger_seq - next_ledger_num_)], address);
					assert(low);
					low->Add(tx_frm);
				}
				else if (*ledger_seq >= next_ledger_num_ && *ledger_seq < next_ledger_num_ + transactions_.size()) {
					std::shared_ptr<TransactionFrmLow> low = GetTranLow(*transactions_[(size_t)(*ledger_seq - next_ledger_num_)], address);
					assert(low);
					low->Add(tx_frm);
				}
				else {
					//too late, have to put it into current ledger
					std::shared_ptr<TransactionFrmLow> low = GetTranLow(*transactions_[0], address);
					assert(low);
					low->Add(tx_frm);
				}
			}

			int64_t now = utils::Timestamp::HighResolution();
			if (next_consensus_can_trigger_ && empty_transaction_times_ > 0) {
				next_consensus_can_trigger_ = false;

				utils::Timer::Instance().DelTimer(trigger_timer_);
				trigger_timer_ = utils::Timer::Instance().AddTimer(0, tracking_consensus_->block_seq_ + 1,
					[this](int64_t data) {
					this->TriggerNextLedger(data);
				});
			}

			status = true;
		} while (false);

		if (bubi::Configure::Instance().monitor_configure_.real_time_status_ && !status) {
			//notice monitor Tx state
			std::shared_ptr<Json::Value> tx_status = std::make_shared<Json::Value>();
			(*tx_status)["type"] = 0;
			(*tx_status)["tx_hash"] = Json::Value(utils::encode_b16(tx_frm->GetContentHash()));
			(*tx_status)["error_code"] = Json::Value(result.code());
			bubi::MonitorMaster::Instance().NoticeMonitor(tx_status->toStyledString());
		}


		if (bubi::Configure::Instance().mqserver_configure_.tx_status_) {
			//notice mqserver Tx status
			protocol::ChainTxStatus cts;
			cts.set_tx_hash(utils::encode_b16(tx_frm->GetContentHash()));
			cts.set_source_address(tx_frm->GetTransactionEnv().transaction().source_address());
			cts.set_error_code((protocol::ERRORCODE)result.code());
			cts.set_error_desc(result.desc());
			cts.set_status(status ? protocol::ChainTxStatus_TxStatus_PENDING : protocol::ChainTxStatus_TxStatus_FAILURE);
			cts.set_timestamp(utils::Timestamp::Now().timestamp());
			std::string str = cts.SerializeAsString();
			bubi::MQServer::Instance().Send(ZMQ_CHAIN_TX_STATUS, str);
		}

		if (bubi::Configure::Instance().wsserver_configure_.listen_tx_status_) {
			//notice mqserver Tx status
			protocol::ChainTxStatus cts;
			cts.set_tx_hash(utils::encode_b16(tx_frm->GetContentHash()));
			cts.set_source_address(tx_frm->GetTransactionEnv().transaction().source_address());
			cts.set_error_code((protocol::ERRORCODE)result.code());
			cts.set_error_desc(result.desc());
			cts.set_status(status ? protocol::ChainTxStatus_TxStatus_PENDING : protocol::ChainTxStatus_TxStatus_FAILURE);
			cts.set_timestamp(utils::Timestamp::Now().timestamp());
			std::string str = cts.SerializeAsString();
			bubi::WebSocketServer::Instance().BroadcastMsg(protocol::CHAIN_TX_STATUS, str);
		}

		return status;
	}

	size_t GlueManager::GetCachedTxSize() {
		utils::ReadLockGuard guard(trans_mutex_);

		return transactions_by_hash_.size();
	}

	std::shared_ptr<TransactionFrmLow> GlueManager::GetTranLow(TransactionFrmMid &mid, const std::string &address) {
		TransactionFrmMid::const_iterator itemiter = mid.find(address);
		if (itemiter == mid.end()) {
			std::shared_ptr<TransactionFrmLow> lowPtr = std::make_shared<TransactionFrmLow>();
			mid.insert(std::make_pair(address, lowPtr));
			return lowPtr;
		}
		else {
			return itemiter->second;
		}
	}

	void GlueManager::peerDoesntHave(uint16_t type, std::string const& itemID, int64_t peer_id) {
		consensus_receiver_->PeerDontHave(type, itemID, peer_id);
	}

	void GlueManager::OnTimer(int64_t current_time) {
		consensus_receiver_->OnTimer(current_time);
	}

	void GlueManager::OnTxSetFetched(const ConsensusMsg consensus_msg) {
		RecvConsensus(consensus_msg);
	}

	void GlueManager::RemoveReceivedTxs(TransactionSetFrm::TxSetByAccountSeq const& dropTxs) {
		utils::WriteLockGuard guard(trans_mutex_);

		std::set<std::shared_ptr<TransactionFrmLow>> toRecalculate;

		for (auto const& tx : dropTxs) {
			for (auto& m : transactions_) {
				TransactionFrmMid::iterator i = m->find(tx->GetSourceAddress());
				if (i != m->end()) {
					if (i->second->Del(tx)) {
						if (i->second->Empty()) {
							m->erase(i);
						}
						else {
							toRecalculate.insert(i->second);
						}
						break;
					}
				}
			}
		}

		for (auto txm : toRecalculate) {
			txm->Recalculate();
		}
	}

	void GlueManager::RemoveReceivedTxs(std::vector<TransactionFrm::pointer> const& dropTxs) {
		utils::WriteLockGuard guard(trans_mutex_);
		std::set<std::shared_ptr<TransactionFrmLow>> toRecalculate;

		for (auto const& tx : dropTxs) {
			for (auto& m : transactions_) {
				LOG_ERROR("Transaction(%s) Trim In valid",
					utils::String::BinToHexString(tx->GetContentHash()).c_str());
				std::string const& acc = tx->GetSourceAddress();
				int64_t const& txID = tx->GetSequnce();
				TransactionFrmMid::iterator i = m->find(acc);
				if (i != m->end()) {
					if (i->second->Del(tx)) {
						if (i->second->Empty()) {
							m->erase(i);
						}
						else {
							toRecalculate.insert(i->second);
						}

						break;
					}
				}
			}
		}

		for (auto txm : toRecalculate) {
			txm->Recalculate();
		}
	}

	void GlueManager::RemoveTimeoutTxs() {
		utils::WriteLockGuard guard(trans_mutex_);

		int64_t expire_time = utils::Timestamp::HighResolution() - MAX_TRANSACTION_KEEPTIME_MICROSECONDS;

		std::set<std::shared_ptr<TransactionFrmLow>> toRecalculate;
		for (auto& m : transactions_) {
			for (auto t1 = m->begin(); t1 != m->end();) {
				bool account_changed = false;
				std::map<int64_t, TransactionFrm::pointer> removed = t1->second->RemoveTimeout(expire_time);
				for (std::map<int64_t, TransactionFrm::pointer>::iterator iter = removed.begin();
					iter != removed.end();
					iter++) {
					if (bubi::Configure::Instance().monitor_configure_.real_time_status_) {
						//notice monitor Tx state
						std::shared_ptr<Json::Value> tx_status = std::make_shared<Json::Value>();
						(*tx_status)["type"] = 0;
						(*tx_status)["tx_hash"] = Json::Value(utils::encode_b16(iter->second->GetContentHash()));
						(*tx_status)["error_code"] = Json::Value(iter->second->GetResult().code());
						bubi::MonitorMaster::Instance().NoticeMonitor(tx_status->toStyledString());
					}

					if (bubi::Configure::Instance().mqserver_configure_.tx_status_) {
						//notice mqserver Tx status
						protocol::ChainTxStatus cts;
						cts.set_tx_hash(utils::encode_b16(iter->second->GetContentHash()));
						cts.set_source_address(iter->second->GetTransactionEnv().transaction().source_address());
						cts.set_error_code((protocol::ERRORCODE)iter->second->GetResult().code());
						cts.set_error_desc(iter->second->GetResult().desc());
						cts.set_status(protocol::ChainTxStatus_TxStatus_FAILURE);
						cts.set_timestamp(utils::Timestamp::Now().timestamp());
						std::string str = cts.SerializeAsString();
						bubi::MQServer::Instance().Send(ZMQ_CHAIN_TX_STATUS, str);
					}

					if (bubi::Configure::Instance().wsserver_configure_.listen_tx_status_) {
						//notice mqserver Tx status
						protocol::ChainTxStatus cts;
						cts.set_tx_hash(utils::encode_b16(iter->second->GetContentHash()));
						cts.set_source_address(iter->second->GetTransactionEnv().transaction().source_address());
						cts.set_error_code((protocol::ERRORCODE)iter->second->GetResult().code());
						cts.set_error_desc(iter->second->GetResult().desc());
						cts.set_status(protocol::ChainTxStatus_TxStatus_FAILURE);
						cts.set_timestamp(utils::Timestamp::Now().timestamp());
						std::string str = cts.SerializeAsString();
						bubi::WebSocketServer::Instance().BroadcastMsg(protocol::CHAIN_TX_STATUS, str);
					}
				}

				if (t1->second->Empty()) {
					m->erase(t1++);
				}
				else {
					if (removed.size() > 0) {
						toRecalculate.insert(t1->second);
					}

					t1++;
				}
			}
		}

		for (auto txm : toRecalculate) {
			txm->Recalculate();
		}
	}

	GlueConsensusData::State GlueManager::getState() const {
		return tracking_consensus_->next_ledger_state_;
	}

	TransactionFrm::pointer GlueManager::GetTx(const std::string &hash) {
		utils::ReadLockGuard guard(trans_mutex_);
		TransactionSetFrm::TxMapByHash::iterator it = transactions_by_hash_.find(hash);
		if (it != transactions_by_hash_.end()) {
			return it->second;
		}
		return NULL;
	}

	void GlueManager::RecvConsensusTx(TransactionFrm::pointer &tx_frm) {
		consensus_receiver_->RecvTx(tx_frm);
	}


	void GlueManager::RecvConsensusTxSet(TransactionSetFrmPtr txset) {
		consensus_receiver_->RecvTxSet(txset);
	}

	TransactionSetFrmPtr GlueManager::GetTxSet(const std::string &hash) {
		return consensus_receiver_->GetTxSet(hash);
	}

	bool GlueManager::WantedTxSet(const std::string txset_hash) {
		return consensus_receiver_->WantedTxSet(txset_hash);
	}

	bool GlueManager::WantedTx(const std::string txset_hash, const std::string tx_hash) {
		return consensus_receiver_->WantedTx(txset_hash, tx_hash);
	}

	void GlueManager::RecvTxHashSet(const protocol::TxHashSet &txHashSet, std::unordered_set<std::string> &missing_txs) {
		return consensus_receiver_->RecvTxHashSet(txHashSet, missing_txs);
	}

	void GlueManager::ProcessConsensusQueue() {
		for (auto& slot : consensus_receiver_->readySlots()) {
			LOG_TRACE("process message queue (" FMT_I64 ")", slot);
			ProcessConsensusQueueAt(slot);
		}

		if (tracking_consensus_) consensus_receiver_->EraseLow(tracking_consensus_->sequence_ - g_creceiver_deloffset_);
	}

	void GlueManager::ProcessConsensusQueueAt(int64_t sequence) {
		while (true) {
			ConsensusMsg env;
			if (consensus_receiver_->PopBack(sequence, env)) {
				LOG_TRACE("Process queue at index (" FMT_U64 ")", sequence);
				consensus_->OnRecv(env);
			}
			else {
				return;
			}
		}
	}

	void GlueManager::OnViewChanged() {
		LOG_INFO("Glue manager view changed");
		//should trigger again
		tracking_consensus_->block_seq_ = LedgerManager::Instance().GetLastClosedLedger().ledger_sequence();
		next_ledger_num_ = tracking_consensus_->block_seq_ + 1;
		tracking_consensus_->next_ledger_state_ = GlueConsensusData::CLOSED;
		LOG_INFO("Set next ledger state to:%d", (int32_t)tracking_consensus_->next_ledger_state_);
		if (consensus_->RepairStatus()) ScheduleNextLedger(false);
	}

	void GlueManager::UpdateTxPool() {
		do {
			if (next_ledger_num_ >= tracking_consensus_->block_seq_ + 1) {
				return;
			}

			utils::WriteLockGuard guard(trans_mutex_);

			//always use transactions_[0] to store next ledger
			while (next_ledger_num_ < tracking_consensus_->block_seq_ + 1) {
				TransactionFrmMid* temp = transactions_[0];
				memmove(&transactions_[0], &transactions_[1], sizeof(TransactionFrmMid*) * (transactions_.size() - 1));
				transactions_[transactions_.size() - 1] = temp;

				for (auto const& map : *temp) {
					std::shared_ptr<TransactionFrmLow> low = GetTranLow(*transactions_[0], map.first);
					assert(low);

					low->MoveFrom(*map.second);
				}
				temp->clear();

				next_ledger_num_++;
			}

		} while (false);
	}

	void GlueManager::ScheduleNextLedger(bool empty_block) {
		UpdateTxPool();

		int64_t next_index = tracking_consensus_->block_seq_ + 1;

		// If we are not a validating node and just watching consensus we don't call
		// triggerNextLedger. Likewise if we are not in synced state.
		if (!consensus_->IsValidator()) {
			LOG_TRACE("Non validating node, not triggering ledger close function.");
			return;
		}


		// process any statements for this slot (this may trigger externalize)
		ProcessConsensusQueueAt(next_index);

		int64_t seconds = GetIntervalTime(empty_block);
		int64_t now = utils::Timestamp::HighResolution();
		if ((now - last_trigger_time_) < seconds) 
		{
			trigger_left_microtime_ = seconds - (now - last_trigger_time_);
		}
		else {
			trigger_left_microtime_ = 0;
		}
		tracking_consensus_->next_ledger_state_ = GlueConsensusData::State::CLOSED;
		LOG_INFO("Set next ledger state to:%d", (int32_t)tracking_consensus_->next_ledger_state_);
		trigger_timer_ = utils::Timer::Instance().AddTimer(trigger_left_microtime_, next_index,
			[this](int64_t data) {
			TriggerNextLedger(data);
		});

		//ledger must be close after the timer, otherwise call the consensus' timeout
		utils::Timer::Instance().DelTimer(ledgerclose_check_timer_);
		ledgerclose_check_timer_ = utils::Timer::Instance().AddTimer(MAX_LEDGER_TIMESPAN_SECONDS + 30 * utils::MICRO_UNITS_PER_SEC, 0,
			[this](int64_t data) {
			tracking_consensus_->next_ledger_state_ = GlueConsensusData::State::TIMER_OUT;
			LOG_INFO("Set next ledger state to:%d", (int32_t)tracking_consensus_->next_ledger_state_);
			consensus_->OnTxTimeout();
		});

		LOG_INFO("Consensus time left " FMT_I64 ", cached tx size(" FMT_SIZE")", trigger_left_microtime_, transactions_by_hash_.size());

		next_consensus_can_trigger_ = true;
	}

	int64_t GlueManager::GetIntervalTime(bool empty_block) {
		LedgerConfigure &ledger_configure = Configure::Instance().ledger_configure_;
		if (ledger_configure.test_model_) {
			return Configure::Instance().validation_configure_.close_interval_;
		}

		//there is still transaction in memory
		bool trans_empty_in_memory = true;

		do {
			utils::ReadLockGuard guard(trans_mutex_);
			if (transactions_by_hash_.size() > 0) {
				trans_empty_in_memory = false;
			}
		} while (false);

		if (trans_empty_in_memory && empty_block) {
			empty_transaction_times_++;
			return empty_transaction_times_ > 20 ? MAX_LEDGER_TIMESPAN_SECONDS :
				MIN(MAX_LEDGER_TIMESPAN_SECONDS, (int64_t)(Configure::Instance().validation_configure_.close_interval_ * pow(2, empty_transaction_times_)));
		}
		else {
			empty_transaction_times_ = 0;
			return Configure::Instance().validation_configure_.close_interval_;
		}
	}

	void GlueManager::TriggerNextLedger(int64_t ledger_seq_trigger) {
		LOG_INFO("Trigger next ledger(" FMT_I64 ") start", ledger_seq_trigger);
		if (tracking_consensus_->next_ledger_state_ != GlueConsensusData::CLOSED
			|| !LedgerManager::Instance().SyncComplete()) {
			LOG_ERROR("Trigger next ledger for some reason skip it,state:%d, sync:%s ",
				(int32_t)tracking_consensus_->next_ledger_state_, LedgerManager::Instance().SyncComplete() ? "true" : "false");
			return;
		}

		// our first choice for this round's set is all the tx we have collected
		// during last ledger close
		LedgerConfigure &ledger_config = Configure::Instance().ledger_configure_;
		auto const& lcl = LedgerManager::Instance().GetLastClosedLedger();
		TransactionSetFrmPtr proposedSet = std::make_shared<TransactionSetFrm>(lcl.hash());

		do {
			utils::ReadLockGuard guard(trans_mutex_);
			TransactionFrmMid::iterator txmap = transactions_[0]->begin();
			for (; txmap != transactions_[0]->end(); txmap++) {
				int64_t seq = 0;
				AccountFrm::pointer acc = NULL;
				if (LedgerManager::Instance().GetAccountEntry(txmap->first, acc)) {
					seq = acc->GetAccountTxSeq();
				}
				else {
					//impossible
					BUBI_EXIT("impossible");
				}

				txmap->second->GetProposeTxs(seq, proposedSet);
			}
		} while (false);

		std::vector<TransactionFrm::pointer> removed;

		proposedSet->TrimInvalid(removed);
		RemoveReceivedTxs(removed);
		if (bubi::Configure::Instance().monitor_configure_.real_time_status_) {
			//notice monitor Tx state
			for (auto tx : removed) {
				std::shared_ptr<Json::Value> tx_status = std::make_shared<Json::Value>();
				(*tx_status)["type"] = 0;
				(*tx_status)["tx_hash"] = Json::Value(utils::encode_b16(tx->GetContentHash()));
				(*tx_status)["error_code"] = Json::Value(tx->GetResult().code());
				bubi::MonitorMaster::Instance().NoticeMonitor(tx_status->toStyledString());
			}
		}

		if (bubi::Configure::Instance().mqserver_configure_.tx_status_) {
			//notice mqserver Tx status
			for (auto tx : removed) {
				protocol::ChainTxStatus cts;
				cts.set_tx_hash(utils::encode_b16(tx->GetContentHash()));
				cts.set_error_code((protocol::ERRORCODE)tx->GetResult().code());
				cts.set_error_desc(tx->GetResult().desc());
				cts.set_status(protocol::ChainTxStatus_TxStatus_FAILURE);
				cts.set_source_address(tx->GetSourceAddress());
				cts.set_timestamp(utils::Timestamp::Now().timestamp());
				std::string str = cts.SerializeAsString();
				bubi::MQServer::Instance().Send(ZMQ_CHAIN_TX_STATUS, str);
			}
		}

		if (bubi::Configure::Instance().wsserver_configure_.listen_tx_status_) {
			//notice mqserver Tx status
			for (auto tx : removed) {
				protocol::ChainTxStatus cts;
				cts.set_tx_hash(utils::encode_b16(tx->GetContentHash()));
				cts.set_error_code((protocol::ERRORCODE)tx->GetResult().code());
				cts.set_error_desc(tx->GetResult().desc());
				cts.set_status(protocol::ChainTxStatus_TxStatus_FAILURE);
				cts.set_source_address(tx->GetSourceAddress());
				cts.set_timestamp(utils::Timestamp::Now().timestamp());
				std::string str = cts.SerializeAsString();
				bubi::WebSocketServer::Instance().BroadcastMsg(ZMQ_CHAIN_TX_STATUS, str);
			}
		}

		LOG_INFO("Tx sequence, propose size " FMT_SIZE " removed " FMT_SIZE,
			proposedSet->size(), removed.size());

		if (proposedSet->CheckValid() <= 0) {
			//throw std::runtime_error("wanting to emit an invalid txSet");
			LOG_ERROR("Want to send an invalid transation set");
			return;
		}
		std::string txSetHash = proposedSet->GetContentsHash();

		consensus_receiver_->RecvTxSet(proposedSet);

		// use the slot index from ledger manager here as our vote is based off
		// the last closed ledger stored in ledger manager
		uint64_t slotIndex = (uint64_t)(lcl.ledger_sequence() + 1);

		// no point in sending out a prepare:
		// externalize was triggered on a more recent ledger
		if (ledger_seq_trigger != slotIndex) {
			LOG_ERROR("Trigger seq:" FMT_U64 " not equal with section index:" FMT_U64, ledger_seq_trigger, slotIndex);
			return;
		}

		// We store at which time we triggered consensus
		last_trigger_time_ = utils::Timestamp::HighResolution();

		// We pick as next close time the current time unless it's before the last
		// close time. We don't know how much time it will take to reach consensus
		// so this is the most appropriate value to use as close_time.
		uint64_t nextCloseTime = utils::Timestamp::Now().timestamp();
		if (nextCloseTime <= lcl.consensus_value().close_time()) {
			nextCloseTime = lcl.consensus_value().close_time() + utils::MICRO_UNITS_PER_SEC;
		}

		protocol::Value newProposedValue;
		newProposedValue.set_hash_set(txSetHash);
		newProposedValue.set_close_time(nextCloseTime);

		if (General::LEDGER_VERSION != lcl.ledger_version()) {
			newProposedValue.mutable_ledger_upgrade()->set_new_ledger_version(General::LEDGER_VERSION);
		}

		if (ledger_config.base_fee_ != lcl.base_fee()) {
			newProposedValue.mutable_ledger_upgrade()->set_new_base_fee(ledger_config.base_fee_);
		}

		if (ledger_config.base_reserve_ != lcl.base_reserve()) {
			newProposedValue.mutable_ledger_upgrade()->set_new_base_reserve(ledger_config.base_reserve_);
		}

		std::string value_hash = utils::Sha256::Crypto(newProposedValue.SerializeAsString());
		LOG_INFO("Trigger next ledger transation set size: " FMT_SIZE " | previous ledger hash: %s | propose txset hash:%s | section: %d",
			proposedSet->txset_by_acc_seq_.size(), utils::String::Bin4ToHexString(proposedSet->PreviousLedgerHash()).c_str(),
			utils::String::Bin4ToHexString(txSetHash).c_str(),
			slotIndex);

		tracking_consensus_->next_ledger_state_ = GlueConsensusData::State::TRIGGER;
		LOG_TRACE("Set next ledger state to:%d", (int32_t)tracking_consensus_->next_ledger_state_);

		protocol::Value prevValue = lcl.consensus_value();

		if (bubi::Configure::Instance().monitor_configure_.real_time_status_) {
			//notice monitor consensus start
			std::shared_ptr<Json::Value> cons_status = std::make_shared<Json::Value>();
			(*cons_status)["type"] = 3;
			(*cons_status)["seq"] = slotIndex;
			(*cons_status)["time"] = last_trigger_time_;
			(*cons_status)["ledger"] = consensus_->IsLeader();
			bubi::MonitorMaster::Instance().NoticeMonitor(cons_status->toStyledString());
		}

		consensus_->Request(slotIndex, newProposedValue);
	}

	int32_t GlueManager::CheckValueHelper(int64_t sequence, const protocol::Value &b) {
		const protocol::LedgerHeader &header = LedgerManager::Instance().GetLastClosedLedger();
		const protocol::Value &last_value = header.consensus_value();
		if (b.close_time() < last_value.close_time()) {
			LOG_ERROR("Close time(" FMT_U64 ") less than previous ledger close time(" FMT_U64 ")", b.close_time(), last_value.close_time());
			return Consensus::CHECK_VALUE_INVALID;
		}

		// Check close_time (not too far in future)
		int64_t now = utils::Timestamp::Now().timestamp();
		if (b.close_time() > now + MAX_TIME_SLIP_MICROSECONDS) {
			LOG_ERROR("Close time(" FMT_U64 ") larger than (" FMT_U64 ")", b.close_time(), now + MAX_TIME_SLIP_MICROSECONDS);
			return Consensus::CHECK_VALUE_INVALID;
		}

		if (!LedgerManager::Instance().SyncComplete())
		{
			// if we're not tracking, there is not much more we can do to
			// validate
			LOG_TRACE("Tracking is not active");
			return Consensus::CHECK_VALUE_MAYVALID;
		}

		LOG_INFO("Validate value start, sequence(" FMT_I64 ") hash(%s)", sequence, utils::String::Bin4ToHexString(b.hash_set()).c_str());
		TransactionSetFrmPtr txset = consensus_receiver_->GetTxSet(b.hash_set());
		int32_t res = Consensus::CHECK_VALUE_INVALID;
		if (!txset) {
			LOG_ERROR("Validate value i: " FMT_I64 " transaction set not found?", sequence);
			res = Consensus::CHECK_VALUE_INVALID;
		}
		else {
			int32_t ret = txset->CheckValid();
			if (ret < 0) {
				LOG_ERROR("Validate value i: " FMT_I64  " Invalid txsethash:%s",
					sequence, utils::String::Bin4ToHexString(txset->GetContentsHash()).c_str());
				res = Consensus::CHECK_VALUE_INVALID;
			}
			else {
				LOG_TRACE("Validate value  i: " FMT_I64 " ret: %d, txsethash: %s OK ", sequence, ret, utils::String::Bin4ToHexString(txset->GetContentsHash()).c_str());
				res = ret > 0 ? Consensus::CHECK_VALUE_VALID : Consensus::CHECK_VALUE_MAYVALID;
			}
		}
		LOG_INFO("Validate value end, sequence(" FMT_I64 ")", sequence);
		return res;
	}

	void GlueManager::RecvConsensus(const ConsensusMsg &msg) {
		consensus_receiver_->Recv(msg);
	}

	bool GlueManager::ConsensusHasRecv(const ConsensusMsg &msg) {
		return consensus_receiver_->ConsensusHasRecv(msg);
	}

	int32_t GlueManager::CheckValue(int64_t sequence, const protocol::Value &value) {
		//check the txset if exist
		TransactionSetFrmPtr ptr = std::make_shared<TransactionSetFrm>(utils::Sha256::Crypto("null"));
		if (value.hash_set() == ptr->GetContentsHash()) {
			return Consensus::CHECK_VALUE_VALID;
		}

		int32_t res = CheckValueHelper(sequence, value);
		do {
			if (Consensus::CHECK_VALUE_INVALID == res) {
				break;
			}

			LedgerConfigure &ledger_config = Configure::Instance().ledger_configure_;
			if (value.has_ledger_upgrade()) {
				protocol::LedgerUpgrade ledger_upgrade = value.ledger_upgrade();
				if (ledger_upgrade.has_new_ledger_version() && ledger_upgrade.new_ledger_version() != General::LEDGER_VERSION) {
					LOG_ERROR("Propose ledger version(%u) not equal local(%u)", ledger_upgrade.new_ledger_version(), General::LEDGER_VERSION);
					res = Consensus::CHECK_VALUE_INVALID;
					break;
				}

				if (ledger_upgrade.has_new_base_fee() && ledger_upgrade.new_base_fee() != ledger_config.base_fee_) {
					LOG_ERROR("Propose base fee(%u) not equal local(%u)", ledger_upgrade.new_base_fee(), ledger_config.base_fee_);
					res = Consensus::CHECK_VALUE_INVALID;
					break;
				}

				if (ledger_upgrade.has_new_base_reserve() && ledger_upgrade.new_base_reserve() != ledger_config.base_reserve_) {
					LOG_ERROR("Propose base reserve(%u) not equal local(%u)", ledger_upgrade.new_base_reserve(), ledger_config.base_reserve_);
					res = Consensus::CHECK_VALUE_INVALID;
					break;
				}
			}
		} while (false);
		return res;
	}

	void GlueManager::SendConsensusMessage(const PeerMessagePointer &message) {
		message->hash_ = utils::Sha256::Crypto(message->GetString());
		PeerManager::Instance().Broadcast(message);
		if (message->header_.type == PeerMessage::PEER_MESSAGE_PBFT) {

			ConsensusMsg msg(*(protocol::PbftEnv *)message->data_);
			if (!GlueManager::Instance().ConsensusHasRecv(msg)) {
				std::string block_seq_log = msg.GetBlockSeq() > 0 ? utils::String::Format(" block(" FMT_I64 ")", msg.GetBlockSeq()) : "";
				LOG_INFO("Receive consensus from self node address(%s) sequence(" FMT_I64 ")%s pbft type(%s)",
					msg.GetNodeAddress(), msg.GetSeq(), block_seq_log.c_str(), PbftDesc::GetMessageTypeDesc(msg.GetPbft().pbft().type()));
				GlueManager::Instance().RecvConsensus(msg);
			}
		}
	}

	std::string GlueManager::FetchNullMsg() {
		TransactionSetFrmPtr ptr = std::make_shared<TransactionSetFrm>(utils::Sha256::Crypto("null"));
		RecvConsensusTxSet(ptr);
		return ptr->GetContentsHash();
	}

	std::string GlueManager::OnValueCommited(int64_t block_seq, int64_t request_seq, protocol::Value const& value, bool calculate_total) {

		std::string const& txset_hash = value.hash_set();
		LOG_INFO("Value(hash:%s) commited ", utils::String::Bin4ToHexString(txset_hash).c_str());

		tracking_consensus_->sequence_ = request_seq;
		tracking_consensus_->block_seq_ = block_seq;
		tracking_consensus_->value_ = value;

		LOG_INFO("Receive consensus, closing (" FMT_I64 ")", request_seq);

		TransactionSetFrmPtr externalizedSet = consensus_receiver_->GetTxSet(txset_hash);

		int64_t time1 = utils::Timestamp().HighResolution();
		RemoveReceivedTxs(externalizedSet->txset_by_acc_seq_);
		int64_t time2 = utils::Timestamp().HighResolution();
		RemoveTimeoutTxs();
		int64_t time3 = utils::Timestamp().HighResolution();
		LOG_INFO("RemoveReceivedTxs: " FMT_I64 " RemoveTimeoutTxs:" FMT_I64 "", time2 - time1, time3 - time2);

		UpdateTxPool();

		// trigger will be recreated when the ledger is closed
		// we do not want it to trigger while downloading the current set
		// and there is no point in taking a position after the round is over
		utils::Timer::Instance().DelTimer(trigger_timer_);

		consensus_receiver_->Erase(tracking_consensus_->sequence_);

		LedgerManager::Instance().OnConsent(block_seq, request_seq,
			externalizedSet,
			value,
			calculate_total);

		ScheduleNextLedger(externalizedSet->size() == 0);

		auto now = utils::Timestamp::HighResolution();
		if (bubi::Configure::Instance().monitor_configure_.real_time_status_) {
			//notice monitor consensus end
			std::shared_ptr<Json::Value> cons_status = std::make_shared<Json::Value>();
			(*cons_status)["type"] = 4;
			(*cons_status)["seq"] = block_seq;
			(*cons_status)["time"] = now;
			bubi::MonitorMaster::Instance().NoticeMonitor(cons_status->toStyledString());

			//notice monitor Tx state
			for (auto tx : externalizedSet->txset_by_acc_seq_) {
				std::shared_ptr<Json::Value> tx_status = std::make_shared<Json::Value>();
				(*tx_status)["type"] = 0;
				(*tx_status)["tx_hash"] = Json::Value(utils::encode_b16(tx->GetContentHash()));
				(*tx_status)["error_code"] = Json::Value(tx->GetResult().code());
				bubi::MonitorMaster::Instance().NoticeMonitor(tx_status->toStyledString());
			}
		}

		if (bubi::Configure::Instance().mqserver_configure_.tx_status_) {
			//notice mqserver Tx status
			for (auto tx : externalizedSet->txset_by_acc_seq_) {
				protocol::ChainTxStatus ctx;
				ctx.set_tx_hash(utils::encode_b16(tx->GetContentHash()));
				ctx.set_ledger_seq(block_seq);
				ctx.set_new_account_seq(block_seq << 32);
				ctx.set_source_address(tx->GetTx().source_address());
				ctx.set_source_account_seq(tx->GetTx().sequence_number());
				ctx.set_error_code((protocol::ERRORCODE)tx->GetResult().code());
				ctx.set_error_desc(tx->GetResult().desc());
				ctx.set_status(tx->GetResult().code() == protocol::ERRCODE_SUCCESS ? protocol::ChainTxStatus_TxStatus_COMPLETE : protocol::ChainTxStatus_TxStatus_FAILURE);
				ctx.set_timestamp(utils::Timestamp::Now().timestamp());
				std::string str = ctx.SerializeAsString();
				bubi::MQServer::Instance().Send(ZMQ_CHAIN_TX_STATUS, str);
			}
		}

		if (bubi::Configure::Instance().wsserver_configure_.listen_tx_status_) {
			//notice mqserver Tx status
			for (auto tx : externalizedSet->txset_by_acc_seq_) {
				protocol::ChainTxStatus ctx;
				ctx.set_tx_hash(utils::encode_b16(tx->GetContentHash()));
				ctx.set_ledger_seq(block_seq);
				ctx.set_new_account_seq(block_seq << 32);
				ctx.set_source_address(tx->GetTx().source_address());
				ctx.set_source_account_seq(tx->GetTx().sequence_number());
				ctx.set_error_code((protocol::ERRORCODE)tx->GetResult().code());
				ctx.set_error_desc(tx->GetResult().desc());
				ctx.set_status(tx->GetResult().code() == protocol::ERRCODE_SUCCESS ? protocol::ChainTxStatus_TxStatus_COMPLETE : protocol::ChainTxStatus_TxStatus_FAILURE);
				ctx.set_timestamp(utils::Timestamp::Now().timestamp());
				std::string str = ctx.SerializeAsString();
				bubi::WebSocketServer::Instance().BroadcastMsg(protocol::CHAIN_TX_STATUS, str);
			}
		}

		auto const& lcl = LedgerManager::Instance().GetLastClosedLedger();
		return lcl.hash();
	}

	int64_t GlueManager::GetLastConsensusSeq() const {
		if (tracking_consensus_) {
			return tracking_consensus_->block_seq_;
		}
		else {
			return 0;
		}
	}

	int64_t GlueManager::GetCurrentLedgerSeq() const {
		if (tracking_consensus_) {
			return tracking_consensus_->block_seq_;
		}
		else {
			auto lcl = LedgerManager::Instance().GetLastClosedLedger();
			return lcl.ledger_sequence();
		}
	}

	void GlueManager::GetModuleStatus(Json::Value &data) {
		int64_t begin_time = utils::Timestamp::HighResolution();
		data["name"] = "glue_manager";

		Json::Value txpool = Json::Value(Json::arrayValue);
		do {
			utils::ReadLockGuard guard((utils::ReadWriteLock &)trans_mutex_);
			data["transaction_size"] = transactions_by_hash_.size();
			for (unsigned int i = 0; i < transactions_.size(); i++) {
				txpool[txpool.size()] = transactions_[i]->size();
			}
		} while (false);

		data["transactions pool"] = txpool;
		data["valid_txsetcache_Size"] = valid_txset_cache_.size();
		data["trigger_left_second"] = trigger_left_microtime_ / utils::MICRO_UNITS_PER_SEC;
		utils::Timestamp last_trigger_time(last_trigger_time_);
		data["last_trigger_time"] = last_trigger_time.ToFormatString(false);
		data["tx_hash_delete"] = del_hash_map;
		data["tx_hash_add"] = add_hash_map;

		consensus_receiver_->GetModuleStatus(data["consensus_receiver"]);

		Json::Value &system_json = data["system"];
		utils::Timestamp time_stamp(utils::GetStartupTime() * utils::MICRO_UNITS_PER_SEC);
		system_json["uptime"] = time_stamp.ToFormatString(false);
		utils::Timestamp process_time_stamp(process_uptime_ * utils::MICRO_UNITS_PER_SEC);
		system_json["process_uptime"] = process_time_stamp.ToFormatString(false);
		system_json["current_time"] = utils::Timestamp::Now().ToFormatString(false);

		Json::Value &counter_json = data["counter"];
		counter_json["tx_new"] = General::tx_new_count;
		counter_json["tx_delete"] = General::tx_delete_count;
		counter_json["txset_new"] = General::txset_new_count;
		counter_json["txset_delete"] = General::txset_delete_count;
		counter_json["peermsg_new"] = General::peermsg_new_count;
		counter_json["peermsg_delete"] = General::peermsg_delete_count;
		counter_json["account_new"] = General::account_new_count;
		counter_json["account_delete"] = General::account_delete_count;
		counter_json["trans_low_new"] = General::trans_low_new_count;
		counter_json["trans_low_delete"] = General::trans_low_delete_count;
	}
}