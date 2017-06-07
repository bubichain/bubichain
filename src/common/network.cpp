#include <utils/timestamp.h>
#include <utils/logger.h>
#include "general.h"
#include "network.h"

namespace bubi {

	Connection::Connection(server *server_h, client *client_h, connection_hdl con, const std::string &uri, int64_t id) :
		server_(server_h),
		client_(client_h),
		handle_(con),
		in_bound_(server_h ? true : false),
		uri_(uri), 
		id_(id), 
		sequence_(0){
		connect_start_time_ = 0;
		connect_end_time_ = 0;
		last_receive_time_ = 0;
		last_send_time_ = 0;

		std::error_code ec;
		last_receive_time_ = connect_start_time_ = utils::Timestamp::HighResolution();
		if (server_){
			connect_end_time_ = connect_start_time_;

			server::connection_ptr con = server_->get_con_from_hdl(handle_, ec);
			if(!ec) peer_address_ = utils::InetAddress(con->get_remote_endpoint());
		}
		else {
			client::connection_ptr con = client_->get_con_from_hdl(handle_, ec);
			if (!ec) peer_address_ = utils::InetAddress(con->get_host(), con->get_port());
		}
	}
	Connection::~Connection() {}

	utils::InetAddress Connection::GetPeerAddress() const {
		return peer_address_;
	}

	void Connection::TouchReceiveTime() {
		last_receive_time_ = utils::Timestamp::HighResolution();
	}

	bool Connection::NeedPing(int64_t interval) {
		return utils::Timestamp::HighResolution() - last_send_time_ > interval;
	}

	void Connection::SetConnectTime() {
		connect_end_time_ = utils::Timestamp::HighResolution();
	}

	int64_t Connection::GetId() const{
		return id_;
	}

	connection_hdl Connection::GetHandle() const {
		return handle_;
	}

	websocketpp::lib::error_code Connection::GetErrorCode() const {
		std::error_code ec;
		if (in_bound_) {
			server::connection_ptr con = server_->get_con_from_hdl(handle_, ec);
			if (!ec) {
				ec = con->get_ec();
			}
		}
		else {
			client::connection_ptr con = client_->get_con_from_hdl(handle_, ec);
			if (!ec) {
				ec = con->get_ec();
			}
		}

		return ec;
	}

	bool Connection::InBound() const {
		return in_bound_;
	}

	bool Connection::SendByteMessage(const std::string &message, std::error_code &ec) {
		std::error_code ec1;
		if (in_bound_){
			server_->send(handle_, message, websocketpp::frame::opcode::BINARY, ec1);
		} else{
			client_->send(handle_, message, websocketpp::frame::opcode::BINARY, ec1);
		}

		if (ec1.value() == 0) {
			return true;
		} else{
			ec = ec1;
			return false;
		}
	}

	bool Connection::Ping(std::error_code &ec) {
		do {
			std::error_code ec1;
			if (in_bound_) {
				server::connection_ptr con = server_->get_con_from_hdl(handle_, ec1);
				if (ec1.value() != 0) break;
				std::string payload = utils::String::Format("%s - %d", utils::Timestamp::Now().ToFormatString(true).c_str(), rand());
				con->ping(payload, ec);
			}
			else {
				client::connection_ptr con = client_->get_con_from_hdl(handle_, ec1);
				if (ec1.value() != 0) break;
				std::string payload = utils::String::Format("%s - %d", utils::Timestamp::Now().ToFormatString(true).c_str(), rand());
				con->ping(payload, ec);
			}

			last_send_time_ = utils::Timestamp::HighResolution();
		} while (false);

		return ec.value() == 0;
	}

	bool Connection::SendMessage(int64_t type, bool request, int64_t sequence, const std::string &data, std::error_code &ec) {
		protocol::WsMessage message;
		message.set_type(type);
		message.set_request(request);
		message.set_sequence(sequence);
		message.set_data(data);
		return SendByteMessage(message.SerializeAsString(), ec);
	}

	bool Connection::SendRequest(int64_t type, const std::string &data, std::error_code &ec) {
		protocol::WsMessage message;
		message.set_type(type);
		message.set_request(true);
		message.set_sequence(sequence_++);
		message.set_data(data);
		return SendByteMessage(message.SerializeAsString(), ec);
	}

	bool Connection::SendResponse(const protocol::WsMessage &message, const std::string &data, std::error_code &ec) {
		return SendMessage(message.type(), false, message.sequence(), data, ec);
	}

	bool Connection::Close(const std::string &reason) {
		std::error_code ec1;
		if (server_){
			server_->close(handle_, 0, reason, ec1);
		} else{
			client_->close(handle_, 0, reason, ec1);
		}

		return ec1.value() == 0;
	}

