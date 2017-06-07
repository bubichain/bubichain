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
#include "createaccount_ope_frm.h"

namespace bubi {
	CreateAccountOpeFrm::CreateAccountOpeFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr) :
		OperationFrm(operation, tran, account_ptr), operation_createaccount_(operation.create_account()) {}

	CreateAccountOpeFrm::~CreateAccountOpeFrm() {}
	//check item
	int32_t CreateAccountOpeFrm::CheckValid(const protocol::Operation &ope,
		const std::string &source_address) {
		auto ope_createaccount = ope.create_account();


		if (ope_createaccount.dest_address() == source_address) {
			LOG_ERROR("Destination address is equal source address");
			return(protocol::ERRCODE_ACCOUNT_SOURCEDEST_EQUAL);
		}

		if (!bubi::PublicKey::IsAddressValid(ope_createaccount.dest_address())) {
			LOG_ERROR("Destination address(%s) invalid", ope_createaccount.dest_address().c_str());
			return (protocol::ERRCODE_INVALID_ADDRESS);
		}

		if (ope_createaccount.has_thresholds()) {
			const protocol::AccountThreshold &threshold = ope_createaccount.thresholds();

			if (threshold.has_master_weight() && threshold.master_weight() > UINT8_MAX) {
				LOG_ERROR("Master weight %u is larger than %u or less 0", threshold.master_weight(), UINT8_MAX);
				return (protocol::ERRCODE_WEIGHT_NOT_VALID);
			}

			if (threshold.has_high_threshold() && threshold.high_threshold() > UINT16_MAX) {
				LOG_ERROR("High threshold %u is larger than %u or less 0", threshold.high_threshold(), UINT16_MAX);
				return (protocol::ERRCODE_WEIGHT_NOT_VALID);
			}

			if (threshold.has_med_threshold() && threshold.med_threshold() > UINT16_MAX) {
				LOG_ERROR("Med threshold %u is larger than %u or less 0", threshold.med_threshold(), UINT16_MAX);
				return (protocol::ERRCODE_WEIGHT_NOT_VALID);
			}

			if (threshold.has_low_threshold() && threshold.low_threshold() > UINT16_MAX) {
				LOG_ERROR("Low threshold %u is larger than %u or less 0", threshold.low_threshold(), UINT16_MAX);
				return (protocol::ERRCODE_WEIGHT_NOT_VALID);
			}
		}

		for (int32_t i = 0; i < ope_createaccount.signers_size(); i++) {
			const protocol::Signer &signer = ope_createaccount.signers(i);
			if (signer.weight() > UINT8_MAX) {
				LOG_ERROR("Signer weight %u is larger than %u ", signer.weight(), UINT8_MAX);
				return (protocol::ERRCODE_WEIGHT_NOT_VALID);
			}

			if (!PublicKey::IsAddressValid(signer.address())) {
				LOG_ERROR("Signer address(%s) is not valid", signer.address().c_str());
				return (protocol::ERRCODE_INVALID_ADDRESS);
			}
		}
		return protocol::ERRCODE_SUCCESS;
	}

	void CreateAccountOpeFrm::SourceRelationTx() {
		if (operation_.has_source_address()) {
			transaction_.account_tx_.insert(operation_.source_address());
		}
		transaction_.account_tx_.insert(operation_createaccount_.dest_address());
	}

	bool CreateAccountOpeFrm::Apply(AccountEntry &entry) {

		std::string strAddress = sourceaccount_ptr_->GetAccountAddress();
		if (!entry.GetEntry(strAddress, sourceaccount_ptr_)) {
			LOG_ERROR("source account does not exist!");
			result_.set_code(protocol::ERRCODE_INTERNAL_ERROR);
			return false;
		}

		do {
			AccountFrm::pointer dest_paccountfrm;
			std::string dest_address = operation_createaccount_.dest_address();
			if (entry.GetEntry(dest_address, dest_paccountfrm)) {
				LOG_ERROR("Destination address already exist");
				result_.set_code(protocol::ERRCODE_ACCOUNT_DEST_EXIST);
				break;
			}

			int32_t base_reserve = transaction_.GetHeader()->base_reserve();
			//check the dest address balance
			if (operation_createaccount_.init_balance() < base_reserve) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_LOW_RESERVE);
				result_.set_desc("Dest address balance is low");
				LOG_ERROR("Dest address balance is low");
				break;
			}
			//check the source address balance
			if (sourceaccount_ptr_->GetAccountBalance() - base_reserve < operation_createaccount_.init_balance()) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_LOW_RESERVE);
				result_.set_desc("Source address balance is low");
				LOG_ERROR("Source address(%s) balance is low", strAddress.c_str());
				break;
			}
			//minus the transaction fee from source address
			sourceaccount_ptr_->AddBalance(-1 * operation_createaccount_.init_balance());

			//create new account struct and set the member value from transaction message
			dest_paccountfrm = std::make_shared<AccountFrm>();
			protocol::Account &proto_account = dest_paccountfrm->GetProtoAccount();
			proto_account.set_account_balance(operation_createaccount_.init_balance());
			proto_account.set_previous_ledger_seq(1);
			proto_account.set_previous_tx_hash(transaction_.GetContentHash());
			proto_account.set_account_address(dest_address);
			proto_account.set_metadata("");
			proto_account.clear_unique_asset();
			proto_account.clear_assets();
			proto_account.set_tx_seq(transaction_.GetLedgerSeq() << 32);

			if (operation_createaccount_.has_account_metadata())
				proto_account.set_metadata(operation_createaccount_.account_metadata());
			else
				proto_account.set_metadata("");

			if (transaction_.GetHeader()->ledger_version() >= 1004) {
				proto_account.set_metadata_version(1);
			}
			int32_t master_weight = 1, low_threshold = 1, med_threshold = 1, high_threshold = 1;
			if (operation_createaccount_.has_thresholds()) {
				master_weight = operation_createaccount_.thresholds().master_weight()  &UINT8_MAX;
				low_threshold = operation_createaccount_.thresholds().low_threshold();
				med_threshold = operation_createaccount_.thresholds().med_threshold();
				high_threshold = operation_createaccount_.thresholds().high_threshold();
			}

			dest_paccountfrm->SetProtoMasterWeight(master_weight);
			dest_paccountfrm->SetProtoLowThreshold(low_threshold);
			dest_paccountfrm->SetProtoMedThreshold(med_threshold);
			dest_paccountfrm->SetProtoHighThreshold(high_threshold);
			for (int32_t i = 0; i < operation_createaccount_.signers_size(); i++) {
				dest_paccountfrm->UpdateSigner(operation_createaccount_.signers(i).address(), operation_createaccount_.signers(i).weight());
			}
			if (!entry.AddEntry(dest_address, dest_paccountfrm)) {
				BUBI_EXIT("fatal error");
			}
			return true;
		} while (false);

		return false;
	}
	int32_t CreateAccountOpeFrm::GetThreshold() {
		return sourceaccount_ptr_->GetProtoMedThreshold();
	}

	void CreateAccountOpeFrm::ToJson(const protocol::OperationCreateAccount &ope, Json::Value &js) {
		js["dest_address"] = ope.dest_address();
		js["init_balance"] = ope.init_balance();
		if (ope.has_thresholds()) {
			Json::Value &threshold = js["threshold"];
			const protocol::AccountThreshold &ts = ope.thresholds();
			if (ts.has_high_threshold()) {
				threshold["high_threshold"] = ts.high_threshold();
			}
			if (ts.has_low_threshold()) {
				threshold["low_threshold"] = ts.low_threshold();
			}
			if (ts.has_med_threshold()) {
				threshold["med_threshold"] = ts.med_threshold();
			}
			if (ts.has_master_weight()) {
				threshold["master_weight"] = ts.master_weight();
			}
		}

		if (ope.signers_size() != 0) {
			Json::Value &signers = js["signers"];
			for (int32_t i = 0; i < ope.signers_size(); i++) {
				const protocol::Signer &signer = ope.signers(i);
				Json::Value item;
				item["address"] = signer.address();
				item["weight"] = signer.weight();
				signers[i] = item;
			}
		}
	}
}