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

#ifndef _MEMORY_MANAGER_H
#define _MEMORY_MANAGER_H

#include <qcc/Mutex.h>
#include <qcc/Thread.h>

#include "common.h"

namespace ajn{
namespace services{

class MemoryManager{
public:
	static MemoryManager* Instance();
	void DestroyInstance();

	/**
     *  Get a task from the list of task by task.
	 *  @remark Not thread safe.
	 *  @return task
     */
	Task* PopTask();
	
	/**
     *  Push the task into the end of the task list
     */
	void PushTask(Task*);

	/**
     *  Erase a device from the list of device by deviceID
	 *  @pama  deviceID the identification of a device
	 *  @return Device 
     */ 
	Device* PopDevice(const char* deviceID);

	/**
     *  Delete the device from the device list
	 *  @pama  device container of a device
     */ 
	void ReleaseDevice(Device* device);

	/**
     *  Add a device to the device list 
	 *  @pama  device container of a device
	 *  @return status 
     */ 
	QStatus PushDevice(Device* device);

	/**
     *  Find a device from the list of device by deviceID
	 *  @pama  deviceID the identification of a device
	 *  @return Device 
     */ 
	Device* FindDevice(const char* deviceID);

	/**
     *  Find a ProxyObject from the list of ProxyObject by deviceID and objectPath
	 *  @pama  deviceID  the identification of a device
	 *  @pama  objectPath  the path of a bus object
	 *  @return ProxyBusObject 
     */ 
	ajn::ProxyBusObject* FindProxyObject(const char* deviceID, const char* objectPath);

	/**
     *  Find a InterFaceName from the list of InterFaceName by deviceID and objectPath
	 *  @pama  deviceID  the identification of a device
	 *  @pama  objectPath  the path of a bus object
	 *  @return InterFaceName 
     */ 
	const char* FindInterfanceName(const char* deviceID, const char* objectPath);

	/**
     *  The Heart Beat Manager
     */
	void HeartBeatManager();

	/**
     *  Get a free task from the list of free task 
	 *  @pama  taskType  the type of the task
	 *  @return task
     */
	Task* PopFreeTask(TaskType taskType);

	/**
     *  Put a free task into the list of free task
	 *  @pama  task container of a any task
     */
	void PushFreeTask(Task* task);

	/*
	TaskRegister* PopFreeTaskRegister();
	void PushFreeTaskRegister(TaskRegister* task);

	TaskRegister* PopFreeTaskUnRegister();
	void PushFreeTaskUnRegister(TaskUnRegister* task);

	TaskExecute* PopFreeTaskExecute();
	void PushFreeTaskExecute(TaskExecute* task);
	*/
private:
	/**
     * MemoryManager Constructor
     */
	MemoryManager();

	/**
     * Destruct MemoryManager
     */
	virtual ~MemoryManager();	

	/**
     * Create an instance of MemoryManager
     */
	static MemoryManager* m_instance;

	/**
     * List of tasks 
     */
	ajn::services::TaskList   m_taskList;

	/**
     * List of free tasks 
     */
	ajn::services::TaskList   m_taskFreeList;

	/**
     * List of mutex tasks
     */
	qcc::Mutex m_mutexTaskList;

	/**
     * List of devices
     */
	MapDevice m_deviceList;
};

} // namespace
} // namespace

#endif
