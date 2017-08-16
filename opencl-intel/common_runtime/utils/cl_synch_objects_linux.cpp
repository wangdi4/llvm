/////////////////////////////////////////////////////////////////////////
// cl_synch_objects_linux.cpp
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
using namespace OCLCRT::Utils;

/************************************************************************
 * Creates the mutex section object.
 ************************************************************************/
OclMutex::OclMutex(unsigned int uiSpinCount) : m_uiSpinCount(uiSpinCount)
{    
    m_mutexAttr = new pthread_mutexattr_t;
    if (0 != pthread_mutexattr_init((pthread_mutexattr_t*)m_mutexAttr))
    {
        assert(0 && "Failed init pthread mutex attribute");
    }
    if (0 != pthread_mutexattr_settype((pthread_mutexattr_t*)m_mutexAttr, PTHREAD_MUTEX_RECURSIVE))
    {
        assert(0 && "Failed settype pthread mutex attribute");
    }
    m_mutexHndl = new pthread_mutex_t;
    if (0 != pthread_mutex_init((pthread_mutex_t*)m_mutexHndl, (pthread_mutexattr_t*)m_mutexAttr))
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

    if (0 != pthread_mutexattr_destroy((pthread_mutexattr_t*)m_mutexAttr))
    {
        assert(0 && "Failed destroy pthread mutex attribute");
    }
    delete((pthread_mutexattr_t*)m_mutexAttr);
    m_mutexAttr = NULL;
}

/************************************************************************
 * Take the lock on this critical section.
 * If lock is acquired, all other threads are blocked on this lock until
 * the current thread unlocked it.
 ************************************************************************/
void OclMutex::Lock()
{
    pthread_mutex_lock((pthread_mutex_t*)m_mutexHndl);
}
/************************************************************************
 * Release the lock
 ************************************************************************/
void OclMutex::Unlock()
{
    pthread_mutex_unlock((pthread_mutex_t*)m_mutexHndl);
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
