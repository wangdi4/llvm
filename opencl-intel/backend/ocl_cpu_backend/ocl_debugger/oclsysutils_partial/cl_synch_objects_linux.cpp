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
#include <pthread.h>
#include <malloc.h>
#include <stdio.h>  // Todo: replace printf with log mechanisem
#include <assert.h>
#include <algorithm>
#include <semaphore.h>
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
	pthread_mutex_destroy((pthread_mutex_t*)m_mutexHndl);
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

/************************************************************************
* OclOsDependentEvent implementation
 ************************************************************************/
typedef struct event_Structure
{
	pthread_mutex_t mutex;
	pthread_cond_t condition;
	bool isFired;
} EVENT_STRUCTURE;

OclOsDependentEvent::OclOsDependentEvent() : m_eventRepresentation(NULL)
{
}

OclOsDependentEvent::~OclOsDependentEvent()
{
	if (m_eventRepresentation)
	{
		pthread_mutex_destroy(&(((EVENT_STRUCTURE*)m_eventRepresentation)->mutex));
		pthread_cond_destroy(&(((EVENT_STRUCTURE*)m_eventRepresentation)->condition));
		delete((EVENT_STRUCTURE*)m_eventRepresentation);
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
	m_eventRepresentation = new EVENT_STRUCTURE;
	if (! m_eventRepresentation)
	{
		return false;
	}
	((EVENT_STRUCTURE*)m_eventRepresentation)->isFired = false;
    if (0 != pthread_mutex_init(&(((EVENT_STRUCTURE*)m_eventRepresentation)->mutex), NULL))
	{
		delete((EVENT_STRUCTURE*)m_eventRepresentation);
		m_eventRepresentation = NULL;
		return false;
	}
	if (0 != pthread_cond_init(&(((EVENT_STRUCTURE*)m_eventRepresentation)->condition), NULL))
	{
        pthread_mutex_destroy(&(((EVENT_STRUCTURE*)m_eventRepresentation)->mutex));
		delete((EVENT_STRUCTURE*)m_eventRepresentation);
		m_eventRepresentation = NULL;
		return false;
	}
	return true;
}

bool OclOsDependentEvent::Wait()
{
	if (NULL == m_eventRepresentation)
	{
		//event not initialized
		return false;
	}
	pthread_mutex_lock(&(((EVENT_STRUCTURE*)m_eventRepresentation)->mutex));
	int err = pthread_cond_wait(&(((EVENT_STRUCTURE*)m_eventRepresentation)->condition), &(((EVENT_STRUCTURE*)m_eventRepresentation)->mutex));
		
    if (err == 0) {
		// Always auto-reset
		((EVENT_STRUCTURE*)m_eventRepresentation)->isFired = false;
	}
	pthread_mutex_unlock(&(((EVENT_STRUCTURE*)m_eventRepresentation)->mutex));
	return (err == 0);
}

void OclOsDependentEvent::Signal()
{
	if (m_eventRepresentation != NULL)
	{
		assert(((EVENT_STRUCTURE*)m_eventRepresentation)->isFired == false && "Event already signaled");

		pthread_mutex_lock(&(((EVENT_STRUCTURE*)m_eventRepresentation)->mutex));
		((EVENT_STRUCTURE*)m_eventRepresentation)->isFired = true;
		pthread_cond_broadcast(&(((EVENT_STRUCTURE*)m_eventRepresentation)->condition));
		pthread_mutex_unlock(&(((EVENT_STRUCTURE*)m_eventRepresentation)->mutex));
	}
	else
	{
		assert(m_eventRepresentation && "m_eventRepresentation is NULL pointer");
	}
}

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


