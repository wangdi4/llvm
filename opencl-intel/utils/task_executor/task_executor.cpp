// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "task_executor.h"

#include <mutex>

#if !defined(__THREAD_EXECUTOR__) && !defined(__TBB_EXECUTOR__)
#define __THREAD_EXECUTOR__
#endif

#ifdef __TBB_EXECUTOR__
#include "tbb_executor.h"
#define PTR_CAST TBBTaskExecutor
#endif
#ifdef __THREAD_EXECUTOR__
#include "thread_executor.h"
#define PTR_CAST ThreadTaskExecutor
#endif

#ifdef _WIN32
#pragma comment(lib, "cl_sys_utils.lib")

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::Utils;

static std::mutex TaskExecutorMutex;
static ITaskExecutor *pTaskExecutor;

ITaskExecutor *Intel::OpenCL::TaskExecutor::GetTaskExecutor() {
  std::lock_guard<std::mutex> Lock(TaskExecutorMutex);

  static ITaskExecutor *S = [] {
#ifdef __TBB_EXECUTOR__
    pTaskExecutor = new TBBTaskExecutor;
#endif
#ifdef __THREAD_EXECUTOR_
    pTaskExecutor = new ThreadTaskExecutor;
#endif
    return pTaskExecutor;
  }();

  return S;
}
