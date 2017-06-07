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

#pragma once
#include <common/general.h>
#include <utils/singleton.h>
#include <utils/thread.h>
#include <string>
#include <vector>
#include "common/zmq_helper.h"

namespace bubi {
	
	class SlaveWorker : 
		public utils::Runnable
	{
		friend class SlaveService;
	public:
		SlaveWorker();
		~SlaveWorker();
		SlaveWorker(const SlaveWorker& s) = delete;
        virtual void Run(utils::Thread *thread) override;
	private:
		void* m_pushSocket_;
		void* m_pullSocket_;
		int m_recvBufferSize_;
		char* m_recvBuffer_;
	};

	class SlaveService :
		public utils::Singleton<bubi::SlaveService>
	{
		friend class utils::Singleton<bubi::SlaveService>;
        friend class SlaveWorker;
	public:
		SlaveService();
		~SlaveService();
		bool Initialize(PipelineConfigure &config);
		bool Exit();
		void AddHandler(const ZMQTaskType type, DynamicHandler callback);
		void AddHandler(const ZMQTaskType type, StaticHandler callback);
        bool HandleTask(const ZMQTaskType type, std::string& buf);
		bool SendToMaster(const ZMQTaskType type, std::string& buf);

	private:
		utils::ReadWriteLock m_pushersMapRWLock_;
        std::vector<utils::Thread *> worker_thread_ptrs_;
		std::map<size_t, void*> m_pushersMap_;
		StaticHandler m_staticHandlers_[256];
		DynamicHandler m_dynamicHandlers_[256];
		std::string m_pullAddress_;
		std::string m_pushAddress_;
        void* m_context_;
        SlaveWorker* m_worker_;
	};
}

