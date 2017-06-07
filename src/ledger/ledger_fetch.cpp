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

#include <random>
#include <utils/headers.h>
#include <glue/glue_manager.h>
#include <overlay/peer_manager.h>
#include "ledger_manager.h"
#include "ledger_fetch.h"

namespace bubi {
	LedgerFetch::LedgerFetch() {
		workers_.clear();
		manual_enabled_ = false;
		last_ontimer_ = 0;
		max_consensus_ledger_ = nullptr;
	}

	LedgerFetch::~LedgerFetch() {}

	Json::Value LedgerFetch::FetchWorker::ToJson()const {
		Json::Value js;
		js["last_send_time"] = last_send_time_;
		js["delay"] = delay_;
		js["score"] = score_;
		js["max_ledger_size"] = max_ledger_size_;
		js["peer_id"] = pid_;
		js["disable_time"] = disable_time_;
		return js;
	}

	void LedgerFetch::FetchWorker::Disable() {
		score_ = -1;
		disable_time_ = utils::Timestamp::HighResolution();
	}
	std::string LedgerFetch::BlockStateName(LedgerFetch::BlockState s) {
		switch (s) {
		case bubi::LedgerFetch::MISSING:
			return "MISSING";
			break;
		case bubi::LedgerFetch::REQUESTING:
			return "REQUESTING";
			break;
		case bubi::LedgerFetch::RECEIVED:
			return "RECEIVED";
			break;
		case bubi::LedgerFetch::CONFIRMED:
			return "CONFIRMED";
			break;
		case bubi::LedgerFetch::CLOSED:
			return "CLOSED";
			break;
		default:
			return "UNKNOWN";
			break;
		}
	}

	bool LedgerFetch::Initialize() {
		RationalDb *db = bubi::Storage::Instance().rational_db();
		if (db == nullptr) {
			LOG_INFO("rational_db is null!");
			return false;
		}
		auto &manager = LedgerManager::Instance();
		int64_t seq_max_closed = manager.GetLastClosedLedger().ledger_sequence();
		block_seg_.push_back(Segment(1, seq_max_closed, CLOSED));

		//get the max consensus ledger seq
		int64_t seq_max_recv = seq_max_closed;
		do {
			std::string sql_ledger_recv = utils::String::Format("SELECT seq FROM %s WHERE seq>" FMT_I64 " order by seq asc", General::TABLE_LEDGER_BUFFER, seq_max_closed);
			Json::Value record_ledgers = Json::Value(Json::arrayValue);
			int32_t ret = db->Query(sql_ledger_recv, record_ledgers);
			if (ret < 0) {
				LOG_ERROR_ERRNO("Execute <%s> failed", sql_ledger_recv.c_str(), db->error_code(), db->error_desc());
				return false;
			}
			else if (ret == 0) {
				LOG_INFO("no ledgers in sync_buffer!");
				break;
			}

			auto b = seq_max_closed;
			auto e = b;

			for (Json::UInt i = 0; i < record_ledgers.size(); i++) {
				Json::Value &ledger = record_ledgers[i];
				int64_t seq = ledger["seq"].asInt64();
				if (i == (Json::UInt) 0) {
					if (seq > seq_max_closed + 1)
						block_seg_.push_back(Segment(seq_max_closed + 1, seq - 1, MISSING));
					b = seq;
					e = seq;
					continue;
				}

				if (seq == e + 1) {
					e = seq;
				}
				else if (seq < e + 1) {
					BUBI_EXIT("seq<e+1");
				}
				else {

					block_seg_.push_back(Segment(b, e, CONFIRMED));
					LedgerFrm::pointer tmp = FromDbBuffer(b);
					confirm_hash_[b - 1] = tmp->GetProtoHeader().parent_hash();
					confirm_hash_[b] = tmp->GetProtoHeader().hash();
					LOG_INFO(FMT_I64 "~" FMT_I64 "CONFIRMED", b, e);
					LOG_INFO(FMT_I64 "~" FMT_I64, e + 1, seq - 1);
					block_seg_.push_back(Segment(e + 1, seq - 1, MISSING));
					b = seq;
					e = b;
				}
				if (seq > seq_max_recv) {
					seq_max_recv = seq;
				}
			}

			if (e >= b && b > seq_max_closed) {
				block_seg_.push_back(Segment(b, e, CONFIRMED));
				LedgerFrm::pointer tmp = FromDbBuffer(b);
				confirm_hash_[b - 1] = tmp->GetProtoHeader().parent_hash();
				confirm_hash_[b] = tmp->GetProtoHeader().hash();
			}

			if (seq_max_recv > seq_max_closed) {
				max_consensus_ledger_ = FromDbBuffer(seq_max_recv);
				LOG_INFO(FMT_I64, seq_max_recv);
				assert(max_consensus_ledger_ != nullptr);
			}

		} while (false);
		return true;
	}

