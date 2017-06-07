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

#include <common/storage.h>
#include <ledger/ledger_manager.h>
#include "account.h"

namespace bubi {
	AccountFrm::AccountFrm() {
		utils::AtomicInc(&bubi::General::account_new_count);
	}

	AccountFrm::AccountFrm(protocol::Account account_info)
		: account_info_(account_info) {
		utils::AtomicInc(&bubi::General::account_new_count);
	}

	AccountFrm::~AccountFrm() {
		utils::AtomicInc(&bubi::General::account_delete_count);
	}

	int64_t AccountFrm::GetAssetAmount(const protocol::AssetProperty &asset_property) {
		if (asset_property.type() == protocol::AssetProperty_Type_NATIVE) {
			return account_info_.account_balance();
		}
		else {
			for (int i = 0; i < account_info_.assets_size(); i++) {
				protocol::Asset asset = account_info_.assets(i);
				if (asset.property().SerializeAsString() == asset_property.SerializeAsString()) {
					return asset.amount();
				}
			}
			return -1;
		}
	}

	bool AccountFrm::GetAsset(const protocol::AssetProperty &_property, protocol::Asset &result) {
		for (int i = 0; i < account_info_.assets_size(); i++) {
			protocol::Asset asset = account_info_.assets(i);
			if (asset.property().SerializeAsString() == _property.SerializeAsString()) {
				result.CopyFrom(asset);
				return true;
			}
		}
		return false;
	}

	bool AccountFrm::CompDetail(const protocol::Detail &l, const protocol::Detail &r) {
		if (l.length() < 0) {
			return false;
		}
		else if (l.length() == 0) {
			if (r.length() < 0) {
				return true;
			}
			else if (r.length() == 0) {
				return l.ext() < r.ext();
			}
			else if (r.length() > 0) {
				return false;
			}
		}
		else if (l.length() > 0) {
			if (r.length() < 0) {
				return true;
			}
			else if (r.length() == 0) {
				return true;
			}
			else if (r.length() > 0) {
				int64_t a = l.length() + l.start() - r.length() - r.start();
				if (a == 0) {
					return l.ext() < r.ext();
				}
				else if (a < 0) {
					return true;
				}
				else if (a > 0) {
					return false;
				}
			}
		}
		return false;
	}

	void AccountFrm::SetPreviousLedgerSeq(int64_t seq) {
		account_info_.set_previous_ledger_seq(seq);
	}

	void AccountFrm::SetPreviousTxHash(const std::string &hash) {
		account_info_.set_previous_tx_hash(hash);
	}

	protocol::Account &AccountFrm::GetProtoAccount() {
		return account_info_;
	}

	int64_t AccountFrm::GetAccountTxSeq() const {
		return account_info_.tx_seq();
	}

	const protocol::AccountThreshold &AccountFrm::GetProtoThreshold() const {
		return account_info_.thresholds();
	}

	const int32_t AccountFrm::GetProtoMasterWeight() const {
		return account_info_.thresholds().master_weight();
	}

	const int32_t AccountFrm::GetProtoLowThreshold() const {
		return account_info_.thresholds().low_threshold();
	}

	const int32_t AccountFrm::GetProtoMedThreshold() const {
		return account_info_.thresholds().med_threshold();
	}

	const int32_t AccountFrm::GetProtoHighThreshold() const {
		return account_info_.thresholds().high_threshold();
	}

	void AccountFrm::SetProtoMasterWeight(int32_t weight) {
		return account_info_.mutable_thresholds()->set_master_weight(weight);
	}

	void AccountFrm::SetProtoLowThreshold(int32_t threshold) {
		return account_info_.mutable_thresholds()->set_low_threshold(threshold);
	}

	void AccountFrm::SetProtoMedThreshold(int32_t threshold) {
		return account_info_.mutable_thresholds()->set_med_threshold(threshold);
	}

	void AccountFrm::SetProtoHighThreshold(int32_t threshold) {
		return account_info_.mutable_thresholds()->set_high_threshold(threshold);
	}

