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
#include <proto/message.pb.h>
#include <common/general.h>
#include <common/configure.h>
#include <common/private_key.h>
#include <overlay/peer.h>
#include "slave_executor.h"
#include "slave_service.h"

namespace bubi {
	SlaveExecutor::SlaveExecutor() {}
	SlaveExecutor::~SlaveExecutor() {}

	void SlaveExecutor::ProcessBroadcastTransaction(const char* msg, int len, std::string &reply) {
		LOG_TRACE("Receive Broadcast TX");

		protocol::SlaveVerifyRequest sv_req;
		if (!sv_req.ParseFromArray(msg, len)) {
			LOG_WARN("internal error");
			return;
		}

		//1  decode message
		PeerMessagePointer peerMessage = std::shared_ptr<PeerMessage>(new PeerMessage);

		// peer_message
		if (!peerMessage->FromString(sv_req.peer_message().c_str(), sv_req.peer_message().size())) {
			LOG_WARN("internal error");
			return;
		}

		const protocol::TransactionEnv &transEnv = ((protocol::TransactionEnvWrapper*)peerMessage->data_)->transaction_env();
		std::string trans = transEnv.transaction().SerializeAsString();

		//get the transaction hash
		std::string trans_hash = utils::Sha256::Crypto(trans);

		//2. check signature
		for (int32_t i = 0; i < transEnv.signatures_size(); i++) {
			const protocol::Signature &signature = transEnv.signatures(i);

			PublicKey pubkey(signature.public_key());

			if (!pubkey.IsValid()) {
				LOG_ERROR("Invalid publickey(%s),trans_hash is (%s)", signature.public_key().c_str(), utils::String::BinToHexString(trans_hash).c_str());
				return;
			}
			if (!PublicKey::Verify(trans, signature.sign_data(), signature.public_key())) {
				LOG_ERROR("Verify signature failed,trans_hash is (%s)", utils::String::BinToHexString(trans_hash).c_str());
				return;
			}
		}

		//3. calculate message hash

		protocol::SlaveVerifyResponse sv_rsp;
		sv_rsp.set_peer_message(sv_req.peer_message().c_str(), sv_req.peer_message().size());
		sv_rsp.set_peer_id(sv_req.peer_id());
		sv_rsp.set_peer_message_hash(utils::Sha256::Crypto(sv_req.peer_message()));
		sv_rsp.set_transaction_hash(utils::Sha256::Crypto(trans));
		sv_rsp.set_transaction_env_hash(utils::Sha256::Crypto(transEnv.SerializeAsString()));

		for (int32_t i = 0; i < transEnv.signatures_size(); i++) {
			const protocol::Signature &signature = transEnv.signatures(i);
			PublicKey pubkey(signature.public_key());

			sv_rsp.add_address(pubkey.GetBase58Address());
		}
		reply = sv_rsp.SerializeAsString();
	}

	void SlaveExecutor::ProcessTxSetTransaction(const char* msg, int len, std::string &reply) {
		LOG_TRACE("Receive Broadcast TX");

		protocol::SlaveVerifyRequest sv_req;
		if (!sv_req.ParseFromArray(msg, len) || !sv_req.has_txset_hash()) {
			LOG_WARN("internal error");
			return;
		}

		//1  decode message
		PeerMessagePointer peerMessage = std::shared_ptr<PeerMessage>(new PeerMessage);

		// peer_message
		if (!peerMessage->FromString(sv_req.peer_message().c_str(), sv_req.peer_message().size())) {
			LOG_WARN("internal error");
			return;
		}

		const protocol::TransactionEnv &transEnv = ((protocol::TransactionEnvWrapper*)peerMessage->data_)->transaction_env();
		std::string trans = transEnv.transaction().SerializeAsString();

		//get the transaction hash
		std::string trans_hash = utils::Sha256::Crypto(trans);

		//2. check signature
		for (int32_t i = 0; i < transEnv.signatures_size(); i++) {
			const protocol::Signature &signature = transEnv.signatures(i);

			PublicKey pubkey(signature.public_key());

			if (!pubkey.IsValid()) {
				LOG_ERROR("Invalid publickey (%s),trans_hash is (%s)", signature.public_key().c_str(), utils::String::BinToHexString(trans_hash).c_str());
				return;
			}
			if (!PublicKey::Verify(trans, signature.sign_data(), signature.public_key())) {
				LOG_ERROR("Verify signature failed,trans_hash is (%s)", utils::String::BinToHexString(trans_hash).c_str());
				return;
			}
		}

		//3. calculate message hash

		reply.assign(msg, len);

		protocol::SlaveVerifyResponse sv_rsp;
		sv_rsp.set_peer_message(sv_req.peer_message().c_str(), sv_req.peer_message().size());
		sv_rsp.set_peer_id(sv_req.peer_id());
		sv_rsp.set_txset_hash(sv_req.txset_hash());
		sv_rsp.set_peer_message_hash(utils::Sha256::Crypto(sv_req.peer_message()));
		sv_rsp.set_transaction_hash(utils::Sha256::Crypto(trans));
		sv_rsp.set_transaction_env_hash(utils::Sha256::Crypto(transEnv.SerializeAsString()));

		for (int32_t i = 0; i < transEnv.signatures_size(); i++) {
			const protocol::Signature &signature = transEnv.signatures(i);
			PublicKey pubkey(signature.public_key());

			sv_rsp.add_address(pubkey.GetBase58Address());
		}
		reply = sv_rsp.SerializeAsString();
	}
}

