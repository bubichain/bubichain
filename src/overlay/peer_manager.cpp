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

#include <utils/logger.h>
#include <utils/timestamp.h>
#include <common/general.h>
#include <common/storage.h>
#include <common/configure.h>
#include <common/private_key.h>
#include <glue/glue_manager.h>
#include <proto/message.pb.h>
#include <ledger/ledger_manager.h>
#include <ledger/transaction_frm.h>
#include <slave/master_service.h>
#include <monitor/monitor_master.h>
#include <api/mq_server.h>
#include <api/websocket_server.h>
#include "peer_manager.h"

namespace bubi {
	PeerNetwork::PeerNetwork(NetworkType type)
		:type_(type),
		broadcast_(this) {
		async_io_ptr_ = NULL;
		dns_seed_inited_ = false;
		acceptor_ptr_ = NULL;
		incoming_peer_ = NULL;
		check_interval_ = 2 * utils::MICRO_UNITS_PER_SEC;
		timer_name_ = utils::String::Format("%s Network", type == CONSENSUS ? "Consensus" : "Transaction");
	}

	PeerNetwork::~PeerNetwork() {
	}

	void PeerNetwork::Clean() {

	}

	bool PeerNetwork::Initialize() {
		do {
			async_io_ptr_ = new utils::AsyncIo();

			async_io_ptr_->AttachServiceIo(&PeerManager::Instance().GetIOService());


			if (!CheckStorage()) {
				break;
			}

			if (!Listen()) {
				break;
			}

			TimerNotify::RegisterModule(this);

			last_heartbeart_time_ = utils::Timestamp::HighResolution();

			return ResetPeerInActive();
		} while (false);

		Clean();
		return false;
	}

	bool PeerNetwork::Exit() {
		LOG_INFO("closing acceptor...");
		if (acceptor_ptr_->Close()) {
			LOG_INFO("close acceptor OK");
		}
		delete acceptor_ptr_;
		acceptor_ptr_ = NULL;

		for (auto item : peer_list_) {
			delete item.second;
		}

		for (auto item : peer_list_delete_) {
			delete item.second;
		}

		if (incoming_peer_) {
			delete incoming_peer_;
		}
		LOG_INFO("closing async...");
		if (async_io_ptr_->Close()) {
			LOG_INFO("close async OK");
		}
		delete async_io_ptr_;
		async_io_ptr_ = NULL;

		return true;
	}

	void PeerManager::Run(utils::Thread *thread) {
		asio::io_service::work work(asio_service_);

		bubi::MasterService::Instance().Bind();

		while (!asio_service_.stopped()) {
			asio::error_code err;
			asio_service_.poll(err);

			utils::Sleep(1);
		}
	}

	void PeerNetwork::OnAccept(utils::AsyncSocketAcceptor *acceptor_ptr) {
		const P2pNetwork &p2p_configure =
			type_ == CONSENSUS
			? Configure::Instance().p2p_configure_.consensus_network_configure_
			: Configure::Instance().p2p_configure_.transaction_network_configure_;

		if (peer_list_.size() < p2p_configure.target_peer_connection_) {
			do {
				utils::MutexGuard guard(peer_list_mutex_);
				incoming_peer_->StartToReceive();
				peer_list_.insert(std::make_pair(incoming_peer_->peer_id(), incoming_peer_));
			} while (false);
		}
		else {
			delete incoming_peer_;
		}
		incoming_peer_ = new Peer(async_io_ptr_, this, PeerManager::GetInstance(), true);
		acceptor_ptr_->AsyncAccept(incoming_peer_);
	}

	void PeerNetwork::OnError(utils::AsyncSocketAcceptor *acceptor_ptr) {
		LOG_ERROR_ERRNO("Accept incoming failed", STD_ERR_CODE, STD_ERR_DESC);
	}

	bool PeerNetwork::Listen() {
		const P2pNetwork &p2p_configure =
			type_ == CONSENSUS
			? Configure::Instance().p2p_configure_.consensus_network_configure_
			: Configure::Instance().p2p_configure_.transaction_network_configure_;
		do {
			acceptor_ptr_ = new utils::AsyncSocketAcceptor(async_io_ptr_, this);
			utils::InetAddress listen_address_ = utils::InetAddress::Any();
			listen_address_.SetPort(p2p_configure.listen_port_);
			if (!acceptor_ptr_->Bind(listen_address_)) {
				LOG_ERROR_ERRNO("Peer acceptor bind address(%s) failed", listen_address_.ToIpPort().c_str(), STD_ERR_CODE, STD_ERR_DESC);
				break;
			}

			if (!acceptor_ptr_->Listen()) {
				LOG_ERROR_ERRNO("Peer acceptor listen address(%s) failed", listen_address_.ToIpPort().c_str(), STD_ERR_CODE, STD_ERR_DESC);
				break;
			}

			LOG_INFO("Peer acceptor listen address(%s) successful", listen_address_.ToIpPort().c_str());

			incoming_peer_ = new Peer(async_io_ptr_, this, PeerManager::GetInstance(), true);
			acceptor_ptr_->AsyncAccept(incoming_peer_);
			return true;
		} while (false);

		return false;
	}

	bool PeerNetwork::CheckStorage() {
		Json::Value table_desc = Json::Value(Json::arrayValue);
		RationalDb *db = Storage::Instance().rational_db();
		do {
			Json::Value columns = Json::Value(Json::arrayValue);
			int32_t ret = db->DescribeTable(General::PEERS_TABLE_NAME[type_], columns);
			if (ret < 0) {
				LOG_ERROR_ERRNO("Describe table(%s) failed", General::PEERS_TABLE_NAME[type_], db->error_code(), db->error_desc());
				break;
			}

			if (ret > 0) {
				return true;
			}

			if (!db->Execute(General::PEERS_CREATE_SQL[type_])) {
				LOG_ERROR_ERRNO("Create table(%s) failed", General::PEERS_TABLE_NAME[type_], db->error_code(), db->error_desc());
				break;
			}

			return true;
		} while (false);

		return false;
	}

