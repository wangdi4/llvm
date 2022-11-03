// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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

#include "backend_wrapper.h"
#include "cl_sys_info.h"
#include "cl_user_logger.h"
#include <assert.h>
#include <memory>

#if defined(_WIN32)
#if defined(_M_X64)
const char *szOclCpuBackendDllName = "OclCpuBackEnd64";
#else
const char *szOclCpuBackendDllName = "OclCpuBackEnd32";
#endif
#else
const char *szOclCpuBackendDllName = "OclCpuBackEnd";
#endif

using Intel::OpenCL::Utils::g_pUserLogger;

namespace Intel {
namespace OpenCL {
namespace CPUDevice {

OpenCLBackendWrapper::OpenCLBackendWrapper(void)
    : // ALERT!!! DK!!! Backend sometimes corrups heap on Linux if it unloads in
      // parallel with shutdown
      m_dll(false), m_funcInit(nullptr), m_funcTerminate(nullptr),
      m_funcGetFactory(nullptr), m_targetDev(CPU_DEVICE) {}

cl_dev_err_code OpenCLBackendWrapper::LoadDll() {
  std::string dllName = std::string(szOclCpuBackendDllName) +
                        (m_targetDev == FPGA_EMU_DEVICE ? OUTPUT_EMU_SUFF : "");

  if (m_dll.Load(Intel::OpenCL::Utils::GetFullModuleNameForLoad(
          OS_DLL_POST(dllName).c_str())) != 0) {
    if (g_pUserLogger && g_pUserLogger->IsErrorLoggingEnabled())
      g_pUserLogger->PrintError("Failed to load " +
                                std::string(OS_DLL_POST(dllName)) +
                                " with error message: " + m_dll.GetError());
    return CL_DEV_ERROR_FAIL;
  }

  m_funcInit = (BACKEND_INIT_FUNCPTR)(intptr_t)m_dll.GetFunctionPtrByName(
      "InitDeviceBackend");
  if (nullptr == m_funcInit) {
    return CL_DEV_ERROR_FAIL;
  }

  m_funcTerminate =
      (BACKEND_TERMINATE_FUNCPTR)(intptr_t)m_dll.GetFunctionPtrByName(
          "TerminateDeviceBackend");
  if (nullptr == m_funcTerminate) {
    return CL_DEV_ERROR_FAIL;
  }

  m_funcGetFactory =
      (BACKEND_GETFACTORY_FUNCPTR)(intptr_t)m_dll.GetFunctionPtrByName(
          "GetDeviceBackendFactory");
  if (nullptr == m_funcGetFactory) {
    return CL_DEV_ERROR_FAIL;
  }

  return CL_DEV_SUCCESS;
}

void OpenCLBackendWrapper::UnloadDll() {
  m_funcInit = nullptr;
  m_funcTerminate = nullptr;
  m_funcGetFactory = nullptr;
  // ALERT!!! DK!!! Backend sometimes corrups heap on Linux if it unloads in
  // parallel with shutdown
  // m_dll.Close();
}

cl_dev_err_code
OpenCLBackendWrapper::Init(const ICLDevBackendOptions *pBackendOptions,
                           DeviceMode dev) {
  assert(
      !m_funcInit &&
      "You must not call Init more then once, without calling Terminate first");
  m_targetDev = dev;
  cl_dev_err_code ret = LoadDll();
  if (CL_DEV_FAILED(ret)) {
    return ret;
  }

  return m_funcInit(pBackendOptions);
}

void OpenCLBackendWrapper::Terminate() {
  assert(m_funcTerminate &&
         "The Init method failed and you didn't noticed, did you?");
  m_funcTerminate();
  UnloadDll();
}

ICLDevBackendServiceFactory *OpenCLBackendWrapper::GetBackendFactory() {
  assert(m_funcGetFactory &&
         "The Init method failed and you didn't noticed, did you?");
  return m_funcGetFactory();
}

} // namespace CPUDevice
} // namespace OpenCL
} // namespace Intel
