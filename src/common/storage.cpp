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

#include <utils/strings.h>
#include <utils/logger.h>
#include <utils/file.h>
#include "common/general.h"
#include "storage.h"

namespace bubi {
	KeyValueDb::KeyValueDb() {}

	KeyValueDb::~KeyValueDb() {}

	std::string KeyValueDb::error_desc() {
		return error_desc_;
	}

#ifdef WIN32
	LevelDbDriver::LevelDbDriver() {
		db_ = NULL;
	}

	LevelDbDriver::~LevelDbDriver() {
		if (db_ != NULL) {
			delete db_;
			db_ = NULL;
		}
	}

	bool LevelDbDriver::Open(const std::string &db_path) {
		leveldb::Options options;
		options.create_if_missing = true;
		leveldb::Status status = leveldb::DB::Open(options, db_path, &db_);
		if (!status.ok()) {
			error_desc_ = status.ToString();
		}
		return status.ok();
	}

	bool LevelDbDriver::Close() {
		delete db_;
		db_ = NULL;
		return true;
	}

	bool LevelDbDriver::Get(const std::string &key, std::string &value) {
		assert(db_ != NULL);
		leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &value);
		if (!status.ok()) {
			error_desc_ = status.ToString();
		}
		return status.ok();
	}

	bool LevelDbDriver::Put(const std::string &key, const std::string &value) {
		assert(db_ != NULL);
		leveldb::WriteOptions opt;
		opt.sync = true;
		leveldb::Status status = db_->Put(opt, key, value);
		if (!status.ok()) {
			error_desc_ = status.ToString();
		}
		return status.ok();
	}

	bool LevelDbDriver::Delete(const std::string &key) {
		assert(db_ != NULL);
		leveldb::Status status = db_->Delete(leveldb::WriteOptions(), key);
		if (!status.ok()) {
			error_desc_ = status.ToString();
		}
		return status.ok();
	}

	bool LevelDbDriver::WriteBatch(WRITE_BATCH &write_batch) {

		leveldb::WriteOptions opt;
		opt.sync = true;
		leveldb::Status status = db_->Write(opt, &write_batch);
		if (!status.ok()) {
			error_desc_ = status.ToString();
		}
		return status.ok();
	}

	void* LevelDbDriver::NewIterator() {
		return db_->NewIterator(leveldb::ReadOptions());
	}

	bool LevelDbDriver::GetOptions(Json::Value &options) {
		return true;
	}

#else

	RocksDbDriver::RocksDbDriver() {
		db_ = NULL;
	}

	RocksDbDriver::~RocksDbDriver() {
		if (db_ != NULL) {
			delete db_;
			db_ = NULL;
		}
	}

	bool RocksDbDriver::Open(const std::string &db_path) {
		rocksdb::Options options;
		options.create_if_missing = true;
		// Optimize RocksDB. This is the easiest way to get RocksDB to perform well
		long cpus = sysconf(_SC_NPROCESSORS_ONLN);
		options.IncreaseParallelism(cpus);
		//options.OptimizeLevelStyleCompaction();
		rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db_);
		if (!status.ok()) {
			error_desc_ = status.ToString();
		}
		return status.ok();
	}

	bool RocksDbDriver::Close() {
		delete db_;
		db_ = NULL;
		return true;
	}

	bool RocksDbDriver::Get(const std::string &key, std::string &value) {
		assert(db_ != NULL);
		rocksdb::Status status = db_->Get(rocksdb::ReadOptions(), key, &value);
		return status.ok();
	}

	bool RocksDbDriver::Put(const std::string &key, const std::string &value) {
		assert(db_ != NULL);
		rocksdb::WriteOptions opt;
		opt.sync = true;
		rocksdb::Status status = db_->Put(opt, key, value);
		if (!status.ok()) {
			error_desc_ = status.ToString();
		}
		return status.ok();
	}

	bool RocksDbDriver::Delete(const std::string &key) {
		assert(db_ != NULL);
		rocksdb::WriteOptions opt;
		opt.sync = true;
		rocksdb::Status status = db_->Delete(opt, key);
		if (!status.ok()) {
			error_desc_ = status.ToString();
		}
		return status.ok();
	}

	bool RocksDbDriver::WriteBatch(WRITE_BATCH &write_batch) {

		rocksdb::WriteOptions opt;
		opt.sync = true;
		rocksdb::Status status = db_->Write(opt, &write_batch);
		if (!status.ok()) {
			error_desc_ = status.ToString();
		}
		return status.ok();
	}

	void* RocksDbDriver::NewIterator() {
		return db_->NewIterator(rocksdb::ReadOptions());
	}

	bool RocksDbDriver::GetOptions(Json::Value &options) {
		std::string out;
		db_->GetProperty("rocksdb.estimate-table-readers-mem", &out);
		options["rocksdb.estimate-table-readers-mem"] = out;

		db_->GetProperty("rocksdb.cur-size-all-mem-tables", &out);
		options["rocksdb.cur-size-all-mem-tables"] = out;
		return true;
	}
