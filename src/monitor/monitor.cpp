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

#include "monitor.h"
#include <common/configure.h>
#include <utils/logger.h>

namespace bubi {
	const char* MONITOR_NAME = "monitor";

	Monitor::Monitor() : monitor_master_(PipelineServer::ROUND_ROBIN), monitor_slave_(PipelineServer::BROADCAST) {}

	Monitor::~Monitor() {}

	bool Monitor::Initialize() {
		bool bret = false;
		do {
			if (!monitor_master_.Initialize()) {
				break;
			}

			if (!monitor_slave_.Initialize()) {
				break;
			}
			bret = true;
		} while (false);
		return bret;
	}

	bool Monitor::Exit() {
		return monitor_master_.Exit() && monitor_slave_.Exit();
	}

	Monitor::MonitorServer::MonitorServer(PipelineType type) : PipelineServer(type) {}

	Monitor::MonitorServer::~MonitorServer() {}

	bool Monitor::MonitorServer::Initialize() {
		bool bret = false;
		do {
			std::string push_address;
			std::string pull_address;

			switch (pipe_type_) {
			case ROUND_ROBIN:
			{
				PipelineConfigure  &pipeline_configure = MonitorConfigure::Instance().bubi_pipeline_configure_;
				push_address = pipeline_configure.send_address_;
				pull_address = pipeline_configure.recv_address_;
				break;
			}
			case BROADCAST:
			{
				PipelineConfigure  &pipeline_configure = MonitorConfigure::Instance().slave_pipeline_configure_;
				push_address = pipeline_configure.send_address_;
				pull_address = pipeline_configure.recv_address_;
				break;
			}
			}

			if (!PipelineServer::Initialize(MONITOR_NAME, push_address, pull_address, 1)) {
				break;
			}

			bret = true;
		} while (false);

		return bret;
	}

	bool Monitor::MonitorServer::Exit() {
		return PipelineServer::Exit();
	}
}