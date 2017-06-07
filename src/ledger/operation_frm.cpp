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

#include "transaction_frm.h"
#include "operation_frm.h"
#include "createaccount_ope_frm.h"
#include "issue_asset_ope_frm.h"
#include "payment_ope_frm.h"
#include "setoptions_ope_frm.h"
#include "production_ope_frm.h"
#include "issue_unique_asset_ope_frm.h"
#include "init_payment_ope_frm.h"
#include "payment_unique_asset_ope_frm.h"
#include "record_ope_frm.h"
#include <ledger/ledger_manager.h>

namespace bubi {
	OperationFrm::OperationFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr) :
		operation_(operation), transaction_(tran), sourceaccount_ptr_(account_ptr) {}

	OperationFrm::~OperationFrm() {}

	std::shared_ptr<OperationFrm> OperationFrm::CreateOperation(
		const protocol::Operation &operation,
		TransactionFrm &tran,
		AccountEntry &entry,
		uint32_t ope_seq) {

		std::string strSource = tran.GetSourceAddress();
		if (operation.has_source_address()) {
			strSource = operation.source_address();
		}

		AccountFrm::pointer source_ptr;
		if (!entry.GetEntry(strSource, source_ptr)) {
			LOG_ERROR("Source address doesn't exist,tx(%s)", utils::encode_b16(tran.GetContentHash()).c_str());
			return nullptr;
		}
		switch (operation.type()) {
		case protocol::Operation_Type_CREATE_ACCOUNT:
		{
			return std::shared_ptr<OperationFrm>(new CreateAccountOpeFrm(operation, tran, source_ptr));
		}
		case protocol::Operation_Type_ISSUE_ASSET:
		{
			return std::shared_ptr<OperationFrm>(new IssueOpeFrm(operation, tran, source_ptr));
		}
		case protocol::Operation_Type_INIT_PAYMENT:
		{
			return std::shared_ptr<OperationFrm>(new InitPaymentOpeFrm(operation, tran, source_ptr));
		}
		case  protocol::Operation_Type_PAYMENT:
		{
			return std::shared_ptr<OperationFrm>(new PaymentOpeFrm(operation, tran, source_ptr));
		}

		case  protocol::Operation_Type_SET_OPTIONS:
		{
			return std::shared_ptr<OperationFrm>(new SetOptionsOpeFrm(operation, tran, source_ptr));
		}
		case  protocol::Operation_Type_PRODUCTION:
		{
			return std::shared_ptr<OperationFrm>(new ProductionFrm(operation, tran, source_ptr));
		}
		case protocol::Operation_Type_ISSUE_UNIQUE_ASSET:
		{
			return std::shared_ptr<OperationFrm>(new IssueUniqueAssetOpeFrm(operation, tran, source_ptr, ope_seq));
		}
		case protocol::Operation_Type_PAYMENT_UNIQUE_ASSET:
		{
			return std::shared_ptr<OperationFrm>(new PaymentUniqueAssetOpeFrm(operation, tran, source_ptr, ope_seq));
		}
		case protocol::Operation_Type_RECORD:
		{
			return std::shared_ptr<OperationFrm>(new RecordOpeFrm(operation, tran, source_ptr, ope_seq));
		}
		}

		return nullptr;
	}

	Result OperationFrm::GetResult() const {
		return result_;
	}

	void OperationFrm::SourceRelationTx() {
		if (operation_.has_source_address()) {
			transaction_.account_tx_.insert(operation_.source_address());
		}
	}

	bool OperationFrm::CheckSignature() {
		if (!transaction_.VerifyWeight(sourceaccount_ptr_, GetThreshold())) {
			LOG_ERROR("Check operation's signature failed");
			result_.set_code(protocol::ERRCODE_INVALID_SIGNATURE);
			return false;
		}

		return true;
	}
}
