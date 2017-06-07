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
#include "issue_unique_asset_ope_frm.h"

namespace bubi {
	IssueUniqueAssetOpeFrm::IssueUniqueAssetOpeFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr, uint32_t ope_seq) :
		OperationFrm(operation, tran, account_ptr), operation_issue_unique_asset_(operation.issue_unique_asset()), ope_seq_(ope_seq) {}

	IssueUniqueAssetOpeFrm::~IssueUniqueAssetOpeFrm() {}

	int32_t IssueUniqueAssetOpeFrm::CheckValid(const protocol::Operation &ope, const std::string &source_address) {
		auto operation_issue_unique_asset = ope.issue_unique_asset();
		auto assetprop = operation_issue_unique_asset.asset().property();

		if (assetprop.type() != protocol::AssetProperty_Type_UNIQUE) {
			LOG_ERROR("Asset type is not legal");
			return (protocol::ERRCODE_ASSET_INVALID);
		}

		if (operation_issue_unique_asset.asset().detailed().size() > DETAILED_MAX_SIZE ||
			assetprop.issuer().size() <= 0 ||
			assetprop.code().size() <= 0 ||
			assetprop.code().size() > ASSET_CODE_MAX_SIZE) {
			return protocol::ERRCODE_INVALID_PARAMETER;
		}

		/// issuer must be equal to operation source address
		if (assetprop.issuer() != source_address) {
			LOG_ERROR("Unique asset's issuer is not equal to transaction owner");
			return (protocol::ERRCODE_ASSET_INVALID);
		}

		return protocol::ERRCODE_SUCCESS;
	}

	bool IssueUniqueAssetOpeFrm::Apply(AccountEntry &entry) {
		std::string str_source_address = sourceaccount_ptr_->GetAccountAddress();
		protocol::UniqueAsset tmp = operation_issue_unique_asset_.asset();

		std::string asset_preperty = tmp.property().SerializeAsString();

		auto &unique_asset_tx = transaction_.unique_asset_;
		auto &unique_asset_ledger = bubi::LedgerManager::Instance().unique_asset_sql_;

		// when unique asset exist,can't payment
		if (unique_asset_tx.find(asset_preperty) != unique_asset_tx.end() ||			// check Tx cache 
			unique_asset_ledger.find(asset_preperty) != unique_asset_ledger.end() ||// check Ledger cache 
			CheckUniqueAsset(tmp.property())										// check DB
			) {
			result_.set_code(protocol::ERRCODE_ALREADY_EXIST);
			LOG_ERROR("Unique asset exist! issuer(%s) code(%s)", tmp.property().issuer().c_str(), tmp.property().code().c_str());
			return false;
		}

		// add unique asset
		if (!sourceaccount_ptr_->RecvUniqueAsset(tmp, transaction_.GetHeader()->ledger_version())) {
			result_.set_code(protocol::ERRCODE_INTERNAL_ERROR);
			LOG_ERROR("Dest account(%s) add asset failed", sourceaccount_ptr_->GetAccountAddress().c_str());
			return false;
		}

		Json::Value sql;
		sql["ope_seq"] = ope_seq_;
		sql["asset_issuer"] = tmp.property().issuer();
		sql["asset_code"] = tmp.property().code();
		sql["asset_detailed"] = utils::String::BinToHexString(tmp.detailed());
		sql["from_address"] = str_source_address;
		sql["to_address"] = str_source_address;

		//add operation to Tx cache
		unique_asset_tx[asset_preperty].push_back(sql);
		return true;
	}

	int32_t IssueUniqueAssetOpeFrm::GetThreshold() {
		return sourceaccount_ptr_->GetProtoMedThreshold();
	}

	bool IssueUniqueAssetOpeFrm::CheckUniqueAsset(const protocol::AssetProperty asset_prop) {
		std::string asset_issuer = asset_prop.issuer();
		std::string asset_code = asset_prop.code();

		RationalDb *db = Storage::Instance().rational_db();

		std::string condition = " WHERE 1=1";
		std::string count_tb_name = "";
		condition += utils::String::Format(" AND %s.asset_issuer='%s'",
			General::UNIQUE_ASSET_NAME, db->Format(asset_issuer).c_str());

		condition += utils::String::Format(" AND %s.asset_code='%s'",
			General::UNIQUE_ASSET_NAME, db->Format(asset_code).c_str());

		count_tb_name = General::UNIQUE_ASSET_NAME;

		int64_t total_count = db->QueryCount(count_tb_name, condition);
		return total_count > 0 ? true : false;
	}
}