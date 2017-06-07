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

#include "net.h"
#include "utils.h"
#include "strings.h"
#include "logger.h"

bool utils::net::Initialize() {

#ifdef WIN32
	WSADATA nWsaData;
	if (WSAStartup(MAKEWORD(2, 2), &nWsaData) != 0) {
		return false;
	}
#endif
	return true;
}

bool utils::net::GetNetworkAddress(InetAddressVec &addresses) {
	bool is_success = false;

#ifdef WIN32

	PIP_ADAPTER_ADDRESSES adapter_address = NULL;
	ULONG out_buffer_length = 0;
	DWORD ret_val = ERROR_SUCCESS;

	for (int i = 0; i < 5; i++) {
		ret_val = GetAdaptersAddresses(AF_INET, 0, NULL, adapter_address, &out_buffer_length);

		if (ret_val != ERROR_BUFFER_OVERFLOW) {
			break;
		}

		if (adapter_address != NULL) {
			HeapFree(GetProcessHeap(), 0, adapter_address);
			adapter_address = NULL;
		}

		adapter_address = (PIP_ADAPTER_ADDRESSES)HeapAlloc(GetProcessHeap(), 0, out_buffer_length);
		if (adapter_address == NULL) {
			ret_val = GetLastError();
			break;
		}
	}

	is_success = (ERROR_SUCCESS == ret_val);
	if (ERROR_SUCCESS == ret_val) {
		PIP_ADAPTER_ADDRESSES pAdapterList = adapter_address;
		while (NULL != pAdapterList) {
			//get ip address list
			ULONG out_buf_len = 0;
			PIP_ADAPTER_INFO  pIPAdapterInfo = (PIP_ADAPTER_INFO)malloc(sizeof(IP_ADAPTER_INFO));
			if ((GetAdaptersInfo(pIPAdapterInfo, &out_buf_len)) == ERROR_BUFFER_OVERFLOW) {
				free(pIPAdapterInfo);
				pIPAdapterInfo = (PIP_ADAPTER_INFO)malloc(out_buf_len);
			}

			if ((GetAdaptersInfo(pIPAdapterInfo, &out_buf_len)) == NO_ERROR) {
				for (PIP_ADAPTER_INFO lpTemp = pIPAdapterInfo; lpTemp != NULL; lpTemp = lpTemp->Next) {
					if (lpTemp->Index != pAdapterList->IfIndex) continue;

					for (IP_ADDR_STRING* lpAddr = &lpTemp->IpAddressList; lpAddr != NULL; lpAddr = lpAddr->Next) {
						addresses.push_back(utils::InetAddress(std::string(lpAddr->IpAddress.String), 0));
					}

					for (IP_ADDR_STRING* lpAddr = &lpTemp->GatewayList; lpAddr != NULL; lpAddr = lpAddr->Next) {
						//pGateway->push_back(utils::InetAddress(lpAddr->IpAddress.String));
					}

					break;
				}
			}

			free(pIPAdapterInfo);
			pIPAdapterInfo = NULL;

			pAdapterList = pAdapterList->Next;
		}
	}

	//add loopback address
	addresses.push_back(utils::InetAddress::Loopback());

	//__WinCheckDisableNetcard(nCards);
	if (NULL != adapter_address) {
		HeapFree(GetProcessHeap(), 0, adapter_address);
		adapter_address = NULL;
	}

#else
	struct ifaddrs *myaddrs;
	struct sockaddr_in *ss;
	is_success = getifaddrs(&myaddrs) == 0;
	if (is_success) {
		for (struct ifaddrs *ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr == NULL) continue;
			//if (strcmp(ifa->ifa_name, "lo") == 0)
			//	continue;
			ss = (struct sockaddr_in *)ifa->ifa_addr;
			if (ss->sin_family == AF_INET) {
				addresses.push_back(utils::InetAddress(ss->sin_addr));
			}
		}

		freeifaddrs(myaddrs);
	}
#endif

	return is_success;
}

utils::InetAddress::InetAddress() {
	memset(&addr_, 0, sizeof(addr_));
}

utils::InetAddress::InetAddress(const utils::InetAddress &address) {
	(*this) = address;
}

utils::InetAddress::InetAddress(uint16_t port) {
	memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_.sin_port = htons(port);
}

utils::InetAddress::InetAddress(const std::string &ip, uint16_t port) {
	memset(&addr_, 0, sizeof(addr_));
	Resolve(ip);
	addr_.sin_port = htons(port);
}

utils::InetAddress::InetAddress(uint32_t ip_big_endian, uint16_t port) {
	memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = ip_big_endian;
	addr_.sin_port = htons(port);
}

utils::InetAddress::InetAddress(const struct sockaddr_in &addr)
	: addr_(addr) {}

utils::InetAddress::InetAddress(const struct in_addr &in_addr) {
	memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_addr = in_addr;
}

