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

#include <overlay/peer_manager.h>
#include <glue/glue_manager.h>
#include <slave/master_service.h>
#include "ledger_manager.h"

namespace bubi {
	LedgerManager::LedgerManager() : tree_(NULL) {
		check_interval_ = 500 * utils::MICRO_UNITS_PER_MILLI;
		timer_name_ = "Ledger Mananger";
	}

	LedgerManager::~LedgerManager() {
		if (tree_) {
			delete tree_;
			tree_ = NULL;
		}
	}

	protocol::LedgerHeader LedgerManager::GetLastClosedLedger() {
		utils::MutexGuard guard(gmutex_);
		return last_closed_ledger_->GetProtoHeader();
	}

	int LedgerManager::GetAccountNum() {
		utils::MutexGuard guard(gmutex_);
		return tree_->LeafCount();
	}

	std::string LedgerManager::GetAccountRootHash() {
		utils::MutexGuard guard(gmutex_);
		return tree_->RootHash();
	}

	void LedgerManager::ManualFetch(int64_t begin, int64_t end, std::string source) {
		utils::MutexGuard guard(gmutex_);
		ledger_fetch_.ManualFetch(begin, end, source);
	}

	void LedgerManager::ReceiveGetLedgers(const PeerMessagePointer &message, int64_t peer_id) {
		utils::Semaphore sem;
		bubi::Global::Instance().GetIoService().post([&, this]() {
			ledger_fetch_.ReceiveGetLedgers(message, peer_id);
			sem.Signal();
		});
		sem.Wait();
	}

	void LedgerManager::OnReceiveLedgers(const PeerMessagePointer &message, int64_t peer_id) {
		utils::Semaphore sem;
		bubi::Global::Instance().GetIoService().post([&, this]() {
			ledger_fetch_.OnReceiveLedgers(message, peer_id);
			sem.Signal();
		});
		sem.Wait();
	}

	bool LedgerManager::WriteTree() {
		bool bstat = Storage::Instance().keyvalue_db()->WriteBatch(tree_->batch_);
		tree_->Record();
		return bstat;
	}

	bool LedgerManager::Initialize() {
		tree_ = new AccountTree();
		if (!CreateTableIfNotExist())
			return false;

		if (!LoadDataFromDb()) {
			return false;
		}

		int64_t max_seq = GetMaxLedger();

		LOG_INFO("max closed ledger seq=" FMT_I64, max_seq);
		last_closed_ledger_ = std::make_shared<LedgerFrm>();
		if (!last_closed_ledger_->LoadFromDb(max_seq)) {
			return false;
		}

		if (!tree_->LoadFromDb()) {
			return false;
		}
		if (last_closed_ledger_->GetProtoHeader().account_tree_hash() != tree_->RootHash()) {
			LOG_ERROR("ledger account_tree_hash(%s)!=account_root_hash(%s)",
				utils::String::Bin4ToHexString(last_closed_ledger_->GetProtoHeader().account_tree_hash()).c_str(),
				utils::String::Bin4ToHexString(tree_->RootHash()).c_str());
			return false;
		}

		ledger_fetch_.max_consensus_ledger_ = std::make_shared<LedgerFrm>(*last_closed_ledger_.get());
		ledger_fetch_.Initialize();

		//   TOSYNCKEYVALUE
		if (Configure::Instance().db_configure_.async_write_kv_) {
			db_worker_thread_ = new utils::Thread(&db_worker_);
			if (!db_worker_thread_->Start("db_worker")) {
				return false;
			}
		}
		TimerNotify::RegisterModule(this);
		StatusModule::RegisterModule(this);
		return true;
	}

	bool LedgerManager::Exit() {
		if (db_worker_thread_) {
			db_worker_thread_->JoinWithStop();
			delete db_worker_thread_;
			db_worker_thread_ = NULL;
		}
		if (tree_) {
			delete tree_;
			tree_ = NULL;
		}
		return true;
	}

	bool LedgerManager::SyncComplete() {
		utils::MutexGuard guard(gmutex_);
		return ledger_fetch_.max_consensus_ledger_->GetProtoHeader().ledger_sequence() == last_closed_ledger_->GetProtoHeader().ledger_sequence();
	}

