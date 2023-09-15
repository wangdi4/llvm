// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

// dllmain.cpp : Defines the entry point for the DLL application.

#define DEVICE_BACKEND_EXPORTS

#include "BackendConfiguration.h"
#include "BuiltinModuleManager.h"
#include "CPUDeviceBackendFactory.h"
#include "Compiler.h"
#include "ImageCallbackManager.h"
#include "LibraryProgramManager.h"
#include "ServiceFactory.h"
#include "cl_cpu_detect.h"
#include "cl_dev_backend_api.h"
#include "cl_disable_sys_dialog.h"
#include "debuggingservicewrapper.h"
#include "llvm/Support/Mutex.h"

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace Intel::OpenCL::DeviceBackend;

// lock used to prevent the simultaneous initialization
static std::mutex s_init_lock;
// initialization count - used to prevent the multiple initialization
static int s_init_count = 0;
static bool s_compiler_initialized = false;
// initialization result
static cl_dev_err_code s_init_result = CL_DEV_SUCCESS;

// flag used to disable the termination sequence
bool s_ignore_termination = false;

#ifdef __cplusplus
extern "C" {
#endif
///@brief
///
cl_dev_err_code InitDeviceBackend(const ICLDevBackendOptions *pBackendOptions) {
  std::lock_guard<std::mutex> lock(s_init_lock);

  // The compiler can only be initialized once, even if the backend is
  //   terminated.  The s_init_count check is not sufficient.
  if (!s_compiler_initialized) {
    Compiler::Init();
    s_compiler_initialized = true;
  }

  ++s_init_count;
  if (s_init_count > 1) {
    //
    // Initialization was already completed - just return the result
    //
    return s_init_result;
  }

  try {
    BackendConfiguration::Init();
    Compiler::InitGlobalState(
        BackendConfiguration::GetInstance().GetGlobalCompilerConfig(
            pBackendOptions));
    ServiceFactory::Init();
    CPUDeviceBackendFactory::Init();
    DeviceMode targetDev = CPU_DEVICE;
    if (pBackendOptions) {
      targetDev = static_cast<DeviceMode>(pBackendOptions->GetIntValue(
          (int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE));
    }
    BuiltinModuleManager::Init(FPGA_EMU_DEVICE == targetDev);
    LibraryProgramManager::Init();
    ImageCallbackManager::Init();
    // Attempt to initialize the debug service. If debugging is
    // disabled this is a no-op returning success.
    //
    if (CL_DEV_FAILED(DebuggingServiceWrapper::GetInstance().Init()))
      s_init_result = CL_DEV_ERROR_FAIL;
    else
      s_init_result = CL_DEV_SUCCESS;
  } catch (std::runtime_error &) {
    s_init_result = CL_DEV_ERROR_FAIL;
  }
  return s_init_result;
}

void TerminateDeviceBackend() {
  if (s_ignore_termination) {
    return;
  }

  std::lock_guard<std::mutex> lock(s_init_lock);
  //
  // Only perform the termination when initialization count drops to zero
  //
  --s_init_count;
  assert(s_init_count >= 0);
  if (s_init_count > 0) {
    return;
  }

  BuiltinModuleManager::Terminate();
  ImageCallbackManager::Terminate();
  LibraryProgramManager::Terminate();
  CPUDeviceBackendFactory::Terminate();
  DebuggingServiceWrapper::GetInstance().Terminate();
  ServiceFactory::Terminate();
  BackendConfiguration::Terminate();
}

ICLDevBackendServiceFactory *GetDeviceBackendFactory() {
  return ServiceFactory::GetInstance();
}
#ifdef __cplusplus
}
#endif