utils::InetAddress::InetAddress(const std::string &ip) {
	memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	utils::StringVector ip_array = utils::String::Strtok(ip, ':');
	if (ip_array.size() > 0) {
		Resolve(ip_array[0]);
	}

	if (ip_array.size() > 1) {
		uint32_t port = utils::String::Stoui(ip_array[1]);
		addr_.sin_port = htons(port);
	}
}

utils::InetAddress::InetAddress(const asio::ip::tcp::endpoint &endpoint) {
	memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = inet_addr(endpoint.address().to_string().c_str());
	addr_.sin_port = htons(endpoint.port());
}

utils::InetAddress::InetAddress(const asio::ip::udp::endpoint &endpoint) {
	InetAddress(endpoint.address().to_string(), endpoint.port());
}

const struct sockaddr_in &utils::InetAddress::sock_addr_in() const {
	return addr_; 
}

const struct sockaddr *utils::InetAddress::sock_addr() const { 
	return (const struct sockaddr *)&addr_; 
}

socklen_t utils::InetAddress::size() const { 
	return sizeof(addr_);
}

void utils::InetAddress::set_sock_addr(const struct sockaddr_in &addr) { 
	addr_ = addr; 
}

uint32_t utils::InetAddress::IpBigEndian() const { 
	return addr_.sin_addr.s_addr; 
}

uint16_t utils::InetAddress::PortBigEndian() const { 
	return addr_.sin_port; 
}

int32_t utils::InetAddress::GetFamily() const { 
	return AF_INET; 
}

uint16_t utils::InetAddress::GetPort() const { 
	return htons(addr_.sin_port); 
}

void utils::InetAddress::operator=(struct sockaddr_in addr) {
	addr_ = addr;
}

void utils::InetAddress::operator=(const InetAddress &address) {
	if (this != &address) memcpy(&addr_, &address.addr_, sizeof(addr_));
}

bool utils::InetAddress::operator==(const InetAddress &address) {
	return ToIpPort() == address.ToIpPort();
}

std::string utils::InetAddress::ToIp() const {
	char *host = inet_ntoa(addr_.sin_addr);
	if (NULL != host)
		return host;
	else
		return "INVALID";
}

std::string utils::InetAddress::ToIpPort() const {
	char buf[32];
	snprintf(buf, sizeof(buf), "%s:%u", ToIp().c_str(), ntohs(addr_.sin_port));
	return buf;
}

std::string utils::GetPeerName(SOCKET s) {
	sockaddr_in addr = { 0 };
	socklen_t len = sizeof(addr);
	getpeername(s, (sockaddr*)&addr, &len);
	return utils::InetAddress(addr).ToIpPort();
}

asio::ip::tcp::endpoint utils::InetAddress::tcp_endpoint() const {
	return asio::ip::tcp::endpoint(asio::ip::address_v4::from_string(ToIp()), ntohs(addr_.sin_port));
}

asio::ip::udp::endpoint utils::InetAddress::udp_endpoint() const {
	return asio::ip::udp::endpoint(asio::ip::address_v4::from_string(ToIp()), ntohs(addr_.sin_port));
}

bool utils::InetAddress::IsAny() const {
	return addr_.sin_addr.s_addr == htonl(INADDR_ANY);
}

bool utils::InetAddress::IsLoopback() const {
	return addr_.sin_addr.s_addr == htonl(INADDR_LOOPBACK);
}

bool utils::InetAddress::IsNone() const {
	return addr_.sin_addr.s_addr == htonl(INADDR_NONE);
}

utils::InetAddress utils::InetAddress::Any() {
	return InetAddress(htonl(INADDR_ANY), (uint16_t)0);
}

utils::InetAddress utils::InetAddress::Loopback() {
	return InetAddress(htonl(INADDR_LOOPBACK), (uint16_t)0);
}

utils::InetAddress utils::InetAddress::None() {
	return InetAddress(htonl(INADDR_NONE), (uint16_t)0);
}

bool utils::InetAddress::Resolve(const std::string &address) {
	unsigned long address_int = inet_addr(address.c_str());
	if (address_int != htonl(INADDR_NONE)) {
		addr_.sin_family = AF_INET;
		addr_.sin_addr.s_addr = address_int;
		return true;
	}

	struct hostent *lpHost = gethostbyname(address.c_str());
	if (NULL == lpHost || NULL == lpHost->h_addr_list) {
		return false;
	}

	switch (lpHost->h_addrtype) {
	case AF_INET:
		addr_.sin_family = AF_INET;
		addr_.sin_addr.s_addr = *(unsigned long *)lpHost->h_addr_list[0];
		return true;

	default:
		utils::set_error_code(ERROR_NOT_SUPPORTED);
		return false;
	}

	utils::set_error_code(ERROR_NOT_SUPPORTED);
	return false;
}

utils::net::net() {};
utils::net::~net() {};

const SOCKET utils::Socket::INVALID_HANDLE = (SOCKET)-1;
const int utils::Socket::ERR_VALUE = -1;

