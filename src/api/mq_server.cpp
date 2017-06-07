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

#include <utils/headers.h>
#include <common/general.h>
#include <common/configure.h>
#include "mq_server.h"
#include "slave/master_service.h"
#include "slave/slave_service.h"
#include "monitor/monitor_master.h"
#include <overlay/peer_manager.h>
#include <ledger/operation_frm.h>

namespace bubi {
    MQServer::MQServer(int32_t send_buffer_size)
        : PipelineServer(BROADCAST),
        send_list_mutex_(),
        send_list_(),
        init_(false), 
        send_buffer_size_(send_buffer_size) {
		AddHandler(ZMQ_CHAIN_HELLO, MQServer::OnChainHello);
		AddHandler(ZMQ_CHAIN_PEER_MESSAGE, MQServer::OnChainPeerMessage);
		AddHandler(ZMQ_CHAIN_SUBMITTRANSACTION, MQServer::OnSubmitTransaction);
	}

	MQServer::~MQServer() {

	}

	bool MQServer::Initialize(MqServerConfigure & mq_server_configure) {

		if (!PipelineServer::Initialize("api_mqs", mq_server_configure.pipeline_configure_.send_address_, mq_server_configure.pipeline_configure_.recv_address_, mq_server_configure.pipeline_configure_.workers_count_)) {
			return false;
		}
		Bind();
		init_ = true;
		return true;
	}

	bool MQServer::Exit() {
		return PipelineServer::Exit();
	}

	bool MQServer::Send(const ZMQTaskType type, const std::string& buf) {
		do {
			if (!init_)
				break;

			utils::WriteLockGuard guard(send_list_mutex_);
			if (send_list_.size() > send_buffer_size_)
				break;

			//	LOG_INFO("mqserver message join the send list");
			send_list_.push_back(std::make_pair(type, buf));
			return true;
		} while (false);
		return false;
	}

	void MQServer::Recv(const ZMQTaskType type, std::string& buf) {
		if (!init_)
			return;

		PipelineServer::Recv(type, buf);
	}

	void MQServer::OnTimer(int64_t current_time) {
		PipelineServer::OnTimer(current_time);

		utils::WriteLockGuard guardw(send_list_mutex_);
		while (send_list_.size() > 0) {
			std::pair<ZMQTaskType, std::string> &pa = send_list_.front();
			PipelineServer::Send(pa.first, pa.second);
			send_list_.pop_front();
		}

	}
	void MQServer::OnSlowTimer(int64_t current_time) {
		PipelineServer::OnSlowTimer(current_time);
	}

	void MQServer::OnChainHello(const char* msg, int len, std::string &reply) {
		protocol::ChainStatus cmsg;
		cmsg.set_bubi_version(General::BUBI_VERSION);
		cmsg.set_ledger_version(General::LEDGER_VERSION);
		cmsg.set_self_addr(PeerManager::Instance().GetPeerNodeAddress());
		cmsg.set_timestamp(utils::Timestamp::HighResolution());
		MQServer::Instance().Send(ZMQ_CHAIN_STATUS, cmsg.SerializeAsString());
	}

	void MQServer::OnChainPeerMessage(const char* msg, int len, std::string &reply) {
		protocol::ChainPeerMessage cpm;
		if (!cpm.ParseFromArray(msg, len)) {
			LOG_ERROR("ChainPeerMessage FromString fail");
			return;
		}

		bubi::PeerManager::Instance().BroadcastPayLoad(cpm);
	}

	void MQServer::OnSubmitTransaction(const char* msg, int len, std::string &reply) {
		protocol::TransactionEnvWrapper tran_env_wrapper;
		protocol::TransactionEnv &tran_env = *tran_env_wrapper.mutable_transaction_env();
		if (!tran_env.ParseFromArray(msg, len)) {

		}

		int64_t active_time = utils::Timestamp::HighResolution();
		Result result;
		result.set_code(protocol::ERRCODE_SUCCESS);
		do {
			//check parameter
			const protocol::Transaction &tx = tran_env.transaction();
			if (!bubi::PublicKey::IsAddressValid(tx.source_address())) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("transaction 'source_address' parameter error");
				break;
			}

			if (tx.fee() < 0) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'fee'  parameter error,'fee' value must be greater than 0");
				break;
			}

