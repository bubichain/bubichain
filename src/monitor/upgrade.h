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

#ifndef UPGRADE_H_
#define UPGRADE_H_

#include <string>
#include <json/json.h>
#include <utils/thread.h>

namespace bubi {
	class ProcessMessage;
	class Upgrade : public utils::Runnable {
	public:
		Upgrade(ProcessMessage* process_message);
		~Upgrade();
		bool Initialize();
		bool Exit();
		void SetItems(const Json::Value value);
		void Run(utils::Thread *thread);
	private:
		bool DownloadFile(const std::string& url_address, const std::string& file_path);
		static size_t DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam);
	private:
		ProcessMessage* pprocess_message_;
		utils::Thread* thread_ptr_;
		Json::Value value_;
	};
}

#endif