	bool PeerNetwork::ConnectToPeers(size_t max) {
		const P2pNetwork &p2p_configure =
			type_ == CONSENSUS
			? Configure::Instance().p2p_configure_.consensus_network_configure_
			: Configure::Instance().p2p_configure_.transaction_network_configure_;

		RationalDb *db = Storage::Instance().rational_db();
		std::string sql = utils::String::Format("SELECT * FROM %s WHERE next_attempt_time < " FMT_I64
			" AND active_time <= 0 ORDER BY rank DESC, num_failures ASC LIMIT " FMT_SIZE, General::PEERS_TABLE_NAME[type_], utils::Timestamp::Now().timestamp(), max);
		Json::Value records = Json::Value(Json::arrayValue);
		do {
			int32_t row_count = db->Query(sql, records);
			if (row_count < 0) {
				LOG_ERROR_ERRNO("Query records with sql(%s) failed", sql.c_str(), db->error_code(), db->error_desc());
				break;
			}

			utils::InetAddressVec addresses;
			utils::net::GetNetworkAddress(addresses);

			utils::MutexGuard guard(peer_list_mutex_);

			for (size_t i = 0; i < records.size(); i++) {
				const Json::Value &item = records[i];
				utils::InetAddress address(item["ip"].asString(), item["port"].asUInt());
				int32_t num_failures = item["num_failures"].asInt();

				LOG_TRACE("checking address(%s),current thread id is (%llu)", address.ToIpPort().c_str(), utils::Thread::current_thread_id());

				bool exist = false;
				for (PeerMap::iterator iter = peer_list_.begin(); iter != peer_list_.end(); iter++) {
					if (iter->second->GetRemoteAddress() == address) {
						exist = true;
						break;
					}
				}

				if (exist) {
					LOG_TRACE("skip to connect existed address(%s), " FMT_SIZE, address.ToIpPort().c_str(), utils::Thread::current_thread_id());
					continue;
				}
				bool is_local_addr = false;
				for (utils::InetAddressVec::iterator iter = addresses.begin();
					iter != addresses.end();
					iter++) {
					if (iter->ToIp() == address.ToIp() && p2p_configure.listen_port_ == address.GetPort()) {
						is_local_addr = true;
						break;
					}
				}

				if (is_local_addr) {
					LOG_TRACE("skip to connect local address(%s), " FMT_SIZE, address.ToIpPort().c_str(), utils::Thread::current_thread_id());
					continue;
				}


				LOG_TRACE("connect to address(%s), " FMT_SIZE, address.ToIpPort().c_str(), utils::Thread::current_thread_id());

				Peer *outbound_peer = new Peer(async_io_ptr_, this, PeerManager::GetInstance(), false);
				peer_list_.insert(std::make_pair(outbound_peer->peer_id(), outbound_peer));
				outbound_peer->AsyncConnect(address);

				utils::StringMap update_values;
				num_failures++;
				update_values["next_attempt_time"] = utils::String::ToString(int64_t(utils::Timestamp::Now().timestamp() + num_failures * 10 * utils::MICRO_UNITS_PER_SEC));
				update_values["num_failures"] = utils::String::ToString(num_failures);
				if (!UpdatePeer(address, update_values)) {
					LOG_ERROR_ERRNO("Update peers failed", db->error_code(), db->error_desc());
				}
				if (peer_list_.size() >= p2p_configure.target_peer_connection_) {
					break;
				}
			}

			return true;
		} while (false);

		return false;
	}

	bool PeerNetwork::ResolveSeeds(const utils::StringList &address_list, int32_t rank) {
		utils::NameResolver resolver(async_io_ptr_);
		for (utils::StringList::const_iterator iter = address_list.begin();
			iter != address_list.end();
			iter++) {
			const std::string &longip = *iter;
			utils::StringVector ip_array = utils::String::Strtok(longip, ':');
			std::string ip = longip;
			uint16_t port = type_ == CONSENSUS ? General::CONSENSUS_PORT : General::TRANSACTION_PORT;
			if (ip_array.size() > 1) {
				port = utils::String::Stoui(ip_array[1]);
				ip = ip_array[0];
			}
			else {
				continue;
			}

			utils::InetAddressList resolved_ips;
			do {
				utils::InetAddress address(ip);
				if (!address.IsNone()) {
					resolved_ips.push_back(address);
					break;
				}
				//go to resolve
				resolver.Query(ip, resolved_ips);
			} while (false);

			for (utils::InetAddressList::iterator iter = resolved_ips.begin();
				iter != resolved_ips.end();
				iter++) {

				utils::InetAddress &address = *iter;
				address.SetPort(port);

				CreatePeerIfNotExist(address, rank);
			}

		}
		return true;
	}

