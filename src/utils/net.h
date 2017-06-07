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

#ifndef UTILS_NET_H_
#define UTILS_NET_H_

#include "common.h"
#include "thread.h"
#include <asio.hpp>
#include <asio/ssl.hpp>

#ifdef WIN32
#include <mstcpip.h>
#include <iphlpapi.h>
#endif

#ifndef WIN32
typedef int SOCKET;
#endif

namespace utils {
	class InetAddress {
	public:
		InetAddress();
		InetAddress(const InetAddress &address);
		explicit InetAddress(uint16_t port_litter_endian);
		explicit InetAddress(const std::string &ip);
		explicit InetAddress(const std::string &ip, uint16_t port_litter_endian);
		explicit InetAddress(uint32_t ip_big_endian, uint16_t port_litter_endian);
		explicit InetAddress(const struct sockaddr_in &addr);
		explicit InetAddress(const struct in_addr &in_addr);
		explicit InetAddress(const asio::ip::tcp::endpoint &endpoint);
		explicit InetAddress(const asio::ip::udp::endpoint &endpoint);
		std::string ToIp() const;
		std::string ToIpPort() const;
		asio::ip::tcp::endpoint tcp_endpoint() const;
		asio::ip::udp::endpoint udp_endpoint() const;
		// default copy/assignment are Okay
		const struct sockaddr_in &sock_addr_in() const;
		const struct sockaddr *sock_addr() const;
		socklen_t size() const;
		void set_sock_addr(const struct sockaddr_in &addr);
		uint32_t IpBigEndian() const;
		uint16_t PortBigEndian() const;
		int32_t GetFamily() const;
		bool Resolve(const std::string &address);
		void SetPort(uint16_t port){ addr_.sin_port = htons(port); };
		uint16_t GetPort() const;
		void operator=(struct sockaddr_in addr);
		void operator=(const InetAddress &address);
		bool operator==(const InetAddress &address);
		bool IsNone() const;
		bool IsLoopback() const;
		bool IsAny() const;
		static InetAddress Any();
		static InetAddress Loopback();
		static InetAddress None();
	private:
		struct sockaddr_in addr_;
	};

	std::string GetPeerName(SOCKET s);
	typedef std::vector<utils::InetAddress> InetAddressVec;
	typedef std::list<utils::InetAddress> InetAddressList;

	class net {
	public:
		net();
		~net();
		static bool Initialize();
		static bool GetNetworkAddress(InetAddressVec &addresses);
	};

	class Socket {
	public:
		typedef enum SocketTypeEnum {
			SOCKET_TYPE_TCP = 0,
			SOCKET_TYPE_UDP = 1
		}SocketType;
		explicit Socket();
		~Socket();
		bool Create(SocketType type, const InetAddress &address);
		InetAddress local_address() const;
		bool Close();
		bool SetBlock(bool block);
		void SetKeepAlive(bool on);
		void SetTcpNoDelay(bool on);
		bool Connect(const InetAddress &server);
		bool Connect(const InetAddress &server, int timeout_milli);
		bool Accept(Socket &new_socket);
		InetAddress peer_address() const;
		int Send(const void *buffer, int size);
		bool SendComplete(const void *buffer, int size);
		int Receive(void *buffer, int size);
		int SendTo(const void *buffer, int size, const utils::InetAddress &address);
		int ReceiveFrom(void *buffer, int size, const utils::InetAddress &address);
		SOCKET handle() const;
		bool IsValid() const;
		static bool IsNomralError(uint32_t error_code);
		static const SOCKET INVALID_HANDLE;
		static const int ERR_VALUE;
	protected:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(Socket);
		SOCKET handle_;
		InetAddress local_address_;
		utils::InetAddress peer_address_;
		bool blocked_;
	};
	class AsyncIoThread : public utils::Thread {
		friend class AsyncIo;
	protected:
		asio::io_service io_service;
	public:
		AsyncIoThread();
		virtual ~AsyncIoThread();
		virtual void Run();
		void Stop();
	};

	typedef std::vector<AsyncIoThread *> AsyncIoThreadArray;
	typedef std::shared_ptr<asio::io_service::work> work_ptr;

	class AsyncIo {
	public:
		AsyncIo();
		~AsyncIo();
		bool Create(size_t poll_size, size_t thread_count);
		bool Close();
		bool AttachServiceIo(asio::io_service *io);
		asio::io_service *GetIoService();
	private:
		bool enabled_;
		size_t next_id_;
	protected:
		AsyncIoThreadArray *threads_ptr_;
		std::vector<work_ptr> work_;
		asio::io_service *io_service_ptr_;
	};

	class AsyncSocket {
		DISALLOW_COPY_AND_ASSIGN(AsyncSocket);
	protected:
		AsyncSocket(AsyncIo *asyncio_ptr);
		virtual ~AsyncSocket();
		virtual bool Bind(const utils::InetAddress &address) = 0;
		virtual bool Close() = 0;
		virtual bool IsValid() = 0;
		virtual bool SetKeepAlive(bool on) = 0;
		virtual bool SetReuse(bool on) = 0;
	public:
		InetAddress local_address();
		InetAddress peer_address();
	protected:
		AsyncIo *asyncio_ptr_;
		InetAddress local_address_;
		InetAddress peer_address_;
	};

	class AsyncSocketAcceptor;

