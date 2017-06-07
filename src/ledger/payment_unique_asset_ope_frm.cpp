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
#include "payment_unique_asset_ope_frm.h"

namespace bubi {
	PaymentUniqueAssetOpeFrm::PaymentUniqueAssetOpeFrm(const protocol::Operation &operation, TransactionFrm &tran, AccountFrm::pointer &account_ptr, uint32_t ope_seq) :
		OperationFrm(operation, tran, account_ptr), operation_payment_(operation.payment_unique_asset()), ope_seq_(ope_seq) {}

	PaymentUniqueAssetOpeFrm::~PaymentUniqueAssetOpeFrm() {}

	int32_t PaymentUniqueAssetOpeFrm::CheckValid(const protocol::Operation &ope, const std::string &source_address) {
		auto operation_payment_unique_asset = ope.payment_unique_asset();
		auto assetprop = operation_payment_unique_asset.asset_pro();

		if (assetprop.type() != protocol::AssetProperty_Type_UNIQUE) {
			LOG_ERROR("Asset type is not legal");
			return (protocol::ERRCODE_ASSET_INVALID);
		}

		if (!PublicKey::IsAddressValid(operation_payment_unique_asset.destaddress())) {
			LOG_ERROR("Destination address(%s) invalid", operation_payment_unique_asset.destaddress().c_str());
			return(protocol::ERRCODE_INVALID_ADDRESS);
		}

		//can't pay source addresss
		if (operation_payment_unique_asset.destaddress() == source_address) {
			LOG_ERROR("Destination address is equal source address");
			return(protocol::ERRCODE_ACCOUNT_SOURCEDEST_EQUAL);
		}

		if (assetprop.issuer().size() <= 0 ||
			assetprop.code().size() <= 0 ||
			assetprop.code().size() > ASSET_CODE_MAX_SIZE) {
			return protocol::ERRCODE_INVALID_PARAMETER;
		}

		return protocol::ERRCODE_SUCCESS;
	}

	void PaymentUniqueAssetOpeFrm::SourceRelationTx() {
		if (operation_.has_source_address()) {
			transaction_.account_tx_.insert(operation_.source_address());
		}
		transaction_.account_tx_.insert(operation_payment_.destaddress());
	}

	bool PaymentUniqueAssetOpeFrm::Apply(AccountEntry &entry) {
		do {
			// get latest account message
			std::string source_address = sourceaccount_ptr_->GetAccountAddress();

			AccountFrm::pointer dest_account;
			std::string dest_address = operation_payment_.destaddress();
			if (!entry.GetEntry(dest_address, dest_account)) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
				result_.set_desc("dest account does not exist");
				LOG_ERROR("Dest account(%s) does not exist", dest_address.c_str());
				break;
			}

			protocol::UniqueAsset unique_asset;
			protocol::AssetProperty *preperty = unique_asset.mutable_property();
			preperty->CopyFrom(operation_payment_.asset_pro());
			std::string asset_preperty = preperty->SerializeAsString();

			auto &unique_asset_tx = transaction_.unique_asset_;
			auto &unique_asset_ledger = bubi::LedgerManager::Instance().unique_asset_sql_;

			// when unique asset not exist,can't payment
			if (unique_asset_tx.find(asset_preperty) == unique_asset_tx.end() &&			// check Tx cache 
				unique_asset_ledger.find(asset_preperty) == unique_asset_ledger.end() &&	// check Ledger cache 
				!CheckUniqueAsset(*preperty)												// check DB
				) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_ASSET_LOW_RESERVE);
				LOG_ERROR("Unique Asset not exist! issuer(%s) code(%s)", preperty->issuer().c_str(), preperty->code().c_str());
				return false;
			}


			// source account pay unique asset
			protocol::LedgerHeader hd;
			hd.CopyFrom(*transaction_.GetHeader());
			if (!sourceaccount_ptr_->PayUniqueAsset(unique_asset, hd.consensus_value().close_time() / 1000000,
				hd.ledger_version())) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_ASSET_LOW_RESERVE);
				LOG_ERROR("Source account(%s): asset not enough", source_address.c_str());
				result_.set_desc("asset not enough");
				break;
			}

			// dest account add unique asset
			if (!dest_account->RecvUniqueAsset(unique_asset, hd.ledger_version())) {
				result_.set_code(protocol::ERRCODE_INTERNAL_ERROR);
				result_.set_desc("add asset failed");
				LOG_ERROR("Dest account(%s):add asset failed", dest_address.c_str());
				break;
			}

			Json::Value sql;
			sql["ope_seq"] = ope_seq_;
			sql["asset_issuer"] = unique_asset.property().issuer();
			sql["asset_code"] = unique_asset.property().code();
			sql["asset_detailed"] = utils::String::BinToHexString(unique_asset.detailed());
			sql["from_address"] = source_address;
			sql["to_address"] = dest_address;

			//add operation to Tx cache
			unique_asset_tx[asset_preperty].push_back(sql);
			return true;
		} while (false);
		return false;
	}

	int32_t PaymentUniqueAssetOpeFrm::GetThreshold() {
		return sourceaccount_ptr_->GetProtoMedThreshold();
	}

	bool PaymentUniqueAssetOpeFrm::CheckUniqueAsset(const protocol::AssetProperty asset_prop) {
		std::string asset_issuer = asset_prop.issuer();
		std::string asset_code = asset_prop.code();

		RationalDb *db = Storage::Instance().rational_db();


		std::string condition = " WHERE 1=1";
		std::string count_tb_name = "";
		condition += utils::String::Format(" AND %s.asset_issuer='%s'",
			General::UNIQUE_ASSET_NAME, db->Format(asset_issuer).c_str());

		condition += utils::String::Format(" AND %s.asset_code='%s'",
			General::UNIQUE_ASSET_NAME, db->Format(asset_code).c_str());

		count_tb_name = General::UNIQUE_ASSET_NAME;

		int64_t total_count = db->QueryCount(count_tb_name, condition);
		return total_count > 0 ? true : false;
	}
}