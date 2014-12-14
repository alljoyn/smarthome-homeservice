#ifndef _COMMON_H
#define _COMMON_H

#include <list>
#include <map>

#include <alljoyn/ProxyBusObject.h>
#include <alljoyn/MsgArg.h>

namespace ajn{
namespace services{

#define MAX_ARRAY_LEN 255
#define MAX_TASK_LIST_LENGTH		50
#define MAX_FREE_TASK_LIST_LENGTH   50

/**
* On behalf of task type.
*/
typedef enum _taskType{
	TASK_TYPE_OF_UNKNOWN = 0,
	TASK_TYPE_OF_REGISTER = 1,
	TASK_TYPE_OF_UNREGISTER = 2,
	TASK_TYPE_OF_HEARTBEAT = 3,
	TASK_TYPE_OF_EXECUTE = 4,
	TASK_TYPE_OF_VERIFICATION = 5
}TaskType;

/**
* Container of a any task.
*/
typedef struct _task {
	TaskType taskType;
	void* task;
} Task;

/**
* Container of a task of register.
*/
typedef struct _taskRegister{
	char wellKnownName[MAX_ARRAY_LEN];
	char uniqueName[MAX_ARRAY_LEN];
	char deviceId[MAX_ARRAY_LEN];
	size_t methodArgsNum;
	ajn::MsgArg* methodArgs;

	_taskRegister()
	{
		memset(wellKnownName, '\0', sizeof(wellKnownName));
		memset(uniqueName, '\0', sizeof(uniqueName));
		memset(deviceId, '\0', sizeof(deviceId));
		methodArgsNum = 0;
		methodArgs = NULL;
	}

	~_taskRegister()
	{
		if (methodArgs){
			delete []methodArgs;
			methodArgs = NULL;
		}
	}
} TaskRegister;

/**
* Container of a task of unregister.
*/
typedef struct _taskUnRegister{
   char deviceId[MAX_ARRAY_LEN];
   _taskUnRegister()
	{
		memset(deviceId, '\0', sizeof(deviceId));	
	}
}TaskUnRegister;

/**
* Container of a task of heart beat.
*/
typedef struct _taskHeartBeat{
   char deviceId[MAX_ARRAY_LEN] ;
   char randomString[MAX_ARRAY_LEN] ;
    _taskHeartBeat()
	{
		memset(deviceId, '\0', sizeof(deviceId));
		memset(randomString, '\0', sizeof(randomString));
	}
}TaskHeartBeat;

/**
* Container of a task of Execute.
*/
typedef struct _taskExecute {
	int needReturn;	// we need to return a value if zero. 
	char deviceId[MAX_ARRAY_LEN];
	char objectPath[MAX_ARRAY_LEN];
	char interfaceName[MAX_ARRAY_LEN];
	char methodName[MAX_ARRAY_LEN];
	ajn::MsgArg* methodArgs; 
	_taskExecute()
	{
		needReturn = 0;
		memset(deviceId, '\0', sizeof(deviceId));
		memset(objectPath, '\0', sizeof(objectPath));
		memset(methodName, '\0', sizeof(methodName));
		memset(interfaceName, '\0', sizeof(interfaceName));
		methodArgs = new ajn::MsgArg();
	}

	~_taskExecute()
	{
		if (methodArgs){
			delete methodArgs;
			methodArgs = NULL;
		}
	}
} TaskExecute;

/**
* Container of a task of verification.
*/
typedef struct _taskVerification {
	
	char deviceId[MAX_ARRAY_LEN];
	
	_taskVerification()
	{
		memset(deviceId, '\0', sizeof(deviceId));
	}

	~_taskVerification()
	{
	}
} TaskVerification;

/**
* The list of TaskRegister.
*/
typedef std::list<ajn::services::TaskRegister*> TaskRegisterList;

/**
* The list of TaskExecute.
*/
typedef std::list<ajn::services::TaskExecute*> TaskExecuteList;

/**
* The list of Task.
*/
typedef std::list<ajn::services::Task*> TaskList;

struct _keyPtrCmp
{
	bool operator()( const char * s1, const char * s2 ) const
	{
		return strcmp( s1, s2 ) < 0;
	}
};

/**
* The list of ProxyBusObject of a Device.
* <ObjectPath, ProxyBusObject>
*/
typedef std::map<const char*, ajn::ProxyBusObject*, _keyPtrCmp> MapProxyObject;

/**
* The list of interface name of BusObject of a Device.
* <ObjectPath, InterfaceName>
*/
typedef std::map<const char*, const char*, _keyPtrCmp> MapInerfaceName;

/**
* Device information.
*/
typedef struct _device{
	char wellKnownName[MAX_ARRAY_LEN];
	char uniqueName[MAX_ARRAY_LEN];
	char deviceId[MAX_ARRAY_LEN];
	char interfaceName[MAX_ARRAY_LEN];
	char objectPath[MAX_ARRAY_LEN];
	size_t methodArgsNum;
	ajn::MsgArg* methodArgs;

	int  heartCount;
	int stringTime;
	int deviceLock;
	char randomString[8]; 
	MapProxyObject  proxyObjectList;
	MapInerfaceName interFaceNameList;
	_device()
	{
		heartCount = 3;
		stringTime = 60;
		deviceLock = 0;
		memset(randomString,'\0',sizeof(randomString));
		memset(wellKnownName, '\0', sizeof(wellKnownName));
		memset(uniqueName, '\0', sizeof(uniqueName));
		memset(deviceId, '\0', sizeof(deviceId));
		memset(interfaceName, '\0', sizeof(interfaceName));
		memset(objectPath, '\0', sizeof(objectPath));
		methodArgs = new ajn::MsgArg();
	
		methodArgsNum = 0;
		methodArgs = NULL;
	}
	~_device()
	{
	
	}
} Device;

/**
* The list of Device stored on smart home server.
* <deviceId, Device>
*/
typedef std::map<const char*, Device*, _keyPtrCmp> MapDevice;

} //namespace
} //namespace
#endif