utils::Socket::Socket() {
	handle_ = INVALID_HANDLE;
	blocked_ = true;
}

utils::Socket::~Socket() {
	if (IsValid()) {
		Close();
	}
}

utils::InetAddress utils::Socket::local_address() const {
	return local_address_; 
}

SOCKET utils::Socket::handle() const {
	return handle_; 
}

bool utils::Socket::IsValid() const { 
	return handle_ != INVALID_HANDLE; 
}

bool utils::Socket::Create(SocketType type, const InetAddress &address) {
	assert(!IsValid());

	int32_t family = address.GetFamily();
	if (type == SOCKET_TYPE_TCP) {
		handle_ = socket(family, SOCK_STREAM, IPPROTO_TCP);
	}
	else {
		handle_ = socket(family, SOCK_DGRAM, IPPROTO_UDP);
	}

	if (!IsValid()) {
		return false;
	}

	do {
		if (bind(handle_, address.sock_addr(), address.size()) == ERR_VALUE) {
			break;
		}

		//get the address after binding
		socklen_t len = address.size();
		if (getsockname(handle_, (struct sockaddr *)local_address_.sock_addr(), &len) == ERR_VALUE) {
			break;
		}

		return true;
	} while (false);

	Close();
	return false;
}

bool utils::Socket::Close() {
#ifdef WIN32
	closesocket(handle_);
	handle_ = Socket::INVALID_HANDLE;
#else
	close(handle_);
	handle_ = Socket::INVALID_HANDLE;
#endif
	return true;
}

bool utils::Socket::IsNomralError(uint32_t error_code) {
#ifdef WIN32
	return (WSAETIMEDOUT == error_code || WSAEWOULDBLOCK == error_code || WSAEINPROGRESS == error_code || WSAEINTR == error_code);
#else
	return (EAGAIN == error_code || EWOULDBLOCK == error_code || EINPROGRESS == error_code || EINTR == error_code);
#endif
}

bool utils::Socket::Connect(const InetAddress &server) {
	if (connect(handle_, server.sock_addr(), server.size()) == 0) {
		peer_address_ = server;
		return true;
	}

#ifdef WIN32
	if (utils::error_code() != WSAEWOULDBLOCK)
#else
	if (utils::error_code() != EINPROGRESS)
#endif
	{
		// failed 
		return false;
	}

	peer_address_ = server;
	return false;
}

bool utils::Socket::Connect(const InetAddress &server, int timeout_milli) {

	//must set async connect after save the block method
	bool old_blocking = blocked_;
	do {
		if (!SetBlock(false)) {
			break;
		}

		connect(handle_, (const struct sockaddr *)server.sock_addr(), server.size());
#ifdef WIN32

		//select model
		struct timeval timeout;
		fd_set r;

		FD_ZERO(&r);
		FD_SET(handle_, &r);
		timeout.tv_sec = timeout_milli / utils::MILLI_UNITS_PER_SEC; //second of timeout
		timeout.tv_usec = (timeout_milli % utils::MILLI_UNITS_PER_SEC) * utils::MICRO_UNITS_PER_MILLI;
		int ret = select(0, 0, &r, 0, &timeout);
		if (ret <= 0) {
			break;
		}

		//set back to the original block method
		SetBlock(old_blocking);
#endif
		return true;

	} while (false);

	SetBlock(old_blocking);
	return  false;
}

bool utils::Socket::SetBlock(bool block) {
	unsigned long enabled = block ? 0 : 1;
#ifdef WIN32
	if (ioctlsocket(handle_, FIONBIO, &enabled) == 0) {
		blocked_ = block;
		return true;
	}
#else
	if (ioctl(handle_, FIONBIO, &enabled) == 0) {
		blocked_ = block;
		return true;
	}

#endif
	return false;
}

void utils::Socket::SetTcpNoDelay(bool on) {
	char optval = on ? 1 : 0;
	setsockopt(handle_, SOL_SOCKET, TCP_NODELAY,
		&optval, static_cast<socklen_t>(sizeof optval));
}

void utils::Socket::SetKeepAlive(bool on) {
	char optval = on ? 1 : 0;
	setsockopt(handle_, SOL_SOCKET, SO_KEEPALIVE,
		&optval, static_cast<socklen_t>(sizeof optval));
}

bool utils::Socket::Accept(Socket &new_socket) {
	SOCKET new_handle = accept(handle_, NULL, 0);
	if (Socket::INVALID_HANDLE == new_handle) {
		return false;
	}

	utils::InetAddress peer_address;
	utils::InetAddress local_address;

	socklen_t len = local_address.size();
	if (getsockname(new_handle, (struct sockaddr *)local_address.sock_addr(), &len) != Socket::ERR_VALUE) {
		new_socket.local_address_ = local_address;
	}

	len = peer_address.size();
	if (getpeername(new_handle, (struct sockaddr *)peer_address.sock_addr(), &len) != Socket::ERR_VALUE) {
		new_socket.peer_address_ = peer_address;
	}

	return true;
}

