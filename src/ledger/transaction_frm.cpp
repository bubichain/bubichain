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

#include <utils/crypto.h>
#include <common/configure.h>
#include <common/storage.h>
#include <ledger/ledger_manager.h>
#include "createaccount_ope_frm.h"
#include "issue_asset_ope_frm.h"
#include "issue_unique_asset_ope_frm.h"
#include "payment_ope_frm.h"
#include "payment_unique_asset_ope_frm.h"
#include "record_ope_frm.h"
#include "init_payment_ope_frm.h"
#include "setoptions_ope_frm.h"
#include "transaction_frm.h"
#include "setoptions_ope_frm.h"
#include "production_ope_frm.h"

namespace bubi {
	TransactionFrm::TransactionFrm() :
		suggestion_seq_(0),
		time_use_(0),
		apply_time_(0),
		result_(),
		transaction_env_(),
		hash_(),
		full_hash_(),
		data_(),
		full_data_(),
		check_valid_(false),
		valid_signature_(),
		insert_sql_(),
		header_(),
		incoming_time_(utils::Timestamp::HighResolution()),
		txset_hash_() {
		utils::AtomicInc(&bubi::General::tx_new_count);
	}

	TransactionFrm::TransactionFrm(const protocol::TransactionEnv &env) :
		suggestion_seq_(0),
		time_use_(0),
		apply_time_(0),
		result_(),
		transaction_env_(env),
		hash_(),
		full_hash_(),
		data_(),
		full_data_(),
		check_valid_(false),
		valid_signature_(),
		insert_sql_(),
		header_(),
		incoming_time_(utils::Timestamp::HighResolution()),
		txset_hash_() {

		Initialize();
		utils::AtomicInc(&bubi::General::tx_new_count);
	}

	TransactionFrm::TransactionFrm(const protocol::TransactionEnvWrapper &env_wrapper, const protocol::SlaveVerifyResponse &sv_rsp) :
		suggestion_seq_(env_wrapper.suggest_ledger_seq()),
		time_use_(0),
		apply_time_(0),
		result_(),
		transaction_env_(env_wrapper.transaction_env()),
		hash_(),
		full_hash_(),
		data_(),
		full_data_(),
		check_valid_(false),
		valid_signature_(),
		insert_sql_(),
		header_(),
		incoming_time_(utils::Timestamp::HighResolution()),
		txset_hash_() {

		data_ = transaction_env_.transaction().SerializeAsString();
		hash_ = sv_rsp.transaction_hash();
		full_data_ = transaction_env_.SerializeAsString();
		full_hash_ = sv_rsp.transaction_env_hash();
		txset_hash_ = sv_rsp.txset_hash();

		for (int i = 0; i < sv_rsp.address_size(); i++) {
			valid_signature_.insert(sv_rsp.address(i));
		}

		utils::AtomicInc(&bubi::General::tx_new_count);
		incoming_time_ = utils::Timestamp::HighResolution();
	}

	TransactionFrm::~TransactionFrm() {
		utils::AtomicInc(&bubi::General::tx_delete_count);
	}

	protocol::TransactionEnv &TransactionFrm::GetProtoTxEnv() {
		return transaction_env_;
	}
	int64_t TransactionFrm::GetLedgerSeq() {
		return header_->ledger_sequence();
	}
	void TransactionFrm::SetHeader(std::shared_ptr<protocol::LedgerHeader> header) {
		header_ = header;
	}
	const std::shared_ptr<protocol::LedgerHeader> TransactionFrm::GetHeader() const {
		return header_;
	}

