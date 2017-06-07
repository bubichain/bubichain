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

#ifndef LEDGER_FRM_H_
#define LEDGER_FRM_H_

#include <utils/utils.h>
#include "transaction_frm.h"
#include "glue/transaction_set_frm.h"
#include "account.h"

namespace bubi {
	class AccountEntry;
	class LedgerFrm {
	public:
		typedef std::shared_ptr <LedgerFrm>	pointer;
		LedgerFrm();
		LedgerFrm(protocol::LedgerHeader header, TransactionSetFrm::TxSetByAccountSeq txset);
		void FromProtoLedger(protocol::Ledger &ledger);
		LedgerFrm(protocol::LedgerHeader &header);
		~LedgerFrm();
		static std::string CalculateTxTreeHash(const std::vector<TransactionFrm::pointer> &tx_array, const uint32_t ledger_version);
		protocol::LedgerHeader GetProtoHeader() const;
		protocol::LedgerHeader &ProtoHeader();
		bool PreAllpy(protocol::LedgerHeader lastheader, int64_t request_seq, TransactionSetFrmPtr tx_set);
		bool Apply(const protocol::LedgerHeader &lastheader);
		void SetHeaderHashs(const std::string &account_tree_hash, int64_t last_txcount);
		bool PreOk();
		void GetSqlTx(std::string &sqltx, std::string &sql_account_tx);
		bool AddToDb();
		bool LoadFromDb(int64_t seq);
		bool LoadFromDb(std::string seq, std::string num, Json::Value& value);
		static bool LoadFromDb(int64_t seq, protocol::Ledger &ledger);
		int64_t GetTxCount();
		int64_t GetNextId();
		bool CheckValidation();
		Json::Value ToJson();
	public:
		std::shared_ptr<AccountEntry> entry_;
		bool closed_;
		std::vector<TransactionFrm::pointer> apply_tx_frms_;
		int64_t next_requeset_seq_;
		std::map<int64_t, TransactionSetFrmPtr> request_buffer_;
	private:
		int64_t id_;
		std::vector<TransactionFrm::pointer> tmp_tx_frms_;
		protocol::LedgerHeader header_;
	};

	class GetSqlTask :public utils::Runnable {
	public:
		GetSqlTask(std::vector<TransactionFrm::pointer> &apply_tx_frms, std::size_t from_index, std::size_t to_index, std::string &sql_tx,
			std::string &sql_account_tx, int64_t ledger_seq, int64_t seq_in_global)
			:apply_tx_frms_(apply_tx_frms), from_index_(from_index), to_index_(to_index), sql_tx_(sql_tx), sql_account_tx_(sql_account_tx),
			ledger_seq_(ledger_seq), seq_in_global_(seq_in_global){}
		void Run(utils::Thread *this_thread) override;
	private:
		std::vector<TransactionFrm::pointer> &apply_tx_frms_;
		std::size_t from_index_;
		std::size_t to_index_;
		std::string &sql_tx_;
		std::string &sql_account_tx_;
		int64_t ledger_seq_;
		int64_t seq_in_global_;
	};
}
#endif //end of ifndef
