#include "SmartHomeService.h"

#include <qcc/Debug.h>
#include <alljoyn/BusAttachment.h>
#include "MemoryManager.h"
#include "BusListenerImpl.h"
#include "SmartHomeServiceMain.h"
#include "xmldecoder.h"
#include <CTIME>
#define QCC_MODULE "SMART_HOME_SERVICE"
#define VERSION 1
#define SERVICEINTERFACEVERSION 1

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
		case TASK_TYPE_OF_VERIFICATION:
		{
			TaskVerification* taskVerification = static_cast<TaskVerification*>(task->task);
			HandleTaskVerification(*taskVerification);

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
	Device* device = PopDevice(taskRegister.deviceId);
	if (!device){
		device = new Device();
		memcpy(device->wellKnownName, taskRegister.wellKnownName, sizeof(taskRegister.wellKnownName));
		memcpy(device->uniqueName, taskRegister.uniqueName, sizeof(taskRegister.uniqueName));
		memcpy(device->deviceId, taskRegister.deviceId, sizeof(taskRegister.deviceId));
	}
	 
	String Source(taskRegister.methodArgs->v_string.str);
	size_t objpath_num = ReturnHowManyObjectPath(Source);
	String* obj_path = new String[objpath_num];
	for(size_t i=1;i<=objpath_num;i++)
	{
		obj_path[i-1] = ReturnObjectPath(Source,i);
		ProxyBusObject *remoteObj = new ajn::ProxyBusObject(*m_BusAttachment, taskRegister.uniqueName, obj_path[i-1].c_str(), ID);
		status = remoteObj->IntrospectRemoteObject();
		//if(status != ER_OK)
		//	return status;
		device->proxyObjectList.insert(std::pair<const char*, ajn::ProxyBusObject*>(obj_path[i-1].c_str(), remoteObj));
	}
	status = ER_OK;
	if (status == ER_OK){
		PushDevice(device);		
	}else{
		QCC_LogError(status, ("HandleTaskRegister Error:%s, %d, %d\n"
							, device->deviceId, device->proxyObjectList.size()
							,device->interFaceNameList.size()));
	}
	
	return status;
}

void SmartHomeService::HandleTaskUnRegister(const TaskUnRegister & taskUnRegister)
{

	Device* device = PopDevice(taskUnRegister.deviceId);
	if (!device){
		printf("There is no device named %s", taskUnRegister.deviceId);
	}
	
	ReleaseDevice(device);
	Device* findDevice = FindDevice(taskUnRegister.deviceId);
	if (findDevice){
	printf("Unregister the device %s failed! %d\n", taskUnRegister.deviceId);
	}else{
	printf("Unregister the device %s successful! %d\n", taskUnRegister.deviceId);
	}
}