//send
int utils::Socket::Send(const void *buffer, int size) {
	return send(handle_, (const char *)buffer, size, 0);
}

//send data tils complete
bool utils::Socket::SendComplete(const void *buffer, int size) {
	int left_size = size;
	while (left_size > 0) {
		int sent_size = Send(((const char *)buffer) + (size - left_size), left_size);
		if (sent_size > 0) {
			left_size -= sent_size;
			continue;
		}

		if (IsNomralError(utils::error_code())) {
			utils::Sleep(10);
			continue;
		}
		else {
			return false;
		}
	}

	return true;
}

//receive stream data
int utils::Socket::Receive(void *buffer, int size) {
	return recv(handle_, (char *)buffer, size, 0);
}

int utils::Socket::SendTo(const void *pBuffer, int nSize, const utils::InetAddress &address) {
	return sendto(handle_, (const char *)pBuffer, nSize, 0, address.sock_addr(), address.size());
}

int utils::Socket::ReceiveFrom(void *pBuffer, int nSize, const utils::InetAddress &address) {
	socklen_t len = address.size();
	return recvfrom(handle_, (char *)pBuffer, nSize, 0, (struct sockaddr *)address.sock_addr(), &len);
}

utils::AsyncIoThread::AsyncIoThread() {};

utils::AsyncIoThread:: ~AsyncIoThread() {};

void utils::AsyncIoThread::Run() {
	io_service.run();
}

void utils::AsyncIoThread::Stop() {
	io_service.stop();
}

utils::AsyncIo::AsyncIo() {
	next_id_ = 0;
	threads_ptr_ = new AsyncIoThreadArray();
	io_service_ptr_ = NULL;
}

utils::AsyncIo::~AsyncIo() {
	delete threads_ptr_;
	threads_ptr_ = NULL;
}

bool utils::AsyncIo::Create(size_t poll_size, size_t thread_count) {

	if (threads_ptr_->size() > 0) {
		return false;
	}

	if (thread_count == 0) {
		thread_count = 2; // should the core number of cpu
	}

	size_t success_count = 0;
	for (size_t i = 0; i < thread_count; i++) {
		AsyncIoThread *thread_ptr = new AsyncIoThread();
		threads_ptr_->push_back(thread_ptr);

		work_ptr dummy_work(new asio::io_service::work(thread_ptr->io_service));
		work_.push_back(dummy_work);
		if (!thread_ptr->Start()) {
			break;
		}

		success_count++;
	}

	if (success_count != thread_count) {
		Close();
		return false;
	}

	return true;
}

bool utils::AsyncIo::AttachServiceIo(asio::io_service *io) {
	io_service_ptr_ = io;
	return true;
}

bool utils::AsyncIo::Close() {
	for (size_t i = 0; i < threads_ptr_->size(); i++) {
		AsyncIoThread *async_thread_ptr = threads_ptr_->at(i);
		if (async_thread_ptr == NULL) {
			continue;
		}

		if (async_thread_ptr->thread_id() == utils::Thread::current_thread_id()) {
			//__ULOG_ERROR(__ULOG_FMT("Utils::AsyncIo", "Can't close in event thread("_SIZE_TFMT_")"), n);

			utils::set_error_code(ERROR_ACCESS_DENIED);
			return false;
		}
	}

	for (size_t i = 0; i < threads_ptr_->size(); i++) {
		AsyncIoThread *async_thread_ptr = threads_ptr_->at(i);
		if (async_thread_ptr == NULL) continue;

		if (async_thread_ptr->IsRunning()) {
			//__ULOG_TRACE(__ULOG_FMT("Utils::AsyncIo", "Waiting for data thread("_SIZE_TFMT_") exit..."), n);
			async_thread_ptr->Stop();
			async_thread_ptr->JoinWithStop();

			//__ULOG_TRACE(__ULOG_FMT("Utils::AsyncIo", "Data thread("_SIZE_TFMT_") exited"), n);
		}

		//delete async_thread_ptr;
	}

	threads_ptr_->clear();
	return true;
}

asio::io_service *utils::AsyncIo::GetIoService() {
	if (io_service_ptr_) {
		return io_service_ptr_;
	}
	else {
		size_t id = (next_id_++) % threads_ptr_->size();
		return &threads_ptr_->at(id)->io_service;
	}
}

utils::AsyncSocket::AsyncSocket(AsyncIo *asyncio_ptr) : asyncio_ptr_(asyncio_ptr) {}

utils::AsyncSocket::~AsyncSocket() {}

utils::InetAddress utils::AsyncSocket::local_address() {
	return local_address_; 
}

utils::InetAddress utils::AsyncSocket::peer_address() {
	return peer_address_; 
}

