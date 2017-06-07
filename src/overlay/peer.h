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

#ifndef PEER_H_
#define PEER_H_

#include <utils/net.h>
#include <utils/strings.h>
#include <json/value.h>
#include <proto/message.pb.h>

namespace bubi {
	typedef struct tagPeerMsgHearder {
		uint16_t type;
		uint16_t compress_type; //0: no 1:gzip
		uint32_t data_len;
		uint32_t checksum;
	}PeerMsgHearder;

	class PeerMessage;
	typedef std::shared_ptr<PeerMessage> PeerMessagePointer;

	class PeerMessage {
	public:
		~PeerMessage();
		PeerMessage();

		typedef enum tagPEER_MESSAGE_TYPE {
			PEER_MESSAGE_HELLO = 1,
			PEER_MESSAGE_PING = 2,
			PEER_MESSAGE_PONG = 3,
			PEER_MESSAGE_GETPEERS = 4,
			PEER_MESSAGE_PEERS = 5,
			PEER_MESSAGE_GETLEDGERS = 6,
			PEER_MESSAGE_LEDGERS = 7,
			PEER_MESSAGE_GETTXHASHSET = 8,
			PEER_MESSAGE_TXHASHSET = 9,
			PEER_MESSAGE_GETQUORUMSET = 10,
			PEER_MESSAGE_QUORUMSET = 11,
			PEER_MESSAGE_TRANSACTION = 12,
			PEER_MESSAGE_BPAXOS = 13,
			PEER_MESSAGE_DONTHAVE = 14,
			PEER_MESSAGE_GETTXSET = 15,
			PEER_MESSAGE_TXSET = 16,
			PEER_MESSAGE_PBFT = 17,
			PEER_MESSAGE_PAYLOAD = 18,
			PEER_MESSAGE_KEEPONLINE = 19,
		}PEER_MESSAGE_TYPE;

		std::string ToString();
		std::string GetString();
		bool FromString(const std::string &data);
		bool FromString(const char* data, size_t len);
		virtual bool FromStringOther(uint16_t type, const char* data, size_t len) { return false; };
		static std::string GetTypeDesc(uint16_t type);
		static std::shared_ptr<PeerMessage> NewHello();
		static std::shared_ptr<PeerMessage> NewPing();
		static std::shared_ptr<PeerMessage> NewPong();
		static std::shared_ptr<PeerMessage> NewPeers();
		static std::shared_ptr<PeerMessage> NewGetPeers();
		static std::shared_ptr<PeerMessage> NewTransaction();
		static std::shared_ptr<PeerMessage> NewGetLedgers();
		static std::shared_ptr<PeerMessage> NewLedgers();
		static std::shared_ptr<PeerMessage> NewGetTxSet();
		static std::shared_ptr<PeerMessage> NewGetTxHashSet();
		static std::shared_ptr<PeerMessage> NewGetQuorumset();
		static std::shared_ptr<PeerMessage> NewQuorumset();
		static std::shared_ptr<PeerMessage> NewTxHashSet();
		static std::shared_ptr<PeerMessage> NewTxSet();
		static std::shared_ptr<PeerMessage> NewBpaxos();
		static std::shared_ptr<PeerMessage> NewDontHave();
		static std::shared_ptr<PeerMessage> NewPbft();
		static std::shared_ptr<PeerMessage> NewPayLoad();
	public:
		PeerMsgHearder header_;
		google::protobuf::Message *data_;
		//hash_ is calculated by slave, and set from outside instead of FromString
		std::string hash_;
		std::string string_;
	};

	class Peer;

	typedef std::function<bool(const PeerMessagePointer &message)> MessagePoc;
	typedef std::map<uint16_t, MessagePoc> PeerMethodMap;
	typedef std::function<bool(const PeerMessagePointer &message, Peer *peer)> MessagePeerPoc;
	typedef std::map<uint16_t, MessagePeerPoc> MessagePeerPocMap;
	
	class PeerNetwork;
	class IPeerAppNotify {
	public:
		IPeerAppNotify() {};
		~IPeerAppNotify() {};
		virtual bool OnAppMsgNotify(const PeerMessagePointer &message, Peer *) = 0;
		virtual int32_t OnTransactionNotify(const PeerMessagePointer &message, const std::string &buffer, Peer *) = 0;
		virtual void OnPeerConnect(Peer *) {};
		virtual PeerMessagePointer GetMessageObject() { return std::shared_ptr<PeerMessage>(new PeerMessage); };
	};

	class Peer : public utils::AsyncSocketTcp {
	public:
		Peer(utils::AsyncIo *asyncio_ptr,
			bubi::PeerNetwork *peer_network,
			bubi::IPeerAppNotify *notify,
			bool inbound);
		~Peer();
		int64_t delay();
		void   UpdateDelay(int64_t now);
		std::string peer_node_id();
		int64_t peer_id();
		std::string remote_node_id();
		bool IsActive();
		bool IsConnectExpired(int64_t time_out);
		bool IsDataExpired(int64_t time_out);
		int64_t active_time();
		bool in_bound();
		bool state_changed();
		void clean_state_changed();
		utils::InetAddress GetRemoteAddress();
		void StartToReceive();
		PeerNetwork *GetPeerNetwork();
		void SetPeerInfo(const protocol::Hello &hello);
		void SetActiveTime(int64_t current_time);
		void SetStateChanged(bool state);
		bool SendPing();
		bool SendPong(int64_t nonce);
		bool SendHello(int32_t listen_port, const std::string &node_id, const std::string &network_type);
		bool SendPeers(const Json::Value &db_peers);
		bool SendGetPeers();
		bool SendGetTxHashSet(const std::string &set_hash);
		bool SendGetQuorumset(const std::string &set_hash);
		bool SendDontHave(uint16_t type, const std::string &hash);
		bool OnGetQuorumSet(const PeerMessagePointer &message);
		bool OnQuorumSet(const PeerMessagePointer &message);
		bool SendMessage(const std::string &buffer);
		static int64_t NewId();
		virtual void OnSend(size_t bytes_transferred);
		virtual void OnError();
		virtual void OnReceive(void *buffer, size_t bytes_transferred);
		virtual void OnConnect();
		size_t GetSendBufferListSize();
		size_t GetSendBufferSize();
		size_t GetRecvBufferSize();
		void ToJson(Json::Value &status);
	public:
		static int64_t peer_id_seed_;
	private:
		PeerNetwork *peer_network_;
		bubi::IPeerAppNotify *notify_;
		int64_t id_;
		bool inbound_;
		bool state_changed_;
		int64_t connect_start_time_;
		int64_t connect_end_time_;
		int64_t active_time_;
		int64_t last_receive_time_;
		int64_t last_send_time_;
		int64_t delay_;
		size_t send_size_;
		std::string send_buffer_;
		utils::StringList send_buffer_list_;
		utils::Mutex send_mutex_;
		std::string recv_buffer_;
		size_t recv_need_size_;
		bool header_recieved_;
		std::string peer_version_;
		uint32_t peer_ledger_version_;
		uint32_t peer_overlay_version_;
		uint32_t peer_listen_port_;
		std::string peer_node_id_;
		PeerMethodMap peer_methods_;
	};

	typedef std::list<Peer *> PeerList;
	typedef std::map<int64_t, Peer *> PeerMap;
}

#endif
