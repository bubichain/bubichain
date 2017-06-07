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

#ifndef MASTER_SERVICE_H_
#define MASTER_SERVICE_H_

#include <common/general.h>
#include <utils/singleton.h>
#include <utils/thread.h>
#include <string>
#include <vector>
#include "common/pipeline_server.h"
#include "common/zmq_helper.h"

namespace bubi {
	class MasterService :
		public utils::Singleton<bubi::MasterService>,
		public bubi::PipelineServer {
		friend class utils::Singleton<bubi::MasterService>;
	public:
		MasterService();
		~MasterService();
		bool Initialize();
		bool Exit();
		//Make sure call this in the same thread
		bool SendToSlave(const ZMQTaskType type, const std::string& buf);
		//process a valid transaction
		static void OnNewTransaction(const char* msg, int len, std::string &reply);
		//verify transaction first, then call OnBroadcastTransaction
		static void OnBroadcastTransactionVerify(const char* msg, int len, std::string &reply);
		//verify transaction first, then call OnTxSetTransaction
		static void OnTxSetTransactionVerify(const char* msg, int len, std::string &reply);
		static void OnBroadcastTransaction(const char* msg, int len, std::string &reply);
		static void OnTxSetTransaction(const char* msg, int len, std::string &reply);
	};
}

#endif
