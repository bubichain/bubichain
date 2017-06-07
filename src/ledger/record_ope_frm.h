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

#ifndef RECORD_OPE_FRM_H_
#define RECORD_OPE_FRM_H_

#include "operation_frm.h"

namespace bubi {
	class RecordOpeFrm : public OperationFrm {
	public:
		RecordOpeFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr, uint32_t ope_seq);
		~RecordOpeFrm();
		static int32_t  CheckValid(const protocol::Operation &, const std::string &source_address);
		virtual bool Apply(AccountEntry &entry);
		virtual int32_t GetThreshold();
		virtual bool CheckOriginalRecord(const std::string source_record_address, const std::string id);
	private:
		protocol::OperationRecord operation_record_;
		const uint32_t ope_seq_;
	};
};

#endif