#endif

	RationalDb::RationalDb() {}

	RationalDb::~RationalDb() {}

	std::string RationalDb::Format(const std::string &value) {
		std::string new_value = value;
		utils::String::Replace(new_value, "\\", "\\\\");
		utils::String::Replace(new_value, "'", "\\'");
		utils::String::Replace(new_value, "\"", "\\\"");
		utils::String::Replace(new_value, "\r", "\\r");
		utils::String::Replace(new_value, "\n", "\\n");

		return new_value;
	}


	int32_t RationalDb::QueryRecord(const std::string &table, const std::string &sqlwhere, Json::Value &record) {
		std::string sql = utils::String::Format("select * from %s %s", table.c_str(), sqlwhere.c_str());
		return QueryRecord(sql, record);
	}

	Storage::Storage() {
		keyvalue_db_ = NULL;
		rational_db_ = NULL;
	}

	Storage::~Storage() {}

	bool Storage::Initialize(const std::string &keyvalue_db_path, const std::string &rational_db_con, const std::string &rational_db_type, bool bdropdb) {
		do {
			std::string strConnect = "";
			std::string str_dbname = "";
			std::vector<std::string> nparas = utils::String::split(rational_db_con, " ");
			for (std::size_t i = 0; i < nparas.size(); i++) {
				std::string str = nparas[i];
				std::vector<std::string> n = utils::String::split(str, "=");
				if (n.size() == 2) {
					if (n[0] != "dbname") {
						strConnect += " ";
						strConnect += str;
					}
					else {
						str_dbname = n[1];
					}
				}
			}
			if (bdropdb) {
				RationalDb *rational_db = NULL;
#ifdef BUBI_WITH_MYSQL
				if (rational_db_type == "mysql") rational_db = new MysqlDriver();
				else
#endif //BUBI_WITH_MYSQL
					rational_db = new SociDriver();

				bool do_success = false;
				do {
					if (utils::File::IsExist(keyvalue_db_path) && !utils::File::DeleteFolder(keyvalue_db_path)) {
						LOG_ERROR_ERRNO("Delete keyvalue db failed", STD_ERR_CODE, STD_ERR_DESC);
					}
					if (!rational_db->Open(strConnect)) {
						LOG_ERROR_ERRNO("Delete rational db failed", rational_db->error_code(), rational_db->error_desc());
						rational_db->Close();
						delete rational_db;
						return false;
					}

					std::string cmd = utils::String::Format("drop database if exists %s", str_dbname.c_str());
					if (!rational_db->Execute(cmd)) {
						LOG_ERROR_ERRNO("Delete rational db failed", rational_db->error_code(), rational_db->error_desc());
						break;
					}

					//delete tmp files
					std::string path = Configure::Instance().db_configure_.tmp_path_;
					utils::FileAttributes nlist;
					utils::File::GetFileList(path, nlist);
					for (auto it = nlist.begin(); it != nlist.end(); ++it) {
						if (it->first.rfind(bubi::General::RATIONAL_TMPDB_POSTFIX) != std::string::npos
							|| it->first.rfind(bubi::General::KEYVALUE_TMPDB_POSTFIX) != std::string::npos) {
							std::string filename = utils::String::Format("%s/%s", path.c_str(), it->first.c_str());
							LOG_INFO("removing tmp file %s", filename.c_str());
							utils::File::Delete(filename);
						}
					}

					LOG_INFO("Drop db successful");
					do_success = true;
				} while (false);

				rational_db->Close();
				delete rational_db;
				return do_success;
			}

#ifdef WIN32
			keyvalue_db_ = new LevelDbDriver();
			if (!keyvalue_db_->Open(keyvalue_db_path)) {
				LOG_ERROR("keyvalue_db path(%s) open fail(%s)\n",
					keyvalue_db_path.c_str(), keyvalue_db_->error_desc().c_str());
				break;
			}
#else
			keyvalue_db_ = new RocksDbDriver();
			if (!keyvalue_db_->Open(keyvalue_db_path)) {
				LOG_ERROR("keyvalue_db path(%s) open fail(%s)\n",
					keyvalue_db_path.c_str(), keyvalue_db_->error_desc().c_str());
				break;
			}
#endif
			if (!utils::File::IsExist(Configure::Instance().db_configure_.tmp_path_)) {
				utils::File::CreateDir(Configure::Instance().db_configure_.tmp_path_);
			}
			rational_db_ = NULL;
#ifdef BUBI_WITH_MYSQL
			if (rational_db_type == "mysql") rational_db_ = new MysqlDriver();
			else
#endif //BUBI_WITH_MYSQL
				rational_db_ = new SociDriver();

			//save the rational db type
			ratiional_db_type_ = rational_db_type;

			if (!rational_db_->Open(rational_db_con)) {
				if (!rational_db_->Open(strConnect)) {
					LOG_ERROR_ERRNO("open rational db failed", rational_db_->error_code(), rational_db_->error_desc());
					break;
				}

				std::string sql = utils::String::Format("create database %s", str_dbname.c_str());
				if (!rational_db_->Execute(sql)) {
					LOG_ERROR_ERRNO("create database failed", rational_db_->error_code(), rational_db_->error_desc());
					break;
				}

				if (!rational_db_->Open(rational_db_con)) {
					LOG_ERROR_ERRNO("open rational db failed", rational_db_->error_code(), rational_db_->error_desc());
					break;
				}
			}
			return true;
		} while (false);

		CloseDb();

		return false;
	}


	bool  Storage::CloseDb() {
		bool ret1 = true, ret2 = true;
		if (keyvalue_db_ != NULL) {
			ret1 = keyvalue_db_->Close();
			delete keyvalue_db_;
			keyvalue_db_ = NULL;
		}

		if (rational_db_ != NULL) {
			ret2 = rational_db_->Close();
			delete rational_db_;
			rational_db_ = NULL;
		}

		return ret1 && ret2;
	}

	bool Storage::Exit() {
		return CloseDb();
	}

	KeyValueDb *Storage::keyvalue_db() {
		return keyvalue_db_;
	}

	RationalDb *Storage::rational_db() {
		return rational_db_;
	}

	std::shared_ptr<RationalDb> Storage::NewRationalDb() {
		DbConfigure  &config = bubi::Configure::Instance().db_configure_;
		RationalDb *db = NULL;
#ifdef BUBI_WITH_MYSQL
		if (ratiional_db_type_ == "mysql") db = new MysqlDriver();
		else
#endif //BUBI_WITH_MYSQL
			db = new SociDriver();

		do {
			if (!db->Open(config.soci_connection_)) {
				LOG_ERROR("open rational db failed(%s)", rational_db_->error_desc());
				break;
			}

			return std::shared_ptr<RationalDb>(db);

		} while (false);

		return NULL;
	}

	SociDriver::SociDriver(void) {

	}

	SociDriver::~SociDriver(void) {

	}

	bool SociDriver::CreateTable(const std::string &table_name, const Json::Value &columns) {
		return false;
	}

	std::string SociDriver::GetSqls() {
		return sql_;
	}

	bool SociDriver::Open(const DbConfigure &cfg) {
		std::string str = utils::String::Format("dbname=%s user=%s password=%s",
			cfg.soci_dbname_.c_str(), cfg.soci_user_.c_str(), cfg.soci_password_.c_str());
		session_ = PQconnectdb(str.c_str());

		if (session_ != nullptr) {
			return true;
		}
		else {
			LOG_ERROR("connect error!");
			return false;
		}

	}


	bool SociDriver::Open(const std::string &connect) {
		session_ = PQconnectdb(connect.c_str());
		if (PQstatus(session_) != CONNECTION_OK) {
			error_code_ = PQstatus(session_);
			error_desc_ = PQerrorMessage(session_);
			return false;
		}
		else {
			return true;
		}
	}

	bool SociDriver::Execute(const std::string &sql) {
		utils::MutexGuard guard(mutex_);
		PGresult* res = PQexec(session_, sql.c_str());
		ExecStatusType statustype = PQresultStatus(res);

		switch (statustype) {
		case PGRES_COMMAND_OK:
		case PGRES_TUPLES_OK:
			PQclear(res);
			return true;
		case PGRES_EMPTY_QUERY:
		case PGRES_COPY_OUT:
		case PGRES_COPY_IN:
		case PGRES_BAD_RESPONSE:
		case PGRES_NONFATAL_ERROR:
		case PGRES_FATAL_ERROR:
			PQclear(res);
			error_code_ = statustype;
			error_desc_ = PQerrorMessage(session_);
			return false;
		default:
			PQclear(res);
			error_code_ = statustype;
			error_desc_ = PQerrorMessage(session_);
			return false;
		}
	}

	bool SociDriver::Insert(const std::string &table_name, const utils::StringMap &record) {
		std::string names, values;
		for (utils::StringMap::const_iterator iter = record.begin();
			iter != record.end();
			iter++) {
			if (names.empty()) {
				names = utils::String::Format("%s", Format(iter->first).c_str());
				values = utils::String::Format("'%s'", Format(iter->second).c_str());
			}
			else {
				names = utils::String::Format("%s,%s", names.c_str(), Format(iter->first).c_str());
				values = utils::String::Format("%s,'%s'", values.c_str(), Format(iter->second).c_str());
			}
		}

		std::string sql = utils::String::Format("INSERT INTO %s(%s) VALUES(%s)", table_name.c_str(), names.c_str(), values.c_str());
		return Execute(sql);
	}

	bool SociDriver::Close() {
		PQfinish(session_);
		return true;
	}

	int32_t SociDriver::Query(const std::string &sql, Json::Value &record_array) {
		utils::MutexGuard guard(mutex_);

		int32_t nrows = 0;
		PGresult* res = PQexec(session_, sql.c_str());
		ExecStatusType status = PQresultStatus(res);
		if (status != PGRES_TUPLES_OK) {
			error_code_ = status;
			error_desc_ = PQerrorMessage(session_);
			PQclear(res);
			return -1;
		}
		int row_count = PQntuples(res);
		int col_count = PQnfields(res);

		for (int row = 0; row < row_count; row++) {
			for (int col = 0; col < col_count; col++) {
				std::string colname(PQfname(res, col));
				char * buf = PQgetvalue(res, row, col);
				int length = PQgetlength(res, row, col);
				Oid ctype = PQftype(res, col);
				switch (ctype) {
				case 25:   // text
				case 1043: // varchar
				case 2275: // cstring
				case 18:   // char
				case 1042: // bpchar
				case 142: // xml
				case 114:  // json
				case 17: // bytea
				case 2950: // uuid
				{
					record_array[row][colname] = std::string(buf, length);
					break;
				}

				case 702:  // abstime
				case 703:  // reltime
				case 1082: // date
				case 1083: // time
				case 1114: // timestamp
				case 1184: // timestamptz
				case 1266: // timetz
				{
					record_array[row][colname] = std::string(buf, length);
					break;
				}

				case 700:  // float4
				case 701:  // float8
				case 1700: // numeric
				{
					record_array[row][colname] = utils::String::Stod(std::string(buf, length));
					break;
				}

				case 16:   // bool
				case 21:   // int2
				case 23:   // int4
				case 26:   // oid
				{
					record_array[row][colname] = utils::String::Stoi64(std::string(buf, length));
					break;
				}
				case 20:   // int8
				{
					record_array[row][colname] = utils::String::Stoi64(std::string(buf, length));
					break;
				}
				default:
				{
				}
				}
			}
		}
		PQclear(res);
		return row_count;
	}

	int32_t SociDriver::QueryRecord(const std::string &sql, Json::Value &record) {
		utils::MutexGuard guard(mutex_);

		PGresult* res = PQexec(session_, sql.c_str());
		ExecStatusType status = PQresultStatus(res);
		if (status != PGRES_TUPLES_OK) {
			error_code_ = status;
			error_desc_ = PQerrorMessage(session_);
			PQclear(res);
			return -1;
		}
		int row_count = PQntuples(res);
		int col_count = PQnfields(res);
		if (row_count <= 0) {
			return row_count;
		}

		for (int col = 0; col < col_count; col++) {
			std::string colname(PQfname(res, col));
			char * buf = PQgetvalue(res, 0, col);
			Oid ctype = PQftype(res, col);
			int length = PQgetlength(res, 0, col);
			switch (ctype) {
			case 25:   // text
			case 1043: // varchar
			case 2275: // cstring
			case 18:   // char
			case 1042: // bpchar
			case 142: // xml
			case 114:  // json
			case 17: // bytea
			case 2950: // uuid
			{
				record[colname] = std::string(buf, length);
				break;
			}

			case 702:  // abstime
			case 703:  // reltime
			case 1082: // date
			case 1083: // time
			case 1114: // timestamp
			case 1184: // timestamptz
			case 1266: // timetz
			{
				record[colname] = std::string(buf, length);
				break;
			}

			case 700:  // float4
			case 701:  // float8
			case 1700: // numeric
			{
				std::string s = std::string(buf, length);
				record[colname] = utils::String::Stod(s);
				break;
			}

			case 16:   // bool
			case 21:   // int2
			case 23:   // int4
			case 26:   // oid
			{
				std::string s = std::string(buf, length);
				record[colname] = utils::String::Stoi64(s);
				break;
			}
			case 20:   // int8
			{
				int length = PQgetlength(res, 0, col);
				record[colname] = utils::String::Stoi64(std::string(buf, length));
				break;
			}
			default:
			{
			}
			}
		}

		PQclear(res);
		return row_count;
	}

	bool SociDriver::Update(const std::string &table_name, const utils::StringMap &record, const std::string &sql_where) {
		std::string sets;
		for (utils::StringMap::const_iterator iter = record.begin();
			iter != record.end();
			iter++) {
			if (sets.empty()) {
				sets = utils::String::Format("%s=%s", Format(iter->first).c_str(), Format(iter->second).c_str());
			}
			else {
				sets = utils::String::Format("%s,%s=%s", sets.c_str(), Format(iter->first).c_str(), Format(iter->second).c_str());
			}
		}

		std::string sql = utils::String::Format("UPDATE %s SET %s %s", table_name.c_str(), sets.c_str(), sql_where.c_str());
		return Execute(sql);
	}

	void SociDriver::SetError(const char *pMessage) {
		
	}
	const char *SociDriver::error_desc() {
		return error_desc_.c_str();
	}


	int SociDriver::error_code() {
		return error_code_;
	}

	int32_t SociDriver::DescribeTable(const std::string &table_name, Json::Value &table_desc) {
		std::string strcond = utils::String::Format("where relname='%s'", table_name.c_str());
		int64_t row_count = QueryCount("pg_class", strcond);
		if (row_count <= 0) {
			return 0;
		}
		return 1;
	}

	int64_t SociDriver::QueryCount(const std::string &table_name, const std::string &condition) {
		utils::MutexGuard guard(mutex_);

		std::string sql = utils::String::Format("SELECT COUNT(*) AS count FROM %s %s", table_name.c_str(), condition.c_str());
		int64_t count = 0;
		PGresult* res = PQexec(session_, sql.c_str());
		ExecStatusType ty = PQresultStatus(res);
		if (ty != PGRES_TUPLES_OK) {
			error_code_ = ty;
			error_desc_ = PQerrorMessage(session_);
			PQclear(res);
			return -1;
		}
		char * buf = PQgetvalue(res, 0, 0);
		int len = PQgetlength(res, 0, 0);
		count = utils::String::Stoi64(std::string(buf, len));
		PQclear(res);
		return count;
	}

	void SociDriver::AppendSql(const std::string &sql) {
		sql_ += sql;
		sql_ += ";";
	}

	bool SociDriver::Begin() {
		sql_ = "BEGIN;";
		return true;
	}

	bool SociDriver::Commit() {
		sql_ += "COMMIT;";
		return Execute(sql_);
	}

