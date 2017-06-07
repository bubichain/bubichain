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

#ifndef GLUE_MANAGER_
#define GLUE_MANAGER_

#include <utils/singleton.h>
#include <utils/net.h>
#include <utils/lrucache.hpp>
#include <overlay/peer.h>
#include <ledger/transaction_frm.h>
#include <consensus/consensus_manager.h>
#include "consensus_receiver.h"

#define MAX_SLOTS_TO_REMEMBER 4
namespace bubi {
	class TransactionFrmLow {
	public:
		TransactionFrmLow();
		~TransactionFrmLow();
		bool Add(const TransactionFrm::pointer &tran);
		bool Del(const TransactionFrm::pointer &tran);
		bool Empty() const;
		void Recalculate();
		int64_t GetMaxSeq() { return max_seq_; };
		int64_t GetTotalFees() { return total_fees_; };
		TransactionFrm::pointer GetTransaction(int64_t sequence);
		std::map<int64_t, TransactionFrm::pointer> RemoveTimeout(int64_t expire_time);
		void MoveFrom(TransactionFrmLow &low);
		void GetProposeTxs(int64_t sequence, TransactionSetFrmPtr &proposed_set);
	private:
		int64_t max_seq_;
		int64_t total_fees_;
		std::map<int64_t, TransactionFrm::pointer> txmap_by_seq_; // transaction full hash => transaction frame
	};

	typedef std::map < std::string, std::shared_ptr<TransactionFrmLow> > TransactionFrmMid; // account => translow
	typedef std::vector<TransactionFrmMid*> TransactionFrmHigh;

	class GlueConsensusData {
	public:
		enum State {
			NONE,
			TRIGGER,
			CLOSING,
			CLOSED,
			TIMER_OUT,
			NUM_STATE
		};
		GlueConsensusData(int64_t index, const protocol::Value &value);
		virtual ~GlueConsensusData();
	public:
		int64_t block_seq_;
		int64_t sequence_;
		protocol::Value value_;
		enum State next_ledger_state_;
	};

	class GlueManager : public utils::Singleton < bubi::GlueManager>,
		public TimerNotify,
		public StatusModule,
		public ITxSetFetcherNotify,
		public IConsensusNotify {
		friend class utils::Singleton < bubi::GlueManager>;
		friend class TransactionFrmLow;
		static int64_t const MAX_LEDGER_TIMESPAN_SECONDS;
		static uint64_t const MAX_TIME_SLIP_MICROSECONDS;
		static uint64_t const MAX_TRANSACTION_KEEPTIME_MICROSECONDS;
	public:
		//for consensus received message
		bool Initialize();
		bool Exit();
		bool StartConsensus(); // not used
		virtual void OnTimer(int64_t current_time) override;
		virtual void OnSlowTimer(int64_t current_time) override {};
		virtual void OnTxSetFetched(const ConsensusMsg consensus_msg);
		virtual void GetModuleStatus(Json::Value &data);
		//for consensus implement
		virtual std::string OnValueCommited(int64_t block_seq, int64_t request_seq, const protocol::Value &value, bool calculate_total);
		virtual void OnViewChanged();
		virtual int32_t CheckValue(int64_t block_seq, const protocol::Value &value);
		virtual void SendConsensusMessage(const PeerMessagePointer &message);
		virtual std::string FetchNullMsg();
		//receive transaction
		bool ReceiveTransaction(TransactionFrm::pointer &tx_data, int64_t* ledger_seq);
		Result GetResult() const;
		std::shared_ptr<TransactionFrmLow> GetTranLow(TransactionFrmMid &mid, const std::string &address);
		//receive transaction about consensus
		void RecvConsensusTx(TransactionFrm::pointer &tx_frm);
		void RecvConsensusTxSet(TransactionSetFrmPtr txset);
		void RecvConsensus(const ConsensusMsg &msg);
		bool ConsensusHasRecv(const ConsensusMsg &msg);
		void RecvTxHashSet(const protocol::TxHashSet &txHashSet, std::unordered_set<std::string> &missing_txs);
		TransactionSetFrmPtr GetTxSet(const std::string &hash);
		TransactionFrm::pointer GetTx(const std::string &hash);
		void peerDoesntHave(uint16_t type, std::string const& itemID, int64_t peer_id);
		size_t GetCachedTxSize();
		int64_t GetLastConsensusSeq() const;
		int64_t GetCurrentLedgerSeq() const;
		GlueConsensusData::State getState() const;
		void RemoveReceivedTxs(std::vector<TransactionFrm::pointer> const& dropTxs);
		void RemoveReceivedTxs(TransactionSetFrm::TxSetByAccountSeq const& dropTxs);
		void RemoveTimeoutTxs();
		void ProcessConsensusQueueAt(int64_t ledger_seq);
		void ProcessConsensusQueue();
		void ScheduleNextLedger(bool empty_block);
		//trigger next ledger
		void TriggerNextLedger(int64_t sequence);
		//consensus have been committed, then executed the value
		void ProposeValue(int64_t slotIndex, protocol::Value const& value);
		bool WantedTxSet(const std::string txset_hash);
		bool WantedTx(const std::string txset_hash, const std::string tx_hash);
	private:
		GlueManager();
		~GlueManager();
		void UpdateTxPool();
		int64_t GetIntervalTime(bool empty_block);
		int32_t CheckValueHelper(int64_t sequence, const protocol::Value &sv);
	private:
		TransactionFrmHigh transactions_;
		TransactionSetFrm::TxMapByHash transactions_by_hash_;
		utils::ReadWriteLock trans_mutex_;
		int64_t next_ledger_num_;
		uint32_t delay_consensus_;
		cache::lru_cache<std::string, bool> valid_txset_cache_;
		//for get module status
		time_t process_uptime_;
		//consensus message service
		std::shared_ptr<ConsensusReceiver> consensus_receiver_;
		//consensus
		std::shared_ptr<Consensus> consensus_;
		//current consensus data
		std::unique_ptr<GlueConsensusData> tracking_consensus_;
		//for get module status
		int64_t last_trigger_time_;
		//timer close ledger
		int64_t trigger_timer_;
		int64_t ledgerclose_check_timer_;
		//for delay trigger
		bool next_consensus_can_trigger_;
		int32_t empty_transaction_times_;
		int64_t trigger_left_microtime_;
	};
};

#endif