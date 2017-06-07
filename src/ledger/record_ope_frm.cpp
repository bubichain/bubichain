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

#include <common/configure.h>
#include <ledger/ledger_manager.h>
#include "account.h"
#include "record_ope_frm.h"

namespace bubi {
	RecordOpeFrm::RecordOpeFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr, uint32_t ope_seq) :
		ope_seq_(ope_seq), OperationFrm(operation, tran, account_ptr), operation_record_(operation.record()) {}

	RecordOpeFrm::~RecordOpeFrm() {}

	int32_t RecordOpeFrm::CheckValid(const protocol::Operation &ope, const std::string &source_address) {
		auto operation_record = ope.record();

		if (operation_record.has_address() && !bubi::PublicKey::IsAddressValid(operation_record.address())) {
			LOG_ERROR("Destination address(%s) invalid", operation_record.address().c_str());
			return (protocol::ERRCODE_INVALID_ADDRESS);
		}
		if (operation_record.id().size() <= 0 || operation_record.id().size() > 64) {
			LOG_ERROR("id %u is larger than %u or less 0", operation_record.id().size(), 64);
			return (protocol::ERRCODE_INVALID_PARAMETER);
		}
		if (operation_record.ext().size() <= 0 || operation_record.ext().size() >= 1024) {
			LOG_ERROR("ext %u is larger than %u or less 0", operation_record.ext().size(), 1024);
			return (protocol::ERRCODE_INVALID_PARAMETER);
		}

		return protocol::ERRCODE_SUCCESS;
	}

	bool RecordOpeFrm::Apply(AccountEntry &entry) {
		std::string record_address;

		auto &record_tx = transaction_.records_;
		auto &record_ledger = LedgerManager::Instance().record_sql_;

		std::string id = sourceaccount_ptr_->GetAccountAddress() + operation_record_.id();
		if (operation_record_.has_address()) {
			// additional record	 
			if (record_tx.find(id) == record_tx.end() &&
				record_ledger.find(id) == record_ledger.end() &&
				!CheckOriginalRecord(operation_record_.address(), operation_record_.id())) {
				result_.set_code(protocol::ERRCODE_NOT_EXIST);
				LOG_ERROR("Original record does not exist!address(%s) id(%s)", operation_record_.address().c_str(), operation_record_.id().c_str());
				return false;
			}
			record_address = operation_record_.address();
		}
		else {
			// record
			if (record_tx.find(id) != record_tx.end() ||
				record_ledger.find(id) != record_ledger.end() ||
				CheckOriginalRecord(sourceaccount_ptr_->GetAccountAddress(), operation_record_.id())) {
				result_.set_code(protocol::ERRCODE_ALREADY_EXIST);
				LOG_ERROR("Original record exist!address(%s) id(%s)", sourceaccount_ptr_->GetAccountAddress().c_str(), operation_record_.id().c_str());
				return false;
			}
			record_address = "";
		}

		Json::Value sql;
		sql["ope_seq"] = ope_seq_;
		sql["record_participant"] = sourceaccount_ptr_->GetAccountAddress();
		sql["record_address"] = record_address;
		sql["record_id"] = operation_record_.id();
		sql["record_ext"] = utils::String::BinToHexString(operation_record_.ext()).c_str();

		//add operation to Tx cache
		record_tx[id].push_back(sql);


		return true;
	}
	int32_t RecordOpeFrm::GetThreshold() {
		return sourceaccount_ptr_->GetProtoMedThreshold();
	}

	bool RecordOpeFrm::CheckOriginalRecord(const std::string source_record_address, const std::string id) {

		std::string condition = "WHERE 1=1";


		RationalDb *db = Storage::Instance().rational_db();

		condition += utils::String::Format(" AND %s.record_participant='%s'",
			General::RECORD_TABLE_NAME, db->Format(source_record_address).c_str());

		condition += utils::String::Format(" AND %s.record_address=''",
			General::RECORD_TABLE_NAME);

		condition += utils::String::Format(" AND %s.record_id='%s'",
			General::RECORD_TABLE_NAME, db->Format(id).c_str());
		std::string count_tb_name = General::RECORD_TABLE_NAME;

		int64_t total_count = db->QueryCount(count_tb_name, condition);
		return total_count > 0 ? true : false;
	}
}