/* ************************************************************************* *\
#               INTEL CORPORATION PROPRIETARY INFORMATION
#     This software is supplied under the terms of a license agreement or
#     nondisclosure agreement with Intel Corporation and may not be copied
#     or disclosed except in accordance with the terms of that agreement.
#        Copyright (C) 2011 Intel Corporation. All Rights Reserved.
#\* ************************************************************************* */

#pragma once

// C++ implementations should define INT16_MAX macro only when __STDC_LIMIT_MACROS is defined before <stdint.h> is included 
#define __STDC_LIMIT_MACROS

#include <stdint.h>
#include <pthread.h>
#include <vector>
#include <utility>
#include <set>

#include "cl_synch_objects.h"

#include <common/COITypes_common.h>

using namespace std;
using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace MICDevice {

class NotificationPort
{
public:

	// CallBack interface
	class CallBack
	{
	public:
		virtual void fireCallBack(void* arg) = 0;
	};

	enum ERROR_CODE{
		SUCCESS=0,
		ALREADY_INITIALIZE_FAILURE=1,
		MEM_OBJECT_ALLOCATION_FAILURE=2,
		CREATE_BARRIER_FAILURE=3,
		CREATE_WORKER_THREAD_FAILURE=4,
		NOT_INITIASLIZE_FAILURE=5
	};
	
	/* Factory for notification port.
	   Create new NotificationPort object and call to initialize.
	   maxBarriers - initial size of allocate barriers for the notification port.
	   In case of success - the notification port initialization completes and return pointer to NotificationPort object.
	   In case of failure - return NULL.
	   Do NOT delete this object it will delete itself when the worker thread will finish its work. */
	static NotificationPort* notificationPortFactory(uint16_t maxBarriers);

	int addBarrier(const COIEVENT& barrier, NotificationPort::CallBack* callBack, void* arg);

	int release();

	/* This method blocks the calling thread until all the notification ports threads finish their life cycle. 
	   The calling thread have to call release() of each notification port before. */
	static void waitForAllNotificationPortThreads();

private:

	enum AVAILABLE_OPERATIONS{
		RELEASE=0,
		ADD,
		AVAILABLE_OPERATIONS_LEN
	};
	
	enum WORKER_STATE {
	    CREATED=0,
		BEGINNING,
		RUNNING,
		FINISHED
	};

	struct notificationPackage
	{
		NotificationPort::CallBack* callBack;
		void* arg;
		uint64_t age;

		static bool compare(const notificationPackage& first, const notificationPackage& second)
		{
			return (first.age < second.age);
		}
	};

	typedef pair<COIEVENT, notificationPackage> EventNotificationPackagePair;
	
	volatile size_t m_poc;

	bool m_operationMask[AVAILABLE_OPERATIONS_LEN];

	COIEVENT* m_barriers;
	notificationPackage* m_notificationsPackages;

	vector<EventNotificationPackagePair> m_pendingNotificationArr;

	pthread_mutex_t m_mutex;

	pthread_t m_workerThread;

	pthread_cond_t m_clientCond;

	uint16_t m_maxBarriers;
	uint16_t m_waitingSize;

	// Need it in order to enforce right order of callbacks
	uint64_t m_lastCallBackAge;
	
	volatile WORKER_STATE m_workerState;

	// Set of currently notification port threads.
	static set<pthread_t> m_NotificationThreadsSet;
	static OclMutex      m_notificationThreadsMutex;

	static void* ThreadEntryPoint( void* threadObject );

	/* Register the new created notification port thread. */
	static void registerNotificationPortThread(pthread_t threadHandler);

	/* Unregister the notification port thread. */
	static void unregisterNotificationPortThread(pthread_t threadHandler);

    /* Private constructor in order to avoid construction by the client. */
	NotificationPort(void);

	/* Private destructor in order to avoid destruction by the client. It will destruct itself when the thread will finish its life. */
	virtual ~NotificationPort(void);

	/* Init the data structures.
	Initialize and start the responsible thread.
	Call this function only once after constructor.
	Do not call addBarrier() or quit() before init() function completed. */
	int initialize(uint16_t maxBarriers);

	void releaseResources();

	void getFiredCallBacks(unsigned int numSignaled, unsigned int* signaledIndices, vector<notificationPackage>& callBacksRet, bool* workerThreadSignaled);

	void resizeBuffers(vector<notificationPackage>* fireCallBacksArr, unsigned int** firedIndicesArr, size_t minimumResize);

};

}}}
