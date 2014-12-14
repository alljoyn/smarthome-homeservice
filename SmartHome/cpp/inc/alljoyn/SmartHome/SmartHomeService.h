#ifndef _SMART_HOME_SERVICE_H
#define _SMART_HOME_SERVICE_H

#include <alljoyn/BusAttachment.h>
#include <alljoyn/Session.h>

#include "ThreadService.h"
const char CCH[] = "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
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
     * Handles the GetPropery request
     * @param[in]  ifcName  interface name
     * @param[in]  propName the name of the properly
     * @param[in]  val reference of MsgArg out parameter.
     * @return ER_OK if successful.
     */
    QStatus Get(const char* ifcName, const char* propName, MsgArg& val);

	const char* GenerateRandomString(char *device_randomString);
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
     * Handles Verification method
	 * @param[in]  taskFindDevice  Container of a task of Verification
     */
	QStatus HandleTaskVerification(const TaskVerification & taskVerification);

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
     * Handles ReturnValueVerification signal
	 * As long as there is a Verification call, there will be a ReturnValueVerification signal 
	 * @param[in]  device  the structure of the device named device ID
	 * @return status.
     */
	QStatus ReturnValueVerification(Device *device);

	/**
     * Handles DeviceHeartBeat method
	 * one part of heart beat function, the other part is 
	 * @param[in]  taskHeartBeat  Container of a task of heart beat
     */
	void HandleTaskHeartBeat(const TaskHeartBeat & taskHeartBeat);

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
     *  Parse the args to the Verification task and push the task into the end of the task list
	 *  @pama  member 
	 *  @pama  msg reference of AllJoyn Message
     */  
	void Verification(const InterfaceDescription::Member* member, Message& msg);

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
     *  Erase a device from the list of device by deviceId
	 *  @pama  deviceId the identification of a device
	 *  @return Device 
     */ 
	Device* PopDevice(const char* deviceId);

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
     *  Find a device from the list of device by deviceId
	 *  @pama  deviceId the identification of a device
	 *  @return Device 
     */ 
	Device* FindDevice(const char* deviceId);

	/**
     *  Find a ProxyObject from the list of ProxyObject by deviceId and objectPath
	 *  @pama  deviceId  the identification of a device
	 *  @pama  objectPath  the path of a bus object
	 *  @return ProxyBusObject 
     */ 
	ajn::ProxyBusObject* FindProxyObject(const char* deviceId, const char* objectPath);

	/**
     *  Find a InterFaceName from the list of InterFaceName by deviceId and objectPath
	 *  @pama  deviceId  the identification of a device
	 *  @pama  objectPath  the path of a bus object
	 *  @return InterFaceName 
     */ 
	const char* FindInterFaceName(const char* deviceId, const char* objectPath);

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
	const ajn::InterfaceDescription::Member * m_ReturnValueVerificationSignalMember;
};

} // namespace
} // namespace

#endif