	bool LedgerFetch::ToDbBuffer(const protocol::Ledger &ledger) {
		utils::StringMap updatevalues;
		updatevalues["seq"] = utils::String::Format(FMT_I64, ledger.ledger_header().ledger_sequence());
		updatevalues["value"] = utils::String::BinToHexString(ledger.SerializeAsString());
		auto db = Storage::Instance().rational_db();
		if (!db->Insert(General::TABLE_LEDGER_BUFFER, updatevalues)) {
			LOG_ERROR("%s:%d", db->error_desc(), db->error_code());
			return false;
		}
		else {
			return true;
		}
	}

	LedgerFrm::pointer LedgerFetch::FromDbBuffer(int64_t seq) {
		Json::Value record;
		std::string sql = utils::String::Format("SELECT * FROM %s WHERE seq=" FMT_I64 "", General::TABLE_LEDGER_BUFFER, seq);

		int32_t n = Storage::Instance().rational_db()->QueryRecord(sql, record);
		if (n <= 0) {
			return nullptr;
		}
		protocol::Ledger protoledger;
		if (!protoledger.ParseFromString(utils::String::HexStringToBin(record["value"].asString()))) {
			return nullptr;
		}
		LedgerFrm::pointer frm_pt = std::make_shared<LedgerFrm>();
		frm_pt->FromProtoLedger(protoledger);
		return frm_pt;
	}

	void LedgerFetch::GetModuleStatus(Json::Value& data) const {
		Json::Value& s = data["ledger_fetch.block_seg"];
		for (auto x = block_seg_.rbegin(); x != block_seg_.rend(); x++) {
			Json::Value item;
			item["state"] = BlockStateName(x->state_);
			item["range"] = utils::String::Format("[" FMT_I64 "->" FMT_I64 "]", x->begin_, x->end_);
			item["peer_id"] = x->pid_;
			s[s.size()] = item;
		}
		data["ledger_fetch.confirm_hash_size"] = confirm_hash_.size();
		Json::Value workers;
		for (auto it = workers_.begin(); it != workers_.end(); it++) {
			Json::Value item = it->second->ToJson();
			item["pid"] = it->second->pid_;
			workers[workers.size()] = item;

		}
		data["ledger_fetch.workers"] = workers;
	}

	bool LedgerFetch::ReceiveGetLedgers(const PeerMessagePointer &message, int64_t peer_id) {
		SlowTimer::Instance().io_service_.post([message, peer_id]() {

			protocol::GetLedgers *getledgers = (protocol::GetLedgers *)message->data_;
			int64_t ledgerseq = getledgers->ledger_seq();
			int64_t ledgernum = getledgers->ledger_num();

			auto msg = PeerMessage::NewLedgers();
			protocol::Ledgers *ledgers = (protocol::Ledgers *)msg->data_;
			ledgers->set_max_ledgers(Configure::Instance().ledger_configure_.max_ledger_per_message_);
			ledgers->set_sync_code(protocol::Ledgers_SyncCode_OK);

			int64_t total_time = 0;
			int64_t counts = 0;
			do {
				if (ledgernum > Configure::Instance().ledger_configure_.max_ledger_per_message_ || ledgerseq <= 0) {
					LOG_ERROR("Illegal getLedger message! parameter error");
					ledgers->set_sync_code(protocol::Ledgers_SyncCode_OUT_OF_LEDGERS);
					break;
				}

				if (ledgerseq + ledgernum - 1 > LedgerManager::Instance().GetLastClosedLedger().ledger_sequence()) {
					LOG_ERROR("Illegal getLedger message, seq bigger than last closed ledger seq!");
					ledgers->set_sync_code(protocol::Ledgers_SyncCode_OUT_OF_SYNC);
					break;
				}

				for (int64_t i = ledgerseq; i < ledgerseq + ledgernum; i++) {
					int64_t begin_time = utils::Timestamp::HighResolution();
					protocol::Ledger *ledger = ledgers->add_ledger();
					if (!LedgerFrm::LoadFromDb(i, *ledger)) {
						LOG_ERROR("Load ledger(" FMT_I64 ") from db failed", i);
						ledgers->set_sync_code(protocol::Ledgers_SyncCode_INTERNAL);
						ledgers->clear_ledger();
						break;
					}

					int64_t use_time = utils::Timestamp::HighResolution() - begin_time;
					total_time += use_time;
					counts++;
				}
			} while (false);

			LOG_INFO("SEND LEDGERS [" FMT_I64 "," FMT_I64 "] to (" FMT_I64 ") use avg time " FMT_I64 " count( " FMT_I64 ")",
				ledgerseq, ledgerseq + ledgernum - 1, peer_id, counts > 0 ? total_time / counts : 0, counts);
			PeerManager::Instance().SendMessage(peer_id, msg);
			return true;
		});
		return true;
	}

