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

#ifndef TRANSACTION_FRM_H_
#define TRANSACTION_FRM_H_

#include <proto/message.pb.h>
#include <utils/common.h>
#include <common/general.h>
#include <ledger/account.h>
#include <overlay/peer.h>
#include <api/web_server.h>
#include "operation_frm.h"

namespace bubi {
	class OperationFrm;
	class AccountEntry;
	class TransactionFrm {
	public:
		typedef std::shared_ptr<bubi::TransactionFrm> pointer;
		TransactionFrm();
		TransactionFrm(const protocol::TransactionEnv &env);
		TransactionFrm(const protocol::TransactionEnvWrapper &env_wrapper, const protocol::SlaveVerifyResponse &sv_rsp);
		virtual ~TransactionFrm();
		std::string GetContentHash() const;
		std::string GetContentData() const;
		std::string GetFullHash() const;
		void ToJson(Json::Value &json);
		std::string GetSourceAddress() const;
		int64_t GetSequnce() const;
		AccountFrm::pointer GetSourceAccount() const;
		const protocol::TransactionEnv &GetTransactionEnv() const;
		bool CheckValid(int64_t last_seq);
		bool ValidForApply(std::shared_ptr<protocol::LedgerHeader> header, AccountEntry &entry);
		bool VerifyWeight(AccountFrm::pointer, int32_t needed_weight) const;
		const protocol::Transaction &GetTx() const;
		Result GetResult() const;
		float GetFeeRatio() const;
		uint32_t GetFee() const;
		uint32_t GetMinFee() const;
		void Initialize();
		uint32_t LoadFromDb(const std::string &hash);
		void GetSql(int64_t ledger_seq, int64_t seq_in_global, std::string &sql, std::string &account_tx, std::string unique_asset);
		bool CheckTimeout(int64_t expire_time);
		void PayFee();
		bool Apply(std::shared_ptr<protocol::LedgerHeader> header, AccountEntry &entry);
		protocol::TransactionEnv &GetProtoTxEnv();
		int64_t GetLedgerSeq();
		void SetHeader(std::shared_ptr<protocol::LedgerHeader> header);
		const std::shared_ptr<protocol::LedgerHeader> GetHeader() const;
	private:
		bool CheckCommon();
	public:
		AccountFrm::pointer account_;
		std::set<std::string> account_tx_;
		std::map<std::string, std::vector<Json::Value>> unique_asset_;
		std::map<std::string, std::list<Json::Value>> records_;
		int64_t suggestion_seq_;
		int64_t time_use_;
		//only valid when the transaction belongs to a txset
		std::string txset_hash_;
		uint64_t apply_time_;
		Result result_;
	private:
		protocol::TransactionEnv transaction_env_;
		std::string hash_;
		std::string full_hash_;
		std::string data_;
		std::string full_data_;
		bool check_valid_;
		std::set<std::string> valid_signature_;
		std::string insert_sql_;
		std::shared_ptr<protocol::LedgerHeader> header_;
		int64_t incoming_time_;
	};
};

#endif