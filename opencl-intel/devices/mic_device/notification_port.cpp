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

#include "notification_port.h"

#include <common/COIEvent_common.h>
#include <source/COIEvent_source.h>

#include <malloc.h>
#include <cstring>
#include <algorithm>
#include <signal.h>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;

#define CALL_BACKS_ARRAY_RESIZE_AMOUNT 1024
#define MAX_WAITING_EVENTS 16384

NotificationPort::THREAD_SET* NotificationPort::m_NotificationThreadsSet   = nullptr;
OclMutex*                     NotificationPort::m_notificationThreadsMutex = nullptr;

//
// Helper class
//
class NotificationPort::StaticInitializer
{
public:
    StaticInitializer()
    {
        m_NotificationThreadsSet   = new THREAD_SET;
        m_notificationThreadsMutex = new OclMutex;
    };

    void Release()
    {
        if (nullptr != m_notificationThreadsMutex)
        {
            delete m_notificationThreadsMutex;
            m_notificationThreadsMutex = nullptr;

            delete m_NotificationThreadsSet;
            m_NotificationThreadsSet = nullptr;
        }
    };
};

// init all static classes
NotificationPort::StaticInitializer NotificationPort::init_statics;

void NotificationPort::registerNotificationPortThread( THREAD_HANDLE handle )
{
    // Add the new thread to the live threads set.
    OclAutoMutex lock(m_notificationThreadsMutex);
    m_NotificationThreadsSet->insert(handle);
}

void NotificationPort::unregisterNotificationPortThread( THREAD_HANDLE handle )
{
    // Remove the thread from the live threads set.
    OclAutoMutex lock(m_notificationThreadsMutex);
    m_NotificationThreadsSet->erase(handle);
}

void NotificationPort::waitForAllNotificationPortThreads()
{
    vector<THREAD_HANDLE> liveThreadsSnapshot;

    if (nullptr == m_notificationThreadsMutex)
    {
        return;
    }

    {
        // Get snapshot of the current threads alive.
        OclAutoMutex lock(m_notificationThreadsMutex);
        liveThreadsSnapshot.insert(liveThreadsSnapshot.begin(), m_NotificationThreadsSet->begin(), m_NotificationThreadsSet->end());
    }
    while (liveThreadsSnapshot.size() > 0)
    {
        // Waits for all threads.
        for (unsigned int i = 0; i < liveThreadsSnapshot.size(); i++)
        {
            OclThread::WaitForOsThreadCompletion( liveThreadsSnapshot[i] );
        }

        liveThreadsSnapshot.clear();

        {
            // Get snapshot of the current threads alive.
            OclAutoMutex lock(m_notificationThreadsMutex);
            liveThreadsSnapshot.insert(liveThreadsSnapshot.begin(), m_NotificationThreadsSet->begin(), m_NotificationThreadsSet->end());
        }
    }

    init_statics.Release();

}

NotificationPort::NotificationPort(const ocl_gpa_data* pGPAData) :
    m_barriers(nullptr), m_notificationsPackages(nullptr), m_maxBarriers(0),
    m_waitingSize(0), m_lastCallBackAge(0), m_workerState(CREATED),
    m_pGPAData(pGPAData)
{
}

NotificationPort::~NotificationPort(void)
{
}

SharedPtr<NotificationPort> NotificationPort::notificationPortFactory(uint16_t maxBarriers, const ocl_gpa_data* pGPAData)
{
    NotificationPort* retNotificationPort = new NotificationPort(pGPAData);
    if (nullptr == retNotificationPort)
    {
        return nullptr;
    }

    int err = retNotificationPort->initialize(maxBarriers);
    if (SUCCESS == err)
    {
        // Increase the reference counter by 1 in order to delete NotificationPort objetct only after the thread function completes.
        // Otherwise can be double delete in case that the notification port thread call to release() and after that delete its object.
        retNotificationPort->IncRefCnt();
    }
    else
    {
        delete retNotificationPort;
        retNotificationPort = nullptr;
    }

    return retNotificationPort;
}

