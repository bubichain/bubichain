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

#include <utils/timestamp.h>
#include <utils/logger.h>
#include <common/general.h>
#include "peer.h"

namespace bubi {
	int64_t Peer::peer_id_seed_ = 0;

	int64_t Peer::NewId() {
		peer_id_seed_++;
		return peer_id_seed_;
	}

	Peer::Peer(utils::AsyncIo *asyncio_ptr, PeerNetwork *peer_network, IPeerAppNotify *notify, bool inbound) :
		AsyncSocketTcp(asyncio_ptr), peer_network_(peer_network), notify_(notify), inbound_(inbound) {
		id_ = NewId();

		state_changed_ = false;
		header_recieved_ = false;
		last_receive_time_ = connect_start_time_ = utils::Timestamp::HighResolution();
		connect_end_time_ = 0;
		active_time_ = 0;
		last_send_time_ = 0;
		delay_ = 0;
		send_size_ = 0;
		recv_need_size_ = 0;

		peer_ledger_version_ = 0;
		peer_overlay_version_ = 0;
		peer_listen_port_ = 0;

		peer_methods_[PeerMessage::PEER_MESSAGE_GETQUORUMSET] = std::bind(&Peer::OnGetQuorumSet, this, std::placeholders::_1);
		peer_methods_[PeerMessage::PEER_MESSAGE_QUORUMSET] = std::bind(&Peer::OnQuorumSet, this, std::placeholders::_1);
	}

	Peer::~Peer() {

	}

	int64_t Peer::delay() {
		return delay_;
	}

	size_t Peer::GetSendBufferListSize() {
		utils::MutexGuard guard(send_mutex_);
		return send_buffer_list_.size();
	}

	size_t Peer::GetSendBufferSize() {
		utils::MutexGuard guard(send_mutex_);
		return send_buffer_.size();
	}

	size_t Peer::GetRecvBufferSize() {
		utils::MutexGuard guard(send_mutex_);
		return recv_buffer_.size();
	}

	void Peer::UpdateDelay(int64_t now) {
		delay_ = (now - last_send_time_) / (2);
	}

	std::string bubi::Peer::peer_node_id() {
		return peer_node_id_;
	}

	int64_t Peer::peer_id() {
		return id_;
	}

	std::string Peer::remote_node_id() {
		return peer_node_id_;
	}

	bool Peer::IsActive() {
		return active_time_ > 0;
	}

	int64_t Peer::active_time() {
		return active_time_;
	}

	void Peer::StartToReceive() {
		connect_start_time_ = last_receive_time_ = utils::Timestamp::HighResolution();

		recv_need_size_ = sizeof(PeerMsgHearder);
		AsyncReceiveSome(recv_need_size_);
	}

	PeerNetwork *Peer::GetPeerNetwork() {
		return peer_network_;
	}

	void Peer::SetPeerInfo(const protocol::Hello &hello) {
		peer_overlay_version_ = hello.overlayversion();
		peer_ledger_version_ = hello.ledger_version();
		peer_version_ = hello.bubiversion();
		peer_listen_port_ = hello.listeningport();
		peer_node_id_ = hello.nodeid();
	}

	void Peer::SetActiveTime(int64_t current_time) {
		active_time_ = current_time;
	}

	void Peer::SetStateChanged(bool state) {
		state_changed_ = state;
	}

	bool Peer::in_bound() {
		return inbound_;
	}

	bool Peer::state_changed() {
		return state_changed_;
	}

	void Peer::clean_state_changed() {
		state_changed_ = false;
	}

	utils::InetAddress Peer::GetRemoteAddress() {
		utils::InetAddress address = peer_address_;
		if (in_bound()) {
			address.SetPort((uint16_t)peer_listen_port_);
		}
		return address;
	}

	bool Peer::IsConnectExpired(int64_t time_out) {
		return connect_end_time_ == 0 &&
			utils::Timestamp::HighResolution() - connect_start_time_ > time_out &&
			!in_bound();
	}

