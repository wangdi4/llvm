// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
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
