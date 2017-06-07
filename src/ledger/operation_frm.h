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

#ifndef OPERATION_FRM_H_
#define OPERATION_FRM_H_

#include <proto/message.pb.h>
#include <utils/common.h>
#include <common/general.h>
#include <ledger/account.h>
#include "transaction_frm.h"

#define METADATA_MAXSIZE 1048576
#define DETAILED_MAX_SIZE 2097152 //2*1024*1024
#define ASSET_CODE_MAX_SIZE 64
#define RECORD_ID_MAX_SIZE 64

namespace bubi {

	class TransactionFrm;
	class AccountEntry;
	class OperationFrm {
	public:
		OperationFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr);
		virtual ~OperationFrm();
		static std::shared_ptr<bubi::OperationFrm> CreateOperation(
			const protocol::Operation &operation,
			TransactionFrm &tran,
			AccountEntry &entry,
			uint32_t ope_seq);
		virtual bool Apply(AccountEntry &entry) = 0;
		virtual int32_t GetThreshold() = 0;
		virtual void SourceRelationTx();
		bool CheckSignature();
		Result GetResult() const;
	protected:
		protocol::Operation operation_;
		TransactionFrm &transaction_;
		AccountFrm::pointer sourceaccount_ptr_;
		Result result_;
	};
};

#endif