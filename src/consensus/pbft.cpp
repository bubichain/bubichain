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
#include "pbft.h"

namespace bubi {
	Pbft::Pbft() :view_number_(0),
		last_exe_seq_(0),
		sequence_(1),
		low_water_mark_(0),
		fault_number_(0),
		view_active_(true),
		new_view_repond_timer_(0),
		last_check_time_(utils::Timestamp::HighResolution()) {
		name_ = "pbft";

		//should load from the configure
		ckp_count_ = 5;
		ckp_interval_ = 2 * ckp_count_;
	}

	Pbft::~Pbft() {

	}

	bool Pbft::Initialize() {
		if (!Consensus::Initialize()) {
			return false;
		}

		if (validators_.size() < 2) {
			LOG_ERROR("The validator size(" FMT_SIZE ") is not large than 2", validators_.size());
			return false;
		}

		if (validators_.size() < 4) {
			LOG_WARN("The validator size(" FMT_SIZE ") can't tolerate fault node", validators_.size());
		}

		fault_number_ = (validators_.size() - 1) / 3;



		//should load from the check point
		LoadValues();

		//sequence_ = last_exe_seq_ + 1;
		// 0 : 1~5     5:6~10      10 : 11~15
		//low_water_mark_ = (sequence_ - 1) / ckp_count_ * ckp_count_;
		LOG_INFO("The validator size(" FMT_SIZE ") can tolerate " FMT_SIZE " fault nodes, it(replica_id:" FMT_I64 ") think it %s a leader",
			validators_.size(), fault_number_, replica_id_, view_number_ % validators_.size() == replica_id_ ? "is" : "isnot");

		return true;
	}

	bool Pbft::Exit() {
		return true;
	}

	void Pbft::LoadValues() {
		LoadValue(PbftDesc::SEQUENCE_NAME, sequence_);
		LoadValue(PbftDesc::LOW_WATER_MRAK_NAME, low_water_mark_);

		LoadValue(PbftDesc::VIEWNUMBER_NAME, view_number_);
		LoadCheckPoint();
		LoadValue(PbftDesc::LAST_EXE_SEQUENCE_NAME, last_exe_seq_);
		LoadInstance();
	}

	void Pbft::ClearStatus() {
		DelValue(PbftDesc::SEQUENCE_NAME);
		DelValue(PbftDesc::LOW_WATER_MRAK_NAME);
		DelValue(PbftDesc::VIEWNUMBER_NAME);
		DelValue(PbftDesc::LAST_EXE_SEQUENCE_NAME);
		DelValue(PbftDesc::CHECKPOINT_NAME);
		DelValue(PbftDesc::INSTANCE_NAME);
	}

	int32_t Pbft::LoadCheckPoint() {
		do {
			std::string str_checkpoint;
			int32_t ret = LoadValue(PbftDesc::CHECKPOINT_NAME, str_checkpoint);
			if (ret < 0) {
				LOG_ERROR("Load checkpoint failed");
				return ret;
			}
			else if (ret == 0) {
				return ret;
			}

			Json::Value json_checkpoint;
			if (!json_checkpoint.fromString(str_checkpoint)) {
				LOG_ERROR("Parse loaded checkpoint failed, string checkpoint(%s)", str_checkpoint.c_str());
				return -1;
			}

			for (uint32_t i = 0; i < json_checkpoint.size(); i++) {
				Json::Value &item = json_checkpoint[i];

				//for checkpoint instance
				PbftCkpInstanceIndex index;
				index.sequence_ = item["sequence"].asInt64();
				index.state_digest_ = utils::String::HexStringToBin(item["state_digest"].asString());
				PbftCkpInstance &instance = ckp_instances_[index];

				//for stable tag
				instance.stable_ = item["stable"].asBool();

				//for message buffer
				const Json::Value &msg_buffer_json = item["msg_buffer"];
				for (uint32_t m = 0; m < msg_buffer_json.size(); m++) {
					protocol::PbftEnv env;
					if (!env.ParseFromString(utils::String::HexStringToBin(msg_buffer_json[m].asString()))) {
						LOG_ERROR("Consensus load checkpoint, parse message buffer string failed");
						continue;
					}
					instance.msg_buf_.push_back(env);
				}

				//for checkpoint map
				const Json::Value &checkpoint_map = item["checkpoints"];
				for (Json::ValueConstIterator iter = checkpoint_map.begin(); iter != checkpoint_map.end(); iter++) {
					std::string key = iter.memberName();
					std::string checkpoint_str = utils::String::HexStringToBin(checkpoint_map[key].asString());
					protocol::PbftCheckPoint cp;
					if (!cp.ParseFromString(checkpoint_str)) {
						LOG_ERROR("Consensus load checkpoint, parse checkpoint map string failed");
						continue;
					}
					instance.checkpoints_.insert(std::make_pair(utils::String::Stoi64(key), cp));
				}
			}
		} while (false);

		return 1;
	}

	bool Pbft::SaveCheckPoint(ValueSaver &saver) {
		Json::Value total = Json::Value(Json::arrayValue);
		for (PbftCkpInstanceMap::const_iterator iter = ckp_instances_.begin(); iter != ckp_instances_.end(); iter++) {
			const PbftCkpInstanceIndex &index = iter->first;
			const PbftCkpInstance &instance = iter->second;
			Json::Value &item = total[total.size()];
			item["sequence"] = index.sequence_;
			item["state_digest"] = utils::String::BinToHexString(index.state_digest_);
			item["stable"] = instance.stable_;

			//for message buffer
			Json::Value &msg_buffer_json = item["msg_buffer"];
			for (PbftPhaseVector::const_iterator iter_msg = instance.msg_buf_.begin(); iter_msg != instance.msg_buf_.end(); iter_msg++) {
				msg_buffer_json[msg_buffer_json.size()] = utils::String::BinToHexString(iter_msg->SerializeAsString());
			}

			//for checkpoint map
			Json::Value &checkpoint_map = item["checkpoints"];
			for (PbftCheckPointMap::const_iterator iter_cp = instance.checkpoints_.begin(); iter_cp != instance.checkpoints_.end(); iter_cp++) {
				checkpoint_map[utils::String::ToString(iter_cp->first)] = utils::String::BinToHexString(iter_cp->second.SerializeAsString());
			}
		}

		saver.SaveValue(PbftDesc::CHECKPOINT_NAME, total.toFastString());

		return true;
	}

	int32_t Pbft::LoadInstance() {
		do {
			std::string str_instance;
			int32_t ret = LoadValue(PbftDesc::INSTANCE_NAME, str_instance);
			if (ret <= 0) {
				LOG_ERROR("Load instances failed");
				return ret;
			}
			else if (ret == 0) {
				return ret;
			}

			Json::Value json_instance;
			if (!json_instance.fromString(str_instance)) {
				LOG_ERROR("Parse loaded instances failed, string instances(%s)", str_instance.c_str());
				return -1;
			}

			for (uint32_t i = 0; i < json_instance.size(); i++) {
				const Json::Value &item = json_instance[i];

				//for checkpoint instance
				PbftInstanceIndex index(item["view_number"].asInt64(), item["sequence"].asInt64());
				PbftInstance &instance = instances_[index];

				//for stable tag
				instance.phase_ = (PbftInstancePhase)item["phase"].asUInt();
				instance.phase_item_ = item["phase_item"].asUInt();

				//for message buffer
				const Json::Value &msg_buffer_json = item["msg_buffer"];
				for (uint32_t m = 0; m < msg_buffer_json.size(); m++) {
					PbftPhaseVector pv;
					const Json::Value &msg_item_json = msg_buffer_json[m];
					for (uint32_t n = 0; n < msg_item_json.size(); n++) {
						protocol::PbftEnv env;
						if (!env.ParseFromString(utils::String::HexStringToBin(msg_item_json[n].asString()))) {
							LOG_ERROR("Consensus load instance, parse message buffer string failed");
							continue;
						}
						pv.push_back(env);
					}
					instance.msg_buf_[m] = pv;
				}

				//for pre-prepare message
				if (item.isMember("pre_prepare_msg") && !item["pre_prepare_msg"].asString().empty()) {
					if (!instance.pre_prepare_msg_.ParseFromString(utils::String::HexStringToBin(item["pre_prepare_msg"].asString()))) {
						LOG_ERROR("Consensus load instance, parse pre-prepare message string failed");
					}
				}

				//for tags
				instance.start_time_ = item["start_time"].asInt64();
				instance.end_time_ = item["end_time"].asInt64();
				instance.last_propose_time_ = item["last_propose_time"].asInt64();
				instance.have_send_viewchange_ = item["have_send_viewchange"].asBool();
				instance.pre_prepare_round_ = item["pre_prepare_round"].asUInt();
				instance.check_value_result_ = item["check_value_result"].asInt();
				instance.last_commit_send_time_ = item["last_commit_send_time"].asInt64();

				if (instance.end_time_ == 0) {
					instance.start_time_ = utils::Timestamp::HighResolution();
				}

				//for pre-prepare
				if (!instance.pre_prepare_.ParseFromString(utils::String::HexStringToBin(item["pre_prepare"].asString()))) {
					LOG_ERROR("Consensus load instance, parse pre-prepare string failed");
				}
				//for prepare
				const Json::Value &prepares = item["prepares"];
				for (Json::ValueConstIterator iter = prepares.begin(); iter != prepares.end(); iter++) {
					std::string key = iter.memberName();
					std::string prepare_str = utils::String::HexStringToBin(prepares[key].asString());
					protocol::PbftPrepare ppre;
					if (!ppre.ParseFromString(prepare_str)) {
						LOG_ERROR("Consensus load instance, parse prepare string failed");
						continue;
					}
					instance.prepares_.insert(std::make_pair(utils::String::Stoi64(key), ppre));
				}
				//for commit
				const Json::Value &commits = item["commits"];
				for (Json::ValueConstIterator iter = commits.begin(); iter != commits.end(); iter++) {
					std::string key = iter.memberName();
					std::string value = utils::String::HexStringToBin(commits[key].asString());
					protocol::PbftCommit commit;
					if (!commit.ParseFromString(value)) {
						LOG_ERROR("Consensus load instance, parse commit string failed");
						continue;
					}
					instance.commits_.insert(std::make_pair(utils::String::Stoi64(key), commit));
				}

			}
		} while (false);

		return 1;
	}