	class AsyncSocketTcp : public AsyncSocket {
		friend class AsyncIo;
		friend class AsyncSocketAcceptor;
	public:
		AsyncSocketTcp(AsyncIo *asyncio_ptr);
		virtual ~AsyncSocketTcp();
		virtual bool Bind(const utils::InetAddress &address);
		virtual bool Close();
		virtual bool IsValid();
		virtual bool SetKeepAlive(bool on);
		virtual bool SetReuse(bool on);
		bool SetTcpNoDelay(bool on);
		bool Connect(const utils::InetAddress &server);
		bool AsyncConnect(const utils::InetAddress &server);
		size_t SendSome(const void *buffer, size_t size);
		int AsyncSendSome(const void *buffer, size_t size);
		size_t ReceiveSome(void *buffer, size_t size);
		int AsyncReceiveSome(size_t max_size = utils::ETH_MAX_PACKET_SIZE);
		virtual void OnError();
		virtual void OnConnect();
		virtual void OnSend(std::size_t bytes_transferred);
		virtual void OnReceive(void *buffer, size_t bytes_transferred);
	private:
		asio::ip::tcp::socket *tcpsocket_ptr_;
		char buffer_[utils::ETH_MAX_PACKET_SIZE];
	};

	class IAsyncSocketAcceptorNotify {
	public:
		IAsyncSocketAcceptorNotify() {};
		~IAsyncSocketAcceptorNotify() {};
		virtual void OnAccept(AsyncSocketAcceptor *acceptor) = 0;
		virtual void OnError(AsyncSocketAcceptor *acceptor) = 0;
	};

	class AsyncSocketSsl;

	class AsyncSocketAcceptor : public AsyncSocket {
		friend class AsyncIo;
	public:
		AsyncSocketAcceptor(AsyncIo *asyncio_ptr, IAsyncSocketAcceptorNotify *notify_ptr = NULL);
		virtual ~AsyncSocketAcceptor();
		virtual bool Bind(const utils::InetAddress &address);
		virtual bool Close();
		virtual bool IsValid();
		virtual bool SetKeepAlive(bool on);
		virtual bool SetReuse(bool on);
		bool Listen(int back_log = SOMAXCONN);
		bool Accept(AsyncSocketTcp *new_socket);
		bool AsyncAccept(AsyncSocketTcp *new_socket);
		bool AsyncAccept(AsyncSocketSsl *new_socket);
		virtual void OnAccept();
		virtual void OnError();
	private:
		asio::ip::tcp::acceptor *acceptor_ptr_;
		IAsyncSocketAcceptorNotify *notify_lptr_;
		AsyncSocketTcp *tcpsocket_lptr_;
		AsyncSocketSsl *sslsocket_lptr_;
	};

	class AsyncSocketUdp : public AsyncSocket {
		friend class AsyncIo;
	public:
		AsyncSocketUdp(AsyncIo *asyncio_ptr);
		virtual ~AsyncSocketUdp();
		virtual bool Bind(const utils::InetAddress &address);
		virtual bool Close();
		virtual bool IsValid();
		virtual bool SetKeepAlive(bool on);
		virtual bool SetReuse(bool on);
		size_t SendTo(const void *buffer, size_t size, const utils::InetAddress &address);
		size_t ReceiveFrom(void *buffer, size_t size, utils::InetAddress &address);
		int AsyncSendTo(const void *buffer, size_t size, const utils::InetAddress &address);
		int AsyncReceiveFrom(size_t max_size = utils::ETH_MAX_PACKET_SIZE);
		virtual void OnError();
		virtual void OnSend(size_t bytes_transferred);
		virtual void OnReceive(void *buffer, size_t bytes_transferred, const utils::InetAddress &address);
	private:
		asio::ip::udp::socket *udpsocket_ptr_;
		char buffer_[utils::ETH_MAX_PACKET_SIZE];
		asio::ip::udp::endpoint sender_endpoint_;
	};
	typedef asio::ssl::stream<asio::ip::tcp::socket> SslSocket;
	class AsyncSocketSsl : public AsyncSocket {
		friend class AsyncIo;
		friend class AsyncSocketAcceptor;
	public:
		AsyncSocketSsl(AsyncIo *asyncio_ptr, asio::ssl::context& context);
		virtual ~AsyncSocketSsl();
		virtual bool Bind(const utils::InetAddress &address);
		virtual bool Close();
		virtual bool IsValid();
		virtual bool SetKeepAlive(bool on);
		virtual bool SetReuse(bool on);
		bool SetTcpNoDelay(bool on);
		bool Connect(const utils::InetAddress &server);
		bool AsyncConnect(const utils::InetAddress &server);
		size_t SendSome(const void *buffer, size_t size);
		int AsyncSendSome(const void *buffer, size_t size);
		size_t ReceiveSome(void *buffer, size_t size);
		int AsyncReceiveSome(size_t max_size = utils::ETH_MAX_PACKET_SIZE);
		bool AsyncHandShake();
		bool HandShake();
		virtual void OnError();
		virtual void OnConnect();
		virtual void OnSend(std::size_t bytes_transferred);
		virtual void OnReceive(void *buffer, size_t bytes_transferred);
		virtual bool OnVerifyCertificate(bool preverified, asio::ssl::verify_context& ctx);
		virtual void OnHandShake();
	private:
		SslSocket *sslsocket_ptr_;
		char buffer_[utils::ETH_MAX_PACKET_SIZE];
	};
	class NameResolver {
	public:
		NameResolver(utils::AsyncIo *async_io_lptr);
		~NameResolver();
		bool Query(const std::string &name, utils::InetAddressList &list);
	private:
		utils::AsyncIo *async_io_lptr_;
		asio::ip::tcp::resolver *resolver_ptr_;
	};
}

#endif