	//Called when receive consensus ledger
	void LedgerFetch::OnConsent(LedgerFrm::pointer ledger) {

		int64_t	max_recv_seq = max_consensus_ledger_->GetProtoHeader().ledger_sequence();
		int64_t seq = ledger->GetProtoHeader().ledger_sequence();
		int64_t max_closed_seq = LedgerManager::Instance().GetLastClosedLedger().ledger_sequence();
		auto it = block_seg_.rbegin();
		assert(max_closed_seq <= it->end_);
		assert(max_closed_seq <= max_recv_seq);

		if (max_closed_seq + 1 == seq) {
			LedgerManager::Instance().CloseLedger(ledger, true);
			block_seg_.push_back(Segment(seq, seq, CLOSED));
			max_consensus_ledger_ = ledger;
		}
		else {
			if (max_recv_seq + 1 <= seq) {
				block_seg_.push_back(Segment(it->end_ + 1, seq - 1, MISSING));
				max_consensus_ledger_ = ledger;
				confirm_hash_[seq - 1] = ledger->GetProtoHeader().parent_hash();
			}
			else {
				LOG_ERROR("Max consensus seq is (" FMT_I64 ") but receive consensus seq [" FMT_I64 "] ", max_recv_seq, seq);
			}
		}
		MergeSegment();
	}

	bool LedgerFetch::CheckLedgerValidation(const protocol::Ledgers *ledgers, std::vector<protocol::Ledger> &frms) {
		if (ledgers->ledger_size() == 0) {
			return false;
		}
		for (int i = 0; i < ledgers->ledger_size(); i++) {
			protocol::Ledger ledger = ledgers->ledger(i);

			LedgerFrm ledgerfrm;
			ledgerfrm.FromProtoLedger(ledger);
			if (!ledgerfrm.CheckValidation())
				return false;

			if (i != 0) {
				protocol::Ledger preledger = ledgers->ledger(i - 1);
				//seq in ledgers is not sequential
				if (preledger.ledger_header().ledger_sequence() + 1 != ledger.ledger_header().ledger_sequence()) {
					LOG_ERROR("Receive ledgers seq error!");
					return false;
				}
				//a ledger's parent_hash does not equal last ledger's hash
				if (preledger.ledger_header().hash() != ledger.ledger_header().parent_hash()) {
					LOG_ERROR("Receive ledgers hash error! ledger(" FMT_I64 ") hash=%s; ledger(" FMT_I64 ") parent_hash=%s",
						preledger.ledger_header().ledger_sequence(),
						utils::String::BinToHexString(preledger.ledger_header().hash()).c_str(),
						ledger.ledger_header().ledger_sequence(),
						utils::String::BinToHexString(ledger.ledger_header().parent_hash()).c_str()
						);
					return false;
				}
			}

			if (ledger.ledger_header().ledger_version() > General::LEDGER_VERSION) {
				BUBI_EXIT("Ledger(" FMT_I64 "),version (%u) bigger than local version (%u)",
					ledger.ledger_header().ledger_sequence(),
					ledger.ledger_header().ledger_version(),
					General::LEDGER_VERSION);
			}

			frms.push_back(ledger);
		}
		return true;
	}

