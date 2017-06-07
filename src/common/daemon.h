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

#ifndef  DAEMON_H
#define  DAEMON_H
#include <utils/singleton.h>
#include <common/general.h>
#ifdef WIN32
#else 
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include<sys/mman.h>
#endif
#define TEXT_SZ 2048

namespace utils {
	class Daemon : public utils::Singleton<utils::Daemon>, public bubi::TimerNotify {
		friend class utils::Singleton<utils::Daemon>;
	private:
		Daemon();
		~Daemon();
	public:
		bool Initialize(int32_t key);
		bool Exit();
		void OnTimer(int64_t current_time);
		virtual void OnSlowTimer(int64_t current_time);
		void GetModuleStatus(Json::Value &data);
	private:
		pthread_mutex_t *mptr_;
		int64_t last_write_time_;
		int running_;
		void *shm_;
		int64_t* shared_;
		int shmid_;
	};
}

#endif