	void TransactionFrm::ToJson(Json::Value &result) {
		const protocol::Transaction &tx = transaction_env_.transaction();

		Json::Value &signatures = result["signatures"];
		for (int i = 0; i < transaction_env_.signatures_size(); i++) {
			Json::Value item;
			item["public_key"] = transaction_env_.signatures(i).public_key();
			item["sign_data"] = utils::String::BinToHexString(transaction_env_.signatures(i).sign_data());
			PublicKey mykey(transaction_env_.signatures(i).public_key());
			item["address"] = mykey.GetBase58Address();
			signatures[i] = item;
		}
		Json::Value &operations = result["operations"];
		result["hash"] = utils::String::BinToHexString(hash_);
		result["fee"] = tx.fee();
		result["metadata"] = utils::String::BinToHexString(tx.metadata());
		result["source_address"] = tx.source_address();
		result["sequence_number"] = tx.sequence_number();
		result["error_code"] = result_.code();
		result["apply_time"] = result_.close_time_;
		result["ledger_seq"] = result_.ledger_seq_;
		for (int i = 0; i < tx.operations_size(); i++) {
			const protocol::Operation &operation = tx.operations(i);
			Json::Value item;
			protocol::Operation_Type type = operation.type();
			item["type"] = type;
			if (operation.has_source_address()) {
				item["source_address"] = operation.source_address();
			}
			if (operation.has_metadata()) {
				item["metadata"] = utils::String::BinToHexString(operation.metadata());
			}

			switch (type) {
			case protocol::Operation_Type_CREATE_ACCOUNT:
			{
				protocol::OperationCreateAccount ope = operation.create_account();
				CreateAccountOpeFrm::ToJson(ope, item);
				break;
			}
			case protocol::Operation_Type_PAYMENT:
			{
				protocol::Asset asset = operation.payment().asset();
				item["dest_address"] = operation.payment().destaddress();
				WebServer::AssetToJson(asset, item);
				break;
			}
			case protocol::Operation_Type_INIT_PAYMENT:
			{
				const protocol::Asset &asset = operation.init_payment().asset();
				item["dest_address"] = operation.init_payment().destaddress();
				WebServer::AssetToJson(asset, item);
				break;
			}
			case protocol::Operation_Type_ISSUE_ASSET:
			{
				protocol::Asset asset = operation.issue_asset().asset();
				WebServer::AssetToJson(asset, item);
				break;
			}
			case protocol::Operation_Type_ISSUE_UNIQUE_ASSET:
			{
				protocol::UniqueAsset unique_asset = operation.issue_unique_asset().asset();
				item["asset_issuer"] = unique_asset.property().issuer();
				item["asset_code"] = unique_asset.property().code();
				item["asset_detailed"] = utils::String::BinToHexString(unique_asset.detailed());
				break;
			}
			case protocol::Operation_Type_PAYMENT_UNIQUE_ASSET:
			{
				protocol::AssetProperty asset_pro = operation.payment_unique_asset().asset_pro();
				item["asset_issuer"] = asset_pro.issuer();
				item["asset_code"] = asset_pro.code();
				item["dest_address"] = operation.payment_unique_asset().destaddress();
				break;
			}
			case protocol::Operation_Type_SET_OPTIONS:
			{
				protocol::OperationSetOptions ope = operation.setoptions();
				SetOptionsOpeFrm::ToJson(ope, item);
				ope.signers();
				break;
			}
			case protocol::Operation_Type_PRODUCTION:
			{
				protocol::OperationProduction ope = operation.production();
				ProductionFrm::ToJson(item, ope);
				break;
			}
			case protocol::Operation_Type_RECORD:
			{
				protocol::OperationRecord ope = operation.record();
				item["record_id"] = ope.id();
				item["record_ext"] = utils::String::BinToHexString(ope.ext());
				if (ope.has_address()) {
					item["record_address"] = ope.address();
				}
			}
			default:
			{
				break;
			}
			}
			operations[operations.size()] = item;
		}
	}

	void TransactionFrm::Initialize() {
		const protocol::Transaction &tran = transaction_env_.transaction();
		data_ = tran.SerializeAsString();
		hash_ = utils::Sha256::Crypto(data_);
		full_data_ = transaction_env_.SerializeAsString();
		full_hash_ = utils::Sha256::Crypto(full_data_);

		for (int32_t i = 0; i < transaction_env_.signatures_size(); i++) {
			const protocol::Signature &signature = transaction_env_.signatures(i);
			PublicKey pubkey(signature.public_key());

			if (!pubkey.IsValid()) {
				LOG_ERROR("Invalid publickey(%s)", signature.public_key().c_str());
				continue;
			}
			valid_signature_.insert(pubkey.GetBase58Address());
		}
	}

	std::string TransactionFrm::GetContentHash() const {
		return hash_;
	}

	std::string TransactionFrm::GetContentData() const {
		return data_;
	}

