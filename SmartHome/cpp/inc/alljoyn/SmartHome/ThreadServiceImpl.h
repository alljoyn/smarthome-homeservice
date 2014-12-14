#ifndef _THREAD_SERVICE_IMPL_H
#define _THREAD_SERVICE_IMPL_H

#include <qcc/platform.h>
#include <qcc/String.h>
#include <qcc/Thread.h>

namespace ajn {
namespace services {

class ThreadServiceImpl : public qcc::Thread {
public:
	enum State {
		IMPL_INVALID,           /**< Should never be seen on a constructed object */
		IMPL_SHUTDOWN,          /**< Nothing is running and object may be destroyed */
		IMPL_INITIALIZING,      /**< Object is in the process of coming up and may be inconsistent */
		IMPL_RUNNING,           /**< Object is running and ready to go */
		IMPL_STOPPING,          /**< Object is stopping */
	};

	ThreadServiceImpl();
	~ThreadServiceImpl();
	
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