	void LedgerManager::OnTimer(int64_t current_time) {
		utils::MutexGuard guard(gmutex_);
		ledger_fetch_.OnTimer();
	}

	void LedgerManager::OnSlowTimer(int64_t current_time) {
		AsyncWriteRationalData();
	}

	void LedgerManager::AsyncWriteRationalData() {
		std::string path = Configure::Instance().db_configure_.tmp_path_;
		utils::FileAttributes nlist;
		{
			utils::MutexGuard guard(mutex_);
			utils::File::GetFileList(path, nlist);
		}
		auto sql_db = Storage::Instance().rational_db();
		for (auto it = nlist.begin(); it != nlist.end(); ++it) {
			if (it->first.rfind(General::RATIONAL_TMPDB_POSTFIX) != std::string::npos) {
				std::string filename = utils::String::Format("%s/%s", path.c_str(), it->first.c_str());
				utils::File f;
				if (!f.Open(filename, utils::File::FILE_OPEN_MODE::FILE_M_READ)) {
					BUBI_EXIT("filename:%s,error_code:%d,error_desc:%s", filename.c_str(), utils::error_code(), utils::error_desc().c_str());
				}

				if (!sql_db->Execute("begin;")) {
					BUBI_EXIT("excute sql fail\n begin;:%d:%s", sql_db->error_code(), sql_db->error_desc());
				}

				int64_t t1, t2, t3, t4;
				t1 = utils::Timestamp::HighResolution();
				std::string sql = "";


				if (f.ReadData(sql, 1024 * 1024 * 1024) < 1024 * 1024 * 1024) {
					t2 = utils::Timestamp::HighResolution();
					if (!sql_db->Execute(sql)) {
						BUBI_EXIT("excute sql fail\n %s:%d:%s", sql.c_str(), sql_db->error_code(), sql_db->error_desc());
					}
				}
				else {
					BUBI_EXIT("sql file size(" FMT_I64 ") larger than 1G", utils::File::GetAttribue(filename).size_);
				}

				t3 = utils::Timestamp::HighResolution();

				if (!sql_db->Execute("commit;")) {
					BUBI_EXIT("excute sql fail\n commit:%d:%s", sql_db->error_code(), sql_db->error_desc());
				}
				t4 = utils::Timestamp::HighResolution();
				LOG_INFO("%s,readdata,excute,commit:" FMT_I64 "," FMT_I64 "," FMT_I64,
					it->first.c_str(),
					t2 - t1, t3 - t2, t4 - t3);
				f.Close();
				if (!utils::File::Delete(filename)) {
					BUBI_EXIT("%d:%s", utils::error_code(), utils::error_desc().c_str());
				}
			}
		}
	}

