#include "ThreadServiceImpl.h"

#include <qcc/Debug.h>


#include "ThreadService.h"

#define QCC_MODULE "THREAD_SERVICE_IMPL"

namespace ajn{
namespace services{

ThreadServiceImpl::ThreadServiceImpl()
	:qcc::Thread("ThreadServiceImpl")
	,m_state(IMPL_SHUTDOWN)
{
	QCC_DbgPrintf(("ThreadServiceImpl::ThreadServiceImpl()"));
}

ThreadServiceImpl::~ThreadServiceImpl()
{
	QCC_DbgPrintf(("ThreadServiceImpl::~ThreadServiceImpl()"));
}

QStatus ThreadServiceImpl::Init()
{
	QCC_DbgPrintf(("ThreadServiceImpl::Init()"));

	// Can only call Init() if the object is not in running or
	// in process of initializing.
	if (m_state != IMPL_SHUTDOWN){
		return ER_FAIL;
	}
	m_state = IMPL_INITIALIZING;

	return ER_OK;
}

QStatus ThreadServiceImpl::Start(void* arg)
{
	QCC_DbgPrintf(("ThreadServiceImpl::Start()"));
	
	m_mutex.Lock();
	if(IsRunning() == true){
		return ER_OK;
	}
	m_state = IMPL_RUNNING;
	QCC_DbgHLPrintf(("ThreadServiceImpl::Start():Starting thread"));
	QStatus status = qcc::Thread::Start(arg);
	QCC_DbgHLPrintf(("ThreadServiceImpl::Start():Started"));
	m_mutex.Unlock();

	return status;
}

bool ThreadServiceImpl::Started()
{
	QCC_DbgPrintf(("ThreadServiceImpl::Started()"));

	return IsRunning();
}

QStatus ThreadServiceImpl::Stop()
{
	QCC_DbgPrintf(("ThreadServiceImpl::Stop()"));

	m_mutex.Lock();
	if (m_state != IMPL_SHUTDOWN){
		m_state = IMPL_STOPPING;
	}
	QCC_DbgPrintf(("ThreadServiceImpl::Stop():Stopping thead"));
	QStatus status = qcc::Thread::Stop();
	QCC_DbgPrintf(("ThreadServiceImpl::Stop():Stopped"));
	m_mutex.Unlock();

	return status;
}

QStatus ThreadServiceImpl::Join()
{
	QCC_DbgPrintf(("ThreadServiceImpl::Join()"));

	if ((m_state != IMPL_STOPPING) || (m_state != IMPL_SHUTDOWN)){
		return ER_FAIL;
	}
	QCC_DbgPrintf(("ThreadServiceImpl::Join():Joining thead"));
	QStatus status = qcc::Thread::Join();
	QCC_DbgPrintf(("ThreadServiceImpl::Join():Joined"));
	m_state = IMPL_SHUTDOWN;

	return status;
}

qcc::ThreadReturn ThreadServiceImpl::Run(void* arg)
{
	QCC_DbgPrintf(("ThreadServiceImpl::Run()"));

	ThreadService* pThreadService = static_cast<ThreadService*>(arg);
	if (!pThreadService){
		(qcc::ThreadReturn)ER_FAIL;
	}
	
	while (m_state == IMPL_RUNNING){		
		Task* task = pThreadService->PopTask();

		if (task){
			pThreadService->PostTask(task);
			pThreadService->RecycleTask(task);		

			continue;
		}else{			
			qcc::Sleep(10);
		}
	}
	
	return (qcc::ThreadReturn)ER_OK;
}

} //namespace
} //namespace