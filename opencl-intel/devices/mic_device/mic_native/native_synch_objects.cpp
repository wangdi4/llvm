/////////////////////////////////////////////////////////////////////////
// native_synch_objects.cpp
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
#include "pragmas.h"
#include "native_synch_objects.h"
#include <pthread.h>
#include <stdio.h>  // Todo: replace printf with log mechanisem
#include <assert.h>
#include <errno.h>
#include "native_common_macros.h"

/************************************************************************
 * This file is the Linux implementation of the cl_synch_objects interface
 ************************************************************************/
using namespace Intel::OpenCL::UtilsNative;

/************************************************************************
 * Creates the mutex section object.
 ************************************************************************/
OclMutexNative::OclMutexNative(unsigned int uiSpinCount) : m_uiSpinCount(uiSpinCount)
{
    if (0 != pthread_mutex_init(&m_mutexHndl, NULL))
    {
        assert(0 && "Failed initialize pthread mutex");
    }
}

/************************************************************************
 * Destroys the critical section object.
 ************************************************************************/
OclMutexNative::~OclMutexNative()
{
    if (0 != pthread_mutex_destroy(&m_mutexHndl))
    {
        assert(0 && "Failed destroy pthread mutex");
    }
}

/************************************************************************
 * Take the lock on this critical section.
 * If lock is acquired, all other threads are blocked on this lock until
 * the current thread unlocked it.
 ************************************************************************/
void OclMutexNative::Lock()
{
    spinCountMutexLock();
}
/************************************************************************
 * Release the lock
 ************************************************************************/
void OclMutexNative::Unlock()
{
    pthread_mutex_unlock(&m_mutexHndl);
}

void OclMutexNative::spinCountMutexLock()
{
    int err = 0;
    unsigned int i = 0;
    do
    {
        err = pthread_mutex_trylock(&m_mutexHndl);
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
    pthread_mutex_lock(&m_mutexHndl);
}

/************************************************************************
 *
 ************************************************************************/
OclSpinMutexNative::OclSpinMutexNative()
{
    lMutex.exchange(0);
    threadId = -1; // On WIN32 implemetation you initiate threadId as 0, on Linux threadID can be any non_negative integer so it prefered to initiate it as -1 in order to prevent defects.
}
void OclSpinMutexNative::Lock()
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
void OclSpinMutexNative::Unlock()
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
OclAutoMutexNative::OclAutoMutexNative(IMutex* mutexObj, bool bAutoLock)
{
    m_mutexObj = mutexObj;
    if ( bAutoLock )
    {
        m_mutexObj->Lock();
    }
}

OclAutoMutexNative::~OclAutoMutexNative()
{
    m_mutexObj->Unlock();
}

long AtomicCounterNative::operator ++() //prefix, returns new val
{
    return __sync_add_and_fetch(&m_val, 1);
}
long AtomicCounterNative::operator ++(int alwaysZero) //postfix, returns previous val
{
    return __sync_fetch_and_add(&m_val, 1);
}
long AtomicCounterNative::operator --() //prefix, returns new val
{
    return __sync_sub_and_fetch(&m_val, 1);
}
long AtomicCounterNative::operator --(int alwaysZero) //postfix, returns previous val
{
    return __sync_fetch_and_sub(&m_val, 1);
}
long AtomicCounterNative::add(long val)
{
    return __sync_add_and_fetch(&m_val, val);
}
long AtomicCounterNative::test_and_set(long comparand, long exchange)
{
    return __sync_val_compare_and_swap(&m_val, comparand, exchange);    // CAS(*ptr, old, new)
}

long AtomicCounterNative::exchange(long val)
{
    return __sync_lock_test_and_set(&m_val, val);
}

