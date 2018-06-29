// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <tbb/concurrent_queue.h>
#include <pthread.h>
#include <malloc.h>
#include <stdio.h>  // Todo: replace printf with log mechanisem
#include <assert.h>
#include <algorithm>
#include <semaphore.h>

#include "cl_synch_objects.h"
#include "hw_utils.h"
/************************************************************************
 * This file is the Linux implementation of the cl_synch_objects interface
 ************************************************************************/
using namespace Intel::OpenCL::Utils;

void Intel::OpenCL::Utils::InnerSpinloopImpl()
{
    hw_pause();
}

/************************************************************************
 * Creates the mutex section object.
 ************************************************************************/
OclMutex::OclMutex(unsigned int uiSpinCount, bool recursive) : m_uiSpinCount(uiSpinCount), m_bRecursive(recursive)
{
    pthread_mutexattr_t     attr;
    pthread_mutexattr_t*    p_attr = nullptr;

    if (m_bRecursive)
    {
        p_attr = &attr;
        pthread_mutexattr_init( p_attr );
        pthread_mutexattr_settype( p_attr, PTHREAD_MUTEX_RECURSIVE );
    }

    if (0 != pthread_mutex_init(&m_mutex, p_attr))
    {
        assert(0 && "Failed initialize pthread mutex");
    }
    if (nullptr != p_attr)
    {
        pthread_mutexattr_destroy(p_attr);
    }
}

/************************************************************************
 * Destroys the critical section object.
 ************************************************************************/
OclMutex::~OclMutex()
{
    if (0 != pthread_mutex_destroy(&m_mutex))
    {
        assert(0 && "Failed destroy pthread mutex");
    }
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
    pthread_mutex_unlock(&m_mutex);
}

void OclMutex::spinCountMutexLock()
{
    int err = 0;
    unsigned int i = 0;
    do
    {
        err = pthread_mutex_trylock(&m_mutex);
        // Mutex lock succeded.
        if (err == 0)
        {
            return;
        }
        // The mutex could not be acquired because it was already locked.
        if (err == EBUSY)
        {
            // In order to improve the performance of spin-wait loops.
            InnerSpinloopImpl();
        }
        i++;
    } while (i < m_uiSpinCount);
    pthread_mutex_lock(&m_mutex);
}

/************************************************************************
 *
 ************************************************************************/
OclCondition::OclCondition()
{
	int err = 0;
	err = pthread_cond_init(&m_condVar, nullptr);
	assert(0 == err && "pthread_cond_init failed");
}

/************************************************************************
 * Condition distructor must be called when there are no threads waiting
 * on this condition.
 * Else, the behavior is undefined.
 ************************************************************************/
OclCondition::~OclCondition()
{
	int err = 0;
	err = pthread_cond_destroy(&m_condVar);
	assert(0 == err && "pthread_cond_destroy failed");
}

/************************************************************************
*
 ************************************************************************/
COND_RESULT OclCondition::Wait(OclMutex* mutexObj)
{
	assert( mutexObj && "mutexObj must be valid object");
	if ( nullptr == mutexObj )
    {
        return COND_RESULT_FAIL;
    }
	if (0 != pthread_cond_wait(&m_condVar, &mutexObj->m_mutex))
	{
		return COND_RESULT_FAIL;
	}
    return COND_RESULT_OK;
}

/************************************************************************
*
 ************************************************************************/
COND_RESULT OclCondition::Signal()
{
	if (0 != pthread_cond_broadcast(&m_condVar))
	{
		return COND_RESULT_FAIL;
	}
    return COND_RESULT_OK;
}

/************************************************************************
* OclOsDependentEvent implementation
 ************************************************************************/
OclOsDependentEvent::OclOsDependentEvent()
{
}

OclOsDependentEvent::OclOsDependentEvent(bool bAutoReset)
{
	Init(bAutoReset);
}

OclOsDependentEvent::~OclOsDependentEvent()
{
	pthread_cond_destroy(&m_eventRepresentation.condition);
	pthread_mutex_destroy(&m_eventRepresentation.mutex);
}

bool OclOsDependentEvent::Init(bool bAutoReset /* = false */)
{
	m_eventRepresentation.bAutoReset = bAutoReset;
	m_eventRepresentation.isFired = false;
	if (0 != pthread_mutex_init(&m_eventRepresentation.mutex, nullptr))
	{
		return false;
	}
	if (0 != pthread_cond_init(&m_eventRepresentation.condition, nullptr))
	{
	    pthread_mutex_destroy(&m_eventRepresentation.mutex);
		return false;
	}
	return true;
}

bool OclOsDependentEvent::Wait()
{
	pthread_mutex_lock(&(m_eventRepresentation.mutex));
	int err = 0;
	while (!m_eventRepresentation.isFired) //Todo: maybe && err == 0?
	{
		err = pthread_cond_wait(&m_eventRepresentation.condition, &m_eventRepresentation.mutex);
	}
	if ( m_eventRepresentation.bAutoReset )
	{
		m_eventRepresentation.isFired = false;
	}
	pthread_mutex_unlock(&m_eventRepresentation.mutex);
	return (err == 0);
}

void OclOsDependentEvent::Signal()
{
	assert( (!m_eventRepresentation.isFired) && "Event already signaled" );

	pthread_mutex_lock(&m_eventRepresentation.mutex);
	m_eventRepresentation.isFired = true;
	pthread_cond_broadcast(&m_eventRepresentation.condition);
	pthread_mutex_unlock(&m_eventRepresentation.mutex);
}

void OclOsDependentEvent::Reset()
{
	pthread_mutex_lock(&m_eventRepresentation.mutex);
	m_eventRepresentation.isFired = false;
	pthread_mutex_unlock(&m_eventRepresentation.mutex);
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
    return __sync_val_compare_and_swap(&m_val, comparand, exchange);    // CAS(*ptr, old, new)
}

long AtomicCounter::exchange(long val)
{
    return __sync_lock_test_and_set(&m_val, val);
}

/************************************************************************
* AtomicBitField implementation
 ************************************************************************/
AtomicBitField::AtomicBitField() : m_size(0), m_oneTimeFlag(0), m_isInitialize(false), m_eventLock()
{
    m_bitField = nullptr;
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
    if (nullptr == m_bitField)
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

long AtomicBitField::bitTestAndSet(unsigned int bitNum)
{
    if ((nullptr == m_bitField) || ((int)bitNum < 0) || (bitNum >= m_size))
    {
        return -1;
    }
    return __sync_val_compare_and_swap((m_bitField + bitNum), 0, 1);
}

/************************************************************************
* OclBinarySemaphore implementation
 ************************************************************************/
OclBinarySemaphore::OclBinarySemaphore()
{
    sem_init(&m_semaphore, 0, 0);
}

OclBinarySemaphore::~OclBinarySemaphore()
{
}

void OclBinarySemaphore::Signal()
{
    sem_post(&m_semaphore);
}

void OclBinarySemaphore::Wait()
{
    sem_wait(&m_semaphore);
}

/************************************************************************
* OclReaderWriterLock implementation
************************************************************************/
OclReaderWriterLock::OclReaderWriterLock()
{
    pthread_rwlock_init(&m_rwLock, nullptr);
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
    pthread_rwlock_destroy(&m_rwLock);
}

void OclReaderWriterLock::EnterRead()
{
    pthread_rwlock_rdlock(&m_rwLock);
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
    pthread_rwlock_unlock(&m_rwLock);
}

void OclReaderWriterLock::EnterWrite()
{
    pthread_rwlock_wrlock(&m_rwLock);
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
    pthread_rwlock_unlock(&m_rwLock);
}
