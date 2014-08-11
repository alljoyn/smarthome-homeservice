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
#ifndef _HEART_THREAD_SERVICE_IMPL_H
#define _HEART_THREAD_SERVICE_IMPL_H

#include <qcc/platform.h>
#include <qcc/String.h>
#include <qcc/Thread.h>

namespace ajn {
namespace services {

class HeartThreadServiceImpl : public qcc::Thread {
public:
	enum State {
		IMPL_INVALID,           /**< Should never be seen on a constructed object */
		IMPL_SHUTDOWN,          /**< Nothing is running and object may be destroyed */
		IMPL_INITIALIZING,      /**< Object is in the process of coming up and may be inconsistent */
		IMPL_RUNNING,           /**< Object is running and ready to go */
		IMPL_STOPPING,          /**< Object is stopping */
	};

	HeartThreadServiceImpl();
	~HeartThreadServiceImpl();
	
    /**
     * @brief Initialize the central thread.
     */
    QStatus Init();

    /**
     * @brief Start any required name service threads.
     */
    QStatus Start(void* arg);

    /**
     * @brief return true if central threads are running
     */
    bool Started();

    /**
     * @brief Stop any central threads.
     */
    QStatus Stop();

    /**
     * @brief Join any central threads.
     */
    QStatus Join();

private:
	/**
     * Main thread entry point.
     *
     * @param arg  Unused thread entry arg.
     */
    qcc::ThreadReturn STDCALL Run(void* arg);

	/**
     * @internal
     * @brief State variable to indicate what the implementation is doing or is
     * capable of doing.
     */
    State m_state;

	/**
     * @internal
     * @brief Mutex object used to protect various lists that may be accessed
     * by multiple threads.
     */
    qcc::Mutex m_mutex;
};

} //namespace
} //namespace
#endif
