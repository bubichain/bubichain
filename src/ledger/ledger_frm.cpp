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

#include <sstream>
#include <utils/utils.h>
#include <common/storage.h>
#include "ledger_manager.h"
#include "ledger_frm.h"

namespace bubi {
#define COUNT_PER_PARTITION 1000000
	LedgerFrm::LedgerFrm() {
		id_ = 0;
		closed_ = false;
		entry_ = nullptr;
	}

	LedgerFrm::LedgerFrm(protocol::LedgerHeader header, TransactionSetFrm::TxSetByAccountSeq txset) {
		id_ = 0;
		closed_ = false;
		entry_ = nullptr;
		for (auto it = txset.begin(); it != txset.end(); it++) {
			tmp_tx_frms_.push_back(*it);
		}
		header_.CopyFrom(header);
	}

	void LedgerFrm::FromProtoLedger(protocol::Ledger &ledger) {
		header_.CopyFrom(ledger.ledger_header());
		id_ = 0;
		closed_ = false;
		std::shared_ptr<protocol::LedgerHeader> header = std::make_shared<protocol::LedgerHeader>(ledger.ledger_header());
		for (int i = 0; i < ledger.transaction_envs_size(); i++) {
			protocol::TransactionEnv env = ledger.transaction_envs(i);
			bubi::TransactionFrm::pointer tx_frm = std::make_shared<bubi::TransactionFrm>(env);
			tx_frm->SetHeader(header);
			tmp_tx_frms_.push_back(tx_frm);
		}
	}

	LedgerFrm::LedgerFrm(protocol::LedgerHeader &header) {
		header_.CopyFrom(header);
		id_ = 0;
		closed_ = false;
		entry_ = nullptr;
	}

	LedgerFrm::~LedgerFrm() {}

	protocol::LedgerHeader LedgerFrm::GetProtoHeader() const {
		return header_;
	}

	int64_t LedgerFrm::GetTxCount() {
		return apply_tx_frms_.size();
	}
	int64_t LedgerFrm::GetNextId() {
		id_++;
		return (header_.ledger_sequence() << 32) + id_;
	}

	std::string TransactionEnvToJson(protocol::TransactionEnv &txEnv) {
		Json::Value js;
		Json::Value &Operations = js["operations"];
		for (int i = 0; i < txEnv.transaction().operations_size(); i++) {
			Json::Value item;
			protocol::Operation operation = txEnv.transaction().operations(i);

			protocol::Operation_Type type = operation.type();
			item["type"] = type;
			switch (type) {
			case protocol::Operation_Type_CREATE_ACCOUNT:
			{
				protocol::OperationCreateAccount ope = operation.create_account();
				item["dest_address"] = ope.dest_address();
				item["init_balance"] = ope.init_balance();
				break;
			}
			case protocol::Operation_Type_PAYMENT:
			{
				protocol::Asset ope = operation.payment().asset();
				item["dest_address"] = operation.payment().destaddress();
				item["amount"] = ope.amount();
				item["type"] = ope.property().type() == 1 ? "IOU" : "NATIVE";

				Json::Value &property = item["property"];
				property["issuer"] = ope.property().issuer();
				property["code"] = ope.property().code();
				property["type"] = ope.property().type();
				break;
			}
			case protocol::Operation_Type_ISSUE_ASSET:
			{
				protocol::OperationIssueAsset ope = operation.issue_asset();
				item["amount"] = ope.asset().amount();
				Json::Value &property = item["property"];
				property["issuer"] = ope.asset().property().issuer();
				property["code"] = ope.asset().property().code();
				property["type"] = ope.asset().property().type();
				break;
			}
			default:
			{
				break;
			}
			}

			Operations[(Json::UInt) i] = item;
		}
		return js.toStyledString();
	}

	std::string LedgerFrm::CalculateTxTreeHash(
		const std::vector<TransactionFrm::pointer> &tx_array,
		const uint32_t ledger_version) {
		utils::Sha256 sha256;
		for (std::size_t i = 0; i < tx_array.size(); i++) {
			TransactionFrm::pointer env = tx_array[i];
			if (ledger_version == 1000) {
				sha256.Update(env->GetContentData());
			}
			else
				sha256.Update(env->GetFullHash());
		}
		return sha256.Final();
	}

