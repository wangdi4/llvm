//===--- ihc_thread_linux.cpp -                              -*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Wrapper library to keep mutexes and threads out of kernel code
//
// ===--------------------------------------------------------------------=== //

#include "cl_dev_backend_api.h"
#include "ihc_threadsupport.h"
#include <pthread.h>

extern "C" LLVM_BACKEND_API void *_ihc_mutex_create() {
  pthread_mutex_t *handle = new pthread_mutex_t;
  if (handle == 0)
    return nullptr;

  *handle = PTHREAD_MUTEX_INITIALIZER;
  return handle;
}

extern "C" LLVM_BACKEND_API int _ihc_mutex_delete(void *handle) {
  pthread_mutex_t *_m = (pthread_mutex_t *)handle;
  int res = pthread_mutex_destroy(_m);
  delete _m;
  return res;
}

extern "C" LLVM_BACKEND_API int _ihc_mutex_lock(void *handle) {
  pthread_mutex_t *_m = (pthread_mutex_t *)handle;
  int res = pthread_mutex_lock(_m);
  return res;
}

extern "C" LLVM_BACKEND_API int _ihc_mutex_unlock(void *handle) {
  pthread_mutex_t *_m = (pthread_mutex_t *)handle;
  int res = pthread_mutex_unlock(_m);
  return res;
}

extern "C" LLVM_BACKEND_API void *_ihc_cond_create() {
  pthread_cond_t *cv = new pthread_cond_t;
  if (cv == 0)
    return nullptr;

  *cv = PTHREAD_COND_INITIALIZER;
  return cv;
}

extern "C" LLVM_BACKEND_API int _ihc_cond_delete(void *cv) {
  pthread_cond_t *_cond = (pthread_cond_t *)cv;
  int res = pthread_cond_destroy(_cond);
  delete _cond;
  return res;
}

extern "C" LLVM_BACKEND_API int _ihc_cond_notify_one(void *cv) {
  pthread_cond_t *_cond = (pthread_cond_t *)cv;
  int res = pthread_cond_signal(_cond);
  return res;
}

extern "C" LLVM_BACKEND_API int _ihc_cond_wait(void *m, void *cv) {
  pthread_cond_t *_cond = (pthread_cond_t *)cv;
  pthread_mutex_t *_m = (pthread_mutex_t *)m;
  int res = pthread_cond_wait(_cond, _m);
  return res;
}

extern "C" LLVM_BACKEND_API void *_ihc_pthread_create(void *(*func)(void *),
                                                      void *arg) {
  pthread_t *handle = new pthread_t;
  if (handle == 0)
    return nullptr;
  int res = pthread_create(handle, NULL, func, arg);
  if (res == 0) {
    return handle;
  } else {
    return 0;
  }
}

extern "C" LLVM_BACKEND_API int _ihc_pthread_join(void *handle) {
  pthread_t *_thread = (pthread_t *)handle;
  int res = pthread_join(*_thread, nullptr);
  return res;
}

extern "C" LLVM_BACKEND_API int _ihc_pthread_detach(void *handle) {
  pthread_t *_thread = (pthread_t *)handle;
  int res = pthread_detach(*_thread);
  return res;
}