	std::string TransactionFrm::GetFullHash() const {
		return full_hash_;
	}

	const protocol::TransactionEnv &TransactionFrm::GetTransactionEnv() const {
		return transaction_env_;
	}

	std::string TransactionFrm::GetSourceAddress() const {
		const protocol::Transaction &tran = transaction_env_.transaction();
		return tran.source_address();
	}

	int64_t TransactionFrm::GetSequnce() const {
		return transaction_env_.transaction().sequence_number();
	}

	AccountFrm::pointer TransactionFrm::GetSourceAccount() const {
		return account_;
	}

	float TransactionFrm::GetFeeRatio() const {
		return ((float)GetFee() / (float)GetMinFee());
	}

	uint32_t TransactionFrm::GetFee() const {
		return transaction_env_.transaction().fee();
	}

	uint32_t TransactionFrm::GetMinFee() const {
		size_t count = transaction_env_.transaction().operations_size();

		if (count == 0) {
			count = 1;
		}
		protocol::LedgerHeader lcl = LedgerManager::Instance().GetLastClosedLedger();
		return lcl.base_fee() *count;
	}

	bool TransactionFrm::ValidForApply(std::shared_ptr<protocol::LedgerHeader> header, AccountEntry &entry) {


		SetHeader(header);
		std::string str_address = GetSourceAddress();
		if (!entry.GetEntry(str_address, account_)) {
			LOG_ERROR("Source account(%s) does not exist", str_address.c_str());
			result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
			return false;
		}

		int64_t last_seq = account_->GetAccountTxSeq();
		if (last_seq + 1 != GetSequnce()) {
			LOG_ERROR("Account(%s) Tx sequence(" FMT_I64 ") doesnot match reserve sequence (" FMT_I64 " + 1)",
				str_address.c_str(),
				GetSequnce(),
				last_seq);
			result_.set_code(protocol::ERRCODE_BAD_SEQUENCE);
			return false;
		}

		if (!CheckCommon())
			return false;

		const protocol::Transaction &tran = transaction_env_.transaction();
		int64_t minfee = header_->base_fee()*tran.operations_size();
		if (tran.fee() < minfee) {
			std::string error_desc = utils::String::Format(
				"Receive transaction seq_number=" FMT_I64 " fee(%u<%d) not enought",
				tran.sequence_number(), tran.fee(), minfee);

			result_.set_code(protocol::ERRCODE_FEE_NOT_ENOUGH);
			result_.set_desc(error_desc);
			LOG_ERROR("%s", error_desc.c_str());
			return false;
		}

		if (account_->GetAccountBalance() - minfee < (int64_t)header_->base_reserve()) {
			std::string error_desc = utils::String::Format(
				"Account reserve balance not enough for transaction fee and base reserve:" FMT_I64 " - " FMT_I64 " < %u",
				account_->GetAccountBalance(), tran.fee(), header_->base_reserve());
			result_.set_code(protocol::ERRCODE_ACCOUNT_LOW_RESERVE);
			result_.set_desc(error_desc);
			LOG_ERROR("%s", error_desc.c_str());
			return false;
		}

		//judge current ledger close time
		if (tran.has_close_time_range()) {

			const protocol::CloseTimeRange &range = tran.close_time_range();
			int64_t min_time = range.mintime();
			int64_t max_time = range.maxtime();

			uint64_t last_close_time = header_->consensus_value().close_time();
			if ((uint64_t)min_time > last_close_time) {
				LOG_ERROR("Receive transaction time=" FMT_I64 " < " FMT_I64 " is too early",
					last_close_time, min_time);
				result_.set_code(protocol::ERRCODE_TIMEBOUND_TOO_EARLY);
				return false;
			}

			if ((uint64_t)max_time < last_close_time) {
				LOG_ERROR("Receive transaction time=" FMT_I64 " > " FMT_I64 " is too late",
					last_close_time, max_time);
				result_.set_code(protocol::ERRCODE_TIMEBOUND_TOO_LATE);
				return false;
			}
		}

		return true;
	}

