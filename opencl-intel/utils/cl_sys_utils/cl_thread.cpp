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
#include <Windows.h>
#include <stdio.h>
#include <process.h>
#include "cl_utils.h"

using namespace Intel::OpenCL::Utils;

const int THREAD_RESULT_FAIL = -1;
const int THREAD_RESULT_SUCCESS = 0;
const unsigned int MAX_UINT = 0xffffffff;

/************************************************************************
 * Creates thread object. Doesn't create 
/************************************************************************/
OclThread::OclThread(string name, bool bAutoDelete ):
m_threadHandle(NULL),
m_threadId(MAX_UINT),
m_running(false),
m_join(false),
m_Name(name),
m_bAutoDelete(bAutoDelete)
{
}

/************************************************************************
 * Destroys the thread object.
 * If the thread is running, the function wait for it completion
/************************************************************************/
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
/************************************************************************/
void OclThread::Clean()
{
    // m_running = false;	-- thread still can run when cleaning
    if (NULL != m_threadHandle)
    {
        // Close handle
        CloseHandle(m_threadHandle);
        m_threadHandle = NULL;
    }
    m_threadId = MAX_UINT;
    m_join = false;
}

/************************************************************************
 * The thread entry point.
 * When thread starts, it calls to this static function.
 * The function calls the worker run function.
 * when the function ends.
/************************************************************************/
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // must be 0x1000
	LPCSTR szName; // pointer to name (in user addr space)
	DWORD dwThreadID; // thread ID (-1=caller thread)
	DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

unsigned int OclThread::ThreadEntryPoint(void* threadObject)
{
	cl_start;
	OclThread* thisWorker = (OclThread*)threadObject;
#ifdef _DEBUG
	if (thisWorker->m_Name.size() > 0)
	{
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = thisWorker->m_Name.c_str();
		info.dwThreadID = -1;
		info.dwFlags = 0;

		__try
		{
			RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
		}
		__except(EXCEPTION_CONTINUE_EXECUTION)
		{
		}
	}
#endif
    int rc = thisWorker->Run();
    // The worker finished his job
    thisWorker->m_running = false;

	if ( thisWorker->m_bAutoDelete )
	{
		delete thisWorker;
	}

    cl_return rc;
}

/************************************************************************
 * Starts new thread and immediately runs it.
 * You can start the thread only if the thread isn't running
/************************************************************************/
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
    if(NULL != m_threadHandle)
    {
        Clean();
    }

    // Create the thread and run it immediately
    m_threadHandle = (void*)_beginthreadex(NULL, 0, ThreadEntryPoint, this, 0, &m_threadId);
    if(!m_threadHandle)
    {
       Clean();
    }

    cl_return THREAD_RESULT_SUCCESS;
}

/************************************************************************
 * Stops the running thread peacefully.
 * By using this function, the thread stops when it able to.
 * The function notifies the thread to stop and wait for its completion.
 * Note that this function may wait for long time.
 *
/************************************************************************/
int OclThread::Join()
{
    if(m_running)
    {
        m_join = true;
        WaitForCompletion();
    }
    return THREAD_RESULT_SUCCESS;
}   

/************************************************************************
 * Wait until the running thread is finished.
 * If there isn't any running thread, error value is returned
/************************************************************************/
int OclThread::WaitForCompletion()
{
    if(!m_running)
    {
        return THREAD_RESULT_FAIL;
    }
    else
    {
        WaitForSingleObject(m_threadHandle, INFINITE);
        Clean();        
        return THREAD_RESULT_SUCCESS;
    }
}

/************************************************************************
 * Terminate the thread immediately.
 * Be careful when using this function, since the thread exists before 
 * distructors can be called. It might keep the environment unstable.
 * Note that only user space thread is terminate. Kernel space threads such
 * as the Graphics driver thread may continue to work.
/************************************************************************/
void OclThread::Terminate(unsigned int exitCode)
{
	if ( NULL != m_threadHandle )
	{
		TerminateThread(m_threadHandle, exitCode);
	}
	Clean();
}

/************************************************************************
 *
/************************************************************************/
void OclThread::Exit(unsigned int exitCode)
{
	m_running = false;
	if ( m_bAutoDelete )
	{
		delete this;
	}
    _endthreadex(exitCode); 
}

/************************************************************************
 * Sets the thread affinity
/************************************************************************/
int OclThread::SetAffinity(unsigned char ucAffinity)
{
    if(!m_running)
    {
        return THREAD_RESULT_FAIL;
    }
    //DWORD_PTR affinityMask = ((0 == affinity) ? (affinity) : (0x1 << affinity)) ;
	DWORD_PTR affinityMask = (0x1 << ucAffinity) ;
    if (0 == SetThreadAffinityMask(m_threadHandle, affinityMask))
    {
        //Report Error
        printf("WorkerThread SetThreadAffinityMask error: %d\n", GetLastError());
        return THREAD_RESULT_SUCCESS;
    }
	return THREAD_RESULT_SUCCESS;
}