	bool AccountFrm::AddBalance(int64_t amount) {
		int64_t tmp = this->account_info_.account_balance() + amount;
		if (tmp < 0)	return false;
		account_info_.set_account_balance(tmp);
		return true;
	}

	std::string AccountFrm::Serializer() {
		return account_info_.SerializeAsString();
	}

	bool AccountFrm::UnSerializer(const std::string &str) {
		if (!account_info_.ParseFromString(str)) {
			LOG_ERROR("Accountunserializer is erro!");
			return false;
		}
		return true;
	}

	int64_t AccountFrm::GetAccountBalance()const {
		return account_info_.account_balance();
	}

	int64_t AccountFrm::GetPreviousLedgerSeq()const {
		return account_info_.previous_ledger_seq();
	}


	std::string AccountFrm::GetAccountAddress()const {
		return account_info_.account_address();
	}

	bool AccountFrm::LoadFromDb(const std::string &address) {
		bubi::KeyValueDb  *db = bubi::Storage::Instance().keyvalue_db();

		std::string str_value;
		if (!db->Get(address, str_value)) {
			LOG_ERROR("Get address(%s) from database failed(%s)", address.c_str(),db->error_desc().c_str());
			return false;
		}
		return UnSerializer(str_value);
	}

	bool AccountFrm::UpdateSigner(const std::string &signer, int32_t weight) {
		weight = weight & UINT8_MAX;
		if (weight > 0) {
			bool found = false;
			for (int32_t i = 0; i < account_info_.signers_size(); i++) {
				if (account_info_.signers(i).address() == signer) {
					found = true;
					account_info_.mutable_signers(i)->set_weight(weight);
				}
			}

			if (!found) {
				if (account_info_.signers_size() >= protocol::Account_Limit_SIGNER) {
					return false;
				}

				protocol::Signer* signer1 = account_info_.add_signers();
				signer1->set_address(signer);
				signer1->set_weight(weight);
			}
		}
		else {
			bool found = false;
			std::vector<std::pair<std::string, int32_t> > nold;
			for (int32_t i = 0; i < account_info_.signers_size(); i++) {
				if (account_info_.signers(i).address() != signer) {
					nold.push_back(std::make_pair(account_info_.signers(i).address(), account_info_.signers(i).weight()));
				}
				else {
					found = true;
				}
			}

			if (found) {
				account_info_.clear_signers();
				for (size_t i = 0; i < nold.size(); i++) {
					protocol::Signer* signer = account_info_.add_signers();
					signer->set_address(nold[i].first);
					signer->set_weight(nold[i].second);
				}
			}
		}

		return true;
	}

	bool AccountFrm::RecvAsset(protocol::Asset &asset_para, uint32_t ledger_version) {
		for (int x = 0; x < asset_para.details_size(); x++) {
			if (asset_para.mutable_details(x)->length() == 0) {
				asset_para.mutable_details(x)->set_start(0);
			}
			if (ledger_version > 1000) {
				if (asset_para.mutable_details(x)->length() < 0) {
					asset_para.mutable_details(x)->set_ext("");
					asset_para.mutable_details(x)->set_start(0);
				}
			}
		}

		protocol::Asset *asset = nullptr;
		for (int i = 0; i < account_info_.assets_size(); i++) {
			if (account_info_.mutable_assets(i)->property().SerializeAsString() ==
				asset_para.property().SerializeAsString()) {
				asset = account_info_.mutable_assets(i);
				break;
			}
		}

		if (asset == nullptr) {
			asset = account_info_.add_assets();
			asset->CopyFrom(asset_para);
			return true;
		}

		for (int i = 0; i < asset_para.details_size(); i++) {
			const protocol::Detail &item = asset_para.details(i);
			bool bexist = false;
			protocol::Detail *p = nullptr;
			for (int j = 0; j < asset->details_size(); j++) {
				p = asset->mutable_details(j);
				if (item.length() == p->length() && (
					(item.ext() == p->ext() && item.start() == p->start() && item.length()>0) ||
					(p->length() < 0) ||
					(p->length() == 0 && p->ext() == item.ext())
					)
					) {
					bexist = true;
					break;
				}
			}
			if (!bexist) {
				p = asset->add_details();
				p->set_ext(item.ext());
				p->set_start(item.start());
				p->set_length(item.length());
				p->set_amount(0);
			}
			int64_t add = p->amount() + item.amount();
			if (add < 0) {
				return false;
			}
			else {
				p->set_amount(add);
			}
			int64_t amount_total = asset->amount() + item.amount();
			if (amount_total <= 0)
				return false;
			else
				asset->set_amount(asset->amount() + item.amount());
		}
		return true;
	}

