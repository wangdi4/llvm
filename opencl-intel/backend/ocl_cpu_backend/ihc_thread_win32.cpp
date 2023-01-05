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

#include "cl_dev_backend_api.h"
#include "ihc_threadsupport.h"
#include <Windows.h>

/* On Windows we use SRWLocks instead of mutexes since condition variables
   doesn't works otherwise */
extern "C" LLVM_BACKEND_API void *_ihc_mutex_create() {
  SRWLOCK *handle = new SRWLOCK;
  if (handle == 0)
    return nullptr;

  InitializeSRWLock(handle);
  return handle;
}

extern "C" LLVM_BACKEND_API int _ihc_mutex_delete(void *handle) {
  SRWLOCK *_m = (SRWLOCK *)handle;
  delete _m;
  return 0;
}

extern "C" LLVM_BACKEND_API int _ihc_mutex_lock(void *handle) {
  SRWLOCK *_m = (SRWLOCK *)handle;
  AcquireSRWLockExclusive(_m);
  return 0;
}

extern "C" LLVM_BACKEND_API int _ihc_mutex_unlock(void *handle) {
  SRWLOCK *_m = (SRWLOCK *)handle;
  ReleaseSRWLockExclusive(_m);
  return 0;
}

extern "C" LLVM_BACKEND_API void *_ihc_cond_create() {
  CONDITION_VARIABLE *cv = new CONDITION_VARIABLE;
  if (cv == 0)
    return nullptr;

  InitializeConditionVariable(cv);
  return cv;
}

extern "C" LLVM_BACKEND_API int _ihc_cond_delete(void *cv) {
  CONDITION_VARIABLE *_cond = (CONDITION_VARIABLE *)cv;
  delete _cond;
  return 0;
}

extern "C" LLVM_BACKEND_API int _ihc_cond_notify_one(void *cv) {
  CONDITION_VARIABLE *_cond = (CONDITION_VARIABLE *)cv;
  WakeConditionVariable(_cond);
  return 0;
}

extern "C" LLVM_BACKEND_API int _ihc_cond_wait(void *m, void *cv) {
  CONDITION_VARIABLE *_cond = (CONDITION_VARIABLE *)cv;
  SRWLOCK *_m = (SRWLOCK *)m;
  BOOL res = SleepConditionVariableSRW(_cond, _m, INFINITE, 0);
  if (res == 0) {
    return GetLastError();
  }
  return 0;
}

extern "C" LLVM_BACKEND_API void *_ihc_pthread_create(void *(*func)(void *),
                                                      void *arg) {
  // Note that CreateThread takes a pointer to the function arguments,
  // while pthreads takes the argument
  HANDLE _thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg,
                                DETACHED_PROCESS, NULL);
  return _thread;
}

extern "C" LLVM_BACKEND_API int _ihc_pthread_join(void *_thread) {
  HANDLE threadp = (HANDLE)_thread;
  DWORD res = WaitForMultipleObjects(1, &threadp, TRUE, INFINITE);
  if (res != WAIT_FAILED) {
    return 0;
  }
  return GetLastError();
}

extern "C" LLVM_BACKEND_API int _ihc_pthread_detach(void *_thread) {
  HANDLE handle = (HANDLE)_thread;
  /* Closing handle is not supposed to kill thread according to Microsoft docs,
  just drop the handle */
  BOOL res = CloseHandle(handle);
  if (res == 0)
    return 0;
  return GetLastError();
}

/* Fast-emulator uses GNU mangling scheme regardless of the actual OS it's on,
 therefore we need following adapters to translate GNU mangled functions to
 functions can be recognized by MSVC */
extern "C" LLVM_BACKEND_API void *_Znwy(unsigned long long size) {
  return ::operator new(size);
}

extern "C" LLVM_BACKEND_API void _ZdlPvy(void *ptr, unsigned long long size) {
  return ::operator delete(ptr, size);
}

extern "C" LLVM_BACKEND_API void _ZSt14_Xlength_errorPKc(char const *str) {
  return std::_Xlength_error(str);
}

extern "C" LLVM_BACKEND_API void _ZdlPv(void *ptr) {
  return ::operator delete(ptr);
}
