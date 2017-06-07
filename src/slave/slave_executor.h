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

#ifndef SLAVE_EXECUTOR_H_
#define SLAVE_EXECUTOR_H_

#include <utils/singleton.h>
#include <utils/thread.h>
#include <common/general.h>
#include <common/zmq_helper.h>

namespace bubi {
	class SlaveExecutor {
	public:
		SlaveExecutor();
		~SlaveExecutor();
		SlaveExecutor(const SlaveExecutor& s) = delete;
		static void ProcessBroadcastTransaction(const char* msg, int len, std::string &reply);
		static void ProcessTxSetTransaction(const char* msg, int len, std::string &reply);
	};
}

#endif