	bool AccountFrm::PayAsset2000(protocol::Asset &asset_para, int64_t close_time, uint32_t ledger_version) {

		if (ledger_version > 1000) {
			for (int i = 0; i < asset_para.details_size(); i++) {
				if (asset_para.details(i).length() <= 0) {
					asset_para.mutable_details(i)->set_start(0);
				}
				if (asset_para.details(i).length() <= -1) {
					asset_para.mutable_details(i)->set_ext("");
				}

				int64_t start = asset_para.details(i).start();
				int64_t  length = asset_para.details(i).length();
				if ((start > close_time || close_time > start + length) && length > 0) {
					return false;
				}
			}
		}

		protocol::Asset asset_source;
		int  asset_index = -1;
		for (int i = 0; i < account_info_.assets_size(); i++) {
			if (account_info_.assets(i).property().SerializeAsString() == asset_para.property().SerializeAsString()) {
				asset_index = i;
				asset_source.CopyFrom(account_info_.assets(i));
				break;
			}
		}
		if (asset_index < 0) {
			return false;
		}

		if (asset_para.details_size() == 0) {

			std::vector<protocol::Detail> vector_sort;
			for (int j = 0; j < asset_source.details_size(); j++)
				vector_sort.push_back(asset_source.details(j));

			std::sort(vector_sort.begin(), vector_sort.end(), CompDetail);
			int64_t need_amount = asset_para.amount();
			for (std::size_t i = 0; i < vector_sort.size(); i++) {
				if (need_amount <= 0) {
					break;
				}
				protocol::Detail d = vector_sort[i];

				protocol::Detail* new_detail = asset_para.add_details();
				new_detail->CopyFrom(d);
				if (d.amount() < need_amount) {
					need_amount -= d.amount();
				}
				else {
					new_detail->set_amount(need_amount);
					need_amount = 0;
					break;
				}
			}
			if (need_amount > 0) {
				return false;
			}
		}

		std::vector<protocol::Detail> vdetails;
		for (int j = 0; j < asset_source.details_size(); j++)
			vdetails.push_back(asset_source.details(j));

		for (int i = 0; i < asset_para.details_size(); i++) {
			const protocol::Detail &item = asset_para.details(i);
			bool bexist = false;

			for (std::size_t j = 0; j < vdetails.size(); j++) {
				protocol::Detail &p = vdetails[j];
				if (item.length() == p.length() &&
					(
					(item.ext() == p.ext() && item.start() == p.start() && item.length()>0) ||
					(item.length() < 0) ||
					(item.ext() == p.ext() && item.length() == 0)
					)
					) {
					if (p.length() > 0 &&
						(p.length() + p.start() < close_time || p.start() > close_time)
						) {
						return false;
					}

					if (p.amount() - item.amount() < 0) {
						return false;
					}
					else {
						p.set_amount(p.amount() - item.amount());
					}
					bexist = true;
					break;
				}
			}
			if (!bexist) {
				return false;
			}
		}

		asset_source.clear_details();
		asset_source.set_amount(0);
		int64_t total_amount = 0;
		for (std::size_t i = 0; i < vdetails.size(); i++) {
			if (vdetails[i].amount() > 0) {
				protocol::Detail *new_d = asset_source.add_details();
				new_d->CopyFrom(vdetails[i]);
				total_amount += new_d->amount();
			}
		}
		asset_source.set_amount(total_amount);
		if (total_amount > 0)
			account_info_.mutable_assets(asset_index)->CopyFrom(asset_source);
		else {

			protocol::Account tmp = account_info_;
			account_info_.clear_assets();
			for (int i = 0; i < tmp.assets_size(); i++) {
				if (i != asset_index) {
					account_info_.add_assets()->CopyFrom(tmp.assets(i));
				}
			}
			account_info_.assets();
		}
		return true;
	}

