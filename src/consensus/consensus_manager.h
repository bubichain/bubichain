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

#ifndef CONSENSUS_MANAGER_H_
#define CONSENSUS_MANAGER_H_

#include <utils/singleton.h>
#include <utils/net.h>
#include <common/general.h>
#include "pbft.h"

namespace bubi {

	class ConsensusManager : public utils::Singleton<ConsensusManager>,
		public TimerNotify,
		public StatusModule {
		friend class utils::Singleton<ConsensusManager>;
	private:
		ConsensusManager();
		~ConsensusManager();
		std::shared_ptr<Consensus> consensus_;
	public:
		bool Initialize(const ValidationConfigure &config);
		bool Exit();
		std::shared_ptr<Consensus> GetConsensus();
		virtual void OnTimer(int64_t current_time);
		virtual void OnSlowTimer(int64_t current_time);
		virtual void GetModuleStatus(Json::Value &data);
	};
}

#endif