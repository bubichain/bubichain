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
#include "pbft_instance.h"
#include "pbft.h"

namespace bubi {
	PbftInstanceIndex::PbftInstanceIndex(int64_t view_number, int64_t sequence) :
		view_number_(view_number), sequence_(sequence) {}

	PbftInstanceIndex::~PbftInstanceIndex() {}

	bool PbftInstanceIndex::operator <(const PbftInstanceIndex &index) const {
		if (view_number_ < index.view_number_) {
			return true;
		}
		else if (view_number_ == index.view_number_ && sequence_ < index.sequence_) {
			return true;
		}

		return false;
	}

	PbftCkpInstanceIndex::PbftCkpInstanceIndex() :sequence_(0) {}
	PbftCkpInstanceIndex::PbftCkpInstanceIndex(int64_t seq, const std::string &state_digest) :
		sequence_(seq), state_digest_(state_digest) {}

	PbftCkpInstanceIndex::~PbftCkpInstanceIndex() {}

	bool PbftCkpInstanceIndex::operator <(const PbftCkpInstanceIndex &index) const {
		if (sequence_ < index.sequence_) {
			return true;
		}
		else if (sequence_ == index.sequence_ && state_digest_ < index.state_digest_) {
			return true;
		}

		return false;
	}

	PbftCkpInstance::PbftCkpInstance() :stable_(false) {}
	PbftCkpInstance::~PbftCkpInstance() {}

	PbftInstance::PbftInstance() {
		phase_ = PBFT_PHASE_NONE;
		phase_item_ = 0;
		msg_buf_.resize(PBFT_PHASE_MAX);
		last_propose_time_ = start_time_ = utils::Timestamp::HighResolution();
		last_commit_send_time_ = 0;
		have_send_viewchange_ = false;
		pre_prepare_round_ = 1;
		commit_rount_ = 1;
		end_time_ = 0;
		check_value_result_ = Consensus::CHECK_VALUE_VALID;
	}

	PbftInstance::~PbftInstance() {

	}

	bool PbftInstance::Go(const protocol::PbftEnv &env, Pbft *pbft_object) {
		if (env.pbft().type() < (int32_t)phase_) {
			//it is received again
			if (env.pbft().type() == protocol::PBFT_TYPE_PREPREPARE)
				pbft_object->OnPrePrepare(env.pbft(), *this);
			else if (env.pbft().type() == protocol::PBFT_TYPE_PREPARE)
				pbft_object->OnPrepare(env.pbft(), *this);
			else if (env.pbft().type() == protocol::PBFT_TYPE_COMMIT)
				pbft_object->OnCommit(env.pbft(), *this);
			return true;
		}

		bool doret = false;
		while (msg_buf_[phase_].size() > phase_item_) {
			const protocol::PbftEnv &env = msg_buf_[phase_][phase_item_++];
			const protocol::Pbft &pbft = env.pbft();

			switch (pbft.type()) {
			case protocol::PBFT_TYPE_PREPREPARE:{
				doret = pbft_object->OnPrePrepare(pbft, *this);
				break;
			}
			case protocol::PBFT_TYPE_PREPARE:{
				doret = pbft_object->OnPrepare(pbft, *this);
				break;
			}
			case protocol::PBFT_TYPE_COMMIT:{
				doret = pbft_object->OnCommit(pbft, *this);
				break;
			}
			default: break;
			}
		}

		return doret;
	}

	bool PbftInstance::IsExpire(int64_t current_time) {
		return current_time - start_time_ >= g_pbft_instance_timeout_ && phase_ < PBFT_PHASE_COMMITED;
	}

	bool PbftInstance::NeedSendAgain(int64_t current_time) {
		return (current_time - last_propose_time_ >= g_pbft_instance_timeout_ / 4) && (phase_ < PBFT_PHASE_COMMITED);
	}

	bool PbftInstance::SendPrepareAgain(Pbft *pbft, int64_t current_time) {
		PeerMessagePointer new_ptr = pbft->IncPeerMessageRound(pre_prepare_msg_, ++pre_prepare_round_);
		pbft->SendMessage(new_ptr);
		SetLastProposeTime(current_time);

		return true;
	}

	bool PbftInstance::NeedSendCommitAgain(int64_t current_time) {
		return last_commit_send_time_ != 0 &&  //should be sent normal firstly, then we can repeart it
			(current_time - last_commit_send_time_ >= g_pbft_commit_send_interval) && (phase_ >= PBFT_PHASE_PREPARED);
	}

	void PbftInstance::SetLastProposeTime(int64_t current_time) {
		last_propose_time_ = current_time;
	}

	void PbftInstance::SetLastCommitSendTime(int64_t current_time) {
		last_commit_send_time_ = current_time;
	}