	void LedgerFetch::OnReceiveLedgers(const PeerMessagePointer &message, int64_t peer_id) {
		int64_t now = utils::Timestamp::HighResolution();
		const protocol::Ledgers *ledgers = (const protocol::Ledgers *)message->data_;
		auto s = workers_.find(peer_id);
		if (s == workers_.end()) {
			LOG_ERROR("Receive message from unknown peer, peerid=" FMT_I64, peer_id);
			return;
		}
		std::shared_ptr<FetchWorker> worker = s->second;

		worker->delay_ = now - worker->last_send_time_;
		worker->last_send_time_ = 0;
		
		auto seg = block_seg_.rend();
		for (auto it = block_seg_.rbegin(); it != block_seg_.rend(); it++) {
			if (it->pid_ == peer_id &&  REQUESTING == it->state_) {
				seg = it;
				seg->pid_ = 0;
				break;
			}
		}

		if (seg == block_seg_.rend()) {
			LOG_ERROR("Receive unknown message from peer, peerid=" FMT_I64, peer_id);
			return;
		}

		if (!ledgers->IsInitialized()) {
			worker->Disable();
			seg->SetMissing();
			LOG_ERROR("Uninitialized receiveledgers message from peer(" FMT_I64 ")", peer_id);
			return;
		}

		if (ledgers->has_sync_code()) {
			switch (ledgers->sync_code()) {
			case protocol::Ledgers::OUT_OF_SYNC:{
				worker->Disable();
				worker->disable_time_ = now;
				seg->SetMissing();
				LOG_ERROR("OUT_OF_SYNC,peer(" FMT_I64 ")", peer_id);
				return;
				break;
			}
			case protocol::Ledgers::OUT_OF_LEDGERS:{
				worker->Disable();
				worker->disable_time_ = now;
				worker->max_ledger_size_ = ledgers->max_ledgers();
				seg->SetMissing();
				LOG_ERROR("OUT_OF_LEDGERS,peer[" FMT_I64 "]", peer_id);
				return;
				break;
			}
			case protocol::Ledgers::BUSY:{
				seg->SetMissing();
				LOG_ERROR("BUSY,peer(" FMT_I64 ")", peer_id);
				return;
				break;
			}
			case protocol::Ledgers::REFUSE:{
				worker->Disable();
				seg->SetMissing();

				LOG_ERROR("REFUSE,peer(" FMT_I64 ")", peer_id);
				return;
				break;
			}
			case protocol::Ledgers::OK:{
				worker->max_ledger_size_ = ledgers->max_ledgers();
				seg->state_ = RECEIVED;
				break;
			}
			case protocol::Ledgers_SyncCode_INTERNAL:{
				seg->SetMissing();
				LOG_ERROR("INTERNAL,set score=-1,peerid=" FMT_I64, peer_id);
				return;
				break;
			}
			default:
			{
				LOG_ERROR("Unknown sync_code(%d),peerid=" FMT_I64, ledgers->sync_code(), peer_id);
				return;
			}
			}
		}

		if (ledgers->ledger_size() == 0) {
			worker->Disable();
			seg->SetMissing();
			LOG_ERROR("Receieve empty ledgers from peer(" FMT_I64 ")", peer_id);
			return;
		}

		std::vector<protocol::Ledger> ledger_vector;
		int64_t seq_min = ledgers->ledger(0).ledger_header().ledger_sequence();
		int64_t seq_max = ledgers->ledger(ledgers->ledger_size() - 1).ledger_header().ledger_sequence();

		if (seq_min != seg->begin_ || seq_max != seg->end_) {
			worker->Disable();
			worker->max_ledger_size_ = ledgers->ledger_size();
			seg->SetMissing();
			LOG_ERROR("receieve invalid ledger[" FMT_I64 "," FMT_I64 "][" FMT_I64 "," FMT_I64 "] from %d",
				seq_min, seg->begin_, seq_max, seg->end_, peer_id);
			return;
		}

		if (!CheckLedgerValidation(ledgers, ledger_vector)) {
			worker->Disable();
			seg->SetMissing();
			LOG_ERROR("Receieve invalid ledger from peer(" FMT_I64 ")", peer_id);
			return;
		}

		seg->state_ = RECEIVED;
		seg->rec_buf_.CopyFrom(*ledgers);
		seg->pid_ = 0;
		for (auto it = block_seg_.rbegin(); it != block_seg_.rend(); ++it) {
			if (it->state_ != RECEIVED)
				continue;

			if (confirm_hash_.find(it->end_) == confirm_hash_.end())
				continue;

			protocol::Ledgers& ledgers = it->rec_buf_;
			protocol::Ledger end_ledger = ledgers.ledger(ledgers.ledger_size() - 1);

			if (end_ledger.ledger_header().hash() == confirm_hash_[it->end_]) {
				it->state_ = CONFIRMED;
				for (int i = 0; i < ledgers.ledger_size(); i++) {
					confirm_hash_[ledgers.ledger(i).ledger_header().ledger_sequence() - 1] =
						ledgers.ledger(i).ledger_header().parent_hash();
					if (!ToDbBuffer(ledgers.ledger(i))) {
						BUBI_EXIT("ToDbBuffer failed, ledger(%d)", i);
					}
				}
			}
			else {
				worker->Disable();
				it->SetMissing();
				LOG_ERROR("Receieve invalid ledger peerid=" FMT_I64 " ledger_seq=" FMT_I64 "/" FMT_I64 " %s!=%s",
					it->pid_,
					end_ledger.ledger_header().ledger_sequence(),
					it->end_,
					utils::String::BinToHexString(end_ledger.ledger_header().hash()).c_str(),
					utils::String::BinToHexString(confirm_hash_[it->end_]).c_str()
					);
				return;
			}
		}

		MergeSegment();
		RequestMissingLedger(worker);
	}