#ifdef BUBI_WITH_MYSQL
	MysqlDriver::MysqlDriver(void) {
		session_ = NULL;
	}

	MysqlDriver::~MysqlDriver(void) {

	}

	bool MysqlDriver::Open(const std::string &connect) {
		std::map<std::string, std::string> attr = utils::String::ParseAttribute(connect, " ", "=");

		session_ = mysql_init(NULL);
		if (NULL == session_) {
			return false;
		}

		my_bool true_value_ = true;
		mysql_options(session_, MYSQL_SET_CHARSET_NAME, "utf8");
		mysql_options(session_, MYSQL_OPT_RECONNECT, &true_value_);

		if (mysql_real_connect(session_, attr["hostaddr"].c_str(), attr["user"].c_str(), attr["password"].c_str(), attr["dbname"].c_str(), atoi(attr["port"].c_str()), NULL, CLIENT_MULTI_STATEMENTS) == NULL) {
			return false;
		}

		return true;
	}

	bool MysqlDriver::Execute(const std::string &sql) {
		utils::MutexGuard guard(mutex_);

		assert(NULL != session_);

		if (mysql_real_query(session_, sql.c_str(), sql.length()) != 0) {
			return false;
		}

		int64_t affect_rows = (int64_t)mysql_affected_rows(session_);
		do {
			MYSQL_RES *res = mysql_store_result(session_);
			if (NULL != res) {
				mysql_free_result(res);
				res = NULL;
			}
		} while (mysql_next_result(session_) == 0);

		return true;
	}

	bool MysqlDriver::Insert(const std::string &table_name, const utils::StringMap &record) {
		std::string names, values;
		for (utils::StringMap::const_iterator iter = record.begin();
			iter != record.end();
			iter++) {
			if (names.empty()) {
				names = utils::String::Format("%s", Format(iter->first).c_str());
				values = utils::String::Format("'%s'", Format(iter->second).c_str());
			}
			else {
				names = utils::String::Format("%s,%s", names.c_str(), Format(iter->first).c_str());
				values = utils::String::Format("%s,'%s'", values.c_str(), Format(iter->second).c_str());
			}
		}

		std::string sql = utils::String::Format("INSERT INTO %s(%s) VALUES(%s)", table_name.c_str(), names.c_str(), values.c_str());
		return Execute(sql);
	}

	bool MysqlDriver::Close() {
		if (NULL != session_) {
			mysql_close(session_);
			session_ = NULL;
		}
		return true;
	}

	void MysqlDriver::GetJson(MYSQL_RES *res, MYSQL_ROW pprow, Json::Value &values) const {
		assert(NULL != session_);
		assert(NULL != pprow);

		values = Json::Value(Json::objectValue);

		int num = (int)mysql_num_fields(res);
		for (int i = 0; i < num; i++) {
			MYSQL_FIELD *field = mysql_fetch_field_direct(res, i);

			Json::Value &nItemValue = values[field->name];
			if (NULL == pprow[i]) {
				nItemValue = Json::nullValue;
				continue;
			}

			switch (field->type) {
			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_NEWDECIMAL:
				if (field->decimals > 0) nItemValue = utils::String::Stod((const char *)pprow[i]);
				else if (UNSIGNED_FLAG & field->flags) nItemValue = (Json::UInt)utils::String::Stoui((const char *)pprow[i]);
				else nItemValue = (Json::Int)utils::String::Stoi((const char *)pprow[i]);
				break;

			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_ENUM:
				if (UNSIGNED_FLAG & field->flags) nItemValue = (Json::UInt)utils::String::Stoui((const char *)pprow[i]);
				else nItemValue = (Json::Int)utils::String::Stoi((const char *)pprow[i]);
				break;

			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
				nItemValue = utils::String::Stod((const char *)pprow[i]);
				break;

			case MYSQL_TYPE_LONGLONG:
				if (UNSIGNED_FLAG & field->flags) nItemValue = (Json::UInt64)utils::String::Stoui64((const char *)pprow[i]);
				else nItemValue = (Json::Int64)utils::String::Stoi64((const char *)pprow[i]);
				break;

			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VARCHAR:
			case MYSQL_TYPE_VAR_STRING:
				nItemValue = (const char *)pprow[i];
				break;

			default:
				nItemValue = (const char *)pprow[i]; //Json::nullValue;
				break;
			}
		}
	}


	int32_t MysqlDriver::Query(const std::string &sql, Json::Value &record_array) {
		utils::MutexGuard guard(mutex_);

		int32_t nrows = -1;
		do {
			if (mysql_real_query(session_, sql.c_str(), sql.length()) != 0) {
				break;
			}

			MYSQL_RES *res = NULL;
			do {
				MYSQL_RES *resTmp = mysql_store_result(session_);
				if (resTmp != NULL) {
					if (res != NULL) {
						mysql_free_result(resTmp);
					}
					else {
						res = resTmp;
					}
				}
			} while (!mysql_next_result(session_));

			if (NULL == res) {
				break;
			}

			nrows = 0;
			MYSQL_ROW  pprow = NULL;
			while (pprow = mysql_fetch_row(res)) {
				GetJson(res, pprow, record_array[record_array.size()]);
				nrows++;
			}

			mysql_free_result(res);
			res = NULL;

		} while (false);

		return nrows;
	}

	int32_t MysqlDriver::QueryRecord(const std::string &sql, Json::Value &record) {
		Json::Value record_array = Json::Value(Json::arrayValue);
		int32_t row_count = Query(sql, record_array);
		if (row_count >= 1) {
			record = record_array[(Json::UInt)0];
			return 1;
		}

		return row_count;
	}

	bool MysqlDriver::Update(const std::string &table_name, const utils::StringMap &record, const std::string &sql_where) {
		std::string sets;
		for (utils::StringMap::const_iterator iter = record.begin();
			iter != record.end();
			iter++) {
			if (sets.empty()) {
				sets = utils::String::Format("%s='%s'", Format(iter->first).c_str(), Format(iter->second).c_str());
			}
			else {
				sets = utils::String::Format("%s,%s='%s'", sets.c_str(), Format(iter->first).c_str(), Format(iter->second).c_str());
			}
		}

		std::string sql = utils::String::Format("UPDATE %s SET %s %s", table_name.c_str(), sets.c_str(), sql_where.c_str());
		return Execute(sql);
	}

	void MysqlDriver::SetError(const char *pMessage) {
		if (NULL == pMessage) {
			assert(NULL != session_);
			error_code_ = mysql_errno(session_);
			error_desc_ = mysql_error(session_);
		}
		else {
			error_code_ = -1;
			error_desc_ = pMessage;
		}
	}

	const char *MysqlDriver::error_desc() {
		return error_desc_.c_str();
	}


	int MysqlDriver::error_code() {
		return error_code_;
	}

	int32_t MysqlDriver::DescribeTable(const std::string &table_name, Json::Value &table_desc) {
		std::string sql = utils::String::Format("DESCRIBE `%s`", table_name.c_str());
		int64_t row_count = Query(sql, table_desc);
		if (row_count <= 0) {
			return 0;
		}
		return 1;
	}

	int64_t MysqlDriver::QueryCount(const std::string &table_name, const std::string &condition) {

		std::string sql = utils::String::Format("SELECT COUNT(*) AS count FROM %s %s", table_name.c_str(), condition.c_str());
		Json::Value record;
		int64_t count = QueryRecord(sql, record);
		if (count <= 0) {
			return -1;
		}

		return record["count"].asInt64();
	}

	void MysqlDriver::AppendSql(const std::string &sql) {
		sql_ += sql;
		sql_ += ";";
	}

	bool MysqlDriver::Begin() {
		sql_ = "BEGIN;";
		return true;
	}

	bool MysqlDriver::Commit() {
		sql_ += "COMMIT;";
		return Execute(sql_);
	}
#endif // BUBI_WITH_MYSQL
}