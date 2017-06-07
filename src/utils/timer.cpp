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

#include "timer.h"

namespace utils {
	TimerElement::TimerElement(int64_t id, int64_t data, int64_t expire_time, std::function<void(int64_t)> const &func) :
		id_(id), data_(data), expire_time_(expire_time), func_(func) {}

	int64_t TimerElement::GetIndex() {
		return id_;
	}

	void TimerElement::Excute() {
		func_(data_);
	}

	Timer::Timer() :check_interval_(100 * utils::MICRO_UNITS_PER_MILLI),
		global_element_id_(1),
		last_check_time_(0) {}

	Timer::~Timer() {}

	bool Timer::Initialize() {
		return true;
	}

	bool Timer::Exit() {
		return true;
	}

	int64_t Timer::AddTimer(int64_t micro_time, int64_t data, std::function<void(int64_t)> const &func) {
		utils::MutexGuard guard(lock_);
		int64_t expire_time = utils::Timestamp::HighResolution() + micro_time;
		TimerElement element(global_element_id_++, data, expire_time, func);

		time_ele_.insert(std::make_pair(expire_time, element));
		return element.GetIndex();
	}

	bool Timer::DelTimer(int64_t index) {
		utils::MutexGuard guard(lock_);
		for (std::multimap<int64_t, TimerElement>::iterator iter = time_ele_.begin();
			iter != time_ele_.end();
			iter++) {
			TimerElement &ele = iter->second;
			if (ele.GetIndex() == index) {
				time_ele_.erase(iter);
				return true;
			}
		}

		return false;;
	}

	void Timer::OnTimer(int64_t current_time) {
		if (current_time > last_check_time_ + check_interval_) {
			CheckExpire(current_time);

			for (std::list<TimerElement>::iterator iter = exeute_list_.begin(); iter != exeute_list_.end(); iter++) {
				iter->Excute();
			}
			exeute_list_.clear();

			last_check_time_ = current_time;
		}
	}

	void Timer::CheckExpire(int64_t cur_time) {
		utils::MutexGuard guard(lock_);
		for (std::multimap<int64_t, TimerElement>::iterator iter = time_ele_.begin();
			iter != time_ele_.end();
			) {
			TimerElement ele = iter->second;
			if (iter->first > cur_time) {
				break;
			}

			time_ele_.erase(iter++);
			exeute_list_.push_back(ele);
		}
	}

}