void SmartHomeService::HandleTaskHeartBeat(const TaskHeartBeat & taskHeartBeat)
{
	const char* deviceId = taskHeartBeat.deviceId;
	const char* randomString = taskHeartBeat.randomString;
	Device * device = FindDevice(taskHeartBeat.deviceId);
	if (!device){
		ajn::MsgArg Args;
		Args.Set("s","NO DEVICE!");
		MsgArg GWReplyArgs[3];
		GWReplyArgs[0].Set("s","HeartBeat");
		GWReplyArgs[1].Set("s",QCC_StatusText(ER_NO_SUCH_DEVICE));
		GWReplyArgs[2].Set("v",Args);
		QStatus status = Signal(NULL, ID, *m_ReturnValueSignalMember,GWReplyArgs,3, 0);
		printf("There is no device named %s ! \n", taskHeartBeat.deviceId);
	}
	else if(!strcmp(randomString,device->randomString) && device->deviceLock != -1)
	{
	    device->heartCount = 3;
		device->stringTime = 60;
		printf("Receive heart beat from %s and refresh arguments! \n", taskHeartBeat.deviceId);
		ajn::MsgArg Args;
		Args.Set("s","RandomString MATCHED and VALID!");
		MsgArg GWReplyArgs[3];
		GWReplyArgs[0].Set("s","HeartBeat");
		GWReplyArgs[1].Set("s",QCC_StatusText(ER_OK));
		GWReplyArgs[2].Set("v",Args);
		QStatus status = Signal(NULL, ID, *m_ReturnValueSignalMember,GWReplyArgs,3, 0);
		printf("Heartbeat:Signal ID %d, method is %s, status is %s, Args are %s.\n", ID, "HeartBeat", QCC_StatusText(ER_TIMEOUT),Args.v_string.str);
	}
	else if(device->deviceLock == -1)//the device has locked and need 
	{
		ajn::MsgArg Args;
		Args.Set("s","Locked = -1");
		MsgArg GWReplyArgs[3];
		GWReplyArgs[0].Set("s","HeartBeat");
		GWReplyArgs[1].Set("s",QCC_StatusText(ER_TIMEOUT));
		GWReplyArgs[2].Set("v",Args);
		QStatus status = Signal(NULL, ID, *m_ReturnValueSignalMember,GWReplyArgs,3, 0);
		printf("Heartbeat:Signal ID %d, method is %s, status is %s, Args are %s.\n", ID, "HeartBeat", QCC_StatusText(ER_TIMEOUT),Args.v_string.str);
	}
	else if(strcmp(randomString,device->randomString)&&strcmp(randomString,"NULL"))//string not match
	{
		ajn::MsgArg Args;
		Args.Set("s","RandomString unmatched");
		MsgArg GWReplyArgs[3];
		GWReplyArgs[0].Set("s","HeartBeat");
		GWReplyArgs[1].Set("s",QCC_StatusText(ER_BUS_MATCH_RULE_NOT_FOUND));
		GWReplyArgs[2].Set("v",Args);
		QStatus status = Signal(NULL, ID, *m_ReturnValueSignalMember,GWReplyArgs,3, 0);
		printf("Heartbeat:Signal ID %d, method is %s, status is %s, Args are %s.\n", ID, "HeartBeat", QCC_StatusText(ER_TIMEOUT),Args.v_string.str);
	}else if(!strcmp(randomString,"NULL")){    //first heartbeat,string is NULL so gateway give a random string to the client
	    ajn::MsgArg Args;
		const char * newString;
		newString = GenerateRandomString(device->randomString);
		Args.Set("s",newString);
		MsgArg GWReplyArgs[3];
		GWReplyArgs[0].Set("s","HeartBeat");
		GWReplyArgs[1].Set("s",QCC_StatusText(ER_BAD_ARG_3));
		GWReplyArgs[2].Set("v",Args);
		QStatus status = Signal(NULL, ID, *m_ReturnValueSignalMember,GWReplyArgs,3, 0);
		printf("Heartbeat:Signal ID %d, method is %s, status is %s, Args are %s.\n", ID, "HeartBeat", QCC_StatusText(ER_TIMEOUT),Args.v_string.str);
	
	}
}

