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

#ifndef MQ_SERVER_H_
#define MQ_SERVER_H_
#include <proto/message.pb.h>
#include <common/general.h>
#include <common/pipeline_server.h>

namespace bubi {
	class MQServer :public utils::Singleton<bubi::MQServer>,
		public bubi::PipelineServer {
		friend class utils::Singleton<bubi::MQServer>;
	public:
		MQServer(int32_t send_buffer_size = 100000);
		~MQServer();
        bool Send(const ZMQTaskType type, const std::string& buf);
		bool Initialize(MqServerConfigure & mq_server_configure);
		bool Exit();
		//Handlers
		static void OnChainHello(const char* msg, int len, std::string &reply);
		static void OnChainPeerMessage(const char* msg, int len, std::string &reply);
		static void OnSubmitTransaction(const char* msg, int len, std::string &reply);
	protected:
		virtual void OnTimer(int64_t current_time) override;
		virtual void OnSlowTimer(int64_t current_time) override;
	private:
		virtual void Recv(const ZMQTaskType type, std::string& buf);
		//check operation
		static bool CheckCreateAccountOpe(const protocol::Operation &ope, Result &result);
		static bool CheckInitPayment(const protocol::Operation &ope, Result &result);
		static bool CheckPayment(const protocol::Operation &ope, Result &result);
		static bool CheckIssueAsset(const protocol::Operation &ope, Result &result);
		static bool CheckIssueUniqueAsset(const protocol::Operation &ope, Result &result);
		static bool CheckPaymentUniqueAsset(const protocol::Operation &ope, Result &result);
		static bool CheckSetOptions(const protocol::Operation &ope, Result &result);
		static bool CheckProduction(const protocol::Operation &ope, Result &result);
        static bool CheckRecord(const protocol::Operation &ope, Result &result);
	private:
        utils::ReadWriteLock send_list_mutex_;
        std::list<std::pair<ZMQTaskType, std::string>> send_list_;
        bool init_;
        uint32_t send_buffer_size_;
	};
}

#endif