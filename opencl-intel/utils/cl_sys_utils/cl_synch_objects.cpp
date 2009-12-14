/////////////////////////////////////////////////////////////////////////
// cl_synch_objects.cpp
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////
#include "cl_synch_objects.h"
#include <windows.h>
#include <stdio.h>  // Todo: replace printf with log mechanisem
#include <assert.h>
/************************************************************************
 * This file is the Windows implementation of the cl_synch_objects interface
/************************************************************************/
using namespace Intel::OpenCL::Utils;

/************************************************************************
 * Creates the mutex section object.
/************************************************************************/
OclMutex::OclMutex(unsigned int uiSpinCount)
{
    m_mutexHndl = new CRITICAL_SECTION();
    InitializeCriticalSectionAndSpinCount((LPCRITICAL_SECTION)m_mutexHndl, uiSpinCount);
}

/************************************************************************
 * Destroys the critical section object.
/************************************************************************/
OclMutex::~OclMutex()
{
    DeleteCriticalSection((LPCRITICAL_SECTION)m_mutexHndl);    
    delete m_mutexHndl;
    m_mutexHndl = NULL;
}

/************************************************************************
 * Take the lock on this critical section.
 * If lock is acquired, all other threads are blocked on this lock until
 * the current thread unlocked it.
/************************************************************************/
void OclMutex::Lock()
{
    EnterCriticalSection((LPCRITICAL_SECTION)m_mutexHndl);
}
/************************************************************************
 * Release the lock
/************************************************************************/
void OclMutex::Unlock()
{
    LeaveCriticalSection((LPCRITICAL_SECTION)m_mutexHndl);
}

/************************************************************************
 * Creates a named Mutex.
 * First call with a unique name will create the handle.
 * Any other call with the same name creates the Mutex with the original handle.
/************************************************************************/
OclNameMutex::OclNameMutex(const char* mutexUniqueName)
{
    // Creates a Win32 API named Mutex without initial owner.
    m_mutexHndl = CreateMutex( 
        NULL,   // default security attributes
        FALSE,  // initially not owned
        mutexUniqueName  // name
        );
}

/************************************************************************
 * Destroys the Mutex object
/************************************************************************/
OclNameMutex::~OclNameMutex()
{
    CloseHandle(m_mutexHndl);    
}

/************************************************************************
 * Use this function to acquire the Names Mutex.
 * If a thread locked the object, any other thread will be blocked on this
 * function call until the holder unlock the Mutex.  
 * However, the frame buffer does not been copy.
 * If the holder thread tries to acquire the same Mutex again without releasing
 * it, the behavior is undefined.
/************************************************************************/
void OclNameMutex::Lock()
{
    // Infinite wait to a Mutex release
    DWORD waitStatus = WaitForSingleObject(m_mutexHndl, INFINITE);
    switch(waitStatus)
    {
    case WAIT_OBJECT_0:
        // Great, the current thread acquired the object without any problems.
        break;
    case WAIT_ABANDONED:
        // Potential problem: A thread ended without releasing the Mutex.
        // The resources may not be in stable state. Notify the developer for debugging purposes.
        // LogFatalErorr
        printf("OclNamedMutex Lock got WAIT_ABANDONED. You didn't released the Mutex!\n");
        break;
    default:
        // Any other status is a fatal error.
        // LogFatalErorr
        printf("OclNamedMutex lock fatal error: %d\n", GetLastError());
        exit(1);
        break;
    }
}


/************************************************************************
 * Releases the Mutex
/************************************************************************/
void OclNameMutex::Unlock()
{
    BOOL retVal = ReleaseMutex(m_mutexHndl);
    if(0 == retVal)
    {
        // Print debug error message for debugging. The thread may tried to release object
        // that it doesn't own.
        printf("OclNameMutex release error: %d\n", GetLastError());
    }    
}

/************************************************************************
 * 
/************************************************************************/
OclAutoMutex::OclAutoMutex(IMutex* mutexObj, bool bAutoLock)
{
    m_mutexObj = mutexObj;
	if ( bAutoLock )
	{
		m_mutexObj->Lock();
	}
}

/************************************************************************
 * 
/************************************************************************/
OclAutoMutex::~OclAutoMutex()
{
    m_mutexObj->Unlock();
}


/************************************************************************
 * 
/************************************************************************/
OclCondition::OclCondition():
m_ulNumWaiters(0)
{
    // Signal event is an auto-reset event, only 1 thread released. 
    // Fairness is not guaranteed,
    m_signalEvent = (void*)CreateEvent(NULL,FALSE,FALSE,NULL);

    // Broadcast event is a manual-reset event. When signaled all threads are released
    m_broadcastEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
}

/************************************************************************
 * Condition distructor must be called when there are no threads waiting
 * on this condition.
 * Else, the behavior is undefined.
/************************************************************************/
OclCondition::~OclCondition()
{
    assert(m_ulNumWaiters == 0);
    // Free all ... 
    CloseHandle(m_signalEvent);
    CloseHandle(m_broadcastEvent);
    m_signalEvent = NULL;
    m_broadcastEvent = NULL;
}

/************************************************************************
* 
/************************************************************************/
COND_RESULT OclCondition::Wait(IMutex* mutexObj)
{
    COND_RESULT res = COND_RESULT_OK;
    if(NULL == mutexObj)
    {
        return COND_RESULT_FAIL;
    }
    { OclAutoMutex lock(&m_numWaitersMutex);
        m_ulNumWaiters++;
    }
    // Released the attached Mutex
    mutexObj->Unlock();
    // Wait both for broadcast and single, but act differently.
    DWORD waitRes = WaitForMultipleObjects(2, &m_signalEvent,FALSE, INFINITE);
    // Critical section - check if it is the last waiter on a broadcast
    // If it is, reset the manual-reset event
    bool resetEvent = false;
    { OclAutoMutex lock(&m_numWaitersMutex);
        m_ulNumWaiters--;
        resetEvent = ((m_ulNumWaiters == 0 ) && (waitRes == WAIT_OBJECT_0 + 1)); // Reset event only at the end
    }
    if(waitRes == WAIT_OBJECT_0 + 1)    // Second object is the broadcast event
        res = COND_RESULT_COND_BROADCASTED;
    if(resetEvent)
    {
        ResetEvent(m_broadcastEvent);        
    }
    // Acquire the Mutex
    mutexObj->Lock();
    return res;
}

/************************************************************************
* 
/************************************************************************/
COND_RESULT OclCondition::Signal()
{
    OclAutoMutex lock(&m_numWaitersMutex);
    if( m_ulNumWaiters > 0 )
    {
        // There are waiters
        SetEvent((HANDLE)m_signalEvent);
    }
    return COND_RESULT_OK;
}

/************************************************************************
* 
/************************************************************************/
COND_RESULT OclCondition::Broadcast()
{
    OclAutoMutex lock(&m_numWaitersMutex);
    if(m_ulNumWaiters > 0 )
    {
        // There are waiters
        SetEvent((HANDLE)m_broadcastEvent);
    }
    return COND_RESULT_OK;
}