utils::AsyncSocketTcp::AsyncSocketTcp(AsyncIo *asyncio_ptr) : AsyncSocket(asyncio_ptr) {
	tcpsocket_ptr_ = new asio::ip::tcp::socket(*asyncio_ptr_->GetIoService());
}

utils::AsyncSocketTcp::~AsyncSocketTcp() {
	//if (IsValid()) Close();
	//delete tcpsocket_ptr_;
	if (tcpsocket_ptr_) {
		delete tcpsocket_ptr_;
		tcpsocket_ptr_ = NULL;
	}

	//LOG_ERROR("DELETE pointer " FMT_U64 " peer %s ", (int64_t)this, peer_address_.ToIpPort().c_str());
};

size_t utils::AsyncSocketTcp::SendSome(const void *buffer, size_t size) {
	asio::error_code err;
	return tcpsocket_ptr_->write_some(asio::buffer(buffer, size), err);
}

int utils::AsyncSocketTcp::AsyncSendSome(const void *buffer, size_t size) {
	tcpsocket_ptr_->async_write_some(asio::buffer(buffer, size),
		[this](asio::error_code ec, std::size_t bytes_transferred) {
		if (!ec) {
			OnSend(bytes_transferred);
		}
		else {
			utils::set_error_code(ec.value());
			OnError();
		}
	});

	return 0;
}

bool utils::AsyncSocketTcp::Bind(const utils::InetAddress &address) {
	do {
		asio::error_code ec;
		tcpsocket_ptr_->open(address.GetFamily() == AF_INET ? asio::ip::tcp::v4() : asio::ip::tcp::v6(), ec);
		if (ec.value() != 0) {
			break;
		}

		SetReuse(true);
		tcpsocket_ptr_->bind(address.tcp_endpoint(), ec);
		if (ec.value() != 0) {
			break;
		}

		local_address_ = utils::InetAddress(tcpsocket_ptr_->local_endpoint());
		utils::set_error_code((uint32_t)ec.value());
		return true;
	} while (false);
	return false;
}

bool utils::AsyncSocketTcp::Close() {
	asio::error_code ignored_ec;
	tcpsocket_ptr_->shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
	tcpsocket_ptr_->close();

	return true;
}

bool utils::AsyncSocketTcp::IsValid() {
	return tcpsocket_ptr_ && tcpsocket_ptr_->is_open();
}

bool utils::AsyncSocketTcp::SetTcpNoDelay(bool on) {
	asio::ip::tcp::no_delay no_delay(on);
	asio::error_code ec;
	tcpsocket_ptr_->set_option(no_delay, ec);
	return !ec;
}

bool utils::AsyncSocketTcp::SetKeepAlive(bool on) {
	asio::socket_base::keep_alive option(true);
	asio::error_code ec;
	tcpsocket_ptr_->set_option(option, ec);
	return !ec;
}

bool utils::AsyncSocketTcp::SetReuse(bool on) {
	//tcpsocket_ptr_->set_option(asio::ip::tcp::reuse_address(on));
	return true;
}

bool utils::AsyncSocketTcp::Connect(const utils::InetAddress &server) {
	asio::error_code ec;
	tcpsocket_ptr_->connect(server.tcp_endpoint(), ec);
	return ec.value() == 0;
}

bool utils::AsyncSocketTcp::AsyncConnect(const utils::InetAddress &server) {
	peer_address_ = server;

	tcpsocket_ptr_->async_connect(server.tcp_endpoint(),
		[this](asio::error_code ec) {
		if (!ec) {
			peer_address_ = utils::InetAddress(tcpsocket_ptr_->remote_endpoint());
			OnConnect();
		}
		else {
			LOG_ERROR("On connect err pointer " FMT_U64 " %s", (uint64_t)this, peer_address_.ToIpPort().c_str());
			utils::set_error_code(ec.value());
			OnError();
		}
	});
	return true;
}

size_t utils::AsyncSocketTcp::ReceiveSome(void *buffer, size_t size) {
	return tcpsocket_ptr_->read_some(asio::buffer(buffer, size));
}

int utils::AsyncSocketTcp::AsyncReceiveSome(size_t max_size) {
	size_t size = MIN(max_size, utils::ETH_MAX_PACKET_SIZE);
	tcpsocket_ptr_->async_read_some(asio::buffer(buffer_, size),
		[this](asio::error_code ec, std::size_t bytes_transferred) {
		if (!ec) {
			OnReceive(buffer_, bytes_transferred);
		}
		else {
			utils::set_error_code(ec.value());
			OnError();
		}
	});

	return 0;
}

void utils::AsyncSocketTcp::OnConnect() {}

void utils::AsyncSocketTcp::OnError() {

}

void utils::AsyncSocketTcp::OnSend(std::size_t bytes_transferred) {
	//printf("socket send ok len " FMT_SIZE, bytes_transferred);
}

void utils::AsyncSocketTcp::OnReceive(void *buffer, size_t bytes_transferred) {
	//printf("socket recv ok len " FMT_SIZE " --- %s", bytes_transferred, buffer_);
}

