// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "GenericMemObj.h"
#include "cl_disable_sys_dialog.h"
#include "cl_shutdown.h"
#include "cl_sys_info.h"
#include "framework_proxy.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;
using namespace llvm;

#if !_WIN32
__attribute__((constructor)) static void dll_init(void);
// As far as possible let dll_fini be called last
__attribute__((destructor(101))) static void dll_fini(void);
#endif

static void dll_init(void) {
#if defined(_WIN32) && !defined(INTEL_PRODUCT_RELEASE) && !defined(_DEBUG)
  DisableSystemDialogsOnCrash();
#endif
}

static void dll_fini(void) {
  UpdateShutdownMode(ExitStarted);
  MemoryObjectFactory::Destroy();
  // release the framework proxy object
  FrameworkProxy::Destroy();
  DestroyHwloc();
  // FIXME: Call llvm_shutdown() here will cause some llvm global variables to
  // be released too early. Temporarily disable it, we'll fix it later.
  // llvm_shutdown();
  UpdateShutdownMode(ExitDone);
}

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    dll_init();
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    dll_fini();
    break;
  }
  return TRUE;
}
#endif
