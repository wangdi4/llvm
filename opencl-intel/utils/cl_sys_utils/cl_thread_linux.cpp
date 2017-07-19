/////////////////////////////////////////////////////////////////////////
// cl_thread.cpp:
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
#include "cl_thread.h"
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <stdio.h>
#include "cl_utils.h"

using namespace Intel::OpenCL::Utils;

const unsigned int MAX_UINT = 0xffffffff;

/************************************************************************
 * Creates thread object. Doesn't create
 ************************************************************************/
OclThread::OclThread(string name, bool bAutoDelete ):
m_threadHandle(nullptr),
m_threadId(MAX_UINT),
m_running(false),
m_bAutoDelete(bAutoDelete),
m_Name(name)
{
}

/************************************************************************
 * Destroys the thread object.
 * If the thread is running, the function wait for it completion iff the
 * thread is not joined already (If m_join == 0).
 ************************************************************************/
OclThread::~OclThread()
{
    if( m_running )
    {
        Join();
    }

	Clean();
}

/************************************************************************
 * Cleans the private members
 *
 ************************************************************************/
void OclThread::Clean()
{
    // m_running = false;	-- thread still can run when cleaning
    if (nullptr != m_threadHandle)
    {
        // delete handle
        delete((pthread_t *)m_threadHandle);
        m_threadHandle = nullptr;
    }
    m_threadId = MAX_UINT;
    m_join.exchange(0);
	m_numWaiters.exchange(0);
}

/************************************************************************
 * The thread entry point.
 * When thread starts, it calls to this static function.
 * The function calls the worker run function.
 * when the function ends.
 ************************************************************************/
RETURN_TYPE_ENTRY_POINT OclThread::ThreadEntryPoint(void* threadObject)
{
	cl_start;
	OclThread* thisWorker = (OclThread*)threadObject;
	thisWorker->m_threadId = pthread_self();
    RETURN_TYPE_ENTRY_POINT rc = thisWorker->Run();
    // The worker finished his job
    thisWorker->m_running = false;

	if ( thisWorker->m_bAutoDelete )
	{
		delete thisWorker;
	}

    return (RETURN_TYPE_ENTRY_POINT)rc;
}

/************************************************************************
 * Starts new thread and immediately runs it.
 * You can start the thread only if the thread isn't running
 ************************************************************************/
int OclThread::Start()
{
	cl_start;
    if(m_running)
    {
        // Cannot start the thread
        return THREAD_RESULT_FAIL;
    }
    m_running = true;

    // Check if the previous start call ended naturally or by Joined/Terminated
    if(nullptr != m_threadHandle)
    {
        Clean();
    }

    // Create the thread and run it immediately
    m_threadHandle = new pthread_t;
    int err = pthread_create((pthread_t*)m_threadHandle, nullptr, ThreadEntryPoint, (void*) this);
    if (0 != err)
    {
       Clean();
       cl_return THREAD_RESULT_FAIL;
    }

    cl_return THREAD_RESULT_SUCCESS;
}

/************************************************************************
 * Stops the running thread peacefully.
 * By using this function, the thread stops when it able to.
 * The function notifies the thread to stop and wait for its completion.
 * Note that this function may wait for long time.
 *
 ************************************************************************/
int OclThread::Join()
{
    if(m_running)
    {
	    // If I called to join myself or more than one thread try to join than return error.
	    if ((isSelf()) || (0 != m_join.test_and_set(0, 1)))
	    {
	        return THREAD_RESULT_FAIL;
	    }
	    return WaitForCompletion();
    }
    return THREAD_RESULT_SUCCESS;
}

/************************************************************************
 * Wait until the running thread is finished.
 * If there isn't any running thread, error value is returned
 * If trying to join, joined thread, error value is returned
 ************************************************************************/
int OclThread::WaitForCompletion()
{
    // If threadHandle already released or I try to wait for myself, return error.
	if ((nullptr == m_threadHandle) || (isSelf()))
    {
        return THREAD_RESULT_FAIL;
    }
	// If I'm the first thread that try to wait for completion than use join
	// (If multiple threads simultaneously try to join with the same thread, the results are undefined)
	if (0 == m_numWaiters.test_and_set(0, 1))
	{
		pthread_join((*(pthread_t*)m_threadHandle), nullptr);
	}
	else
	{
		// multiple threads are waiting for completion
		while (0 != m_numWaiters.operator long())
		{
			hw_pause();
		}
	}
    Clean();
    return THREAD_RESULT_SUCCESS;
}

/************************************************************************
 * Terminate the thread immediately.
 * Be careful when using this function, since the thread exists before
 * distructors can be called. It might keep the environment unstable.
 * Note that only user space thread is terminate. Kernel space threads such
 * as the Graphics driver thread may continue to work.
 ************************************************************************/
void OclThread::Terminate(RETURN_TYPE_ENTRY_POINT exitCode)
{
	if ( nullptr != m_threadHandle )
	{
		// The exitCode doesn't exist in Linux
		pthread_cancel((*(pthread_t*)m_threadHandle));
	}
	Clean();
}

void OclThread::SelfTerminate(RETURN_TYPE_ENTRY_POINT exitCode)
{
	// The exitCode doesn't exist in Linux
	pthread_cancel(pthread_self());
#ifndef ANDROID
	// As default, A cancellation request is deferred until the thread next calls a function that is a cancellation point.
	pthread_testcancel();
#else
	assert(0 && "pthread_testcancel() does not support in ANDROID, consider the use of this method, pthread_cancel does not cancel the thread immediately, the call to pthread_testcancel ensure that it happend at this call");
#endif
}

bool OclThread::isSelf()
{
	return (pthread_self() == GetThreadId());
}

THREAD_HANDLE  OclThread::GetThreadHandle() const
{
    return (nullptr != m_threadHandle) ? *(pthread_t*)m_threadHandle : (pthread_t)nullptr;
}


/************************************************************************
 *
 ************************************************************************/
void OclThread::Exit(RETURN_TYPE_ENTRY_POINT exitCode)
{
	m_running = false;
	if ( m_bAutoDelete )
	{
		delete this;
	}
    pthread_exit((void*)exitCode);
}

/************************************************************************
 * Sets the thread affinity
 ************************************************************************/
int OclThread::SetAffinity(unsigned char ucAffinity)
{
    if(!m_running)
    {
        return THREAD_RESULT_FAIL;
    }
    affinityMask_t affinityMask;
    // CPU_ZERO initializes all the bits in the mask to zero.
    CPU_ZERO(&affinityMask);
    // CPU_SET sets only the bit corresponding to cpu.
    CPU_SET((unsigned int)ucAffinity, &affinityMask);
    if (0 != sched_setaffinity( 0, sizeof(affinityMask), &affinityMask))
    {
        //Report Error
        printf("WorkerThread SetThreadAffinityMask error: %d\n", errno);
        return THREAD_RESULT_FAIL;
    }
	return THREAD_RESULT_SUCCESS;
}


/************************************************************************
 * Check for OS thread
 ************************************************************************/
bool OclThread::IsOsThreadRunning( THREAD_HANDLE handle )
{
    return (ESRCH != pthread_kill(handle, 0));
}

/************************************************************************
 * Wait for OS thread
 ************************************************************************/
void OclThread::WaitForOsThreadCompletion( THREAD_HANDLE handle )
{
    while (IsOsThreadRunning(handle))
    {
#ifdef __ANDROID__
        hw_pause();
#else
        pthread_yield();
#endif
    }
}


