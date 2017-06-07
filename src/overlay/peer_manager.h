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

#ifndef PEER_MANAGER_H_
#define PEER_MANAGER_H_

#include <utils/singleton.h>
#include <utils/net.h>
#include <common/general.h>
#include <common/private_key.h>
#include "broadcast.h"
#include "peer.h"

namespace bubi {
	class IPeerManagerNotify {
	public:
		IPeerManagerNotify();
		~IPeerManagerNotify();
		virtual void OnNetworkPrepared() = 0;
		virtual void OnMessage() = 0;
	};

	typedef std::list<IPeerManagerNotify *> PeerManagerNotifier;

	class PeerNetwork :
		public utils::IAsyncSocketAcceptorNotify,
		public bubi::TimerNotify,
		public IBroadcastDriver {
	public:
		enum NetworkType {
			CONSENSUS = 0,
			TRANSACTION = 1
		};
		PeerNetwork(NetworkType type);
		~PeerNetwork();
		bool Initialize();
		bool Exit();
		bool RegisterNotify(IPeerManagerNotify *notify);
		Json::Value GetPeersCache();
		void AddReceivedPeers(const utils::StringMap &item);
		void Broadcast(const bubi::PeerMessagePointer &message);
		bool ReceiveBroadcastMsg(const bubi::PeerMessagePointer &message, int64_t peer_id);
		const PeerMap &GetPeers() const { return peer_list_; };
		void GetPeers(Json::Value &peers);
		NetworkType GetNetworkType() const { return type_; };
		virtual void OnAccept(utils::AsyncSocketAcceptor *acceptor_ptr);
		virtual void OnError(utils::AsyncSocketAcceptor *acceptor_ptr);
		virtual void OnTimer(int64_t current_time) override;
		virtual void OnSlowTimer(int64_t current_time) override {};
		void GetModuleStatus(Json::Value &data);
		virtual bool SendMessage(int64_t peer_id, PeerMessagePointer msg);
		virtual std::set<int64_t> GetActivePeerIds();
		bool NodeExist(std::string node_address, int64_t peer_id);
	private:
		void Clean();
		bool Listen();
		bool CheckStorage();
		bool ResolveSeeds(const utils::StringList &address_list, int32_t rank);
		bool ConnectToPeers(size_t max);
		bool LoadSeed();
		bool LoadHardcode();
		bool ResetPeerInActive();
		bool CreatePeerIfNotExist(const utils::InetAddress &address, int32_t rank);
		bool UpdatePeer(const utils::InetAddress &local_address, const utils::StringMap &values);
		bool GetActivePeers(int32_t max);
	private:
		NetworkType type_;
		utils::AsyncIo *async_io_ptr_;
		bool dns_seed_inited_;
		PeerMap peer_list_;
		PeerMap peer_list_delete_;
		utils::Mutex peer_list_mutex_;
		int64_t last_heartbeart_time_;
		Json::Value db_peer_cache_;
		PeerManagerNotifier notifiers_;
		utils::AsyncSocketAcceptor *acceptor_ptr_;
		Peer *incoming_peer_;
		std::list<utils::StringMap> received_peer_list_;
		bubi::Broadcast broadcast_;
	};

	class PeerManager : public utils::Singleton<PeerManager>,
		public IPeerAppNotify,
		public StatusModule,
		public utils::Runnable {
		friend class utils::Singleton<PeerManager>;
	public:
		PeerManager();
		~PeerManager();
		bool Initialize();
		bool Exit();
		virtual void Run(utils::Thread *thread) override;
		void Broadcast(const bubi::PeerMessagePointer &message);
		bool SendMessage(int64_t peer_id, PeerMessagePointer &message);
		PeerNetwork& ConsensusNetwork();
		PeerNetwork& TransactionNetwork();
		asio::io_service& GetIOService();
		std::string GetPeerNodeAddress();
		virtual bool OnAppMsgNotify(const PeerMessagePointer &message, Peer *peer_ptr);
		virtual int32_t OnTransactionNotify(const PeerMessagePointer &message, const std::string &buffer, Peer *peer_ptr);
		virtual void GetModuleStatus(Json::Value &data);
		bool BroadcastPayLoad(protocol::ChainPeerMessage &cpm);
	private:
		bool OnPbft(const PeerMessagePointer &message, Peer *peer);
		bool OnGetLedgers(const PeerMessagePointer &message, Peer *peer);
		bool OnLedgers(const PeerMessagePointer &message, Peer *peer);
		bool OnGetTxSet(const PeerMessagePointer &message, Peer *peer);
		bool OnTxSet(const PeerMessagePointer &message, Peer *peer);
		bool OnGetTxHashSet(const PeerMessagePointer &message, Peer *peer);
		bool OnTxHashSet(const PeerMessagePointer &message, Peer *peer);
		bool OnPeers(const PeerMessagePointer &message, Peer *peer);
		bool OnGetPeers(const PeerMessagePointer &message, Peer *peer);
		bool OnHello(const PeerMessagePointer &message, Peer *peer);
		bool OnDontHave(const PeerMessagePointer &message, Peer *peer);
		bool OnPing(const PeerMessagePointer &message, Peer *peer);
		bool OnPong(const PeerMessagePointer &message, Peer *peer);
		void OnPeerConnect(Peer *peer);
		bool OnPayLoad(const PeerMessagePointer &message, Peer *peer);
		bool OnKeepOnline(const PeerMessagePointer &message, Peer *peer);
	private:
		//high way to deliver urgent message asap
		PeerNetwork consensus_network_;
		//one-way network, only for broadcasting transaction 
		PeerNetwork transaction_network_;
		MessagePeerPocMap peer_methods_;

		asio::io_service asio_service_;
		utils::Thread *thread_ptr_;
		PrivateKey priv_key_;
		std::string peer_node_address_;
	};
}

#endif

