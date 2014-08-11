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
#include "SmartHomeService.h"

#include <qcc/Debug.h>
#include <alljoyn/BusAttachment.h>
#include "MemoryManager.h"
#include "BusListenerImpl.h"
#include "SmartHomeServiceMain.h"

#define QCC_MODULE "SMART_HOME_SERVICE"

namespace ajn{
namespace services{

const char* SmartHomeService::INTERFACE_NAME = "org.alljoyn.SmartHome.CentralizedManagement";
const char* SmartHomeService::OBJECT_PATH = "/org/alljoyn/SmartHome/CentralizedManagement";

SmartHomeService::SmartHomeService(ajn::BusAttachment& bus)
:BusObject(OBJECT_PATH)
, m_BusAttachment(&bus)
, m_ReturnValueSignalMember(NULL)
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));

	QStatus status = CreateInterfaces();

	ThreadService::RegisterListener(this);
	ThreadService::Instance().Start();
}

SmartHomeService::~SmartHomeService()
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));
}

void SmartHomeService::HandleTask(Task* task)
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));

	if (task){
		switch (task->taskType)
		{
		case TASK_TYPE_OF_REGISTER:
			{
				TaskRegister* taskRegister = static_cast<TaskRegister*>(task->task);
				QStatus status = HandleTaskRegister(*taskRegister);

				break;
			}
		case TASK_TYPE_OF_UNREGISTER:
			{
				TaskUnRegister* taskUnRegister = static_cast<TaskUnRegister*>(task->task);
				 HandleTaskUnRegister(*taskUnRegister);

				break;
			}
		case TASK_TYPE_OF_HEARTBEAT:
			{
				TaskHeartBeat* taskHeartBeat = static_cast<TaskHeartBeat*>(task->task);
				 HandleTaskHeartBeat(*taskHeartBeat);

				break;
			}
		case TASK_TYPE_OF_EXECUTE:
			{
				TaskExecute* taskExecute = static_cast<TaskExecute*>(task->task);
				QStatus status = HandleTaskExecute(*taskExecute);

				break;
			}
		default :
			{
			}
		}
	}
}

QStatus SmartHomeService::HandleTaskRegister(const TaskRegister & taskRegister)
{
	QStatus status = ER_OK;

	Device* device = PopDevice(taskRegister.deviceID);
	if (!device){
		device = new Device();
		memcpy(device->wellKnownName, taskRegister.wellKnownName, sizeof(taskRegister.wellKnownName));
		memcpy(device->uniqueName, taskRegister.uniqueName, sizeof(taskRegister.uniqueName));
		memcpy(device->deviceID, taskRegister.deviceID, sizeof(taskRegister.deviceID));
	}

	const char* wellKnownName = taskRegister.wellKnownName;
	ajn::ProxyBusObject* remoteObj = NULL;
	
	for (int i=0; i<taskRegister.methodArgsNum; ++i){
		status = FillDevice(taskRegister.methodArgs[i], device);
		if (status != ER_OK){
			break;
		}
	}

	if (status == ER_OK){
		PushDevice(device);		
	}else{
		QCC_LogError(status, ("HandleTaskRegister Error:%s, %d, %d\n"
							, device->deviceID, device->proxyObjectList.size()
							,device->interFaceNameList.size()));
	}
	
	return status;
}

void SmartHomeService::HandleTaskUnRegister(const TaskUnRegister & taskUnRegister)
{

	Device* device = PopDevice(taskUnRegister.deviceID);
	if (!device){
		printf("There is no device named %s", taskUnRegister.deviceID);
	}

	if (device){
		MapProxyObject::iterator proxyObjectList = device->proxyObjectList.begin();
		while (proxyObjectList != device->proxyObjectList.end()){
			delete proxyObjectList->second;
			++proxyObjectList;
		}
		device->proxyObjectList.clear();

		MapInerfaceName::iterator interFaceNameList = device->interFaceNameList.begin();
		while (interFaceNameList != device->interFaceNameList.end()){
			delete interFaceNameList->first;
			delete interFaceNameList->second;
			++interFaceNameList;
		}
		device->interFaceNameList.clear();
	}
    delete device;	
	Device* findDevice = FindDevice(taskUnRegister.deviceID);
	if (findDevice){
	printf("Unregister the device %s failed! %d\n", taskUnRegister.deviceID);
	}else{
	printf("Unregister the device %s successful! %d\n", taskUnRegister.deviceID);
	}
}

