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

#include <utils/logger.h>
#include <common/configure.h>
#include <ledger/ledger_manager.h>
#include "production_ope_frm.h"

namespace bubi {
	ProductionFrm::ProductionFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr) :
		OperationFrm(operation, tran, account_ptr), operation_production_(operation.production()) {}

	ProductionFrm::~ProductionFrm() {

	}

	int32_t ProductionFrm::CheckValid(const protocol::Operation &ope,
		const std::string &source_address) {
		auto operation_production = ope.production();

		for (int i = 0; i < operation_production.inputs_size(); i++) {
			protocol::Input tmp = operation_production.inputs(i);
			if (tmp.hash().size() != 32 ||
				tmp.index() < 0 ||
				tmp.metadata().length() > METADATA_MAXSIZE) {
				LOG_ERROR("Invalid parameter");
				return(protocol::ERRCODE_INVALID_PARAMETER);
			}
		}

		for (int i = 0; i < operation_production.outputs_size(); i++) {
			protocol::Output tmp = operation_production.outputs(i);

			if (!PublicKey::IsAddressValid(tmp.address()) ||
				tmp.metadata().size() > METADATA_MAXSIZE) {
				LOG_ERROR("Invalid parameter");
				return (protocol::ERRCODE_INVALID_ADDRESS);
			}
		}
		return (protocol::ERRCODE_SUCCESS);
	}

	void ProductionFrm::SourceRelationTx() {
		if (operation_.has_source_address()) {
			transaction_.account_tx_.insert(operation_.source_address());
		}
	}

	bool ProductionFrm::Apply(AccountEntry &entry) {
		for (int i = 0; i < operation_production_.inputs_size(); i++) {
			protocol::Input input = operation_production_.inputs(i);
			TransactionFrm last_tx;
			std::string hash_hex = utils::String::BinToHexString(input.hash());
			uint32_t loadcode = last_tx.LoadFromDb(hash_hex);
			if (protocol::ERRCODE_SUCCESS != loadcode) {
				result_.set_code(loadcode);
				result_.set_desc(utils::String::Format("Input transaction(%s) not exists", hash_hex.c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				return false;
			}

			if (last_tx.GetResult().code() != protocol::ERRCODE_SUCCESS) {
				result_.set_code(protocol::ERRCODE_INPUT_INVALID);
				result_.set_desc(utils::String::Format("A failed transaction(%s) can not be a input.", hash_hex.c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				return false;
			}

			if (last_tx.GetTransactionEnv().transaction().operations_size() != 1) {
				result_.set_code(protocol::ERRCODE_INPUT_INVALID);
				result_.set_desc(utils::String::Format("Transaction(%s) is not a valid transaction for supply", hash_hex.c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				return false;
			}

			if (!last_tx.GetTransactionEnv().transaction().operations(0).has_production()) {
				result_.set_code(protocol::ERRCODE_INPUT_INVALID);
				result_.set_desc(utils::String::Format("Transaction(%s) is not a valid transaction for supply", hash_hex.c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				return false;
			}

			const protocol::OperationProduction last_product = last_tx.GetTransactionEnv().transaction().operations(0).production();
			if (input.index() >= last_product.outputs_size()) {
				result_.set_code(protocol::ERRCODE_INPUT_NOT_EXIST);
				result_.set_desc(utils::String::Format("Try use transaction(%s) output(" FMT_I64 ") as a input,but it does not have one",
					input.index(), hash_hex.c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				return false;
			}

			//the last production output is for account A, if A does not agree with this transaction,this transaction fail.
			std::string address = last_product.outputs(int(input.index())).address();
			AccountFrm::pointer account_frm;
			if (!entry.GetEntry(address, account_frm)) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
				result_.set_desc(utils::String::Format("Account(%s) not exists. This should not happen", address.c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				return false;
			}

			if (!transaction_.VerifyWeight(account_frm, account_frm->GetProtoMedThreshold())) {
				result_.set_code(protocol::ERRCODE_INVALID_SIGNATURE);
				result_.set_desc(utils::String::Format("Permission denied.try to use tx(%s) output[" FMT_I64 "] as input "
					"which is for account(%s).May be not enough valid signatures.",
					hash_hex.c_str(), input.index(), address.c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				return false;
			}
		}
		for (int i = 0; i < operation_production_.outputs_size(); i++) {
			protocol::Output tmp = operation_production_.outputs(i);
			AccountFrm::pointer output_account;
			if (!entry.GetEntry(tmp.address(), output_account)) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
				result_.set_desc(utils::String::Format("Output[%d],account (%s) does not exist", i, tmp.address().c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				return false;
			}
		}
		SourceRelationTx();
		return true;
	}

	int32_t ProductionFrm::GetThreshold() {
		return sourceaccount_ptr_->GetProtoMedThreshold();
	}

	void ProductionFrm::ToJson(Json::Value &js, const protocol::OperationProduction &production) {
		Json::Value &inputs = js["inputs"];
		for (int i = 0; i < production.inputs_size(); i++) {
			Json::Value inputitem;
			inputitem["hash"] = utils::String::BinToHexString(production.inputs(i).hash());
			inputitem["index"] = production.inputs(i).index();
			if (production.inputs(i).has_metadata()) {
				inputitem["metadata"] = utils::String::BinToHexString(production.inputs(i).metadata());
			}
			inputs[i] = inputitem;
		}

		Json::Value &outputs = js["outputs"];
		for (int i = 0; i < production.outputs_size(); i++) {
			outputs[i]["address"] = production.outputs(i).address();
			if (production.outputs(i).has_metadata()) {
				std::string metadata_b16 = utils::String::BinToHexString(production.outputs(i).metadata());
				outputs[i]["metadata"] = metadata_b16;
			}
		}
	}
}