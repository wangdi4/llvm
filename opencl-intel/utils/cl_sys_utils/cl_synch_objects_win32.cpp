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
#include "hw_utils.h"

#include <tbb/concurrent_queue.h>
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
 *
/************************************************************************/
OclSpinMutex::OclSpinMutex()
{
	lMutex.exchange(0);
	threadId = 0;
}
void OclSpinMutex::Lock()
{
	if (threadId == GetCurrentThreadId())
	{
		++lMutex;
		return;
	}
	while (lMutex.test_and_set(0, 1))
	{
		// In order to improve the performance of spin-wait loops.
		hw_pause();
	}
	threadId = GetCurrentThreadId();
}
void OclSpinMutex::Unlock()
{
	//Prevent a thread that doesn't own the mutex from unlocking it
	if (GetCurrentThreadId() != threadId)
	{
		return;
	}
	if ( 1 == (long)lMutex )
	{
		threadId = 0;
		lMutex.exchange(0);
		return;
	}
	long val = --lMutex;
	assert(val != -1);
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
	assert(0 && "Invalid condition implementation");
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

/************************************************************************
* OclOsDependentEvent implementation
/************************************************************************/

OclOsDependentEvent::OclOsDependentEvent() : m_eventRepresentation(NULL)
{
}

OclOsDependentEvent::~OclOsDependentEvent()
{
	if (m_eventRepresentation)
	{
		CloseHandle((HANDLE)m_eventRepresentation);
	}
	m_eventRepresentation = NULL;
}

bool OclOsDependentEvent::Init(bool bAutoReset /* = false */)
{
	if (m_eventRepresentation != NULL) //event already initialized
	{
		//Todo: raise error?
		return true;
	}
	m_eventRepresentation = CreateEvent(NULL, !bAutoReset, FALSE, NULL);
	return m_eventRepresentation != NULL;
}

bool OclOsDependentEvent::Wait()
{
	if (NULL == m_eventRepresentation)
	{
		//event not initialized
		return false;
	}
	return WAIT_OBJECT_0 == WaitForSingleObject(m_eventRepresentation, INFINITE);
}

void OclOsDependentEvent::Signal()
{
	if (m_eventRepresentation != NULL)
	{
		SetEvent(m_eventRepresentation);
	}
	else
	{
		assert(m_eventRepresentation && "m_eventRepresentation is NULL pointer");
	}
}

OclBinarySemaphore::OclBinarySemaphore()
{
    m_semaphore = CreateEvent(NULL, false, false, NULL);
}
OclBinarySemaphore::~OclBinarySemaphore()
{
    CloseHandle(m_semaphore);
}
void OclBinarySemaphore::Signal()
{
    SetEvent(m_semaphore);
}
void OclBinarySemaphore::Wait()
{
    WaitForSingleObject(m_semaphore, INFINITE);
}

AtomicCounter::operator long() const
{
	return InterlockedCompareExchange(const_cast<volatile LONG*>(&m_val), 0, 0);
}

long AtomicCounter::operator ++() //prefix, returns new val
{
	return InterlockedIncrement(&m_val);
}
long AtomicCounter::operator ++(int alwaysZero) //postfix, returns previous val
{
	return InterlockedExchangeAdd(&m_val, 1);
}
long AtomicCounter::operator --() //prefix, returns new val
{
	return InterlockedDecrement(&m_val);
}
long AtomicCounter::operator --(int alwaysZero) //postfix, returns previous val
{
	return InterlockedExchangeAdd(&m_val, -1);
}
long AtomicCounter::add(long val)
{
	return InterlockedExchangeAdd(&m_val, val) + val;
}
long AtomicCounter::test_and_set(long comparand, long exchange)
{
	return InterlockedCompareExchange(&m_val, exchange, comparand);
}


long AtomicCounter::exchange(long val)
{
	return InterlockedExchange(&m_val, val);
}


AtomicBitField::AtomicBitField() : m_size(0), m_oneTimeFlag(0), m_isInitialize(false), m_eventLock()
{
	m_bitField = NULL;
	m_eventLock.Init(false);
}


AtomicBitField::~AtomicBitField()
{
	if (m_bitField)
	{
		free(m_bitField);
	}
}

// Disable std::fill_n warning
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(push) 
#pragma warning(disable:4996) 
#endif 

void AtomicBitField::init(unsigned int size, bool initVal)
{
	// test if already initialized (by other thread)
	if ((m_oneTimeFlag != 0) || (InterlockedCompareExchange(&m_oneTimeFlag, 1, 0) != 0))
	{
		if (m_isInitialize)
		{
			return;
		}
        m_eventLock.Wait();
		return;
	}
	if (size <= 0)
	{
		assert(0 && "Error occurred while trying to create bit field array, invalid size");
	}
	m_size = size;
	m_bitField = (long*)malloc(sizeof(long) * m_size);
	if (NULL == m_bitField)
	{
		assert(0 && "Error occurred while trying to create bit field array, malloc failed");
		m_eventLock.Signal();
		return;
	}
	if (initVal)
	{
		std::fill_n(m_bitField, m_size, 1);
	}
	else
	{
		memset(m_bitField, 0, sizeof(long) * m_size);
	}
	m_isInitialize = true;
	m_eventLock.Signal();
}

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(pop) 
#endif 

long AtomicBitField::bitTestAndReset(unsigned int bitNum)
{
	if ((NULL == m_bitField) || (bitNum < 0) || (bitNum >= m_size))
	{
		return -1;
	}
	return InterlockedCompareExchange((m_bitField + bitNum), 0, 1);
}

long AtomicBitField::bitTestAndSet(unsigned int bitNum)
{
	if ((NULL == m_bitField) || (bitNum < 0) || (bitNum >= m_size))
	{
		return -1;
	}
	return InterlockedCompareExchange((m_bitField + bitNum), 1, 0);
}