void SmartHomeService::HandleTaskHeartBeat(const TaskHeartBeat & taskHeartBeat)
{
	const char* deviceID = taskHeartBeat.deviceID;
	Device * device = FindDevice(taskHeartBeat.deviceID);
	if (!device){
		printf("There is no device named %s ! \n", taskHeartBeat.deviceID);
	
	}else {
	    device->heartCount = 3;
	    printf("Receive heart beat from %s ! \n", taskHeartBeat.deviceID);
	}
}

QStatus SmartHomeService::HandleTaskExecute(const TaskExecute & taskExecute)
{
	Message reply(*m_BusAttachment);
	QStatus status = ER_OK;
	const char* interfaceName = FindInterFaceName(taskExecute.deviceID, taskExecute.objectPath);
	ajn::ProxyBusObject* remoteObj = FindProxyObject(taskExecute.deviceID, taskExecute.objectPath);
	Device *senderDevice = FindDevice(taskExecute.deviceID);

    if (remoteObj){
		status = remoteObj->MethodCall(interfaceName, taskExecute.methodName, taskExecute.methodArgs, 1, reply, 5000);
		status = ReturnValue(reply,senderDevice,taskExecute,status);
	}else{
		QCC_LogError(status, ("%s, %s, %s\n", taskExecute.deviceID, taskExecute.objectPath, interfaceName));
    }

	return status;
}

QStatus SmartHomeService::ReturnValue(Message& reply, Device *senderDevice,const TaskExecute & taskExecute,QStatus status)
{
	size_t argsNum = 0;
    const ajn::MsgArg * Args = NULL;
	MsgArg ReplyArgs;
    MsgArg GWReplyArgs[3];
	const char* methodName = taskExecute.methodName;
	
	reply->GetArgs(argsNum, Args);
	Args[0].Get("v",&ReplyArgs);
	GWReplyArgs[0].Set("s",methodName);
	GWReplyArgs[1].Set("s",QCC_StatusText(status));
	GWReplyArgs[2].Set("v",ReplyArgs);

    status = Signal(senderDevice->uniqueName, ID, *m_ReturnValueSignalMember,GWReplyArgs,3, 0);
	 
	return status;	
}

QStatus SmartHomeService::FillDevice(const ajn::MsgArg interfaceArgs, Device* device)
{
	QStatus status = ER_FAIL;
	if (!device){
		return status;
	}
	
	const char* wellKnownName = device->wellKnownName;
	ajn::ProxyBusObject* remoteObj = NULL;	

	const char* interfaceName = NULL;
	const char* objectPath = NULL;
	const ajn::MsgArg* args = NULL;
	status = interfaceArgs.Get("(sov)", &interfaceName, &objectPath, &args);
	if (status != ER_OK){
		return status;
	}

	int objPathLen = strlen(objectPath);
	int ifaceNameLen = strlen(interfaceName);
	char* objPath = new char[objPathLen+1];
	char* ifaceName = new char[ifaceNameLen+1];
	memcpy(objPath, objectPath, objPathLen);
	memcpy(ifaceName, interfaceName, ifaceNameLen);
	objPath[objPathLen] = '\0';
	ifaceName[ifaceNameLen] = '\0';

	device->interFaceNameList.insert(std::pair<const char*, const char*>(objPath, ifaceName));

	int medthodNum = 0;
	const ajn::MsgArg* methods = NULL;
	status = args->Get("a(ssss)", &medthodNum, &methods);
	if (status != ER_OK){
		return status;
	}

	InterfaceDescription* interfaceDescription = const_cast<InterfaceDescription*>(m_BusAttachment->GetInterface(interfaceName));
	if (!interfaceDescription) {
		status = m_BusAttachment->CreateInterface(interfaceName, interfaceDescription);

		if (status == ER_OK) {
			const char* methodName = NULL;
			const char* inputSig = NULL;
			const char* outoutSig = NULL;
			const char* argNames = NULL;
			
			for (int i=0; i<medthodNum; ++i){
				status = methods[i].Get("(ssss)", &methodName, &inputSig, &outoutSig, &argNames);
				if (status == ER_OK) {
					status = interfaceDescription->AddMethod(methodName, inputSig, outoutSig, argNames);
					if (status != ER_OK){
						break;
					}
				}
			}

			if (status == ER_OK){
				interfaceDescription->Activate();
				remoteObj = new ajn::ProxyBusObject(*m_BusAttachment, wellKnownName, objectPath, 0);
				remoteObj->AddInterface(*interfaceDescription);
				device->proxyObjectList.insert(std::pair<const char*, ajn::ProxyBusObject*>(objPath, remoteObj));
			}
		}
	}

	return status;
}

