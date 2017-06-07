//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include <signal.h>
#include <utility>
#include <sstream>
#include <utils/logger.h>
#include <utils/timestamp.h>

#ifndef WIN32
#include <pwd.h>
#include <fnmatch.h>
#else
#include <shlobj.h>
#endif

namespace http
{
namespace server
{


server::server(const std::string& address,
	unsigned short port, asio::ssl::context *context, size_t pool_size)
    : io_server_pool_(pool_size)
	, signals_(io_server_pool_.get_first_service())
	, acceptor_(io_server_pool_.get_first_service())
    , connection_manager_()
	, socket_ptr_(NULL)
	, sslsocket_ptr_(NULL)
	, context_(context)
{
    asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(address),
                                     port);
    // Open the acceptor with the option to reuse the address (i.e.
    // SO_REUSEADDR).
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    do_accept();

	compress_type_["gzip__"] = "gzip";
	compress_type_["gziped"] = "gzip";
	compress_type_["deflate__"] = "deflate";
	compress_type_["deflated"] = "deflate";

	content_type_["html"] = "text/html";
	content_type_["shtml"] = "text/html";
	content_type_["htm"] = "text/html";
	content_type_["css"] = "text/css";
	content_type_["xml"] = "text/xml";
	content_type_["zip"] = "application/zip";
	content_type_["gz"] = "application/x-gzip";
	content_type_["js"] = "application/x-javascript";
	content_type_["exe"] = "application/octet-stream";
	content_type_["swf"] = "application/x-shockwave-flash";