	bool AccountFrm::PayAsset(protocol::Asset &asset_para, int64_t close_time, uint32_t ledger_version) {
		if (ledger_version > 1000) {
			for (int i = 0; i < asset_para.details_size(); i++) {
				if (asset_para.details(i).length() <= 0) {
					asset_para.mutable_details(i)->set_start(0);
				}

				if (asset_para.details(i).length() <= -1) {
					asset_para.mutable_details(i)->set_ext("");
				}

				int64_t start = asset_para.details(i).start();
				int64_t  length = asset_para.details(i).length();
				if ((start > close_time || close_time > start + length) && length > 0) {
					return false;
				}
			}
		}

		if (ledger_version >= 2000) {
			return PayAsset2000(asset_para, close_time, ledger_version);
		}

		protocol::Asset asset_source;
		int  asset_index = -1;
		for (int i = 0; i < account_info_.assets_size(); i++) {
			if (account_info_.assets(i).property().SerializeAsString() == asset_para.property().SerializeAsString()) {
				asset_index = i;
				asset_source.CopyFrom(account_info_.assets(i));
				break;
			}
		}
		if (asset_index < 0) {
			return false;
		}

		if (asset_para.details_size() == 0) {

			std::vector<protocol::Detail> vector_sort;
			for (int j = 0; j < asset_source.details_size(); j++)
				vector_sort.push_back(asset_source.details(j));

			std::sort(vector_sort.begin(), vector_sort.end(), CompDetail);
			int64_t need_amount = asset_para.amount();
			for (std::size_t i = 0; i < vector_sort.size(); i++) {
				if (need_amount <= 0) {
					break;
				}
				protocol::Detail d = vector_sort[i];

				if (d.length() > 0 && (d.start() + d.length() < close_time || d.start() > close_time))
					continue;

				protocol::Detail* new_detail = asset_para.add_details();
				new_detail->CopyFrom(d);
				if (d.amount() < need_amount) {

					need_amount -= d.amount();
				}
				else {
					new_detail->set_amount(need_amount);
					need_amount = 0;
					break;
				}
			}
			if (need_amount > 0) {
				return false;
			}
		}

		std::vector<protocol::Detail> vdetails;
		for (int j = 0; j < asset_source.details_size(); j++)
			vdetails.push_back(asset_source.details(j));

		for (int i = 0; i < asset_para.details_size(); i++) {
			const protocol::Detail &item = asset_para.details(i);
			bool bexist = false;

			for (std::size_t j = 0; j < vdetails.size(); j++) {
				protocol::Detail &p = vdetails[j];
				if (item.length() == p.length() &&
					(
					(item.ext() == p.ext() && item.start() == p.start() && item.length()>0) ||
					(item.length() < 0) ||
					(item.ext() == p.ext() && item.length() == 0)
					)
					) {
					if (p.length() > 0 &&
						(p.length() + p.start() < close_time || p.start() > close_time)
						) {
						return false;
					}

					if (p.amount() - item.amount() < 0) {
						return false;
					}
					else {
						p.set_amount(p.amount() - item.amount());
					}
					bexist = true;
					break;
				}
			}
			if (!bexist) {
				return false;
			}
		}

		asset_source.clear_details();
		asset_source.set_amount(0);
		int64_t total_amount = 0;
		for (std::size_t i = 0; i < vdetails.size(); i++) {
			if (vdetails[i].amount() > 0) {
				protocol::Detail *new_d = asset_source.add_details();
				new_d->CopyFrom(vdetails[i]);
				total_amount += new_d->amount();
			}
		}
		asset_source.set_amount(total_amount);
		if (total_amount > 0)
			account_info_.mutable_assets(asset_index)->CopyFrom(asset_source);
		else {

			account_info_.mutable_assets()->DeleteSubrange(asset_index, 1);

		}
		return true;
	}