	bool Peer::IsDataExpired(int64_t time_out) {
		return IsActive() && utils::Timestamp::HighResolution() - last_receive_time_ > time_out;
	}

	bool Peer::SendMessage(const std::string &buffer) {
		if (!IsValid()) {
			return false;
		}

		utils::MutexGuard guard(send_mutex_);

		if (send_size_ != send_buffer_.size()) {
			send_buffer_list_.push_back(buffer);
			return true;
		}

		send_buffer_ = buffer;
		send_size_ = 0;
		AsyncSendSome(send_buffer_.c_str(), send_buffer_.size());

		return true;
	}

	void Peer::OnConnect() {
		LOG_TRACE("Peer(%s) connected", peer_address_.ToIpPort().c_str());
		connect_end_time_ = utils::Timestamp::HighResolution();
		//send hello and receive
		notify_->OnPeerConnect(this);

		recv_need_size_ = sizeof(PeerMsgHearder);
		AsyncReceiveSome(recv_need_size_);
	}

	void Peer::OnSend(size_t bytes_transferred) {
		utils::MutexGuard guard(send_mutex_);
		send_size_ += bytes_transferred;
		if (send_size_ >= send_buffer_.size()) {
			if (send_buffer_list_.size() > 0) {
				send_buffer_ = send_buffer_list_.front();
				send_size_ = 0;
				send_buffer_list_.pop_front();
				AsyncSendSome(send_buffer_.c_str(), send_buffer_.size());
			}
		}
		else {
			AsyncSendSome((char *)send_buffer_.c_str() + send_size_, send_buffer_.size() - send_size_);
		}
	}

	void Peer::OnReceive(void *buffer, size_t bytes_transferred) {
		recv_buffer_.append((char *)buffer, bytes_transferred);

		size_t next_need_size = 0;
		if (recv_buffer_.size() < recv_need_size_) {
			next_need_size = recv_need_size_ - recv_buffer_.size();
		}
		else {
			if (!header_recieved_) {
				PeerMsgHearder *msg_header = (PeerMsgHearder *)recv_buffer_.c_str();
				next_need_size = ntohl(msg_header->data_len);

				recv_need_size_ = sizeof(PeerMsgHearder) + next_need_size;
				header_recieved_ = true;

				if (recv_need_size_ > 100000000) {
					LOG_ERROR("Receive need size is too large,received need size is (%ld)", recv_need_size_);
					OnError();
					return;
				}
			}
			else {
				do {
					PeerMessagePointer peer_message = notify_->GetMessageObject();
					if (!peer_message->FromString(recv_buffer_))
						break;

					if (peer_message->header_.type == PeerMessage::PEER_MESSAGE_TRANSACTION) {
						if (notify_->OnTransactionNotify(peer_message, recv_buffer_, this) < 0) {
							break;
						}

						static int xyz = 0;
						if (xyz++ % 10000 == 9999) {
							LOG_INFO("Peer received %d transactions", xyz);
						}

						break;
					}

					LOG_TRACE("Receive message type(%s) from peer(" FMT_SIZE ")",
						PeerMessage::GetTypeDesc(peer_message->header_.type).c_str(), id_);
					if (active_time_ > 0 ||
						peer_message->header_.type == PeerMessage::PEER_MESSAGE_HELLO) {
						last_receive_time_ = utils::Timestamp::HighResolution();
						notify_->OnAppMsgNotify(peer_message, this);
					}
				} while (false);

				recv_buffer_ = "";
				header_recieved_ = false;
				recv_need_size_ = sizeof(PeerMsgHearder);
			}
		}

		if (IsValid()) AsyncReceiveSome(next_need_size);
	}

	void Peer::OnError() {
		LOG_ERROR_ERRNO("Peer(%s) error", peer_address_.ToIpPort().c_str(), STD_ERR_CODE, STD_ERR_DESC);
		state_changed_ = true;
		active_time_ = 0;
		Close();
	}