	bool Pbft::SaveInstance(ValueSaver &saver) {
		Json::Value total = Json::Value(Json::arrayValue);
		for (PbftInstanceMap::const_iterator iter = instances_.begin(); iter != instances_.end(); iter++) {
			const PbftInstanceIndex &index = iter->first;
			const PbftInstance &instance = iter->second;
			Json::Value &item = total[total.size()];
			item["sequence"] = index.sequence_;
			item["view_number"] = index.view_number_;

			//for tags
			item["phase"] = instance.phase_;
			item["phase_item"] = instance.phase_item_;
			item["start_time"] = instance.start_time_;
			item["end_time"] = instance.end_time_;
			item["last_propose_time"] = instance.last_propose_time_;
			item["have_send_viewchange"] = instance.have_send_viewchange_;
			item["pre_prepare_round"] = instance.pre_prepare_round_;
			item["check_value_result"] = instance.check_value_result_;
			item["last_commit_send_time"] = instance.last_commit_send_time_;

			//for message buffer
			Json::Value &msg_buffer_json = item["msg_buffer"];
			for (PbftPhaseVector2::const_iterator iter_msg = instance.msg_buf_.begin(); iter_msg != instance.msg_buf_.end(); iter_msg++) {
				Json::Value &msg_buffer_item_json = msg_buffer_json[msg_buffer_json.size()];
				const PbftPhaseVector &pitem = *iter_msg;
				for (PbftPhaseVector::const_iterator iter_item = pitem.begin(); iter_item != pitem.end(); iter_item++) {
					msg_buffer_item_json[msg_buffer_item_json.size()] = utils::String::BinToHexString(iter_item->SerializeAsString());
				}
			}

			//for pre-prepare message
			if (instance.pre_prepare_msg_.IsInitialized()) item["pre_prepare_msg"] = utils::String::BinToHexString(instance.pre_prepare_msg_.SerializeAsString());

			//for pre-prepare
			if (instance.pre_prepare_.IsInitialized()) item["pre_prepare"] = utils::String::BinToHexString(instance.pre_prepare_.SerializeAsString());

			//for prepare
			Json::Value &prepares = item["prepares"];
			for (PbftPrepareMap::const_iterator iter = instance.prepares_.begin(); iter != instance.prepares_.end(); iter++) {
				prepares[utils::String::ToString(iter->first)] = utils::String::BinToHexString(iter->second.SerializeAsString());
			}

			//for commit
			Json::Value &commmits = item["commits"];
			for (PbftCommitMap::const_iterator iter = instance.commits_.begin(); iter != instance.commits_.end(); iter++) {
				commmits[utils::String::ToString(iter->first)] = utils::String::BinToHexString(iter->second.SerializeAsString());
			}
		}

		saver.SaveValue(PbftDesc::INSTANCE_NAME, total.toFastString());

		return true;
	}

	void Pbft::OnTimer(int64_t current_time) {

		//lock the instances in the child for less trigger
		utils::MutexGuard guard(lock_);

		//check the lost sequence have not been achieve
		PbftInstance *last_prepared_instance = NULL;
		const PbftInstanceIndex *index = NULL;
		PbftInstanceMap::iterator last_prepared_iter = instances_.end();
		for (PbftInstanceMap::iterator iter = instances_.begin(); iter != instances_.end(); iter++) {
			//check if we should send the prepare again

			//check if it is timeout
			if (iter->second.IsExpire(current_time) &&
				!iter->second.have_send_viewchange_
				) {
				LOG_INFO("The pbft instance is timeout, vn(" FMT_I64 ") seq(" FMT_I64 ") phase(%d)",
					iter->first.view_number_, iter->first.sequence_, iter->second.phase_);
				OnTxTimeout();
				iter->second.have_send_viewchange_ = true;
			}

			if (iter->second.NeedSendAgain(current_time) &&
				view_active_ &&
				iter->second.pre_prepare_msg_.IsInitialized()) {
				iter->second.SendPrepareAgain(this, current_time);
				LOG_INFO("Send pre-prepare message again actively, view number(" FMT_I64 "),sequence(" FMT_I64 ") round number(%u)",
					iter->first.view_number_, iter->first.sequence_, iter->second.pre_prepare_round_);
			}

			if (iter->second.phase_ >= PBFT_PHASE_PREPARED) {
				last_prepared_instance = &iter->second;
				index = &iter->first;
			}
		}

		if (last_prepared_instance != NULL &&
			last_prepared_instance->check_value_result_ == Consensus::CHECK_VALUE_VALID &&
			last_prepared_instance->NeedSendCommitAgain(current_time)) { //for broadcast only
			PeerMessagePointer commit_msg = NewCommit(last_prepared_instance->prepares_.begin()->second, ++last_prepared_instance->commit_rount_);
			SendMessage(commit_msg);
			last_prepared_instance->SetLastCommitSendTime(current_time);
			LOG_INFO("Send commit message again actively, view number(" FMT_I64 "),sequence(" FMT_I64 ") round number(%u)",
				index->view_number_, index->sequence_, last_prepared_instance->commit_rount_);
		}

		//check the view change timeout, and get the last new view send
		PbftVcInstance *lastvc_instance = NULL;
		for (PbftVcInstanceMap::iterator iter_vc = vc_instances_.begin(); iter_vc != vc_instances_.end(); iter_vc++) {
			if (iter_vc->second.NeedSendAgain(current_time) && iter_vc->second.view_change_msg_.IsInitialized()) {
				PeerMessagePointer new_ptr = IncPeerMessageRound(iter_vc->second.view_change_msg_, ++iter_vc->second.view_change_round_);
				SendMessage(new_ptr);
				iter_vc->second.SetLastProposeTime(current_time);
				LOG_INFO("Send view-change message again actively, view number(" FMT_I64 "),round number(%u)",
					iter_vc->first, iter_vc->second.view_change_round_);
			}

			if (iter_vc->second.NeedSendNewViewAgain(current_time) &&
				iter_vc->second.view_number_ % validators_.size() == replica_id_) {
				lastvc_instance = &iter_vc->second;
			}
		}

		if (lastvc_instance != NULL) {
			lastvc_instance->SendNewViewAgain(this, current_time);
			LOG_INFO("Send new view message again actively, view number(" FMT_I64 "),round number(%u)",
				lastvc_instance->view_number_, lastvc_instance->new_view_round_);
		}

		//check the view change object should be teminated
		//for (PbftVcInstanceMap::iterator iter_vc = vc_instances_.begin(); iter_vc != vc_instances_.end(); ){
		//	if (iter_vc->second.ShouldTeminated(current_time, g_pbft_vcinstance_terminatedtime_)){
		//		vc_instances_.erase(iter_vc++);
		//	}
		//	else{
		//		iter_vc++;
		//	}
		//}

		//check the stable checkpoint's sequence is too large than the execute sequence
		int64_t last_check_point_seq = 0;
		for (PbftCkpInstanceMap::iterator iter = ckp_instances_.begin(); iter != ckp_instances_.end(); iter++) {
			PbftCkpInstance &ckp_instance = iter->second;
			const PbftCkpInstanceIndex &ckp_index = iter->first;
			if (ckp_instance.stable_) {
				last_check_point_seq = ckp_index.sequence_ / ckp_count_ * ckp_count_;
			}
		}

		if (last_check_point_seq > 0 && last_check_point_seq - last_exe_seq_ >= ckp_count_) {
			//trigger the ledger synchronize
			//call ledger manager, and return the block hash
			//LOG_INFO("Call the ledger synchronize");
		}

	}

	bool Pbft::InWaterMark(int64_t seq) {
		return seq > low_water_mark_ && seq <= low_water_mark_ + ckp_interval_;
	}

	bool Pbft::Request(int64_t block_seq, const protocol::Value &value) {
		if (view_number_ % validators_.size() != replica_id_) {
			return false;
		}
		LOG_INFO("Start to request value(%s)", GetValueString(value).c_str());

		if (!view_active_) {
			LOG_INFO("The view(vn:" FMT_I64 ") is not active, so request failed", view_number_);
			return false;
		}

		//lock the instances
		utils::MutexGuard lock_guad(lock_);
		ValueSaver saver;

		int64_t seq_find = -1;
		//delete the last uncommitted logs
		for (PbftInstanceMap::iterator iter_inst = instances_.begin();
			iter_inst != instances_.end();
			) {
			if (iter_inst->first.sequence_ > last_exe_seq_ && iter_inst->second.phase_ < PBFT_PHASE_COMMITED) {
				LOG_INFO("Before request, erase the uncommitted log, sequence(" FMT_I64 ")", iter_inst->first.sequence_);
				instances_.erase(iter_inst++);
			}
			else {
				if (iter_inst->first.sequence_ > seq_find) {
					seq_find = iter_inst->first.sequence_;
				}

				iter_inst++;
			}
		}

		if (seq_find > 0 && seq_find >= last_exe_seq_) {
			sequence_ = seq_find + 1;

			saver.SaveValue(PbftDesc::SEQUENCE_NAME, sequence_);
			SaveInstance(saver);
		}

		PeerMessagePointer pmessage = NewPrePrepare(value, block_seq);
		protocol::PbftEnv *env = (protocol::PbftEnv *)pmessage->data_;

		//check the index if exist
		PbftInstanceIndex index(view_number_, sequence_);
		PbftInstanceMap::const_iterator iter = instances_.find(index);
		if (iter != instances_.end()) {
			LOG_ERROR("Request failed, the view number(" FMT_I64 ") sequence(" FMT_I64 ") has been sent", view_number_, sequence_);
			return false;
		}

		//auto increase the sequence
		sequence_++;
		saver.SaveValue(PbftDesc::SEQUENCE_NAME, sequence_);

		//insert the instance to map
		PbftInstance pinstance;
		pinstance.pre_prepare_msg_ = *(protocol::PbftEnv *)pmessage->data_;
		pinstance.phase_ = PBFT_PHASE_PREPREPARED;
		pinstance.pre_prepare_ = env->pbft().pre_prepare();
		pinstance.msg_buf_[env->pbft().type()].push_back(*env);
		instances_.insert(std::make_pair(index, pinstance));
		SaveInstance(saver);

		saver.Commit();
		LOG_INFO("Send pre-prepare message, view number(" FMT_I64 "),sequence(" FMT_I64 ") for block(" FMT_I64 ")", view_number_, index.sequence_, block_seq);
		//broadcast the message to other nodes
		return SendMessage(pmessage);
	}

	bool Pbft::RepairStatus() {
		bool normal = true;
		for (PbftInstanceMap::iterator iter_inst = instances_.begin();
			iter_inst != instances_.end();
			iter_inst++
			) {
			if (iter_inst->first.sequence_ > last_exe_seq_ &&
				iter_inst->second.phase_ < PBFT_PHASE_COMMITED &&
				IsLeader() && iter_inst->first.view_number_ == view_number_) {
				normal = false;

				iter_inst->second.SendPrepareAgain(this, utils::Timestamp::HighResolution());
				LOG_INFO("Send pre-prepare message again repaired, view number(" FMT_I64 "),sequence(" FMT_I64 ") round number(%u)",
					iter_inst->first.view_number_, iter_inst->first.sequence_, iter_inst->second.pre_prepare_round_);
			}
		}

		return normal;
	}

