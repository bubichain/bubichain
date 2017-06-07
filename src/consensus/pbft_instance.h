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

#ifndef PBFT_INSTANCE_
#define PBFT_INSTANCE_

#include <utils/headers.h>
#include <overlay/peer.h>

namespace bubi {
	typedef std::map<int64_t, protocol::PbftPrepare> PbftPrepareMap; //replica id => message
	typedef std::map<int64_t, protocol::PbftCommit> PbftCommitMap; //replica id => message
	typedef std::map<int64_t, protocol::PbftCheckPoint> PbftCheckPointMap; //replica id => message
	typedef std::map<int64_t, protocol::PbftViewChange> PbftViewChangeMap; //replica id => message
	typedef std::vector<protocol::PbftEnv> PbftPhaseVector;
	typedef std::vector<PbftPhaseVector> PbftPhaseVector2;

	const int64_t g_pbft_vcinstance_timeout_ = 60 * utils::MICRO_UNITS_PER_SEC;
	const int64_t g_pbft_vcinstance_terminatedtime_ = 300 * utils::MICRO_UNITS_PER_SEC;
	const int64_t g_pbft_instance_timeout_ = 60 * utils::MICRO_UNITS_PER_SEC;
	const int64_t g_pbft_commit_send_interval = 15 * utils::MICRO_UNITS_PER_SEC; // for retransmit
	const int64_t g_pbft_newview_send_interval = 15 * utils::MICRO_UNITS_PER_SEC; // for retransmit

	typedef enum PbftInstancePhaseTag {
		PBFT_PHASE_NONE,
		PBFT_PHASE_PREPREPARED,
		PBFT_PHASE_PREPARED,
		PBFT_PHASE_COMMITED,
		PBFT_PHASE_MAX
	}PbftInstancePhase;

	//Phase => message type
	//Phase           NONE          | PRE-PREPARED     | PREPARED | COMMITED
	//message type    PRE-PREPARE   | PREPARE          | COMMIT   | REPLY


	//pbft sequence    1  2  3  4  5  6  7  8  9  10
	//pbft checkpoint              5              10

	class PbftInstanceIndex {
	public:
		PbftInstanceIndex(int64_t view_number, int64_t sequence);
		~PbftInstanceIndex();
		bool operator < (const PbftInstanceIndex &index) const;
	public:
		int64_t view_number_;
		int64_t sequence_;
	};

	class Pbft;
	class PbftInstance {
	public:
		PbftInstance();
		~PbftInstance();
		bool Go(const protocol::PbftEnv &env, Pbft *pbft);
		bool IsExpire(int64_t current_time);
		bool NeedSendAgain(int64_t current_time);
		bool NeedSendCommitAgain(int64_t current_time);
		void SetLastProposeTime(int64_t current_time);
		void SetLastCommitSendTime(int64_t current_time);
		bool SendPrepareAgain(Pbft *pbft, int64_t current_time);
	public:
		PbftInstancePhase phase_;
		size_t phase_item_;
		protocol::PbftPrePrepare pre_prepare_;
		PbftPrepareMap prepares_;
		PbftCommitMap commits_;
		PbftPhaseVector2 msg_buf_;
		protocol::PbftEnv pre_prepare_msg_;
		int64_t start_time_;
		int64_t end_time_;
		int64_t last_propose_time_;
		int64_t last_commit_send_time_;
		bool have_send_viewchange_;
		uint32_t pre_prepare_round_;
		uint32_t commit_rount_;
		int32_t check_value_result_;
	};

	typedef std::map<PbftInstanceIndex, PbftInstance> PbftInstanceMap;

	class PbftCkpInstanceIndex {
	public:
		PbftCkpInstanceIndex();
		PbftCkpInstanceIndex(int64_t seq, const std::string &state_digest);
		~PbftCkpInstanceIndex();
		bool operator < (const PbftCkpInstanceIndex &index) const;
	public:
		int64_t sequence_;
		std::string state_digest_;
	};
	class PbftCkpInstance {
	public:
		PbftCkpInstance();
		~PbftCkpInstance();
	public:
		bool stable_;
		PbftCheckPointMap checkpoints_;
		PbftPhaseVector msg_buf_;
	};

	typedef std::map<PbftCkpInstanceIndex, PbftCkpInstance> PbftCkpInstanceMap; // seq => map
	typedef std::pair<PbftCkpInstanceIndex, PbftCkpInstance> PbftCkpInstancePair;

	class PbftVcInstance {
	public:
		PbftVcInstance();
		PbftVcInstance(int64_t view_number);
		~PbftVcInstance();
		bool ShouldTeminated(int64_t current_time, int64_t time_out);
		bool NeedSendAgain(int64_t current_time);
		void SetLastProposeTime(int64_t current_time);
		bool NeedSendNewViewAgain(int64_t current_time);
		bool SendNewViewAgain(Pbft *pbft, int64_t current_time);
		bool SendNewView(Pbft *pbft, int64_t current_time, PeerMessagePointer new_ptr);
		void ChangeComplete(int64_t current_time);
	private:
		void SetLastNewViewTime(int64_t current_time);
	public:
		protocol::PbftEnv view_change_msg_;
		int64_t view_number_;
		PbftViewChangeMap viewchanges_;
		uint32_t view_change_round_;
		PbftPhaseVector msg_buf_; //view change message
		int64_t start_time_;
		int64_t last_propose_time_;
		int64_t end_time_;
		int64_t last_newview_time_;
		protocol::PbftEnv newview_;
		uint32_t new_view_round_;
	};

	typedef std::map<int64_t, PbftVcInstance> PbftVcInstanceMap; // view_number => map

	class PbftDesc {
	public:
		static std::string GetPbft(const protocol::Pbft &message);
		static std::string GetPrePrepare(const protocol::PbftPrePrepare &pre_prepare);
		static std::string GetPrepare(const protocol::PbftPrepare &prepare);
		static std::string GetCommit(const protocol::PbftCommit &commit);
		static std::string GetCheckPoint(const protocol::PbftCheckPoint &checkpoint);
		static std::string GetViewChange(const protocol::PbftViewChange &viewchange);
		static std::string GetNewView(const protocol::PbftNewView &newview);
		static std::string GetValueString(const protocol::Value &value);
		static const char *GetMessageTypeDesc(enum protocol::PbftMessageType type);
	public:
		static const char *SEQUENCE_NAME;
		static const char *VIEWNUMBER_NAME;
		static const char *CHECKPOINT_NAME;
		static const char *LAST_EXE_SEQUENCE_NAME;
		static const char *LOW_WATER_MRAK_NAME;
		static const char *INSTANCE_NAME;
	};
}

#endif