int NotificationPort::initialize(uint16_t maxBarriers)
{
    assert(maxBarriers < INT16_MAX && "maxBarriers must be smaller than INT16_MAX");
    OclAutoMutex lock(&m_mutex);
    // if initialize already called after constructor or release operation
    if (m_maxBarriers > 0)
    {
        return ALREADY_INITIALIZE_FAILURE;
    }
    // Reserve "maxBarriers" to "m_pendingNotificationArr". (The size of the vector is at least "maxBarriers")
    m_pendingNotificationArr.reserve(maxBarriers);
    // Add 1 barrier for the main barrier (The first barrier)
    m_maxBarriers = maxBarriers + 1;

    m_barriers = nullptr;
    m_notificationsPackages = nullptr;

    m_barriers = (COIEVENT*)malloc(m_maxBarriers * sizeof(COIEVENT));
    if (!m_barriers)
    {
        releaseResources();
        return MEM_OBJECT_ALLOCATION_FAILURE;
    }

    m_notificationsPackages = (notificationPackage*)malloc(m_maxBarriers * sizeof(notificationPackage));
    if (!m_notificationsPackages)
    {
        releaseResources();
        return MEM_OBJECT_ALLOCATION_FAILURE;
    }

    // reset the operation mask
    memset(m_operationMask, 0, sizeof(bool) * AVAILABLE_OPERATIONS_LEN);

    // create the main thread barrier
    COIRESULT result = COIEventRegisterUserEvent(m_barriers);

    if (result != COI_SUCCESS)
    {
        releaseResources();
        return CREATE_BARRIER_FAILURE;
    }

    // waiting size = 1 becuase currently it waits only on the main barrier.
    m_waitingSize = 1;

    m_workerState = BEGINNING;

    if (THREAD_RESULT_SUCCESS != Start())
    {
        releaseResources();
        return CREATE_WORKER_THREAD_FAILURE;
    }

    // wait until the worker thread will start execution
    while (BEGINNING == m_workerState)
    {
        // lock and mutex refer to the same mutex.
        m_clientCond.Wait(&m_mutex);
    }

    return SUCCESS;
}

NotificationPort::ERROR_CODE NotificationPort::addBarrier(const COIEVENT &barrier, NotificationPort::CallBack *callBack, void *arg)
{
    COIRESULT result = COI_SUCCESS;
    ERROR_CODE return_value = SUCCESS;

    assert(callBack && "Error - callBack must be non-NULL pointer");
    OclAutoMutex lock(&m_mutex);

    // The object does not initialize
    if (m_maxBarriers == 0)
    {
        return NOT_INITIASLIZE_FAILURE;
    }

    // Set the new pending notification
    EventNotificationPackagePair tNotification;
    tNotification.first = barrier;
    tNotification.second.callBack = callBack;
    tNotification.second.arg = arg;
    tNotification.second.age = m_lastCallBackAge;

    // Push the new notification to the pending list
    m_pendingNotificationArr.push_back(tNotification);

    m_lastCallBackAge ++;

    if (1 == m_pendingNotificationArr.size())
    {
        m_operationMask[ADD] = true;
        result = COIEventSignalUserEvent(m_barriers[0]);
        assert(result == COI_SUCCESS && "Signal main barrier failed");
        if (COI_SUCCESS != result)
        {
            return_value = CREATE_BARRIER_FAILURE;
       }
    }

    return return_value;
}

NotificationPort::ERROR_CODE NotificationPort::release()
{
    {
        OclAutoMutex lock(&m_mutex);
        COIRESULT result = COI_SUCCESS;

        // The object does not initialize
        if (m_maxBarriers == 0)
        {
            return NOT_INITIASLIZE_FAILURE;
        }

        m_operationMask[RELEASE] = true;

        result = COIEventSignalUserEvent(m_barriers[0]);
        assert(result == COI_SUCCESS && "Signal main barrier failed");
    }

    // wait for the termination of worker thread (If the calling thread is NOT the worker thread)
    if (!isSelf())
    {
        WaitForCompletion();
    }

    return SUCCESS;
}