	bool Connection::IsConnectExpired(int64_t time_out) const {
		return connect_end_time_ == 0 &&
			utils::Timestamp::HighResolution() - connect_start_time_ > time_out &&
			!in_bound_;
	}

	bool Connection::IsDataExpired(int64_t time_out) const {
		return utils::Timestamp::HighResolution() - last_receive_time_ > time_out;
	}

	void Connection::ToJson(Json::Value &status) const {
		status["id"] = id_;
		status["in_bound"] = in_bound_;
		status["peer_address"] = GetPeerAddress().ToIpPort();
	}

	Network::Network() : next_id_(0), enabled_(false){
		server_.init_asio();
		server_.set_reuse_addr(true);
		// Register handler callbacks
		server_.set_open_handler(bind(&Network::OnOpen, this, _1));
		server_.set_close_handler(bind(&Network::OnClose, this, _1));
		server_.set_fail_handler(bind(&Network::OnFailed, this, _1));
		server_.set_message_handler(bind(&Network::OnMessage, this, _1, _2));
		server_.set_pong_handler(bind(&Network::OnPong, this, _1, _2));
		server_.clear_access_channels(websocketpp::log::alevel::all);
		server_.clear_error_channels(websocketpp::log::elevel::all);

		client_.clear_access_channels(websocketpp::log::alevel::all);
		client_.clear_error_channels(websocketpp::log::elevel::all);
		client_.init_asio();
	}

	Network::~Network() {
		for (ConnectionMap::iterator iter = connections_.begin();
			iter != connections_.end();
			iter++) {
			delete iter->second;
		}

		for (ConnectionMap::iterator iter = connections_delete_.begin();
			iter != connections_delete_.end();
			iter++) {
			delete iter->second;
		}
	}

	void Network::OnOpen(connection_hdl hdl) {
		utils::MutexGuard guard_(conns_list_lock_);
		int64_t new_id = next_id_++;
		Connection *conn = CreateConnectObject(&server_, NULL, hdl, "", new_id);
		connections_.insert(std::make_pair(new_id, conn));
		connection_handles_.insert(std::make_pair(hdl, new_id));

		LOG_INFO("Peer accepted, ip(%s)", conn->GetPeerAddress().ToIpPort().c_str());
		//peer->Ping(ec_);
		if (!OnConnectOpen(conn)) { //delete
			conn->Close("connections exceed");
			RemoveConnection(conn);
		}
	}

	void Network::OnClose(connection_hdl hdl) {
		utils::MutexGuard guard_(conns_list_lock_);
		Connection *conn = GetConnection(hdl);
		if (conn) {
			LOG_INFO("Peer closed, ip(%s)", conn->GetPeerAddress().ToIpPort().c_str());
			OnDisconnect(conn);
			RemoveConnection(conn);
		} 
	}

	void Network::OnFailed(connection_hdl hdl) {
		utils::MutexGuard guard_(conns_list_lock_);
		Connection *conn = GetConnection(hdl);
		if (conn) {
			LOG_ERROR("Peer closed, ip(%s), error desc(%s)", conn->GetPeerAddress().ToIpPort().c_str(), conn->GetErrorCode().message().c_str());
			OnDisconnect(conn);
			RemoveConnection(conn);
		}
	}

	void Network::OnMessage(connection_hdl hdl, server::message_ptr msg) {

		do {
			protocol::WsMessage message; 
			try {
				message.ParseFromString(msg->get_payload());
			}
			catch (std::exception const e) {
				LOG_ERROR("Parse websocket message failed(%s)", e.what());
				return ;
			}

			utils::MutexGuard guard(conns_list_lock_);
			Connection *conn = GetConnection(hdl);
			if (!conn) { break; }

			conn->TouchReceiveTime();
			if (message.request()){
				MessageConnPocMap::iterator iter = request_methods_.find(message.type());
				if (iter == request_methods_.end()) break;
				MessageConnPoc proc = iter->second;
				if (!proc(message, conn)) RemoveConnection(conn); 
			} else{
				MessageConnPocMap::iterator iter = response_methods_.find(message.type());
				if (iter == response_methods_.end()) break;
				MessageConnPoc proc = iter->second;
				if (!proc(message, conn))  RemoveConnection(conn);
			}

			return;
		} while (false);
	}

	void Network::Stop() {
		enabled_ = false;
	}

