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

#ifndef BUBI_ACCOUNT_H_
#define BUBI_ACCOUNT_H_

#include <utils/base_int.h>
#include <utils/crypto.h>
#include <utils/logger.h>
#include <proto/message.pb.h>
#include <utils/entry_cache.h>
#include "asset_frm.h"

namespace bubi {
	class AccountFrm {
	public:
		typedef std::shared_ptr<AccountFrm>	pointer;
		AccountFrm();
		AccountFrm(protocol::Account account);
		~AccountFrm();
		void ToJson(Json::Value &result);
		std::string Serializer();
		bool UnSerializer(const std::string &str);
		int64_t GetAccountBalance()const;
		int64_t GetPreviousLedgerSeq()const;
		std::string GetAccountAddress()const;
		bool AddBalance(int64_t amount);
		int64_t GetAssetAmount(const protocol::AssetProperty &asset_property);
		bool GetAsset(const protocol::AssetProperty &_property, protocol::Asset &result);
		void SetAsset(const protocol::Asset &input);
		static 	bool CompDetail(const protocol::Detail &l, const protocol::Detail &r);
		bool PayAsset(protocol::Asset &asset_para, int64_t close_time, uint32_t ledger_version);
		bool PayAsset2000(protocol::Asset &asset_para, int64_t close_time, uint32_t ledger_version);
		bool RecvAsset(protocol::Asset &asset__para, uint32_t ledger_version);
		bool PayUniqueAsset(protocol::UniqueAsset &unique_asset, int64_t close_time, int32_t version);
		bool RecvUniqueAsset(const protocol::UniqueAsset &unique_asset, int32_t version);
		void SetPreviousLedgerSeq(int64_t seq);
		void SetPreviousTxHash(const std::string &hash);
		protocol::Account &GetProtoAccount();
		int64_t GetAccountTxSeq() const;
		const protocol::AccountThreshold &GetProtoThreshold() const;
		const int32_t GetProtoMasterWeight() const;
		const int32_t GetProtoLowThreshold() const;
		const int32_t GetProtoMedThreshold() const;
		const int32_t GetProtoHighThreshold() const;
		void SetProtoMasterWeight(int32_t weight);
		void SetProtoLowThreshold(int32_t threshold);
		void SetProtoMedThreshold(int32_t threshold);
		void SetProtoHighThreshold(int32_t threshold);
		bool UpdateSigner(const std::string &signer, int32_t weight);
		bool LoadFromDb(const std::string &address);
	private:
		protocol::Account account_info_;
	};
}

#endif