	bool LedgerFrm::LoadFromDb(int64_t ledger_seq, protocol::Ledger &ledger) {
		bubi::RationalDb *db = bubi::Storage::Instance().rational_db();
		std::string sql = utils::String::Format("SELECT * FROM %s WHERE ledger_seq=" FMT_I64 "", General::LEDGER_TABLE_NAME, ledger_seq);
		Json::Value record;
		if (db->QueryRecord(sql, record) <= 0) {
			LOG_INFO("load ledger from db failed! (%s)", db->error_desc());
			return false;
		}
		protocol::LedgerHeader *header = ledger.mutable_ledger_header();
		header->set_ledger_sequence(record["ledger_seq"].asInt64());
		header->set_hash(utils::String::HexStringToBin(record["hash"].asString()));
		header->set_parent_hash(utils::String::HexStringToBin(record["phash"].asString()));
		header->set_transaction_tree_hash(utils::String::HexStringToBin(record["txhash"].asString()));
		header->set_account_tree_hash(utils::String::HexStringToBin(record["account_hash"].asString()));
		header->mutable_consensus_value()->ParseFromString(utils::String::HexStringToBin(record["consensus_value"].asString()));
		header->set_base_fee(record["base_fee"].asUInt());
		header->set_base_reserve(record["base_reserve"].asUInt());
		header->set_ledger_version(record["ledger_version"].asUInt());
		header->set_tx_count(record["tx_count"].asInt64());

		// load transactions
		std::string sql_tx = utils::String::Format("SELECT * FROM %s WHERE ledger_seq=" FMT_I64 " ORDER BY seq_in_global asc",
			General::TX_TABLE_NAME, ledger_seq);
		Json::Value tx_records = Json::Value(Json::arrayValue);

		if (db->Query(sql_tx, tx_records) < 0) {
			LOG_ERROR("query (%s) from db failed(%s)", sql_tx.c_str(), db->error_desc());
			return false;
		}

		for (Json::UInt i = 0; i < tx_records.size(); i++) {
			Json::Value item = tx_records[i];
			std::string str_body;
			std::string str_b16 = item["body"].asString();
			utils::decode_b16(str_b16, str_body);
			protocol::TransactionEnv *tx_env = ledger.add_transaction_envs();
			if (!tx_env->ParseFromString(str_body)) {
				LOG_ERROR("TransactionEnv ParseFrom table %s failed!", General::TX_TABLE_NAME);
				return false;
			}
		}
		return true;
	}

	bool LedgerFrm::LoadFromDb(int64_t ledger_seq) {

		bubi::RationalDb *db = bubi::Storage::Instance().rational_db();
		std::string sql = utils::String::Format("SELECT * FROM %s WHERE ledger_seq=" FMT_I64 "",
			General::LEDGER_TABLE_NAME, ledger_seq);
		Json::Value record;
		if (db->QueryRecord(sql, record) <= 0) {
			LOG_INFO("load ledger from db failed! (%s) sql(%s)", db->error_desc(), sql.c_str());
			return false;
		}
		header_.set_ledger_sequence(record["ledger_seq"].asInt64());
		header_.set_hash(utils::String::HexStringToBin(record["hash"].asString()));
		header_.set_parent_hash(utils::String::HexStringToBin(record["phash"].asString()));
		header_.set_transaction_tree_hash(utils::String::HexStringToBin(record["txhash"].asString()));
		header_.set_account_tree_hash(utils::String::HexStringToBin(record["account_hash"].asString()));
		header_.mutable_consensus_value()->ParseFromString(utils::String::HexStringToBin(record["consensus_value"].asString()));
		header_.set_base_fee(record["base_fee"].asUInt());
		header_.set_base_reserve(record["base_reserve"].asUInt());
		header_.set_ledger_version(record["ledger_version"].asUInt());
		header_.set_tx_count(record["tx_count"].asInt64());
		return true;
	}

