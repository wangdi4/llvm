// INTEL CONFIDENTIAL
//
// Copyright 2006-2019 Intel Corporation.
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

///////////////////////////////////////////////////////////
//  dllmain.cpp
///////////////////////////////////////////////////////////

#pragma comment(lib, "cl_sys_utils.lib")
#ifdef _M_X64
#define TASK_EXECUTOR_LIB_NAME "task_executor64.dll"
#else
#define TASK_EXECUTOR_LIB_NAME "task_executor32.dll"
#endif

#include "backend_wrapper.h"
#include "cl_disable_sys_dialog.h"
#include "cl_dynamic_lib.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "cpu_device.h"
#include <stdlib.h>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace Intel::OpenCL::CPUDevice;

namespace {
Intel::OpenCL::Utils::OclDynamicLib *m_dlTaskExecutor;
}

BOOL LoadTaskExecutor() {
#if 0
    std::string tePath = std::string(MAX_PATH, '\0');

    Intel::OpenCL::Utils::GetModuleDirectory(&tePath[0], MAX_PATH);
    tePath.resize(tePath.find_first_of('\0'));
    tePath += TASK_EXECUTOR_LIB_NAME;

    if (m_dlTaskExecutor->Load(tePath.c_str()) != 0) {
        return FALSE;
    }
#endif
  return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
#if !defined(INTEL_PRODUCT_RELEASE) && !defined(_DEBUG)
    Intel::OpenCL::Utils::DisableSystemDialogsOnCrash();
#endif
    m_dlTaskExecutor = new Intel::OpenCL::Utils::OclDynamicLib();
    return LoadTaskExecutor();
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    delete m_dlTaskExecutor;
    break;
  }
  return TRUE;
}