	bool LedgerManager::LoadDataFromDb() {
		std::string path = Configure::Instance().db_configure_.tmp_path_;
		utils::FileAttributes nlist;
		{
			utils::MutexGuard guard(mutex_);
			utils::File::GetFileList(path, nlist);
		}

		// load kv form file
		auto kv_db = Storage::Instance().keyvalue_db();
		for (auto it = nlist.begin(); it != nlist.end(); ++it) {

			if (it->first.rfind(General::KEYVALUE_TMPDB_POSTFIX) != std::string::npos) {
				std::string filename = utils::String::Format("%s/%s", path.c_str(), it->first.c_str());
				utils::File f;
				if (!f.Open(filename, utils::File::FILE_OPEN_MODE::FILE_M_READ)) {
					BUBI_EXIT("filename:%s,error_code:%d,error_desc:%s", filename.c_str(), utils::error_code(), utils::error_desc().c_str());
				}
				std::string data;
				f.ReadData(data, 0x0fffffff);
				f.Close();
				WRITE_BATCH batch;
				memcpy(&batch, data.c_str(), data.size());

				if (!kv_db->WriteBatch(batch)) {
					BUBI_EXIT("%s", kv_db->error_desc().c_str());
				}
				if (!utils::File::Delete(filename)) {
					BUBI_EXIT("%d:%s", utils::error_code(), utils::error_desc().c_str());
				}
			}
		}
		//load sql ledger seq
		int64_t seq_sql = GetMaxLedger();

		//load kv ledger seq 
		auto kvdb = Storage::Instance().keyvalue_db();
		std::string str_max_seq;
		int64_t seq_kvdb = 0;
		if (kvdb->Get(General::KEY_LEDGER_SEQ, str_max_seq)) {
			seq_kvdb = utils::String::Stoi64(str_max_seq);

			//make sure kvdb >= sql
			if (seq_kvdb < seq_sql) {
				LOG_ERROR(
					"ledger_seq from k/vdb(" FMT_I64 ") smaller than rational db(" FMT_I64 ")!\n "
					"that means the blocks had been writen to the database but did not update account data.\n"
					"please deal with it manually.",
					seq_kvdb, seq_sql);
				return false;
			}
		}
		//kv seq not exist,may be 1.0 . downward compatibility
		else if (seq_sql != 0) {
			LOG_INFO("kv ledger_seq not exist and rational db(" FMT_I64 ")!,may be 1.0 date", seq_sql);
			return true;
		}
		// seq_kvdb == 0  && seq_sql == 0. db is empty
		else {

			if (!CreateGenesisAccount()) {
				LOG_ERROR("Create genesis account failed");
				return false;
			}
			//update seq
			seq_kvdb = 1;
			seq_sql = GetMaxLedger();
		}

		//load sql form file
		std::string name;
		auto sql_db = Storage::Instance().rational_db();
		for (int64_t i = seq_sql + 1; i <= seq_kvdb; i++) {
			std::string filename = utils::String::Format("%s/block_" FMT_I64 "%s", path.c_str(), i, General::RATIONAL_TMPDB_POSTFIX);
			utils::File f;
			if (!f.Open(filename, utils::File::FILE_OPEN_MODE::FILE_M_READ)) {
				BUBI_EXIT("Filename:%s,error_code:%d,error_desc:%s", filename.c_str(), utils::error_code(), utils::error_desc().c_str());
			}

			if (!sql_db->Execute("begin;")) {
				BUBI_EXIT("Execute sql fail\n begin;:%d:%s", sql_db->error_code(), sql_db->error_desc());
			}


			int64_t t1, t2, t3, t4;
			t1 = utils::Timestamp::HighResolution();
			std::string sql = "";
			if (f.ReadData(sql, utils::MAX_INT32)) {
				t2 = utils::Timestamp::HighResolution();
				if (!sql_db->Execute(sql)) {
					BUBI_EXIT("Execute sql fail\n %s:%d:%s", sql.c_str(), sql_db->error_code(), sql_db->error_desc());
				}
			}
			t3 = utils::Timestamp::HighResolution();
			if (!sql_db->Execute("commit;")) {
				BUBI_EXIT("Execute sql fail\n commit:%d:%s", sql_db->error_code(), sql_db->error_desc());
			}
			t4 = utils::Timestamp::HighResolution();
			LOG_INFO("readdata,Excute,commit:" FMT_I64 "," FMT_I64 "," FMT_I64, t2 - t1, t3 - t2, t4 - t3);
			f.Close();
			if (!utils::File::Delete(filename)) {
				BUBI_EXIT_ERRNO("Delete file(%s) failed", filename.c_str(), STD_ERR_CODE, STD_ERR_DESC);
			}
		}

		//delete surplus sql file
		seq_sql = seq_kvdb + 1;
		while (nlist.find(name) != nlist.end()) {
			std::string filename = utils::String::Format("%s/block_" FMT_I64 "%s", path.c_str(), seq_sql, General::RATIONAL_TMPDB_POSTFIX);
			if (!utils::File::Delete(filename)) {
				BUBI_EXIT("%d:%s", utils::error_code(), utils::error_desc().c_str());
			}
			seq_sql++;
		}
		return true;
	}

	protocol::LedgerHeader LedgerManager::GetMaxConsentHeader() {
		return ledger_fetch_.max_consensus_ledger_->GetProtoHeader();
	}
	void LedgerManager::SetMaxConsensusLedger(LedgerFrm &ledger) {
		ledger_fetch_.max_consensus_ledger_ = std::make_shared<LedgerFrm>(ledger);
	}

