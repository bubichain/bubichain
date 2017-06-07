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

#ifndef UTILS_THREAD_H_
#define UTILS_THREAD_H_

#include "utils.h"

namespace utils {
typedef std::function<void()> ThreadCallback;
	class Thread;
	class Runnable {
	public:
		Runnable() {}
		virtual ~Runnable() {}
		virtual void Run(Thread *this_thread) = 0;
	};
	//thread 
	class Thread {
	public:
		enum StateT { kInit, kStart, kJoined, kStop };
		explicit Thread();
		explicit Thread(Runnable *target);
		~Thread();
		//stop thread, return true if succeed
		bool Stop();
		//force to terminate thread
		bool Terminate();
		bool Start(std::string name = "");
		//stop and waiting for thead stopping
		bool JoinWithStop();
		bool enabled() const;
		size_t thread_id() const;
		bool IsRunning() const;
		//get current thread id
		static size_t current_thread_id();
		bool IsObjectValid() const;
		static bool SetCurrentThreadName(std::string name);
		const std::string &GetName() const;
	protected:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(Thread);
		std::string name_;
#ifdef WIN32
		static DWORD WINAPI threadProc(LPVOID param);
#else
		static void *threadProc(void *param);
#endif
		StateT state_;
		volatile bool enabled_;
		Runnable *target_;
#ifdef WIN32
		HANDLE handle_;
		static const HANDLE INVALID_HANDLE;
#else
		pthread_t handle_;
		static const pthread_t INVALID_HANDLE;
#endif
		size_t thread_id_;
	protected:
		virtual void Run();
	};
	//thread group
	class ThreadGroup {
	public:
		ThreadGroup();
		~ThreadGroup();
		void AddThread(Thread *thread);
		void StartAll();
		void JoinAll();
		void StopAll();
		size_t size() const;
	private:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(ThreadGroup);
		std::vector<Thread *> threads_;
	};
	class Mutex {
	public:
		Mutex();
		~Mutex();
		void Lock();
		void Unlock();
		pthread_mutex_t *mutex_pointer();
	private:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(Mutex);
		uint32_t thread_id_;
		pthread_mutex_t mutex_;
	};
	class MutexGuard {
	public:
		MutexGuard(Mutex &mutex);
			~MutexGuard();
	private:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(MutexGuard);
		Mutex &mutex_;
	};
	class ReadWriteLock {
	public:
		ReadWriteLock();
		~ReadWriteLock();
		void ReadLock();
		void ReadUnlock();
		void WriteLock();
		void WriteUnlock();
	private:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(ReadWriteLock);
		volatile long reads_;
		Mutex enterLock_;
	};
	class ReadLockGuard {
	public:
		ReadLockGuard(ReadWriteLock &lock);
		~ReadLockGuard();
	private:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(ReadLockGuard);
		ReadWriteLock &lock_;
	};
	class WriteLockGuard {
	public:
		WriteLockGuard(ReadWriteLock &lock);
		~WriteLockGuard();
	private:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(WriteLockGuard);
		ReadWriteLock &lock_;
	};
	// implement a spin lock using lock free.
	class SpinLock {
	public:
		/** ctor */
		SpinLock();
		/** dtor */
		virtual	~SpinLock();
		// lock
		inline void Lock();
		// unlock
		inline void Unlock();
	private:
		SpinLock(const SpinLock&);
		SpinLock& operator = (const SpinLock&);
		volatile uint32_t m_busy;
		static const int SPINLOCK_FREE = 0;
		static const int SPINLOCK_BUSY = 1;
	};
#define ReadLockGuard(x) error "Missing guard object name"
#define WriteLockGuard(x) error "Missing guard object name"
	class Semaphore {
	public:
#ifdef WIN32
		static const uint32_t kInfinite = INFINITE;
		typedef HANDLE sem_t;
#else
		static const uint32_t kInfinite = UINT_MAX;
#endif
		Semaphore(int32_t num = 0);
		~Semaphore();
		// P
		bool Wait(uint32_t millisecond = kInfinite);
		// V
		bool Signal();
	private:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(Semaphore);
		sem_t sem_;
	};
	class ThreadTaskQueue {
	public:
		ThreadTaskQueue();
		~ThreadTaskQueue();
		int PutFront(Runnable *task);
		int Put(Runnable *task);
		int Size();
		Runnable *Get();
	private:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(ThreadTaskQueue);
		typedef std::list<Runnable *> Tasks;
		Tasks tasks_;
		SpinLock spinLock_;
		Semaphore sem_;
	};
	class ThreadPool : public Runnable {
	public:
		ThreadPool();
		~ThreadPool();
		bool Init(int threadNum = kDefaultThreadNum);
		bool Exit();
		//add task
		void AddTask(Runnable *task);
		void JoinwWithStop();
		/// wait all tasks has been 
		bool WaitAndJoin();
		// get thread's size
		size_t Size()const;
		//terminate
		void Terminate();
	private:
		UTILS_DISALLOW_EVIL_CONSTRUCTORS(ThreadPool);
		typedef std::vector<Thread *> ThreadVector;
		//add worker
		void AddWorker(int threadNum);
		void Run(Thread *this_thread);
		ThreadVector threads_;
		ThreadTaskQueue tasks_;
		bool enabled_;
		static const int32_t kDefaultThreadNum = 10;
	};
}

#endif // _UTILS_THREAD_H_
