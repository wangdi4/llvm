// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#include "cl_synch_objects.h"

#include <iostream>
using namespace std;

#include <windows.h>
#include <stdio.h>  // Todo: replace printf with log mechanisem
#include <assert.h>

void clSleep( int milliseconds )
{
    SleepEx( milliseconds, TRUE );
}

// This file is the Windows implementation of the cl_synch_objects interface
using namespace OCLCRT::Utils;

// Creates the mutex section object.
OclMutex::OclMutex( unsigned int uiSpinCount )
{
    m_mutexHndl = new CRITICAL_SECTION();
    InitializeCriticalSectionAndSpinCount( (LPCRITICAL_SECTION)m_mutexHndl, uiSpinCount );
}

// Destroys the critical section object.
OclMutex::~OclMutex()
{
    DeleteCriticalSection( (LPCRITICAL_SECTION)m_mutexHndl );
    delete m_mutexHndl;
    m_mutexHndl = NULL;
}

// Take the lock on this critical section.
// If lock is acquired, all other threads are blocked on this lock until
// the current thread unlocked it.
void OclMutex::Lock()
{
    EnterCriticalSection( (LPCRITICAL_SECTION)m_mutexHndl );
}
// Release the lock
void OclMutex::Unlock()
{
    LeaveCriticalSection( (LPCRITICAL_SECTION)m_mutexHndl );
}


OclAutoMutex::OclAutoMutex( IMutex* mutexObj, bool bAutoLock )
{
    m_mutexObj = mutexObj;
    if( bAutoLock )
    {
        m_mutexObj->Lock();
    }
}

OclAutoMutex::~OclAutoMutex()
{
    m_mutexObj->Unlock();
}

// OclBinarySemaphore
OclBinarySemaphore::OclBinarySemaphore()
{
    m_semaphore = CreateEvent( NULL, false, false, NULL );
}
OclBinarySemaphore::~OclBinarySemaphore()
{
    CloseHandle( m_semaphore );
}
void OclBinarySemaphore::Signal()
{
    SetEvent( m_semaphore );
}
void OclBinarySemaphore::Wait()
{
    WaitForSingleObject( m_semaphore, INFINITE );
}
