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
#include "setoptions_ope_frm.h"

namespace bubi {

	SetOptionsOpeFrm::SetOptionsOpeFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr) :
		OperationFrm(operation, tran, account_ptr), operation_setoptions_(operation.setoptions()) {}

	SetOptionsOpeFrm::~SetOptionsOpeFrm() {}

	int32_t SetOptionsOpeFrm::CheckValid(const protocol::Operation &ope,
		const std::string &source_address) {
		auto operation_setoptions = ope.setoptions();
		if (operation_setoptions.has_master_weight() && operation_setoptions.master_weight() > UINT8_MAX) {
			LOG_ERROR("Master weight %u is larger than %u ", operation_setoptions.master_weight(), UINT8_MAX);
			return(protocol::ERRCODE_WEIGHT_NOT_VALID);
		}

		if (operation_setoptions.has_high_threshold() && operation_setoptions.high_threshold() > UINT16_MAX) {
			LOG_ERROR("High threshold %u is larger than %u or less 0", operation_setoptions.high_threshold(), UINT16_MAX);
			return(protocol::ERRCODE_WEIGHT_NOT_VALID);
		}

		if (operation_setoptions.has_med_threshold() && operation_setoptions.med_threshold() > UINT16_MAX) {
			LOG_ERROR("Med threshold %u is larger than %u or less 0", operation_setoptions.med_threshold(), UINT16_MAX);
			return(protocol::ERRCODE_WEIGHT_NOT_VALID);
		}

		if (operation_setoptions.has_low_threshold() && operation_setoptions.low_threshold() > UINT16_MAX) {
			LOG_ERROR("Low threshold %u is larger than %u or less 0", operation_setoptions.low_threshold(), UINT16_MAX);
			return (protocol::ERRCODE_WEIGHT_NOT_VALID);
		}

		if (operation_setoptions.has_account_metadata() && operation_setoptions.account_metadata().size() > METADATA_MAXSIZE) {
			LOG_ERROR("The length of the metadata exceeds the maximum length");
			return (protocol::ERRCODE_INVALID_PARAMETER);
		}

		for (int32_t i = 0; i < operation_setoptions.signers_size(); i++) {
			const protocol::Signer &signer = operation_setoptions.signers(i);
			if (signer.weight() > UINT8_MAX) {
				LOG_ERROR("Signer weight %u is larger than %u", signer.weight(), UINT8_MAX);
				return(protocol::ERRCODE_WEIGHT_NOT_VALID);
			}

			if (!PublicKey::IsAddressValid(signer.address())) {
				LOG_ERROR("Signer address(%s) is not valid", signer.address().c_str());
				return (protocol::ERRCODE_INVALID_ADDRESS);
			}
		}

		return protocol::ERRCODE_SUCCESS;
	}

	bool SetOptionsOpeFrm::Apply(AccountEntry &entry) {
		std::string strAddress = sourceaccount_ptr_->GetAccountAddress();
		if (!entry.GetEntry(strAddress, sourceaccount_ptr_)) {
			LOG_ERROR("Source account does not exist!");
			result_.set_code(protocol::ERRCODE_ACCOUNT_SOURCEDEST_EQUAL);
			return false;
		}

		do {
			if (operation_setoptions_.has_master_weight()) {
				sourceaccount_ptr_->SetProtoMasterWeight(operation_setoptions_.master_weight()  &UINT8_MAX);
			}

			if (operation_setoptions_.has_low_threshold()) {
				sourceaccount_ptr_->SetProtoLowThreshold(operation_setoptions_.low_threshold());
			}

			if (operation_setoptions_.has_med_threshold()) {
				sourceaccount_ptr_->SetProtoMedThreshold(operation_setoptions_.med_threshold());
			}

			if (operation_setoptions_.has_high_threshold()) {
				sourceaccount_ptr_->SetProtoHighThreshold(operation_setoptions_.high_threshold());
			}

			if (operation_setoptions_.has_account_metadata()) {
				if (transaction_.GetHeader()->ledger_version() >= 1004) {
					int64_t pre_version = 1;
					protocol::Account &proto_account = sourceaccount_ptr_->GetProtoAccount();
					if (proto_account.has_metadata_version()) {
						pre_version = proto_account.metadata_version();
					}

					if (operation_setoptions_.has_account_metadata_version() &&
						operation_setoptions_.account_metadata_version() != pre_version) {
						result_.set_code(protocol::ERRCODE_INVALID_DATAVERSION);
						break;
					}
					proto_account.set_metadata_version(pre_version + 1);
				}
				sourceaccount_ptr_->GetProtoAccount().set_metadata(operation_setoptions_.account_metadata());
			}

			for (int32_t i = 0; i < operation_setoptions_.signers_size(); i++) {
				sourceaccount_ptr_->UpdateSigner(operation_setoptions_.signers(i).address(), operation_setoptions_.signers(i).weight());
			}
			return true;
		} while (false);

		return false;
	}

	int32_t SetOptionsOpeFrm::GetThreshold() {
		return sourceaccount_ptr_->GetProtoHighThreshold();
	}

	void SetOptionsOpeFrm::ToJson(const protocol::OperationSetOptions &ope, Json::Value &js) {
		Json::Value &threshold = js["threshold"];
		if (ope.has_master_weight())
			threshold["master_weight"] = ope.master_weight();
		if (ope.has_low_threshold())
			threshold["low_threshold"] = ope.low_threshold();
		if (ope.has_med_threshold())
			threshold["med_threshold"] = ope.med_threshold();
		if (ope.has_high_threshold())
			threshold["high_threshold"] = ope.high_threshold();
		if (ope.has_account_metadata())
			threshold["metadata"] = utils::String::BinToHexString(ope.account_metadata());
		if (ope.has_account_metadata_version())
			threshold["metadata_version"] = ope.account_metadata_version();

		Json::Value &json_signers = js["signers"];
		for (int32_t i = 0; i < ope.signers_size(); i++) {
			const protocol::Signer *signer = &ope.signers(i);
			Json::Value json_signer;
			json_signer["address"] = signer->address();
			json_signer["weight"] = signer->weight();
			json_signers[i] = json_signer;
		}
	}
}