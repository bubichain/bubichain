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
#include <common/general.h>
#include <common/configure.h>
#include <common/storage.h>
#include <common/private_key.h>
#include <overlay/peer_manager.h>
#include <ledger/ledger_manager.h>
#include <consensus/consensus_manager.h>
#include <glue/glue_manager.h>
#include <monitor/monitor_master.h>
#include <api/web_server.h>
#include <common/daemon.h>
#include "slave/master_service.h"
#include  <api/mq_server.h>
#include  <api/websocket_server.h>

bool g_enable_ = true;
void RunLoop();
void InstallSignal();
void SignalFunc(int32_t code);
void Usage();
int main(int argc, char *argv[]) {
	utils::Thread::SetCurrentThreadName("bubi-thread");

	utils::Daemon::InitInstance();
	utils::net::Initialize();
	utils::Timer::InitInstance();
	bubi::Configure::InitInstance();
	bubi::MasterService::InitInstance();
	bubi::Storage::InitInstance();
	bubi::Global::InitInstance();
	bubi::SlowTimer::InitInstance();
	utils::Logger::InitInstance();
	bubi::PeerManager::InitInstance();
	bubi::LedgerManager::InitInstance();
	bubi::ConsensusManager::InitInstance();
	bubi::GlueManager::InitInstance();
	bubi::MQServer::InitInstance();
	bubi::WebSocketServer::InitInstance();
	bubi::WebServer::InitInstance();
	bubi::MonitorMaster::InitInstance();

	bool bdropdb = false;
	bool bupdate = false;
	bool peer_addr = false;
	bool bdel_peer_addr = false;
	bool bclear_consensus_status = false;
	for (int i = 1; i < argc; i++) {
		std::string s(argv[i]);
		if (s == "--dropdb") {
			//create database
			bdropdb = true;
		}
		else if (s == "--update") {
			bupdate = true;
		}
		else if (s == "--peer-address") {
			peer_addr = true;
		}
		else if (s == "--del-peer-address") {
			bdel_peer_addr = true;
		}
		else if (s == "--clear-consensus-status") {
			bclear_consensus_status = true;
		}
		else if (s == "--version") {
			printf("2.0.0.0\n");
#ifdef SVNVERSION
			printf("2.0.0.0; " SVNVERSION "\n");
#endif 
			return 0;
		}
		else if (s == "--help") {
			Usage();
			return 0;
		}
		else {
			Usage();
			return 0;
		}
	}

	do {
		utils::BubiAtExit bubiAtExit;
		InstallSignal();

		srand((uint32_t)time(NULL));
		bubi::StatusModule::modules_status_ = new Json::Value;
		utils::Daemon &daemon = utils::Daemon::Instance();
		if (!g_enable_ || !daemon.Initialize((int32_t)1234)) {
			LOG_STD_ERRNO("Initialize daemon failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		bubiAtExit.Push(std::bind(&utils::Daemon::Exit, &daemon));
		LOG_INFO("Initialize daemon successful");

		bubi::Configure &config = bubi::Configure::Instance();
		std::string config_path = bubi::General::CONFIG_FILE;
		if (!utils::File::IsAbsolute(config_path)) {
			config_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), config_path.c_str());
		}
		if (!config.Load(config_path)) {
			LOG_STD_ERRNO("Load configure failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		LOG_INFO("Load configure successful");

		std::string log_path = config.logger_configure_.path_;
		if (!utils::File::IsAbsolute(log_path)) {
			log_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), log_path.c_str());
		}
		bubi::LoggerConfigure logger_config = bubi::Configure::Instance().logger_configure_;
		utils::Logger &logger = utils::Logger::Instance();
		logger.SetCapacity(logger_config.time_capacity_, logger_config.size_capacity_);
		logger.SetExpireDays(logger_config.expire_days_);
		if (!g_enable_ || !logger.Initialize((utils::LogDest)logger_config.dest_, (utils::LogLevel)logger_config.level_, log_path, true)) {
			LOG_STD_ERR("Initialize logger failed");
			break;
		}
		bubiAtExit.Push(std::bind(&utils::Logger::Exit, &logger));
		LOG_INFO("Initialize logger successful");

		bubi::Storage &storage = bubi::Storage::Instance();
		LOG_INFO("%s,%s => %s", config.db_configure_.keyvalue_db_path_.c_str(), config.db_configure_.rational_db_type_.c_str(), config.db_configure_.soci_connection_.c_str());

		if (!g_enable_ || !storage.Initialize(config.db_configure_.keyvalue_db_path_, config.db_configure_.soci_connection_, config.db_configure_.rational_db_type_, bdropdb)) {
			LOG_ERROR("Initialize db failed");
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::Storage::Exit, &storage));

		if (bclear_consensus_status) {
			bubi::Pbft::ClearStatus();
			LOG_INFO("Clear consensus status successfully");
			break;
		}

		if (bdel_peer_addr) {
			bubi::KeyValueDb *db = storage.keyvalue_db();
			std::string key = utils::String::Format("%s_nodeprivkey", bubi::General::OVERLAY_PREFIX);
			bool ret = db->Delete(key);
			LOG_INFO("Delete peer node address %s", ret ? "successfully" : "failed");
			break;
		}

		if (peer_addr) {
			bubi::KeyValueDb *db = storage.keyvalue_db();
			std::string key = utils::String::Format("%s_nodeprivkey", bubi::General::OVERLAY_PREFIX);
			std::string name;
			bubi::PrivateKey priv_key(bubi::ED25519SIG);
			if (db->Get(key, name) && priv_key.From(name)) {
				LOG_INFO(" peer node address (%s)", priv_key.GetBase58Address().c_str());
			}
			else {
				LOG_INFO(" peer node address not exist");
			}
			break;
		}

		if (bdropdb)
			break;
		LOG_INFO("Initialize db successful");

		bubi::Global &global = bubi::Global::Instance();
		if (!g_enable_ || !global.Initialize()) {
			LOG_ERROR_ERRNO("Initialize global variable failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::Global::Exit, &global));
		LOG_INFO("Initialize global variable successful");

		bubi::MonitorMaster& monitor_master = bubi::MonitorMaster::Instance();
		if (!g_enable_ || !monitor_master.Initialize()) {
			LOG_ERROR("Initialize monitor failed");
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::MonitorMaster::Exit, &monitor_master));
		LOG_INFO("Initialize monitor successful");

		bubi::LedgerManager &ledgermanger = bubi::LedgerManager::Instance();
		if (!g_enable_ || !ledgermanger.Initialize()) {
			LOG_ERROR("Initialize ledger manager failed");
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::LedgerManager::Exit, &ledgermanger));
		LOG_INFO("Initialize ledger successful");

		bubi::ConsensusManager &consensus_manager = bubi::ConsensusManager::Instance();
		if (!g_enable_ || !consensus_manager.Initialize(bubi::Configure::Instance().validation_configure_)) {
			LOG_ERROR("Initialize consensus manager failed");
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::ConsensusManager::Exit, &consensus_manager));
		LOG_INFO("Initialize consensus manager successful");

		bubi::GlueManager &glue = bubi::GlueManager::Instance();
		if (!g_enable_ || !glue.Initialize()) {
			LOG_ERROR("Initialize glue manager failed");
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::GlueManager::Exit, &glue));
		LOG_INFO("Initialize glue manager successful");

		bubi::MasterService &master_service = bubi::MasterService::Instance();
		if (!g_enable_ || !master_service.Initialize()) {
			LOG_ERROR_ERRNO("Initialize Master Service failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}

		bubiAtExit.Push(std::bind(&bubi::MasterService::Exit, &master_service));
		LOG_INFO("Initialize Master Service successful");

		bubi::PeerManager &p2p = bubi::PeerManager::Instance();
		if (!g_enable_ || !p2p.Initialize()) {
			LOG_ERROR("Initialize peer network failed");
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::PeerManager::Exit, &p2p));
		LOG_INFO("Initialize peer network successful");

		bubi::SlowTimer &slow_timer = bubi::SlowTimer::Instance();
		if (!g_enable_ || !slow_timer.Initialize(1)) {
			LOG_ERROR_ERRNO("Initialize slow timer failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::SlowTimer::Exit, &slow_timer));
		LOG_INFO("Initialize slow timer with " FMT_SIZE " successful", utils::GetCpuCoreCount());

		bubi::MQServer &mq_server = bubi::MQServer::Instance();
		if (!g_enable_ || !mq_server.Initialize(bubi::Configure::Instance().mqserver_configure_)) {
			LOG_ERROR_ERRNO("Initialize mq server failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::MQServer::Exit, &mq_server));
		LOG_INFO("Initialize mq server successful");


		bubi::WebSocketServer &ws_server = bubi::WebSocketServer::Instance();
		if (!g_enable_ || !ws_server.Initialize(bubi::Configure::Instance().wsserver_configure_)) {
			LOG_ERROR_ERRNO("Initialize mq server failed", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::WebSocketServer::Exit, &ws_server));
		LOG_INFO("Initialize websocket server successful");

		bubi::WebServer &web_server = bubi::WebServer::Instance();
		if (!g_enable_ || !web_server.Initialize(bubi::Configure::Instance().webserver_configure_)) {
			LOG_ERROR("Initialize web server failed");
			break;
		}
		bubiAtExit.Push(std::bind(&bubi::WebServer::Exit, &web_server));
		LOG_INFO("Initialize web server successful");

		RunLoop();

		LOG_INFO("Process begin quit...");
		delete bubi::StatusModule::modules_status_;

	} while (false);

	bubi::MasterService::ExitInstance();
	bubi::SlowTimer::ExitInstance();
	bubi::MonitorMaster::ExitInstance();
	bubi::GlueManager::ExitInstance();
	bubi::LedgerManager::ExitInstance();
	bubi::PeerManager::ExitInstance();
	bubi::WebServer::ExitInstance();
	bubi::MQServer::ExitInstance();
	bubi::Configure::ExitInstance();
	bubi::Global::ExitInstance();
	bubi::Storage::ExitInstance();
	utils::Logger::ExitInstance();
	utils::Daemon::ExitInstance();
	printf("process exit\n");
}

void RunLoop() {
	int64_t check_module_interval = utils::MICRO_UNITS_PER_SEC;
	int64_t last_check_module = 0;
	while (g_enable_) {
		int64_t current_time = utils::Timestamp::HighResolution();

		for (auto item : bubi::TimerNotify::notifys_) {
			item->TimerWrapper(utils::Timestamp::HighResolution());
			if (item->IsExpire(utils::MICRO_UNITS_PER_SEC)) {
				LOG_WARN("The timer(%s) execute time(" FMT_I64 " us) is expire than 1s", item->GetTimerName().c_str(), item->GetLastExecuteTime());
			}
		}

		utils::Timer::Instance().OnTimer(current_time);
		utils::Logger::Instance().CheckExpiredLog();

		if (current_time - last_check_module > check_module_interval) {
			utils::WriteLockGuard guard(bubi::StatusModule::status_lock_);
			bubi::StatusModule::GetModulesStatus(*bubi::StatusModule::modules_status_);
			last_check_module = current_time;
		}

		utils::Sleep(1);
	}
}

void SignalFunc(int32_t code) {
	fprintf(stderr, "Get quit signal(%d)\n", code);
	g_enable_ = false;
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

void Usage() {
	printf(
		"Usage: bubi [OPTIONS]\n"
		"OPTIONS:\n"
		"  --dropdb                        clean up database\n"
		"  --update                        sync data from remote peers\n"
		"  --del-peer-address              delete peer address\n"
		"  --peer-address                  get local peer address\n"
		"  --clear-consensus-status        delete consensus status\n"
		"  --version                       display version information\n"
		"  --help                          display this help\n"
		);
}
