// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
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

// recordering the header files will lead to compilation error on Windows.
// clang-format off
#include "framework_proxy.h"
#include "cl_disable_sys_dialog.h"
#include "cl_dynamic_lib.h"
// clang-format on
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "cl_user_logger.h"
#include <string>

#include "llvm/Support/ManagedStatic.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#pragma comment(lib, "cl_sys_utils.lib")

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
#if !defined(INTEL_PRODUCT_RELEASE) && !defined(_DEBUG)
    Intel::OpenCL::Utils::DisableSystemDialogsOnCrash();
#endif
#ifdef _DEBUG
    // this is needed to initialize allocated objects DB, which is
    // maintained in only in debug
    InitSharedPtrs();
#endif
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    Intel::OpenCL::Framework::MemoryObjectFactory::Destroy();
    // release the framework proxy object
    Intel::OpenCL::Framework::FrameworkProxy::Destroy();
    llvm::llvm_shutdown();
#ifdef _DEBUG
    FiniSharedPts();
#endif
    break;
  }
  return TRUE;
}
