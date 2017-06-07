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
#include <common/private_key.h>
#include "master_service.h"
#include "slave_executor.h"
#include "overlay/peer_manager.h"
#include "glue/glue_manager.h"

namespace bubi {
	MasterService::MasterService() :
		PipelineServer(PipelineServer::ROUND_ROBIN) {
		AddHandler(ZMQ_NEW_TX, OnNewTransaction);
		AddHandler(ZMQ_BROADCAST_TX, OnBroadcastTransaction);
		AddHandler(ZMQ_BROADCAST_TX_VERIFY, OnBroadcastTransactionVerify);
		AddHandler(ZMQ_TXSET_TX, OnTxSetTransaction);
		AddHandler(ZMQ_TXSET_TX_VERIFY, OnTxSetTransactionVerify);
	}

	MasterService::~MasterService() {}

	bool MasterService::Initialize() {
		std::string pushAddress = Configure::Instance().slave_configure_.send_address_;
		std::string pullAddress = Configure::Instance().slave_configure_.recv_address_;
		int32_t workerCount = Configure::Instance().slave_configure_.workers_count_;

		return PipelineServer::Initialize("master", pushAddress, pullAddress, workerCount);
	}

	bool MasterService::Exit() {
		return PipelineServer::Exit();
	}

	bool MasterService::SendToSlave(const ZMQTaskType type, const std::string& buf) {
		//only PeerIOService Thread invokes this method
		if (send_num_ > 0 && Send(type, buf)) {
			return true;
		}

		switch (type) {
		case ZMQ_BROADCAST_TX:
			Recv(ZMQ_BROADCAST_TX_VERIFY, buf);
			break;
		case ZMQ_TXSET_TX:
			Recv(ZMQ_TXSET_TX_VERIFY, buf);
			break;
		default:
			LOG_ERROR("Unknown type of zmq message");
			break;
		}

		return true;
	}

	void MasterService::OnBroadcastTransactionVerify(const char* msg, int len, std::string &reply) {
		SlaveExecutor::ProcessBroadcastTransaction(msg, len, reply);

		//verify success, continue to call OnNewTransaction in the same worker thread
		if (reply.length() > 0) {
			std::string unused;
			OnBroadcastTransaction(reply.c_str(), reply.length(), unused);
		}
	}

	void MasterService::OnTxSetTransactionVerify(const char* msg, int len, std::string &reply) {
		SlaveExecutor::ProcessTxSetTransaction(msg, len, reply);

		//verify success, continue to call OnNewTransaction in the same worker thread
		if (reply.length() > 0) {
			std::string unused;
			OnTxSetTransaction(reply.c_str(), reply.length(), unused);
		}
	}

	void MasterService::OnNewTransaction(const char* msg, int len, std::string &reply) {
		static int xyz = 0;
		static int64_t startTime = utils::Timestamp::HighResolution();
		if (xyz++ % 10000 == 9999) {
			LOG_INFO("ApiServer OnNewTransaction %d after %llu", xyz, utils::Timestamp::HighResolution() - startTime);
		}

		//check and decode
		protocol::SlaveVerifyResponse sv_rsp;
		if (!sv_rsp.ParseFromArray(msg, len)) {
			LOG_WARN("internal error");
			return;
		}
		PeerMessagePointer message = std::shared_ptr<PeerMessage>(new PeerMessage);
		//check message and hash from peer
		if (!message->FromString(sv_rsp.peer_message()) || sv_rsp.peer_message_hash().empty()) {
			LOG_WARN("internal error");
			return;
		}
		//decode transaction from peer message
		protocol::TransactionEnvWrapper *tran_env_wrapper = (protocol::TransactionEnvWrapper *)message->data_;
		TransactionFrm::pointer tran = std::make_shared<TransactionFrm>(*tran_env_wrapper, sv_rsp);
		int64_t ledger_seq = 0;
		//deliver transaction to glue manager and broadcast message to peer
		if (bubi::GlueManager::Instance().ReceiveTransaction(tran, &ledger_seq)) {
			tran_env_wrapper->set_suggest_ledger_seq(ledger_seq);
			message->hash_ = utils::Sha256::Crypto(message->ToString());
			bubi::PeerManager::Instance().Broadcast(message);
		}
	}

	void MasterService::OnBroadcastTransaction(const char* msg, int len, std::string &reply) {
		//check and decode
		protocol::SlaveVerifyResponse sv_rsp;
		if (!sv_rsp.ParseFromArray(msg, len)) {
			LOG_WARN("internal error");
			return;
		}
		PeerMessagePointer message = std::shared_ptr<PeerMessage>(new PeerMessage);

		if (!message->FromString(sv_rsp.peer_message()) || sv_rsp.peer_message_hash().empty()) {
			LOG_WARN("internal error");
			return;
		}
		//set hash to avoid re-calculate when add and send
		message->hash_ = sv_rsp.peer_message_hash();

		const protocol::TransactionEnvWrapper *tran_env_wrapper = (const protocol::TransactionEnvWrapper *)message->data_;
		TransactionFrm::pointer tran = std::make_shared<TransactionFrm>(*tran_env_wrapper, sv_rsp);

		int64_t ledger_seq = tran_env_wrapper->suggest_ledger_seq();

		//LOG_INFO("Recv tx of block %llu", ledger_seq);
		if (ledger_seq == 0) {
			ledger_seq = bubi::GlueManager::Instance().GetLastConsensusSeq() + 1;
		}
		if (bubi::GlueManager::Instance().ReceiveTransaction(tran, &ledger_seq)) {
			//the message has been recorded when receiving it
			bubi::PeerManager::Instance().TransactionNetwork().Broadcast(message);
		}
	}

	void MasterService::OnTxSetTransaction(const char* msg, int len, std::string &reply) {
		//check and decode
		protocol::SlaveVerifyResponse sv_rsp;
		if (!sv_rsp.ParseFromArray(msg, len)) {
			LOG_WARN("internal error");
			return;
		}
		PeerMessagePointer message = std::shared_ptr<PeerMessage>(new PeerMessage);

		if (!message->FromString(sv_rsp.peer_message()) || sv_rsp.peer_message_hash().empty()) {
			LOG_WARN("internal error");
			return;
		}
		//set hash to avoid re-calculate when add and send
		message->hash_ = sv_rsp.peer_message_hash();

		const protocol::TransactionEnvWrapper *tran_env_wrapper = (const protocol::TransactionEnvWrapper *)message->data_;
		TransactionFrm::pointer tran = std::make_shared<TransactionFrm>(*tran_env_wrapper, sv_rsp);

		//no need to deliver txset transaction to glue any more, neither need to broadcast
		//just pass it to fetcher
		bubi::GlueManager::Instance().RecvConsensusTx(tran);
	}
}