	bool Pbft::CheckMessageItem(const protocol::PbftEnv &env) {
		//this function should output the error log
		const protocol::Pbft &pbft = env.pbft();
		const protocol::Signature &sig = env.signature();

		//get the node address
		PublicKey public_key(sig.public_key());

		//check the node id if exist in the validator' list
		int64_t should_replica_id = GetValidatorIndex(public_key.GetBase58Address());
		if (should_replica_id < 0) {
			LOG_ERROR("Cann't find the validator(%s) in list", public_key.GetBase58Address().c_str());
			return false;
		}

		//check pbft type is not large than max
		int64_t replica_id = -1;
		switch (pbft.type()) {
		case protocol::PBFT_TYPE_PREPREPARE:
		{
			if (!pbft.has_pre_prepare()) {
				LOG_ERROR("Check received message failed, Pre-Prepare message has not related object");
				return false;
			}
			replica_id = pbft.pre_prepare().replica_id();
			break;
		}
		case protocol::PBFT_TYPE_PREPARE:
		{
			if (!pbft.has_prepare()) {
				LOG_ERROR("Check received message failed, Prepare message has not related object");
				return false;
			}
			replica_id = pbft.prepare().replica_id();
			break;
		}
		case protocol::PBFT_TYPE_COMMIT:
		{
			if (!pbft.has_commit()) {
				LOG_ERROR("Check received message failed, Commit message has not related object");
				return false;
			}
			replica_id = pbft.commit().replica_id();
			break;
		}
		case protocol::PBFT_TYPE_CHECKPOINT:
		{
			if (!pbft.has_checkpoint()) {
				LOG_ERROR("Check received message failed, Check point message has not related object");
				return false;
			}
			replica_id = pbft.checkpoint().replica_id();
			break;
		}
		case protocol::PBFT_TYPE_VIEWCHANGE:
		{
			if (!pbft.has_view_change()) {
				LOG_ERROR("Check received message failed, View change message has not related object");
				return false;
			}
			replica_id = pbft.view_change().replica_id();
			break;
		}
		case protocol::PBFT_TYPE_NEWVIEW:
		{
			if (!pbft.has_new_view()) {
				LOG_ERROR("Check received message failed, New view message has not related object");
				return false;
			}
			replica_id = pbft.new_view().replica_id();
			break;
		}
		default:
		{
			LOG_ERROR("Check received message failed, Cannot parse the type(%d)", pbft.type());
			return false;
		}
		}
		//check should replica_id equal be the object id
		if (replica_id != should_replica_id) {
			LOG_ERROR("Check received message(type:%s) failed, message replica id(" FMT_I64 ") not equal the signature id(" FMT_I64")",
				PbftDesc::GetPbft(pbft).c_str(), replica_id, should_replica_id);
			return false;
		}

		//check the signature
		if (!PublicKey::Verify(pbft.SerializeAsString(), sig.sign_data(), sig.public_key())) {
			LOG_ERROR("Check received message's signature failed, desc(%s)", PbftDesc::GetPbft(pbft).c_str());
			return false;
		}
		return true;
	}

	bool Pbft::TraceOutPbftPrePrepare(const protocol::PbftEnv &env) {
		const protocol::Pbft &pbft = env.pbft();
		const protocol::PbftPrePrepare &pre_pre = pbft.pre_prepare();
		PbftInstanceIndex index(pre_pre.view_number(), pre_pre.sequence());
		PbftInstanceMap::iterator iter = out_pbft_instances_.find(index);

		PbftInstance &instance = out_pbft_instances_[index];
		//if (iter == out_pbft_instances_.end()){
		//	PbftInstance instance;
		*instance.pre_prepare_.mutable_value() = pre_pre.value();
		instance.pre_prepare_.set_value_digest(pre_pre.value_digest());
		//	out_pbft_instances_.insert(std::make_pair(index, instance));
		//}

		LOG_INFO("Receive trace out pre-prepare from replica(" FMT_I64 "),  view number(" FMT_I64 "),sequence(" FMT_I64 "),round number(%u)",
			pre_pre.replica_id(), pre_pre.view_number(), pre_pre.sequence(), pbft.round_number());
		TryDoTraceOut(index, instance);
		return true;
	}

	bool Pbft::TraceOutPbftCommit(const protocol::PbftEnv &env) {
		const protocol::Pbft &pbft = env.pbft();
		const protocol::PbftCommit &commit = pbft.commit();
		PbftInstanceIndex index(commit.view_number(), commit.sequence());
		//check if it exist in normal object
		PbftInstanceMap::iterator iter_normal = instances_.find(index);
		if (iter_normal != instances_.end()) {
			LOG_INFO("Receive trace out commit but normal from replica(" FMT_I64 "),  view number(" FMT_I64 "),sequence(" FMT_I64 "),round number(%u)",
				commit.replica_id(), commit.view_number(), commit.sequence(), pbft.round_number());
			return OnCommit(pbft, iter_normal->second);
		}

		PbftInstanceMap::iterator iter = out_pbft_instances_.find(index);
		if (iter == out_pbft_instances_.end()) {
			PbftInstance instance;
			instance.pre_prepare_.set_value_digest(commit.value_digest());
			out_pbft_instances_.insert(std::make_pair(index, instance));
		}

		PbftInstance &instance_exist = out_pbft_instances_[index];
		if (0 != CompareValue(instance_exist.pre_prepare_.value_digest(), commit.value_digest())) {
			LOG_ERROR("The commit message view number(" FMT_I64 ") seq(" FMT_I64 ") is not equal with pre commit message",
				commit.view_number(), commit.sequence());
			return false;
		}

		LOG_INFO("Receive trace out commit from replica(" FMT_I64 "),  view number(" FMT_I64 "),sequence(" FMT_I64 "),round number(%u)",
			commit.replica_id(), commit.view_number(), commit.sequence(), pbft.round_number());
		instance_exist.commits_.insert(std::make_pair(commit.replica_id(), commit));
		TryDoTraceOut(index, instance_exist);

		return true;
	}

	void Pbft::TryDoTraceOut(const PbftInstanceIndex &index, const PbftInstance &instance) {
		if (instance.commits_.size() >= GetQuorumSize() + 1 /*&& instance.pre_prepare_.has_value()*/) {
			LOG_INFO("Trace out pbft commited, vn(" FMT_I64 "), seq(" FMT_I64 ")", index.view_number_, index.sequence_);
			if (index.sequence_ - last_exe_seq_ >= ckp_interval_) {
				LOG_INFO("The trace out sequence(" FMT_I64 ") is large than last exe sequence(" FMT_I64 ") for checkpoint interval(" FMT_I64 "), try move watermark",
					index.sequence_, last_exe_seq_, ckp_interval_);

				//we should move to the new watermark
				view_active_ = true;
				view_number_ = index.view_number_;
				sequence_ = index.sequence_;
				low_water_mark_ = sequence_ / ckp_count_ * ckp_count_;

				ValueSaver saver;
				saver.SaveValue(PbftDesc::LOW_WATER_MRAK_NAME, low_water_mark_);

				//clear the view change instance
				for (PbftVcInstanceMap::iterator iter_vc = vc_instances_.begin(); iter_vc != vc_instances_.end();) {
					if (iter_vc->second.view_change_msg_.IsInitialized()) {
						vc_instances_.erase(iter_vc++);
					}
					else {
						iter_vc++;
					}
				}

				//delete checkpoint
				for (PbftCkpInstanceMap::iterator iter = ckp_instances_.begin(); iter != ckp_instances_.end();) {
					PbftCkpInstance &ckp_instance = iter->second;
					if (iter->first.sequence_ < sequence_) ckp_instances_.erase(iter++);
					else iter++;
				}
				SaveCheckPoint(saver);

				//delete instance
				for (PbftInstanceMap::iterator iter = instances_.begin(); iter != instances_.end();) {
					const PbftInstanceIndex &index = iter->first;
					if (index.sequence_ <= sequence_) instances_.erase(iter++);
					else iter++;
				}
				SaveInstance(saver);

				last_exe_seq_ = sequence_;
				saver.SaveValue(PbftDesc::LAST_EXE_SEQUENCE_NAME, last_exe_seq_);

				//clear the Out pbft instance
				out_pbft_instances_.clear();
			}
		}
	}

	PbftInstance *Pbft::CreateInstanceIfNotExist(const protocol::PbftEnv &env) {
		const protocol::Pbft &pbft = env.pbft();
		int64_t view_number = 0;
		int64_t sequence = 0;
		switch (pbft.type()) {
		case protocol::PBFT_TYPE_PREPREPARE:
		{
			view_number = pbft.pre_prepare().view_number();
			sequence = pbft.pre_prepare().sequence();
			break;
		}
		case protocol::PBFT_TYPE_PREPARE:{

			view_number = pbft.prepare().view_number();
			sequence = pbft.prepare().sequence();
			break;
		}
		case protocol::PBFT_TYPE_COMMIT:{

			view_number = pbft.commit().view_number();
			sequence = pbft.commit().sequence();
			break;
		}
		default:{
			return NULL;
		}
		}

		bool in_water = InWaterMark(sequence);
		bool same_view = view_number_ == view_number;
		if (!in_water || !same_view) {

			if (!in_water) LOG_TRACE("The message(type:%s) sequence(" FMT_I64 ") is not in water mark(" FMT_I64 ", " FMT_I64"), desc(%s)", PbftDesc::GetMessageTypeDesc(pbft.type()),
				sequence, low_water_mark_, low_water_mark_ + ckp_interval_, PbftDesc::GetPbft(pbft).c_str());
			if (!same_view)	LOG_ERROR("The message(type:%s) view number(" FMT_I64 ") != this view number(" FMT_I64 "), desc(%s)", PbftDesc::GetMessageTypeDesc(pbft.type()),
				view_number, view_number_, PbftDesc::GetPbft(pbft).c_str());
			if (sequence > last_exe_seq_) {
				if (pbft.type() == protocol::PBFT_TYPE_COMMIT) {
					TraceOutPbftCommit(env);
				}
				//else if (pbft.type() == protocol::PBFT_TYPE_PREPREPARE){
				//	TraceOutPbftPrePrepare(env);
				//}
			}
			return NULL;
		}

		if (!view_active_) {
			LOG_INFO("The message(type:%s) sequence(" FMT_I64 ") would not be processed as view is not active, desc(%s)", PbftDesc::GetMessageTypeDesc(pbft.type()),
				sequence, PbftDesc::GetPbft(pbft).c_str());
			return NULL;
		}

		//get last sequence, and insert the middle instance to map
		int64_t last_seq = sequence - 1;
		PbftInstanceMap::const_reverse_iterator iter = instances_.rbegin();
		if (iter != instances_.rend()) {
			last_seq = iter->first.sequence_;
		}

		for (int64_t tmp_seq = last_seq + 1; tmp_seq <= sequence && last_exe_seq_ < tmp_seq; tmp_seq++) {
			PbftInstanceIndex index(view_number, tmp_seq);
			LOG_INFO("Create pbft instance vn(" FMT_I64 "), seq(" FMT_I64 ")", view_number, tmp_seq);
			instances_.insert(std::make_pair(index, PbftInstance()));
		}

		PbftInstanceIndex index(view_number, sequence);
		PbftInstanceMap::iterator iter_find = instances_.find(index);
		if (iter_find == instances_.end()) {
			LOG_INFO("Pbft instance vn(" FMT_I64 "), seq(" FMT_I64 ") have passed", view_number, sequence);
			return NULL;
		}

		PbftInstance &instace = instances_[index];
		instace.msg_buf_[pbft.type()].push_back(env);

		ValueSaver saver;
		SaveInstance(saver);
		saver.Commit();

		return &instace;
	}

	bool Pbft::OnRecv(const ConsensusMsg &message) {
		if (message.GetType() != name_) {
			LOG_ERROR("The received consensus message may be error, the type is not pbft");
			return false;
		}
		protocol::PbftEnv env = message.GetPbft();
		const protocol::Pbft &pbft = env.pbft();
		const protocol::Signature &sig = env.signature();

		//check the message item 
		if (!CheckMessageItem(env)) {
			return false;
		}

		utils::MutexGuard lock_guad(lock_);

		bool doret = false;
		switch (pbft.type()) {
		case protocol::PBFT_TYPE_PREPREPARE:
		case protocol::PBFT_TYPE_PREPARE:
		case protocol::PBFT_TYPE_COMMIT:{
			//the current view must active
			PbftInstance *pinstance = CreateInstanceIfNotExist(env);
			if (pinstance) doret = pinstance->Go(env, this);
			break;
		}
		case protocol::PBFT_TYPE_CHECKPOINT:{
			doret = OnCheckPoint(env);
			break;
		}
		case protocol::PBFT_TYPE_VIEWCHANGE:{
			doret = OnViewChange(env);
			break;
		}
		case protocol::PBFT_TYPE_NEWVIEW:{
			doret = OnNewView(env);
			break;
		}
		default: break;
		}
		return doret;
	}

