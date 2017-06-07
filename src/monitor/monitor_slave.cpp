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

#include "monitor_slave.h"
#include <common/configure.h>
#include <utils/file.h>

namespace bubi {
	MonitorSlave::MonitorSlave() : MonitorAgent(MonitorAgent::SUBSCRIBE) {

	}

	MonitorSlave::~MonitorSlave() {

	}

	bool MonitorSlave::Initialize() {
		bool bret = false;
		do {
			std::string log_path = SlaveConfigure::Instance().logger_configure_.path_;
			if (!utils::File::IsAbsolute(log_path)) {
				log_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), log_path.c_str());
			}

			PipelineConfigure& monitor_configure = SlaveConfigure::Instance().monitor_configure_.pipeline_configure_;
			if (!MonitorAgent::Initialize(monitor_configure.send_address_, monitor_configure.recv_address_, log_path))
				break;

			bret = true;
		} while (false);
		return bret;
	}

	bool MonitorSlave::Exit() {
		return MonitorAgent::Exit();
	}
}