	bool LedgerFrm::LoadFromDb(std::string seq, std::string num, Json::Value& value) {
		bool bret = false;
		int32_t error_code = protocol::ERRORCODE::ERRCODE_SUCCESS;
		do {
			bubi::RationalDb *db = bubi::Storage::Instance().rational_db();
			seq = db->Format(seq);

			std::string ledger_seq = "";
			std::string num_limit = "LIMIT ";
			int inum = 0;
			if (num.length() != 0) {
				inum = atoi(num.c_str());
			}
			if (seq.length() != 0 && inum > 0) {
				ledger_seq = "WHERE ledger_seq<=\'" + seq + "\'";
				num_limit += num;
			}
			else if (inum > 0) {
				num_limit += num;
			}
			else {
				num_limit += "1";
			}

			std::string sql = utils::String::Format("SELECT * FROM %s %s ORDER BY ledger_seq DESC %s",
				bubi::General::LEDGER_TABLE_NAME,
				ledger_seq.c_str(),
				num_limit.c_str());

			Json::Value results;
			if (db->Query(sql, results) <= 0) {
				error_code = protocol::ERRORCODE::ERRCODE_NOT_EXIST;
				LOG_INFO("load ledger from db failed! (%s) sql(%s)", db->error_desc(), sql.c_str());
				break;
			}
			Json::Value modules_status;
			{
				utils::ReadLockGuard guard(bubi::StatusModule::status_lock_);
				if (bubi::StatusModule::modules_status_ != NULL && !bubi::StatusModule::modules_status_->empty()) {
					modules_status.fromString(bubi::StatusModule::modules_status_->toFastString());
				}
			}
			Json::Value blocks;
			int size = results.size();
			int index = 0;
			for (index = 0; index < size; index++) {
				Json::Value block;
				Json::Value& result = results[index];

				if (modules_status.empty()) {
					result["transaction_size"] = 0;
					result["account_count"] = 0;
				}
				else {
					result["transaction_size"] = modules_status.get("glue_manager", Json::Value::null).get("transaction_size", Json::Value::null);
					result["account_count"] = modules_status.get("ledger_manager", Json::Value::null).get("account_count", Json::Value::null);
				}

				std::string str_consensus = result["consensus_value"].asString();
				protocol::Value proto_value;
				std::string decode_str;
				if (!utils::String::HexStringToBin(str_consensus, decode_str)) {
					error_code = protocol::ERRORCODE::ERRCODE_INTERNAL_ERROR;
					break;
				}
				if (!proto_value.ParseFromString(decode_str)) {
					error_code = protocol::ERRORCODE::ERRCODE_INTERNAL_ERROR;
					break;
				}

				Json::Value &consensus = result["consensus_value"];
				consensus["hash_set"] = utils::String::BinToHexString(proto_value.hash_set());
				consensus["close_time"] = proto_value.close_time();

				if (proto_value.has_ledger_upgrade()) {
					Json::Value &ledger_upgrade = consensus["ledger_upgrade"];

					const protocol::LedgerUpgrade &upgrade = proto_value.ledger_upgrade();
					if (upgrade.has_new_ledger_version()) {
						ledger_upgrade["new_ledger_version"] = upgrade.new_ledger_version();
					}
					if (upgrade.has_new_base_fee()) {
						ledger_upgrade["new_base_fee"] = upgrade.new_base_fee();
					}
					if (upgrade.has_new_base_reserve()) {
						ledger_upgrade["new_base_reserve"] = upgrade.new_base_reserve();
					}
				}

				blocks.append(result);
			}

			if (index == size) {
				value["blocks"] = blocks;
			}
			bret = true;
		} while (false);

		value["error_code"] = error_code;
		return bret;
	}

	void LedgerFrm::GetSqlTx(std::string &sql_tx, std::string &sql_account_tx) {
		if (apply_tx_frms_.size() > 100 && Configure::Instance().system_config_.thread_count_ > 1) {
			utils::ThreadPool thread_pool_;
			thread_pool_.Init(Configure::GetInstance()->system_config_.thread_count_);
			std::vector<GetSqlTask*> tasks;
			std::vector<std::string> sql_tx_vector(16);
			std::vector<std::string> sql_account_tx_vector(16);

			int64_t t0 = utils::Timestamp().HighResolution();

			for (std::size_t i = 0; i < 16; i++) {
				GetSqlTask *t1 = new GetSqlTask(apply_tx_frms_, (i * apply_tx_frms_.size()) / 16, ((i + 1) * apply_tx_frms_.size()) / 16,
					sql_tx_vector[i], sql_account_tx_vector[i], header_.ledger_sequence(), header_.tx_count() + 1 - apply_tx_frms_.size());
				thread_pool_.AddTask(t1);
				tasks.push_back(t1);
			}
			thread_pool_.WaitAndJoin();

			int64_t t1 = utils::Timestamp().HighResolution();

			for (std::size_t i = 0; i < 16; i++) {
				sql_tx += sql_tx_vector[i];
				sql_account_tx += sql_account_tx_vector[i];
			}

			int64_t t2 = utils::Timestamp().HighResolution();

			for (std::size_t i = 0; i < tasks.size(); i++) {
				delete tasks[i];
			}
			int64_t t3 = utils::Timestamp().HighResolution();

			LOG_INFO("mysql-m ledger(" FMT_I64 ")  add task=" FMT_I64_EX(-8)
				"merge result=" FMT_I64_EX(-8) " delete task=" FMT_I64_EX(-8) " total=" FMT_I64_EX(-8),
				header_.ledger_sequence(),
				t1 - t0,
				t2 - t1,
				t3 - t2,
				t3 - t0);
		}
		else {
			for (std::size_t i = 0; i < apply_tx_frms_.size(); i++) {
				std::string tmpsql_reserve;
				apply_tx_frms_[i]->GetSql(header_.ledger_sequence(), header_.tx_count() + i + 1 - apply_tx_frms_.size(), sql_tx, sql_account_tx, tmpsql_reserve);
			}
		}
	}

