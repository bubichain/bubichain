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

#include <common/general.h>
#include <utils/logger.h>
#include "peer.h"

namespace bubi {
	bubi::PeerMessage::PeerMessage()
		:data_(NULL) {
		memset(&header_, 0, sizeof(header_));
		utils::AtomicInc(&bubi::General::peermsg_new_count);
	}

	PeerMessage::~PeerMessage() {
		if (data_) delete data_;
		utils::AtomicInc(&bubi::General::peermsg_delete_count);
	}

	std::string PeerMessage::ToString() {
		if (string_.length() > 0) {
			return string_;
		}
		std::string data_buffer;
		data_buffer = data_->SerializeAsString();

		header_.compress_type = 0;

		if (data_buffer.length() > 102400) //100k
		{
			LOG_INFO("need compress message(%d) %d bytes", header_.type, data_buffer.length());
		}

		header_.data_len = data_buffer.size();
		header_.checksum = 0;

		string_.resize(sizeof(PeerMsgHearder) + data_buffer.size());

		PeerMsgHearder *header_ptr = (PeerMsgHearder *)string_.c_str();
		header_ptr->type = htons(header_.type);
		header_ptr->compress_type = htons(header_.compress_type);
		header_ptr->data_len = htonl(header_.data_len);
		header_ptr->checksum = htonl(header_.checksum);

		memcpy((char *)string_.c_str() + sizeof(header_), data_buffer.c_str(), data_buffer.size());

		return string_;
	}

	std::string PeerMessage::GetString() {
		if (string_.length() == 0) {
			ToString();
		}

		return string_;
	}

	std::string PeerMessage::GetTypeDesc(uint16_t type) {
		std::string ret;
		switch (type) {
		case PEER_MESSAGE_HELLO:
			ret = "hello";
			break;
		case PEER_MESSAGE_PING:
			ret = "ping";
			break;
		case PEER_MESSAGE_PONG:
			ret = "ping";
			break;
		case PEER_MESSAGE_PEERS:
			ret = "peers";
			break;
		case PEER_MESSAGE_TRANSACTION:
			ret = "transaction";
			break;
		case PEER_MESSAGE_GETLEDGERS:
			ret = "getledgers";
			break;
		case PEER_MESSAGE_LEDGERS:
			ret = "ledgers";
			break;
		case PEER_MESSAGE_GETTXHASHSET:
			ret = "gettxhashset";
			break;
		case PEER_MESSAGE_GETQUORUMSET:
			ret = "getquorumsest";
			break;
		case PEER_MESSAGE_QUORUMSET:
			ret = "quorumsest";
			break;
		case PEER_MESSAGE_TXHASHSET:
			ret = "txhashset";
			break;
		case PEER_MESSAGE_BPAXOS:
			ret = "bpaxos";
			break;
		case PEER_MESSAGE_DONTHAVE:
			ret = "donthave";
			break;
		case PEER_MESSAGE_GETTXSET:
			ret = "gettxset";
			break;
		case PEER_MESSAGE_TXSET:
			ret = "txset";
			break;
		case PEER_MESSAGE_PBFT:
			ret = "pbft";
			break;
		default:
			break;
		}

		return ret;
	}

	bool PeerMessage::FromString(const std::string &data) {
		return FromString(data.c_str(), data.length());
	}

	bool PeerMessage::FromString(const char* msg, size_t len) {
		PeerMsgHearder *header = (PeerMsgHearder *)msg;
		header_.type = ntohs(header->type);
		header_.compress_type = ntohs(header->compress_type);
		header_.data_len = ntohl(header->data_len);
		header_.checksum = ntohl(header->checksum);

		if (len < header_.data_len + sizeof(PeerMsgHearder)) {
			return false;
		}

		const char* data = msg + sizeof(PeerMsgHearder);

		//avoid memory copy, use ParseFromArray instead of ParseFromString
		try {
			switch (header_.type) {
			case PEER_MESSAGE_HELLO:
				data_ = new protocol::Hello();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_PING:
				data_ = new protocol::Ping();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_PONG:
				data_ = new protocol::Pong();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_PEERS:
				data_ = new protocol::Peers();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_TRANSACTION:
				data_ = new protocol::TransactionEnvWrapper();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_GETLEDGERS:
				data_ = new protocol::GetLedgers();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_LEDGERS:
				data_ = new protocol::Ledgers();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_GETTXHASHSET:
				data_ = new protocol::GetTxHashSet();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_GETTXSET:
				data_ = new protocol::GetTxSet();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_TXHASHSET:
				data_ = new protocol::TxHashSet();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_TXSET:
				data_ = new protocol::TxSet();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_GETQUORUMSET:
				data_ = new protocol::GetQuorumset();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_QUORUMSET:
			case PEER_MESSAGE_BPAXOS:
				break;
			case PEER_MESSAGE_DONTHAVE:
				data_ = new protocol::DontHave();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_PBFT:
				data_ = new protocol::PbftEnv();
				data_->ParseFromArray(data, header_.data_len);
				break;
			case PEER_MESSAGE_PAYLOAD:
				data_ = new protocol::PayLoadEnv();
				data_->ParseFromArray(data, header_.data_len);
				break;
			default:
				if (!FromStringOther(header_.type, data, header_.data_len)) {
					LOG_ERROR("PeerMessage unknow message type(%d)", header_.type);
					return false;
				}
			}
		}
		catch (std::exception const e) {
			LOG_ERROR("PeerMessage failed(%s)", e.what());
			return false;
		}

		return true;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewHello() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_HELLO;
		msg->data_ = new protocol::Hello();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewPing() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_PING;
		msg->data_ = new protocol::Ping();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewPong() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_PONG;
		msg->data_ = new protocol::Pong();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewPeers() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_PEERS;
		msg->data_ = new protocol::Peers();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewGetPeers() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_GETPEERS;
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewTransaction() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_TRANSACTION;
		msg->data_ = new protocol::TransactionEnvWrapper();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewGetLedgers() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_GETLEDGERS;
		msg->data_ = new protocol::GetLedgers();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewLedgers() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_LEDGERS;
		msg->data_ = new protocol::Ledgers();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewGetTxSet() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_GETTXSET;
		msg->data_ = new protocol::GetTxSet();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewGetTxHashSet() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_GETTXHASHSET;
		msg->data_ = new protocol::GetTxHashSet();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewGetQuorumset() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_GETQUORUMSET;
		msg->data_ = new protocol::GetQuorumset();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewQuorumset() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_QUORUMSET;
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewTxHashSet() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_TXHASHSET;
		msg->data_ = new protocol::TxHashSet();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewTxSet() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_TXSET;
		msg->data_ = new protocol::TxSet();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewBpaxos() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_BPAXOS;
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewDontHave() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_DONTHAVE;
		msg->data_ = new protocol::DontHave();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewPbft() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_PBFT;
		msg->data_ = new protocol::PbftEnv();
		return msg;
	}

	std::shared_ptr<PeerMessage> PeerMessage::NewPayLoad() {
		std::shared_ptr<PeerMessage>  msg = std::shared_ptr<PeerMessage>(new PeerMessage());
		memset(&msg->header_, 0, sizeof(PeerMsgHearder));
		msg->header_.type = PEER_MESSAGE_PAYLOAD;
		msg->data_ = new protocol::PayLoadEnv();
		return msg;
	}
}