	void Network::Start(const utils::InetAddress &ip) {
		try
		{
			if (!ip.IsNone()) {
				// listen on specified port
				server_.listen(ip.tcp_endpoint());
				// Start the server accept loop
				server_.start_accept();
				LOG_INFO("WebSocket listen at ip(%s)", ip.ToIpPort().c_str());
			}
			enabled_ = true;

			// Start the ASIO io_service run loop
			int64_t last_check_time = 0;
			while (enabled_) {
				if (!ip.IsNone()) server_.poll();
				client_.poll();
				utils::Sleep(1);

				int64_t now = utils::Timestamp::HighResolution();
				if (now  - last_check_time > utils::MICRO_UNITS_PER_SEC) {

					utils::MutexGuard guard_(conns_list_lock_);
					//check ping
					std::list<Connection *> delete_list;
					for (ConnectionMap::iterator iter = connections_.begin();
						iter != connections_.end();
						iter++) {
						if (iter->second->NeedPing(15 * utils::MICRO_UNITS_PER_SEC)) {
							iter->second->Ping(ec_);
						}

						if (iter->second->IsDataExpired(60 * utils::MICRO_UNITS_PER_SEC)) {
							iter->second->Close("expired");
							delete_list.push_back(iter->second);
						}
					}

					//move current connection to delete array
					for (std::list<Connection *>::iterator iter = delete_list.begin();
						iter != delete_list.end();
						iter++) {
						LOG_INFO("Peer closed as expired, ip(%s)", (*iter)->GetPeerAddress().ToIpPort().c_str());
						OnDisconnect(*iter);
						RemoveConnection(*iter);
					}

					//check delete the connections
					for (ConnectionMap::iterator iter = connections_delete_.begin();
						iter != connections_delete_.end();) {
						if (iter->first < now) {
							iter = connections_delete_.erase(iter);
						} else{
							iter++;
						}
					}

					last_check_time = now;
				}
			}
		}
		catch (const std::exception & e) {
			LOG_ERROR("%s", e.what());
		}

		enabled_ = false;
	}

	bool Network::Connect(const std::string &uri) {
		websocketpp::lib::error_code ec;

		client::connection_ptr con = client_.get_connection(uri, ec);

		if (ec) {
			std::cout << "> Connect initialization error: " << ec.message() << std::endl;
			return false;
		}

		utils::MutexGuard guard_(conns_list_lock_);
		int64_t new_id = next_id_++;
		Connection *peer = CreateConnectObject(NULL, &client_, con->get_handle(), uri, new_id);
		connections_.insert(std::make_pair(new_id, peer));
		connection_handles_.insert(std::make_pair(con->get_handle(), new_id));

		con->set_open_handler(bind(&Network::OnClientOpen, this, _1));
		con->set_close_handler(bind(&Network::OnClose, this, _1));
		con->set_message_handler(bind(&Network::OnMessage, this, _1, _2));
		con->set_fail_handler(bind(&Network::OnFailed, this, _1));
		con->set_pong_handler(bind(&Network::OnPong, this, _1, _2));
		
		client_.connect(con);
		return true;
	}

	Connection *Network::GetConnection(int64_t id) {
		ConnectionMap::iterator iter = connections_.find(id);
		if (iter != connections_.end()){
			return iter->second;
		}

		return NULL;
	}

	Connection *Network::GetConnection(connection_hdl hdl) {
		ConnectHandleMap::iterator iter = connection_handles_.find(hdl);
		if (iter == connection_handles_.end()) {
			return NULL;
		}

		return GetConnection(iter->second);
	}

	void Network::RemoveConnection(Connection *conn) {
		connections_.erase(conn->GetId());
		connection_handles_.erase(conn->GetHandle());
		connections_delete_.insert(std::make_pair(utils::Timestamp::HighResolution() + 5 * utils::MICRO_UNITS_PER_SEC,
			conn));
	}

	void Network::OnClientOpen(connection_hdl hdl) {
		Connection * conn = GetConnection(hdl);
		if (conn) {
			LOG_INFO("Peer connected, ip(%s)", conn->GetPeerAddress().ToIpPort().c_str());
			conn->SetConnectTime();

			if (!OnConnectOpen(conn)) { //delete
				conn->Close("no reason");
				RemoveConnection(conn);
			} 
			//conn->Ping(ec_);
		}
	}

	void Network::OnPong(connection_hdl hdl, std::string payload) {
		Connection *peer = GetConnection(hdl);
		if (peer){
			peer->TouchReceiveTime();
			LOG_INFO("Recv Pong, payload(%s)", payload.c_str());
		} 
	}

	bool Network::OnRequestPing(protocol::WsMessage &message, Connection *conn) {
		LOG_INFO("On Ping Request");
		protocol::WsMessage res = message;
		res.set_request(false);
		return conn->SendByteMessage(res.SerializeAsString(), ec_);
	}

	bool Network::OnResponsePing(protocol::WsMessage &message, Connection *conn) {
		LOG_INFO("On Ping Response");
		return true;
	}

	bool Network::Ping(Connection *conn) {
		return conn->SendRequest(1, "ping", ec_);
	}

	Connection *Network::CreateConnectObject(server *server_h, client *client_, connection_hdl con, const std::string &uri, int64_t id) {
		return new Connection(server_h, client_, con, uri, id);
	}

}