			if (tx.sequence_number() <= 0) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'sequence_number' value must be greater than 0");
				break;
			}

			if (tx.has_metadata()) {
				if (tx.metadata().size() > METADATA_MAXSIZE) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("tx 'metadata' value must be Hex,'metadata' value is in the range of 0 and %d", METADATA_MAXSIZE));
					break;
				}
			}

			if (tx.has_close_time_range()) {
				const protocol::CloseTimeRange &time_range = tx.close_time_range();
				if (time_range.mintime() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'min_time' parameter error,value must be greater than 0");
					break;
				}

				if (time_range.maxtime() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'max_time'  parameter error,value must be greater than 0");
					break;
				}
			}


			if (tx.operations_size() <= 0) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("transaction operation not exist!");
				break;
			}

			for (int i = 0; i < tx.operations_size(); i++) {
				const protocol::Operation &ope = tx.operations(i);
				if (ope.has_source_address() &&
					!bubi::PublicKey::IsAddressValid(ope.source_address())) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("operation 'source_address' parameter error");
					break;
				}

				if (ope.has_metadata() &&
					ope.metadata().size() > METADATA_MAXSIZE) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("operation 'metadata' value is in the range of 0 and %d", METADATA_MAXSIZE));
					break;
				}

				switch (ope.type()) {
				case protocol::Operation_Type_CREATE_ACCOUNT:{
					CheckCreateAccountOpe(ope, result);
					break;
				}
				case protocol::Operation_Type_INIT_PAYMENT:{
					CheckInitPayment(ope, result);
					break;
				}
				case protocol::Operation_Type_ISSUE_ASSET:{
					CheckIssueAsset(ope, result);
					break;
				}
				case protocol::Operation_Type_ISSUE_UNIQUE_ASSET:{
					CheckIssueUniqueAsset(ope, result);
					break;
				}
				case protocol::Operation_Type_PAYMENT:{
					CheckPayment(ope, result);
					break;
				}
				case protocol::Operation_Type_PAYMENT_UNIQUE_ASSET:{
					CheckPaymentUniqueAsset(ope, result);
					break;
				}
				case protocol::Operation_Type_PRODUCTION:{
					CheckProduction(ope, result);
					break;
				}
				case protocol::Operation_Type_RECORD:{
					CheckRecord(ope, result);
					break;
				}
				case protocol::Operation_Type_SET_OPTIONS:{
					CheckSetOptions(ope, result);
					break;
				}
				default:
					break;
				}(ope.type());

				if (result.code() != protocol::ERRCODE_SUCCESS) break;
			}
			if (result.code() != protocol::ERRCODE_SUCCESS) break;

		} while (false);

		//commit  result
		std::string transaction_hash;
		std::string transStr = tran_env.transaction().SerializeAsString();
		transaction_hash = utils::Sha256::Crypto(transStr);

		if (result.code() == protocol::ERRCODE_SUCCESS) {
			bubi::PeerMessage  msg;
			msg.header_.type = PeerMessage::PEER_MESSAGE_TRANSACTION;

			msg.data_ = &tran_env_wrapper;
			std::string peerMessage = msg.ToString();
			std::string transEvnStr = tran_env.SerializeAsString();

			protocol::SlaveVerifyResponse sv_rsp;
			sv_rsp.set_peer_message(peerMessage);
			sv_rsp.set_peer_message_hash(utils::Sha256::Crypto(peerMessage));
			sv_rsp.set_transaction_hash(transaction_hash);
			sv_rsp.set_transaction_env_hash(utils::Sha256::Crypto(transEvnStr));

			//check signatures
			for (int i = 0; i < tran_env.signatures_size(); i++) {
				const protocol::Signature &signature = tran_env.signatures(i);

				//check public key
				PublicKey pubkey(signature.public_key());
				if (!pubkey.IsValid()) {
					LOG_ERROR("Invalid publickey (%s)", signature.public_key().c_str());
					continue;
				}
                
                //MQ is internal interface for app layer, it is expected the signature is well signed.
                //skip verifying to improve performance
				bubi::PublicKey pub(signature.public_key());
				sv_rsp.add_address(pub.GetBase58Address());
			}
			if (sv_rsp.address_size() == 0) {
				result.set_code(protocol::ERRCODE_INVALID_PUBKEY);
				result.set_desc("invalid pubkey");
			}
			else {

				std::string slaveTransMsg = sv_rsp.SerializeAsString();
				//Send to MasterService
				bubi::MasterService::GetInstance()->Recv(ZMQ_NEW_TX, slaveTransMsg);
				if (bubi::Configure::Instance().monitor_configure_.real_time_status_) {
					//notice monitor transaction state
					std::shared_ptr<Json::Value> tx_status = std::make_shared<Json::Value>();
					(*tx_status)["type"] = 1;
					(*tx_status)["tx_hash"] = Json::Value(sv_rsp.transaction_hash());
					(*tx_status)["active_time"] = Json::Value(active_time);
					bubi::MonitorMaster::Instance().NoticeMonitor(tx_status->toStyledString());
				}
			}
			msg.data_ = NULL;
		}

		if (bubi::Configure::Instance().mqserver_configure_.tx_status_) {
			//send transaction status notification
			protocol::ChainTxStatus cts;
			cts.set_tx_hash(utils::encode_b16(transaction_hash));
			cts.set_error_code((protocol::ERRORCODE)result.code());
			cts.set_source_address(tran_env.transaction().source_address());
			cts.set_status(result.code() == protocol::ERRCODE_SUCCESS ? protocol::ChainTxStatus_TxStatus_CONFIRMED : protocol::ChainTxStatus_TxStatus_FAILURE);
			cts.set_error_desc(result.desc());
			cts.set_timestamp(utils::Timestamp::Now().timestamp());
			std::string str = cts.SerializeAsString();
			bubi::MQServer::Instance().Send(ZMQ_CHAIN_TX_STATUS, str);
		}
	}

	bool MQServer::CheckCreateAccountOpe(const protocol::Operation &ope, Result &result) {
		if (!ope.has_create_account()) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("protocol create_account not exist");
			return false;
		}
		const protocol::OperationCreateAccount &ope_crea = ope.create_account();

		if (!bubi::PublicKey::IsAddressValid(ope_crea.dest_address())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'dest_address' parameter error");
			return false;
		}

		if (ope_crea.init_balance() < 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'init_balance' parameter error,'init_balance' value must be greater than 0");
			return false;
		}

		if (ope_crea.has_account_metadata() &&
			ope_crea.account_metadata().size() > METADATA_MAXSIZE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'account_metadata' value is in the range of 0 and %d", METADATA_MAXSIZE));
			return false;
		}

		if (ope_crea.has_thresholds()) {
			const protocol::AccountThreshold &acc_threshold = ope_crea.thresholds();
			if (acc_threshold.has_master_weight() &&
				acc_threshold.master_weight() > UINT8_MAX) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'master_weight' parameter error,'master_weight' value is in the range of 0 and %d", UINT8_MAX));
				return false;
			}

			if (acc_threshold.has_low_threshold() &&
				acc_threshold.low_threshold() > UINT8_MAX) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'low_threshold' parameter error,'low_threshold' value is in the range of 0 and %d", UINT8_MAX));
				return false;
			}

			if (acc_threshold.has_med_threshold() &&
				acc_threshold.med_threshold() > UINT8_MAX) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'med_threshold' parameter error,'med_threshold' value is in the range of 0 and %d", UINT8_MAX));
				return false;
			}

			if (acc_threshold.has_high_threshold() &&
				acc_threshold.high_threshold() > UINT8_MAX) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'high_threshold' parameter error,'high_threshold' value is in the range of 0 and %d", UINT8_MAX));
				return false;
			}
		}

		for (int i = 0; i < ope_crea.signers_size(); i++) {
			const protocol::Signer &signer = ope_crea.signers(i);
			if (!bubi::PublicKey::IsAddressValid(signer.address())) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'address' parameter error");
				return false;
			}

			if (signer.weight() < 0 ||
				signer.weight() > UINT8_MAX) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'weight' parameter error,'weight' value is in the range of 0 and %d", UINT8_MAX));
				return false;
			}
		}
		return true;
	}

	bool MQServer::CheckInitPayment(const protocol::Operation &ope, Result &result) {
		if (!ope.has_init_payment()) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("protocol init_payment not exist");
			return false;
		}

		const protocol::OperationInitPayment &init_payment = ope.init_payment();

		if (!bubi::PublicKey::IsAddressValid(init_payment.destaddress())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'dest_address' parameter error");
			return false;
		}

		const protocol::Asset &asset = init_payment.asset();
		if (asset.amount() <= 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_amount' value must be greater than 0");
			return false;
		}

		const protocol::AssetProperty &property = asset.property();

		if (property.type() != protocol::AssetProperty_Type_IOU) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' must be IOU");
			return false;
		}

		if (!bubi::PublicKey::IsAddressValid(property.issuer())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_issuer'  parameter error");
		}

		if (property.code().size() > ASSET_CODE_MAX_SIZE ||
			property.code().size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
			return false;
		}

		if (asset.details_size() > 0) {
			int64_t cont = 0;
			for (int i = 0; i < asset.details_size(); i++) {
				const protocol::Detail &detail = asset.details(i);

				if (detail.amount() <= 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'amount' value must be greater than 0");
					return false;
				}
				cont += detail.amount();

				if (detail.start() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'start' value must be greater than or equal to  0");
					return false;
				}

				if (detail.length() < -1) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'length' value must be greater than or equal to  -1");
					return false;
				}
			}

			if (cont != asset.amount()) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("sum of details 'amount' must be equal to 'asset_amount'");
				return false;
			}
		}

		return true;
	}

	bool MQServer::CheckPayment(const protocol::Operation &ope, Result &result) {
		if (!ope.has_payment()) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("protocol payment not exist");
			return false;
		}

		const protocol::OperationPayment &payment = ope.payment();

		if (!bubi::PublicKey::IsAddressValid(payment.destaddress())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'dest_address' parameter error");
			return false;
		}

		const protocol::Asset &asset = payment.asset();
		if (asset.amount() <= 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_amount' value must be greater than 0");
			return false;
		}

		const protocol::AssetProperty &property = asset.property();

		if (property.type() == protocol::AssetProperty_Type_UNIQUE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' not exist or parameter error");
			return false;
		}

		if (property.code().size() > ASSET_CODE_MAX_SIZE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
			return false;
		}

		if (property.type() == protocol::AssetProperty_Type_IOU) {
			if (!bubi::PublicKey::IsAddressValid(property.issuer())) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'asset_issuer'  parameter error");
				return false;
			}

			if (property.code().size() == 0) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
				return false;
			}
		}

		if (asset.details_size() > 0) {
			int64_t cont = 0;
			for (int i = 0; i < asset.details_size(); i++) {
				const protocol::Detail &detail = asset.details(i);

				if (detail.amount() <= 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'amount' value must be greater than 0");
					return false;
				}
				cont += detail.amount();

				if (detail.start() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'start' value must be greater than or equal to  0");
					return false;
				}

				if (detail.length() < -1) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'length' value must be greater than or equal to  -1");
					return false;
				}
			}

			if (cont != asset.amount()) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("sum of details 'amount' must be equal to 'asset_amount'");
				return false;
			}
		}
		return true;
	}

	bool MQServer::CheckIssueAsset(const protocol::Operation &ope, Result &result) {
		if (!ope.has_issue_asset()) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("issue operation not exist");
			return false;
		}

		const protocol::Asset &asset = ope.issue_asset().asset();
		if (asset.amount() <= 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_amount' parameter error,'asset_amount' value must be greater than 0");
			return false;
		}
		const protocol::AssetProperty &property = asset.property();
		if (property.type() != protocol::AssetProperty_Type_IOU) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' must be IOU");
			return false;
		}

		if (property.code().size() > ASSET_CODE_MAX_SIZE ||
			property.code().size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
			return false;
		}

		if (!bubi::PublicKey::IsAddressValid(property.issuer())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_issuer' parameter error");
			return false;
		}

		if (asset.details_size() > 0) {
			int64_t cont = 0;
			for (int i = 0; i < asset.details_size(); i++) {
				const protocol::Detail &detail = asset.details(i);

				if (detail.amount() <= 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'amount' value must be greater than 0");
					return false;
				}
				cont += detail.amount();

				if (detail.start() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'start' value must be greater than or equal to  0");
					return false;
				}

				if (detail.length() < -1) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("details 'length' value must be greater than or equal to  -1");
					return false;
				}
			}

			if (cont != asset.amount()) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("sum of details 'amount' must be equal to 'asset_amount'");
				return false;
			}
		}
		return true;
	}

	bool MQServer::CheckIssueUniqueAsset(const protocol::Operation &ope, Result &result) {
		if (!ope.has_issue_unique_asset()) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("protocol issue_unique_asset not exist");
			return false;
		}

		const protocol::UniqueAsset &asset = ope.issue_unique_asset().asset();

		if (asset.detailed().size() > (DETAILED_MAX_SIZE / 2)) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_detailed' value is in the range of 0 and %d", DETAILED_MAX_SIZE));
			return false;
		}

		const protocol::AssetProperty &property = asset.property();
		if (property.type() != protocol::AssetProperty_Type_UNIQUE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' must be UNIQUE");
			return false;
		}

		if (property.code().size() > ASSET_CODE_MAX_SIZE ||
			property.code().size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
			return false;
		}

		if (!bubi::PublicKey::IsAddressValid(property.issuer())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_issuer' parameter error");
			return false;
		}

		return true;
	}

	bool MQServer::CheckPaymentUniqueAsset(const protocol::Operation &ope, Result &result) {
		if (!ope.has_payment_unique_asset()) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("protocol payment_unique_asset not exist");
			return false;
		}

		const protocol::OperationPaymentUniqueAsset &payment_unique_asset = ope.payment_unique_asset();

		if (!bubi::PublicKey::IsAddressValid(payment_unique_asset.destaddress())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'dest_address' parameter error");
			return false;
		}

		const protocol::AssetProperty &property = payment_unique_asset.asset_pro();
		if (property.type() != protocol::AssetProperty_Type_UNIQUE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_type' must be UNIQUE");
			return false;
		}

		if (property.code().size() > ASSET_CODE_MAX_SIZE ||
			property.code().size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'asset_code' value is in the range of 1 and %d", ASSET_CODE_MAX_SIZE));
			return false;
		}

		if (!bubi::PublicKey::IsAddressValid(property.issuer())) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'asset_issuer' parameter error");
			return false;
		}

		return true;
	}

	bool MQServer::CheckSetOptions(const protocol::Operation &ope, Result &result) {

		const protocol::OperationSetOptions &setoptions = ope.setoptions();

		if (setoptions.has_high_threshold() &&
			(setoptions.high_threshold() > UINT8_MAX || setoptions.high_threshold() < 0)) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'high_threshold' parameter error,'high_threshold' value is in the range of 0 and %d", UINT8_MAX));
			return false;
		}

		if (setoptions.has_med_threshold() &&
			(setoptions.med_threshold() > UINT8_MAX || setoptions.med_threshold() < 0)) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'med_threshold' parameter error,'med_threshold' value is in the range of 0 and %d", UINT8_MAX));
			return false;
		}

		if (setoptions.has_low_threshold() &&
			(setoptions.low_threshold() > UINT8_MAX || setoptions.low_threshold() < 0)) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'low_threshold' parameter error,'low_threshold' value is in the range of 0 and %d", UINT8_MAX));
			return false;
		}

		if (setoptions.has_master_weight() &&
			(setoptions.master_weight() > UINT8_MAX || setoptions.master_weight() < 0)) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'master_weight' parameter error,'master_weight' value is in the range of 0 and %d", UINT8_MAX));
			return false;
		}

		if (setoptions.has_account_metadata_version() &&
			setoptions.account_metadata_version() < 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'metadata_version'  parameter error");
			return false;
		}

		if (setoptions.has_account_metadata() &&
			setoptions.account_metadata().size() > METADATA_MAXSIZE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'metadata' value is in the range of 0 and %d", METADATA_MAXSIZE));
			return false;
		}

		if (setoptions.signers_size() > 0) {
			for (int i = 0; i < setoptions.signers_size(); i++) {
				const protocol::Signer &sig = setoptions.signers(i);

				if (!bubi::PublicKey::IsAddressValid(sig.address())) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc("'address' parameter error");
					return false;
				}

				if (sig.weight() > UINT8_MAX ||
					sig.weight() < 0) {
					result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result.set_desc(utils::String::Format("'weight' parameter error,'weight' value is in the range of 0 and %d", UINT8_MAX));
					return false;
				}
			}
		}
		return true;
	}

	bool MQServer::CheckProduction(const protocol::Operation &ope, Result &result) {
		if (!ope.has_production()) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("protocol production not exist");
			return false;
		}

		const protocol::OperationProduction &production = ope.production();

		for (int32_t i = 0; i < production.inputs_size(); i++) {
			const protocol::Input &input = production.inputs(i);
			if (input.hash().size() != 32) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'hash' parameter error");
				return false;
			}

			if (input.index() < 0) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'index' value must be greater than or equal to 0");
			}

			if (input.has_metadata() &&
				input.metadata().size() > METADATA_MAXSIZE) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'metadata' size is in the range of 0 and %d", METADATA_MAXSIZE));
				return false;
			}
		}

		if (production.outputs_size() == 0) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("'outputs' not exist");
			return false;
		}

		for (int32_t i = 0; i < production.outputs_size(); i++) {
			const protocol::Output &output = production.outputs(i);

			if (!bubi::PublicKey::IsAddressValid(output.address())) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'address' parameter error");
				return false;
			}

			if (output.metadata().size() >METADATA_MAXSIZE) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc(utils::String::Format("'metadata' size is in the range of 0 and %d", METADATA_MAXSIZE));
				return false;
			}
		}
		return true;
	}

	bool MQServer::CheckRecord(const protocol::Operation &ope, Result &result) {
		if (!ope.has_record()) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc("protocol record not exist");
			return false;
		}

		const protocol::OperationRecord &record = ope.record();

		if (record.id().size() == 0 ||
			record.id().size() > RECORD_ID_MAX_SIZE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'record_id' parameter error,'record_id' value is in the range of 1 and %d", RECORD_ID_MAX_SIZE));
			return false;
		}

		if (record.ext().size() > METADATA_MAXSIZE) {
			result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result.set_desc(utils::String::Format("'metadata' size is in the range of 0 and %d", METADATA_MAXSIZE));
			return false;
		}

		if (record.has_address()) {
			if (!bubi::PublicKey::IsAddressValid(record.address())) {
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("'record_address' parameter error");
				return false;
			}
		}
		return true;
	}
}