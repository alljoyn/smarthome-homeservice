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

#include "MemoryManager.h"

namespace ajn{
namespace services{

MemoryManager* MemoryManager::m_instance = NULL;

MemoryManager* MemoryManager::Instance()
{
	if (!m_instance) {
		m_instance = new MemoryManager();
	}

	return m_instance;
}

void MemoryManager::DestroyInstance()
{
	if (m_instance){
		delete m_instance;
		m_instance = NULL;
	}
}

Task* MemoryManager::PopTask()
{
	Task* task = NULL;
	
	m_mutexTaskList.Lock();
	TaskList::iterator iter = m_taskList.begin();
	if(iter != m_taskList.end()){
		task = *iter;
		m_taskList.pop_front();
	}
	m_mutexTaskList.Unlock();

	return task;
}

void MemoryManager::PushTask(Task* task)
{
	if (!task){
		return;
	}

	m_mutexTaskList.Lock();
	if (m_taskList.size() >= MAX_TASK_LIST_LENGTH){
		while(m_taskList.size() >= MAX_TASK_LIST_LENGTH){
			m_mutexTaskList.Unlock();
			qcc::Sleep(10);
			m_mutexTaskList.Lock();
		}
	}
	m_taskList.push_back(task);
	m_mutexTaskList.Unlock();
}

Device* MemoryManager::PopDevice(const char* deviceID)
{
	if (!deviceID){
		return NULL;
	}

	Device* device = NULL;

	MapDevice::iterator iter = m_deviceList.find(deviceID);
	if (iter != m_deviceList.end()){
		device = iter->second;
		m_deviceList.erase(iter);
	}

	return device;
}

void MemoryManager::ReleaseDevice(Device* device)
{
	if(!device)
	{
		return;
	}

	if (device){
		MapProxyObject::iterator iter4 = device->proxyObjectList.begin();
		while (iter4 != device->proxyObjectList.end()){
			delete iter4->second;
			++iter4;
		}
		device->proxyObjectList.clear();

		MapInerfaceName::iterator iter5 = device->interFaceNameList.begin();
		while (iter5 != device->interFaceNameList.end()){
			delete iter5->first;
			delete iter5->second;
			++iter5;
		}
		device->interFaceNameList.clear();
	}
	delete device;
}

QStatus MemoryManager::PushDevice(Device* device)
{
	if (!device){
		return ER_FAIL;
	}

	QStatus status = ER_OK;

	MapDevice::iterator iter = m_deviceList.find(device->deviceID);
	if (iter == m_deviceList.end()){
		m_deviceList.insert(std::pair<const char*, Device*>(device->deviceID, device));
	}

	return status;
}

Device* MemoryManager::FindDevice(const char* deviceID)
{
	if (!deviceID){
		return NULL;
	}

	MapDevice::iterator iter = m_deviceList.find(deviceID);
	if (iter != m_deviceList.end()){
		return iter->second;
	}

	return NULL;
}

ajn::ProxyBusObject* MemoryManager::FindProxyObject(const char* deviceID, const char* objectPath)
{
	if (!deviceID || !objectPath){
		return NULL;
	}

	MapDevice::iterator iter = m_deviceList.find(deviceID);
	if (iter != m_deviceList.end()){
		MapProxyObject::iterator iter2 = iter->second->proxyObjectList.find(objectPath);
		if (iter2 != iter->second->proxyObjectList.end()){
			return iter2->second;
		}
	}

	return NULL;
}

const char* MemoryManager::FindInterfanceName(const char* deviceID, const char* objectPath)
{
	if (!deviceID || !objectPath){
		return NULL;
	}

	MapDevice::iterator iter = m_deviceList.find(deviceID);
	if (iter != m_deviceList.end()){
		MapInerfaceName::iterator iter2 = iter->second->interFaceNameList.find(objectPath);
		if (iter2 != iter->second->interFaceNameList.end()){
			return iter2->second;
		}
	}

	return NULL;
}

void MemoryManager::HeartBeatManager()
{
	Device *device;
	MapDevice::iterator iter = m_deviceList.begin();
	char* missedDeviceID[MAX_ARRAY_LEN] = {NULL};
	int missedDeviceIDLen = 0;
	
	while (iter != m_deviceList.end()){
		const char * deviceID = iter->first;
		device = iter->second;
		if (device){
			if((*device).heartCount==0){
				missedDeviceID[missedDeviceIDLen++] = const_cast<char*>(deviceID);
				printf("The device named %s is missing.\n",missedDeviceID[missedDeviceIDLen-1]);
				
			}else {
				(*device).heartCount--;
			}
		}
		++iter;
	}

	Device* missedDevice = NULL;
	for (int i=0; i<missedDeviceIDLen; ++i)
	{
		missedDevice = PopDevice(missedDeviceID[i]);
		if(missedDevice)
		{
			ReleaseDevice(missedDevice);
			missedDevice = NULL;
		}
	}		
}

Task* MemoryManager::PopFreeTask(TaskType taskType)
{
	Task* task = NULL;

	TaskList::iterator iter = m_taskFreeList.begin();
	while(iter != m_taskFreeList.end()){		
		if ((*iter)->taskType == taskType){
			task = *iter;
			break;
		}
		++iter;
	}

	if (!task){
		task = new Task();
		task->taskType = taskType;
		if (taskType == TASK_TYPE_OF_REGISTER){
			task->task = static_cast<void*>(new TaskRegister());
		}else if (taskType == TASK_TYPE_OF_UNREGISTER){
			task->task = static_cast<void*>(new TaskUnRegister);
		}else if (taskType == TASK_TYPE_OF_HEARTBEAT){
			task->task = static_cast<void*>(new TaskHeartBeat());
		}else if (taskType == TASK_TYPE_OF_EXECUTE){
			task->task = static_cast<void*>(new TaskExecute());
		}
	}

	return task;
}

void MemoryManager::PushFreeTask(Task* task)
{
	if (!task){
		return;
	}

	if (m_taskFreeList.size() >= MAX_FREE_TASK_LIST_LENGTH){
		if (task->taskType == TASK_TYPE_OF_REGISTER){
			delete static_cast<TaskRegister*>(task->task);			
		}else if (task->taskType == TASK_TYPE_OF_UNREGISTER){
			delete static_cast<TaskUnRegister*>(task->task);			
		}else if (task->taskType == TASK_TYPE_OF_HEARTBEAT){
			delete static_cast<TaskHeartBeat*>(task->task);			
		}else if (task->taskType == TASK_TYPE_OF_EXECUTE){
			delete static_cast<TaskExecute*>(task->task);			
		}
		task->task = NULL;

		delete task;
	}else{
		m_taskFreeList.push_back(task);
	}
}

MemoryManager::MemoryManager()
{

}

MemoryManager::~MemoryManager()
{
	Task* task = NULL;
	
	m_mutexTaskList.Lock();
	TaskList::iterator iter1 = m_taskList.begin();
	while (iter1 != m_taskList.end()){
		task = *iter1;
		if (task){
			if (task->taskType == TASK_TYPE_OF_REGISTER){
				delete static_cast<TaskRegister*>(task->task);			
			}else if (task->taskType == TASK_TYPE_OF_EXECUTE){
				delete static_cast<TaskExecute*>(task->task);			
			}else if (task->taskType == TASK_TYPE_OF_HEARTBEAT){
				delete static_cast<TaskHeartBeat*>(task->task);			
			}else if (task->taskType == TASK_TYPE_OF_UNREGISTER){
				delete static_cast<TaskUnRegister*>(task->task);			
			}
			task->task = NULL;

			delete task;
		}

		++iter1;
	}
	m_taskList.clear();
	m_mutexTaskList.Unlock();

	TaskList::iterator iter2 = m_taskFreeList.begin();
	while (iter2 != m_taskFreeList.end()){
		task = *iter2;
		if (task){
			if (task->taskType == TASK_TYPE_OF_REGISTER){
				delete static_cast<TaskRegister*>(task->task);			
			}else if (task->taskType == TASK_TYPE_OF_UNREGISTER){
				delete static_cast<TaskUnRegister*>(task->task);			
			}else if (task->taskType == TASK_TYPE_OF_HEARTBEAT){
				delete static_cast<TaskHeartBeat*>(task->task);			
			}else if (task->taskType == TASK_TYPE_OF_EXECUTE){
				delete static_cast<TaskExecute*>(task->task);			
			}
			task->task = NULL;

			delete task;
		}

		++iter2;
	}
	m_taskFreeList.clear();

	Device* device = NULL;
	MapDevice::iterator iter3 = m_deviceList.begin();
	while (iter3 != m_deviceList.end()){
		device = iter3->second;
		if (device){
			MapProxyObject::iterator iter4 = device->proxyObjectList.begin();
			while (iter4 != device->proxyObjectList.end()){
				//delete iter4->first; // it is not time to delete.
				delete iter4->second;
				++iter4;
			}
			device->proxyObjectList.clear();

			MapInerfaceName::iterator iter5 = device->interFaceNameList.begin();
			while (iter5 != device->interFaceNameList.end()){
				delete iter5->first; // it is time to delete.
				delete iter5->second;
				++iter5;
			}
			device->interFaceNameList.clear();
		}
		delete device;
		++iter3;
	}
	m_deviceList.clear();
}

} //namespace
} //namespace
