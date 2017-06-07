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

#ifndef INIT_PAYMENT_OPE_FRM_H_
#define INIT_PAYMENT_OPE_FRM_H_

#include "init_payment_ope_frm.h"

namespace bubi {
	class InitPaymentOpeFrm : public OperationFrm {
	public:
		InitPaymentOpeFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr);
		~InitPaymentOpeFrm();
		static int32_t CheckValid(const protocol::Operation&, const std::string &);
		virtual bool Apply(AccountEntry &entry);
		virtual int32_t GetThreshold();
		virtual void SourceRelationTx();
	private:
		protocol::OperationInitPayment operation_init_payment_;
	};
}

#endif