	bool Pbft::OnPrePrepare(const protocol::Pbft &pbft, PbftInstance &pinstance) {
		if (view_number_ % validators_.size() == replica_id_) {
			return false;
		}

		const protocol::PbftPrePrepare &pre_prepare = pbft.pre_prepare();
		//check the value digest
		if (pre_prepare.value_digest() != utils::Sha256::Crypto(pre_prepare.value().SerializeAsString())) {
			LOG_ERROR("Check the value digest(%s) not equal(%s)'s digest, desc(%s)",
				utils::String::BinToHexString(pre_prepare.value_digest()).c_str(),
				GetValueString(pre_prepare.value()).c_str(), PbftDesc::GetPbft(pbft).c_str());
			return false;
		}

		//check the value
		int32_t cret = CheckValue(pre_prepare.block_seq(), pre_prepare.value());
		if (cret == Consensus::CHECK_VALUE_INVALID) {
			LOG_ERROR("Check the value(%s) failed, desc(%s)", GetValueString(pre_prepare.value()).c_str(), PbftDesc::GetPbft(pbft).c_str());
			return false;
		}
		if (pinstance.phase_ != PBFT_PHASE_NONE) {
			bool ret = false;
			do {
				if (CompareValue(pinstance.pre_prepare_.value(), pre_prepare.value()) != 0) {
					LOG_ERROR("The message value(%s) != this value(%s) , desc(%s)",
						GetValueString(pre_prepare.value()).c_str(), GetValueString(pinstance.pre_prepare_.value()).c_str(), PbftDesc::GetPbft(pbft).c_str());
					break;
				}

				LOG_INFO("The message value(%s) receive duplicated, desc(%s)",
					GetValueString(pre_prepare.value()).c_str(), PbftDesc::GetPbft(pbft).c_str());

				if (cret != Consensus::CHECK_VALUE_VALID) {
					LOG_INFO("Dont send prepare message, view number(" FMT_I64 "),sequence(" FMT_I64 "), round number(1)", pre_prepare.view_number(), pre_prepare.sequence());
					ret = true;
					break;
				}

				LOG_INFO("Send prepare message again, view number(" FMT_I64 "),sequence(" FMT_I64 ") round number(%u)",
					pre_prepare.view_number(), pre_prepare.sequence(), pbft.round_number());
				PeerMessagePointer prepare_msg = NewPrepare(pre_prepare, pbft.round_number());
				if (!SendMessage(prepare_msg)) {
					break;
				}
				ret = true;
			} while (false);

			return ret;
		}
		LOG_INFO("Receive pre-prepare message from replica id(" FMT_I64 "), view number(" FMT_I64 "),sequence(" FMT_I64 "), round number(%u), block(" FMT_I64 ")",
			pre_prepare.replica_id(), pre_prepare.view_number(), pre_prepare.sequence(), pbft.round_number(), pre_prepare.block_seq());

		//insert the instance to map
		pinstance.phase_ = PBFT_PHASE_PREPREPARED;
		pinstance.phase_item_ = 0;
		pinstance.pre_prepare_ = pre_prepare;
		pinstance.check_value_result_ = cret;

		ValueSaver saver;
		SaveInstance(saver);
		saver.Commit();

		if (pinstance.check_value_result_ != Consensus::CHECK_VALUE_VALID) {
			LOG_INFO("Dont send prepare message, view number(" FMT_I64 "),sequence(" FMT_I64 "), round number(1), block(" FMT_I64 ")",
				pre_prepare.view_number(), pre_prepare.sequence(), pre_prepare.block_seq());
			return true;
		}

		LOG_INFO("Send prepare message, view number(" FMT_I64 "),sequence(" FMT_I64 "), round number(1), block(" FMT_I64 ")",
			pre_prepare.view_number(), pre_prepare.sequence(), pre_prepare.block_seq());
		//NewPrepare();
		PeerMessagePointer prepare_msg = NewPrepare(pre_prepare, 1);
		if (!SendMessage(prepare_msg)) {
			return false;
		}

		//start timer
		return true;
	}

	bool Pbft::OnPrepare(const protocol::Pbft &pbft, PbftInstance &pinstance) {
		LOG_INFO("Receive prepare");
		const protocol::PbftPrepare &prepare = pbft.prepare();
		if (CompareValue(pinstance.pre_prepare_.value_digest(), prepare.value_digest()) != 0) {
			LOG_ERROR("The message prepare value(%s) != this pre-prepare value(%s) , desc(%s)",
				utils::String::Bin4ToHexString(prepare.value_digest()).c_str(),
				utils::String::Bin4ToHexString(pinstance.pre_prepare_.value_digest()).c_str(), PbftDesc::GetPbft(pbft).c_str());
			return false;
		}

		LOG_INFO("Receive prepare message from replica id(" FMT_I64 "), view number(" FMT_I64 "),sequence(" FMT_I64 "), round number(%u)",
			prepare.replica_id(), prepare.view_number(), prepare.sequence(), pbft.round_number());

		bool exist = false;
		PbftPrepareMap::iterator iter_prepare = pinstance.prepares_.find(prepare.replica_id());
		if (iter_prepare != pinstance.prepares_.end()) {
			LOG_INFO("The prepare message view number(" FMT_I64 ") sequence(" FMT_I64 ") round number(%u) has been receive duplicated, desc(%s)",
				prepare.view_number(), prepare.sequence(), pbft.round_number(), PbftDesc::GetPbft(pbft).c_str());
			exist = true;
		}

		pinstance.prepares_.insert(std::make_pair(prepare.replica_id(), prepare));
		if (pinstance.prepares_.size() >= GetQuorumSize()) {

			if (pinstance.phase_ < PBFT_PHASE_PREPARED) {  //detect the receive again
				pinstance.phase_ = PBFT_PHASE_PREPARED;
				pinstance.phase_item_ = 0;
			}

			ValueSaver saver;
			SaveInstance(saver);
			saver.Commit();

			//send commit
			if (pinstance.check_value_result_ == Consensus::CHECK_VALUE_VALID) {
				LOG_INFO("Send commit message%s, view number(" FMT_I64 "),sequence(" FMT_I64 "), round number(%u)", exist ? " again" : "",
					pinstance.pre_prepare_.view_number(), pinstance.pre_prepare_.sequence(), pbft.round_number());
				PeerMessagePointer commit_msg = NewCommit(prepare, pbft.round_number());
				pinstance.SetLastCommitSendTime(utils::Timestamp::HighResolution());
				return SendMessage(commit_msg);
			}
			else {
				LOG_INFO("Dont send commit message%s, view number(" FMT_I64 "),sequence(" FMT_I64 "), round number(%u)", exist ? " again" : "",
					pinstance.pre_prepare_.view_number(), pinstance.pre_prepare_.sequence(), pbft.round_number());
			}
		}

		return true;
	}

	bool Pbft::OnCommit(const protocol::Pbft &pbft, PbftInstance &pinstance) {
		const protocol::PbftCommit &commit = pbft.commit();
		if (CompareValue(pinstance.pre_prepare_.value_digest(), commit.value_digest()) != 0) {
			LOG_ERROR("The message commit value(%s) != this pre-prepare value(%s) , desc(%s)",
				utils::String::Bin4ToHexString(commit.value_digest()).c_str(), GetValueString(pinstance.pre_prepare_.value()).c_str(), PbftDesc::GetPbft(pbft).c_str());
			return false;
		}

		PbftCommitMap::iterator iter_commit = pinstance.commits_.find(commit.replica_id());
		if (iter_commit != pinstance.commits_.end()) {
			LOG_INFO("The prepare message view number(" FMT_I64 ") sequence(" FMT_I64 ") has been receive duplicated",
				commit.view_number(), commit.sequence());
			return true;
		}

		LOG_INFO("Receive commit from replica(" FMT_I64 "),  view number(" FMT_I64 "),sequence(" FMT_I64 "),round number(%u)",
			commit.replica_id(), commit.view_number(), commit.sequence(), pbft.round_number());
		pinstance.commits_.insert(std::make_pair(commit.replica_id(), commit));
		if (pinstance.commits_.size() >= GetQuorumSize() + 1 && pinstance.phase_ < PBFT_PHASE_COMMITED) {
			pinstance.phase_ = PBFT_PHASE_COMMITED;
			pinstance.phase_item_ = 0;
			pinstance.end_time_ = utils::Timestamp::HighResolution();
			LOG_INFO("Request commited, view number(" FMT_I64 "),sequence(" FMT_I64 "), try to execute value", pinstance.pre_prepare_.view_number(), pinstance.pre_prepare_.sequence());

			ValueSaver saver;
			if (sequence_ < commit.sequence() + 1) {
				saver.SaveValue(PbftDesc::SEQUENCE_NAME, commit.sequence() + 1);
				sequence_ = commit.sequence() + 1;
			}

			SaveInstance(saver);
			// this consensus has achieve
			return TryExecuteValue();
		}

		return true;
	}

	bool Pbft::OnCheckPoint(const protocol::PbftEnv &pbft_env) {
		const protocol::Pbft &pbft = pbft_env.pbft();
		const protocol::PbftCheckPoint &checkpoint = pbft.checkpoint();

		LOG_INFO("Receive check point");
		if (!InWaterMark(checkpoint.sequence())) {
			LOG_TRACE("The message(type:%s) sequence(" FMT_I64 ") is not in water mark(" FMT_I64 ", " FMT_I64"), desc(%s)", PbftDesc::GetMessageTypeDesc(pbft.type()),
				checkpoint.sequence(), low_water_mark_, low_water_mark_ + ckp_interval_, PbftDesc::GetPbft(pbft).c_str());
			return NULL;
		}

		LOG_INFO("Receive check point message from replica(" FMT_I64 "), sequence(" FMT_I64 "), state digest(%s)",
			checkpoint.replica_id(), checkpoint.sequence(), utils::String::Bin4ToHexString(checkpoint.state_digest()).c_str());

		PbftCkpInstanceIndex index(checkpoint.sequence(), checkpoint.state_digest());
		PbftCkpInstanceMap::iterator iter = ckp_instances_.find(index);
		if (iter == ckp_instances_.end()) {
			ckp_instances_.insert(std::make_pair(index, PbftCkpInstance()));
		}

		PbftCkpInstance &ckp_instance = ckp_instances_[index];
		ckp_instance.msg_buf_.push_back(pbft_env);
		ckp_instance.checkpoints_.insert(std::make_pair(checkpoint.replica_id(), checkpoint));

		do {
			ValueSaver saver;
			SaveCheckPoint(saver);
		} while (false);

		if (ckp_instance.checkpoints_.size() >= GetQuorumSize() + 1) {
			//check point have achieve
			ckp_instance.stable_ = true;

			LOG_INFO("Try to move water mark, sequence(" FMT_I64 "), state digest(%s)",
				checkpoint.sequence(), utils::String::Bin4ToHexString(checkpoint.state_digest()).c_str());
			TryMoveWaterMark();
		}

		return true;
	}