QStatus SmartHomeService::HandleTaskExecute(const TaskExecute & taskExecute)
{
	Message reply(*m_BusAttachment);
	QStatus status = ER_OK;
	ajn::ProxyBusObject* remoteObj = FindProxyObject(taskExecute.deviceId, taskExecute.objectPath);
	Device *senderDevice = FindDevice(taskExecute.deviceId);

    if (remoteObj){
		status = remoteObj->MethodCall(taskExecute.interfaceName, taskExecute.methodName, taskExecute.methodArgs, 1, reply, 5000);
		ajn::AllJoynMessageType i = reply->GetType();
		printf("HandleTaskExecute->[%s][%s]\n", taskExecute.methodName, reply->GetArg(0)->v_string.str);
		status = SmartHomeServiceApi::getInstance()->ReturnValue(reply,senderDevice,taskExecute,status);
		printf("ReturnValue Signal send %s\n", QCC_StatusText(status));
	}else{
		QCC_LogError(status, ("%s, %s, %s\n", taskExecute.deviceId, taskExecute.objectPath, taskExecute.interfaceName));
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
	GWReplyArgs[0].Set("s",methodName);
	GWReplyArgs[1].Set("s",QCC_StatusText(status));
	GWReplyArgs[2].Set("v",Args);
    QStatus status_signal = Signal(NULL, ID, *m_ReturnValueSignalMember,GWReplyArgs,3, 0);
	printf("Signal ID %d, method is %s, status is %s, Args are %s.\n", ID, methodName, QCC_StatusText(status),Args->v_variant.val->v_string.str);
	
	return status_signal;
}

QStatus SmartHomeService::HandleTaskVerification(const TaskVerification & taskVerification)
{
	QStatus status = ER_OK;
	const char* deviceId = taskVerification.deviceId;
	Device* device = FindDevice(taskVerification.deviceId);
	if (!device){
		Device *nullDevice = new Device();
		char* wellKnownName = "NULL";
		char* uniqueName = "NULL";
		memcpy(nullDevice->deviceId, deviceId, strlen(deviceId));
		memcpy(nullDevice->wellKnownName, wellKnownName, strlen(wellKnownName));
		memcpy(nullDevice->uniqueName, uniqueName, strlen(uniqueName));

		printf("HandleTaskVerification-> No device verified %s\n", taskVerification.deviceId);
		status = ReturnValueVerification(nullDevice);
		return ER_FAIL;
	}else{
		printf("HandleTaskVerification->deviceId: %s\n", taskVerification.deviceId);
		status = ReturnValueVerification(device);
		return status;
	}
}

QStatus SmartHomeService::ReturnValueVerification(Device *device)
{
	QStatus status = ER_OK;
	size_t argsNum = 0;
	const ajn::MsgArg * Args = NULL;
	MsgArg ReplyArgs;
	ReplyArgs.Set("(sss)",device->deviceId,device->wellKnownName,device->uniqueName);

	status = Signal(NULL, ID, *m_ReturnValueVerificationSignalMember,&ReplyArgs, 1, 0);
	
	printf("UniqueName : %s.\n", device->uniqueName);
	printf("WellName : %s.\n", device->wellKnownName);
	printf("Return : %s.\n",QCC_StatusText(status));
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

		status = interfaceDescription->AddMethod("ApplianceRegistration"
													, "ss", "u"
													, "RandomString,XML,returnRandomString");

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
													, "ss", NULL
													, "deviceId,validateCode");

		if (status != ER_OK) {
			return status;
		}

		status = interfaceDescription->AddMethod("Execute", "bsossv", NULL
													, "isReturn,deviceId,objectPath,interfaceName,methodName,arguments");
		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->AddMethod("Verification", "s", NULL
											, "deviceId");
		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->AddSignal("ReturnValue", "ssv", "methodName,ReturnStatus,value");
		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->AddSignal("ReturnValueVerification", "(sss)", "VerificationResult");

		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->AddProperty("Version", "u", (uint8_t) PROP_ACCESS_READ);
		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->SetPropertyDescription("Version", "1.0.0");
		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->AddProperty("ServiceInterfaceVercion", "q", (uint8_t) PROP_ACCESS_READ);
		if (status != ER_OK) {
			return status;
		}
		status = interfaceDescription->SetPropertyDescription("ServiceInterfaceVercion", "1.0.0");
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
		status = AddMethodHandler(interfaceDescription->GetMember("Verification"),
			static_cast<MessageReceiver::MethodHandler>(&SmartHomeService::Verification));
		if (status != ER_OK) {
			return status;
		}
	    m_ReturnValueSignalMember = interfaceDescription->GetMember("ReturnValue");
		assert(m_ReturnValueSignalMember);
		m_ReturnValueVerificationSignalMember = interfaceDescription->GetMember("ReturnValueVerification");
		assert(m_ReturnValueVerificationSignalMember);
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
	TaskRegister* taskRegister = static_cast<TaskRegister*>(task->task);
	QueueTask(task);
	MsgArg outArg;
	outArg.Set("u", ER_OK);
	printf("Queued Task!\n");
	QStatus status = MethodReply(msg,&outArg,1);
}

void SmartHomeService::ApplianceUnRegistration(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));

	size_t argsNum =1 ;
	const ajn::MsgArg* args = NULL;
	msg->GetArgs(argsNum, args);

	Task* task = ArgsParse(args, argsNum, TASK_TYPE_OF_UNREGISTER);
	QueueTask(task);
	MsgArg outArg("u", ER_OK);
	QStatus status = MethodReply(msg);
}

void SmartHomeService::DeviceHeartBeat(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));

	size_t argsNum = 1;
	const ajn::MsgArg* args = NULL;
	msg->GetArgs(argsNum, args);

	Task* task = ArgsParse(args, argsNum, TASK_TYPE_OF_HEARTBEAT);
	QueueTask(task);
	MsgArg outArg("u", ER_OK);
	QStatus status = MethodReply(msg);
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
	MethodReply(msg);
}

void SmartHomeService::Verification(const InterfaceDescription::Member* member, Message& msg)
{
	QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));

	size_t argsNum = 0;
	const ajn::MsgArg* args = NULL;
	msg->GetArgs(argsNum, args);
	
	Task* task ;
	task = ArgsParse(args, argsNum, TASK_TYPE_OF_VERIFICATION);
	
	QueueTask(task);
	MethodReply(msg);
}

void SmartHomeService::QueueTask(Task* task)
{
	ThreadService::Instance().QueueTask(task);
}

Task* SmartHomeService::GetFreeTask(TaskType taskType)
{
	return ThreadService::Instance().PopFreeTask(taskType);
}

Device* SmartHomeService::PopDevice(const char* deviceId)
{
	return ThreadService::Instance().PopDevice(deviceId);
}

void SmartHomeService::ReleaseDevice(Device* device)
{
	return ThreadService::Instance().ReleaseDevice(device);
}
QStatus SmartHomeService::PushDevice(Device* device)
{
	return ThreadService::Instance().PushDevice(device);
}

Device* SmartHomeService::FindDevice(const char* deviceId)
{
	return ThreadService::Instance().FindDevice(deviceId);
}