	bool PeerNetwork::CreatePeerIfNotExist(const utils::InetAddress &address, int32_t rank) {
		RationalDb *db = Storage::Instance().rational_db();
		std::string sql_where = utils::String::Format("WHERE ip='%s' AND port=%u", address.ToIp().c_str(), address.GetPort());
		int64_t peer_count = db->QueryCount(General::PEERS_TABLE_NAME[type_], sql_where);
		if (peer_count < 0) {
			LOG_ERROR_ERRNO("Query peer with sql(%s) failed,peer is not existed", sql_where.c_str(), db->error_code(), db->error_desc());
			return false;
		}

		if (peer_count > 0) {
			LOG_TRACE("Query peer(%s) exist", address.ToIpPort().c_str());
			return true;
		}

		utils::StringMap values;
		values["ip"] = address.ToIp();
		values["port"] = utils::String::ToString(address.GetPort());
		values["rank"] = utils::String::ToString(rank);
		values["num_failures"] = "0";
		values["next_attempt_time"] = "0";
		values["active_time"] = "0";

		if (!db->Insert(General::PEERS_TABLE_NAME[type_], values)) {
			LOG_ERROR_ERRNO("Insert peer failed", db->error_code(), db->error_desc());
			return false;
		}

		return true;
	}

	bool PeerNetwork::ResetPeerInActive() {
		RationalDb *db = Storage::Instance().rational_db();
		utils::StringMap values;
		values["active_time"] = "0";
		return db->Update(General::PEERS_TABLE_NAME[type_], values, "");
	}

	bool PeerNetwork::UpdatePeer(const utils::InetAddress &local_address, const utils::StringMap &values) {
		RationalDb *db = Storage::Instance().rational_db();
		std::string sql_where = utils::String::Format("WHERE ip='%s' AND port=%u", local_address.ToIp().c_str(), local_address.GetPort());
		return db->Update(General::PEERS_TABLE_NAME[type_], values, sql_where);
	}

	bool PeerNetwork::GetActivePeers(int32_t max) {
		const P2pNetwork &p2p_configure =
			type_ == CONSENSUS
			? Configure::Instance().p2p_configure_.consensus_network_configure_
			: Configure::Instance().p2p_configure_.transaction_network_configure_;
		RationalDb *db = Storage::Instance().rational_db();
		std::string sql = utils::String::Format("SELECT * FROM %s "
			" ORDER BY active_time DESC, num_failures ASC LIMIT %d", General::PEERS_TABLE_NAME[type_], max);
		do {
			db_peer_cache_.clear();
			int32_t row_count = db->Query(sql, db_peer_cache_);
			if (row_count < 0) {
				LOG_ERROR_ERRNO("Query records with sql(%s) failed", sql.c_str(), db->error_code(), db->error_desc());
				break;
			}

			return true;
		} while (false);
		return false;
	}


	void PeerNetwork::GetPeers(Json::Value &peers) {
		utils::MutexGuard guard(peer_list_mutex_);
		for (auto item : peer_list_) {
			item.second->ToJson(peers[peers.size()]);
		}
	}

	void PeerNetwork::OnTimer(int64_t current_time) {
		const P2pNetwork &p2p_configure =
			type_ == CONSENSUS
			? Configure::Instance().p2p_configure_.consensus_network_configure_
			: Configure::Instance().p2p_configure_.transaction_network_configure_;
		if (!dns_seed_inited_) {
			ResolveSeeds(p2p_configure.known_peer_list_, 2);
			dns_seed_inited_ = true;
			return;
		}

		//start to connect peers
		if (peer_list_.size() < p2p_configure.target_peer_connection_) {
			ConnectToPeers(p2p_configure.target_peer_connection_ - peer_list_.size());
		}

		do {
			utils::MutexGuard guard(peer_list_mutex_);
			for (PeerMap::iterator iter = peer_list_.begin();
				iter != peer_list_.end();
				) {
				Peer *peer = iter->second;
				if (peer->IsConnectExpired(p2p_configure.connect_timeout_)) {

					//Expired
					peer->Close();
					peer_list_.erase(iter++);
					peer_list_delete_.insert(std::make_pair(utils::Timestamp::HighResolution() + 5 * utils::MICRO_UNITS_PER_SEC, peer));
				}
				else if (peer->IsActive() && peer->state_changed()) {
					//connect has been established, update db data
					if (peer->in_bound()) {
						//create
						CreatePeerIfNotExist(peer->GetRemoteAddress(), 1);

						//send local peer list
						GetActivePeers(50);
						peer->SendPeers(db_peer_cache_);
					}

					//update status
					utils::StringMap values;
					values["num_failures"] = "0";
					values["active_time"] = utils::String::ToString(peer->active_time());
					UpdatePeer(peer->GetRemoteAddress(), values);
					peer->clean_state_changed();

					iter++;
				}
				else if (!peer->IsActive() && peer->state_changed()) {
					utils::StringMap values;
					values["active_time"] = "0";
					UpdatePeer(peer->GetRemoteAddress(), values);
					peer->clean_state_changed();

					peer->Close();
					peer_list_.erase(iter++);
					peer_list_delete_.insert(std::make_pair(utils::Timestamp::HighResolution() + 5 * utils::MICRO_UNITS_PER_SEC, peer));
				}
				else if (peer->IsDataExpired(p2p_configure.heartbeat_interval_ * 3)) {
					utils::StringMap values;
					values["active_time"] = "0";
					UpdatePeer(peer->GetRemoteAddress(), values);
					peer->clean_state_changed();

					peer->Close();
					peer_list_.erase(iter++);
					peer_list_delete_.insert(std::make_pair(utils::Timestamp::HighResolution() + 5 * utils::MICRO_UNITS_PER_SEC, peer));
				}
				else {
					iter++;
				}
			}
		} while (false);

		int64_t high_resolution_time = utils::Timestamp::HighResolution();
		if (high_resolution_time - last_heartbeart_time_ > p2p_configure.heartbeat_interval_) {
			utils::MutexGuard guard(peer_list_mutex_);
			for (PeerMap::iterator iter = peer_list_.begin();
				iter != peer_list_.end();
				iter++
				) {
				if (iter->second->IsActive()) {
					iter->second->SendPing();
				}
			}
			last_heartbeart_time_ = high_resolution_time;
		}

		//check the peer to be delete
		for (auto iter = peer_list_delete_.begin();
			iter != peer_list_delete_.end();) {
			if (iter->first < utils::Timestamp::HighResolution()) {
				delete iter->second;
				peer_list_delete_.erase(iter++);
			}
			else {
				break;
			}
		}

		//process the peers that have been received
		std::list<utils::StringMap> received_list;
		if (received_peer_list_.size() > 0) {
			utils::MutexGuard guard(peer_list_mutex_);
			received_list = received_peer_list_;
			received_peer_list_.clear();
		}
		for (std::list<utils::StringMap>::iterator iter = received_list.begin();
			iter != received_list.end();
			iter++) {
			utils::StringMap item = *iter;
			std::string ip = item["ip"];
			uint16_t port = utils::String::Stoui(item["port"]);

			//check if it's local address
			utils::InetAddressVec addresses;
			bool is_local_addr = false;
			if (utils::net::GetNetworkAddress(addresses)) {
				for (utils::InetAddressVec::iterator iter = addresses.begin();
					iter != addresses.end();
					iter++) {
					if (iter->ToIp() == ip && p2p_configure.listen_port_ == port) {
						is_local_addr = true;
						break;
					}
				}
			}

			if (is_local_addr) {
				continue;
			}

			CreatePeerIfNotExist(utils::InetAddress(ip, port), 1);
		}

		broadcast_.OnTimer();
	}