	bool LedgerManager::CreateTableIfNotExist() {
		RationalDb *db = Storage::Instance().rational_db();
		do {
			Json::Value columns;
			if (!db->DescribeTable(std::string(General::LEDGER_TABLE_NAME), columns) &&
				!db->Execute(General::LEDGER_CREATE_SQL)) {
				LOG_ERROR_ERRNO("Create table(%s) failed", General::LEDGER_CREATE_SQL, db->error_code(), db->error_desc());
			}
			if (!db->DescribeTable(std::string(General::TX_TABLE_NAME), columns) &&
				!db->Execute(General::TX_CREATE_SQL)) {
				LOG_ERROR_ERRNO("Create table(%s) failed", General::TX_CREATE_SQL, db->error_code(), db->error_desc());
			}
			if (!db->DescribeTable(std::string(General::ACCOUNT_TX_NAME), columns) &&
				!db->Execute(General::ACCOUNT_TX_CREATE_SQL)) {
				LOG_ERROR_ERRNO("Create table(%s) failed", General::ACCOUNT_TX_CREATE_SQL, db->error_code(), db->error_desc());
			}
			if (!db->DescribeTable(std::string(General::UNIQUE_ASSET_NAME), columns) &&
				!db->Execute(General::UNIQUE_ASSET_CREATE_SQL)) {
				LOG_ERROR_ERRNO("Create table(%s) failed", General::UNIQUE_ASSET_CREATE_SQL, db->error_code(), db->error_desc());
			}
			if (!db->DescribeTable(std::string(General::RECORD_TABLE_NAME), columns) &&
				!db->Execute(General::RECORD_SQL)) {
				LOG_ERROR_ERRNO("Create table(%s) failed", General::RECORD_SQL, db->error_code(), db->error_desc());
			}
			if (!db->DescribeTable(std::string(General::TABLE_LEDGER_BUFFER), columns) &&
				!db->Execute(General::TABLE_LEDGER_BUFFER_CREATE_SQL)) {
				LOG_ERROR_ERRNO("Create table(%s) failed", General::TABLE_LEDGER_BUFFER, db->error_code(), db->error_desc());
			}
		} while (false);
		return true;
	}


	bool LedgerManager::CreateGenesisAccount() {
		LOG_INFO("There is no ledger exist,then create a init ledger");
		protocol::Account acc;
		acc.set_account_address(Configure::Instance().ledger_configure_.genesis_account_);
		acc.set_account_balance(100000000000000000);
		acc.set_previous_tx_hash("");
		acc.set_previous_ledger_seq(1);
		acc.set_tx_seq((int64_t)1 << 32);
		acc.clear_assets();
		acc.clear_unique_asset();
		acc.set_metadata("");

		AccountFrm::pointer acc_frm = std::make_shared<AccountFrm>(acc);
		acc_frm->SetProtoMasterWeight(1);
		acc_frm->SetProtoLowThreshold(1);
		acc_frm->SetProtoMedThreshold(1);
		acc_frm->SetProtoHighThreshold(1);
		tree_->Record();
		AccountEntry entry;
		if (!entry.AddEntry(acc.account_address(), acc_frm)) {
			BUBI_EXIT("genesi account already exist");
		}

		if (!LedgerManager::Instance().Commit(entry)) {
			BUBI_EXIT("create genesi account fail");
		}
		tree_->UpdateTreeHash(false);
		protocol::Ledger ledger;
		protocol::LedgerHeader *header = ledger.mutable_ledger_header();
		ledger.clear_transaction_envs();
		protocol::Value *v = header->mutable_consensus_value();
		v->set_hash_set("0000000000");
		v->mutable_ledger_upgrade()->set_new_base_fee(1000);
		v->mutable_ledger_upgrade()->set_new_base_reserve(10000000);
		v->mutable_ledger_upgrade()->set_new_ledger_version(1000);
		v->set_close_time(0);

		header->set_parent_hash("");
		std::vector<TransactionFrm::pointer> tx_array;
		tx_array.clear();
		header->set_base_fee(1000);
		header->set_base_reserve(10000000);
		header->set_ledger_version(1000);
		header->set_ledger_sequence(1);
		header->set_tx_count(0);
		header->set_transaction_tree_hash(LedgerFrm::CalculateTxTreeHash(tx_array, header->ledger_version()));
		header->set_account_tree_hash(GetAccountRootHash());
		header->set_hash("");

		header->set_hash(utils::Sha256::Crypto(header->SerializeAsString()));
		LOG_INFO("first ledger hash:%s", utils::encode_b16(header->hash()).c_str());
		last_closed_ledger_ = std::make_shared<LedgerFrm>();
		last_closed_ledger_->FromProtoLedger(ledger);
		WRITE_BATCH &batch = tree_->batch_;
		batch.Put(bubi::General::KEY_LEDGER_SEQ, "1");
		if (!last_closed_ledger_->AddToDb()) {
			BUBI_EXIT("AddToDb fail");
		}
		if (!Storage::Instance().keyvalue_db()->WriteBatch(batch)) {
			BUBI_EXIT("write batch fail, %s", Storage::Instance().keyvalue_db()->error_desc().c_str());
		}

		return true;
	}


