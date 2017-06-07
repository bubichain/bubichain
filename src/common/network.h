#ifndef NETWORK_H_
#define NETWORK_H_

#include <utils/net.h>
#include <utils/strings.h>
#include <utils/net.h>
#include <json/value.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>
#include <proto/message.pb.h>

namespace bubi {
	typedef websocketpp::client<websocketpp::config::asio_client> client;
	typedef websocketpp::server<websocketpp::config::asio> server;

	using websocketpp::connection_hdl;
	using websocketpp::lib::placeholders::_1;
	using websocketpp::lib::placeholders::_2;
	using websocketpp::lib::bind;

	typedef std::set<connection_hdl, std::owner_less<connection_hdl> > con_list;
	
	class Connection {
	private:
		server *server_;
		client *client_;
		connection_hdl handle_;

		//status
		int64_t connect_start_time_;
		int64_t connect_end_time_;

		int64_t last_receive_time_;
		int64_t last_send_time_;

		std::string uri_;
		int64_t id_;
		int64_t sequence_;
		bool in_bound_;
		utils::InetAddress peer_address_;
	public:
		Connection(server *server_h, client *client_, connection_hdl con, const std::string &uri, int64_t id);
		virtual ~Connection();
		
		bool SendByteMessage(const std::string &message, std::error_code &ec);
		bool SendMessage(int64_t type, bool request, int64_t sequence, const std::string &data, std::error_code &ec);
		bool SendRequest(int64_t type, const std::string &data, std::error_code &ec);
		bool SendResponse(const protocol::WsMessage &message, const std::string &data, std::error_code &ec);
		bool Ping(std::error_code &ec);
		bool Close(const std::string &reason);

		bool NeedPing(int64_t interval);
		void TouchReceiveTime();
		void SetConnectTime();
		utils::InetAddress GetPeerAddress() const;
		int64_t GetId() const;
		connection_hdl GetHandle() const;
		websocketpp::lib::error_code GetErrorCode() const;
		bool InBound() const;

		//get status
		bool IsConnectExpired(int64_t time_out) const;
		bool IsDataExpired(int64_t time_out) const;
		virtual void ToJson(Json::Value &status) const;
	};

	typedef std::map<int64_t, Connection *> ConnectionMap;
	typedef std::map<connection_hdl, int64_t, std::owner_less<connection_hdl>> ConnectHandleMap;

	typedef std::function<bool(protocol::WsMessage &message, Connection *peer)> MessageConnPoc;
	typedef std::map<int64_t, MessageConnPoc> MessageConnPocMap;

	class Network {
	protected:
		server server_;
		client client_;

		ConnectionMap connections_;
		ConnectionMap connections_delete_;
		ConnectHandleMap connection_handles_;

		int64_t next_id_;
		bool enabled_;

		std::error_code ec_;
		utils::Mutex conns_list_lock_;
	public:
		Network();
		virtual ~Network();

		void Start(const utils::InetAddress &ip);
		void Stop();
		//for client
		bool Connect(std::string const & uri);
	protected:
		//for server
		void OnOpen(connection_hdl hdl);
		void OnClose(connection_hdl hdl);
		void OnMessage(connection_hdl hdl, server::message_ptr msg);
		void OnFailed(connection_hdl hdl);

		//for client
		void OnClientOpen(connection_hdl hdl);
		//void OnClientMessage(connection_hdl hdl, server::message_ptr msg);

		void OnPong(connection_hdl hdl, std::string payload);

		//Get peer object not thread safe
		Connection *GetConnection(int64_t id);
		Connection *GetConnection(connection_hdl hdl);

		//remove peer,  not thread safe
		void RemoveConnection(Connection *conn);

		//message type to function
		MessageConnPocMap request_methods_;
		MessageConnPocMap response_methods_;

		//send custom message
		bool OnRequestPing(protocol::WsMessage &message, Connection *conn);
		bool OnResponsePing(protocol::WsMessage &message, Connection *conn);
		bool Ping(Connection *conn);

		//could be drived
		virtual Connection *CreateConnectObject(server *server_h, client *client_, connection_hdl con, const std::string &uri, int64_t id);
		virtual void OnDisconnect(Connection *conn) {};
		virtual bool OnConnectOpen(Connection *conn) { return true; };
	};

}
#endif
