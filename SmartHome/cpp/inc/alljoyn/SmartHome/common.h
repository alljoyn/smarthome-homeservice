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
	TASK_TYPE_OF_EXECUTE = 4
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
	char deviceID[MAX_ARRAY_LEN];
	char interfaceName[MAX_ARRAY_LEN];	
	size_t methodArgsNum;
	ajn::MsgArg* methodArgs;

	_taskRegister()
	{
		memset(wellKnownName, '\0', sizeof(wellKnownName));
		memset(uniqueName, '\0', sizeof(uniqueName));
		memset(deviceID, '\0', sizeof(deviceID));
		memset(interfaceName, '\0', sizeof(interfaceName));
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
   char deviceID[MAX_ARRAY_LEN];
   _taskUnRegister()
	{
		memset(deviceID, '\0', sizeof(deviceID));	
	}
}TaskUnRegister;

/**
* Container of a task of heart beat.
*/
typedef struct _taskHeartBeat{
   char deviceID[MAX_ARRAY_LEN] ;
    _taskHeartBeat()
	{
		memset(deviceID, '\0', sizeof(deviceID));
	}
}TaskHeartBeat;

/**
* Container of a task of Execute.
*/
typedef struct _taskExecute {
	int needReturn;	// we need to return a value if zero. 
	char deviceID[MAX_ARRAY_LEN];
	char objectPath[MAX_ARRAY_LEN];
	char methodName[MAX_ARRAY_LEN];
	ajn::MsgArg* methodArgs; 
	_taskExecute()
	{
		needReturn = 0;
		memset(deviceID, '\0', sizeof(deviceID));
		memset(objectPath, '\0', sizeof(objectPath));
		memset(methodName, '\0', sizeof(methodName));
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
	char deviceID[MAX_ARRAY_LEN];

	int  heartCount;
	MapProxyObject  proxyObjectList;
	MapInerfaceName interFaceNameList;
	_device()
	{
		heartCount = 3;
		memset(wellKnownName, '\0', sizeof(wellKnownName));
		memset(uniqueName, '\0', sizeof(uniqueName));
		memset(deviceID, '\0', sizeof(deviceID));
	}
	~_device()
	{
	
	}
} Device;

/**
* The list of Device stored on smart home server.
* <DeviceID, Device>
*/
typedef std::map<const char*, Device*, _keyPtrCmp> MapDevice;

} //namespace
} //namespace
#endif
