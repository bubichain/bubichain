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

#ifndef WIN32
#include <sys/prctl.h>
#endif

#include "thread.h"

#ifdef WIN32
const HANDLE utils::Thread::INVALID_HANDLE = NULL;
#else
const pthread_t utils::Thread::INVALID_HANDLE = (pthread_t)-1;
#endif

#ifdef WIN32
DWORD WINAPI utils::Thread::threadProc(LPVOID param)
#else
void *utils::Thread::threadProc(void *param)
#endif
{
	Thread *this_thread = reinterpret_cast<Thread *>(param);

	this_thread->Run();
	this_thread->thread_id_ = 0;

#ifdef WIN32
	CloseHandle(this_thread->handle_);
#endif // WIN32

#ifdef WIN32
	_endthreadex(0);
	return 0;
#else
	return NULL;
#endif
}

#undef ReadLockGuard
#undef WriteLockGuard

bool utils::Thread::Start(std::string name) {
	name_ = name;
	if (kInit != state_) {
		return false;
	}

	bool result = false;
#ifdef WIN32
	handle_ = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadProc, (LPVOID)this, 0, (LPDWORD)&thread_id_);
	result = (NULL != handle_);
#else
	int ret = pthread_create(&handle_, NULL, threadProc, (void *)this);
	result = (0 == ret);
	thread_id_ = handle_;
#endif
	state_ = kStart;
	enabled_ = true;
	return result;
}

utils::Thread::Thread(){
	state_ = kInit;
	enabled_ = false;
	handle_ = INVALID_HANDLE;
	thread_id_ = 0;
}

utils::Thread::Thread(Runnable *target){
    target_ = target;
	state_ = kInit;
    enabled_ = false;
    handle_ = INVALID_HANDLE;
    thread_id_ = 0;
}

utils::Thread::~Thread() {
	state_ = kStop;
}

bool utils::Thread::enabled() const { 
	return enabled_; 
}

size_t utils::Thread::thread_id() const { 
	return thread_id_; 
}

bool utils::Thread::IsRunning() const { 
	return state_ == kStart; 
}

bool utils::Thread::IsObjectValid() const { 
	return Thread::INVALID_HANDLE != handle_; 
}

const std::string &utils::Thread::GetName() const {
	return name_; 
}

bool utils::Thread::Stop() {
	if (!IsObjectValid() || !enabled_) {
		return false;
	}

	enabled_ = false;
	return true;
}


bool utils::Thread::Terminate() {
	if (kStop == state_ || kInit == state_) {
		return true;
	}

	bool result = true;
#ifdef WIN32
	if (0 == ::TerminateThread(handle_, 0)) {
		result = false;
	}
#else
	if (0 != pthread_cancel(thread_id_)) {
		result = false;
	}
#endif
	if (result) {
		state_ = kStop;
	}

	enabled_ = false;

	return result;
}

bool utils::Thread::JoinWithStop() {
	if (kStart != state_) {
		return false;
	}

	enabled_ = false;

	bool result = false;

#ifdef WIN32
	if (INVALID_HANDLE != handle_) {
		DWORD ret = ::WaitForSingleObject(handle_, INFINITE);
		if (WAIT_OBJECT_0 == ret || WAIT_ABANDONED == ret) {
			result = true;
			handle_ = NULL;
		}
	}
#else
	if (INVALID_HANDLE != handle_) {
		int ret = pthread_join(handle_, NULL);
		if (0 == ret) {
			result = true;
			handle_ = INVALID_HANDLE;
		}
	}
#endif
	state_ = kJoined;
	return result;
}

void utils::Thread::Run() {
	assert(target_ != NULL);

	SetCurrentThreadName(name_);

	target_->Run(this);
}

bool utils::Thread::SetCurrentThreadName(std::string name) {
#ifdef WIN32
	//not supported
	return true;
#else
	return 0 == prctl(PR_SET_NAME, name.c_str(), 0, 0, 0);
#endif //WIN32
}

size_t utils::Thread::current_thread_id() {
#ifdef WIN32
	return (size_t)::GetCurrentThreadId();
#else
	return (size_t)pthread_self();
#endif
}

utils::ThreadGroup::ThreadGroup() {}

