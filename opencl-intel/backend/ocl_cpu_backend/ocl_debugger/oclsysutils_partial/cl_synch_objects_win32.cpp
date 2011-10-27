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

