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

#ifndef UTILS_TIMER_H_
#define UTILS_TIMER_H_

#include "singleton.h"
#include "thread.h"
#include "utils.h"
#include "timestamp.h"

namespace utils {
	class TimerElement {
	public:
		TimerElement(int64_t id_, int64_t data_, int64_t expire_time_, std::function<void(int64_t)> const &func);
		~TimerElement() {};
		int64_t GetIndex();
		void Excute();
	private:
		std::function<void(int64_t)> func_;
		int64_t id_;
		int64_t data_;
		int64_t expire_time_;
	};
	class Timer : public Singleton<Timer> {
		friend class Singleton<Timer>;
	public:
		Timer();
		virtual ~Timer();
	public:
		bool Initialize();
		bool Exit();
		void OnTimer(int64_t current_time);
		int64_t AddTimer(int64_t micro_time, int64_t data, std::function<void(int64_t)> const &func); /* msec unit: millisecond (1/1000);*/
		bool DelTimer(int64_t index);
		void CheckExpire(int64_t cur_time);
	private:
		std::multimap<int64_t, TimerElement> time_ele_;
		std::list<TimerElement> exeute_list_;
		utils::Mutex lock_;
		int64_t global_element_id_;
		int64_t check_interval_;
		int64_t last_check_time_;
	};
}
#endif 