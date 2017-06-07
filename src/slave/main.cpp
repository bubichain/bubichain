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

#include <utils/logger.h>
#include <utils/net.h>
#include <utils/file.h>
#include <utils/timestamp.h>
#include <common/general.h>
#include <common/configure.h>
#include <common/private_key.h>
#include <overlay/peer_manager.h>
#include <monitor/monitor_slave.h>
#include <common/daemon.h>
#include "api/web_server.h"
#include "slave_executor.h"
#include "slave_service.h"


bool g_enable_ = true;
void RunLoop();
void InstallSignal();
void SignalFunc(int32_t code);
int main(int argc, char *argv[]){
	utils::Daemon::InitInstance();

	utils::net::Initialize();
	bubi::SlaveConfigure::InitInstance();
	utils::Logger::InitInstance();
	bubi::WebServer::InitInstance();
	bubi::SlaveService::InitInstance();
	bubi::MonitorSlave::InitInstance();


    do {
        utils::BubiAtExit bubiAtExit;
        InstallSignal();

		srand((uint32_t)time(NULL));
		bubi::StatusModule::modules_status_ = new Json::Value;
		
		bubi::SlaveConfigure &config = bubi::SlaveConfigure::Instance();
		std::string config_path = bubi::General::SLAVE_CONFIG_FILE;
		if (!utils::File::IsAbsolute(config_path)){
			config_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), config_path.c_str());
		}
		if (!config.Load(config_path)){
			LOG_STD_ERRNO("Load slave configure failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
        }
        LOG_INFO("Load configure successful");

		std::string log_path = config.logger_configure_.path_;
		if (!utils::File::IsAbsolute(log_path)){
			log_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), log_path.c_str());
		}

		bubi::LoggerConfigure logger_config = bubi::SlaveConfigure::Instance().logger_configure_;
		utils::Logger &logger = utils::Logger::Instance();
		logger.SetCapacity(logger_config.time_capacity_, logger_config.size_capacity_);
		logger.SetExpireDays(logger_config.expire_days_);
        if (!g_enable_ || !logger.Initialize((utils::LogDest)logger_config.dest_, (utils::LogLevel)logger_config.level_, log_path, true)){
			LOG_STD_ERR("Initialize logger failed");
			break;
        }
        bubiAtExit.Push(std::bind(&utils::Logger::Exit, &logger));

		LOG_INFO("Initialize logger successful");
        utils::Daemon &daemon = utils::Daemon::Instance();
        if (!g_enable_ || !daemon.Initialize((int32_t)1235))
        {
            LOG_STD_ERRNO("Initialize daemon failed", STD_ERR_CODE, STD_ERR_DESC);
            break;
        }
        bubiAtExit.Push(std::bind(&utils::Daemon::Exit, &daemon));
        LOG_INFO("Initialize daemon successful");

		bubi::MonitorSlave& monitor_slave = bubi::MonitorSlave::Instance();
		if (!g_enable_ || !monitor_slave.Initialize()) {
			LOG_ERROR("Initialize monitor slave failed");
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::MonitorSlave::Exit, &monitor_slave));
		LOG_INFO("Initialize monitor slave successful");
        
		bubi::SlaveService &slave_service = bubi::SlaveService::Instance();
        if (!g_enable_ || !slave_service.Initialize(bubi::SlaveConfigure::Instance().bubi_configure_))
		{
			LOG_ERROR_ERRNO("Initialize Slave Service failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
        bubiAtExit.Push(std::bind(&bubi::SlaveService::Exit, &slave_service));
		LOG_INFO("Initialize Slave Service successful");
        
        bubi::WebServer &web_server = bubi::WebServer::Instance();
		if (!g_enable_ || !web_server.Initialize(bubi::SlaveConfigure::Instance().webserver_configure_)) {
			LOG_ERROR("Initialize web server failed");
			break;
        }
        bubiAtExit.Push(std::bind(&bubi::WebServer::Exit, &web_server));
		LOG_INFO("Initialize web server successful");

		RunLoop();

        LOG_INFO("Process begin quit...");
		delete bubi::StatusModule::modules_status_;

	} while ( false );

    bubi::WebServer::ExitInstance();
    bubi::SlaveService::ExitInstance();
    bubi::SlaveConfigure::ExitInstance();
    bubi::MonitorSlave::ExitInstance();
	utils::Logger::ExitInstance();
    utils::Daemon::ExitInstance();
    printf("process exit\n");
}

void RunLoop(){
	int64_t check_module_interval = utils::MICRO_UNITS_PER_SEC;
	int64_t last_check_module = 0;
	while (g_enable_){
		int64_t current_time = utils::Timestamp::HighResolution();
		for (auto item : bubi::TimerNotify::notifys_){
			item->TimerWrapper(current_time);
		}

		utils::Logger::Instance().CheckExpiredLog();

		if (current_time - last_check_module > check_module_interval){
			utils::WriteLockGuard guard(bubi::StatusModule::status_lock_);
			bubi::StatusModule::GetModulesStatus(*bubi::StatusModule::modules_status_);
			last_check_module = current_time;
		}

		utils::Sleep(100);
	}
}

void SignalFunc(int32_t code){
	fprintf(stderr, "Get quit signal(%d)\n", code);
	g_enable_ = false;
}

void InstallSignal(){
	signal(SIGHUP, SignalFunc);
	signal(SIGQUIT, SignalFunc);
	signal(SIGINT, SignalFunc);
	signal(SIGTERM, SignalFunc);
#ifndef WIN32
	signal(SIGPIPE, SIG_IGN);
#endif
}

