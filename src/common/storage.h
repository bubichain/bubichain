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

#ifndef STORAGE_H_
#define STORAGE_H_

#include <unordered_map>
#include <utils/singleton.h>
#include <utils/thread.h>
#include <utils/strings.h>
#include <json/json.h>
#include "configure.h"
#ifdef WIN32
#include <leveldb/leveldb.h>
#else
#include <rocksdb/db.h>
#endif

#include <libpq-fe.h>
//#include <mysql.h>

namespace bubi {
#ifdef WIN32
#define KVDB leveldb
#define WRITE_BATCH leveldb::WriteBatch
#define WRITE_BATCH_DATA(batch) (((std::string*)(&batch))->c_str())
#define WRITE_BATCH_DATA_SIZE(batch) (((std::string*)(&batch))->size())
#define SLICE leveldb::Slice
#else 
#define KVDB rocksdb
#define WRITE_BATCH rocksdb::WriteBatch
#define WRITE_BATCH_DATA(batch) (batch.Data().c_str())
#define WRITE_BATCH_DATA_SIZE(batch) (batch.GetDataSize())
#define SLICE       rocksdb::Slice
#endif

	class KeyValueDb {
	public:
		KeyValueDb();
		~KeyValueDb();
		virtual bool Open(const std::string &db_path) = 0;
		virtual bool Close() = 0;
		virtual bool Get(const std::string &key, std::string &value) = 0;
		virtual bool Put(const std::string &key, const std::string &value) = 0;
		virtual bool Delete(const std::string &key) = 0;
		virtual bool GetOptions(Json::Value &options) = 0;
		std::string error_desc();
		virtual bool WriteBatch(WRITE_BATCH &values) = 0;
		virtual void* NewIterator() = 0;
	public:
		std::string error_desc_;
	};

#ifdef WIN32
	class LevelDbDriver : public KeyValueDb {
	public:
		LevelDbDriver();
		~LevelDbDriver();
		bool Open(const std::string &db_path);
		bool Close();
		bool Get(const std::string &key, std::string &value);
		bool Put(const std::string &key, const std::string &value);
		bool Delete(const std::string &key);
		bool GetOptions(Json::Value &options);
		bool WriteBatch(WRITE_BATCH &values);
		void* NewIterator();
	private:
		leveldb::DB* db_;
		utils::Mutex mutex_;
	};
#else
	class RocksDbDriver : public KeyValueDb {
	public:
		RocksDbDriver();
		~RocksDbDriver();
		bool Open(const std::string &db_path);
		bool Close();
		bool Get(const std::string &key, std::string &value);
		bool Put(const std::string &key, const std::string &value);
		bool Delete(const std::string &key);
		bool GetOptions(Json::Value &options);
		bool WriteBatch(WRITE_BATCH &values);
		void* NewIterator();
	private:
		rocksdb::DB* db_;
		utils::Mutex mutex_;
	};
#endif

	class RationalDb {
	public:
		RationalDb();
		~RationalDb();
		virtual bool Open(const std::string &db_path) = 0;
		virtual bool Close() = 0;
		virtual bool Execute(const std::string &sql) = 0;
		virtual int32_t Query(const std::string &sql, Json::Value &record_array) = 0;
		virtual int32_t QueryRecord(const std::string &sql, Json::Value &record) = 0;
		virtual int32_t QueryRecord(const std::string &table, const std::string &sqlwhere, Json::Value &record);
		virtual bool Insert(const std::string &table_name, const utils::StringMap &record) = 0;
		virtual bool Update(const std::string &table_name, const utils::StringMap &record, const std::string &sql_where) = 0;
		virtual void SetError(const char *pMessage) = 0;
		virtual const char *error_desc() = 0;
		virtual int error_code() = 0;
		virtual int32_t DescribeTable(const std::string &table_name, Json::Value &columns) = 0;
		virtual bool CreateTable(const std::string &table_name, const Json::Value &columns) = 0;
		virtual int64_t QueryCount(const std::string &table_name, const std::string &condition) = 0;
		virtual std::string Format(const std::string &value);
		virtual bool Begin() = 0;
		virtual bool Commit() = 0;
		virtual void AppendSql(const std::string& sql) = 0;
		utils::Mutex &GetLock() { return mutex_; };
		virtual std::string GetSqls() = 0;
	protected:
		utils::Mutex mutex_;
	};

	class SociDriver : public RationalDb {
	public:
		SociDriver(void);
		~SociDriver();
		bool Open(const std::string &connect_str);
		bool Open(const DbConfigure &cfg);
		bool Insert(const std::string &table_name, const utils::StringMap &record);
		bool Close();
		virtual int32_t Query(const std::string &sql, Json::Value &record_array);
		virtual int32_t QueryRecord(const std::string &sql, Json::Value &record);
		virtual bool Update(const std::string &table_name, const utils::StringMap &record, const std::string &sql_where);
		virtual bool Execute(const std::string &sql);
		virtual void SetError(const char *pMessage);
		virtual const char *error_desc();
		virtual int error_code();
		virtual int32_t DescribeTable(const std::string &table_name, Json::Value &table_desc);
		virtual int64_t QueryCount(const std::string &table_name, const std::string &condition);
		bool CreateTable(const std::string &table_name, const Json::Value &columns);
		virtual void AppendSql(const std::string& sql);
		virtual bool Begin();
		virtual bool Commit();
		std::string GetSqls() override;
	private:
		PGconn *  session_;
		std::string error_desc_;
		int error_code_;
		std::string sql_;
	};

#ifdef BUBI_WITH_MYSQL
	class MysqlDriver : public RationalDb {
	public:
		MysqlDriver(void);
		~MysqlDriver();
		bool Open(const std::string &connect_str);
		bool Insert(const std::string &table_name, const utils::StringMap &record);
		bool Close();
		virtual int32_t Query(const std::string &sql, Json::Value &record_array);
		virtual int32_t QueryRecord(const std::string &sql, Json::Value &record);
		virtual bool Update(const std::string &table_name, const utils::StringMap &record, const std::string &sql_where);
		virtual bool Execute(const std::string &sql);
		virtual void SetError(const char *pMessage);
		virtual const char *error_desc();
		virtual int error_code();
		virtual int32_t DescribeTable(const std::string &table_name, Json::Value &table_desc);
		virtual int64_t QueryCount(const std::string &table_name, const std::string &condition);
		bool CreateTable(const std::string &table_name, const Json::Value &columns);
		virtual void AppendSql(const std::string& sql);
		virtual bool Begin();
		virtual bool Commit();
		std::string GetSqls() override;
		void GetJson(MYSQL_RES *res, MYSQL_ROW pprow, Json::Value &values) const;
	private:
		MYSQL *session_;
		std::string error_desc_;
		int error_code_;
		std::string sql_;
	};
#endif //BUBI_WITH_MYSQL

	class Storage : public utils::Singleton<bubi::Storage> {
		friend class utils::Singleton<Storage>;
	public:
		bool Initialize(const std::string &keyvalue_db_path, const std::string &rational_db_path, const std::string &rational_db_type, bool bdropdb);
		bool Exit();
		KeyValueDb *keyvalue_db();
		RationalDb *rational_db();
		std::shared_ptr<RationalDb> NewRationalDb();
	private:
		Storage();
		~Storage();
		bool CloseDb();
	private:
		KeyValueDb *keyvalue_db_;
		RationalDb *rational_db_;
		std::string ratiional_db_type_;
	};
}

#endif