	PbftVcInstance::PbftVcInstance() :view_number_(0), end_time_(0), view_change_round_(0) {
		start_time_ = last_propose_time_ = utils::Timestamp::HighResolution();
		last_newview_time_ = 0;
		new_view_round_ = 1;
	}
	PbftVcInstance::PbftVcInstance(int64_t view_number) : view_number_(view_number), end_time_(0), view_change_round_(0) {
		start_time_ = last_propose_time_ = utils::Timestamp::HighResolution();
		last_newview_time_ = 0;
		new_view_round_ = 1;
	}
	PbftVcInstance::~PbftVcInstance() {}

	bool PbftVcInstance::NeedSendAgain(int64_t current_time) {
		return current_time - last_propose_time_ > g_pbft_vcinstance_timeout_ && end_time_ == 0;
	}

	bool PbftVcInstance::NeedSendNewViewAgain(int64_t current_time) {
		return current_time - last_newview_time_ > g_pbft_newview_send_interval
			&& newview_.IsInitialized()
			&& end_time_ > 0;
	}

	void PbftVcInstance::SetLastNewViewTime(int64_t current_time) {
		last_newview_time_ = current_time;
	}

	bool PbftVcInstance::SendNewViewAgain(Pbft *pbft, int64_t current_time) {
		PeerMessagePointer new_ptr = pbft->IncPeerMessageRound(newview_, ++new_view_round_);
		SetLastNewViewTime(current_time);
		return pbft->SendMessage(new_ptr);
	}

	bool PbftVcInstance::SendNewView(Pbft *pbft, int64_t current_time, PeerMessagePointer new_ptr) {
		SetLastNewViewTime(current_time);
		newview_ = *(protocol::PbftEnv *)new_ptr->data_;
		return pbft->SendMessage(new_ptr);
	}

	bool PbftVcInstance::ShouldTeminated(int64_t current_time, int64_t time_out) {
		return current_time - start_time_ >= time_out;
	}

	void PbftVcInstance::SetLastProposeTime(int64_t current_time) {
		last_propose_time_ = current_time;
	}

	void PbftVcInstance::ChangeComplete(int64_t current_time) {
		end_time_ = current_time;
	}

	std::string PbftDesc::GetPrePrepare(const protocol::PbftPrePrepare &pre_prepare) {
		return utils::String::Format("type:Pre-Prepare | vn:" FMT_I64 " seq:" FMT_I64 " replica:" FMT_I64 " | value:%s",
			pre_prepare.view_number(), pre_prepare.sequence(), pre_prepare.replica_id(), GetValueString(pre_prepare.value()).c_str());
	}

	std::string PbftDesc::GetPrepare(const protocol::PbftPrepare &prepare) {
		return utils::String::Format("type:Prepare | vn:" FMT_I64 " seq:" FMT_I64 " replica:" FMT_I64 " | value:%s",
			prepare.view_number(), prepare.sequence(), prepare.replica_id(), utils::String::BinToHexString(prepare.value_digest()).c_str());
	}

	std::string PbftDesc::GetCommit(const protocol::PbftCommit &commit) {
		return utils::String::Format("type:Commit | vn:" FMT_I64 " seq:" FMT_I64 " replica:" FMT_I64 " | value:%s",
			commit.view_number(), commit.sequence(), commit.replica_id(), utils::String::BinToHexString(commit.value_digest()).c_str());
	}

	std::string PbftDesc::GetCheckPoint(const protocol::PbftCheckPoint &checkpoint) {
		return utils::String::Format("type:CheckPoint | seq:" FMT_I64 " replica:" FMT_I64 "| digest:%s",
			checkpoint.sequence(), checkpoint.replica_id(), utils::String::BinToHexString(checkpoint.state_digest()).c_str());
	}

	std::string PbftDesc::GetViewChange(const protocol::PbftViewChange &viewchange) {
		std::string checkpoints;
		for (int32_t i = 0; i < viewchange.checkpoints_size(); i++) {
			const protocol::PbftEnv &checkpoint_env = viewchange.checkpoints(i);
			if (i > 0) {
				checkpoints = utils::String::AppendFormat(checkpoints, ",");
			}
			checkpoints = utils::String::AppendFormat(checkpoints, "%s", GetPbft(checkpoint_env.pbft()).c_str());
		}
		std::string prepared_set;
		for (int32_t i = 0; i < viewchange.prepared_set_size(); i++) {
			const protocol::PbftPreparedSet &prepare_set_env = viewchange.prepared_set(i);
			if (i > 0) {
				prepared_set = utils::String::AppendFormat(prepared_set, ",");
			}
			std::string prepares;
			for (int32_t m = 0; m < prepare_set_env.prepare_size(); m++) {
				const protocol::PbftEnv &prepare = prepare_set_env.prepare(m);
				if (m > 0) {
					prepares = utils::String::AppendFormat(prepares, ",");
				}
				prepares = utils::String::AppendFormat(prepares, "%s", GetPbft(prepare.pbft()).c_str());
			}
			prepared_set = utils::String::AppendFormat(prepared_set, "pp:%s|p:%s ",
				GetPbft(prepare_set_env.pre_prepare().pbft()).c_str(),
				prepares.c_str());
		}
		return utils::String::Format("type:ViewChange | vn:" FMT_I64 " seq:" FMT_I64 " replica:" FMT_I64 " |ckp:[%s] | prepared_set:[%s]",
			viewchange.view_number(), viewchange.sequence(), viewchange.replica_id(), checkpoints.c_str(), prepared_set.c_str());
	}