RETURN_TYPE_ENTRY_POINT NotificationPort::Run()
{
    bool keepWork = true;

    notificationPackage* fireCallBacksArr;
    fireCallBacksArr = (notificationPackage*)malloc(sizeof(notificationPackage) * m_maxBarriers);
    unsigned int* firedIndicesArr = (unsigned int*)malloc(sizeof(unsigned int) * m_maxBarriers);
    assert(firedIndicesArr && "memory allocation for firedIndicesArr failed");
    unsigned int firedAmount;
    bool workerThreadSigaled;

    THREAD_HANDLE myHandle = GetThreadHandle();

    COIRESULT result;
    
    // Register the new thread
    registerNotificationPortThread(myHandle);

    // Block the main thread until this point in order to ensure that the thread begin it's life before initialize() method completed
    {
        OclAutoMutex lock(&m_mutex);
        m_workerState = RUNNING;
        m_clientCond.Signal();
    }

    while (keepWork)
    {
        firedAmount = 0;
        workerThreadSigaled = false;

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
        {
            static __thread __itt_string_handle* pTaskName = nullptr;
            if ( nullptr == pTaskName )
            {
                pTaskName = __itt_string_handle_create("NotificationPort::Run()->COIEventWait()");
            }
        __itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, pTaskName);
        }
#endif
        // wait for barrier(s) signal(s)
        result = COIEventWait(m_waitingSize, m_barriers, -1, false, &firedAmount, firedIndicesArr);
        assert(result == COI_SUCCESS && "COIBarrierWait failed for some reason");
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
        {
            __itt_task_end(m_pGPAData->pDeviceDomain);
        }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
        {
            static __thread __itt_string_handle* pTaskName = nullptr;
            if ( nullptr == pTaskName )
            {
                pTaskName = __itt_string_handle_create("NotificationPort::Run()->ProcessNotification...");
            } 
            __itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, pTaskName);
        }
#endif
        {
            OclAutoMutex lock(&m_mutex);

            // get all the signaled barriers.
            getFiredCallBacks(firedAmount, firedIndicesArr, fireCallBacksArr, &workerThreadSigaled);

            // If the main thread signaled
            if (workerThreadSigaled)
            {
                result = COIEventUnregisterUserEvent(m_barriers[0]);
                assert(result == COI_SUCCESS && "UnRegister main barrier failed");
                result = COIEventRegisterUserEvent(&(m_barriers[0]));
                assert(result == COI_SUCCESS && "Register main barrier failed");
                firedAmount --;
                // If Add barrier operation
                if (m_operationMask[ADD] == true)
                {
                    size_t pendingNotificationsAmount = m_pendingNotificationArr.size();
                    // If there is no enough space, should resize the buffers "m_barriers" and "m_notificationsPackages".
                    if ((m_waitingSize + pendingNotificationsAmount) >= m_maxBarriers)
                    {
                        resizeBuffers(&fireCallBacksArr, &firedIndicesArr, pendingNotificationsAmount);
                    }
                    addPendingEvents();
                    m_operationMask[ADD] = false;
                }
                // If Release operation
                if (m_operationMask[RELEASE] == true)
                {
                    keepWork = false;
                }
            }
            else if (m_pendingNotificationArr.size() > 0)
            {
                // We have pending events because we couldn't resize the events array.
                addPendingEvents();
            }

        } //end of m_mutex.lock()

        // If more than one element fired, sort the fired element according to their age
        if (firedAmount > 1)
        {
            sort(&fireCallBacksArr[0], &fireCallBacksArr[firedAmount], notificationPackage::compare);
        }

        for (unsigned int i = 0; i < firedAmount; i++)
        {
            fireCallBacksArr[i].callBack->fireCallBack(fireCallBacksArr[i].arg);
        }
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
        {
            __itt_task_end(m_pGPAData->pDeviceDomain);
        }
#endif

    }

    releaseResources();

    free(firedIndicesArr);
    free(fireCallBacksArr);

    // Decrement the reference counter of this object (We did ++ in the factory)
    // It can couse destruction of this object.
    DecRefCnt();

    // Unregister this thread.
    unregisterNotificationPortThread(myHandle);

    // AdirD - TODO change the selfTerminate to static function
    RETURN_TYPE_ENTRY_POINT exitCode = nullptr;
    SelfTerminate(exitCode);

    return exitCode;
}

void NotificationPort::getFiredCallBacks(unsigned int numSignaled, unsigned int* signaledIndices, notificationPackage* callBacksRet, bool* workerThreadSignaled)
{
    unsigned int notSignaledIndex = m_waitingSize - 1;

    unsigned int index = 0;
    for (unsigned int i = 0; i < numSignaled; i++)
    {
        // if worker thread signaled
        if (signaledIndices[i] == 0)
        {
            *workerThreadSignaled = true;
            continue;
        }
        // save the notification package
        callBacksRet[index] = m_notificationsPackages[signaledIndices[i]];
        index ++;
        // signature which give hint that the barrier signaled. (hint for the swaps in the next loop)
        m_notificationsPackages[signaledIndices[i]].callBack = nullptr;
    }

    // swap signaled barriers with non-signaled
    for (unsigned int i = 0; i < numSignaled; i++)
    {
        // if worker thread signaled
        if (signaledIndices[i] == 0)
        {
            continue;
        }
        // while barrier in location notSignaledIndex is fired and notSignaledIndex >= current barrier index
        while ((notSignaledIndex >= signaledIndices[i]) && (m_notificationsPackages[notSignaledIndex].callBack == nullptr))
        {
            notSignaledIndex --;
        }
        if (notSignaledIndex > signaledIndices[i])
        {
            // switch between the current barrier and the last not signaled barrier.
            m_barriers[signaledIndices[i]] = m_barriers[notSignaledIndex];
            m_notificationsPackages[signaledIndices[i]] = m_notificationsPackages[notSignaledIndex];

            notSignaledIndex --;
        }

        m_waitingSize --;
    }
}