	void AccountFrm::ToJson(Json::Value &result) {
		Json::Value assets = Json::Value(Json::arrayValue);
		for (int i = 0; i < account_info_.assets_size(); i++) {
			Json::Value item;
			protocol::Asset ast = account_info_.assets(i);
			AssetFrm::ToJson(ast, item);

			assets[i] = item;
		}
		Json::Value unique_assets = Json::Value(Json::arrayValue);
		for (int i = 0; i < account_info_.unique_asset_size(); i++) {
			Json::Value item;
			protocol::UniqueAsset ast = account_info_.unique_asset(i);
			item["detailed"] = utils::String::BinToHexString(ast.detailed());
			item["issuer"] = ast.property().issuer();
			item["code"] = ast.property().code();
			item["type"] = ast.property().type();
			unique_assets[i] = item;
		}
		result["address"] = account_info_.account_address();
		result["balance"] = GetAccountBalance();
		result["assets"] = assets;
		result["unique_assets"] = unique_assets;
		result["previous_ledger_seq"] = GetPreviousLedgerSeq();
		result["tx_seq"] = account_info_.tx_seq();
		result["previous_tx_hash"] = utils::String::BinToHexString(account_info_.previous_tx_hash());
		result["hash"] = utils::String::BinToHexString(utils::Sha256::Crypto(Serializer()));
		result["last_close_time"] = LedgerManager::Instance().GetLastClosedLedger().consensus_value().close_time()
			/ utils::MICRO_UNITS_PER_SEC;
		result["metadata"] = utils::String::BinToHexString(account_info_.metadata());
		result["metadata_version"] = account_info_.has_metadata_version() ? account_info_.metadata_version() : 1;
		Json::Value &signers = result["signers"];
		signers = Json::Value(Json::arrayValue);
		for (int32_t i = 0; i < account_info_.signers_size(); i++) {
			Json::Value &signer_item = signers[i];
			signer_item["address"] = account_info_.signers(i).address();
			signer_item["weight"] = account_info_.signers(i).weight();
		}

		Json::Value &threshold = result["threshold"];
		const protocol::AccountThreshold &threshold_acc = account_info_.thresholds();
		threshold["master_weight"] = threshold_acc.master_weight();
		threshold["low_threshold"] = threshold_acc.low_threshold();
		threshold["med_threshold"] = threshold_acc.med_threshold();
		threshold["high_threshold"] = threshold_acc.high_threshold();
	}

	void AccountFrm::SetAsset(const protocol::Asset &input) {
		int index = -1;
		for (int i = 0; i < account_info_.assets_size(); i++) {
			if (account_info_.assets(i).property().SerializeAsString() == input.property().SerializeAsString()) {
				index = i;
				break;
			}
		}
		if (index == -1) {
			account_info_.add_assets();
			index = account_info_.assets_size() - 1;
		}

		if (input.amount() > 0)
			account_info_.mutable_assets(index)->CopyFrom(input);
		else {
			protocol::Account tmp;
			tmp.CopyFrom(account_info_);
			account_info_.clear_assets();
			for (int i = 0; i < tmp.assets_size(); i++) {
				if (i == index)
					continue;
				account_info_.add_assets()->CopyFrom(tmp.assets(i));
			}
		}
	}

	bool AccountFrm::PayUniqueAsset(protocol::UniqueAsset &unique_asset, int64_t close_time, int32_t version) {
		std::string s = unique_asset.property().SerializeAsString();
		for (int i = 0; i < account_info_.unique_asset_size(); i++) {
			if (account_info_.unique_asset(i).property().SerializeAsString() == s) {
				unique_asset.set_detailed(account_info_.unique_asset(i).detailed());
				account_info_.mutable_unique_asset()->DeleteSubrange(i, 1);
				return true;
			}
		}
		return false;
	}

	bool AccountFrm::RecvUniqueAsset(const protocol::UniqueAsset &unique_asset, int32_t version) {
		account_info_.mutable_unique_asset()->Add()->CopyFrom(unique_asset);
		return true;
	}
}
