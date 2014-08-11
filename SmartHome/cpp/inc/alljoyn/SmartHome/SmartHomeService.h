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

#ifndef _SMART_HOME_SERVICE_H
#define _SMART_HOME_SERVICE_H

#include <alljoyn/BusAttachment.h>
#include <alljoyn/Session.h>

#include "ThreadService.h"

namespace ajn {
namespace services {

class SmartHomeService : public ajn::BusObject, public  ThreadServiceListener{
public:

	 /**
     * Name of the interface
     */
	static const char* INTERFACE_NAME;  

	/**
     * The path of 'BusObject'
     */
	static const char* OBJECT_PATH;	  

	/**
     * SmartHomeService Constructor
     * @param  bus reference to BusAttachment
     */
	SmartHomeService(ajn::BusAttachment& bus);

	/**
     * Destruct SmartHomeService
     */
	virtual ~SmartHomeService();

	/**
     * Call the corresponding handle task function according to different task type t
	 * @param[in]  task  Container of a any task
     */
	virtual void HandleTask(Task*task);    

private:
	
	/**
     * Handles ApplianceRegistration method
	 * @param[in]  taskRegister  Container of a task of register
	 * @return status.
     */
	QStatus HandleTaskRegister(const TaskRegister & taskRegister);

	/**
     * Handles ApplianceUnRegistration method
	 * @param[in]  taskUnRegister  Container of a task of unregister
     */
	void HandleTaskUnRegister(const TaskUnRegister & taskUnRegister);

	/**
     * Handles Execute method
	 * @param[in]  taskExecute  Container of a task of Execute
	 * @return status.
     */
	QStatus HandleTaskExecute(const TaskExecute & taskExecute);

	/**
     * Handles ReturnValue signal
	 * As long as there is a method call, there will be a ReturnValue signal 
	 * @param[in]  reply  reference of AllJoyn Message from the callee
	 * @param[in]  senderDevice  the information about the caller
	 * @param[in]  taskExecute  container of a task of Execute
	 * @param[in]  status of the method call to the callee
	 * @return status.
     */
	QStatus ReturnValue(Message& reply, Device *senderDevice,const TaskExecute & taskExecute,QStatus status);

	/**
     * Handles DeviceHeartBeat method
	 * one part of heart beat function, the other part is 
	 * @param[in]  taskHeartBeat  Container of a task of heart beat
     */
	void HandleTaskHeartBeat(const TaskHeartBeat & taskHeartBeat);

	/**
     * Fill the device information to the device container
	 * The information contains interface name, objectPath, method name, method args, ProxyBusObject 
	 * @param[in]  interfaceArgs  reference of MsgArg out parameter
	 * @param[in]  device container of device
	 * @return status.
     */
	QStatus FillDevice(const ajn::MsgArg interfaceArgs, Device* device);

	/**
     *  Create interfaces
     */ 
	QStatus CreateInterfaces();

	/**
     *  Parse the args to the Registration task and push the task into the end of the task list
	 *  @pama  member 
	 *  @pama  msg reference of AllJoyn Message
     */ 
	void ApplianceRegistration(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);

	/**
     *  Parse the args to the UnRegistration task and push the task into the end of the task list
	 *  @pama  member 
	 *  @pama  msg reference of AllJoyn Message
     */ 
	void ApplianceUnRegistration(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);

	/**
     *  Parse the args to the DeviceHeartBeat task and push the task into the end of the task list
	 *  @pama  member 
	 *  @pama  msg reference of AllJoyn Message
     */  
	void DeviceHeartBeat(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);

	/**
     *  Parse the args to the Execute task and push the task into the end of the task list
	 *  @pama  member 
	 *  @pama  msg reference of AllJoyn Message
     */  
	void Execute(const InterfaceDescription::Member* member, Message& msg);

	/**
     *  Push a any task into the end of the task list
	 *  @pama  task container of a any task
     */ 
	void QueueTask(Task* task);

	/**
     *  Get a free task 
	 *  @pama  taskType the type of the task
	 *  @return Task 
     */ 
	Task* GetFreeTask(TaskType taskType);

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
	const char* FindInterFaceName(const char* deviceID, const char* objectPath);

	/**
     *  The Heart Beat Manager
     */ 
	void HeartBeatManager();
	
	/**
     *  Parse the args to some task by task type
	 *  @pama  args  reference of MsgArg  parameter
	 *  @pama  argsNum  the number of the args
	 *  @pama  taskType  the type of the task
	 *  @return task 
     */ 
	Task* ArgsParse(const ajn::MsgArg* args, size_t argsNum, TaskType taskType);

	 /**
     *  pointer to BusAttachment
     */
    ajn::BusAttachment* m_BusAttachment;

	/**
     *  stores the signal member initialized  in the CreateInterfaces(..)
     */
    const ajn::InterfaceDescription::Member * m_ReturnValueSignalMember;
};

} // namespace
} // namespace

#endif
