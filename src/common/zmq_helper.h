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

#ifndef ZMQ_HELPER_H_
#define ZMQ_HELPER_H_

#include <common/general.h>
#include <utils/singleton.h>
#include <utils/thread.h>
#include <string>
#include <vector>
#include "zmq.h"

namespace bubi {
	//To avoid copy use C-string for request, protobuf is compatible with C-string
	typedef std::function<void(const char* msg, int len, std::string &reply)> DynamicHandler;
	typedef void(*StaticHandler)(const char* msg, int len, std::string &reply);

	static const int32_t ZMQ_RECV_BUFFER_SIZE = 10 * 1024 * 1024;

	//To keep consistency between master and slave, please use ZMQTaskType to define task type instead of hard code.
	enum ZMQTaskType {
		// ZMQ_LARGE_MSG is reserved for internal control
		ZMQ_LARGE_MSG = 0,

		// [1,100) public message defined in message.proto
		ZMQ_CHAIN_HELLO = 1,
		ZMQ_CHAIN_STATUS,
		ZMQ_CHAIN_TX_STATUS,
		ZMQ_CHAIN_PEER_ONLINE,
		ZMQ_CHAIN_PEER_OFFLINE,
		ZMQ_CHAIN_PEER_MESSAGE,
		ZMQ_CHAIN_SUBMITTRANSACTION,

		// [100,200) internal message between bubi and slave
		ZMQ_BROADCAST_TX = 100,
		ZMQ_NEW_TX,
		ZMQ_TXSET_TX,
		ZMQ_BROADCAST_TX_VERIFY,
		ZMQ_TXSET_TX_VERIFY,

		// [200, 255) monitor message 
		ZMQ_MONITOR_NULL = 200,
		ZMQ_MONITOR_BUBI,
		ZMQ_MONITOR_LEDGER,
		ZMQ_MONITOR_SYSTEM,
		ZMQ_MONITOR_ALERT,
		ZMQ_MONITOR_ACCOUNT_EXCEPTION,
		ZMQ_MONITOR_NOTICE
	};

	inline int zmq_send_ex(void* zsocket, const char* msg, int len) {
		if (len > ZMQ_RECV_BUFFER_SIZE) {
			char largeMsg[5] = {
				char((len >> 24) & 0xFF),
				char((len >> 16) & 0xFF),
				char(len >> 8 & 0xFF),
				char(len & 0xFF),
				ZMQ_LARGE_MSG
			};
			zmq_send(zsocket, largeMsg, sizeof(largeMsg), ZMQ_SNDMORE);
		}
		return zmq_send(zsocket, msg, len, 0);
	}

	//zmq_send will fail to send because of EAGAIN when use non-block mode
	//need to figure out how to use it before use
	inline int zmq_send_ex_nonblock(void* zsocket, const char* msg, int len) {
		if (len > ZMQ_RECV_BUFFER_SIZE) {
			char largeMsg[5] = {
				char((len >> 24) & 0xFF),
				char((len >> 16) & 0xFF),
				char(len >> 8 & 0xFF),
				char(len & 0xFF),
				ZMQ_LARGE_MSG
			};
			zmq_send(zsocket, largeMsg, sizeof(largeMsg), ZMQ_SNDMORE | ZMQ_NOBLOCK);
		}
		return zmq_send(zsocket, msg, len, ZMQ_NOBLOCK);
	}
}

#endif