	Json::Value PeerNetwork::GetPeersCache() {
		return db_peer_cache_;
	}

	void PeerNetwork::AddReceivedPeers(const utils::StringMap &item) {
		utils::MutexGuard guard(peer_list_mutex_);
		received_peer_list_.push_back(item);
	}

	void PeerNetwork::Broadcast(const PeerMessagePointer &message) {
		broadcast_.Send(message);
	}

	bool PeerNetwork::ReceiveBroadcastMsg(const PeerMessagePointer &message, int64_t peer_id) {
		return broadcast_.Add(message, peer_id);
	}

	bool PeerNetwork::SendMessage(int64_t peer_id, PeerMessagePointer message) {
		utils::MutexGuard guard(peer_list_mutex_);
		PeerMap::iterator iter = peer_list_.find(peer_id);
		if (iter != peer_list_.end() && iter->second->IsActive()) {
			return iter->second->SendMessage(message->ToString());
		}

		return false;
	}

	std::set<int64_t> PeerNetwork::GetActivePeerIds() {
		std::set<int64_t> ids;
		utils::MutexGuard guard(peer_list_mutex_);
		for (auto item : peer_list_) {
			if (item.second->IsActive()) {
				ids.insert(item.second->peer_id());
			}
		}

		return ids;
	}

	bool PeerNetwork::NodeExist(std::string node_address, int64_t peer_id) {
		bool exist = false;
		for (PeerMap::iterator iter = peer_list_.begin(); iter != peer_list_.end(); iter++) {
			if (iter->second->peer_node_id() == node_address && iter->second->peer_id() != peer_id) {
				exist = true;
				break;
			}
		}
		return exist;
	}

	void PeerNetwork::GetModuleStatus(Json::Value &data) {
		utils::MutexGuard guard(peer_list_mutex_);
		data["peer_list_size"] = peer_list_.size();
		data["peer_listdel_size"] = peer_list_delete_.size();
		data["peer_cache_size"] = db_peer_cache_.size();
		data["recv_peerlist_size"] = received_peer_list_.size();
		data["broad_reocrd_size"] = broadcast_.GetRecordSize();
		data["notify_size"] = notifiers_.size();
		int active_size = 0;
		Json::Value peers;

		size_t send_buffer_listsize = 0;
		size_t send_buffer_size = 0;
		size_t recv_buffer_size = 0;
		for (auto &item : peer_list_) {
			send_buffer_listsize += item.second->GetSendBufferListSize();
			send_buffer_size += item.second->GetSendBufferSize();
			recv_buffer_size += item.second->GetRecvBufferSize();

			Json::Value peer;
			peer["id"] = Json::Value(item.second->peer_node_id());
			peer["delay"] = Json::Value(item.second->delay());
			peers.append(peer);

			if (item.second->IsActive()) {
				active_size++;
			}
		}
		data["peers"] = peers;
		data["sendbuf_list_size"] = send_buffer_listsize;
		data["sendbuf_size"] = send_buffer_size;
		data["recvbuf_size"] = recv_buffer_size;
		data["active_size"] = active_size;
	}


	PeerManager::PeerManager()
		:consensus_network_(PeerNetwork::CONSENSUS),
		transaction_network_(PeerNetwork::TRANSACTION),
		peer_methods_(),
		asio_service_(),
		thread_ptr_(NULL),
		priv_key_(ED25519SIG) {
		peer_methods_[PeerMessage::PEER_MESSAGE_PBFT] = std::bind(&PeerManager::OnPbft, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_GETTXHASHSET] = std::bind(&PeerManager::OnGetTxHashSet, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_TXHASHSET] = std::bind(&PeerManager::OnTxHashSet, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_GETTXSET] = std::bind(&PeerManager::OnGetTxSet, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_TXSET] = std::bind(&PeerManager::OnTxSet, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_LEDGERS] = std::bind(&PeerManager::OnLedgers, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_GETLEDGERS] = std::bind(&PeerManager::OnGetLedgers, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_PEERS] = std::bind(&PeerManager::OnPeers, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_GETPEERS] = std::bind(&PeerManager::OnGetPeers, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_HELLO] = std::bind(&PeerManager::OnHello, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_DONTHAVE] = std::bind(&PeerManager::OnDontHave, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_PING] = std::bind(&PeerManager::OnPing, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_PONG] = std::bind(&PeerManager::OnPong, this, std::placeholders::_1, std::placeholders::_2);
		peer_methods_[PeerMessage::PEER_MESSAGE_PAYLOAD] = std::bind(&PeerManager::OnPayLoad, this, std::placeholders::_1, std::placeholders::_2);
	}
	PeerManager::~PeerManager() {
		if (thread_ptr_) {
			delete thread_ptr_;
		}
	}

