/////////////////////////////////////////////////////////////////////////
// cl_synch_objects.cpp
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2013 Intel Corporation All Rights Reserved.
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

#include <tbb/concurrent_queue.h>
#include <windows.h>
#include <stdio.h>  // Todo: replace printf with log mechanisem
#include <assert.h>

#include "cl_synch_objects.h"
#include "hw_utils.h"

/************************************************************************
 * This file is the Windows implementation of the cl_synch_objects interface
/************************************************************************/
using namespace Intel::OpenCL::Utils;

void Intel::OpenCL::Utils::InnerSpinloopImpl()
{
        if (0 == SwitchToThread())
        {
            //0 means no other thread is ready to run
            hw_pause();
        }
}

/************************************************************************
 * Creates the mutex section object.
/************************************************************************/
OclMutex::OclMutex(unsigned int uiSpinCount, bool recursive) : m_uiSpinCount(uiSpinCount), m_bRecursive(recursive)
{
    InitializeCriticalSectionAndSpinCount((LPCRITICAL_SECTION)&m_mutex, uiSpinCount);
}

/************************************************************************
 * Destroys the critical section object.
/************************************************************************/
OclMutex::~OclMutex()
{
    DeleteCriticalSection((LPCRITICAL_SECTION)&m_mutex);
}

/************************************************************************
 * Take the lock on this critical section.
 * If lock is acquired, all other threads are blocked on this lock until
 * the current thread unlocked it.
/************************************************************************/
void OclMutex::Lock()
{
    EnterCriticalSection((LPCRITICAL_SECTION)&m_mutex);
}
/************************************************************************
 * Release the lock
/************************************************************************/
void OclMutex::Unlock()
{
    LeaveCriticalSection((LPCRITICAL_SECTION)&m_mutex);
}

/************************************************************************
 *
/************************************************************************/
OclCondition::OclCondition()
{
	STATIC_ASSERT(sizeof(m_condVar)==sizeof(CONDITION_VARIABLE));
	// Initializing the condition variable
    InitializeConditionVariable((CONDITION_VARIABLE*)&m_condVar);
}

/************************************************************************
 * Condition distructor must be called when there are no threads waiting
 * on this condition.
 * Else, the behavior is undefined.
/************************************************************************/
OclCondition::~OclCondition()
{
}

/************************************************************************
*
/************************************************************************/
COND_RESULT OclCondition::Wait(OclMutex* mutexObj)
{
	assert( NULL!=mutexObj && "mutexObj must be valid object");
	if ( NULL == mutexObj )
    {
        return COND_RESULT_FAIL;
    }
	if (0 == SleepConditionVariableCS((CONDITION_VARIABLE*)&m_condVar, (PCRITICAL_SECTION)&(mutexObj->m_mutex), INFINITE))
	{
		return COND_RESULT_FAIL;
	}
    return COND_RESULT_OK;
}

/************************************************************************
*
/************************************************************************/
COND_RESULT OclCondition::Signal()
{
	WakeAllConditionVariable((CONDITION_VARIABLE*)&m_condVar);
    return COND_RESULT_OK;
}

/************************************************************************
* OclOsDependentEvent implementation
/************************************************************************/
OclOsDependentEvent::OclOsDependentEvent() : m_eventRepresentation(NULL)
{
}

OclOsDependentEvent::OclOsDependentEvent(bool bAutoReset) : m_eventRepresentation(NULL)
{
	Init(bAutoReset);
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

void OclOsDependentEvent::Reset()
{
	if (m_eventRepresentation != NULL)
	{
		ResetEvent(m_eventRepresentation);
	}
	else
	{
		assert(m_eventRepresentation && "m_eventRepresentation is NULL pointer");
	}
}

/************************************************************************
* OclOsDependentEvent implementation
/************************************************************************/
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

/************************************************************************
* AtomicCounter implementation
/************************************************************************/
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

/************************************************************************
* AtomicBitField implementation
/************************************************************************/
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

/************************************************************************
* OclReaderWriterLock implementation
/************************************************************************/
OclReaderWriterLock::OclReaderWriterLock()
{
	STATIC_ASSERT(sizeof(void*)==sizeof(SRWLOCK)); // We assume that SRWLOCK defined as struct{void*}
	InitializeSRWLock((PSRWLOCK)&m_rwLock);
#ifdef _DEBUG
	readEnter = 0;
	writeEnter = 0;
#endif
}

OclReaderWriterLock::~OclReaderWriterLock()
{
#ifdef _DEBUG
	assert( (writeEnter==0) && (readEnter==0) && "Writers or Readers are active in destructor");
#endif
}

void OclReaderWriterLock::EnterRead()
{
	AcquireSRWLockShared((PSRWLOCK)&m_rwLock);
#ifdef _DEBUG
	readEnter++;
	assert( writeEnter == 0 && "No writer is allowed insde EnterRead()");
#endif
}

void OclReaderWriterLock::LeaveRead()
{
#ifdef _DEBUG
	readEnter--;
#endif
	ReleaseSRWLockShared((PSRWLOCK)&m_rwLock);
}

void OclReaderWriterLock::EnterWrite()
{
	AcquireSRWLockExclusive((PSRWLOCK)&m_rwLock);
#ifdef _DEBUG
	writeEnter++;
	assert( (writeEnter == 1) && (readEnter==0) && "Only single writer and no readers are allowed insde EnterWrite()");
#endif
}

void OclReaderWriterLock::LeaveWrite()
{
#ifdef _DEBUG
	writeEnter--;
#endif
	ReleaseSRWLockExclusive((PSRWLOCK)&m_rwLock);
}
