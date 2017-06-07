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

#include "daemon.h"
#include <utils/timestamp.h>
#include <utils/logger.h>

namespace utils {
	Daemon::Daemon() {
		last_write_time_ = 0;
		timer_name_ = "Daemon";
	}

	Daemon::~Daemon() {}

	bool Daemon::Initialize(int32_t key) {
		bubi::TimerNotify::RegisterModule(this);
#ifdef WIN32

#else
		int fd;
		pthread_mutexattr_t mattr;
		fd = open("/dev/zero", O_RDWR, 0);
		mptr_ = (pthread_mutex_t*)mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		close(fd);
		pthread_mutexattr_init(&mattr);
		pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(mptr_, &mattr);

		shmid_ = shmget((key_t)key, sizeof(int64_t), 0666 | IPC_CREAT);
		if (shmid_ == -1) {
			LOG_ERROR("shmget failed");
			return false;
		}
		
		shm_ = shmat(shmid_, (void*)0, 0);
		if (shm_ == (void*)-1) {
			LOG_ERROR("shmat failed\n");
			return false;
		}
		LOG_INFO("Memory attached at %lx\n", (unsigned long int)shm_);
		shared_ = (int64_t*)shm_;

#endif
		return true;
	}

	bool Daemon::Exit() {
#ifdef WIN32
#else
		if (shmdt(shm_) == -1) {
			LOG_ERROR("shmdt failed");
			return false;
		}
		return true;
#endif
		return true;
	}

	void Daemon::OnTimer(int64_t current_time) {
		
#ifdef WIN32
#else
		if (current_time - last_write_time_ > 500000) {
			pthread_mutex_lock(mptr_);
			*shared_ = current_time;
			last_write_time_ = current_time;
			pthread_mutex_unlock(mptr_);
		}
#endif
	}

	void Daemon::OnSlowTimer(int64_t current_time) {

	}

	void Daemon::GetModuleStatus(Json::Value &data) {
	}
}