	PeerNetwork& PeerManager::ConsensusNetwork() {
		return consensus_network_;
	}

	PeerNetwork& PeerManager::TransactionNetwork() {
		return transaction_network_;
	}

	asio::io_service& PeerManager::GetIOService() {
		return asio_service_;
	}

	std::string PeerManager::GetPeerNodeAddress() {
		return priv_key_.GetBase58Address();
	}

	bool PeerManager::OnAppMsgNotify(const PeerMessagePointer &message, Peer *peer) {
		MessagePeerPocMap::iterator iter = peer_methods_.find(message->header_.type);
		if (iter != peer_methods_.end()) {
			MessagePeerPoc proc = iter->second;
			return proc(message, peer);
		}
		return false;
	}

	int32_t PeerManager::OnTransactionNotify(const PeerMessagePointer &message, const std::string &buffer, Peer *peer) {
		if (!TransactionNetwork().ReceiveBroadcastMsg(message, peer->peer_id())) {
			return -1;
		}

		if (bubi::Configure::Instance().monitor_configure_.real_time_status_) {
			protocol::TransactionEnvWrapper *tran_env_wrapper = (protocol::TransactionEnvWrapper *)message->data_;
			//notice monitor Tx state
			int64_t t = utils::Timestamp::HighResolution();
			std::shared_ptr<Json::Value> tx_status = std::make_shared<Json::Value>();
			(*tx_status)["type"] = 2;
			(*tx_status)["tx_hash"] = Json::Value(utils::encode_b16(utils::Sha256::Crypto(tran_env_wrapper->transaction_env().transaction().SerializeAsString())));
			(*tx_status)["active_time"] = Json::Value(t);
			(*tx_status)["id"] = Json::Value(peer->peer_id());
			bubi::MonitorMaster::Instance().NoticeMonitor(tx_status->toStyledString());
		}

		protocol::SlaveVerifyRequest svr;
		svr.set_peer_id(peer->peer_id());
		svr.set_peer_message(buffer.c_str(), buffer.length());
		std::string buf = svr.SerializeAsString();
		MasterService::GetInstance()->SendToSlave(ZMQ_BROADCAST_TX, buf);
		return 0;
	}

	bool PeerManager::Initialize() {
		if (!consensus_network_.Initialize() || !transaction_network_.Initialize()) {
			return false;
		}

		KeyValueDb *db = Storage::Instance().keyvalue_db();
		std::string key = utils::String::Format("%s_nodeprivkey", General::OVERLAY_PREFIX);
		std::string name;
		if (db->Get(key, name) && priv_key_.From(name)) {
			peer_node_address_ = priv_key_.GetBase58Address();
		}
		else {
			peer_node_address_ = priv_key_.GetBase58Address();
			db->Put(key, priv_key_.GetBase58PrivateKey());
		}

		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("peer-manager")) {
			return false;
		}

		StatusModule::RegisterModule(this);