utils::AsyncSocketAcceptor::AsyncSocketAcceptor(AsyncIo *asyncio_ptr, IAsyncSocketAcceptorNotify *notify_ptr) :AsyncSocket(asyncio_ptr),
notify_lptr_(notify_ptr) {
	acceptor_ptr_ = new asio::ip::tcp::acceptor(*asyncio_ptr_->GetIoService());
	tcpsocket_lptr_ = NULL;
}

utils::AsyncSocketAcceptor::~AsyncSocketAcceptor() {
	if (IsValid()) Close();
	delete acceptor_ptr_;
}

bool utils::AsyncSocketAcceptor::Bind(const utils::InetAddress &address) {
	do {
		asio::error_code ec;
		acceptor_ptr_->open(address.GetFamily() == AF_INET ? asio::ip::tcp::v4() : asio::ip::tcp::v6(), ec);
		if (ec.value() != 0) {
			break;
		}

		SetReuse(true);
		acceptor_ptr_->bind(address.tcp_endpoint(), ec);
		if (ec.value() != 0) {
			break;
		}

		local_address_ = utils::InetAddress(acceptor_ptr_->local_endpoint());

		utils::set_error_code((uint32_t)ec.value());
		return true;
	} while (false);
	return false;
}

bool utils::AsyncSocketAcceptor::Close() {
	acceptor_ptr_->close();
	return true;
}

bool utils::AsyncSocketAcceptor::IsValid() {
	return acceptor_ptr_->is_open();
}

bool utils::AsyncSocketAcceptor::SetKeepAlive(bool on) {
	return true;
}

bool utils::AsyncSocketAcceptor::SetReuse(bool on) {
	acceptor_ptr_->set_option(asio::ip::tcp::acceptor::reuse_address(on));
	return true;
}

bool utils::AsyncSocketAcceptor::Listen(int back_log) {
	asio::error_code ec;
	acceptor_ptr_->listen(back_log, ec);
	return !ec;
}

bool utils::AsyncSocketAcceptor::Accept(AsyncSocketTcp *new_socket) {
	asio::ip::tcp::endpoint endpoint;
	asio::error_code ec;
	acceptor_ptr_->accept(*new_socket->tcpsocket_ptr_, ec);
	if (!ec) {
		new_socket->peer_address_ = utils::InetAddress(new_socket->tcpsocket_ptr_->remote_endpoint());
		new_socket->local_address_ = utils::InetAddress(new_socket->tcpsocket_ptr_->local_endpoint());
	}
	return !ec;
}

bool utils::AsyncSocketAcceptor::AsyncAccept(AsyncSocketTcp *new_socket) {
	tcpsocket_lptr_ = new_socket;
	tcpsocket_lptr_->local_address_ = local_address_;
	acceptor_ptr_->async_accept(*new_socket->tcpsocket_ptr_,
		[this](asio::error_code ec) {
		if (!ec) {
			tcpsocket_lptr_->peer_address_ = utils::InetAddress(tcpsocket_lptr_->tcpsocket_ptr_->remote_endpoint());
			OnAccept();
		}
		else {
			OnError();
		}
	});

	return true;
}

bool utils::AsyncSocketAcceptor::AsyncAccept(AsyncSocketSsl *new_socket) {
	sslsocket_lptr_ = new_socket;
	sslsocket_lptr_->local_address_ = local_address_;
	acceptor_ptr_->async_accept(sslsocket_lptr_->sslsocket_ptr_->lowest_layer(),
		[this](asio::error_code ec) {
		if (!ec) {
			sslsocket_lptr_->peer_address_ = utils::InetAddress(sslsocket_lptr_->sslsocket_ptr_->lowest_layer().remote_endpoint());
			OnAccept();
		}
		else {
			OnError();
		}
	});

	return true;
}

void utils::AsyncSocketAcceptor::OnAccept() {
	if (notify_lptr_) {
		notify_lptr_->OnAccept(this);
	}
}

void utils::AsyncSocketAcceptor::OnError() {
	if (notify_lptr_) {
		notify_lptr_->OnError(this);
	}
}

utils::AsyncSocketUdp::AsyncSocketUdp(AsyncIo *asyncio_ptr) :AsyncSocket(asyncio_ptr) {
	udpsocket_ptr_ = new asio::ip::udp::socket(*asyncio_ptr_->GetIoService());
}

utils::AsyncSocketUdp::~AsyncSocketUdp() {
	if (IsValid()) Close();
	delete udpsocket_ptr_;
}

bool utils::AsyncSocketUdp::Bind(const utils::InetAddress &address) {
	do {
		asio::error_code ec;
		udpsocket_ptr_->open(address.GetFamily() == AF_INET ? asio::ip::udp::v4() : asio::ip::udp::v6(), ec);
		if (ec.value() != 0) {
			break;
		}
		udpsocket_ptr_->bind(address.udp_endpoint(), ec);
		if (ec.value() != 0) {
			break;
		}

		local_address_ = utils::InetAddress(udpsocket_ptr_->local_endpoint());
		utils::set_error_code((uint32_t)ec.value());
		return true;
	} while (false);
	return false;
}