	bool TransactionFrm::CheckValid(int64_t last_seq) {
		if (!LedgerManager::Instance().GetAccountEntry(GetSourceAddress(), account_)) {
			LOG_ERROR("Source account(%s) does not exists", GetSourceAddress().c_str());
			result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
			return false;
		}

		if (GetSequnce() <= account_->GetAccountTxSeq()) {
			result_.set_code(protocol::ERRCODE_BAD_SEQUENCE);
			return false;
		}

		if (!CheckCommon())
			return false;

		if (last_seq == 0 && GetSequnce() != account_->GetAccountTxSeq() + 1) {
			LOG_ERROR("Account(%s) tx sequence(" FMT_I64 ")  doesnot match  reserve sequence (" FMT_I64 " + 1), txhash(%s)",
				GetSourceAddress().c_str(),
				GetSequnce(),
				account_->GetAccountTxSeq(),
				utils::String::Bin4ToHexString(GetContentHash()).c_str());
			result_.set_code(protocol::ERRCODE_BAD_SEQUENCE);
			return false;
		}

		if (last_seq > 0 && (GetSequnce() != last_seq + 1)) {
			LOG_ERROR("Account(%s) Tx sequence(" FMT_I64 ")  doesnot match  reserve sequence (" FMT_I64 " + 1)",
				GetSourceAddress().c_str(),
				GetSequnce(),
				last_seq);
			result_.set_code(protocol::ERRCODE_BAD_SEQUENCE);
			return false;
		}

		protocol::LedgerHeader lcl = LedgerManager::Instance().GetLastClosedLedger();

		const protocol::Transaction &tran = transaction_env_.transaction();

		if (tran.fee() < GetMinFee()) {
			std::string error_desc = utils::String::Format("Receive transaction seq_number=" FMT_I64 " fee(%u<%u) not enought",
				tran.sequence_number(), tran.fee(), GetMinFee());
			result_.set_code(protocol::ERRCODE_FEE_NOT_ENOUGH);
			result_.set_desc(error_desc);
			LOG_ERROR("%s", error_desc.c_str());
			return false;
		}

		if (account_->GetAccountBalance() - (int64_t)GetFee() < (int64_t)lcl.base_reserve()) {
			std::string error_desc = utils::String::Format("Account reserve balance not enough for transaction fee and base reserve:" FMT_I64 " - " FMT_I64 " < %u",
				account_->GetAccountBalance(), GetFee(), lcl.base_reserve());
			result_.set_code(protocol::ERRCODE_ACCOUNT_LOW_RESERVE);
			result_.set_desc(error_desc);
			LOG_ERROR("%s", error_desc.c_str());
			return false;
		}
		return true;
	}

