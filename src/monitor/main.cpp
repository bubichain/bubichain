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

#include <common/configure.h>
#include <common/general.h>
#include <common/daemon.h>
#include <utils/logger.h>
#include <utils/file.h>
#include <utils/timestamp.h>
#include <utils/strings.h>
#include <monitor/monitor_manager.h>
#include <monitor/monitor.h>

#define MONITORAGENT_VERSION "2.0.0.0"

bool g_enable_ = true;
bool Command(int argc, char* argv[]);
void RunLoop();
void InstallSignal();
void SignalFunc(int32_t code);

int main(int argc, char *argv[]) {
	// command response
	if (Command(argc, argv)) {
		return 0;
	}

	// initial
	utils::Daemon::InitInstance();
	bubi::MonitorConfigure::InitInstance();
	bubi::Monitor::InitInstance();
	bubi::MonitorManager::InitInstance();
	bubi::SlowTimer::InitInstance();
	utils::Logger::InitInstance();

	do {
		utils::BubiAtExit bubiAtExit;
		InstallSignal();

		utils::Daemon &daemon = utils::Daemon::Instance();
		if (!g_enable_ && daemon.Initialize((int32_t)1235)) {
			LOG_STD_ERRNO("Initialize daemon failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		bubiAtExit.Push(std::bind(&utils::Daemon::Exit, &daemon));

		bubi::MonitorConfigure &config = bubi::MonitorConfigure::Instance();
		std::string config_path = bubi::General::MONITOR_CONFIG_FILE;
		if (!utils::File::IsAbsolute(config_path)) {
			config_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), config_path.c_str());
		}
		if (!config.Load(config_path)) {
			LOG_STD_ERRNO("Load agent configure failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}

		std::string log_path = config.logger_configure_.path_;
		if (!utils::File::IsAbsolute(log_path)) {
			log_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), log_path.c_str());
		}

		bubi::LoggerConfigure logger_config = bubi::MonitorConfigure::Instance().logger_configure_;
		utils::Logger &logger = utils::Logger::Instance();
		logger.SetCapacity(logger_config.time_capacity_, logger_config.size_capacity_);
		logger.SetExpireDays(logger_config.expire_days_);
		if (!logger.Initialize((utils::LogDest)logger_config.dest_, (utils::LogLevel)logger_config.level_, log_path, true)) {
			LOG_STD_ERR("Initialize logger failed");
			break;
		}
		bubiAtExit.Push(std::bind(&utils::Logger::Exit, &logger));
		LOG_INFO("Initialize logger successful");

		bubi::Monitor& monitor = bubi::Monitor::Instance();
		if (!monitor.Initialize()) {
			LOG_ERROR_ERRNO("Initialize Monitor failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::Monitor::Exit, &monitor));
		LOG_INFO("Initialize Monitor successful");


		bubi::MonitorManager& monitor_manager = bubi::MonitorManager::Instance();
		if (!monitor_manager.Initialize()) {
			LOG_ERROR_ERRNO("Initialize MonitorManager failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::MonitorManager::Exit, &monitor_manager));
		LOG_INFO("Initialize MonitorManager successful");

		bubi::SlowTimer &slow_timer = bubi::SlowTimer::Instance();
		if (!slow_timer.Initialize(1)) {
			LOG_ERROR_ERRNO("Initialize slow timer failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::SlowTimer::Exit, &slow_timer));
		LOG_INFO("Initialize slow timer with " FMT_SIZE " successful", utils::GetCpuCoreCount());

		RunLoop();

		LOG_INFO("Process begin quit...");
	} while (false);

	bubi::SlowTimer::ExitInstance();
	bubi::MonitorManager::ExitInstance();
	bubi::Monitor::ExitInstance();
	bubi::MonitorConfigure::ExitInstance();
	utils::Logger::ExitInstance();
	utils::Daemon::ExitInstance();

	return 0;
}

void RunLoop() {
	int64_t check_module_interval = utils::MICRO_UNITS_PER_SEC;
	int64_t last_check_module = 0;
	while (g_enable_) {
		int64_t current_time = utils::Timestamp::HighResolution();
		for (auto item : bubi::TimerNotify::notifys_) {
			item->TimerWrapper(current_time);
		}
		utils::Logger::Instance().CheckExpiredLog();
		utils::Sleep(100);
	}
}

void SignalFunc(int32_t code) {
	LOG_INFO("Get quit signal(%d)", code);
	g_enable_ = false;
}

bool Command(int argc, char* argv[]) {
	bool bret = false;
	if (argc > 1) {
		bret = true;
		if (0 == strcmp(argv[1], "version") || 0 == strcmp(argv[1], "-V")) {
			printf("%s\n", MONITORAGENT_VERSION);
		}
		else {
			printf("%s -- parameter is invalid\n", argv[1]);
		}
	}
	return bret;
}

void InstallSignal() {
	signal(SIGHUP, SignalFunc);
	signal(SIGQUIT, SignalFunc);
	signal(SIGINT, SignalFunc);
	signal(SIGTERM, SignalFunc);
#ifndef WIN32
	signal(SIGPIPE, SIG_IGN);
#endif
}