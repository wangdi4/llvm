// Copyright (c) 2006-2013 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly


#pragma once

// C++ implementations should define INT16_MAX macro only when __STDC_LIMIT_MACROS is defined before <stdint.h> is included 
#define __STDC_LIMIT_MACROS

#include <stdint.h>
#include <vector>
#include <utility>
#include <set>

#include <ocl_itt.h>
#include <cl_synch_objects.h>
#include <cl_thread.h>
#include <cl_shared_ptr.hpp>

#include <common/COITypes_common.h>

using namespace std;
using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace MICDevice {

class NotificationPort : private OclThread, public ReferenceCountedObject
{
public:

	PREPARE_SHARED_PTR(NotificationPort)

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

	virtual ~NotificationPort();
	
	/* Factory for notification port.
	   Create new NotificationPort object and call to initialize.
	   maxBarriers - initial size of allocate barriers for the notification port.
	   In case of success - the notification port initialization completes and return pointer to NotificationPort object.
	   In case of failure - return NULL.
	   Do NOT delete this object it will delete itself when the worker thread will finish its work. */
	static SharedPtr<NotificationPort> notificationPortFactory(uint16_t maxBarriers, const ocl_gpa_data* pGPAData);

	ERROR_CODE addBarrier(const COIEVENT& barrier, NotificationPort::CallBack* callBack, void* arg);

	ERROR_CODE release();

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

	bool m_operationMask[AVAILABLE_OPERATIONS_LEN];

	COIEVENT* m_barriers;
	notificationPackage* m_notificationsPackages;

	vector<EventNotificationPackagePair> m_pendingNotificationArr;

	OclMutex m_mutex;

	OclCondition m_clientCond;

	uint16_t m_maxBarriers;
	uint16_t m_waitingSize;

	// Need it in order to enforce right order of callbacks
	uint64_t m_lastCallBackAge;
	
	volatile WORKER_STATE m_workerState;

	const ocl_gpa_data* m_pGPAData;

	RETURN_TYPE_ENTRY_POINT Run();

    /* Private constructor in order to avoid construction by the client. */
	NotificationPort(const ocl_gpa_data* pGPAData);

	/* Init the data structures.
	Initialize and start the responsible thread.
	Call this function only once after constructor.
	Do not call addBarrier() or quit() before init() function completed. */
	int initialize(uint16_t maxBarriers);

	void releaseResources();

	void getFiredCallBacks(unsigned int numSignaled, unsigned int* signaledIndices, notificationPackage* callBacksRet, bool* workerThreadSignaled);

	void resizeBuffers(notificationPackage** fireCallBacksArr, unsigned int** firedIndicesArr, size_t minimumResize);

    class StaticInitializer;
    static StaticInitializer init_statics;

    static void registerNotificationPortThread( THREAD_HANDLE handle );
    static void unregisterNotificationPortThread(THREAD_HANDLE handle );

    typedef std::set<THREAD_HANDLE> THREAD_SET;
    
    static THREAD_SET*     m_NotificationThreadsSet;
    static OclMutex*       m_notificationThreadsMutex;

};

}}}
