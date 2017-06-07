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

#ifndef UTILS_SINGLETION_H_
#define UTILS_SINGLETION_H_
#include<assert.h>

#ifndef NULL
#define  NULL 0
#endif

namespace utils {
	template<class T>
	class Singleton {
	private:
		static T *instance_;
	protected:
		Singleton() {}
		virtual ~Singleton() {}
	public:
		static bool InitInstance() {
			if (NULL != instance_) return false;
			instance_ = new T;
			return true;
		}

		template<class CLS_TYPE, class ARG_TYPE> static bool InitInstance(ARG_TYPE nArg) {
			if (NULL != instance_) return false;
			instance_ = new CLS_TYPE(nArg);
			return true;
		}

		static bool ExitInstance() {
			if (NULL == instance_) return false;
			delete instance_;
			instance_ = NULL;
			return true;
		}

		inline static T *GetInstance() {
			return instance_;
		}

		inline static T &Instance() {
			assert(NULL != instance_);
			return *instance_;
		}

		template<class CLS_TYPE> inline static CLS_TYPE *GetSubInstance() {
			if (NULL == instance_) {
				return NULL;
			}
			return dynamic_cast<CLS_TYPE *>(instance_);
		}

		template<class CLS_TYPE> inline static CLS_TYPE &SubInstance() {
			assert(NULL != instance_);
			return *dynamic_cast<CLS_TYPE *>(instance_);
		}
	};
	template<class T> T *utils::Singleton<T>::instance_ = NULL;
} // namespace utils
#endif // _UTILS_SINGLETION_H_