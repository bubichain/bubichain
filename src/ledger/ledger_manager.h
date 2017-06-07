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

#ifndef LEDGER_MANAGER_H_
#define LEDGER_MANAGER_H_

#include <utils/headers.h>
#include <common/general.h>
#include <common/storage.h>
#include <common/configure.h>
#include <common/private_key.h>
#include <overlay/peer.h>
#include <utils/entry_cache.h>
#include "ledger/ledger_fetch.h"
#include "ledger/ledger_frm.h"
#include "account_tree.h"

namespace bubi {
	class LedgerFetch;
	class WriteDbWorker :public utils::Runnable {
	public:
		WriteDbWorker();
		void WriteLedger(LedgerFrm::pointer pledger, std::string filename, WRITE_BATCH &batch);
		void Run(utils::Thread *this_thread) override;
	private:
		LedgerFrm::pointer pledger_;
		std::string filename_;
		WRITE_BATCH batch_;
		utils::Mutex mutex_;
	};
	class LedgerManager : public utils::Singleton<bubi::LedgerManager>,
		public bubi::TimerNotify,
		public bubi::StatusModule {
		friend class utils::Singleton<bubi::LedgerManager>;
		friend class LedgerFetch;
	public:
		bool Initialize();
		bool Exit();
		int OnConsent(int64_t checkpoint_seq, int64_t request_seq,
			const TransactionSetFrmPtr txSet, const protocol::Value &value, bool cacul_total);
		bool GetAccountEntry(const std::string &address, AccountFrm::pointer &account_entry);
		protocol::LedgerHeader GetLastClosedLedger();
		bool SyncComplete();
		int GetAccountNum();
		std::string GetAccountRootHash();
		void ManualFetch(int64_t begin, int64_t end, std::string source);
		void ReceiveGetLedgers(const PeerMessagePointer &message, int64_t peer_id);
		void OnReceiveLedgers(const PeerMessagePointer &message, int64_t peer_id);
		bool WriteTree();
		virtual void OnTimer(int64_t current_time) override;
		virtual void OnSlowTimer(int64_t current_time) override;
		virtual void GetModuleStatus(Json::Value &data);
	public:
		std::map<std::string, std::vector<std::string> > unique_asset_sql_;
		std::map<std::string, std::vector<std::string> > record_sql_;
		utils::Mutex mutex_;
		utils::Mutex gmutex_;
	private:
		LedgerManager();
		~LedgerManager();
		int64_t GetMaxLedger();
		void AddToDb(LedgerFrm::pointer pledger);
		void CloseLedger(LedgerFrm::pointer pledger, bool bconsent);
		bool Commit(AccountEntry &entry);
		void AsyncWriteRationalData();
		//load data when start
		bool LoadDataFromDb();
		protocol::LedgerHeader GetMaxConsentHeader();
		void SetMaxConsensusLedger(LedgerFrm &ledger);
		bool CreateTableIfNotExist();
		bool CreateGenesisAccount();
		LedgerFetch ledger_fetch_;
		utils::Thread* db_worker_thread_;
		WriteDbWorker db_worker_;
		AccountTree* tree_;
		LedgerFrm::pointer last_closed_ledger_;
		LedgerFrm::pointer tmp_ledger_;
		class BatchCache : public WRITE_BATCH::Handler {
		public:
			virtual ~BatchCache() {};
			virtual void Put(const SLICE &key, const SLICE &value) {
				save_[key.ToString()] = value.ToString();
			};
			virtual void Delete(const SLICE &key) {};
			void ClearRecord() {
				save_.clear();
			}
			bool Get(std::string key, std::string  &value) {
				utils::ReadLockGuard guard(mutex_);
				if (save_.find(key) != save_.end()) {
					return true;
				}
				return false;
			}
			size_t Size() {
				utils::ReadLockGuard guard(mutex_);
				return save_.size();
			}
			std::unordered_map<std::string, std::string> save_;
			utils::ReadWriteLock mutex_;
		};
	public:
		BatchCache batch_cache_;
	};

	struct StringSort {
		bool operator() (const std::string &l, const std::string &r) const {
			return l < r;
		}
	};

	class AccountEntry :public EntryCache<std::string, AccountFrm, StringSort> {
		virtual bool LoadValue(const std::string &address, AccountFrm::pointer &frm);
	public:
		AccountEntry();
		AccountEntry(AccountEntry* p);
	};
}
#endif
