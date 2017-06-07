//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include "header.hpp"
#include <utils/net.h>

namespace http {
namespace server {

	extern std::string ToLower(const std::string &str);
	extern void Strtok(const std::string &str, char separator, std::vector<std::string> &arr);
	extern bool url_decode(const std::string& in, std::string& out);

/// A request received from a client.
struct request
{
	utils::InetAddress peer_address_;
	utils::InetAddress local_address_;
  std::string method; //GET or POST
  std::string uri;  // like /hello?a=1&b=2
  std::string command;  // like  hello
  int http_version_major;
  int http_version_minor;
  std::vector<header> headers;
  std::map<std::string, std::string> parameter;
  int body_length;
  std::string body;

  bool Update(){
	  for (size_t i = 0; i < headers.size(); i++){
		  headers[i].name = ToLower(headers[i].name);
	  }

	  std::string request_path;
	  if (!url_decode(uri, request_path))
	  {
		  return false;
	  }
	  if (request_path.size() && request_path[0] == '/')
		  request_path = request_path.substr(1);

	  std::string params;
	  auto pos = request_path.find('?');
	  if (pos == std::string::npos)
		  command = request_path;
	  else
	  {
		  command = request_path.substr(0, pos);
		  params = request_path.substr(pos + 1);
	  }

	  //parse params
	  std::vector<std::string> params1;
	  Strtok(params, '&', params1);
	  for (size_t i = 0; i < params1.size(); i++){
		  std::vector<std::string> params2;
		  Strtok(params1[i], '=', params2);
		  if (params2.size() > 1){
			  parameter[params2[0]] = params2[1];
		  }
	  }

	  return true;
  }

  std::string GetHeaderValue(const std::string &key) const {
	  for (size_t i = 0; i < headers.size(); i++){
		  if (headers[i].name == key){
			  return headers[i].value;
		  }
	  }

	  return "";
  }

  std::string GetParamValue(const std::string &key) const {
	  std::map<std::string, std::string>::const_iterator iter = parameter.find(key);
	  if (iter != parameter.end()){
		  return iter->second;
	  }

	  return "";
  }
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HPP