	void LedgerFetch::OnTimer() {
		ApplyValidLedger();
		int64_t now = utils::Timestamp::HighResolution();
		DeleteTimeOutMsg();
		if (!manual_enabled_)
			GetActivePeer();
	}

	void LedgerFetch::RequestMissingLedger(std::shared_ptr<FetchWorker> worker) {
		auto next_job = block_seg_.rend();
		int64_t now = utils::Timestamp::HighResolution();

		uint32_t num_unknown = 0;

		for (auto it = block_seg_.rbegin(); it != block_seg_.rend(); it++) {
			if (it == block_seg_.rend()) {
				break;
			}

			if (it->state_ == RECEIVED || it->state_ == REQUESTING) {
				num_unknown++;
				continue;
			}

			if (num_unknown > workers_.size() * 5) {
				break;
			}

			if (it->state_ == MISSING) {
				next_job = it;
				break;
			}
		}

		if (next_job == block_seg_.rend()) {
			LOG_DEBUG("Peer " FMT_I64 " next_job = block_seg_.end() ", worker->pid_);
			return;
		}


		int64_t	seq_end = next_job->end_;
		int64_t num = next_job->end_ - next_job->begin_ + 1;

		if (num > worker->max_ledger_size_ && worker->max_ledger_size_ != 0) {
			num = worker->max_ledger_size_;
		}

		auto msg = PeerMessage::NewGetLedgers();
		protocol::GetLedgers proto_msg;
		int64_t request_begin = seq_end - num + 1;
		proto_msg.set_ledger_seq(request_begin);
		proto_msg.set_ledger_num(num);
		msg->data_->CopyFrom(proto_msg);
		worker->last_send_time_ = utils::Timestamp::HighResolution();


		if (!PeerManager::Instance().ConsensusNetwork().SendMessage(worker->pid_, msg)) {
			LOG_ERROR("peer %d SendMessage fail", worker->pid_);
			return;
		}


		next_job->state_ = REQUESTING;
		next_job->pid_ = worker->pid_;

		if (next_job->begin_ != request_begin) {
			auto nx = next_job;
			nx++;
			auto it = block_seg_.insert(nx.base(), Segment(next_job->begin_, request_begin - 1, MISSING));
			it->pid_ = 0;
			next_job->begin_ = request_begin;
		}
		LOG_INFO("request ledgers:[" FMT_I64 "-" FMT_I64 "] from peer(" FMT_I64 ")", request_begin, seq_end, worker->pid_);
	}

	bool LedgerFetch::IsReadyApply(int64_t seq) {
		if (seq == manual_seq_.second + 1 && manual_enabled_) {
			return false;
		}
		for (auto mp : block_seg_) {
			if (mp.begin_ <= seq && seq <= mp.end_ && mp.state_ == CONFIRMED)
				return true;
		}
		return false;
	}

