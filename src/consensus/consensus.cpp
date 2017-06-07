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

#include <utils/crypto.h>
#include <utils/logger.h>
#include <common/storage.h>
#include <common/configure.h>
#include "consensus.h"

namespace bubi {
	Consensus::Consensus() : name_("consensus"),
		is_validator_(false),
		replica_id_(-1),
		private_key_(Configure::Instance().validation_configure_.node_privatekey_) {}

	Consensus::~Consensus() {}

	bool Consensus::Initialize() {
		const ValidationConfigure &config = Configure::Instance().validation_configure_;
		int64_t counter = 0;
		for (auto const &iter : config.validators_) {
			validators_.insert(std::make_pair(iter, counter++));
		}

		std::string node_address = private_key_.GetBase58Address();
		is_validator_ = config.is_validator_;

		std::string str_validators;
		for (auto &iter : config.validators_) {
			str_validators = utils::String::AppendFormat(str_validators, "%s", iter.c_str());
		}
		validators_hash_ = utils::Sha256::Crypto(str_validators);

		if (is_validator_) {
			std::map<std::string, int64_t>::const_iterator iter = validators_.find(node_address);
			if (iter == validators_.end()) {

				LOG_ERROR("Validator node(%s) is not in list", node_address.c_str());
				return false;
			}

			replica_id_ = iter->second;
		}

		return true;
	}

	bool Consensus::Exit() {
		return true;
	}

	bool Consensus::SendMessage(const PeerMessagePointer &message) {
		if (!is_validator_) {
			return false;
		}

		notify_->SendConsensusMessage(message);
		return true;
	}

	int64_t Consensus::GetValidatorIndex(const std::string &node_address) const {
		std::map<std::string, int64_t>::const_iterator iter = validators_.find(node_address);
		if (iter != validators_.end()) {
			return iter->second;
		}

		return -1;
	}

	std::string Consensus::OnValueCommited(int64_t block_seq, int64_t request_seq, const protocol::Value &value, bool calculate_total) {
		return notify_->OnValueCommited(block_seq, request_seq, value, calculate_total);
	}

	void Consensus::OnViewChanged() {
		notify_->OnViewChanged();
	}

	int32_t Consensus::CheckValue(int64_t block_seq, const protocol::Value &value) {
		return notify_->CheckValue(block_seq, value);
	}

	int32_t Consensus::CompareValue(const protocol::Value &value1, const protocol::Value &value2) {
		return value1.SerializeAsString().compare(value2.SerializeAsString());
	}

	int32_t Consensus::CompareValue(const std::string &value1, const std::string &value2) {
		return value1.compare(value2);
	}

	bool Consensus::IsValidator() {
		return is_validator_;
	}

	std::string Consensus::GetNodeAddress() {
		return private_key_.GetBase58Address();
	}

	std::string Consensus::GetValueString(const protocol::Value &value) {
		return utils::String::Format("hash:%s, close time:" FMT_U64, utils::String::Bin4ToHexString(value.hash_set()).c_str(), value.close_time());
	}

	bool Consensus::SaveValue(const std::string &name, const std::string &value) {
		KeyValueDb *db = Storage::Instance().keyvalue_db();
		return db->Put(utils::String::Format("%s_%s", bubi::General::CONSENSUS_PREFIX, name.c_str()), value);
	}

	bool Consensus::SaveValue(const std::string &name, int64_t value) {
		LOG_INFO("Set %s to value(" FMT_I64 ") ", name.c_str(), value);
		return SaveValue(name, utils::String::ToString(value));
	}

	int32_t Consensus::LoadValue(const std::string &name, std::string &value) {
		KeyValueDb *db = Storage::Instance().keyvalue_db();
		return db->Get(utils::String::Format("%s_%s", bubi::General::CONSENSUS_PREFIX, name.c_str()), value) ? 1 : 0;
		/*
		RationalDb *db = Storage::Instance().rational_db();
		Json::Value record;
		int32_t ret = db->QueryRecord(bubi::General::TABLE_GLOBAL, utils::String::Format("where name='%s'", name.c_str()), record);
		if (ret > 0) value = record["value"].asString();
		if (ret < 0){
		LOG_ERROR_ERRNO("Consensus load name(%s) failed", name.c_str(), db->error_code(), db->error_desc());
		}
		return ret;
		*/
	}

	bool Consensus::DelValue(const std::string &name) {
		KeyValueDb *db = Storage::Instance().keyvalue_db();
		return db->Delete(utils::String::Format("%s_%s", bubi::General::CONSENSUS_PREFIX, name.c_str())) ? 1 : 0;
	}

	int32_t Consensus::LoadValue(const std::string &name, int64_t &value) {
		std::string strvalue;
		int32_t ret = LoadValue(name, strvalue);
		if (ret > 0) value = utils::String::Stoi64(strvalue);
		return ret;
	}

	bool Consensus::InsertValue(const std::string &name, const std::string &value) {
		RationalDb *db = Storage::Instance().rational_db();
		utils::StringMap record;
		record["name"] = name;
		record["value"] = value;
		if (!db->Insert(bubi::General::TABLE_GLOBAL, record)) {
			LOG_ERROR_ERRNO("Consensus insert name(%s) failed", name.c_str(), db->error_code(), db->error_desc());
			return false;
		}

		return true;
	}

	void Consensus::SetNotify(IConsensusNotify *notify) {
		notify_ = notify;
	}

	bool Consensus::InsertValue(const std::string &name, int64_t value) {
		return InsertValue(name, utils::String::ToString(value));
	}

	OneNode::OneNode() {
		name_ = "one_node";
	}

	OneNode::~OneNode() {}

	bool OneNode::Request(int64_t block_seq, const protocol::Value &value) {
		OnValueCommited(block_seq, block_seq + 1, value, true);
		return true;
	}

	void OneNode::GetModuleStatus(Json::Value &data) {
		data["type"] = name_;
	}

	ValueSaver::ValueSaver() :write_size_(0) {};
	ValueSaver::~ValueSaver() {
		Commit();
	};

	void ValueSaver::SaveValue(const std::string &name, const std::string &value) {
		writes_.Put(utils::String::Format("%s_%s", bubi::General::CONSENSUS_PREFIX, name.c_str()), value);
		write_size_++;
	}

	void ValueSaver::SaveValue(const std::string &name, int64_t value) {
		LOG_INFO("Set %s to value(" FMT_I64 ") ", name.c_str(), value);
		SaveValue(name, utils::String::ToString(value));
	}

	void ValueSaver::DelValue(const std::string &name) {
		writes_.Delete(name);
		write_size_++;
	}

	bool ValueSaver::Commit() {
		KeyValueDb *db = Storage::Instance().keyvalue_db();
		bool ret = true;
		if (write_size_ > 0) {
			ret = db->WriteBatch(writes_);
			write_size_ = 0;
		}

		return true;
	}
}