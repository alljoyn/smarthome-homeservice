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

#ifndef _THREAD_SERVICE_H
#define _THREAD_SERVICE_H

#include <list>

#include <qcc/Mutex.h>
#include <alljoyn/MsgArg.h>

#include "common.h"

namespace ajn {
namespace services {


class ThreadServiceImpl;
class HeartThreadServiceImpl;

class ThreadServiceListener{
public:

	/**
     * ThreadServiceListener Constructor
     */
	ThreadServiceListener(){}

	/**
     * Destruct ThreadServiceListener
     */
	virtual ~ThreadServiceListener(){}

	/**
     * Call the corresponding handle task function according to different task type t
	 * @param[in]  task  Container of a any task
     */
	virtual void HandleTask(ajn::services::Task*){}
};

class ThreadService{
friend class ThreadServiceImpl;

public:
	
	/**
     * List of threads
     */
	typedef std::list<ThreadServiceImpl*> ThreadList;

	/**
     * List of heart threads
     */
	typedef std::list<HeartThreadServiceImpl*> HeartThreadList;

	/**
     * @brief Initialize the ThreadService singleton.
     */
	static void RegisterListener(ThreadServiceListener* threadServiceListener);

	/**
     * @brief Return a reference to the ThreadService singleton.
     */
	static ThreadService& Instance();

	 /**
     * @brief Start any required name service threads.
     */
    QStatus Start();

     /**
     * @brief Stop any central threads.
     */
    QStatus Stop();

    /**
     * @brief Join any central threads.
     */
    QStatus Join();

	/**
     * @brief Get a free task from the list of free task .
     */
	Task* PopFreeTask(TaskType taskType);

	 /**
     * @brief Put a task into the list of task.
     */
	void QueueTask(Task* task);	

	/**
     * @brief Recycle a task from the list of task.
     */
	void RecycleTask(Task* task);

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
	const char* FindInterfaceName(const char* deviceID, const char* objectPath);

	/**
     *  The Heart Beat Manager
     */
	void HeartBeatManager();

private:

	/**
     * ThreadService Constructor
     */
	ThreadService();

	/**
     * Destruct ThreadService
     */
	~ThreadService();

	/**
     *  Handle task
     */
	void PostTask(Task* task);

	/**
     * @brief Get a task from the list of task by task.
	 * @remark Not thread safe.
     */
	Task* PopTask();	

	
	/**
     *  Create a list of thread
     */
	ThreadList m_threadList;

	
	/**
     *  Create a list of heart list
     */
	HeartThreadList m_heartThreadList;

	
	/**
     *  Thread mutex
     */
	qcc::Mutex m_mutex;

	/**
     *  Create a ThreadServiceListener
     */
	static ThreadServiceListener* m_taskCallBack;
};

} //namespace
} //namespace

#endif
