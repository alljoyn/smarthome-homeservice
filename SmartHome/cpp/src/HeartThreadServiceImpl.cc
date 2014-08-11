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

#include "HeartThreadServiceImpl.h"

#include <qcc/Debug.h>

#include "ThreadService.h"

#define QCC_MODULE "HEART_THREAD_SERVICE_IMPL"

namespace ajn{
namespace services{

HeartThreadServiceImpl::HeartThreadServiceImpl()
	:qcc::Thread("HeartThreadServiceImpl")
	,m_state(IMPL_SHUTDOWN)
{
	QCC_DbgPrintf(("HeartThreadServiceImpl::HeartThreadServiceImpl()"));
}

HeartThreadServiceImpl::~HeartThreadServiceImpl()
{
	QCC_DbgPrintf(("HeartThreadServiceImpl::~HeartThreadServiceImpl()"));
}

QStatus HeartThreadServiceImpl::Init()
{
	QCC_DbgPrintf(("HeartThreadServiceImpl::Init()"));

	// Can only call Init() if the object is not in running or
	// in process of initializing.
	if (m_state != IMPL_SHUTDOWN){
		return ER_FAIL;
	}
	m_state = IMPL_INITIALIZING;

	return ER_OK;
}

QStatus HeartThreadServiceImpl::Start(void* arg)
{
	QCC_DbgPrintf(("HeartThreadServiceImpl::Start()"));
	
	m_mutex.Lock();
	if(IsRunning() == true){
		return ER_OK;
	}
	m_state = IMPL_RUNNING;
	QCC_DbgHLPrintf(("HeartThreadServiceImpl::Start():Starting thread"));
	QStatus status = qcc::Thread::Start(arg);
	QCC_DbgHLPrintf(("HeartThreadServiceImpl::Start():Started"));
	m_mutex.Unlock();

	return status;
}

bool HeartThreadServiceImpl::Started()
{
	QCC_DbgPrintf(("HeartThreadServiceImpl::Started()"));

	return IsRunning();
}

QStatus HeartThreadServiceImpl::Stop()
{
	QCC_DbgPrintf(("HeartThreadServiceImpl::Stop()"));

	m_mutex.Lock();
	if (m_state != IMPL_SHUTDOWN){
		m_state = IMPL_STOPPING;
	}
	QCC_DbgPrintf(("HeartThreadServiceImpl::Stop():Stopping thead"));
	QStatus status = qcc::Thread::Stop();
	QCC_DbgPrintf(("HeartThreadServiceImpl::Stop():Stopped"));
	m_mutex.Unlock();

	return status;
}

QStatus HeartThreadServiceImpl::Join()
{
	QCC_DbgPrintf(("HeartThreadServiceImpl::Join()"));

	if ((m_state != IMPL_STOPPING) || (m_state != IMPL_SHUTDOWN)){
		return ER_FAIL;
	}
	QCC_DbgPrintf(("HeartThreadServiceImpl::Join():Joining thead"));
	QStatus status = qcc::Thread::Join();
	QCC_DbgPrintf(("HeartThreadServiceImpl::Join():Joined"));
	m_state = IMPL_SHUTDOWN;

	return status;
}

qcc::ThreadReturn HeartThreadServiceImpl::Run(void* arg)
{
	QCC_DbgPrintf(("HeartThreadServiceImpl::Run()"));
	static int counter = 0;
	#define TIME_OUT 1000

	ThreadService* pThreadService = static_cast<ThreadService*>(arg);
	if (!pThreadService){
		(qcc::ThreadReturn)ER_FAIL;
	}
	
	while (m_state == IMPL_RUNNING){		
	   if (counter<TIME_OUT){
		   counter++;
		   }else{
		   pThreadService->HeartBeatManager();
		   counter = 0;
	   }

	   qcc::Sleep(10);
	}
	
	return (qcc::ThreadReturn)ER_OK;
}

} //namespace
} //namespace