	bool Peer::SendHello(int32_t listen_port, const std::string &node_id, const std::string &network_type) {
		std::shared_ptr<PeerMessage>  msg = PeerMessage::NewHello();

		protocol::Hello *hello = (protocol::Hello *)msg->data_;

		hello->set_ledger_version(General::LEDGER_VERSION);
		hello->set_overlayversion(General::OVERLAY_VERSION);
		hello->set_listeningport(listen_port);
		hello->set_bubiversion(General::BUBI_VERSION);
		hello->set_nodeid(node_id);
		hello->set_network_type(network_type);
		std::string buffer = msg->ToString();
		return SendMessage(buffer);
	}

	bool Peer::SendPeers(const Json::Value &db_peers) {
		std::shared_ptr<PeerMessage>  msg = PeerMessage::NewPeers();

		for (size_t i = 0; i < db_peers.size(); i++) {
			const Json::Value &item = db_peers[i];
			protocol::Peers *peers = (protocol::Peers *)msg->data_;
			protocol::Peer *peerp = peers->add_peers();
			peerp->set_ip(item["ip"].asCString());
			peerp->set_port(item["port"].asInt());
			peerp->set_num_failures(item["num_failures"].asInt());
		}

		std::string buffer = msg->ToString();
		return SendMessage(buffer);
	}

	bool Peer::SendGetPeers() {
		auto msg = PeerMessage::NewGetPeers();
		std::string buffer = msg->ToString();
		return SendMessage(buffer);
	}

	bool Peer::SendGetTxHashSet(const std::string &set_hash) {
		auto msg = PeerMessage::NewGetTxHashSet();
		((protocol::GetTxHashSet *)msg->data_)->set_hash(set_hash);
		std::string buffer = msg->ToString();

		LOG_INFO("SendGetTxHashSet hash(%s)", utils::String::Bin4ToHexString(set_hash).c_str());
		return SendMessage(buffer);
	}

	bool Peer::SendGetQuorumset(const std::string &set_hash) {
		auto msg = PeerMessage::NewGetQuorumset();
		((protocol::GetQuorumset *)msg->data_)->set_hash(set_hash);
		std::string buffer = msg->ToString();
		return SendMessage(buffer);
	}

	bool Peer::SendDontHave(uint16_t type, const std::string &hash) {
		auto msg = PeerMessage::NewDontHave();
		((protocol::DontHave *)msg->data_)->set_type((int32_t)type);
		((protocol::DontHave *)msg->data_)->set_hash(hash);
		std::string buffer = msg->ToString();
		return SendMessage(buffer);
	}

	bool Peer::SendPing() {
		auto msg = PeerMessage::NewPing();
		uint64_t nonce = rand() * rand();
		nonce *= rand();
		protocol::Ping *ping = (protocol::Ping *)msg->data_;
		ping->set_nonce(nonce);

		std::string buffer = msg->ToString();
		last_send_time_ = utils::Timestamp::HighResolution();
		return SendMessage(buffer);
	}

	bool Peer::SendPong(int64_t nonce) {
		auto msg = PeerMessage::NewPong();
		protocol::Ping *ping = (protocol::Ping *)msg->data_;
		ping->set_nonce(nonce);

		std::string buffer = msg->ToString();
		return SendMessage(buffer);
	}

	bool Peer::OnGetQuorumSet(const PeerMessagePointer &message) {
		LOG_TRACE("On getquorumset");
		return true;
	}

	bool Peer::OnQuorumSet(const PeerMessagePointer &message) {
		return true;
	}

	void Peer::ToJson(Json::Value &status) {
		status["id"] = peer_id();
		status["in_bound"] = in_bound();
		status["is_active"] = IsActive();
		status["active_time"] = active_time();
		status["remove_address"] = GetRemoteAddress().ToIpPort();
	}
}