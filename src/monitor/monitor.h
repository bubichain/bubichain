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

#ifndef MONITOR_H_
#define MONITOR_H_

#include <common/pipeline_server.h>
#include <utils/singleton.h>

namespace bubi {
	class Monitor : public utils::Singleton<Monitor> {
		friend class ProcessMessage;
		friend class Alert;
		class MonitorServer : public PipelineServer {
			friend class ProcessMessage;
		public:
			MonitorServer(PipelineType type);
			~MonitorServer();
			bool Initialize();
			bool Exit();
		};
	public:
		Monitor();
		~Monitor();
		bool Initialize();
		bool Exit();
	private:
		MonitorServer monitor_master_;
		MonitorServer monitor_slave_;
	};
}

#endif
