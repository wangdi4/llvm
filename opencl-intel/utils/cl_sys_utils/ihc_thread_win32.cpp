//===--- ihc_thread_win32.cpp -                              -*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-------------------------------------------------------------------=== //
//
// Wrapper library to keep mutexes and threads out of kernel code
//
// ===-------------------------------------------------------------------=== //

#include "export/ihc_threadsupport.h"
#include <Windows.h>

namespace Intel {
namespace OpenCL {
namespace Utils {

/* On Windows we use SRWLocks instead of mutexes since condition variables
   doesn't works otherwise */
void *_ihc_mutex_create() {
  SRWLOCK *handle = new SRWLOCK;
  InitializeSRWLock(handle);
  return handle;
}

int _ihc_mutex_delete(void *handle) {
  SRWLOCK *_m = (SRWLOCK *)handle;
  delete _m;
  return 0;
}

int _ihc_mutex_lock(void *handle) {
  SRWLOCK *_m = (SRWLOCK *)handle;
  AcquireSRWLockExclusive(_m);
  return 0;
}

int _ihc_mutex_unlock(void *handle) {
  SRWLOCK *_m = (SRWLOCK *)handle;
  ReleaseSRWLockExclusive(_m);
  return 0;
}

void *_ihc_cond_create() {
  CONDITION_VARIABLE *cv = new CONDITION_VARIABLE;
  InitializeConditionVariable(cv);
  return cv;
}

int _ihc_cond_delete(void *cv) {
  CONDITION_VARIABLE *_cond = (CONDITION_VARIABLE *)cv;
  delete _cond;
  return 0;
}

int _ihc_cond_notify_one(void *cv) {
  CONDITION_VARIABLE *_cond = (CONDITION_VARIABLE *)cv;
  WakeConditionVariable(_cond);
  return 0;
}

int _ihc_cond_wait(void *m, void *cv) {
  CONDITION_VARIABLE *_cond = (CONDITION_VARIABLE *)cv;
  SRWLOCK *_m = (SRWLOCK *)m;
  BOOL res = SleepConditionVariableSRW(_cond, _m, INFINITE, 0);
  if (res == 0) {
    return GetLastError();
  }
  return 0;
}

void *_ihc_pthread_create(void *(*func)(void *), void *arg) {
  HANDLE _thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg,
                                DETACHED_PROCESS, NULL);
  return _thread;
}

int _ihc_pthread_join(void *_thread) {
  HANDLE threadp = (HANDLE)_thread;
  DWORD res = WaitForMultipleObjects(1, &threadp, TRUE, INFINITE);
  if (res != WAIT_FAILED) {
    return 0;
  }
  return GetLastError();

} 

int _ihc_pthread_detach(void *_thread) {
  HANDLE handle = (HANDLE)_thread;
  /* Closing handle is not supposed to kill thread according to Microsoft docs,
  just drop the handle */
  BOOL res = CloseHandle(handle);
  if (res == 0) {
    delete handle;
    return 0;
  }
  return GetLastError();
}

} // namespace OpenCL
} // namespace Intel
} // Intel::OpenCL::Utils
