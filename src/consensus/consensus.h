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

#ifndef CONSENSUS_H_
#define CONSENSUS_H_

#include <utils/common.h>
#include <common/general.h>
#include <common/private_key.h>
#include <common/storage.h>
#include <proto/message.pb.h>
#include <overlay/peer.h>
#include "consensus_msg.h"

namespace bubi {
	class IConsensusNotify {
	public:
		IConsensusNotify() {};
		~IConsensusNotify() {};
		virtual std::string OnValueCommited(int64_t block_seq, int64_t request_seq, const protocol::Value &value, bool calculate_total) = 0;
		virtual void OnViewChanged() = 0;
		virtual int32_t CheckValue(int64_t block_seq, const protocol::Value &value) = 0;
		virtual void SendConsensusMessage(const PeerMessagePointer &message) = 0;
		virtual std::string FetchNullMsg() = 0;
	};

	class Consensus {
	public:
		Consensus();
		~Consensus();
		enum CheckValueResult {
			CHECK_VALUE_VALID,
			CHECK_VALUE_MAYVALID,
			CHECK_VALUE_INVALID
		};
		virtual bool Initialize();
		virtual bool Exit();
		virtual bool Request(int64_t block_seq, const protocol::Value &value) { return true; };
		virtual bool RepairStatus() { return true; }; // true : it is normal, false : waiting for pbft's notify
		virtual bool OnRecv(const ConsensusMsg &message) { return true; };
		virtual size_t GetQuorumSize() { return 0; };
		virtual void OnTimer(int64_t current_time) {};
		virtual void OnSlowTimer(int64_t current_time) {};
		virtual void GetModuleStatus(Json::Value &data) {};
		virtual void OnTxTimeout() {};
		static std::string GetValueString(const protocol::Value &value);
		static int32_t CompareValue(const protocol::Value &value1, const protocol::Value &value2);
		static int32_t CompareValue(const std::string &value1, const std::string &value2);
		static bool SaveValue(const std::string &name, const std::string &value);
		static bool SaveValue(const std::string &name, int64_t value);
		static int32_t LoadValue(const std::string &name, std::string &value);
		static int32_t LoadValue(const std::string &name, int64_t &value);
		static bool DelValue(const std::string &name);
		static bool InsertValue(const std::string &name, const std::string &value);
		static bool InsertValue(const std::string &name, int64_t value);
		void SetNotify(IConsensusNotify *notify);
		bool IsValidator();
		virtual int32_t IsLeader() { return -1; };
		std::string GetNodeAddress();
		int64_t GetValidatorIndex(const std::string &node_address) const;
	protected:
		int32_t CheckValue(int64_t block_seq, const protocol::Value &value);
		bool SendMessage(const PeerMessagePointer &message);
		std::string OnValueCommited(int64_t block_seq, int64_t request_seq, const protocol::Value &value, bool calculate_total);
		void OnViewChanged();
	protected:
		std::string name_;
		bool is_validator_;
		PrivateKey private_key_;
		int64_t replica_id_;
		std::map<std::string, int64_t> validators_;
		std::string validators_hash_;
		//lock the instance
		utils::Mutex lock_;
		//notify
		IConsensusNotify *notify_;
	};

	class ValueSaver {
	public:
		ValueSaver();
		~ValueSaver();
		void SaveValue(const std::string &name, const std::string &value);
		void SaveValue(const std::string &name, int64_t value);
		void DelValue(const std::string &name);
		bool Commit();
	public:
		size_t write_size_;
		WRITE_BATCH writes_;
	};

	class OneNode : public Consensus {
	public:
		OneNode();
		~OneNode();
		virtual bool Request(int64_t block_seq, const protocol::Value &value);
		virtual void GetModuleStatus(Json::Value &data);
	};
}

#endif