utils::ThreadGroup::~ThreadGroup() {
	JoinAll();
	for (size_t i = 0; i < threads_.size(); ++i) {
		delete threads_[i];
	}
	threads_.clear();
}

void utils::ThreadGroup::AddThread(Thread *thread) {
	threads_.push_back(thread);
}

void utils::ThreadGroup::StartAll() {
	for (size_t i = 0; i < threads_.size(); ++i) {
		threads_[i]->Start();
	}
}

void utils::ThreadGroup::JoinAll() {
	for (size_t i = 0; i < threads_.size(); ++i) {
		threads_[i]->JoinWithStop();
	}
}

void utils::ThreadGroup::StopAll() {
	for (size_t i = 0; i < threads_.size(); ++i) {
		threads_[i]->Stop();
	}
}

size_t utils::ThreadGroup::size() const {
	return threads_.size();
}

utils::Mutex::Mutex()
	: thread_id_(0) {
#ifdef WIN32
	InitializeCriticalSection(&mutex_);
#else
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex_, &mattr);
	pthread_mutexattr_destroy(&mattr);
#endif
}

utils::Mutex::~Mutex() {
#ifdef WIN32
	DeleteCriticalSection(&mutex_);
#else
	pthread_mutex_destroy(&mutex_);
#endif
}

void utils::Mutex::Lock() {
#ifdef WIN32
	EnterCriticalSection(&mutex_);
#ifdef _DEBUG
	thread_id_ = static_cast<uint32_t>(GetCurrentThreadId());
#endif
#else
	pthread_mutex_lock(&mutex_);
#ifdef _DEBUG
	thread_id_ = static_cast<uint32_t>(pthread_self());
#endif
#endif
}

void utils::Mutex::Unlock() {
#ifdef _DEBUG
	thread_id_ = 0;
#endif
#ifdef WIN32
	LeaveCriticalSection(&mutex_);
#else
	pthread_mutex_unlock(&mutex_);
#endif
}

pthread_mutex_t *utils::Mutex::mutex_pointer() { 
	return &mutex_;
}

utils::MutexGuard::MutexGuard(Mutex &mutex):mutex_(mutex) {
	mutex_.Lock();
}

utils::MutexGuard::~MutexGuard() {
	mutex_.Unlock();
}

utils::ReadWriteLock::ReadWriteLock()
	: reads_(0) {}

utils::ReadWriteLock::~ReadWriteLock() {}

void utils::ReadWriteLock::ReadLock() {
	enterLock_.Lock();
	AtomicInc(&reads_);
	enterLock_.Unlock();
}

void utils::ReadWriteLock::ReadUnlock() {
	AtomicDec(&reads_);
}

void utils::ReadWriteLock::WriteLock() {
	enterLock_.Lock();
	while (reads_ > 0) {
		Sleep(0);
	}
}

utils::WriteLockGuard::WriteLockGuard(ReadWriteLock &lock): lock_(lock) {
	lock_.WriteLock();
}

utils::WriteLockGuard::~WriteLockGuard() { 
	lock_.WriteUnlock(); 
}

utils::ReadLockGuard::ReadLockGuard(ReadWriteLock &lock): lock_(lock) {
	lock_.ReadLock();
}

utils::ReadLockGuard::~ReadLockGuard() { 
	lock_.ReadUnlock();
}

void utils::ReadWriteLock::WriteUnlock() {
	enterLock_.Unlock();
}

utils::SpinLock::SpinLock() :m_busy(SPINLOCK_FREE) {
}

utils::SpinLock::~SpinLock() {
}

void utils::SpinLock::Lock() {
	while (SPINLOCK_BUSY == BUBI_CAS(&m_busy, SPINLOCK_BUSY, SPINLOCK_FREE)) {
		BUBI_YIELD();
	}
}

void utils::SpinLock::Unlock() {
	BUBI_CAS(&m_busy, SPINLOCK_FREE, SPINLOCK_BUSY);
}

size_t utils::ThreadPool::Size() const {
	return threads_.size(); 
}

utils::Semaphore::Semaphore(int32_t num) {
#ifdef _WIN32
	sem_ = ::CreateSemaphore(NULL, num, LONG_MAX, NULL);
#else
	sem_init(&sem_, 0, num);
#endif    
}