	bool Pbft::CheckViewChange(const protocol::PbftViewChange &view_change) {
		//check the checkpoint message
		std::set<int64_t> replica_ids;
		std::string state_digest = "";
		for (int32_t i = 0; i < view_change.checkpoints_size(); i++) {
			const protocol::PbftEnv &env = view_change.checkpoints(i);
			if (!CheckMessageItem(env)) {
				continue;
			}
			const protocol::PbftCheckPoint &check_point = env.pbft().checkpoint();
			if (check_point.sequence() != view_change.sequence()) {
				continue;
			}

			if (state_digest.empty()) {
				state_digest = check_point.state_digest();
			}

			if (state_digest != check_point.state_digest()) {
				continue;
			}
			replica_ids.insert(check_point.replica_id());
		}

		if (view_change.checkpoints_size() > 0 &&
			view_change.sequence() > 0 &&
			replica_ids.size() < GetQuorumSize()) { // for debug
			LOG_ERROR("The view-change message(sequence:" FMT_I64 ")'s  checkpoint message's replica count(" FMT_SIZE ") is less than quorom size(" FMT_SIZE")",
				view_change.sequence(), replica_ids.size(), GetQuorumSize() + 1);
			return false;
		}

		bool error_ret = false;
		//check the prepared message
		for (int32_t i = 0; i < view_change.prepared_set_size(); i++) {
			const protocol::PbftPreparedSet &prepared_set = view_change.prepared_set(i);
			//check the pre-prepare message
			const protocol::PbftEnv &pre_prepare_env = prepared_set.pre_prepare();
			const protocol::PbftPrePrepare &pre_prepare = pre_prepare_env.pbft().pre_prepare();
			if (!CheckMessageItem(pre_prepare_env)) {
				break;
			}

			std::set<int64_t> replica_ids;
			//check the prepare message
			for (int32_t m = 0; m < prepared_set.prepare_size(); m++) {
				const protocol::PbftEnv &prepare_env = prepared_set.prepare(m);
				if (!CheckMessageItem(prepare_env)) {
					error_ret = true;
					break;
				}

				const protocol::PbftPrepare &prepare = prepare_env.pbft().prepare();

				if (prepare.view_number() != pre_prepare.view_number() ||
					prepare.sequence() != pre_prepare.sequence() ||
					CompareValue(prepare.value_digest(), pre_prepare.value_digest()) != 0) {
					error_ret = true;
					break;
				}

				replica_ids.insert(prepare.replica_id());
			}

			if (replica_ids.size() < GetQuorumSize() + 1) {
				LOG_ERROR("The view-change message's sequence(" FMT_I64 ") prepared message's replica number(" FMT_SIZE ") is less than quorom size(" FMT_SIZE")",
					view_change.sequence(), replica_ids.size(), GetQuorumSize() + 1);
				error_ret = false;
			}

			if (error_ret) break;
		}

		return !error_ret;
	}

	bool Pbft::OnViewChange(const protocol::PbftEnv &pbft_env) {
		const protocol::Pbft &pbft = pbft_env.pbft();
		const protocol::PbftViewChange &view_change = pbft.view_change();

		LOG_INFO("Receive view change message from replica id(" FMT_I64 "), view number(" FMT_I64 "),sequence(" FMT_I64 "), round number(%u)",
			view_change.replica_id(), view_change.view_number(), view_change.sequence(), pbft.round_number());
		//check the view change item
		if (!CheckViewChange(view_change)) {
			LOG_ERROR("Check view change message failed");
			return false;
		}

		PbftVcInstanceMap::iterator iter = vc_instances_.find(view_change.view_number());
		if (iter == vc_instances_.end()) {
			vc_instances_.insert(std::make_pair(view_change.view_number(), PbftVcInstance(view_change.view_number())));
		}

		PbftVcInstance &vc_instance = vc_instances_[view_change.view_number()];

		//insert into the msg need to be sent again for timeout
		if (view_change.replica_id() == replica_id_ && !vc_instance.view_change_msg_.IsInitialized()) {
			PeerMessagePointer msg = NewViewChange(view_number_ + 1);
			*((protocol::PbftEnv *)msg->data_) = pbft_env;
			vc_instance.view_change_msg_ = pbft_env;
		}

		vc_instance.msg_buf_.push_back(pbft_env);
		vc_instance.viewchanges_.insert(std::make_pair(view_change.replica_id(), view_change));
		if (vc_instance.viewchanges_.size() > GetQuorumSize() && vc_instance.end_time_ == 0) { //for view change, quorum size is 2f
			//view change have achieve
			return ProcessQuorumViewChange(vc_instance);
		}

		return true;
	}

	bool Pbft::OnNewView(const protocol::PbftEnv &pbft_env) {
		const protocol::Pbft &pbft = pbft_env.pbft();
		const protocol::PbftNewView &new_view = pbft.new_view();

		LOG_INFO("Receive new view message from replica id(" FMT_I64 "), view number(" FMT_I64 "),round number(%u)",
			new_view.replica_id(), new_view.view_number(), pbft.round_number());
		if (new_view.view_number() == view_number_) {
			LOG_INFO("The new view number(" FMT_I64 ") is equal than current view number, do nothing", new_view.view_number());
			return true;
		}
		else if (new_view.view_number() < view_number_) {
			LOG_INFO("The new view number(" FMT_I64 ") is less than current view number(" FMT_I64 "), do nothing",
				new_view.view_number(), view_number_);
			return true;
		}

		//delete the response timer
		bool ret = utils::Timer::Instance().DelTimer(new_view_repond_timer_);
		LOG_INFO("Try to delete new view repond timer id(" FMT_I64 ") %s", new_view_repond_timer_, ret ? "true" : "failed");

		if (new_view.view_number() % validators_.size() == replica_id_) {
			LOG_INFO("It's the new primary(replica_id:" FMT_I64 "), so donn't process the new view message", replica_id_);
			return true;
		}

		//check the view change message
		PbftVcInstance vc_instance_tmp;
		vc_instance_tmp.view_number_ = new_view.view_number();

		bool check_ret = true;
		std::set<int64_t> replica_set;
		for (int32_t i = 0; i < new_view.view_changes_size(); i++) {
			const protocol::PbftEnv &view_change_env = new_view.view_changes(i);
			vc_instance_tmp.msg_buf_.push_back(view_change_env);
			if (!CheckMessageItem(pbft_env)) {
				check_ret = false;
				break;
			}

			const protocol::PbftViewChange &view_change = view_change_env.pbft().view_change();
			if (!CheckViewChange(view_change)) {
				LOG_ERROR("The new view message's view-number(" FMT_I64 ") check it's view-change message failed",
					new_view.view_number());
				check_ret = false;
				break;
			}
			vc_instance_tmp.viewchanges_.insert(std::make_pair(view_change.replica_id(), view_change));

			if (new_view.view_number() != view_change.view_number()) {
				LOG_ERROR("The new view message's view-number(" FMT_I64 ") is not equal with it's view-change number(" FMT_I64 ")",
					new_view.view_number(), view_change.view_number());
				check_ret = false;
				break;
			}

			replica_set.insert(view_change.replica_id());
		}

		if (!check_ret) {
			return false;
		}

		if (replica_set.size() <= GetQuorumSize()) {
			LOG_ERROR("The new view message(number:" FMT_I64 ")'s count(" FMT_SIZE ") is less or equal than qurom size(" FMT_SIZE ")",
				new_view.view_number(), replica_set.size(), GetQuorumSize());
			return false;
		}

		//check the pre-prepare message set
		std::map<int64_t, protocol::PbftEnv> pre_prepares;
		PbftCkpInstancePair ckp_pair;
		if (!CreateViewChangeParam(vc_instance_tmp, pre_prepares, ckp_pair)) {
			LOG_ERROR("The new view message(number:" FMT_I64 ")', create pre-prepare message failed",
				new_view.view_number());
			return false;
		}

		//compare the computed pre-prepare messages with the appended pre-prepare messages
		bool compare_ret = true;
		for (int32_t i = 0; i < new_view.pre_prepares_size(); i++) {
			const protocol::PbftEnv &pre_prepare_env = new_view.pre_prepares(i);
			if (!CheckMessageItem(pre_prepare_env)) {
				LOG_ERROR("The new view message(number:" FMT_I64 ")', check the pre-prepare message failed",
					new_view.view_number());
				compare_ret = false;
				break;
			}

			const protocol::PbftPrePrepare &pre_prepare = pre_prepare_env.pbft().pre_prepare();

			std::map<int64_t, protocol::PbftEnv>::iterator iter = pre_prepares.find(pre_prepare.replica_id());
			if (iter == pre_prepares.end()) {
				LOG_ERROR("The new view message(number:" FMT_I64 ")', check the computed pre-prepare message failed",
					new_view.view_number());
				compare_ret = false;
				break;
			}

			const protocol::PbftEnv &pre_prepare_env_comp = iter->second;
			const protocol::PbftPrePrepare &pre_prepare_comp = pre_prepare_env_comp.pbft().pre_prepare();
			if (pre_prepare_comp.sequence() != pre_prepare.sequence() ||
				pre_prepare_comp.view_number() != pre_prepare.view_number() ||
				pre_prepare_comp.replica_id() != pre_prepare.replica_id() ||
				CompareValue(pre_prepare_comp.value(), pre_prepare.value()) != 0) {
				LOG_ERROR("The new view message(number:" FMT_I64 ")', check the computed pre-prepare message failed",
					new_view.view_number());
				compare_ret = false;
				break;
			}
		}

		//delete the other log
		for (PbftInstanceMap::iterator iter_inst = instances_.begin();
			iter_inst != instances_.end();
			) {
			if (iter_inst->second.phase_ < PBFT_PHASE_PREPARED) {
				instances_.erase(iter_inst++);
			}
			else {
				iter_inst++;
			}
		}

		//insert the lastest stable checkpoint into this log
		ckp_instances_.insert(ckp_pair);

		//get max sequence
		int64_t max_seq = last_exe_seq_;
		for (PbftInstanceMap::iterator iter_inst = instances_.begin();
			iter_inst != instances_.end();
			iter_inst++
			) {
			if (iter_inst->first.sequence_ > max_seq) {
				max_seq = iter_inst->first.sequence_;
			}
		}

		ValueSaver saver;
		//save the sequence
		if (max_seq > 0) {
			sequence_ = max_seq + 1;
			saver.SaveValue(PbftDesc::SEQUENCE_NAME, sequence_);
		}

		//try to move new watermark
		TryMoveWaterMark();

		LOG_INFO("Others enter the new view(number:" FMT_I64 ")", new_view.view_number());
		//enter to new view
		view_number_ = new_view.view_number();
		view_active_ = true;
		saver.SaveValue(PbftDesc::VIEWNUMBER_NAME, view_number_);

		PbftVcInstanceMap::iterator iter = vc_instances_.find(view_number_);
		if (iter != vc_instances_.end()) {
			iter->second.ChangeComplete(utils::Timestamp::HighResolution());
		}

		//delete other not ended view change instance
		for (PbftVcInstanceMap::iterator iter_vc = vc_instances_.begin(); iter_vc != vc_instances_.end();) {
			if (iter_vc->second.end_time_ == 0) {
				LOG_INFO("Delete the view change instance (vn:" FMT_I64 ") that is not complete", iter_vc->second.view_number_);
				vc_instances_.erase(iter_vc++);
			}
			else if (iter_vc->second.view_number_ < view_number_ - 5) {
				LOG_INFO("Delete the view change instance (vn:" FMT_I64 ") that has passed by 5", iter_vc->second.view_number_);
				vc_instances_.erase(iter_vc++);
			}
			else {
				iter_vc++;
			}
		}
		saver.Commit();

		OnViewChanged();

		//send the prepare message to others
		for (int32_t i = 0; i < new_view.pre_prepares_size(); i++) {
			const protocol::PbftEnv &pre_prepare_env = new_view.pre_prepares(i);
			const protocol::PbftPrePrepare &pre_prepare = pre_prepare_env.pbft().pre_prepare();

			//delete the original object with the sequence
			for (PbftInstanceMap::iterator iter_inst = instances_.begin();
				iter_inst != instances_.end();
				iter_inst++) {
				if (iter_inst->first.sequence_ == pre_prepare.sequence()) {
					instances_.erase(iter_inst);
					break;
				}
			}

			//add new
			PbftInstance *pinstance = CreateInstanceIfNotExist(pre_prepare_env);
			if (pinstance) pinstance->Go(pre_prepare_env, this);
		}

		return true;
	}

