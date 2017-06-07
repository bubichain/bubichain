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
#include "payment_ope_frm.h"
#include "asset_frm.h"

namespace bubi {
	PaymentOpeFrm::PaymentOpeFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr) :
		OperationFrm(operation, tran, account_ptr), operation_payment_(operation.payment()) {}

	PaymentOpeFrm::~PaymentOpeFrm() {}

	int32_t PaymentOpeFrm::CheckValid(const protocol::Operation &ope, const std::string &source_address) {
		auto operation_payment = ope.payment();

		protocol::Asset asset = operation_payment.asset();
		if (asset.amount() <= 0) {
			LOG_ERROR("Asset is not legal");
			return(protocol::ERRCODE_ASSET_INVALID);
		}

		if (operation_payment.asset().property().type() == protocol::AssetProperty_Type_IOU) {
			if (!AssetFrm::IsValid(asset)) {
				LOG_ERROR("Asset is not legal");
				return(protocol::ERRCODE_ASSET_INVALID);
			}
		}
		else if (operation_payment.asset().property().type() == protocol::AssetProperty_Type_NATIVE) {
		}
		else {
			LOG_ERROR("Asset's type is not legal");
			return(protocol::ERRCODE_ASSET_INVALID);
		}
		if (!PublicKey::IsAddressValid(operation_payment.destaddress())) {
			LOG_ERROR("Destination address(%s) invalid", operation_payment.destaddress().c_str());
			return(protocol::ERRCODE_INVALID_ADDRESS);
		}

		if (operation_payment.destaddress() == source_address) {
			LOG_ERROR("Destination address is equal to source address");
			return(protocol::ERRCODE_ACCOUNT_SOURCEDEST_EQUAL);
		}
		return protocol::ERRCODE_SUCCESS;
	}


	void PaymentOpeFrm::SourceRelationTx() {
		if (operation_.has_source_address()) {
			transaction_.account_tx_.insert(operation_.source_address());
		}
		transaction_.account_tx_.insert(operation_payment_.destaddress());
	}

	bool PaymentOpeFrm::Apply(AccountEntry &entry) {
		std::string source_address = sourceaccount_ptr_->GetAccountAddress();
		if (!entry.GetEntry(source_address, sourceaccount_ptr_)) {
			LOG_ERROR("Source account does not exist");
			result_.set_code(protocol::ERRCODE_INTERNAL_ERROR);
			result_.set_desc("Source account does not exist");
			return false;
		}

		protocol::AssetProperty_Type pay_type = operation_payment_.asset().property().type();

		AccountFrm::pointer dest_account;
		std::string dest_address = operation_payment_.destaddress();
		if (!entry.GetEntry(dest_address, dest_account)) {
			dest_account = std::make_shared<AccountFrm>();
			if (operation_payment_.asset().amount() >= transaction_.GetHeader()->base_reserve()
				&& (pay_type == protocol::AssetProperty_Type_NATIVE)
				&& transaction_.GetHeader()->ledger_version() < 2000) {

				protocol::Account &proto_account = dest_account->GetProtoAccount();
				proto_account.set_account_address(operation_payment_.destaddress());
				proto_account.set_account_balance(0);
				proto_account.set_metadata("");
				proto_account.set_previous_ledger_seq(transaction_.GetLedgerSeq());
				proto_account.set_previous_tx_hash("");
				proto_account.set_tx_seq(transaction_.GetLedgerSeq() << 32);
				proto_account.set_previous_tx_hash(transaction_.GetContentHash());
				proto_account.clear_assets();
				proto_account.clear_signers();
				proto_account.clear_unique_asset();

				dest_account->SetProtoMasterWeight(1);
				dest_account->SetProtoLowThreshold(1);
				dest_account->SetProtoMedThreshold(1);
				dest_account->SetProtoHighThreshold(1);
				entry.AddEntry(operation_payment_.destaddress(), dest_account);
			}
			else {
				result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
				result_.set_desc("Destination account does not exist");
				LOG_ERROR("Destination account(%s) does not exist", dest_address.c_str());
				return false;
			}
		}


		if (pay_type == protocol::AssetProperty_Type_NATIVE) {
			if (sourceaccount_ptr_->GetAccountBalance() - operation_payment_.asset().amount() < (int64_t)transaction_.GetHeader()->base_reserve()) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_LOW_RESERVE);
				result_.set_desc("Source account low reserve");
				LOG_ERROR("Source account(%s) low reserve", source_address.c_str());
				return false;
			}
			else {
				dest_account->AddBalance(operation_payment_.asset().amount());
				sourceaccount_ptr_->AddBalance(-1 * operation_payment_.asset().amount());
				return true;
			}
		}
		else if (operation_payment_.asset().property().type() == protocol::AssetProperty_Type::AssetProperty_Type_IOU) {
			protocol::Asset need_pay = operation_payment_.asset();

			protocol::LedgerHeader hd;
			hd.CopyFrom(*transaction_.GetHeader());
			if (!sourceaccount_ptr_->PayAsset(need_pay, hd.consensus_value().close_time() / 1000000,
				hd.ledger_version())) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_ASSET_LOW_RESERVE);
				LOG_ERROR("Source account(%s): asset not enough", source_address.c_str());
				result_.set_desc("asset not enough");
				return false;
			}

			if (!dest_account->RecvAsset(need_pay, hd.ledger_version())) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_ASSET_AMOUNT_TOO_LARGE);
				result_.set_desc("Destination account asset amount too large");
				LOG_ERROR("Destination account(%s) asset amount too large", dest_address.c_str());
				return false;
			}
			return true;
		}
		else {
			return false;
		}
	}

	int32_t PaymentOpeFrm::GetThreshold() {
		return sourceaccount_ptr_->GetProtoMedThreshold();
	}
}