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

#include <assert.h>
#include <pthread.h>
#include <signal.h>
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
    // delete handle
    delete ((pthread_t *)m_threadHandle);
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
RETURN_TYPE_ENTRY_POINT OclThread::ThreadEntryPoint(void *threadObject) {
  cl_start;
  OclThread *thisWorker = (OclThread *)threadObject;
  thisWorker->m_threadId = pthread_self();
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
  m_threadHandle = new pthread_t;
  int err = pthread_create((pthread_t *)m_threadHandle, nullptr,
                           ThreadEntryPoint, (void *)this);
  if (0 != err) {
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
  // If I'm the first thread that try to wait for completion than use join
  // (If multiple threads simultaneously try to join with the same thread, the
  // results are undefined)
  long OldValue = 0;
  if (m_numWaiters.compare_exchange_strong(OldValue, 1)) {
    pthread_join((*(pthread_t *)m_threadHandle), nullptr);
  } else {
    // multiple threads are waiting for completion
    while (0 != m_numWaiters.load()) {
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
void OclThread::Terminate(RETURN_TYPE_ENTRY_POINT /*exitCode*/) {
  if (nullptr != m_threadHandle) {
    // The exitCode doesn't exist in Linux
    pthread_cancel((*(pthread_t *)m_threadHandle));
  }
  Clean();
}

void OclThread::SelfTerminate(RETURN_TYPE_ENTRY_POINT /*exitCode*/) {
  // The exitCode doesn't exist in Linux
  pthread_cancel(pthread_self());
  // As default, A cancellation request is deferred until the thread next calls
  // a function that is a cancellation point.
  pthread_testcancel();
}

bool OclThread::isSelf() { return (pthread_self() == GetThreadId()); }

THREAD_HANDLE OclThread::GetThreadHandle() const {
  return (nullptr != m_threadHandle) ? *(pthread_t *)m_threadHandle
                                     : (pthread_t) nullptr;
}

/************************************************************************
 *
 ************************************************************************/
void OclThread::Exit(RETURN_TYPE_ENTRY_POINT exitCode) {
  m_running = false;
  if (m_bAutoDelete) {
    delete this;
  }
  pthread_exit((void *)exitCode);
}

/************************************************************************
 * Sets the thread affinity
 ************************************************************************/
int OclThread::SetAffinity(unsigned char ucAffinity) {
  if (!m_running) {
    return THREAD_RESULT_FAIL;
  }
  affinityMask_t affinityMask;
  // CPU_ZERO initializes all the bits in the mask to zero.
  CPU_ZERO(&affinityMask);
  // CPU_SET sets only the bit corresponding to cpu.
  CPU_SET((unsigned int)ucAffinity, &affinityMask);
  if (0 != sched_setaffinity(0, sizeof(affinityMask), &affinityMask)) {
    // Report Error
    printf("WorkerThread SetThreadAffinityMask error: %d\n", errno);
    return THREAD_RESULT_FAIL;
  }
  return THREAD_RESULT_SUCCESS;
}

/************************************************************************
 * Check for OS thread
 ************************************************************************/
bool OclThread::IsOsThreadRunning(THREAD_HANDLE handle) {
  return (ESRCH != pthread_kill(handle, 0));
}

/************************************************************************
 * Wait for OS thread
 ************************************************************************/
void OclThread::WaitForOsThreadCompletion(THREAD_HANDLE handle) {
  while (IsOsThreadRunning(handle)) {
    sched_yield();
  }
}