	std::string PbftDesc::GetNewView(const protocol::PbftNewView &new_view) {
		std::string viewchanges;
		for (int32_t i = 0; i < new_view.view_changes_size(); i++) {
			const protocol::PbftEnv &viewchange_env = new_view.view_changes(i);
			if (i > 0) {
				viewchanges = utils::String::AppendFormat(viewchanges, ",");
			}
			viewchanges = utils::String::AppendFormat(viewchanges, "%s", GetPbft(viewchange_env.pbft()).c_str());
		}
		std::string pre_prepares;
		for (int32_t i = 0; i < new_view.pre_prepares_size(); i++) {
			const protocol::PbftEnv &pre_prepare_env = new_view.pre_prepares(i);
			if (i > 0) {
				pre_prepares = utils::String::AppendFormat(pre_prepares, ",");
			}
			pre_prepares = utils::String::AppendFormat(pre_prepares, "%s", GetPbft(pre_prepare_env.pbft()).c_str());
		}
		return utils::String::Format("type:NewView | vn:" FMT_I64 " replica:" FMT_I64 " |vc:[%s] | pre_prepares:[%s]",
			new_view.view_number(), new_view.replica_id(), viewchanges.c_str(), pre_prepares.c_str());
	}

	std::string PbftDesc::GetValueString(const protocol::Value &value) {
		return utils::String::Format("hash:%s, close time:%s", utils::String::Bin4ToHexString(value.hash_set()).c_str(),
			utils::Timestamp(value.close_time()).ToFormatString(true).c_str());
	}

	std::string PbftDesc::GetPbft(const protocol::Pbft &pbft) {
		std::string message;
		switch (pbft.type()) {
		case protocol::PBFT_TYPE_PREPREPARE:{
			const protocol::PbftPrePrepare &pre_prepare = pbft.pre_prepare();
			message = GetPrePrepare(pre_prepare);
			break;
		}
		case protocol::PBFT_TYPE_PREPARE:{
			const protocol::PbftPrepare &prepare = pbft.prepare();
			message = GetPrepare(prepare);
			break;
		}
		case protocol::PBFT_TYPE_COMMIT:{
			const protocol::PbftCommit &commit = pbft.commit();
			message = GetCommit(commit);
			break;
		}
		case protocol::PBFT_TYPE_CHECKPOINT:{
			const protocol::PbftCheckPoint &checkpoint = pbft.checkpoint();
			message = GetCheckPoint(checkpoint);
			break;
		}
		case protocol::PBFT_TYPE_VIEWCHANGE:{
			const protocol::PbftViewChange &viewchange = pbft.view_change();
			message = GetViewChange(viewchange);
			break;
		}
		case protocol::PBFT_TYPE_NEWVIEW:{
			const protocol::PbftNewView &new_view = pbft.new_view();
			message = GetNewView(new_view);
			break;
		}
		default:
			break;
		}
		return message;
	}

	const char *PbftDesc::GetMessageTypeDesc(enum protocol::PbftMessageType type) {
		switch (type) {
		case protocol::PBFT_TYPE_PREPREPARE:
		{
			return "PBFT-PRE-PREPARE";
		}
		case protocol::PBFT_TYPE_PREPARE:{
			return "PBFT-PREPARE";
		}
		case protocol::PBFT_TYPE_COMMIT:{
			return "PBFT-COMMIT";
		}
		case protocol::PBFT_TYPE_CHECKPOINT:{
			return "PBFT-CHECKPOINT";
		}
		case protocol::PBFT_TYPE_VIEWCHANGE:{
			return "PBFT-VIEWCHANGE";
		}
		case protocol::PBFT_TYPE_NEWVIEW:{
			return "PBFT-NEWVIEW";
		}
		default:{
			return "";
		}
		}
	}

	const char *PbftDesc::SEQUENCE_NAME = "pbft_sequence";
	const char *PbftDesc::VIEWNUMBER_NAME = "pbft_viewnumber";
	const char *PbftDesc::CHECKPOINT_NAME = "pbft_checkpoint";
	const char *PbftDesc::LAST_EXE_SEQUENCE_NAME = "pbft_lastexesequence";
	const char *PbftDesc::LOW_WATER_MRAK_NAME = "pbft_lowwatermark";
	const char *PbftDesc::INSTANCE_NAME = "pbft_instance";
}