	bool LedgerFrm::AddToDb() {
		int64_t t0 = utils::Timestamp().HighResolution();
		std::string sql = "";
		bubi::RationalDb *db = bubi::Storage::Instance().rational_db();
		sql += utils::String::Format(
			"INSERT INTO %s "
			"(ledger_seq,hash,phash,txhash,account_hash,total_coins,close_time,consensus_value,base_fee,base_reserve,ledger_version,tx_count,state)"
			"values (" FMT_I64 ",'%s','%s','%s','%s'," FMT_I64 "," FMT_I64 ",'%s',%u,%u,%u," FMT_I64 ", %d);",
			General::LEDGER_TABLE_NAME,
			header_.ledger_sequence(),
			utils::encode_b16(header_.hash()).c_str(),
			utils::encode_b16(header_.parent_hash()).c_str(),
			utils::encode_b16(header_.transaction_tree_hash()).c_str(),
			utils::encode_b16(header_.account_tree_hash()).c_str(),
			(int64_t)0, //total_coins
			header_.consensus_value().close_time(),
			utils::encode_b16(header_.consensus_value().SerializeAsString()).c_str(),
			header_.base_fee(),
			header_.base_reserve(),
			header_.ledger_version(),
			header_.tx_count(),
			closed_
			);

		std::string sql_tx = "";
		std::string sql_account_tx = "";

		GetSqlTx(sql_tx, sql_account_tx);

		if (!sql_tx.empty()) {
			sql += sql_tx;
		}

		if (!sql_account_tx.empty()) {
			sql += sql_account_tx;
		}

		auto &unique_asset_sql = LedgerManager::Instance().unique_asset_sql_;
		for (auto i : unique_asset_sql)
			for (auto ii : i.second) {
				sql += ii + ";";
			}
		unique_asset_sql.clear();

		auto &record_sql = LedgerManager::Instance().record_sql_;
		for (auto i : record_sql)
			for (auto ii : i.second) {
				sql += ii + ";";
			}
		record_sql.clear();

		int64_t t1 = utils::Timestamp().HighResolution();

		if (Configure::Instance().db_configure_.async_write_sql_) {
			utils::File file;
			std::string tmpname = Configure::Instance().db_configure_.tmp_path_ + "/account_tx.txt";
			if (!file.Open(tmpname, utils::File::FILE_OPEN_MODE::FILE_M_WRITE)) {
				LOG_ERROR_ERRNO("Open file(%s) failed", tmpname.c_str(), STD_ERR_CODE, STD_ERR_DESC);
				return false;
			}
			file.Write(sql.c_str(), sql.length(), 1);
			file.Close();
			std::string name = utils::String::Format("%s/block_" FMT_I64 "%s", Configure::Instance().db_configure_.tmp_path_.c_str(),
				header_.ledger_sequence(), General::RATIONAL_TMPDB_POSTFIX);
			{
				utils::MutexGuard guard(LedgerManager::Instance().mutex_);
				if (!utils::File::Move(tmpname, name, true)) {
					LOG_ERROR("Move file fail,%d:%s", utils::error_code(), utils::error_desc().c_str());
					return false;
				}
			}
			int64_t t2 = utils::Timestamp().HighResolution();
			LOG_INFO("mysql append sql=" FMT_I64 " write_sql_file=" FMT_I64, t1 - t0, t2 - t1);
			return true;
		}
		else {

			if (!db->Execute("begin;")) {
				LOG_ERROR("excute sql fail, error_desc=[%s],sql = %s", db->error_desc(), sql.c_str());
				return false;
			}

			if (!db->Execute(sql)) {
				LOG_ERROR("excute sql fail, error_desc=[%s],sql = %s", db->error_desc(), sql.c_str());
				return false;
			}

			if (!db->Execute("commit;")) {
				LOG_ERROR("excute sql fail, error_desc=[%s],sql = %s", db->error_desc(), sql.c_str());
				return false;
			}

			int64_t t2 = utils::Timestamp().HighResolution();
			LOG_INFO("Mysql append sql=" FMT_I64 " sql_execute=" FMT_I64, t1 - t0, t2 - t1);
			return true;
		}
	}

