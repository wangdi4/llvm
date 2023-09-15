// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#include "cl_thread.h"
#include "cl_utils.h"

#include <Windows.h>
#include <process.h>
#include <stdio.h>

using namespace Intel::OpenCL::Utils;

const unsigned int MAX_UINT = 0xffffffff;

/************************************************************************
 * Creates thread object. Doesn't create
 ************************************************************************/
OclThread::OclThread(std::string name, bool bAutoDelete)
    : m_threadHandle(nullptr), m_threadId(MAX_UINT), m_running(false),
      m_bAutoDelete(bAutoDelete), m_Name(name) {}

/************************************************************************
 * Destroys the thread object.
 * If the thread is running, the function wait for it completion iff the
 * thread is not joined already (If m_join == 0).
 ************************************************************************/
OclThread::~OclThread() {
  if (m_running) {
    Join();
  }

  Clean();
}

/************************************************************************
 * Cleans the private members
 *
 ************************************************************************/
void OclThread::Clean() {
  // m_running = false;  -- thread still can run when cleaning
  if (nullptr != m_threadHandle) {
    // Close handle
    CloseHandle(m_threadHandle);
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
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;     // must be 0x1000
  LPCSTR szName;    // pointer to name (in user addr space)
  DWORD dwThreadID; // thread ID (-1=caller thread)
  DWORD dwFlags;    // reserved for future use, must be zero
} THREADNAME_INFO;
RETURN_TYPE_ENTRY_POINT OclThread::ThreadEntryPoint(void *threadObject) {
  cl_start;
  OclThread *thisWorker = (OclThread *)threadObject;
#ifdef _DEBUG
  if (thisWorker->m_Name.size() > 0) {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = thisWorker->m_Name.c_str();
    info.dwThreadID = -1;
    info.dwFlags = 0;

    __try {
      RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR),
                     (ULONG_PTR *)&info);
    } __except (EXCEPTION_CONTINUE_EXECUTION) {
    }
  }
#endif
  RETURN_TYPE_ENTRY_POINT rc = thisWorker->Run();
  // The worker finished his job
  thisWorker->m_running = false;

  if (thisWorker->m_bAutoDelete) {
    delete thisWorker;
  }

  return (RETURN_TYPE_ENTRY_POINT)rc;
}

/************************************************************************
 * Starts new thread and immediately runs it.
 * You can start the thread only if the thread isn't running
 ************************************************************************/
int OclThread::Start() {
  cl_start;
  if (m_running) {
    // Cannot start the thread
    return THREAD_RESULT_FAIL;
  }
  m_running = true;

  // Check if the previous start call ended naturally or by Joined/Terminated
  if (nullptr != m_threadHandle) {
    Clean();
  }

  // Create the thread and run it immediately
  m_threadHandle = (void *)_beginthreadex(nullptr, 0, ThreadEntryPoint, this, 0,
                                          &m_threadId);
  if (!m_threadHandle) {
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
int OclThread::Join() {
  if (m_running) {
    // If I called to join myself or more than one thread try to join than
    // return error.
    long OldValue = 0;
    if ((isSelf()) || !m_join.compare_exchange_strong(OldValue, 1)) {
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
int OclThread::WaitForCompletion() {
  // If threadHandle already released or I try to wait for myself, return error.
  if ((nullptr == m_threadHandle) || (isSelf())) {
    return THREAD_RESULT_FAIL;
  }
  WaitForSingleObject(m_threadHandle, INFINITE);
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
void OclThread::Terminate(RETURN_TYPE_ENTRY_POINT exitCode) {
  if (nullptr != m_threadHandle) {
    TerminateThread(m_threadHandle, exitCode);
  }
  Clean();
}

void OclThread::SelfTerminate(RETURN_TYPE_ENTRY_POINT exitCode) {
  TerminateThread(GetCurrentThread(), exitCode);
}

bool OclThread::isSelf() { return (GetCurrentThreadId() == GetThreadId()); }

/************************************************************************
 *
 ************************************************************************/
void OclThread::Exit(RETURN_TYPE_ENTRY_POINT exitCode) {
  m_running = false;
  if (m_bAutoDelete) {
    delete this;
  }
  _endthreadex(exitCode);
}

/************************************************************************
 * Sets the thread affinity
 ************************************************************************/
int OclThread::SetAffinity(unsigned char ucAffinity) {
#if defined(WINDOWS_ONECORE)
  return false;
#else
  if (!m_running) {
    return THREAD_RESULT_FAIL;
  }
  // DWORD_PTR affinityMask = ((0 == affinity) ? (affinity) : (0x1 << affinity))
  // ;
  DWORD_PTR affinityMask = (0x1 << ucAffinity);
  if (0 == SetThreadAffinityMask(m_threadHandle, affinityMask)) {
    return THREAD_RESULT_FAIL;
  }
  return THREAD_RESULT_SUCCESS;
#endif
}

/************************************************************************
 *
 ************************************************************************/
THREAD_HANDLE OclThread::GetThreadHandle() const { return m_threadHandle; }

/************************************************************************
 * Check for OS thread
 ************************************************************************/
bool OclThread::IsOsThreadRunning(THREAD_HANDLE handle) {
  return (WAIT_OBJECT_0 != WaitForSingleObject(handle, 0));
}

/************************************************************************
 * Wait for OS thread
 ************************************************************************/
void OclThread::WaitForOsThreadCompletion(THREAD_HANDLE handle) {
  WaitForSingleObject(handle, INFINITE);
  return;
}