		return true;
	}

	bool PeerManager::Exit() {
		asio_service_.stop();
		bool ret3 = thread_ptr_->JoinWithStop();

		bool ret1 = transaction_network_.Exit();
		bool ret2 = consensus_network_.Exit();
		return ret1 && ret2;
	}


	void PeerManager::Broadcast(const PeerMessagePointer &message) {
		switch (message->header_.type) {
		case PeerMessage::PEER_MESSAGE_TRANSACTION:
		case PeerMessage::PEER_MESSAGE_PAYLOAD:
			transaction_network_.Broadcast(message);
			break;
		default:
			consensus_network_.Broadcast(message);
			break;
		}
	}


	bool PeerManager::SendMessage(int64_t peer_id, PeerMessagePointer &message) {
		bool ret = false;
		switch (message->header_.type) {
		case PeerMessage::PEER_MESSAGE_TRANSACTION:
		case PeerMessage::PEER_MESSAGE_TXHASHSET:
		case PeerMessage::PEER_MESSAGE_GETTXHASHSET:
			ret = transaction_network_.SendMessage(peer_id, message);
			break;
		default:
			ret = consensus_network_.SendMessage(peer_id, message);
			break;
		}

		return ret;
	}


	bool PeerManager::OnGetLedgers(const PeerMessagePointer &message, Peer *peer) {
		LedgerManager::Instance().ReceiveGetLedgers(message, peer->peer_id());
		return true;
	}

	bool PeerManager::OnLedgers(const PeerMessagePointer &message, Peer *peer) {
		LedgerManager::Instance().OnReceiveLedgers(message, peer->peer_id());
		return true;
	}

	bool PeerManager::OnPbft(const PeerMessagePointer &message, Peer *peer) {
		const protocol::PbftEnv *env = (const protocol::PbftEnv *)message->data_;
		if (!env->IsInitialized()) {
			LOG_ERROR("Pbft env is not initialize");
		}

		//should in validators
		ConsensusMsg msg(*env);
		if (ConsensusManager::Instance().GetConsensus()->GetValidatorIndex(msg.GetNodeAddress()) < 0) {
			LOG_TRACE("Cann't find the validator(%s) in list", msg.GetNodeAddress());
			return false;
		}

		peer->GetPeerNetwork()->ReceiveBroadcastMsg(message, peer->peer_id());

		std::string hash = utils::String::Bin4ToHexString(HashWrapper::Crypto(message->GetString()));
		std::string block_seq_log = msg.GetBlockSeq() > 0 ? utils::String::Format(" block(" FMT_I64 ")", msg.GetBlockSeq()) : "";
		LOG_INFO("[" FMT_I64 "]On pbft hash(%s), receive consensus from node address(%s) sequence(" FMT_I64 ")%s pbft type(%s)",
			peer->peer_id(), hash.c_str(), msg.GetNodeAddress(), msg.GetSeq(), block_seq_log.c_str(),
			PbftDesc::GetMessageTypeDesc(msg.GetPbft().pbft().type()));
		if (!GlueManager::Instance().ConsensusHasRecv(msg)) {
			LOG_INFO("Pbft hash(%s) would be processed", hash.c_str());
			GlueManager::Instance().RecvConsensus(msg);
		}

		return true;
	}

	bool PeerManager::OnGetTxHashSet(const PeerMessagePointer &message, Peer *peer) {
		const protocol::GetTxHashSet *gettxhashset = (const protocol::GetTxHashSet *)message->data_;
		auto txset = GlueManager::Instance().GetTxSet(gettxhashset->hash());
		if (txset) {
			PeerMessagePointer ptr = PeerMessage::NewTxHashSet();
			protocol::TxHashSet *txs = (protocol::TxHashSet *)ptr->data_;
			txs->set_hash(gettxhashset->hash());
			txs->set_previous_ledger_hash(txset->PreviousLedgerHash());
			for (TransactionSetFrm::TxSetByAccountSeq::iterator it = txset->txset_by_acc_seq_.begin();
				it != txset->txset_by_acc_seq_.end(); it++) {
				*txs->add_hashs() = (*it)->GetFullHash();
			}
			peer->SendMessage(ptr->ToString());

			LOG_INFO("peer(%s) gets txhashset hash(%s), preledger hash(%s), send %d tx hashs",
				peer->peer_address().ToIpPort().c_str(),
				utils::String::Bin4ToHexString(gettxhashset->hash()).c_str(),
				utils::String::Bin4ToHexString(txset->PreviousLedgerHash()).c_str(),
				txs->hashs_size());
		}
		else {
			LOG_INFO("peer(%s) gets txhashset hash(%s), but not found",
				peer->peer_address().ToIpPort().c_str(),
				utils::String::Bin4ToHexString(gettxhashset->hash()).c_str());
			peer->SendDontHave(PeerMessage::PEER_MESSAGE_TXHASHSET, gettxhashset->hash());
		}

		return true;
	}

	bool PeerManager::OnGetTxSet(const PeerMessagePointer &message, Peer *peer) {
		const protocol::GetTxSet *gettxset = (const protocol::GetTxSet *)message->data_;
		auto txset = GlueManager::Instance().GetTxSet(gettxset->hash());
		if (txset) {
			PeerMessagePointer ptr = PeerMessage::NewTxSet();
			protocol::TxSet *txs = (protocol::TxSet *)ptr->data_;
			txs->set_hash(gettxset->hash());
			txs->set_previous_ledger_hash(txset->PreviousLedgerHash());
			for (::google::protobuf::RepeatedPtrField< ::std::string>::const_iterator it = gettxset->hashs().begin();
				it != gettxset->hashs().end(); it++) {
				TransactionSetFrm::TxMapByHash::iterator i = txset->txmap_by_hash_.find(*it);
				if (i == txset->txmap_by_hash_.end()) {
					LOG_INFO("peer(%s) OnGetTxSet txset hash(%s),preledger hash(%s), but some not found",
						peer->peer_address().ToIpPort().c_str(),
						utils::String::Bin4ToHexString(gettxset->hash()).c_str(),
						utils::String::Bin4ToHexString(txset->PreviousLedgerHash()).c_str());
					peer->SendDontHave(PeerMessage::PEER_MESSAGE_TXSET, gettxset->hash());
					return true;
				}
				*txs->add_tran_envs() = i->second->GetTransactionEnv();
			}
			peer->SendMessage(ptr->ToString());

			LOG_INFO("peer(%s) OnGetTxSet txset hash(%s),preledger hash(%s), send %d transactions",
				peer->peer_address().ToIpPort().c_str(),
				utils::String::Bin4ToHexString(gettxset->hash()).c_str(),
				utils::String::Bin4ToHexString(txset->PreviousLedgerHash()).c_str(),
				txs->tran_envs_size());
		}
		else {
			LOG_INFO("peer(%s) OnGetTxSet txset hash(%s), but not found",
				peer->peer_address().ToIpPort().c_str(),
				utils::String::Bin4ToHexString(gettxset->hash()).c_str());
			peer->SendDontHave(PeerMessage::PEER_MESSAGE_TXSET, gettxset->hash());
		}

		return true;
	}

	bool PeerManager::OnTxHashSet(const PeerMessagePointer &message, Peer *peer) {
		const protocol::TxHashSet *hashSet = (const protocol::TxHashSet *)message->data_;

		LOG_INFO("peer(%s) OnTxHashSet txset hash(%s), preledger hash(%s),  size (%d) ",
			peer->peer_address().ToIpPort().c_str(),
			utils::String::Bin4ToHexString(hashSet->hash()).c_str(),
			utils::String::Bin4ToHexString(hashSet->previous_ledger_hash()).c_str(),
			hashSet->hashs_size());

		//1 check if it is wanted
		if (!GlueManager::Instance().WantedTxSet(hashSet->hash())) {
			LOG_WARN("unwanted txhashset received from %s, attack?", peer->peer_address().ToIpPort().c_str());
			return false;
		}

		//2 check if the hash is correct
		if (!TransactionSetFrm::CheckTxSetHash(hashSet)) {
			LOG_WARN("bad txhashset received from %s, attack?", peer->peer_address().ToIpPort().c_str());
			return false;
		}
		//3 collect missing transactions and request


		std::unordered_set<std::string> missingTxSet;
		GlueManager::Instance().RecvTxHashSet(*hashSet, missingTxSet);

		if (missingTxSet.size() > 0) {
			LOG_INFO("OnTxHashSet need to receive %d missing transactions", missingTxSet.size());

			std::shared_ptr<PeerMessage>  msg = PeerMessage::NewGetTxSet();

			protocol::GetTxSet *gettxset = (protocol::GetTxSet *)msg->data_;

			gettxset->set_hash(hashSet->hash());
			for (auto const & hash : missingTxSet) {
				*gettxset->add_hashs() = hash;
			}

			peer->SendMessage(msg->ToString());
		}

		return true;
	}
	bool PeerManager::OnTxSet(const PeerMessagePointer &message, Peer *peer) {
		const protocol::TxSet *txset = (const protocol::TxSet *)message->data_;

		//1 check if it is wanted
		if (!GlueManager::Instance().WantedTxSet(txset->hash())) {
			LOG_WARN("unwanted txhashset received from %s, attack?", peer->peer_address().ToIpPort().c_str());
			return false;
		}

		std::string hash_out = utils::String::Bin4ToHexString(utils::Sha256::Crypto(txset->SerializeAsString()));
		LOG_INFO("peer(%s) OnTxSet received %d transactions(hash:%s), send them to slave to verify",
			peer->peer_address().ToIpPort().c_str(), txset->tran_envs_size(), hash_out.c_str());

		int32_t send_size = 0;
		for (::google::protobuf::RepeatedPtrField< ::protocol::TransactionEnv >::const_iterator it = txset->tran_envs().begin();
			it != txset->tran_envs().end(); it++) {
			PeerMessagePointer msg = PeerMessage::NewTransaction();
			*((protocol::TransactionEnvWrapper *)msg->data_)->mutable_transaction_env() = *it;

			std::string buffer = msg->ToString();

			//if a txset is too big to finish fetch in seconds, fetcher will try again, so there may be more duplicated txset received.
			//so it is necessary to check before verification to defend attack. hash calculation is much faster than verification
			if (GlueManager::Instance().WantedTx(txset->hash(),
				utils::Sha256::Crypto(it->transaction().SerializeAsString()))) {
				protocol::SlaveVerifyRequest svr;
				svr.set_peer_id(peer->peer_id());
				svr.set_peer_message(buffer.c_str(), buffer.size());
				svr.set_txset_hash(txset->hash());

				send_size++;
				MasterService::Instance().SendToSlave(ZMQ_TXSET_TX, svr.SerializeAsString());
			}
		}
		LOG_INFO("peer(%s) transacations(hash:%s) have sent %d tx",
			peer->peer_address().ToIpPort().c_str(), hash_out.c_str(), send_size);

		return true;
	}

	bool PeerManager::OnPeers(const PeerMessagePointer &message, Peer *peer) {
		utils::StringMap values;
		const protocol::Peers *peers = (const protocol::Peers *)message->data_;
		for (int i = 0; i < peers->peers_size(); i++) {
			const protocol::Peer &peerp = peers->peers(i);
			values["ip"] = peerp.ip();
			values["port"] = utils::String::ToString(peerp.port());
			values["num_failures"] = utils::String::ToString(peerp.num_failures());
			peer->GetPeerNetwork()->AddReceivedPeers(values);
		}

		return true;
	}

	bool PeerManager::OnGetPeers(const PeerMessagePointer &message, Peer *peer) {
		return peer->SendPeers(peer->GetPeerNetwork()->GetPeersCache());
	}

	bool PeerManager::OnHello(const PeerMessagePointer &message, Peer *peer) {
		do {
			PeerNetwork *network = peer->GetPeerNetwork();
			const  protocol::Hello *hello = (const protocol::Hello *)message->data_;

			peer->SetPeerInfo(*hello);

			if ((network->GetNetworkType() == PeerNetwork::CONSENSUS && hello->network_type() != bubi::General::CONSENSUS_NET_MAGICWORD)
				|| (network->GetNetworkType() == PeerNetwork::TRANSACTION && hello->network_type() != bubi::General::TRANSACTION_NET_MAGICWORD)) {
				LOG_ERROR("Wrong network connection %s from %s", hello->network_type().c_str(), hello->nodeid().c_str());
				break;
			}

			if (network->NodeExist(hello->nodeid(), peer->peer_id())) {
				LOG_ERROR("disconnect duplicated connection with %s", peer->peer_address().ToIp().c_str());
				break;
			}

			if (peer_node_address_ == hello->nodeid()) {
				LOG_ERROR("Peer connect self break");
				break;
			}

			if (hello->overlayversion() < bubi::General::OVERLAY_MIN_VERSION) {
				LOG_ERROR("Peer's overlay version(%d) is too old,", hello->overlayversion());
				break;
			}
			if (hello->ledger_version() < bubi::General::LEDGER_MIN_VERSION) {
				LOG_ERROR("Peer's leger version(%d) is too old,", hello->ledger_version());
				break;
			}
			if (hello->listeningport() <= 0 ||
				hello->listeningport() > utils::MAX_UINT16) {
				LOG_ERROR("Peer's listen port(%d) is not valid", hello->listeningport());
				break;
			}

			LOG_INFO("Recv hello, peer(%s) is active", peer->GetRemoteAddress().ToIpPort().c_str());
			peer->SetActiveTime(utils::Timestamp::HighResolution());
			peer->SetStateChanged(true);

			if (peer->in_bound()) {
				const P2pNetwork &p2p_configure =
					network->GetNetworkType() == PeerNetwork::CONSENSUS
					? bubi::Configure::Instance().p2p_configure_.consensus_network_configure_
					: bubi::Configure::Instance().p2p_configure_.transaction_network_configure_;

				peer->SendHello(p2p_configure.listen_port_, peer_node_address_,
					network->GetNetworkType() == PeerNetwork::CONSENSUS ?
					bubi::General::CONSENSUS_NET_MAGICWORD : bubi::General::TRANSACTION_NET_MAGICWORD);
			}
			else {
			}

			return true;
		} while (false);

		peer->OnError();
		return false;
	}

	bool PeerManager::OnPayLoad(const PeerMessagePointer &message, Peer *peer) {
		LOG_INFO("MQSERVER listen message  peerid (%s)", peer->remote_node_id().c_str());
		if (!TransactionNetwork().ReceiveBroadcastMsg(message, peer->peer_id())) {
			LOG_ERROR("duplicate messages");
			return false;
		}

		const  protocol::PayLoadEnv *payload_env = (const protocol::PayLoadEnv *)message->data_;
		const  protocol::PayLoad* payload = &payload_env->payload();

		const protocol::Signature* signature = &payload_env->signature();

		bubi::PublicKey pub(signature->public_key());
		if (payload->src_peer_addr() != pub.GetBase58Address()) {
			LOG_ERROR("Verify signature address failed");
			return false;
		}

		std::string transStr = payload->SerializeAsString();
		if (!PublicKey::Verify(transStr, signature->sign_data(), signature->public_key())) {
			LOG_ERROR("Verify signature data failed");
			return false;
		}
		LOG_INFO("MQSERVER Broadcast src (%s),des (%s)", payload->src_peer_addr().c_str(), payload->des_peer_addrs(0).c_str());
		Broadcast(message);
		if (payload->timestamp() < (utils::Timestamp::HighResolution() - 60 * utils::MICRO_UNITS_PER_SEC)) {
			LOG_ERROR("MQSERVER message time out");
			return false;
		}
		for (std::string des_addr : payload->des_peer_addrs()) {
			if (peer_node_address_ == des_addr) {

				protocol::ChainPeerMessage cpm;
				cpm.set_src_peer_addr(payload->src_peer_addr());
				cpm.mutable_des_peer_addrs()->CopyFrom(payload->des_peer_addrs());
				cpm.set_data(payload->data());
				std::string str = cpm.SerializeAsString();
				LOG_INFO("MQSERVER recv message (%s)", payload->data().c_str());
				bubi::MQServer::Instance().Send(ZMQ_CHAIN_PEER_MESSAGE, str);
				bubi::WebSocketServer::Instance().BroadcastMsg(protocol::CHAIN_PEER_MESSAGE, str);
			}
		}
		return true;
	}

	bool PeerManager::BroadcastPayLoad(protocol::ChainPeerMessage &cpm) {
		PeerMessagePointer message = PeerMessage::NewPayLoad();

		protocol::PayLoadEnv *pl_env = (protocol::PayLoadEnv *) message->data_;
		protocol::PayLoad *pl = pl_env->mutable_payload();

		pl->set_src_peer_addr(peer_node_address_);
		pl->mutable_des_peer_addrs()->CopyFrom(cpm.des_peer_addrs());
		pl->set_timestamp(utils::Timestamp::HighResolution());
		pl->set_data(cpm.data());

		protocol::Signature *sign = pl_env->mutable_signature();
		sign->set_public_key(priv_key_.GetBase58PublicKey());
		sign->set_sign_data(priv_key_.Sign(pl->SerializeAsString()));
		LOG_INFO("MQSERVER Broadcast src (%s),des (%s)", pl->src_peer_addr().c_str(), pl->des_peer_addrs(0).c_str());
		Broadcast(message);
		return true;
	}


	void PeerManager::OnPeerConnect(Peer *peer) {
		PeerNetwork *network = peer->GetPeerNetwork();
		const P2pNetwork &p2p_configure =
			network->GetNetworkType() == PeerNetwork::CONSENSUS
			? bubi::Configure::Instance().p2p_configure_.consensus_network_configure_
			: bubi::Configure::Instance().p2p_configure_.transaction_network_configure_;

		peer->SendHello(p2p_configure.listen_port_, peer_node_address_,
			network->GetNetworkType() == PeerNetwork::CONSENSUS ?
			bubi::General::CONSENSUS_NET_MAGICWORD : bubi::General::TRANSACTION_NET_MAGICWORD);
	}

	bool PeerManager::OnDontHave(const PeerMessagePointer &message, Peer *peer) {
		const protocol::DontHave *env = (const protocol::DontHave *)message->data_;
		GlueManager::Instance().peerDoesntHave(env->type(), env->hash(), peer->peer_id());
		LOG_TRACE("On donthave");
		return true;
	}

	bool PeerManager::OnPing(const PeerMessagePointer &message, Peer *peer) {
		const protocol::Ping *ping = (const protocol::Ping *)message->data_;
		return peer->SendPong(ping->nonce());
	}

	bool PeerManager::OnPong(const PeerMessagePointer &message, Peer *peer) {
		int64_t now = utils::Timestamp::HighResolution();
		peer->UpdateDelay(now);
		return true;
	}


	void PeerManager::GetModuleStatus(Json::Value &data) {
		data["name"] = "peer_manager";
		data["peer_node_address"] = peer_node_address_;
		consensus_network_.GetModuleStatus(data["consensus_network"]);
		transaction_network_.GetModuleStatus(data["transaction_network"]);
	}

}