	bool Pbft::CreateViewChangeParam(const PbftVcInstance &vc_instance,
		std::map<int64_t, protocol::PbftEnv> &pre_prepares,
		PbftCkpInstancePair &lasted_ckp_pair) {
		//get min sequence and max sequence
		int64_t min_seq = 0, max_seq = 0;
		std::map<int64_t, protocol::PbftPrePrepare> pre_prepares_tmp;
		protocol::PbftViewChange lastest_ckp_viewchange;
		for (PbftViewChangeMap::const_iterator iter = vc_instance.viewchanges_.begin();
			iter != vc_instance.viewchanges_.end();
			iter++) {
			const protocol::PbftViewChange &view_change = iter->second;
			if (view_change.sequence() > min_seq) {
				min_seq = view_change.sequence();
				lastest_ckp_viewchange = view_change;
			}
		}

		max_seq = min_seq;
		for (PbftViewChangeMap::const_iterator iter = vc_instance.viewchanges_.begin();
			iter != vc_instance.viewchanges_.end();
			iter++) {
			const protocol::PbftViewChange &view_change = iter->second;
			for (int32_t i = 0; i < view_change.prepared_set_size(); i++) {
				const protocol::PbftPreparedSet &set = view_change.prepared_set(i);
				const protocol::PbftEnv &pre_prepare_env = set.pre_prepare();
				const protocol::PbftPrePrepare &pre_prepare = pre_prepare_env.pbft().pre_prepare();
				if (pre_prepare.sequence() > max_seq) {
					max_seq = pre_prepare.sequence();
				}
			}
		}

		LOG_INFO("Create view change param, min seq(" FMT_I64 "), max seq(" FMT_I64 ")", min_seq, max_seq);

		//get the lastest stable checkpoint
		std::string state_digest;
		for (int32_t i = 0; i < lastest_ckp_viewchange.checkpoints_size(); i++) {
			const protocol::PbftEnv &env = lastest_ckp_viewchange.checkpoints(i);
			const protocol::Pbft &pbft = env.pbft();
			const protocol::PbftCheckPoint &checkpoint = pbft.checkpoint();
			if (state_digest.empty())state_digest = checkpoint.state_digest();
			lasted_ckp_pair.second.checkpoints_.insert(std::make_pair(checkpoint.replica_id(), pbft.checkpoint()));
			lasted_ckp_pair.second.msg_buf_.push_back(env);
		}
		lasted_ckp_pair.second.stable_ = true;
		lasted_ckp_pair.first.sequence_ = lastest_ckp_viewchange.sequence();
		lasted_ckp_pair.first.state_digest_ = state_digest;

		//create the pre-prepare message
		for (PbftViewChangeMap::const_iterator iter = vc_instance.viewchanges_.begin();
			iter != vc_instance.viewchanges_.end();
			iter++) {
			const protocol::PbftViewChange &view_change = iter->second;
			for (int32_t i = 0; i < view_change.prepared_set_size(); i++) {
				const protocol::PbftPreparedSet &set = view_change.prepared_set(i);
				const protocol::PbftEnv &pre_prepare_env = set.pre_prepare();
				const protocol::PbftPrePrepare &pre_prepare = pre_prepare_env.pbft().pre_prepare();
				if (pre_prepare.sequence() <= max_seq &&
					pre_prepare.sequence() > min_seq &&
					pre_prepares_tmp.find(pre_prepare.sequence()) == pre_prepares_tmp.end()) {
					pre_prepares_tmp.insert(std::make_pair(pre_prepare.sequence(), pre_prepare));
					LOG_INFO("Create view change param, insert preprepare, seq(" FMT_I64 ")", pre_prepare.sequence());
				}
			}
		}

		//create the null pre-prepare message
		for (std::map<int64_t, protocol::PbftPrePrepare>::iterator iter = pre_prepares_tmp.begin();
			iter != pre_prepares_tmp.end();
			iter++) {
			protocol::PbftPrePrepare tmp_preprepare = iter->second;
			tmp_preprepare.set_view_number(vc_instance.view_number_);
			tmp_preprepare.set_replica_id(replica_id_);
			pre_prepares.insert(std::make_pair(iter->first, NewPrePrepare(tmp_preprepare)));
			if (iter->first > min_seq + 1) {
				//we should set the property close time
				const protocol::Value &the_value = tmp_preprepare.value();
				int64_t the_time = the_value.close_time();
				//insert null 
				for (int64_t m = min_seq + 1; m < iter->first; m++) {
					protocol::PbftPrePrepare null_preprepare;
					null_preprepare.set_view_number(vc_instance.view_number_);
					null_preprepare.set_sequence(m);
					null_preprepare.set_replica_id(replica_id_);
					null_preprepare.set_block_seq(0);
					std::string null_hash = notify_->FetchNullMsg();
					protocol::Value *value = null_preprepare.mutable_value();
					value->set_hash_set(null_hash);
					value->set_close_time(the_time - (iter->first - (min_seq + 1)));
					null_preprepare.set_value_digest(utils::Sha256::Crypto(value->SerializeAsString()));
					pre_prepares.insert(std::make_pair(m, NewPrePrepare(null_preprepare)));

					//GlueManager::Instance().RecvConsensusTxSet(GetNullTxset());
					LOG_INFO("Create null preprepare(hash:%s) message for new view message, seq(" FMT_I64 "), vn(" FMT_I64 ")",
						utils::String::Bin4ToHexString(null_hash).c_str(),
						m, vc_instance.view_number_);
				}
			}

			min_seq = iter->first;
		}

		return true;
	}

	bool Pbft::ProcessQuorumViewChange(PbftVcInstance &vc_instance) {
		LOG_INFO("Process quorum view change, new view (number:" FMT_I64 ")", vc_instance.view_number_);
		if (vc_instance.view_number_ % validators_.size() != replica_id_) { // we must be the leader

			new_view_repond_timer_ = utils::Timer::Instance().AddTimer(30 * utils::MICRO_UNITS_PER_SEC, vc_instance.view_number_, [this](int64_t data) {
				if (view_active_) {
					LOG_INFO("The current view(" FMT_I64 ") is active, so do not send new view(vn:" FMT_I64 ") ", view_number_
						, data + 1);
				}
				else {
					LOG_INFO("The new view(vn: " FMT_I64 ")'s primary does not respond,  negotiate next view(vn: " FMT_I64 ")", data
						, data + 1);

					//SEND NEW VIEW
					LOG_INFO("Send view change message, new view number(" FMT_I64 ")", data + 1);
					PeerMessagePointer msg = NewViewChange(data + 1);
					SendMessage(msg);
				}
			});

			LOG_INFO("It's not the new primary(replica_id:" FMT_I64 "), so donn't process the quorum view message, waiting new view message 30s, timer id(" FMT_I64")",
				vc_instance.view_number_ % validators_.size(), new_view_repond_timer_);
			return false;
		}

		//try to enter to new view
		//first get the lastest stable checkpoint
		PbftCkpInstancePair ckp_pair;
		std::map<int64_t, protocol::PbftEnv> pre_prepares;
		if (!CreateViewChangeParam(vc_instance, pre_prepares, ckp_pair)) {
			LOG_ERROR("Create view change(vn:" FMT_I64 ") parameter failed", vc_instance.view_number_);
			return false;
		}

		//new view message
		PeerMessagePointer msg = NewNewView(vc_instance, pre_prepares, ckp_pair.first.sequence_);

		LOG_INFO("Send new view message, new view number(" FMT_I64 ")", vc_instance.view_number_);
		//send new view message
		vc_instance.SendNewView(this, utils::Timestamp::HighResolution(), msg);

		//insert the lastest stable checkpoint into this log
		ckp_instances_.insert(ckp_pair);

		//disacard the other log
		for (PbftInstanceMap::iterator iter_inst = instances_.begin();
			iter_inst != instances_.end();
			) {
			if (iter_inst->second.phase_ < PBFT_PHASE_PREPARED) {
				instances_.erase(iter_inst++);
			}
			else {
				iter_inst++;
			}
		}

		//new instance
		for (std::map<int64_t, protocol::PbftEnv>::iterator iter_pre = pre_prepares.begin();
			iter_pre != pre_prepares.end();
			iter_pre++) {

			const protocol::PbftEnv &pre_prepare_env = iter_pre->second;
			const protocol::PbftPrePrepare &pre_prepare = pre_prepare_env.pbft().pre_prepare();
			PbftInstanceIndex index(pre_prepare.view_number(), pre_prepare.sequence());

			if (pre_prepare.sequence() <= last_exe_seq_) {
				continue;
			}

			//delete the original object with the sequence
			for (PbftInstanceMap::iterator iter_inst = instances_.begin();
				iter_inst != instances_.end();
				iter_inst++) {
				if (iter_inst->first.sequence_ == pre_prepare.sequence()) {
					instances_.erase(iter_inst);
					break;
				}
			}

			//add new
			PbftInstance pinstance;
			pinstance.check_value_result_ = CheckValue(pinstance.pre_prepare_.block_seq(), pinstance.pre_prepare_.value());
			pinstance.pre_prepare_msg_ = pre_prepare_env;
			pinstance.phase_ = PBFT_PHASE_PREPREPARED;
			pinstance.pre_prepare_ = pre_prepare;
			pinstance.msg_buf_[pre_prepare_env.pbft().type()].push_back(pre_prepare_env);
			instances_.insert(std::make_pair(index, pinstance));
		}

		ValueSaver saver;
		SaveInstance(saver);

		//get max sequence
		int64_t max_seq = last_exe_seq_;
		for (PbftInstanceMap::iterator iter_inst = instances_.begin();
			iter_inst != instances_.end();
			iter_inst++
			) {
			if (iter_inst->first.sequence_ > max_seq) {
				max_seq = iter_inst->first.sequence_;
			}
		}

		if (max_seq > 0) {
			//save the sequence
			sequence_ = max_seq + 1;
			saver.SaveValue(PbftDesc::SEQUENCE_NAME, sequence_);
		}
		saver.Commit();

		//try to move new watermark
		TryMoveWaterMark();

		//enter to new view
		view_number_ = vc_instance.view_number_;
		view_active_ = true;

		LOG_INFO("Primary enter the new view(number:" FMT_I64 ")", view_number_);
		vc_instance.ChangeComplete(utils::Timestamp::HighResolution());
		saver.SaveValue(PbftDesc::VIEWNUMBER_NAME, view_number_);
		saver.Commit();

		//delete other not ended view change instance or other view change instance which sequence is less than 5
		for (PbftVcInstanceMap::iterator iter_vc = vc_instances_.begin(); iter_vc != vc_instances_.end();) {
			if (iter_vc->second.end_time_ == 0) {
				LOG_INFO("Delete the view change instance (vn:" FMT_I64 ") that is not complete", iter_vc->second.view_number_);
				vc_instances_.erase(iter_vc++);
			}
			else if (iter_vc->second.view_number_ < view_number_ - 5) {
				LOG_INFO("Delete the view change instance (vn:" FMT_I64 ") that has passed by 5", iter_vc->second.view_number_);
				vc_instances_.erase(iter_vc++);
			}
			else {
				iter_vc++;
			}
		}

		OnViewChanged();

		return true;
	}

