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


#include "upgrade.h"

#include <monitor/processmessage.h>
#include <utils/logger.h>
#include <utils/crypto.h>
#include <utils/file.h>
#include <curl/curl.h>
#include <stdlib.h>

#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif // 

namespace bubi {
	Upgrade::Upgrade(ProcessMessage* process_message) : pprocess_message_(process_message) {}

	Upgrade::~Upgrade() {}

	bool Upgrade::Initialize() {
		bool bret = false;
		do {
			value_.clear();
			thread_ptr_ = new utils::Thread(this);
			if (!thread_ptr_->Start("upgrade")) {
				break;
			}
			bret = true;
		} while (false);

		return true;
	}

	bool Upgrade::Exit() {
		if (thread_ptr_) {
			thread_ptr_->JoinWithStop();
			delete thread_ptr_;
			thread_ptr_ = NULL;
		}
		value_.clear();
		return true;
	}


	void Upgrade::SetItems(const Json::Value value) {
		value_.fromString(value.toFastString());
	}

	void Upgrade::Run(utils::Thread *thread) {
		while (thread->enabled()) {
			if (value_.empty()) {
				utils::Sleep(5000);
				continue;
			}
			try {
				Json::Value result;
				std::string method = "upgrade";
				int error_code = 0;
				do {
					// upgrade directory
					std::string dir_path = utils::File::GetBinHome() + "/upgrade/";
					if (-1 == access(dir_path.c_str(), 0)) {
#ifdef WIN32
						if (-1 == _mkdir(dir_path.c_str())) {
#else
						umask(0);
						if (-1 == mkdir(dir_path.c_str(), 0755)) {
#endif
							LOG_ERROR("agent_manager : %s", strerror(errno));
							error_code = 9;
							result["state"] = 0;
							break;
						}
						}

					// download all file
					size_t nitems = value_.size();

					Json::Value downloaded_files;
					bool check_parameter = true;
					bool check_md5 = true;
					bool files_this_program_exists = true;

					for (size_t nitem = 0; nitem < nitems; nitem++) {
						Json::Value& item = value_[nitem];
						if (item["file_name"].empty() || item["url"].empty() || item["md5"].empty()) {
							check_parameter = false;
							break;
						}
						std::string file_name = item["file_name"].asString();
						std::string file_path = dir_path + file_name + ".tmp";
						bool bdownload = false;
						do {
							// download file is not what we need
							std::string newfile_path = "";
							if (0 == file_name.compare("bubi") || 0 == file_name.compare("bubid") ||
								0 == file_name.compare("slave") || 0 == file_name.compare("slaved")) {
								file_name = dir_path + "../bin/" + file_name;
							}
							else if (0 == file_name.compare("bubi.json") ||
								0 == file_name.compare("cacert.pem") ||
								0 == file_name.compare("cacert.crt") ||
								0 == file_name.compare("privkey.pem") ||
								0 == file_name.compare("dh1024.pem")) {
								newfile_path = dir_path + "../config/" + file_name;
							}
							else {
								files_this_program_exists = false;
								break;
							}
							if (false == DownloadFile(item["url"].asString(), file_path)) {
								break;
							}
							// check md5
							if (item["md5"].asString() != utils::MD5::GenerateMd5File(file_path.c_str())) {
								check_md5 = false;
								break;
							}
							bdownload = true;
							// rename download files
							//rename(file_path.c_str(), newfile_path.c_str());
							//file_path = newfile_path;
						} while (false);

						if (bdownload) {
							Json::Value download_file;
							download_file["file_path"] = file_path;
							downloaded_files.append(download_file);
						}
					}

					// some file download failed
					size_t stsize = downloaded_files.size();
					if (!files_this_program_exists || !check_md5 || stsize < nitems) {

						for (size_t i = 0; i < stsize; i++) {
							remove(downloaded_files[i]["file_path"].asString().c_str());
						}
						if (!check_parameter)
							error_code = 14;
						else if (!files_this_program_exists)
							error_code = 19;
						else if (!check_md5)
							error_code = 11;
						else
							error_code = 10;
						result["state"] = 0;
						break;
					}

#ifdef WIN32
					//system("net stop bubi");
					//system("net stop bubid");

					//system("net start bubi");
					//system("net start bubid");
#else
					if (system("service bubi stop") == -1 || system("service bubid stop") == -1) {
						LOG_ERROR("agent_manager : stop bubi failed");
						error_code = 21;
						result["state"] = 0;
						break;
					}
					if (system("service bubi start") == -1 || system("service bubid start") == -1) {
						LOG_ERROR("agent_manager : start bubi failed");
						error_code = 21;
						result["state"] = 0;
						break;
					}
#endif
					error_code = 0;
					result["state"] = 2;
					} while (false);

					if (pprocess_message_) {
						result["session_id"] = pprocess_message_->random_key_;
						pprocess_message_->SendResponseMessage(method, true, error_code, result);
					}

				}
			catch (std::exception& e) {
				LOG_ERROR("http_download : %s", e.what());
			}
			if (pprocess_message_) {
				pprocess_message_->bupgrading_ = false;
			}
			value_.clear();
			}
		}

	bool Upgrade::DownloadFile(const std::string& url_address, const std::string& file_path) {
		// init curl
		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url_address.c_str());

		// set callback function 
		FILE* file = fopen(file_path.c_str(), "wb");
		if (NULL == file) {
			LOG_ERROR("http_download : %s", strerror(errno));
			return false;
		}

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

		CURLcode retcCode = curl_easy_perform(curl);

		if (retcCode != CURLE_OK) {
			LOG_ERROR("http_download : %s", curl_easy_strerror(retcCode));
		}

		fclose(file);

		// clean curl
		curl_easy_cleanup(curl);

		return !retcCode;
	}

	size_t Upgrade::DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam) {
		FILE* fp = (FILE*)pParam;
		size_t nWrite = fwrite(pBuffer, nSize, nMemByte, fp);

		return nWrite;
	}
	}