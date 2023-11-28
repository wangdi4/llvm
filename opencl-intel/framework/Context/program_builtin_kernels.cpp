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

#include "program_builtin_kernels.h"
#include "Context.h"
#include "Device.h"
#include "cl_logger.h"
#include "cl_shared_ptr.hpp"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithBuiltInKernels::ProgramWithBuiltInKernels(
    SharedPtr<Context> pContext, cl_uint uiNumDevices,
    SharedPtr<FissionableDevice> *pDevices, const char *szKernelNames,
    cl_int *piRet)
    : Program(pContext), m_szKernelNames(szKernelNames) {
  cl_int ret = CL_SUCCESS;
  m_szNumAssociatedDevices = uiNumDevices;

  try {
    m_ppDevicePrograms.resize(m_szNumAssociatedDevices);
    bool bDeviceProgramCreated = false;
    size_t i = 0;
    for (i = 0; i < m_szNumAssociatedDevices; ++i) {
      std::unique_ptr<DeviceProgram> &pDevProgram = m_ppDevicePrograms[i];
      pDevProgram.reset(new DeviceProgram());

      cl_dev_program pDevProg;
      cl_dev_err_code err =
          pDevices[i]->GetDeviceAgent()->clDevCreateBuiltInKernelProgram(
              szKernelNames, &pDevProg);
      if (CL_DEV_FAILED(err) && (CL_DEV_INVALID_KERNEL_NAME != err)) {
        ret = CL_OUT_OF_RESOURCES;
        break;
      }

      pDevProgram->SetDevice(pDevices[i]);
      pDevProgram->SetHandle(GetHandle());
      pDevProgram->SetContext(pContext->GetHandle());

      pDevProgram->SetStateInternal(DEVICE_PROGRAM_BUILTIN_KERNELS);
      pDevProgram->SetDeviceHandleInternal(pDevProg);
      bDeviceProgramCreated = true;
    }

    if (!bDeviceProgramCreated) {
      // No device program is created, probably wrong names are provided
      ret = CL_INVALID_VALUE;
    }

    if (CL_FAILED(ret)) {
      for (size_t j = 0; j < i; ++j) {
        cl_dev_program pDevProg =
            m_ppDevicePrograms[j]->GetDeviceProgramHandle();
        if (nullptr != pDevProg) {
          pDevices[j]->GetDeviceAgent()->clDevReleaseProgram(pDevProg);
        }
      }
    } else {
      SetContextDevicesToProgramMappingInternal();
    }

  } catch (std::bad_alloc &e) {
    ret = CL_OUT_OF_HOST_MEMORY;
  }

  if (piRet) {
    *piRet = ret;
  }
}

ProgramWithBuiltInKernels::~ProgramWithBuiltInKernels() {}