	bool LedgerFrm::PreAllpy(protocol::LedgerHeader lastheader, int64_t request_seq, TransactionSetFrmPtr tx_set) {
		return true;
	}

	bool LedgerFrm::Apply(const protocol::LedgerHeader &lastheader) {
		if (entry_ == nullptr) {
			entry_ = std::make_shared<AccountEntry>();
		}

		header_.set_base_fee(lastheader.base_fee());
		header_.set_base_reserve(lastheader.base_reserve());
		header_.set_ledger_version(lastheader.ledger_version());

		if (header_.consensus_value().has_ledger_upgrade()) {
			protocol::LedgerUpgrade upgrade = header_.consensus_value().ledger_upgrade();
			if (upgrade.has_new_base_fee())
				header_.set_base_fee(upgrade.new_base_fee());

			if (upgrade.has_new_base_reserve())
				header_.set_base_reserve(upgrade.new_base_reserve());

			if (upgrade.has_new_ledger_version())
				header_.set_ledger_version(upgrade.new_ledger_version());
		}

		std::shared_ptr<protocol::LedgerHeader> pheader = std::make_shared<protocol::LedgerHeader>(header_);
		uint32_t success_count = 0;
		for (TransactionFrm::pointer const &tx_frm : tmp_tx_frms_) {
			if (!tx_frm->ValidForApply(pheader, *entry_)) {
				LOG_ERROR("transaction(%s) check valid failed while apply",
					utils::String::BinToHexString(tx_frm->GetContentHash()).c_str());
				continue;
			}

			tx_frm->PayFee();
			AccountEntry new_entry(entry_.get());
			if (!tx_frm->Apply(pheader, new_entry)) {
				LOG_ERROR("transaction(%s) apply failed", utils::String::BinToHexString(tx_frm->GetContentHash()).c_str());
			}
			else {
				//Tx success
				success_count++;

				auto &unique_asset_ledger = bubi::LedgerManager::Instance().unique_asset_sql_;
				for (auto i = tx_frm->unique_asset_.begin(); i != tx_frm->unique_asset_.end(); i++) {
					for (auto value_js : i->second) {
						char id[33];
						memset(id, 0, sizeof(id));
						sprintf(id, "%016llX", (unsigned long long)(tx_frm->GetLedgerSeq()));
						sprintf(id + 16, "%08X", success_count);
						sprintf(id + 24, "%08X", value_js["ope_seq"].asUInt());
						std::string sql = utils::String::Format(
							"insert into  table_unique_asset(id,ledger_seq,tx_hash,asset_issuer,asset_code,asset_detailed,from_address,to_address) values"
							"('%s'," FMT_I64 ",'%s','%s','%s','%s','%s','%s')",
							id,
							tx_frm->GetLedgerSeq(),
							utils::encode_b16(tx_frm->GetContentHash()).c_str(),
							value_js["asset_issuer"].asString().c_str(),
							value_js["asset_code"].asString().c_str(),
							value_js["asset_detailed"].asString().c_str(),
							value_js["from_address"].asString().c_str(),
							value_js["to_address"].asString().c_str());
						unique_asset_ledger[i->first].push_back(sql);
					}


				}

				auto &record_sql = bubi::LedgerManager::Instance().record_sql_;
				for (auto i = tx_frm->records_.begin(); i != tx_frm->records_.end(); i++) {

					for (auto value_js : i->second) {
						char id[33];
						sprintf(id, "%016llX", (unsigned long long)(tx_frm->GetLedgerSeq()));
						sprintf(id + 16, "%08X", success_count);
						sprintf(id + 24, "%08X", value_js["ope_seq"].asUInt());
						std::string sql = utils::String::Format(
							"insert into  table_record(id,ledger_seq,tx_hash,record_participant,record_address,record_id,record_ext) values"
							"('%s'," FMT_I64 ",'%s','%s','%s','%s','%s')",
							id,
							tx_frm->GetLedgerSeq(),
							utils::encode_b16(tx_frm->GetContentHash()).c_str(),
							value_js["record_participant"].asString().c_str(),
							value_js["record_address"].asString().c_str(),
							value_js["record_id"].asString().c_str(),
							value_js["record_ext"].asString().c_str());

						record_sql[i->first].push_back(sql);
					}
				}

				entry_->MergeFromBranch(new_entry);
			}
			apply_tx_frms_.push_back(tx_frm);
		}
		return true;
	}