bool utils::AsyncSocketUdp::Close() {
	udpsocket_ptr_->close();
	return true;
}

bool utils::AsyncSocketUdp::IsValid() {
	return udpsocket_ptr_->is_open();
}

bool utils::AsyncSocketUdp::SetKeepAlive(bool on) {
	return true;
}

bool utils::AsyncSocketUdp::SetReuse(bool on) {
	//udpsocket_ptr_->set_option(asio::ip::tcp::acceptor::reuse_address(on));
	return true;
}

size_t utils::AsyncSocketUdp::SendTo(const void *buffer, size_t size, const utils::InetAddress &address) {
	return udpsocket_ptr_->send_to(asio::buffer(buffer, size), address.udp_endpoint());
}

int utils::AsyncSocketUdp::AsyncSendTo(const void *buffer, size_t size, const utils::InetAddress &address) {
	udpsocket_ptr_->async_send_to(asio::buffer(buffer, size), address.udp_endpoint(),
		[this](asio::error_code ec, std::size_t bytes_transferred) {
		if (!ec) {
			OnSend(bytes_transferred);
		}
	});
	return 0;
}

size_t utils::AsyncSocketUdp::ReceiveFrom(void *buffer, size_t size, utils::InetAddress &address) {
	asio::ip::udp::endpoint endpoint_udp;
	size_t bytes_len = udpsocket_ptr_->receive_from(asio::buffer(buffer, size), endpoint_udp);
	address = utils::InetAddress(endpoint_udp);
	return bytes_len;
}

int utils::AsyncSocketUdp::AsyncReceiveFrom(size_t max_size) {
	size_t size = MIN(max_size, utils::ETH_MAX_PACKET_SIZE);
	udpsocket_ptr_->async_receive_from(asio::buffer(buffer_, size), sender_endpoint_,
		[this](asio::error_code ec, std::size_t bytes_transferred) {
		if (!ec) {
			OnReceive(buffer_, bytes_transferred, utils::InetAddress(sender_endpoint_));
		}
		else {
			OnError();
		}
	});

	return 0;
}

void utils::AsyncSocketUdp::OnSend(std::size_t bytes_transferred) {

}

void utils::AsyncSocketUdp::OnReceive(void *buffer, size_t bytes_transferred, const utils::InetAddress &address) {

}

void utils::AsyncSocketUdp::OnError() {

}

utils::NameResolver::NameResolver(utils::AsyncIo *async_io_ptr) : async_io_lptr_(async_io_ptr) {
	resolver_ptr_ = new asio::ip::tcp::resolver(*async_io_lptr_->GetIoService());
}
utils::NameResolver::~NameResolver() {
	resolver_ptr_->cancel();
	delete resolver_ptr_;
}

bool utils::NameResolver::Query(const std::string &name, utils::InetAddressList &list) {
	try {
		asio::ip::tcp::resolver::query::flags resolveflags = asio::ip::tcp::resolver::query::flags::numeric_host;
		asio::ip::tcp::resolver::query query(name, "http");
		asio::ip::tcp::resolver::iterator i = resolver_ptr_->resolve(query);
		while (i != asio::ip::tcp::resolver::iterator()) {
			asio::ip::tcp::endpoint end = *i;
			if (end.address().is_v4()) {
				list.push_back(utils::InetAddress(end.address().to_v4().to_string(), 0));
				break;
			}
			i++;
		}
	}
	catch (std::exception e) {
		return false;
	}
	return true;
}

utils::AsyncSocketSsl::AsyncSocketSsl(AsyncIo *asyncio_ptr, asio::ssl::context& context) : AsyncSocket(asyncio_ptr) {
	sslsocket_ptr_ = new SslSocket(*asyncio_ptr_->GetIoService(), context);
	sslsocket_ptr_->set_verify_mode(asio::ssl::verify_none);
	sslsocket_ptr_->set_verify_callback(std::bind(&AsyncSocketSsl::OnVerifyCertificate, this, _1, _2));
};

utils::AsyncSocketSsl::~AsyncSocketSsl() {
	if (IsValid()) Close();
	delete sslsocket_ptr_;
};

size_t utils::AsyncSocketSsl::SendSome(const void *buffer, size_t size) {
	return sslsocket_ptr_->write_some(asio::buffer(buffer, size));
}

int utils::AsyncSocketSsl::AsyncSendSome(const void *buffer, size_t size) {
	sslsocket_ptr_->async_write_some(asio::buffer(buffer, size),
		[this](asio::error_code ec, std::size_t bytes_transferred) {
		if (!ec) {
			OnSend(bytes_transferred);
		}
		else {
			utils::set_error_code(ec.value());
			OnError();
		}
	});

	return 0;
}