	void LedgerFetch::ApplyValidLedger() {
		LedgerManager &manager = LedgerManager::Instance();

		int num = Configure::Instance().ledger_configure_.max_apply_ledger_per_round_;
		if (manager.GetLastClosedLedger().ledger_sequence() == manager.GetMaxConsentHeader().ledger_sequence()
			&& !manual_enabled_) {
			return;
		}

		if (max_consensus_ledger_ != nullptr &&
			max_consensus_ledger_->GetProtoHeader().ledger_sequence() == manager.GetLastClosedLedger().ledger_sequence() + 1) {
			manager.CloseLedger(max_consensus_ledger_, false);
			auto n = manager.GetLastClosedLedger().ledger_sequence();
			block_seg_.push_back(Segment(n, n, CLOSED));
			MergeSegment();
		}

		auto it = block_seg_.begin();
		it++;
		if (it == block_seg_.end()) {
			return;
		}

		if (it->state_ == CONFIRMED && it->begin_ == manager.GetLastClosedLedger().ledger_sequence() + 1) {

			if (it->end_ - it->begin_ > num) {
				auto b = it->begin_;
				auto e = it->end_;
				auto m = it->begin_ + num;
				it->begin_ = m + 1;
				it = block_seg_.insert(it, Segment(b, m, CLOSED));
			}

			for (auto seq = it->begin_; seq <= it->end_; seq++) {
				LedgerFrm::pointer ledger_frm = FromDbBuffer(seq);
				if (ledger_frm == nullptr) {
					BUBI_EXIT("ledger_frm is null");
					break;
				}
				manager.CloseLedger(ledger_frm, false);
			}
			it->state_ = CLOSED;
		}

		MergeSegment();
	}

	void LedgerFetch::GetActivePeer() {
		Json::Value peers;
		PeerManager::Instance().ConsensusNetwork().GetPeers(peers);

		LOG_DEBUG("GetPeers.size=%u", peers.size());
		std::vector<std::shared_ptr<FetchWorker>> random_workers;
		for (Json::UInt i = 0; i < peers.size(); i++) {
			Json::Value p = peers[i];
			int64_t pid = p["id"].asInt64();

			if (p["is_active"].asBool()) {
				if (workers_.find(pid) == workers_.end()) {
					workers_.insert({ pid, std::make_shared<FetchWorker>(pid) });
				}

				auto worker = workers_[pid];
				if (worker->last_send_time_ == 0 && worker->score_ >= 0) {

					random_workers.push_back(worker);
				}
				else {
					LOG_DEBUG("peer(" FMT_I64 "), last_send_time=" FMT_I64 ", score=%d", pid, worker->last_send_time_, worker->score_);
				}
			}
			else {
				LOG_DEBUG("peer(" FMT_I64 ") not active", pid);
			}
		}

		unsigned seed = std::rand();
		std::shuffle(random_workers.begin(), random_workers.end(), std::default_random_engine(seed));
		for (auto worker = random_workers.begin(); worker != random_workers.end(); worker++) {
			RequestMissingLedger(*worker);
		}
	}

	void LedgerFetch::MergeSegment() {
		auto it = block_seg_.begin();
		auto next = it;

		while (it != block_seg_.end()) {
			next = it;
			next++;
			if (next == block_seg_.end())
				break;

			if (it->state_ == next->state_ && it->state_ != REQUESTING && it->state_ != RECEIVED) {
				it->end_ = next->end_;
				block_seg_.erase(next);
			}
			else {
				it++;
			}
		}
	}

	void LedgerFetch::DeleteTimeOutMsg() {
		std::set<int64_t> live_pid;
		auto lists = PeerManager::Instance().ConsensusNetwork().GetPeers();
		for (const auto item : lists) {
			if (item.second->IsActive()) {
				live_pid.insert(item.second->peer_id());
			}
		}

		int64_t now = utils::Timestamp::HighResolution();
		for (auto it = workers_.begin(); it != workers_.end();) {
			auto &worker = it->second;

			//undisable a worker after 30 senconds,give it another chance
			if (worker->score_ < 0 && now - worker->disable_time_>30000000) {
				it = workers_.erase(it);
				continue;
			}

			//for 20 seconds, a worker is not alive, delete it from workers
			if (live_pid.find(it->first) == live_pid.end() && now - worker->last_send_time_ > 20000000) {
				it = workers_.erase(it);
			}
			else it++;
		}
	}

	void LedgerFetch::ManualFetch(int64_t begin, int64_t end, std::string source) {

	}

	void LedgerFetch::CreateSnapShot(int64_t ledger_seq) {
	}

	bool LedgerFetch::FromSnapShot(const std::string &filename) {
		bool bok = true;
		return bok;
	}
}