utils::Semaphore::~Semaphore() {
#ifdef _WIN32
	if (NULL != sem_) {
		if (0 != ::CloseHandle(sem_)) {
			sem_ = NULL;
		}
	}
#else
	sem_destroy(&sem_);
#endif    
}

bool utils::Semaphore::Wait(uint32_t millisecond) {
#ifdef _WIN32
	if (NULL == sem_)
		return false;

	DWORD ret = ::WaitForSingleObject(sem_, millisecond);
	if (WAIT_OBJECT_0 == ret || WAIT_ABANDONED == ret) {
		return true;
	}
	else {
		return false;
	}
#else
	int32_t ret = 0;

	if (kInfinite == millisecond) {
		ret = sem_wait(&sem_);
	}
	else {
		struct timespec ts = { 0, 0 };
		//TimeUtil::getAbsTimespec(&ts, millisecond);

		ts.tv_sec = millisecond / 1000;
		ts.tv_nsec = millisecond % 1000;

		ret = sem_timedwait(&sem_, &ts);
	}

	return -1 != ret;
#endif
}

bool utils::Semaphore::Signal() {
#ifdef _WIN32
	BOOL ret = FALSE;

	if (NULL != sem_) {
		ret = ::ReleaseSemaphore(sem_, 1, NULL);
	}
	return TRUE == ret;
#else
	return -1 != sem_post(&sem_);
#endif
}

utils::ThreadTaskQueue::ThreadTaskQueue() {}

utils::ThreadTaskQueue::~ThreadTaskQueue() {}

int utils::ThreadTaskQueue::PutFront(Runnable *task) {
	int ret = 0;
	spinLock_.Lock();
	if (task) tasks_.push_front(task);
	ret = tasks_.size();
	spinLock_.Unlock();
	return ret;
}

int utils::ThreadTaskQueue::Put(Runnable *task) {
	int ret = 0;
	spinLock_.Lock();
	if (task) tasks_.push_back(task);
	ret = tasks_.size();
	spinLock_.Unlock();
	return ret;
}


int utils::ThreadTaskQueue::Size() {
	int ret = 0;
	spinLock_.Lock();
	ret = tasks_.size();
	spinLock_.Unlock();
	return ret;
};

utils::Runnable *utils::ThreadTaskQueue::Get() {
	Runnable *task = NULL;
	spinLock_.Lock();
	if (tasks_.size() > 0) {
		task = tasks_.front();
		tasks_.pop_front();
	}
	spinLock_.Unlock();
	return task;
}

utils::ThreadPool::ThreadPool() : enabled_(false) {}

utils::ThreadPool::~ThreadPool() {
	for (size_t i = 0; i < threads_.size(); i++) {
		if (threads_[i]) delete threads_[i];
	}
}

bool utils::ThreadPool::Init(int threadNum) {

	enabled_ = true;
	AddWorker(threadNum);
	return enabled_;
}

bool utils::ThreadPool::Exit() {
	enabled_ = false;
	for (size_t i = 0; i < threads_.size(); i++) {
		if (threads_[i]) threads_[i]->JoinWithStop();
	}

	return true;
}

void utils::ThreadPool::AddTask(Runnable *task) {
	tasks_.Put(task);
}

void utils::ThreadPool::JoinwWithStop() {
	enabled_ = false;
	for (ThreadVector::const_iterator it = threads_.begin(); it != threads_.end(); ++it) {
		(*it)->JoinWithStop();
	}
	threads_.clear();
}

bool utils::ThreadPool::WaitAndJoin() {

	while (tasks_.Size() > 0)
		Sleep(1);

	enabled_ = false;
	for (size_t i = 0; i < threads_.size(); i++) {
		if (threads_[i]) threads_[i]->JoinWithStop();
	}

	return true;
}

void utils::ThreadPool::Terminate() {
	for (ThreadVector::const_iterator it = threads_.begin(); it != threads_.end(); ++it) {
		(*it)->Terminate();
	}
}

void utils::ThreadPool::AddWorker(int threadNum) {
	for (int i = 0; i < threadNum; ++i) {

		Thread *thread = new Thread(this);
		threads_.push_back(thread);
		thread->Start();
	}
}

void utils::ThreadPool::Run(Thread *this_thread) {
	while (enabled_) {
		utils::Runnable *task = tasks_.Get();
		if (task) task->Run(this_thread);
		else utils::Sleep(1);
	}
}