bool utils::AsyncSocketSsl::Bind(const utils::InetAddress &address) {
	do {
		asio::error_code ec;
		sslsocket_ptr_->lowest_layer().open(address.GetFamily() == AF_INET ? asio::ip::tcp::v4() : asio::ip::tcp::v6(), ec);
		if (ec.value() != 0) {
			break;
		}
		sslsocket_ptr_->lowest_layer().bind(address.tcp_endpoint(), ec);
		if (ec.value() != 0) {
			break;
		}

		local_address_ = utils::InetAddress(sslsocket_ptr_->lowest_layer().local_endpoint());
		utils::set_error_code((uint32_t)ec.value());
		return true;
	} while (false);
	return false;
}

bool utils::AsyncSocketSsl::Close() {
	sslsocket_ptr_->lowest_layer().close();
	return true;
}

bool utils::AsyncSocketSsl::IsValid() {
	return sslsocket_ptr_->lowest_layer().is_open();
}

bool utils::AsyncSocketSsl::SetTcpNoDelay(bool on) {
	asio::ip::tcp::no_delay no_delay(on);
	asio::error_code ec;
	sslsocket_ptr_->lowest_layer().set_option(no_delay, ec);
	return !ec;
}

bool utils::AsyncSocketSsl::SetKeepAlive(bool on) {
	asio::socket_base::keep_alive option(true);
	asio::error_code ec;
	sslsocket_ptr_->lowest_layer().set_option(option, ec);
	return !ec;
}

bool utils::AsyncSocketSsl::SetReuse(bool on) {
	return true;
}

bool utils::AsyncSocketSsl::Connect(const utils::InetAddress &server) {
	asio::error_code ec;
	sslsocket_ptr_->lowest_layer().connect(server.tcp_endpoint(), ec);
	return ec.value() == 0;
}

bool utils::AsyncSocketSsl::HandShake() {
	asio::error_code ec;
	sslsocket_ptr_->handshake(asio::ssl::stream_base::client);
	return ec.value() == 0;
}

bool utils::AsyncSocketSsl::AsyncConnect(const utils::InetAddress &server) {
	peer_address_ = server;
	sslsocket_ptr_->lowest_layer().async_connect(server.tcp_endpoint(),
		[this](asio::error_code ec) {
		if (!ec) {
			peer_address_ = utils::InetAddress(sslsocket_ptr_->lowest_layer().remote_endpoint());
			OnConnect();
		}
		else {
			utils::set_error_code(ec.value());
			OnError();
		}
	});
	return true;
}

size_t utils::AsyncSocketSsl::ReceiveSome(void *buffer, size_t size) {
	return sslsocket_ptr_->read_some(asio::buffer(buffer, size));
}

int utils::AsyncSocketSsl::AsyncReceiveSome(size_t max_size) {
	size_t size = MIN(max_size, utils::ETH_MAX_PACKET_SIZE);
	sslsocket_ptr_->async_read_some(asio::buffer(buffer_, size),
		[this](asio::error_code ec, std::size_t bytes_transferred) {
		if (!ec) {
			OnReceive(buffer_, bytes_transferred);
		}
		else {
			utils::set_error_code(ec.value());
			OnError();
		}
	});

	return 0;
}

bool utils::AsyncSocketSsl::AsyncHandShake() {
	sslsocket_ptr_->async_handshake(asio::ssl::stream_base::server,
		[this](asio::error_code ec) {
		if (!ec) {
			OnHandShake();
		}
		else {
			utils::set_error_code(ec.value());
			OnError();
		}
	});
	return true;
}

void utils::AsyncSocketSsl::OnConnect() {
	sslsocket_ptr_->async_handshake(asio::ssl::stream_base::client,
		[this](asio::error_code ec) {
		if (!ec) {
			OnHandShake();
		}
		else {
			utils::set_error_code(ec.value());
			OnError();
		}
	});
}

void utils::AsyncSocketSsl::OnHandShake() {}

void utils::AsyncSocketSsl::OnError() {

}

void utils::AsyncSocketSsl::OnSend(std::size_t bytes_transferred) {}

void utils::AsyncSocketSsl::OnReceive(void *buffer, size_t bytes_transferred) {}

bool utils::AsyncSocketSsl::OnVerifyCertificate(bool preverified, asio::ssl::verify_context& ctx) {
	// The verify callback can be used to check whether the certificate that is  
	// being presented is valid for the peer. For example, RFC 2818 describes  
	// the steps involved in doing this for HTTPS. Consult the OpenSSL  
	// documentation for more details. Note that the callback is called once  
	// for each certificate in the certificate chain, starting from the root  
	// certificate authority.  

	// In this example we will simply print the certificate's subject name.  
	char subject_name[256];
	X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
	X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
	printf("Verifying %s", subject_name);

	return preverified;
}