	int LedgerManager::OnConsent(int64_t checkpoint_seq,
		int64_t request_seq,
		const TransactionSetFrmPtr txSet,
		const protocol::Value &value,
		bool cacul_total) {

		LOG_INFO("OnConsent Ledger(" FMT_I64 ") request seq(" FMT_I64 ") txcount(%u) cacul_total=%s",
			checkpoint_seq,
			request_seq,
			txSet->txset_by_acc_seq_.size(),
			cacul_total ? "true" : "false");
		utils::MutexGuard guard(gmutex_);

		protocol::LedgerHeader header;
		header.CopyFrom(last_closed_ledger_->GetProtoHeader());
		header.mutable_consensus_value()->CopyFrom(value);
		header.set_ledger_sequence(checkpoint_seq);
		header.set_parent_hash(txSet->PreviousLedgerHash());
		tmp_ledger_ = std::make_shared<LedgerFrm>(header, txSet->txset_by_acc_seq_);
		ledger_fetch_.OnConsent(tmp_ledger_);
		return 0;
	}

	int64_t LedgerManager::GetMaxLedger() {
		std::string sql = "select max(ledger_seq) as seq from ledger";
		auto db = Storage::Instance().rational_db();
		Json::Value record;
		int nret = db->QueryRecord(sql, record);
		if (nret < 0) {
			BUBI_EXIT_ERRNO("query sql(%s) fail", sql.c_str(), db->error_code(), db->error_desc());
		}
		int64_t seq = record["seq"].asInt64();
		return seq;
	}