	index_file_ = "index.html";
	star_count_ = 0;
	end_count_ = 0;
	expire_count_ = 0;
}

void server::add404(routeHandler callback)
{
    addRoute("404", callback);
}

void
server::addRoute(const std::string& routeName, routeHandler callback)
{
    mRoutes[routeName] = callback;
}

server::routeHandler *server::getRoute(const std::string& routeName){
	std::map<std::string, routeHandler>::iterator iter = mRoutes.find(routeName);
	if (iter != mRoutes.end()){
		return &iter->second;
	} else{
		return NULL;
	}
}

void
server::do_accept()
{
	if (context_ == NULL){
		socket_ptr_ = new asio::ip::tcp::socket(io_server_pool_.get_other_service());
		acceptor_.async_accept(*socket_ptr_, [this](asio::error_code ec)
		{
			// Check whether the server was stopped by a signal before this
			// completion handler had a chance to run.
			if (!acceptor_.is_open())
			{
				delete socket_ptr_;
				return;
			}

			if (!ec)
			{
				connection_manager_.start(std::make_shared<connection>(
					socket_ptr_, connection_manager_, *this));
			}else
				delete socket_ptr_;

			do_accept();
		});
	}else{
		sslsocket_ptr_ = new SslSocket(io_server_pool_.get_other_service(), *context_);
		acceptor_.async_accept(sslsocket_ptr_->lowest_layer(), [this](asio::error_code ec)
		{
			// Check whether the server was stopped by a signal before this
			// completion handler had a chance to run.
			if (!acceptor_.is_open())
			{
				return;
			}

			if (!ec)
			{
				connection_manager_.start(std::make_shared<connection>(
					sslsocket_ptr_, connection_manager_, *this));

				do_accept();
			}
		});
	}
}

server::~server()
{
	if (sslsocket_ptr_){
		delete sslsocket_ptr_;
	}

	if (socket_ptr_){
		delete socket_ptr_;
	}

    acceptor_.close();
    connection_manager_.stop_all();
}

void server::Run(){
	io_server_pool_.run();
}

void server::Stop(){
	io_server_pool_.stop();
}

std::string Replace(std::string &str, const std::string &from, const std::string &to)
{
	std::string::size_type pos = 0;
	while ((pos = str.find(from, pos)) != -1) {
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
	return str;
}


#ifdef WIN32
time_t __WinFileTime2UnixTime(const FILETIME &nTime)
{
	LONGLONG nTimeTick = (((LONGLONG)nTime.dwHighDateTime) << 32) + (LONGLONG)(nTime.dwLowDateTime);
	return (time_t)((nTimeTick - 116444736000000000L) / 10000000);
}
#endif

std::string RegularPath(const std::string &path){
	std::string path1 = path;
#ifdef WIN32
	if (path1.size() > 1 && path1.at(0) == '/')
	{
		path1.erase(0, 1);
		path1.insert(1, ":");
	}

	Replace(path1, "/", "\\");
#else
	Replace(path1, "\\", "//");
#endif

	return path1;
}

bool server::GetAttribue(const std::string &strFile0, FileAttribute &nAttr)
{
	std::string file1 = RegularPath(strFile0);


#ifdef WIN32
	WIN32_FILE_ATTRIBUTE_DATA nFileAttr;
	if (GetFileAttributesExA(file1.c_str(), GetFileExInfoStandard, &nFileAttr))
	{
		nAttr.m_bDirectory = (nFileAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		nAttr.m_nCreateTime = __WinFileTime2UnixTime(nFileAttr.ftCreationTime);
		nAttr.m_nModifyTime = __WinFileTime2UnixTime(nFileAttr.ftLastWriteTime);
		nAttr.m_nAccessTime = __WinFileTime2UnixTime(nFileAttr.ftLastAccessTime);
		nAttr.m_nSize = (((uint64_t)(nFileAttr.nFileSizeHigh)) << 32) + (uint64_t)(nFileAttr.nFileSizeLow);

		return true;
	}
#else
	struct stat nFileStat;
	if (stat(file1.c_str(), &nFileStat) == 0)
	{
		nAttr.m_bDirectory = (nFileStat.st_mode & S_IFDIR) != 0;
		nAttr.m_nCreateTime = nFileStat.st_ctime;
		nAttr.m_nModifyTime = nFileStat.st_mtime;
		nAttr.m_nAccessTime = nFileStat.st_atime;
		nAttr.m_nSize = nFileStat.st_size;

		return true;
	}
#endif
	else
	{
		return false;
	}
}

std::string ToLower(const std::string &strIn)
{
	std::string str = strIn;
	for (std::string::size_type i = 0; i < str.length(); ++i)
	if (str[i] >= 'A' && str[i] <= 'Z') {
		str[i] += 0x20;
	}
	return str;
}

std::string GetExtensionName(const std::string &strPath)
{
	std::string strNormalPath = RegularPath(strPath);

	// check whether url is like "*****?url=*.*.*.*"
	size_t nEndPos = strNormalPath.find('?');
	if (nEndPos != std::string::npos)
	{
		strNormalPath = strNormalPath.substr(0, nEndPos);
	}

	size_t nPos = strNormalPath.rfind('.');
	if (std::string::npos == nPos || (nPos + 1) == strNormalPath.size())
	{
		return std::string("");
	}

	return strNormalPath.substr(nPos + 1);
}

bool GetLocalTimestamp(time_t nTimestamp, struct tm &nTimeValue)
{
#ifdef WIN32
	return localtime_s(&nTimeValue, &nTimestamp) == 0;
#else
	return localtime_r(&nTimestamp, &nTimeValue) != NULL;
#endif
}

bool GetGmtTimestamp(time_t nTimestamp, struct tm &nTimeValue)
{
#ifdef WIN32
	return gmtime_s(&nTimeValue, &nTimestamp) == 0;
#else
	return gmtime_r(&nTimestamp, &nTimeValue) != NULL;
#endif
}

std::string FormatLongTime(time_t tmValue, bool bGmt)
{
	static const char *pszWeekName[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const char *pszMonthName[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	struct tm nTmInfo = { 0 };

	if (bGmt) GetGmtTimestamp(tmValue, nTmInfo);
	else GetLocalTimestamp(tmValue, nTmInfo);

	if (nTmInfo.tm_wday < 0 || nTmInfo.tm_wday >= 7)
	{
		nTmInfo.tm_wday = 0;
	}

	if (nTmInfo.tm_mon < 0 || nTmInfo.tm_mon >= 12)
	{
		nTmInfo.tm_mon = 0;
	}

	char buffer[256];
#ifdef WIN32
	sprintf_s(buffer, sizeof(buffer), "%s, %02d %s %04d %02d:%02d:%02d%s",
		pszWeekName[nTmInfo.tm_wday],
		nTmInfo.tm_mday,
		pszMonthName[nTmInfo.tm_mon],
		nTmInfo.tm_year + 1900,
		nTmInfo.tm_hour, nTmInfo.tm_min, nTmInfo.tm_sec,
		bGmt ? " GMT" : "");
#else
	sprintf(buffer, "%s, %02d %s %04d %02d:%02d:%02d%s",
		pszWeekName[nTmInfo.tm_wday],
		nTmInfo.tm_mday,
		pszMonthName[nTmInfo.tm_mon],
		nTmInfo.tm_year + 1900,
		nTmInfo.tm_hour, nTmInfo.tm_min, nTmInfo.tm_sec,
		bGmt ? " GMT" : "");
#endif
	std::string bufferstring = buffer;


	return bufferstring;
}

int32_t Compare(const std::string &str1, const std::string &str2, bool bIgnoreCase /* = false */)
{
	if (!bIgnoreCase)
	{
		return strcmp(str1.c_str(), str2.c_str());
	}
	else
	{
#ifdef WIN32
		return _stricmp(str1.c_str(), str2.c_str());
#else
		return strcasecmp(str1.c_str(), str2.c_str());
#endif
	}
}

std::string FormatV(const char *pszFormat, va_list pArgList)
{
	std::string strRes;
	int   nSize = 0;
	char *pszBuffer = NULL;

#ifdef WIN32
	nSize = _vscprintf(pszFormat, pArgList);
#else
	va_list pArgCopy;
	va_copy(pArgCopy, pArgList);
	nSize = vsnprintf(NULL, 0, pszFormat, pArgCopy);
	va_end(pArgCopy);
#endif //OS_PLATFORM_WINDOWS

	pszBuffer = new char[nSize + 1];
	pszBuffer[nSize] = 0;
	vsnprintf(pszBuffer, nSize, pszFormat, pArgList);

	strRes = pszBuffer;
	delete[]pszBuffer;

	return strRes;
}

std::string Format(const char *pszFormat, ...)
{
	va_list pArgList;

	va_start(pArgList, pszFormat);
	std::string strRes = FormatV(pszFormat, pArgList);
	va_end(pArgList);

	return strRes;
}

void Strtok(const std::string &str, char separator, std::vector<std::string> &arr)
{
	size_t pos = 0;
	size_t newPos = 0;

	while (std::string::npos != pos) {
		pos = str.find_first_of(separator, newPos);
		if (std::string::npos == pos) { // ½áÊøÁË
			if (pos > newPos) {
				arr.push_back(str.substr(newPos, pos - newPos));
			}
			break;
		}
		else {
			if (pos > newPos) {
				arr.push_back(str.substr(newPos, pos - newPos));
			}
			newPos = pos + 1;
		}
	}
}

void
server::handle_request(const request& req, reply& rep)
{
    // Decode url to path.
	se_mutex_.lock();
	star_count_++;
	se_mutex_.unlock();
	int64_t start_time = utils::Timestamp::HighResolution();

	std::string command = req.command;
    if (mRoutes.find(command) != mRoutes.end())
    {
		mRoutes[command](req, rep.content);

        rep.status = reply::ok;
        rep.headers.resize(3);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = std::to_string(rep.content.size());
        rep.headers[1].name = "Content-Type";
		rep.headers[1].value = "application/json";
		rep.headers[2].name = "Connection";
		rep.headers[2].value = "close";
    }
    else
    {
		int32_t nNeed = 0;//1:notfound, 2: fobiden, 3: notmodify, 4:nomal
		std::string file_path;
		uint64_t nFileSize = 0;
		std::string strContentEncoding;
		std::string strContentType;
		do {
			
			if (command.size() == 0 || command[0] != '/') command = "/" + command;
			if (command == "/") command += index_file_;
			if (command.find("/../") != std::string::npos ||
				command.find("\\..\\") != std::string::npos ||
				command.find("/..\\") != std::string::npos ||
				command.find("\\../") != std::string::npos)
			{
				nNeed = 2;
				//rep = reply::stock_reply(reply::forbidden);
				break;
			}

			FileAttribute nFileAttr;
			file_path = web_home_ + command;

			if (!GetAttribue(file_path, nFileAttr))
			{
				nNeed = 1;
				//rep = reply::stock_reply(reply::not_found);
				break;
			}
			else if (nFileAttr.m_bDirectory)
			{
				if (command[command.size() - 1] != '/')
				{
					// redirect
					nNeed = 2;
					//rep = reply::stock_reply(reply::forbidden);
					break;
				}

				file_path += "/";
				file_path += index_file_;

				if (!GetAttribue(file_path, nFileAttr))
				{
					nNeed = 1;
					break;
				}
			}

			nFileSize = nFileAttr.m_nSize;

			std::string strFileExt = ToLower(GetExtensionName(file_path));
			std::string strLastModified = FormatLongTime(nFileAttr.m_nModifyTime, true);

			if (compress_type_.find(strFileExt) != compress_type_.end())
			{
				strContentEncoding = compress_type_[strFileExt];

				if (strFileExt.size() + 1 < file_path.size())
				{
					std::string strActualFile = file_path.substr(0, file_path.size() - strFileExt.size() - 1);
					strFileExt = ToLower(GetExtensionName(strActualFile));
				}
			}

			if (content_type_.find(strFileExt) != content_type_.end())
			{
				strContentType = content_type_[strFileExt];
			}

			std::string strRequestModified = req.GetHeaderValue("if-modified-since");
			if ( Compare(strRequestModified, strLastModified, true) == 0)
			{
				nNeed = 3;
			}
			else
			{
				//if (!nFile.Open(strFilePath, Utils::File::FILE_M_READ))
				//{
				//	errorif = true;
				//	rep = reply::stock_reply(reply::internal_server_error);
				//	break;
				//}

				nNeed = 4;
			}
		} while ( false );

		if ( nNeed == 1 ){
			if (mRoutes.find("404") != mRoutes.end())
			{
				mRoutes["404"](req, rep.content);

				rep.status = reply::ok;
				rep.headers.resize(3);
				rep.headers[0].name = "Content-Length";
				rep.headers[0].value = std::to_string(rep.content.size());
				rep.headers[1].name = "Content-Type";
				rep.headers[1].value = "text/html";
				rep.headers[2].name = "Connection";
				rep.headers[2].value = "close";
			}
			else
			{
				rep = reply::stock_reply(reply::not_found);
				return;
			}
		}
		else if (nNeed == 2){
			rep = reply::stock_reply(reply::forbidden);
		}
		else if (nNeed == 3){
			//not modify
			rep.status = reply::not_modified;
		}
		else {
			FILE *handle;
			handle = fopen(file_path.c_str(), "r");
			if (NULL == handle)
			{
				rep = reply::stock_reply(reply::internal_server_error);
			}

			const size_t nBufferSize = 1024;
			size_t nReadBytes = 0;
			char nTmpBuffer[nBufferSize];

			while (nReadBytes < nFileSize)
			{
				size_t nCount = std::min<size_t>(nFileSize - nReadBytes, nBufferSize);
				size_t nRetBytes = fread(nTmpBuffer, 1, nCount, handle);

				if (nRetBytes > 0)
				{
					nReadBytes += nRetBytes;
					rep.content.append(nTmpBuffer, nRetBytes);
				}
				else
				{
					break;
				}
			}

			fclose(handle);

			rep.status = reply::ok;
			rep.headers.resize(4);
			rep.headers[0].name = "Content-Length";
			rep.headers[0].value = std::to_string(rep.content.size());
			rep.headers[1].name = "Content-Type";
			rep.headers[1].value = strContentType;
			rep.headers[2].name = "Content-Encoding";
			rep.headers[2].value = strContentEncoding;
			rep.headers[3].name = "Connection";
			rep.headers[3].value = "close";

		}
    }

	int64_t use_time = (utils::Timestamp::HighResolution() - start_time);
	if (use_time > utils::MICRO_UNITS_PER_SEC) {
		LOG_WARN("Execute request(uri:%s, body:%s) from ip(%s), use time(" FMT_I64 "ms) is too log, request(start:" FMT_I64 "|end:" FMT_I64 ")",
			req.uri.c_str(), req.body.c_str(), req.peer_address_.ToIpPort().c_str(), use_time / utils::MILLI_UNITS_PER_SEC, star_count_, end_count_);

		se_mutex_.lock();
		expire_count_++;
		se_mutex_.unlock();
	}

	// Decode url to path.
	se_mutex_.lock();
	end_count_++;
	se_mutex_.unlock();
}

void server::SetHome(const std::string &home){
	web_home_ = home;
}

void server::SetIndexName(const std::string &index_name){
	index_file_ = index_name;
}

bool
url_decode(const std::string& in, std::string& out)
{
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i)
    {
        if (in[i] == '%')
        {
            if (i + 3 <= in.size())
            {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value)
                {
                    out += static_cast<char>(value);
                    i += 2;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else if (in[i] == '+')
        {
            out += ' ';
        }
        else
        {
            out += in[i];
        }
    }
    return true;
}

void server::parseParams(const std::string& params, std::map<std::string, std::string>& retMap)
{
    bool buildingName=true;
    std::string name,value;
    for(auto c : params)
    {
        if(c == '?')
        {

        }else if(c == '=')
        {
            buildingName = false;
        }else if(c == '&')
        {
            buildingName = true;
            retMap[name] = value;
            name = "";
            value = "";
        } else
        {
            if(buildingName) name += c;
            else value += c;
        }
    }
    if(name.size() && value.size())
    {
        retMap[name] = value;
    }
}

} // namespace server
} // namespace http
