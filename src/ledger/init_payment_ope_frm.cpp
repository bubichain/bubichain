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
#include <utils/logger.h>
#include <common/configure.h>
#include <ledger/ledger_manager.h>
#include "init_payment_ope_frm.h"
#include "asset_frm.h"

namespace bubi {
	InitPaymentOpeFrm::InitPaymentOpeFrm(const protocol::Operation &operation,
		TransactionFrm &tran,
		AccountFrm::pointer &account_ptr) :
		OperationFrm(operation, tran, account_ptr),
		operation_init_payment_(operation.init_payment()) {}

	InitPaymentOpeFrm::~InitPaymentOpeFrm() {}

	int32_t InitPaymentOpeFrm::CheckValid(const protocol::Operation &ope,
		const std::string &source_address) {
		auto operation_init_payment = ope.init_payment();
		protocol::Asset asset = operation_init_payment.asset();
		for (int i = 0; i < asset.details_size(); i++) {
			if (asset.details(i).length() < 0) {
				LOG_ERROR("Asset's detail not invalid");
				return (protocol::ERRCODE_ASSET_INVALID);
			}
		}

		if (!AssetFrm::IsValid(asset)) {
			LOG_ERROR("Asset's invalid");
			return (protocol::ERRCODE_ASSET_INVALID);
		}
		if (!PublicKey::IsAddressValid(operation_init_payment.destaddress())) {
			LOG_ERROR("Destination address(%s) invalid", operation_init_payment.destaddress().c_str());
			return(protocol::ERRCODE_INVALID_ADDRESS);
		}
		if (operation_init_payment.destaddress() == source_address) {
			LOG_ERROR("Destination address is equal source address");
			return(protocol::ERRCODE_ACCOUNT_SOURCEDEST_EQUAL);
		}
		return protocol::ERRCODE_SUCCESS;
	}

	void InitPaymentOpeFrm::SourceRelationTx() {
		if (operation_.has_source_address()) {
			transaction_.account_tx_.insert(operation_.source_address());
		}
		transaction_.account_tx_.insert(operation_init_payment_.destaddress());
	}

	bool InitPaymentOpeFrm::Apply(AccountEntry &entry) {
		std::string source_address = sourceaccount_ptr_->GetAccountAddress();
		if (!entry.GetEntry(source_address, sourceaccount_ptr_)) {
			LOG_ERROR("Source account(%s) does not exist", source_address.c_str());
			result_.set_code(protocol::ERRCODE_INTERNAL_ERROR);
			result_.set_desc("source account does not exist!");
			return false;
		}

		protocol::AssetProperty_Type pay_type = operation_init_payment_.asset().property().type();
		AccountFrm::pointer dest_account;
		std::string dest_address = operation_init_payment_.destaddress();
		if (!entry.GetEntry(dest_address, dest_account)) {
			dest_account = std::make_shared<AccountFrm>();
			if (sourceaccount_ptr_->GetAccountBalance() >= 2 * transaction_.GetHeader()->base_reserve() &&
				transaction_.GetHeader()->ledger_version() < 2000
				) {
				protocol::Account &proto_account = dest_account->GetProtoAccount();
				proto_account.set_account_address(operation_init_payment_.destaddress());
				proto_account.set_metadata("");
				proto_account.set_account_balance(transaction_.GetHeader()->base_reserve());
				proto_account.set_previous_ledger_seq(transaction_.GetLedgerSeq());
				proto_account.set_tx_seq(transaction_.GetLedgerSeq() << 32);
				proto_account.set_previous_ledger_seq(transaction_.GetLedgerSeq());
				proto_account.set_previous_tx_hash(transaction_.GetContentHash());
				proto_account.clear_assets();
				proto_account.clear_signers();

				dest_account->SetProtoMasterWeight(1);
				dest_account->SetProtoLowThreshold(1);
				dest_account->SetProtoMedThreshold(1);
				dest_account->SetProtoHighThreshold(1);
				entry.AddEntry(dest_address, dest_account);
				sourceaccount_ptr_->AddBalance(-1 * (int64_t)transaction_.GetHeader()->base_reserve());
			}
			else {
				result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
				result_.set_desc("dest account does not exist");
				LOG_ERROR("Dest account(%s) does not exist", dest_address.c_str());
				return false;
			}
		}


		protocol::Asset source_asset;
		protocol::Asset pay = operation_init_payment_.asset();
		if (!sourceaccount_ptr_->GetAsset(pay.property(), source_asset)) {
			result_.set_code(protocol::ERRCODE_ACCOUNT_ASSET_LOW_RESERVE);
			result_.set_desc("source account asset not enough");
			LOG_ERROR("Source account(%s) asset not enough", source_address.c_str());
			return false;
		}

		for (int i = 0; i < source_asset.details_size(); i++) {
			if (source_asset.details(i).length() >= 0)
				continue;

			if (source_asset.details(i).amount() < pay.amount()) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_ASSET_LOW_RESERVE);
				result_.set_desc("source account asset not enough");
				LOG_ERROR("Source account(%s) asset not enough", source_address.c_str());
				return false;
			}

			if (pay.details_size() == 0) {
				protocol::Detail* default_detail = pay.add_details();
				default_detail->set_amount(pay.amount());
				default_detail->set_start(0);
				default_detail->set_length(0);
				default_detail->set_ext("");
			}
			source_asset.mutable_details(i)->set_amount(source_asset.details(i).amount() - pay.amount());
			source_asset.set_amount(source_asset.amount() - pay.amount());
			sourceaccount_ptr_->SetAsset(source_asset);
			if (!dest_account->RecvAsset(pay, transaction_.GetHeader()->ledger_version())) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_ASSET_AMOUNT_TOO_LARGE);
				result_.set_desc("Dest account asset amount too large");
				LOG_ERROR("Dest account(%s) asset amount too large", dest_address.c_str());
				return false;
			}

			return true;
		}
		result_.set_code(protocol::ERRCODE_ACCOUNT_ASSET_LOW_RESERVE);
		result_.set_desc("source account asset not enough");
		LOG_ERROR("Source account(%s) asset not enough", source_address.c_str());
		return false;
	}

	int32_t InitPaymentOpeFrm::GetThreshold() {
		return sourceaccount_ptr_->GetProtoMedThreshold();
	}
}