	void LedgerManager::AddToDb(LedgerFrm::pointer pledger) {

		int64_t ledger_seq = pledger->GetProtoHeader().ledger_sequence();
		WRITE_BATCH batch = tree_->batch_;
		batch.Put(bubi::General::KEY_LEDGER_SEQ, utils::String::Format(FMT_I64, ledger_seq));
		int64_t time0 = utils::Timestamp().HighResolution();
		if (!pledger->AddToDb()) {
			BUBI_EXIT("AddToDb failed");
		}

		int64_t time1 = utils::Timestamp().HighResolution();
		if (Configure::Instance().db_configure_.async_write_kv_) {
			utils::MutexGuard guard(mutex_);
			std::string tmpname = Configure::Instance().db_configure_.tmp_path_ + "/account.txt";
			utils::File file;
			if (!file.Open(tmpname, utils::File::FILE_OPEN_MODE::FILE_M_WRITE | utils::File::FILE_OPEN_MODE::FILE_M_BINARY)) {
				BUBI_EXIT_ERRNO("open file(%s) failed", tmpname.c_str(), STD_ERR_CODE, STD_ERR_DESC);
			}

			file.Write(WRITE_BATCH_DATA(batch), WRITE_BATCH_DATA_SIZE(batch), 1);
			file.Close();


			std::string name = utils::String::Format("%s/block_" FMT_I64 "%s",
				Configure::Instance().db_configure_.tmp_path_.c_str(), pledger->GetProtoHeader().ledger_sequence(),
				General::KEYVALUE_TMPDB_POSTFIX);

			if (!utils::File::Move(tmpname, name, true)) {
				LOG_ERROR_ERRNO("Move file(%s) failed", tmpname.c_str(), STD_ERR_CODE, STD_ERR_DESC);
				return;
			}

			int64_t time12 = utils::Timestamp().HighResolution();

			{
				//lock to make sure account is accessible before last batch written complete
				utils::WriteLockGuard guard(batch_cache_.mutex_);
				batch_cache_.ClearRecord();
				batch.Iterate(&batch_cache_);
			}

			int64_t time2 = utils::Timestamp().HighResolution();
			last_closed_ledger_ = pledger;

			if (last_closed_ledger_->GetProtoHeader().ledger_sequence() > GetMaxConsentHeader().ledger_sequence())
				SetMaxConsensusLedger(*pledger);

			LOG_INFO("ledger persist (" FMT_I64 ") txcount(" FMT_I64 ")  hash=%s  %s_sql=" FMT_I64_EX(-8)
				"async_kv=" FMT_I64_EX(-8) "batch_count=%-8u batch_cache=" FMT_I64_EX(-8),
				ledger_seq,
				pledger->GetTxCount(),
				utils::String::Bin4ToHexString(pledger->ProtoHeader().hash()).c_str(),
				Configure::Instance().db_configure_.async_write_sql_ ? "async" : "sync",
				time1 - time0,
				time2 - time1,
				batch_cache_.Size(),
				time2 - time12);

			//this may block until last ledger finish writing
			db_worker_.WriteLedger(pledger, name, batch);
			tree_->Record();
		}
		else {
			if (!Storage::Instance().keyvalue_db()->WriteBatch(batch)) {
				BUBI_EXIT("Write batch failed: %s", Storage::Instance().keyvalue_db()->error_desc().c_str());
			}
			int64_t time2 = utils::Timestamp().HighResolution();
			LOG_INFO("ledger persist (" FMT_I64 ") txcount(" FMT_I64 ")  hash=%s  %s_sql=" FMT_I64_EX(-8)
				"sync_kv=" FMT_I64_EX(-8) " batch_size=%-8u",
				ledger_seq,
				pledger->GetTxCount(),
				utils::String::Bin4ToHexString(pledger->ProtoHeader().hash()).c_str(),
				Configure::Instance().db_configure_.async_write_sql_ ? "async" : "sync",
				time1 - time0,
				time2 - time1,
				WRITE_BATCH_DATA_SIZE(batch));
			tree_->Record();
			last_closed_ledger_ = pledger;
		}

	}

	bool LedgerManager::GetAccountEntry(const std::string &address, AccountFrm::pointer &paccount_frm) {
		std::string index = utils::Sha256::Crypto(address);
		utils::MutexGuard guard(gmutex_);
		std::string str;
		bool ret = batch_cache_.Get(address, str);

		if (!ret
			&& !Storage::Instance().keyvalue_db()->Get(index, str)) {
			return false;
		}
		else {
			paccount_frm = std::make_shared<AccountFrm>();
			return paccount_frm->UnSerializer(str);
		}
	}

	void LedgerManager::GetModuleStatus(Json::Value &data) {
		int64_t begin_time = utils::Timestamp::HighResolution();
		utils::MutexGuard guard(gmutex_);
		data["name"] = "ledger_manager";
		ledger_fetch_.GetModuleStatus(data["ledger_fetch"]);

		data["tx_count"] = GetLastClosedLedger().tx_count();
		data["account_count"] = tree_->LeafCount();
		data["ledger_sequence"] = GetLastClosedLedger().ledger_sequence();
		data["sync_completed"] = SyncComplete();
		data["time"] = utils::String::Format(FMT_I64 " ms",
			(utils::Timestamp::HighResolution() - begin_time) / utils::MICRO_UNITS_PER_MILLI);
	}

