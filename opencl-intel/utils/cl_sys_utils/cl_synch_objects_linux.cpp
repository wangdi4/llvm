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
#include <tbb/concurrent_queue.h>
#include <pthread.h>
#include <malloc.h>
#include <stdio.h>  // Todo: replace printf with log mechanisem
#include <assert.h>
#include <algorithm>
#include <semaphore.h>
#include "hw_utils.h"
/************************************************************************
 * This file is the Linux implementation of the cl_synch_objects interface
 ************************************************************************/
using namespace Intel::OpenCL::Utils;

/************************************************************************
 * Creates the mutex section object.
 ************************************************************************/
OclMutex::OclMutex(unsigned int uiSpinCount) : m_uiSpinCount(uiSpinCount)
{
    m_mutexHndl = new pthread_mutex_t;
    if (0 != pthread_mutex_init((pthread_mutex_t*)m_mutexHndl, NULL))
    {
	assert(0 && "Failed initialize pthread mutex");
    }
}

/************************************************************************
 * Destroys the critical section object.
 ************************************************************************/
OclMutex::~OclMutex()
{
    if (0 != pthread_mutex_destroy((pthread_mutex_t*)m_mutexHndl))
    {
	assert(0 && "Failed destroy pthread mutex");
    }
    delete((pthread_mutex_t*)m_mutexHndl);
    m_mutexHndl = NULL;
}

/************************************************************************
 * Take the lock on this critical section.
 * If lock is acquired, all other threads are blocked on this lock until
 * the current thread unlocked it.
 ************************************************************************/
void OclMutex::Lock()
{
    spinCountMutexLock();
}
/************************************************************************
 * Release the lock
 ************************************************************************/
void OclMutex::Unlock()
{
    pthread_mutex_unlock((pthread_mutex_t*)m_mutexHndl);
}

void OclMutex::spinCountMutexLock()
{
    int err = 0;
    unsigned int i = 0;
    do
	{
	    err = pthread_mutex_trylock((pthread_mutex_t*)m_mutexHndl);
	    // Mutex lock succeded.
	    if (err == 0)
	    {
	        return;
	    }
	    // The mutex could not be acquired because it was already locked.
	    if (err == EBUSY)
	    {
            // In order to improve the performance of spin-wait loops.
            hw_pause();
	    }
	    i++;
    } while (i < m_uiSpinCount);
    pthread_mutex_lock((pthread_mutex_t*)m_mutexHndl);
}

/************************************************************************
 *
 ************************************************************************/
OclSpinMutex::OclSpinMutex()
{
	lMutex.exchange(0);
	threadId = -1; // On WIN32 implemetation you initiate threadId as 0, on Linux threadID can be any non_negative integer so it prefered to initiate it as -1 in order to prevent defects.
}
void OclSpinMutex::Lock()
{
	if (pthread_equal(threadId, pthread_self()) != 0)
	{
		++lMutex;
		return;
	}
	while (lMutex.test_and_set(0, 1))   // CAS(*ptr, old, new)
	{
		// In order to improve the performance of spin-wait loops.
		hw_pause();
	}
	threadId = pthread_self();
}
void OclSpinMutex::Unlock()
{
	//Prevent a thread that doesn't own the mutex from unlocking it
	if (pthread_equal(threadId, pthread_self()) == 0)
	{
		return;
	}
	if ( 1 == (long)lMutex )
	{
		threadId = -1;
		lMutex.exchange(0);
		return;
	}
	--lMutex;
	assert(lMutex > 0);
}

/************************************************************************
 *
 ************************************************************************/
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
 ************************************************************************/
OclCondition::OclCondition():
m_ulNumWaiters(0)
{
	assert(0 && "AdirD Not implemented in Linux");

#if defined(WIN32)

    // Signal event is an auto-reset event, only 1 thread released.
    // Fairness is not guaranteed,
    m_signalEvent = (void*)CreateEvent(NULL,FALSE,FALSE,NULL);

    // Broadcast event is a manual-reset event. When signaled all threads are released
    m_broadcastEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

#endif

}

/************************************************************************
 * Condition distructor must be called when there are no threads waiting
 * on this condition.
 * Else, the behavior is undefined.
 ************************************************************************/
OclCondition::~OclCondition()
{

	assert(0 && "AdirD Not implemented in Linux");

#if defined(WIN32)

    assert(m_ulNumWaiters == 0);
    // Free all ...
    CloseHandle(m_signalEvent);
    CloseHandle(m_broadcastEvent);
    m_signalEvent = NULL;
    m_broadcastEvent = NULL;

#endif
}

/************************************************************************
*
 ************************************************************************/
COND_RESULT OclCondition::Wait(IMutex* mutexObj)
{

	assert(0 && "AdirD Not implemented in Linux");

    COND_RESULT res = COND_RESULT_OK;
#if defined(WIN32)

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
#endif
    return res;
}

/************************************************************************
*
 ************************************************************************/
COND_RESULT OclCondition::Signal()
{

	assert(0 && "AdirD Not implemented in Linux");

#if defined(WIN32)

    OclAutoMutex lock(&m_numWaitersMutex);
    if( m_ulNumWaiters > 0 )
    {
        // There are waiters
        SetEvent((HANDLE)m_signalEvent);
    }
    return COND_RESULT_OK;

#else
    return COND_RESULT_OK;
#endif
}

/************************************************************************
*
 ************************************************************************/
COND_RESULT OclCondition::Broadcast()
{

	assert(0 && "AdirD Not implemented in Linux");

#if defined(WIN32)

    OclAutoMutex lock(&m_numWaitersMutex);
    if(m_ulNumWaiters > 0 )
    {
        // There are waiters
        SetEvent((HANDLE)m_broadcastEvent);
    }
    return COND_RESULT_OK;

#else
    return COND_RESULT_OK;
#endif
}