	bool TransactionFrm::CheckCommon() {
		const protocol::Transaction &tran = transaction_env_.transaction();
		const LedgerConfigure &ledger_config = Configure::Instance().ledger_configure_;
		
		if (tran.operations_size() == 0) {
			LOG_ERROR("Operation size is zero");
			result_.set_code(protocol::ERRCODE_MISSING_OPERATIONS);
			check_valid_ = false;
			return check_valid_;
		}

		if (tran.has_metadata()) {
			if (tran.metadata().size() > METADATA_MAXSIZE) {
				result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result_.set_desc("metadata too long");
				LOG_ERROR("%s", result_.desc().c_str());
				check_valid_ = false;
				return check_valid_;
			}
		}

		check_valid_ = true;
		int64_t t8 = utils::Timestamp::HighResolution();
		bool b_supply = false;
		for (int i = 0; i < tran.operations_size(); i++) {
			protocol::Operation ope = tran.operations(i);
			std::string ope_source = ope.has_source_address() ? ope.source_address() : GetSourceAddress();
			if (!PublicKey::IsAddressValid(ope_source)) {
				check_valid_ = false;
				result_.set_code(protocol::ERRCODE_INVALID_ADDRESS);
				result_.set_desc("invalid operation source address");
				LOG_ERROR("Invalid operation source address");
				break;
			}

			if (ope.has_metadata()) {
				if (ope.metadata().size()>METADATA_MAXSIZE) {
					check_valid_ = false;
					result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result_.set_desc("metadata too long");
					LOG_ERROR("%s", result_.desc().c_str());
					break;
				}
			}
			switch (ope.type()) {
			case protocol::Operation_Type_CREATE_ACCOUNT:
				result_.set_code(CreateAccountOpeFrm::CheckValid(ope, ope_source));
				break;
			case protocol::Operation_Type_INIT_PAYMENT:
				result_.set_code(InitPaymentOpeFrm::CheckValid(ope, ope_source));
				break;
			case protocol::Operation_Type_SET_OPTIONS:
				result_.set_code(SetOptionsOpeFrm::CheckValid(ope, ope_source));
				break;
			case protocol::Operation_Type_ISSUE_ASSET:
				result_.set_code(IssueOpeFrm::CheckValid(ope, ope_source));
				break;
			case protocol::Operation_Type_PAYMENT:
				result_.set_code(PaymentOpeFrm::CheckValid(ope, ope_source));
				break;
			case protocol::Operation_Type_PRODUCTION:
				result_.set_code(ProductionFrm::CheckValid(ope, ope_source));
				b_supply = true;
				break;
			case protocol::Operation_Type_ISSUE_UNIQUE_ASSET:
				result_.set_code(IssueUniqueAssetOpeFrm::CheckValid(ope, ope_source));
				break;
			case protocol::Operation_Type_PAYMENT_UNIQUE_ASSET:
				result_.set_code(PaymentUniqueAssetOpeFrm::CheckValid(ope, ope_source));
				break;
			case protocol::Operation_Type_RECORD:
				result_.set_code(RecordOpeFrm::CheckValid(ope, ope_source));
				break;
			default:
				result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				break;
			}
			if (result_.code() != protocol::ERRCODE_SUCCESS) {
				check_valid_ = false;
				break;
			}
		}
		if (b_supply && tran.operations_size() > 1) {
			result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result_.set_desc("supply operation count greater than one");
			check_valid_ = false;
		}

		if (!VerifyWeight(account_, account_->GetProtoLowThreshold())) {
			LOG_ERROR("Tx(%s) signatures not enough weight", utils::String::BinToHexString(hash_).c_str());
			result_.set_code(protocol::ERRCODE_INVALID_SIGNATURE);
			check_valid_ = false;
		}

		return check_valid_;
	}

	bool TransactionFrm::VerifyWeight(AccountFrm::pointer account_ptr, int32_t needed_weight) const {
		const protocol::Transaction &tran = transaction_env_.transaction();
		const protocol::Account &account_proto = account_ptr->GetProtoAccount();
		bool all_sign = valid_signature_.find(account_ptr->GetAccountAddress()) != valid_signature_.end();
		if (all_sign) needed_weight -= account_proto.thresholds().master_weight();

		for (int32_t i = 0; i < account_proto.signers_size(); i++) {
			const protocol::Signer &signer = account_proto.signers(i);
			if (valid_signature_.find(signer.address()) == valid_signature_.end()) {
				all_sign = false;
			}
			else {
				needed_weight -= signer.weight();
			}
			if (needed_weight <= 0) {
				return true;
			}
		}
		if (all_sign || needed_weight <= 0)
			return true;
		return false;
	}

	Result TransactionFrm::GetResult() const {
		return result_;
	}

	const protocol::Transaction & TransactionFrm::GetTx() const {
		const protocol::Transaction &tran = transaction_env_.transaction();
		return tran;
	};


	uint32_t TransactionFrm::LoadFromDb(const std::string &hash) {

		RationalDb *db = Storage::Instance().rational_db();
		std::string sql = utils::String::Format("SELECT * FROM %s WHERE hash='%s'", General::TX_TABLE_NAME, hash.c_str());
		Json::Value record;
		int res = db->QueryRecord(sql, record);
		if (res < 0) {
			LOG_ERROR("%s %d %s", sql.c_str(),
				db->error_code(), db->error_desc());
			return protocol::ERRCODE_INTERNAL_ERROR;
		}
		else if (res == 0) {
			LOG_ERROR("Tx(%s) not exist", utils::String::BinToHexString(hash).c_str());
			return protocol::ERRCODE_NOT_EXIST;
		}

		std::string str_body = record["body"].asString();
		apply_time_ = record["apply_time"].asUInt64();
		std::string str_decode;
		if (!utils::String::HexStringToBin(str_body, str_decode)) {
			LOG_ERROR("Decode tx(%s) body failed", hash.c_str());
			return protocol::ERRCODE_INTERNAL_ERROR;
		}

		if (!transaction_env_.ParseFromString(str_decode)) {
			LOG_ERROR("Tx(%s) parseFromString failed", hash.c_str());
			return protocol::ERRCODE_INTERNAL_ERROR;
		}
		result_.ledger_seq_ = record["ledger_seq"].asInt64();
		result_.close_time_ = record["apply_time"].asInt64();
		Initialize();
		result_.set_code(record["error_code"].asInt());
		result_.set_desc(record["result"].asString());
		return result_.code();
	}

