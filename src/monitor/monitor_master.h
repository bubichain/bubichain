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

/************************************************************************/
/* Author: FengRuiMing                                                  */
/* Date: 2016.3.28                                                      */
/* Function: connect and communication width thd agent server           */
/************************************************************************/
#ifndef MONITOR_MASTER_H_
#define MONITOR_MASTER_H_

#include "monitoragent.h"
#include <utils/singleton.h>

namespace bubi {
	class MonitorMaster : public MonitorAgent, public utils::Singleton <MonitorMaster> {
	public:
		MonitorMaster();
		~MonitorMaster();
		bool Initialize();
		bool Exit();
	private:
		void on_bubi_monitor(const char* msg, int len, std::string &reply);
		void on_ledger_monitor(const char* msg, int len, std::string &reply);
		void on_alert_monitor(const char* msg, int len, std::string &reply);
		void on_account_exception_monitor(const char* msg, int len, std::string &reply);
	private:
		std::string peer_id_;
	};
}

#endif