	bool Pbft::TryMoveWaterMark() {
		//get lastest stable checkpoint and
		int64_t move_to_seq = -1;
		for (PbftCkpInstanceMap::iterator iter = ckp_instances_.begin(); iter != ckp_instances_.end(); iter++) {
			PbftCkpInstance &ckp_instance = iter->second;
			const PbftCkpInstanceIndex &ckp_index = iter->first;
			if (ckp_instance.stable_ && ckp_index.sequence_ <= last_exe_seq_) {
				move_to_seq = ckp_index.sequence_ / ckp_count_ * ckp_count_;
			}
		}

		if (move_to_seq < 0) {
			return false;
		}

		//delete checkpoint
		for (PbftCkpInstanceMap::iterator iter = ckp_instances_.begin(); iter != ckp_instances_.end();) {
			const PbftCkpInstanceIndex &ckp_index = iter->first;
			if (ckp_index.sequence_ < move_to_seq) { //keep the last stable checkpoint
				ckp_instances_.erase(iter++);
			}
			else {
				iter++;
			}
		}

		ValueSaver saver;
		SaveCheckPoint(saver);

		//delete pbft instance
		for (PbftInstanceMap::iterator iter = instances_.begin(); iter != instances_.end();) {
			const PbftInstanceIndex &index = iter->first;
			if (index.sequence_ <= move_to_seq) {
				instances_.erase(iter++);
			}
			else {
				iter++;
			}
		}

		SaveInstance(saver);

		LOG_INFO("The prepare message view number(" FMT_I64 ") move " FMT_I64 " to new water mark " FMT_I64, view_number_, low_water_mark_, move_to_seq);
		low_water_mark_ = move_to_seq;

		saver.SaveValue(PbftDesc::LOW_WATER_MRAK_NAME, low_water_mark_);
		saver.Commit();

		return true;
	}

	bool Pbft::TryExecuteValue() {
		for (PbftInstanceMap::iterator iter = instances_.begin(); iter != instances_.end(); iter++) {
			PbftInstance &instance = iter->second;
			const PbftInstanceIndex &index = iter->first;
			if (index.sequence_ <= last_exe_seq_) {
				continue;
			}

			if (index.sequence_ == last_exe_seq_ + 1 && instance.phase_ >= PBFT_PHASE_COMMITED) {
				last_exe_seq_++;
			}
			else {
				break;
			}

			std::string state_digest = OnValueCommited(instance.pre_prepare_.block_seq(), index.sequence_, instance.pre_prepare_.value(), true);

			ValueSaver saver;
			saver.SaveValue(PbftDesc::LAST_EXE_SEQUENCE_NAME, last_exe_seq_);
			saver.Commit();

			//call ledger manager, and return the block hash
			if (last_exe_seq_ % ckp_count_ == 0) {
				LOG_INFO("Send check point message, view number(" FMT_I64 "),sequence(" FMT_I64 "), state digest(%s)",
					view_number_, last_exe_seq_, utils::String::Bin4ToHexString(state_digest).c_str());
				//check point
				PeerMessagePointer msg = NewCheckPoint(state_digest, last_exe_seq_);
				SendMessage(msg);
			}
		}
		return true;
	}

	PeerMessagePointer Pbft::NewPrePrepare(const protocol::Value &value, int64_t block_seq) {
		PeerMessagePointer message = PeerMessage::NewPbft();
		protocol::PbftEnv *env = (protocol::PbftEnv *)message->data_;

		protocol::Pbft *pbft = env->mutable_pbft();
		pbft->set_round_number(1);
		pbft->set_validator_hash(validators_hash_);
		pbft->set_type(protocol::PBFT_TYPE_PREPREPARE);

		protocol::PbftPrePrepare *preprepare = pbft->mutable_pre_prepare();
		preprepare->set_view_number(view_number_);
		preprepare->set_replica_id(replica_id_);
		preprepare->set_sequence(sequence_);
		preprepare->set_block_seq(block_seq);
		*preprepare->mutable_value() = value;
		preprepare->set_value_digest(utils::Sha256::Crypto(value.SerializeAsString()));

		protocol::Signature *sig = env->mutable_signature();
		sig->set_public_key(private_key_.GetBase58PublicKey());
		sig->set_sign_data(private_key_.Sign(pbft->SerializeAsString()));
		return message;
	}

	protocol::PbftEnv Pbft::NewPrePrepare(const protocol::PbftPrePrepare &pre_prepare) {
		protocol::PbftEnv env;
		protocol::Pbft *pbft = env.mutable_pbft();
		pbft->set_round_number(1);
		pbft->set_validator_hash(validators_hash_);
		pbft->set_type(protocol::PBFT_TYPE_PREPREPARE);

		*pbft->mutable_pre_prepare() = pre_prepare;

		protocol::Signature *sig = env.mutable_signature();
		sig->set_public_key(private_key_.GetBase58PublicKey());
		sig->set_sign_data(private_key_.Sign(pbft->SerializeAsString()));
		return env;
	}

	PeerMessagePointer Pbft::NewPrepare(const protocol::PbftPrePrepare &pre_prepare, uint32_t round_number) {
		PeerMessagePointer message = PeerMessage::NewPbft();
		protocol::PbftEnv *env = (protocol::PbftEnv *)message->data_;

		protocol::Pbft *pbft = env->mutable_pbft();
		pbft->set_round_number(round_number);
		pbft->set_validator_hash(validators_hash_);
		pbft->set_type(protocol::PBFT_TYPE_PREPARE);

		protocol::PbftPrepare *prepare = pbft->mutable_prepare();
		prepare->set_view_number(pre_prepare.view_number());
		prepare->set_replica_id(replica_id_);
		prepare->set_sequence(pre_prepare.sequence());
		prepare->set_value_digest(pre_prepare.value_digest());

		protocol::Signature *sig = env->mutable_signature();
		sig->set_public_key(private_key_.GetBase58PublicKey());
		sig->set_sign_data(private_key_.Sign(pbft->SerializeAsString()));
		return message;
	}

	PeerMessagePointer Pbft::NewCommit(const protocol::PbftPrepare &prepare, uint32_t round_number) {
		PeerMessagePointer message = PeerMessage::NewPbft();
		protocol::PbftEnv *env = (protocol::PbftEnv *)message->data_;

		protocol::Pbft *pbft = env->mutable_pbft();
		pbft->set_round_number(round_number);
		pbft->set_validator_hash(validators_hash_);
		pbft->set_type(protocol::PBFT_TYPE_COMMIT);

		protocol::PbftCommit *preprepare = pbft->mutable_commit();
		preprepare->set_view_number(prepare.view_number());
		preprepare->set_replica_id(replica_id_);
		preprepare->set_sequence(prepare.sequence());
		preprepare->set_value_digest(prepare.value_digest());

		protocol::Signature *sig = env->mutable_signature();
		sig->set_public_key(private_key_.GetBase58PublicKey());
		sig->set_sign_data(private_key_.Sign(pbft->SerializeAsString()));
		return message;
	}

	PeerMessagePointer Pbft::NewViewChange(int64_t view_number) {
		PeerMessagePointer message = PeerMessage::NewPbft();
		protocol::PbftEnv *env = (protocol::PbftEnv *)message->data_;

		protocol::Pbft *pbft = env->mutable_pbft();
		pbft->set_round_number(0);
		pbft->set_validator_hash(validators_hash_);
		pbft->set_type(protocol::PBFT_TYPE_VIEWCHANGE);

		//get last stable checkpoint
		PbftCkpInstanceMap::reverse_iterator iter = ckp_instances_.rbegin();
		for (; iter != ckp_instances_.rend(); iter++) {
			PbftCkpInstance &ckp_instance = iter->second;
			if (ckp_instance.stable_) {
				break;
			}
		}

		//add checkpoint
		protocol::PbftViewChange *pviewchange = pbft->mutable_view_change();
		pviewchange->set_view_number(view_number);
		pviewchange->set_sequence(0);
		pviewchange->set_replica_id(replica_id_);
		if (iter != ckp_instances_.rend()) {
			PbftCkpInstance &ckp_instance = iter->second;
			pviewchange->set_sequence(iter->first.sequence_);
			for (PbftPhaseVector::iterator iter_vec = ckp_instance.msg_buf_.begin();
				iter_vec != ckp_instance.msg_buf_.end();
				iter_vec++) {
				*pviewchange->add_checkpoints() = *iter_vec;
			}
		}

		//add prepared msg large than lastest checkpoint's sequence
		for (PbftInstanceMap::iterator iter_instance = instances_.begin();
			iter_instance != instances_.end();
			iter_instance++) {
			const PbftInstance &instance = iter_instance->second;
			const PbftInstanceIndex &index = iter_instance->first;
			if (index.sequence_ > pviewchange->sequence() && instance.phase_ == PBFT_PHASE_PREPARED) {
				protocol::PbftPreparedSet *prepared_set = pviewchange->add_prepared_set();

				//add prepared message,add pre-prepare message
				*prepared_set->mutable_pre_prepare() = instance.msg_buf_[0][0];//add prepreare message

				//add prepare message
				for (size_t i = 0; i < instance.msg_buf_[1].size(); i++) {
					*prepared_set->add_prepare() = instance.msg_buf_[1][i];
				}
			}
		}

		protocol::Signature *sig = env->mutable_signature();
		sig->set_public_key(private_key_.GetBase58PublicKey());
		sig->set_sign_data(private_key_.Sign(pbft->SerializeAsString()));

		return message;
	}

	PeerMessagePointer Pbft::NewNewView(PbftVcInstance &vc_instance, std::map<int64_t, protocol::PbftEnv> &pre_prepares, int64_t sequence) {
		PeerMessagePointer message = PeerMessage::NewPbft();
		protocol::PbftEnv *env = (protocol::PbftEnv *)message->data_;

		protocol::Pbft *pbft = env->mutable_pbft();
		pbft->set_round_number(0);
		pbft->set_validator_hash(validators_hash_);
		pbft->set_type(protocol::PBFT_TYPE_NEWVIEW);

		protocol::PbftNewView *pnewview = pbft->mutable_new_view();
		pnewview->set_view_number(vc_instance.view_number_);
		pnewview->set_replica_id(replica_id_);
		pnewview->set_sequence(sequence);

		for (PbftPhaseVector::iterator iter = vc_instance.msg_buf_.begin(); iter != vc_instance.msg_buf_.end(); iter++) {
			*pnewview->add_view_changes() = *iter;
		}

		for (std::map<int64_t, protocol::PbftEnv>::iterator iter = pre_prepares.begin(); iter != pre_prepares.end(); iter++) {
			*pnewview->add_pre_prepares() = iter->second;
		}

		protocol::Signature *sig = env->mutable_signature();
		sig->set_public_key(private_key_.GetBase58PublicKey());
		sig->set_sign_data(private_key_.Sign(pbft->SerializeAsString()));
		return message;
	}