/************************************************************************
* OclOsDependentEvent implementation
 ************************************************************************/
typedef struct event_Structure
{
	pthread_mutex_t mutex;
	pthread_cond_t condition;
	volatile bool isFired;
} EVENT_STRUCTURE;

OclOsDependentEvent::OclOsDependentEvent() : m_eventRepresentation(NULL)
{
}

OclOsDependentEvent::~OclOsDependentEvent()
{
	EVENT_STRUCTURE* pEvent = static_cast<EVENT_STRUCTURE*>(m_eventRepresentation);
	if (pEvent != NULL)
	{
		pthread_cond_destroy(&(pEvent->condition));
		pthread_mutex_destroy(&(pEvent->mutex));
		delete pEvent;
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
	EVENT_STRUCTURE* pEvent = new EVENT_STRUCTURE;
	if (! pEvent)
	{
		return false;
	}
	pEvent->isFired = false;
  if (0 != pthread_mutex_init(&(pEvent->mutex), NULL))
	{
		delete pEvent;
		return false;
	}
	if (0 != pthread_cond_init(&(pEvent->condition), NULL))
	{
    pthread_mutex_destroy(&(pEvent->mutex));
		delete pEvent;
		return false;
	}
	m_eventRepresentation = pEvent;
	return true;
}

bool OclOsDependentEvent::Wait()
{
	EVENT_STRUCTURE* pEvent = static_cast<EVENT_STRUCTURE*>(m_eventRepresentation);
	if (NULL == pEvent)
	{
		//event not initialized
		return false;
	}
	// The condition variable already signaled.
	// Todo: assumes no reset
	if (pEvent->isFired)
	{
		return true;
	}
	pthread_mutex_lock(&(pEvent->mutex));
	int err = 0;
	while (!pEvent->isFired) //Todo: maybe && err == 0?
	{
		err = pthread_cond_wait(&(pEvent->condition), &(pEvent->mutex));
	}
	pthread_mutex_unlock(&(pEvent->mutex));
	return (err == 0);
}

void OclOsDependentEvent::Signal()
{
	EVENT_STRUCTURE* pEvent = static_cast<EVENT_STRUCTURE*>(m_eventRepresentation);
	if (pEvent != NULL)
	{
		assert((!pEvent->isFired) && "Event already signaled");

		pthread_mutex_lock(&(pEvent->mutex));
		pEvent->isFired = true;
		pthread_cond_broadcast(&(pEvent->condition));
		pthread_mutex_unlock(&(pEvent->mutex));
	}
	else
	{
		assert(m_eventRepresentation && "m_eventRepresentation is NULL pointer");
	}
}

/************************************************************************
* AtomicCounter implementation
 ************************************************************************/
AtomicCounter::operator long() const
{
	return __sync_val_compare_and_swap(const_cast<volatile long*>(&m_val), 0, 0);
}

long AtomicCounter::operator ++() //prefix, returns new val
{
	return __sync_add_and_fetch(&m_val, 1);
}
long AtomicCounter::operator ++(int alwaysZero) //postfix, returns previous val
{
	return __sync_fetch_and_add(&m_val, 1);
}
long AtomicCounter::operator --() //prefix, returns new val
{
	return __sync_sub_and_fetch(&m_val, 1);
}
long AtomicCounter::operator --(int alwaysZero) //postfix, returns previous val
{
	return __sync_fetch_and_sub(&m_val, 1);
}
long AtomicCounter::add(long val)
{
	return __sync_add_and_fetch(&m_val, val);
}
long AtomicCounter::test_and_set(long comparand, long exchange)
{
	return __sync_val_compare_and_swap(&m_val, comparand, exchange);	// CAS(*ptr, old, new)
}


long AtomicCounter::exchange(long val)
{
	return __sync_lock_test_and_set(&m_val, val);
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

void AtomicBitField::init(unsigned int size, bool initVal)
{
	// test if already initialized (by other thread)
	if ((m_oneTimeFlag != 0) || (! __sync_bool_compare_and_swap(&m_oneTimeFlag, 0, 1)))
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
		assert(0 && "Error occured while trying to create bit field array, invalid size");
	}
	m_size = size;
	m_bitField = (long*)malloc(sizeof(long) * m_size);
	if (NULL == m_bitField)
	{
		assert(0 && "Error occured while trying to create bit field array, malloc failed");
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


long AtomicBitField::bitTestAndReset(unsigned int bitNum)
{
	if ((NULL == m_bitField) || ((int)bitNum < 0) || (bitNum >= m_size))
	{
		return -1;
	}
	return __sync_val_compare_and_swap((m_bitField + bitNum), 1, 0);
}

long AtomicBitField::bitTestAndSet(unsigned int bitNum)
{
	if ((NULL == m_bitField) || ((int)bitNum < 0) || (bitNum >= m_size))
	{
		return -1;
	}
	return __sync_val_compare_and_swap((m_bitField + bitNum), 0, 1);
}

OclBinarySemaphore::OclBinarySemaphore()
{
    sem_t* pSemaphore = new sem_t();
    assert(NULL != pSemaphore);

    sem_init(pSemaphore, 0, 0);
    m_semaphore = pSemaphore;
}
OclBinarySemaphore::~OclBinarySemaphore()
{
    if (NULL != m_semaphore)
    {
        delete static_cast<sem_t*>(m_semaphore);
    }
}
void OclBinarySemaphore::Signal()
{
    sem_post((sem_t*)m_semaphore);
}
void OclBinarySemaphore::Wait()
{
    sem_wait((sem_t*)m_semaphore);
}