	void LedgerFrm::SetHeaderHashs(const std::string &account_tree_hash, int64_t last_txcount) {
		header_.set_account_tree_hash(account_tree_hash);
		header_.set_transaction_tree_hash(CalculateTxTreeHash(apply_tx_frms_, header_.ledger_version()));
		header_.set_tx_count(last_txcount + apply_tx_frms_.size());
		header_.set_hash("");
		header_.set_hash(utils::Sha256::Crypto(header_.SerializeAsString()));
	}

	bool LedgerFrm::CheckValidation() {
		std::string tx_tree_hash = CalculateTxTreeHash(tmp_tx_frms_, header_.ledger_version());
		if (tx_tree_hash != header_.transaction_tree_hash()) {
			LOG_ERROR("ledger (" FMT_I64 ") ledger_version(%u) txhash diff [%s:%s]",
				header_.ledger_sequence(),
				header_.ledger_version(),
				utils::String::BinToHexString(tx_tree_hash).c_str(),
				utils::String::BinToHexString(header_.transaction_tree_hash()).c_str());
			return false;
		}

		std::string origion_hash = header_.hash();
		protocol::LedgerHeader header(header_);
		header.set_hash("");
		std::string tmp = utils::Sha256::Crypto(header.SerializeAsString());
		if (tmp != origion_hash) {
			LOG_ERROR("" FMT_I64 ",ledger_hash error [%s]!=[%s]",
				header.ledger_sequence(),
				utils::String::BinToHexString(tmp).c_str(),
				utils::String::BinToHexString(origion_hash).c_str());
			return false;
		}
		return true;
	}
	Json::Value LedgerFrm::ToJson() {
		Json::Value js;
		js["ledger_seq"] = header_.ledger_sequence();
		js["hash"] = utils::String::BinToHexString(header_.hash());
		js["phash"] = utils::String::BinToHexString(header_.parent_hash());
		js["txhash"] = utils::String::BinToHexString(header_.transaction_tree_hash());
		js["account_hash"] = utils::String::BinToHexString(header_.account_tree_hash());
		Json::Value &cons = js["consensus_value"];
		cons["hash_set"] = utils::String::BinToHexString(header_.consensus_value().hash_set());
		cons["close_time"] = header_.consensus_value().close_time();
		if (header_.consensus_value().has_ledger_upgrade()) {
			cons["new_base_fee"] = header_.consensus_value().ledger_upgrade().new_base_fee();
			cons["new_base_reserve"] = header_.consensus_value().ledger_upgrade().new_base_reserve();
			cons["new_ledger_version"] = header_.consensus_value().ledger_upgrade().new_ledger_version();
		}
		js["base_fee"] = header_.base_fee();
		js["base_reserve"] = header_.base_reserve();
		js["ledger_version"] = header_.ledger_version();
		js["tx_count"] = header_.tx_count();
		return js;
	}

	protocol::LedgerHeader &LedgerFrm::ProtoHeader() {
		return header_;
	}

	void GetSqlTask::Run(utils::Thread *this_thread) {
		LOG_DEBUG("getsqltx[%d,%d)", from_index_ + seq_in_global_, to_index_ + seq_in_global_);
		for (std::size_t i = from_index_; i != to_index_; i++) {
			std::string tmpsql_reserve;
			apply_tx_frms_[i]->GetSql(ledger_seq_, seq_in_global_ + i, sql_tx_, sql_account_tx_, tmpsql_reserve);
		}
	}
}
