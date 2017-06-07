#pragma once

//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// ASIO is somewhat particular about when it gets included -- it wants to be the
// first to include <windows.h> -- so we try to include it before everything
// else.
#include <asio.hpp>

#include <string>
#include <map>
#include <functional>
#include "io_service_pool.hpp"
#include "connection.hpp"
#include "connection_manager.hpp"

namespace http {
namespace server {

	class FileAttribute
	{
	public:
		bool   m_bDirectory;
		time_t m_nCreateTime;
		time_t m_nModifyTime;
		time_t m_nAccessTime;
		int64_t  m_nSize;

		FileAttribute(){
			m_bDirectory = false;
			m_nCreateTime = 0;
			m_nModifyTime = 0;
			m_nAccessTime = 0;
			m_nSize = 0;
		};
	};

/// The top-level class of the HTTP server.
class server
{
    
public:
    typedef std::function<void(const request&, std::string&)> routeHandler;
    server(const server&) = delete;
    server& operator=(const server&) = delete;

    // construct a server that just doesn't listen

    /// Construct the server to listen on the specified TCP address and port
    explicit server(const std::string& address, unsigned short port, asio::ssl::context *context, size_t pool_size);
    ~server();

	void addRoute(const std::string& routeName, routeHandler callback);
	routeHandler *getRoute(const std::string& routeName);
    void add404(routeHandler callback);

    void handle_request(const request& req, reply& rep);

    static void parseParams(const std::string& params, std::map<std::string, std::string>& retMap);

	bool GetAttribue(const std::string &strFile0, FileAttribute &nAttr);

	void SetHome(const std::string &home);
	void SetIndexName(const std::string &index_name);

	void Run();
	void Stop();

	int64_t star_count_;
	int64_t end_count_;
	int64_t expire_count_;
private:
    /// Perform an asynchronous accept operation.
    void do_accept();

    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
   // static bool url_decode(const std::string& in, std::string& out);
	io_service_pool io_server_pool_;
    /// The io_service used to perform asynchronous operations.
    //asio::io_service& io_service_;

    /// The signal_set is used to register for process termination notifications.
    asio::signal_set signals_;

    /// Acceptor used to listen for incoming connections.
    asio::ip::tcp::acceptor acceptor_;

    /// The connection manager which owns all live connections.
    connection_manager connection_manager_;

    /// The next socket to be accepted.
	asio::ip::tcp::socket *socket_ptr_;
	SslSocket *sslsocket_ptr_;
	asio::ssl::context *context_;

    std::map<std::string, routeHandler> mRoutes;

	std::string web_home_;
	std::string index_file_;

	std::map<std::string, std::string> compress_type_;
	std::map<std::string, std::string> content_type_;

	std::mutex se_mutex_;
};

} // namespace server
} // namespace http