ajn::ProxyBusObject* SmartHomeService::FindProxyObject(const char* deviceId, const char* objectPath)
{
	return ThreadService::Instance().FindProxyObject(deviceId, objectPath);
}

const char* SmartHomeService::FindInterFaceName(const char* deviceId, const char* objectPath)
{
	return ThreadService::Instance().FindInterfaceName(deviceId, objectPath);
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
				printf("Enter the ArgsParse!\n");
				TaskRegister* taskRegister = static_cast<TaskRegister*>(task->task);
				String Source(args[1].v_string.str);
				String wellKnownName;
				String uniqueName;
				String deviceId ;
				
				ReturnNamesAndID(Source,wellKnownName,uniqueName,deviceId);

				const ajn::MsgArg* methodArgs = new ajn::MsgArg("s",Source.c_str());

				memcpy(taskRegister->wellKnownName, wellKnownName.c_str(), strlen(wellKnownName.c_str()));
				memcpy(taskRegister->uniqueName, uniqueName.c_str(), strlen(uniqueName.c_str()));
				memcpy(taskRegister->deviceId, deviceId.c_str(), strlen(deviceId.c_str()));	

				taskRegister->methodArgsNum = 1;
				taskRegister->methodArgs = new ajn::MsgArg[taskRegister->methodArgsNum];
				taskRegister->methodArgs[0] = *methodArgs;

				break;
			}
		case TASK_TYPE_OF_UNREGISTER:
			{
				const char* deviceId = NULL;

				TaskUnRegister* taskUnRegister = static_cast<TaskUnRegister*>(task->task);
				
				args[0].Get("s", &deviceId);
				memcpy(taskUnRegister->deviceId, deviceId, strlen(deviceId));
				
				break;
			}
		case TASK_TYPE_OF_HEARTBEAT:
			{
				const char* deviceId = NULL;
				const char* randomString = NULL;

				TaskHeartBeat* taskHeartBeat = static_cast<TaskHeartBeat*>(task->task);
				
				args[0].Get("s", &deviceId);
				args[1].Get("s", &randomString);
				memcpy(taskHeartBeat->deviceId, deviceId, strlen(deviceId));
				memcpy(taskHeartBeat->randomString, randomString, strlen(randomString));
				break;
			}
		case TASK_TYPE_OF_EXECUTE:
			{
				TaskExecute* taskExecute = static_cast<TaskExecute*>(task->task);
				char* deviceId = NULL;
				char* objectPath = NULL;
				char* methodName = NULL;
				char* interfaceName = NULL;
				ajn::MsgArg* medthodArgs = NULL;
			
				args[0].Get("b", &taskExecute->needReturn);
				args[1].Get("s", &deviceId);
				args[2].Get("o", &objectPath);
				args[3].Get("s", &interfaceName);
				args[4].Get("s", &methodName);
				args[5].Get("v", &medthodArgs);
			
				memcpy(taskExecute->deviceId, deviceId, strlen(deviceId));
				memcpy(taskExecute->objectPath, objectPath, strlen(objectPath));
				memcpy(taskExecute->methodName, methodName, strlen(methodName));
				memcpy(taskExecute->interfaceName, interfaceName, strlen(interfaceName));

				
				taskExecute->methodArgs[0] = args[5];
			
				break;
			}
		case TASK_TYPE_OF_VERIFICATION:
			{
				TaskVerification* taskVerification = static_cast<TaskVerification*>(task->task);
				const char* deviceId = NULL;
				
 				args[0].Get("s", &deviceId);

				memcpy(taskVerification->deviceId, deviceId, strlen(deviceId));	

				break;
			}
		default :
			{
			}
		}
	}

	return task;
}
const char* SmartHomeService::GenerateRandomString(char *device_randomString)
{
	srand((unsigned)time(NULL));
	for (int i = 0; i < 8; i++)
	{
		int x = rand() / (RAND_MAX / (sizeof(CCH) - 1));
		device_randomString[i] = CCH[x];
	}
	return device_randomString;
}
QStatus SmartHomeService::Get(const char* ifcName, const char* propName, MsgArg& val) {
    QCC_DbgTrace(("SmartHomeService::%s", __FUNCTION__));
    QStatus status = ER_BUS_NO_SUCH_PROPERTY;
	if (0 == strcmp(ifcName, INTERFACE_NAME)) {
        if (0 == strcmp("Version", propName)) {
			status = val.Set("u", VERSION);
		}
		if (0 == strcmp("ServiceInterfaceVersion", propName)) {
			status = val.Set("q", SERVICEINTERFACEVERSION);
        }
    }
    return status;
}

} // namespace
} // namespace