	PeerMessagePointer Pbft::NewCheckPoint(const std::string &state_digest, int64_t seq) {
		PeerMessagePointer message = PeerMessage::NewPbft();
		protocol::PbftEnv *env = (protocol::PbftEnv *)message->data_;

		protocol::Pbft *pbft = env->mutable_pbft();
		pbft->set_round_number(0);
		pbft->set_validator_hash(validators_hash_);
		pbft->set_type(protocol::PBFT_TYPE_CHECKPOINT);

		protocol::PbftCheckPoint *pcheckpoint = pbft->mutable_checkpoint();
		pcheckpoint->set_state_digest(state_digest);
		pcheckpoint->set_replica_id(replica_id_);
		pcheckpoint->set_sequence(seq);

		protocol::Signature *sig = env->mutable_signature();
		sig->set_public_key(private_key_.GetBase58PublicKey());
		sig->set_sign_data(private_key_.Sign(pbft->SerializeAsString()));
		return message;
	}

	PeerMessagePointer Pbft::IncPeerMessageRound(const protocol::PbftEnv &message, uint32_t round_number) {
		PeerMessagePointer new_msg = PeerMessage::NewPbft();
		*(protocol::PbftEnv *)new_msg->data_ = message;
		protocol::PbftEnv *env = (protocol::PbftEnv *)new_msg->data_;
		protocol::Pbft *pbft = env->mutable_pbft();
		pbft->set_round_number(round_number);

		protocol::Signature *sig = env->mutable_signature();
		sig->set_public_key(private_key_.GetBase58PublicKey());
		sig->set_sign_data(private_key_.Sign(pbft->SerializeAsString()));

		return new_msg;
	}

	size_t Pbft::GetQuorumSize() {
		if (validators_.size() < 4) { // for debug
			return validators_.size() - 1;
		}

		size_t qsize = validators_.size();
		if (validators_.size() == 3 * fault_number_ + 1) {
			qsize = 2 * fault_number_;
		}
		else if (validators_.size() == 3 * fault_number_ + 2) {
			qsize = 2 * fault_number_ + 1;
		}
		else if (validators_.size() == 3 * fault_number_ + 3) {
			qsize = 2 * fault_number_ + 1;
		}

		return qsize;
	}

	void Pbft::OnTxTimeout() {
		if (!is_validator_) {
			return;
		}

		utils::MutexGuard lock_guad(lock_);

		LOG_INFO("Send view change message, new view number(" FMT_I64 ")", view_number_ + 1);
		view_active_ = false;
		PeerMessagePointer msg = NewViewChange(view_number_ + 1);
		SendMessage(msg);
	}

	std::string Pbft::GetNodeAddress(const protocol::PbftEnv &pbft_env) {
		PublicKey public_key(pbft_env.signature().public_key());
		return public_key.GetBase58Address();
	}

	int64_t Pbft::GetSeq(const protocol::PbftEnv &pbft_env) {
		const protocol::Pbft &pbft = pbft_env.pbft();
		int64_t sequence = 0;
		switch (pbft.type()) {
		case protocol::PBFT_TYPE_PREPREPARE:{
			if (pbft.has_pre_prepare()) sequence = pbft.pre_prepare().sequence();
			break;
		}
		case protocol::PBFT_TYPE_PREPARE:{
			if (pbft.has_prepare()) sequence = pbft.prepare().sequence();
			break;
		}
		case protocol::PBFT_TYPE_COMMIT:{
			if (pbft.has_commit()) sequence = pbft.commit().sequence();
			break;
		}
										//case protocol::PBFT_TYPE_CHECKPOINT:{
										//	if (pbft.has_checkpoint()) sequence = pbft.checkpoint().sequence();
										//	break;
										//}
		case protocol::PBFT_TYPE_VIEWCHANGE:{
			if (pbft.has_view_change()) sequence = pbft.view_change().sequence();
			break;
		}
		case protocol::PBFT_TYPE_NEWVIEW:{
			if (pbft.has_new_view()) sequence = pbft.new_view().sequence();
			break;
		}
		default:{
			break;
		}
		}

		return sequence;
	}

	int64_t Pbft::GetBlockSeq(const protocol::PbftEnv &pbft_env) {
		const protocol::Pbft &pbft = pbft_env.pbft();
		int64_t sequence = 0;
		if (pbft.type() == protocol::PBFT_TYPE_PREPREPARE && pbft.has_pre_prepare()) {
			return pbft.pre_prepare().block_seq();
		}

		return -1;
	}

	std::vector<protocol::Value> Pbft::GetValue(const protocol::PbftEnv &pbft_env) {
		const protocol::Pbft &pbft = pbft_env.pbft();
		std::vector<protocol::Value> values;
		switch (pbft.type()) {
		case protocol::PBFT_TYPE_PREPREPARE:{
			if (pbft.has_pre_prepare()) values.push_back(pbft.pre_prepare().value());
			break;
		}
											//case protocol::PBFT_TYPE_PREPARE:{
											//	if (pbft.has_prepare()) values.push_back(pbft.prepare().value());
											//	break;
											//}
											//case protocol::PBFT_TYPE_COMMIT:{
											//	if (pbft.has_commit()) values.push_back(pbft.commit().value());
											//	break;
											//}
		case protocol::PBFT_TYPE_VIEWCHANGE:{
			if (pbft.has_view_change()) {
				const protocol::PbftViewChange &view_change = pbft_env.pbft().view_change();
				for (int32_t i = 0; i < view_change.prepared_set_size(); i++) {
					const protocol::PbftPreparedSet &prepare_set = view_change.prepared_set(i);
					const protocol::PbftEnv &pre_prepare = prepare_set.pre_prepare();
					values.push_back(pre_prepare.pbft().pre_prepare().value());
				}
			}
			break;
		}
		case protocol::PBFT_TYPE_NEWVIEW:{
			if (pbft.has_new_view()) {
				const protocol::PbftNewView &new_view = pbft_env.pbft().new_view();
				for (int32_t i = 0; i < new_view.view_changes_size(); i++) {
					const protocol::PbftViewChange &view_change = new_view.view_changes(i).pbft().view_change();
					for (int32_t i = 0; i < view_change.prepared_set_size(); i++) {
						const protocol::PbftPreparedSet &prepare_set = view_change.prepared_set(i);
						const protocol::PbftEnv &pre_prepare = prepare_set.pre_prepare();
						values.push_back(pre_prepare.pbft().pre_prepare().value());
					}
				}
				for (int32_t i = 0; i < new_view.pre_prepares_size(); i++) {
					const protocol::PbftEnv &pre_prepare = new_view.pre_prepares(i);
					values.push_back(pre_prepare.pbft().pre_prepare().value());
				}
			}
			break;
		}
		default:{
			break;
		}
		}

		return values;
	}

	void Pbft::GetModuleStatus(Json::Value &data) {
		utils::MutexGuard lock_guad(lock_);
		data["type"] = name_;
		data["replica_id"] = replica_id_;
		data["sequence"] = sequence_;
		data["view_number"] = view_number_;
		data["ckp_count"] = ckp_count_;
		data["ckp_interval"] = ckp_interval_;
		data["last_exe_seq"] = last_exe_seq_;
		data["low_water_mark"] = low_water_mark_;
		data["fault_number"] = fault_number_;
		data["view_active"] = view_active_;
		Json::Value &instances = data["instances"];
		for (PbftInstanceMap::const_iterator iter = instances_.begin(); iter != instances_.end(); iter++) {
			const PbftInstance &instance = iter->second;
			const PbftInstanceIndex &index = iter->first;
			Json::Value &item = instances[instances.size()];
			item["vn"] = index.view_number_;
			item["seq"] = index.sequence_;
			item["phase"] = GetPhaseDesc(instance.phase_);
			item["phase_item"] = instance.phase_item_;
			item["pre_prepare"] = PbftDesc::GetPrePrepare(instance.pre_prepare_);

			Json::Value &prepares = item["prepares"];
			for (PbftPrepareMap::const_iterator iter_pre = instance.prepares_.begin();
				iter_pre != instance.prepares_.end();
				iter_pre++) {
				Json::Value &prepares_item = prepares[prepares.size()];
				prepares_item = PbftDesc::GetPrepare(iter_pre->second);
			}

			Json::Value &commits = item["commits"];
			for (PbftCommitMap::const_iterator iter_commit = instance.commits_.begin();
				iter_commit != instance.commits_.end();
				iter_commit++) {
				Json::Value &commits_item = commits[commits.size()];
				commits_item = PbftDesc::GetCommit(iter_commit->second);
			}
			item["start_time"] = utils::Timestamp(instance.start_time_).ToFormatString(true);
			item["end_time"] = utils::Timestamp(instance.end_time_).ToFormatString(true);
			item["last_propose_time"] = utils::Timestamp(instance.last_propose_time_).ToFormatString(true);
			item["have_send_viewchange"] = instance.have_send_viewchange_ ? "true" : " false";
			item["pre_prepare_round"] = instance.pre_prepare_round_;
		}

		Json::Value &checkpoints = data["checkpoints"];
		for (PbftCkpInstanceMap::const_iterator iter = ckp_instances_.begin(); iter != ckp_instances_.end(); iter++) {
			const PbftCkpInstance &ckp_instance = iter->second;
			Json::Value &item = checkpoints[checkpoints.size()];
			item["sequence"] = iter->first.sequence_;
			item["state_digest"] = utils::String::BinToHexString(iter->first.state_digest_);
			item["stable"] = ckp_instance.stable_ ? "true" : "false";

			Json::Value &ckp = item["checkpoint"];
			for (PbftCheckPointMap::const_iterator iter_ckp = ckp_instance.checkpoints_.begin(); iter_ckp != ckp_instance.checkpoints_.end(); iter_ckp++) {
				Json::Value &ckp_item = ckp[ckp.size()];
				ckp_item = PbftDesc::GetCheckPoint(iter_ckp->second);
			}
		}

		Json::Value &viewchanges = data["viewchanges"];
		for (PbftVcInstanceMap::const_iterator iter = vc_instances_.begin(); iter != vc_instances_.end(); iter++) {
			const PbftVcInstance &vc_instance = iter->second;
			Json::Value &item = viewchanges[viewchanges.size()];
			item["view_number"] = vc_instance.view_number_;
			item["start_time"] = utils::Timestamp(vc_instance.start_time_).ToFormatString(true);
			item["last_propose_time"] = utils::Timestamp(vc_instance.last_propose_time_).ToFormatString(true);
			item["end_time"] = utils::Timestamp(vc_instance.end_time_).ToFormatString(true);

			Json::Value &vc = item["viewchange"];
			for (PbftViewChangeMap::const_iterator iter_vc = vc_instance.viewchanges_.begin(); iter_vc != vc_instance.viewchanges_.end(); iter_vc++) {
				Json::Value &vc_item = vc[vc.size()];
				vc_item = PbftDesc::GetViewChange(iter_vc->second);
			}
		}
	}

	int32_t Pbft::IsLeader() {
		if (IsValidator() && view_number_ % validators_.size() == replica_id_) {
			return 1;
		}

		return 0;
	}

	const char *Pbft::GetPhaseDesc(PbftInstancePhase phase) {
		switch (phase) {
		case PBFT_PHASE_NONE: return "PHASE_NONE";
		case PBFT_PHASE_PREPREPARED: return "PHASE_PREPREPARE";
		case PBFT_PHASE_PREPARED: return "PHASE_PREPARED";
		case PBFT_PHASE_COMMITED: return "PHASE_COMMITED";
		default: break;
		}
		return "UNDEFINE";
	}
}