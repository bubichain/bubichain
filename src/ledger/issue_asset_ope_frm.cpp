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
#include "issue_asset_ope_frm.h"

namespace bubi {
	IssueOpeFrm::IssueOpeFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr) :
		OperationFrm(operation, tran, account_ptr), operation_issue_asset_(operation.issue_asset()) {}

	IssueOpeFrm::~IssueOpeFrm() {}

	int32_t IssueOpeFrm::CheckValid(const protocol::Operation &ope, const std::string &source_address) {
		auto operation_issue_asset = ope.issue_asset();
		auto assetprop = operation_issue_asset.asset().property();

		protocol::Asset asset = operation_issue_asset.asset();
		if (!AssetFrm::IsValid(asset)) {
			LOG_ERROR("Asset is not legal");
			return (protocol::ERRCODE_ASSET_INVALID);
		}

		if (assetprop.issuer() != source_address) {
			LOG_ERROR("Asset's issuer is not equal to transaction owner");
			return (protocol::ERRCODE_ASSET_INVALID);
		}

		return protocol::ERRCODE_SUCCESS;
	}

	bool IssueOpeFrm::Apply(AccountEntry &entry) {
		std::string strAddress = sourceaccount_ptr_->GetAccountAddress();
		if (!entry.GetEntry(strAddress, sourceaccount_ptr_)) {
			LOG_ERROR("Source account does not exist");
			result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
			return false;
		}

		protocol::AssetProperty_Type pay_type = operation_issue_asset_.asset().property().type();

		protocol::Asset tmp = operation_issue_asset_.asset();
		if (tmp.details_size() == 0) {
			protocol::Detail *dtl = tmp.add_details();
			dtl->set_amount(tmp.amount());
			dtl->set_start(0);
			dtl->set_length(-1); //0 for permanent, -1 for uninitialized
			dtl->set_ext("");
		}

		if (!sourceaccount_ptr_->RecvAsset(tmp, transaction_.GetHeader()->ledger_version())) {
			result_.set_desc("Dest account asset amount too large");
			result_.set_code(protocol::ERRCODE_ACCOUNT_ASSET_AMOUNT_TOO_LARGE);
			LOG_ERROR("Dest account(%s) asset amount too large", sourceaccount_ptr_->GetAccountAddress().c_str());
			return false;
		}

		return true;
	}
	int32_t IssueOpeFrm::GetThreshold() {
		return sourceaccount_ptr_->GetProtoMedThreshold();
	}
}