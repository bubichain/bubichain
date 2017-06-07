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

#include <utils/headers.h>
#include "pbft.h"
#include "consensus_msg.h"

namespace bubi {
	ConsensusMsg::ConsensusMsg(const protocol::PbftEnv &pbft_env) :pbft_env_(pbft_env), block_seq_(-1) {
		type_ = "pbft";
		seq_ = Pbft::GetSeq(pbft_env_);
		block_seq_ = Pbft::GetBlockSeq(pbft_env_);
		values_ = Pbft::GetValue(pbft_env_);
		node_address_ = Pbft::GetNodeAddress(pbft_env_);
		hash_ = utils::Sha256::Crypto(pbft_env_.SerializeAsString());
	};

	ConsensusMsg::~ConsensusMsg() {}

	bool ConsensusMsg::operator < (const ConsensusMsg &msg) const {
		return memcmp(hash_.c_str(), msg.hash_.c_str(), hash_.size()) < 0;
	}
	bool ConsensusMsg::operator == (const ConsensusMsg &value_frm) const {
		return type_ == "pbft" &&
			type_ == value_frm.type_ &&
			hash_ == value_frm.hash_;
	}

	int64_t ConsensusMsg::GetSeq() const {
		return seq_;
	}

	int64_t ConsensusMsg::GetBlockSeq() const {
		return block_seq_;
	}

	std::vector<protocol::Value> ConsensusMsg::GetValues() const {
		return values_;
	}

	PeerMessagePointer ConsensusMsg::ComposeNewPeerMessage() const {
		if (type_ == "pbft") {
			PeerMessagePointer peer_msg = PeerMessage::NewPbft();
			((protocol::PbftEnv *)peer_msg->data_)->CopyFrom(pbft_env_);
			peer_msg->hash_ = utils::Sha256::Crypto(peer_msg->GetString());
			return peer_msg;
		}

		return NULL;
	}

	const char *ConsensusMsg::GetNodeAddress() const {
		return node_address_.c_str();
	}

	std::string ConsensusMsg::GetType() const {
		return type_;
	}

	protocol::PbftEnv ConsensusMsg::GetPbft() const {
		return pbft_env_;
	}
}