	void TransactionFrm::GetSql(int64_t ledger_seq, int64_t seq_in_global, std::string &sql_tx, std::string &account_tx, std::string unique_asset) {
		sql_tx += utils::String::Format(
			"INSERT INTO %s (hash,from_account,ledger_seq,seq,body,result,error_code,seq_in_global,apply_time) VALUES "\
			"('%s', '%s', " FMT_I64 ", " FMT_I64 ", '%s', '%s', %d, " FMT_I64 ", " FMT_U64 ");",
			General::TX_TABLE_NAME,
			utils::encode_b16(hash_).c_str(),
			GetSourceAddress().c_str(),
			ledger_seq,
			GetSequnce(),
			utils::encode_b16(full_data_).c_str(),
			bubi::Storage::Instance().rational_db()->Format(result_.desc()).c_str(),
			result_.code(),
			seq_in_global,
			header_->consensus_value().close_time());


		int64_t seqnum = GetSequnce();
		std::string b16_hash = utils::encode_b16(GetContentHash());
		account_tx += utils::String::Format("INSERT INTO %s (trans_id,account,ledger_seq, txn_seq, apply_time,seq_in_global) VALUES ", General::ACCOUNT_TX_NAME);
		for (auto it = account_tx_.begin(); it != account_tx_.end(); it++) {
			std::string address = *it;
			if (!address.empty()) {
				account_tx += utils::String::Format(
					"('%s','%s'," FMT_I64 "," FMT_I64 "," FMT_I64 "," FMT_I64 "),",
					b16_hash.c_str(),
					address.c_str(),
					GetLedgerSeq(),
					seqnum,
					header_->consensus_value().close_time(),
					seq_in_global);
			}
		}
		account_tx[account_tx.size() - 1] = ';';
		account_tx += "\n";
		sql_tx += "\n";
	}


	bool TransactionFrm::CheckTimeout(int64_t expire_time) {
		if (incoming_time_ < expire_time)
			return true;
		result_.set_code(protocol::ERRCODE_TX_TIMEOUT);
		return false;
	}


	void TransactionFrm::PayFee() {
		std::string str_hash = GetContentHash();

		int64_t fee = (int64_t)GetFee();
		account_->AddBalance(-1 * fee);
		account_->SetPreviousLedgerSeq(GetLedgerSeq());
		account_->SetPreviousTxHash(str_hash);
		account_->GetProtoAccount().set_tx_seq(account_->GetAccountTxSeq() + 1);
	}

	bool TransactionFrm::Apply(std::shared_ptr<protocol::LedgerHeader> header, AccountEntry &entry) {

		bool bSucess = true;

		const protocol::Transaction &tran = transaction_env_.transaction();
		for (int32_t i = 0; i < tran.operations_size(); i++) {
			const protocol::Operation &ope = tran.operations(i);
			std::shared_ptr<OperationFrm> opt = OperationFrm::CreateOperation(ope, *this, entry, i);
			if (opt == nullptr) {
				LOG_ERROR("Create operation frame failed");
				result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				bSucess = false;
				break;
			}

			if (!opt->CheckSignature()) {
				LOG_ERROR("Check signature operation frame failed, txhash(%s)", utils::String::Bin4ToHexString(GetContentHash()).c_str());
				result_ = opt->GetResult();
				bSucess = false;
				break;
			}

			opt->SourceRelationTx();
			if (!opt->Apply(entry)) {
				result_ = opt->GetResult();
				bSucess = false;
				LOG_ERROR("Transaction(%s) operation(%d) apply failed",
					utils::String::BinToHexString(hash_).c_str(), i);
				break;
			}
		}
		account_tx_.insert(GetSourceAddress());
		return bSucess;
	}
}