void NotificationPort::resizeBuffers(notificationPackage** fireCallBacksArr, unsigned int** firedIndicesArr, size_t minimumResize)
{
    assert(m_maxBarriers <= MAX_WAITING_EVENTS && "m_maxBarriers must be <= MAX_WAITING_EVENTS");
    if (m_maxBarriers == MAX_WAITING_EVENTS)
    {
        return;
    }
    size_t resizeAmount = (((minimumResize / CALL_BACKS_ARRAY_RESIZE_AMOUNT) + 1) * CALL_BACKS_ARRAY_RESIZE_AMOUNT);
    if (MAX_WAITING_EVENTS > ((size_t)m_maxBarriers + resizeAmount))
    {
        m_maxBarriers += (uint16_t)resizeAmount;
    }
    else
    {
        m_maxBarriers = MAX_WAITING_EVENTS;
    }
    assert(m_maxBarriers <= MAX_WAITING_EVENTS && "m_maxBarriers must be <= MAX_WAITING_EVENTS");
    m_barriers = (COIEVENT*)realloc(m_barriers, m_maxBarriers * sizeof(COIEVENT));
    assert(m_barriers && "memory allocation failed for m_barriers");
    m_notificationsPackages = (notificationPackage*)realloc(m_notificationsPackages, m_maxBarriers * sizeof(notificationPackage));
    assert(m_notificationsPackages && "memory allocation failed for m_notificationsPackages");
    *fireCallBacksArr = (notificationPackage*)realloc(*fireCallBacksArr, sizeof(notificationPackage) * m_maxBarriers);
    assert(*fireCallBacksArr && "memory allocation failed for *fireCallBacksArr");
    *firedIndicesArr = (unsigned int*)realloc(*firedIndicesArr, sizeof(unsigned int) * m_maxBarriers);
    assert(*firedIndicesArr && "memory allocation failed for *firedIndicesArr");
}

void NotificationPort::addPendingEvents()
{
    size_t pendingNotificationsAmount = m_pendingNotificationArr.size();
    size_t numBarriersToAdd = MIN(pendingNotificationsAmount, (size_t)(m_maxBarriers - m_waitingSize));
    if (0 == numBarriersToAdd)
    {
        return;
    }
    for (unsigned int i = 0; i < numBarriersToAdd; i++)
    {
        // Add the pending notification to the real waiting list.
        m_barriers[m_waitingSize] = ((m_pendingNotificationArr)[i]).first;
        m_notificationsPackages[m_waitingSize] = ((m_pendingNotificationArr)[i]).second;
        m_waitingSize ++;
    }
    if (numBarriersToAdd == pendingNotificationsAmount)
    {
        m_pendingNotificationArr.clear();
    }
    else
    {
        m_pendingNotificationArr.erase(m_pendingNotificationArr.begin(), m_pendingNotificationArr.begin() + numBarriersToAdd);
    }
    assert((m_pendingNotificationArr.size() == (pendingNotificationsAmount - numBarriersToAdd)) && "ERROR m_pendingNotificationArr state is not valid");
}


void NotificationPort::releaseResources()
{
    COIRESULT result = COI_SUCCESS;

    OclAutoMutex lock(&m_mutex);

    m_maxBarriers = 0;
    //release the worker barrier
    if (m_barriers)
    {
        result = COIEventUnregisterUserEvent(m_barriers[0]);
        assert(result == COI_SUCCESS && "Unregister main barrier failed");
        free(m_barriers);
        m_barriers = nullptr;
    }
    if (m_notificationsPackages)
    {
        free(m_notificationsPackages);
        m_notificationsPackages = nullptr;
    }
    m_waitingSize = 0;

    m_workerState = FINISHED;
    m_clientCond.Signal();
}