QStatus SmartHomeService::CreateInterfaces()
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));
	QStatus status = ER_OK;

	InterfaceDescription* interfaceDescription = const_cast<InterfaceDescription*>(m_BusAttachment->GetInterface(INTERFACE_NAME));
	if (!interfaceDescription) {
		status = m_BusAttachment->CreateInterface(INTERFACE_NAME, interfaceDescription, false);
		if (status != ER_OK) {
			return status;
		}

		if (!interfaceDescription) {
			return ER_BUS_CANNOT_ADD_INTERFACE;
		}

		// Signature:sssa(sov), (sov):[s:InterfaceName, o:BusObjectPath, v:a(ssss), 
		// (ssss):[s:MethodName, s:InputSig, s:OutputSig, s:ArgNames]]
		status = interfaceDescription->AddMethod("ApplianceRegistration"
													, "sssa(sov)", NULL
													, "wellKnownName,uniqueName,deviceId,objectDescription");

		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->AddMethod("ApplianceUnRegistration"
													, "s", NULL
													, "deviceId");

		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->AddMethod("DeviceHeartBeat"
													, "s", NULL
													, "DeviceID");

		if (status != ER_OK) {
			return status;
		}

		status = interfaceDescription->AddMethod("Execute", "bsosv", NULL
													, "isReturn,deviceId,objectPath,methodName,arguments");
		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->AddSignal("ReturnValue", "ssv", "methodName,ReturnStatus,arguments");
		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->AddProperty("Version", "q", (uint8_t) PROP_ACCESS_READ);
		if (status != ER_OK) {
			return status;
		}
		interfaceDescription->Activate();
	}

	status = AddInterface(*interfaceDescription);
	if (status == ER_OK) {
		status = AddMethodHandler(interfaceDescription->GetMember("ApplianceRegistration"),
			static_cast<MessageReceiver::MethodHandler>(&SmartHomeService::ApplianceRegistration));
		if (status != ER_OK) {
			return status;
		}

		status = AddMethodHandler(interfaceDescription->GetMember("ApplianceUnRegistration"),
			static_cast<MessageReceiver::MethodHandler>(&SmartHomeService::ApplianceUnRegistration));
		if (status != ER_OK) {
			return status;
		}
		status = AddMethodHandler(interfaceDescription->GetMember("DeviceHeartBeat"),
			static_cast<MessageReceiver::MethodHandler>(&SmartHomeService::DeviceHeartBeat));
		if (status != ER_OK) {
			return status;
		}
		status = AddMethodHandler(interfaceDescription->GetMember("Execute"),
			static_cast<MessageReceiver::MethodHandler>(&SmartHomeService::Execute));
		if (status != ER_OK) {
			return status;
		}
		
		//Register signal handler 
	    m_ReturnValueSignalMember = interfaceDescription->GetMember("ReturnValue");
		assert(m_ReturnValueSignalMember);
	}
	return status;
}

void SmartHomeService::ApplianceRegistration(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));

	size_t argsNum = 0;
	const ajn::MsgArg* args = NULL;
	msg->GetArgs(argsNum, args);

	Task* task = ArgsParse(args, argsNum, TASK_TYPE_OF_REGISTER);
	QueueTask(task);
}

void SmartHomeService::ApplianceUnRegistration(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));

	size_t argsNum =1 ;
	const ajn::MsgArg* args = NULL;
	msg->GetArgs(argsNum, args);

	Task* task = ArgsParse(args, argsNum, TASK_TYPE_OF_UNREGISTER);
	QueueTask(task);
}

void SmartHomeService::DeviceHeartBeat(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));

	size_t argsNum = 1;
	const ajn::MsgArg* args = NULL;
	msg->GetArgs(argsNum, args);

	Task* task = ArgsParse(args, argsNum, TASK_TYPE_OF_HEARTBEAT);
	QueueTask(task);
}

void SmartHomeService::Execute(const InterfaceDescription::Member* member, Message& msg)
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));

	size_t argsNum = 0;
	const ajn::MsgArg* args = NULL;
	msg->GetArgs(argsNum, args);
	
	Task* task ;
	task = ArgsParse(args, argsNum, TASK_TYPE_OF_EXECUTE);
	
	QueueTask(task);
}

