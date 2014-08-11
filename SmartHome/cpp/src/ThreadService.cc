/******************************************************************************
 *    Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include "ThreadService.h"

#include "ThreadServiceImpl.h"
#include "HeartThreadServiceImpl.h"
#include "MemoryManager.h"
#include <qcc/Mutex.h>
#include <qcc/Thread.h>

namespace ajn {
namespace services {

ThreadServiceListener* ThreadService::m_taskCallBack = NULL;

void ThreadService::RegisterListener(ThreadServiceListener* threadServiceListener)
{
	m_taskCallBack = threadServiceListener;
}

ThreadService& ThreadService::Instance()
{
	static ThreadService threadService;
	return threadService;
}

QStatus ThreadService::Start()
{
	QStatus status = ER_OK;

	m_mutex.Lock();
	ThreadList::iterator iter = m_threadList.begin();
	while (iter != m_threadList.end()){
		if (*iter){
			//FIXME: Were all threads started with success?
			status = (*iter)->Start(this);			
		}

		++iter;
	}

	HeartThreadList::iterator iter2 = m_heartThreadList.begin();
	while (iter2 != m_heartThreadList.end()){
		if (*iter2){
			//FIXME: Were all threads started with success?
			status = (*iter2)->Start(this);			
		}

		++iter2;
	}
	m_mutex.Unlock();

	return status;
}

QStatus ThreadService::Stop()
{
	QStatus status = ER_OK;

	m_mutex.Lock();
	ThreadList::iterator iter = m_threadList.begin();
	while (iter != m_threadList.end()){
		if (*iter){
			//FIXME: Were all threads stopped with success?
			status = (*iter)->Stop();			
		}

		++iter;
	}

	HeartThreadList::iterator iter2 = m_heartThreadList.begin();
	while (iter2 != m_heartThreadList.end()){
		if (*iter2){
			//FIXME: Were all threads stopped with success?
			status = (*iter2)->Stop();			
		}

		++iter2;
	}
	m_mutex.Unlock();


	return status;
}

QStatus ThreadService::Join()
{
	QStatus status = ER_OK;

	m_mutex.Lock();
	ThreadList::iterator iter = m_threadList.begin();
	while (iter != m_threadList.end()){
		if (*iter){
			//FIXME: Were all threads joined with success?
			status = (*iter)->Join();			
		}

		++iter;
	}

	HeartThreadList::iterator iter2 = m_heartThreadList.begin();
	while (iter2 != m_heartThreadList.end()){
		if (*iter2){
			//FIXME: Were all threads joined with success?
			status = (*iter2)->Join();			
		}

		++iter2;
	}
	m_mutex.Unlock();

	return status;
}

Task* ThreadService::PopFreeTask(TaskType taskType)
{
	m_mutex.Lock();
	Task* task = MemoryManager::Instance()->PopFreeTask(taskType);
	m_mutex.Unlock();
	return task;
}

void ThreadService::QueueTask(Task* task)
{
	m_mutex.Lock();
	MemoryManager::Instance()->PushTask(task);
	m_mutex.Unlock();	
}

void ThreadService::RecycleTask(Task* task)
{
	if (!task){
		return;
	}

	switch (task->taskType)
	{
	case TASK_TYPE_OF_REGISTER:
		{
			TaskRegister* taskRegister = static_cast<TaskRegister*>(task->task);
			if (taskRegister && taskRegister->methodArgs){
				delete [] taskRegister->methodArgs;
				taskRegister->methodArgs = NULL;
			}
			break;
		}
	case TASK_TYPE_OF_UNREGISTER:
		{
			//TaskUnRegister* taskUnRegister = static_cast<TaskUnRegister*>(task->task);
			
			break;
		}
	case TASK_TYPE_OF_HEARTBEAT:
		{
			
			
			break;
		}
	case TASK_TYPE_OF_EXECUTE:
		{
			//TaskExecute* taskExecute = static_cast<TaskExecute*>(task->task);
			break;
		}
	}

	m_mutex.Lock();
	MemoryManager::Instance()->PushFreeTask(task);
	m_mutex.Unlock();	
}

Device* ThreadService::PopDevice(const char* deviceID)
{
	m_mutex.Lock();
	Device* device = MemoryManager::Instance()->PopDevice(deviceID);
	m_mutex.Unlock();

	return device;
}

void ThreadService::ReleaseDevice(Device* device)
{
	m_mutex.Lock();
    MemoryManager::Instance()->ReleaseDevice(device);
	m_mutex.Unlock();
}

QStatus ThreadService::PushDevice(Device* device)
{
	m_mutex.Lock();
	QStatus status = MemoryManager::Instance()->PushDevice(device);
	m_mutex.Unlock();

	return status;
}

Device* ThreadService::FindDevice(const char* deviceID)
{
	m_mutex.Lock();
	Device* device = MemoryManager::Instance()->FindDevice(deviceID);
	m_mutex.Unlock();

	return device;
}

ajn::ProxyBusObject* ThreadService::FindProxyObject(const char* deviceID, const char* objectPath)
{
	m_mutex.Lock();
	ajn::ProxyBusObject* remoteObj = MemoryManager::Instance()->FindProxyObject(deviceID, objectPath);
	m_mutex.Unlock();

	return remoteObj;
}

const char* ThreadService::FindInterfaceName(const char* deviceID, const char* objectPath)
{
	m_mutex.Lock();
	const char* ifaceName = MemoryManager::Instance()->FindInterfanceName(deviceID, objectPath);
	m_mutex.Unlock();

	return ifaceName;
}

void ThreadService::HeartBeatManager()
{
	m_mutex.Lock();
	MemoryManager::Instance()->HeartBeatManager();
	m_mutex.Unlock();
}

ThreadService::ThreadService()
{
	//FIXME: MAX_NUM_OF_THREAD = 3
	const int MAX_NUM_OF_THREAD = 3;
	for (int i=0; i<MAX_NUM_OF_THREAD; ++i){
		m_threadList.push_back(new ThreadServiceImpl());
	}

	m_heartThreadList.push_back(new HeartThreadServiceImpl());
}

ThreadService::~ThreadService()
{
	m_mutex.Lock();
	// Free m_threadList
	ThreadList::iterator iter1 = m_threadList.begin();
	while (iter1 != m_threadList.end()){
		if (*iter1){
			delete *iter1;
		}

		++iter1;
	}
	m_threadList.clear();

	//MemoryManager::Instance()->DestroyInstance();
	// Free m_heartThreadList
	HeartThreadList::iterator iter2 = m_heartThreadList.begin();
	while (iter2 != m_heartThreadList.end()){
		if (*iter2){
			delete *iter2;
		}

		++iter2;
	}
	m_heartThreadList.clear();

	MemoryManager::Instance()->DestroyInstance();
	m_mutex.Unlock();	
}

void ThreadService::PostTask(ajn::services::Task* task)
{
	if (m_taskCallBack){
		m_taskCallBack->HandleTask(task);
	}
}

Task* ThreadService::PopTask()
{
	m_mutex.Lock();
	Task* task = MemoryManager::Instance()->PopTask();
	m_mutex.Unlock();

	return task;
}

} //namespace
} //namespace