	void LedgerManager::CloseLedger(LedgerFrm::pointer pledger, bool bconsent) {
		LOG_INFO("closing ledger(" FMT_I64 ")", pledger->GetProtoHeader().ledger_sequence());

		if (pledger->ProtoHeader().parent_hash() != last_closed_ledger_->ProtoHeader().hash()) {
			LOG_ERROR("close ledger(" FMT_I64 ") failed! parent hash(%s)!=last closed ledger hash(%s)",
				pledger->GetProtoHeader().ledger_sequence(),
				utils::String::Bin4ToHexString(pledger->ProtoHeader().parent_hash()).c_str(),
				utils::String::Bin4ToHexString(last_closed_ledger_->ProtoHeader().hash()).c_str()
				);
			exit(1);
		}

		int64_t time0 = utils::Timestamp().HighResolution();

		pledger->Apply(last_closed_ledger_->GetProtoHeader());
		Commit(*pledger->entry_);
		int64_t time1 = utils::Timestamp().HighResolution();
		tree_->UpdateTreeHash(pledger->apply_tx_frms_.size() > 100);
		int64_t time2 = utils::Timestamp().HighResolution();

		pledger->SetHeaderHashs(tree_->RootHash(), last_closed_ledger_->GetProtoHeader().tx_count());
		AddToDb(pledger);
		int64_t time3 = utils::Timestamp().HighResolution();

		LOG_INFO("ledger closed (" FMT_I64 ") txcount(" FMT_I64 ")  hash=%s  apply="  FMT_I64_EX(-8) "calc_hash="  FMT_I64_EX(-8) "addtodb = " FMT_I64_EX(-8)
			" total=" FMT_I64_EX(-8),
			pledger->GetProtoHeader().ledger_sequence(),
			pledger->GetTxCount(),
			utils::String::Bin4ToHexString(pledger->GetProtoHeader().hash()).c_str(),
			time1 - time0,
			time2 - time1,
			time3 - time2,
			time3 - time0);
	}

	bool LedgerManager::Commit(AccountEntry &entry) {
		for (auto it = entry.entries_.begin(); it != entry.entries_.end(); it++) {
			auto &r = it->second;
			switch (r.action_) {
			case AccountEntry::DEL:{
				break;
			}
			case AccountEntry::ADD:{
				std::string index = utils::Sha256::Crypto(it->first);
				LedgerManager::Instance().tree_->Add(index, it->second.value_, "");
				break;
			}
			case AccountEntry::MOD:{
				std::string index = utils::Sha256::Crypto(it->first);
				LedgerManager::Instance().tree_->Set(index, it->second.value_, "");
				break;
			}
			case AccountEntry::KEEP:{
				break;
			}
			default:
				break;
			}
		}
		return true;
	}

	WriteDbWorker::WriteDbWorker() {
		pledger_ = NULL;
	}

	void WriteDbWorker::WriteLedger(LedgerFrm::pointer pledger, std::string filename, WRITE_BATCH &batch) {
		do {
			mutex_.Lock();
			if (pledger_) {
				mutex_.Unlock();
				utils::Sleep(1);
			}
			else {
				pledger_ = pledger;
				filename_ = filename;
				batch_ = batch;
				mutex_.Unlock();
				break;
			}
		} while (true);
	}

	void WriteDbWorker::Run(utils::Thread *this_thread) {
		while (this_thread->enabled()) {
			utils::Sleep(1);
			utils::MutexGuard guard(mutex_);
			if (pledger_) {
				int64_t time0 = utils::Timestamp().HighResolution();
				if (!Storage::Instance().keyvalue_db()->WriteBatch(batch_)) {
					BUBI_EXIT("Write batch failed, %s", Storage::Instance().keyvalue_db()->error_desc().c_str());
				}

				utils::File::Delete(filename_);
				int64_t time1 = utils::Timestamp().HighResolution();
				LOG_INFO("ledger closed (" FMT_I64 ") txcount(" FMT_I64 ")  hash=%s  keyvalue=" FMT_I64_EX(-8),
					pledger_->GetProtoHeader().ledger_sequence(),
					pledger_->GetTxCount(),
					utils::String::Bin4ToHexString(pledger_->GetProtoHeader().hash()).c_str(),
					time1 - time0);
				pledger_ = NULL;
				batch_.Clear();
			}
		}
	}

	AccountEntry::AccountEntry() {
		parent_ = nullptr;
	}

	AccountEntry::AccountEntry(AccountEntry* p) {
		parent_ = p;
	}

	bool AccountEntry::LoadValue(const std::string &address, AccountFrm::pointer &frm) {
		AccountFrm::pointer account_pt = std::make_shared<AccountFrm>();
		if (!LedgerManager::Instance().GetAccountEntry(address, account_pt)) {
			return false;
		}
		else
			frm = account_pt;
		return true;
	}
}