void SmartHomeService::QueueTask(Task* task)
{
	ThreadService::Instance().QueueTask(task);
}

Task* SmartHomeService::GetFreeTask(TaskType taskType)
{
	return ThreadService::Instance().PopFreeTask(taskType);
}

Device* SmartHomeService::PopDevice(const char* deviceID)
{
	return ThreadService::Instance().PopDevice(deviceID);
}

void SmartHomeService::ReleaseDevice(Device* device)
{
	return ThreadService::Instance().ReleaseDevice(device);
}
QStatus SmartHomeService::PushDevice(Device* device)
{
	return ThreadService::Instance().PushDevice(device);
}

Device* SmartHomeService::FindDevice(const char* deviceID)
{
	return ThreadService::Instance().FindDevice(deviceID);
}

ajn::ProxyBusObject* SmartHomeService::FindProxyObject(const char* deviceID, const char* objectPath)
{
	return ThreadService::Instance().FindProxyObject(deviceID, objectPath);
}

const char* SmartHomeService::FindInterFaceName(const char* deviceID, const char* objectPath)
{
	return ThreadService::Instance().FindInterfaceName(deviceID, objectPath);
}

void SmartHomeService::HeartBeatManager()
{
	return ThreadService::Instance().HeartBeatManager();
}

Task* SmartHomeService::ArgsParse(const ajn::MsgArg* args, size_t argsNum, TaskType taskType)
{
	Task* task = GetFreeTask(taskType);
	if (!task || (task->task == NULL)){
		return NULL;
	}

	if (args){
		switch (taskType)
		{
		case TASK_TYPE_OF_REGISTER:
			{
				TaskRegister* taskRegister = static_cast<TaskRegister*>(task->task);
				const ajn::MsgArg* methodArgs = NULL;
				const char* wellKnownName = NULL;
				const char* uniqueName = NULL;
				const char* deviceID = NULL;
				
				args[0].Get("s", &wellKnownName);
				args[1].Get("s", &uniqueName);
 				args[2].Get("s", &deviceID);
				args[3].Get("a(sov)", &taskRegister->methodArgsNum, &methodArgs);

				memcpy(taskRegister->wellKnownName, wellKnownName, strlen(wellKnownName));
				memcpy(taskRegister->uniqueName, uniqueName, strlen(uniqueName));
				memcpy(taskRegister->deviceID, deviceID, strlen(deviceID));	

				taskRegister->methodArgs = new ajn::MsgArg[taskRegister->methodArgsNum];
				for (int i=0; i<taskRegister->methodArgsNum; ++i){
					taskRegister->methodArgs[i] = methodArgs[i];
				}

				break;
			}
		case TASK_TYPE_OF_UNREGISTER:
			{
				const char* deviceID = NULL;

				TaskUnRegister* taskUnRegister = static_cast<TaskUnRegister*>(task->task);
				
				args[0].Get("s", &deviceID);
				memcpy(taskUnRegister->deviceID, deviceID, strlen(deviceID));
				
				break;
			}
		case TASK_TYPE_OF_HEARTBEAT:
			{
				const char* deviceID = NULL;

				TaskHeartBeat* taskHeartBeat = static_cast<TaskHeartBeat*>(task->task);
				
				args[0].Get("s", &deviceID);
				memcpy(taskHeartBeat->deviceID, deviceID, strlen(deviceID));
				
				break;
			}
		case TASK_TYPE_OF_EXECUTE:
			{
				TaskExecute* taskExecute = static_cast<TaskExecute*>(task->task);
				char* deviceID = NULL;
				char* objectPath = NULL;
				char* methodName = NULL;
				ajn::MsgArg* medthodArgs = NULL;
			
				args[0].Get("b", &taskExecute->needReturn);
				args[1].Get("s", &deviceID);
				args[2].Get("o", &objectPath);
				args[3].Get("s", &methodName);
				args[4].Get("v", &medthodArgs);
			
				memcpy(taskExecute->deviceID, deviceID, strlen(deviceID));
				memcpy(taskExecute->objectPath, objectPath, strlen(objectPath));
				memcpy(taskExecute->methodName, methodName, strlen(methodName));
				
				taskExecute->methodArgs[0] = args[4];
			
				break;
			}
		default :
			{
			}
		}
	}

	return task;
}

} // namespace
} // namespace
