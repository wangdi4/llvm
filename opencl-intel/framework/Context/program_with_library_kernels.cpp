// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

#include "program_with_library_kernels.h"
#include "Context.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithLibraryKernels::ProgramWithLibraryKernels(
    SharedPtr<Context> Ctx, cl_uint NumDevices,
    SharedPtr<FissionableDevice> *Devices, std::string &KernelNames,
    cl_int *Ret)
    : Program(Ctx) {
  cl_int ret = CL_SUCCESS;
  m_szNumAssociatedDevices = NumDevices;

  try {
    m_ppDevicePrograms.resize(m_szNumAssociatedDevices);
    bool DeviceProgramCreated = false;
    size_t i = 0;
    for (; i < m_szNumAssociatedDevices; ++i) {
      std::unique_ptr<DeviceProgram> &DevProgram = m_ppDevicePrograms[i];
      DevProgram.reset(new DeviceProgram());

      cl_dev_program DevProg;
      const char *KNames = nullptr;
      cl_dev_err_code err =
          Devices[i]->GetDeviceAgent()->clDevCreateLibraryKernelProgram(
              &DevProg, &KNames);
      if (CL_DEV_FAILED(err)) {
        LOG_ERROR(TEXT("clDevCreateLibraryKernelProgram failed, err %d"), err);
        ret = CL_OUT_OF_RESOURCES;
        break;
      }
      // KNames is a list of kernel names separated by comma.
      KernelNames = KNames;

      DevProgram->SetDevice(Devices[i]);
      DevProgram->SetHandle(GetHandle());
      DevProgram->SetContext(Ctx->GetHandle());

      DevProgram->SetStateInternal(DEVICE_PROGRAM_LIBRARY_KERNELS);
      DevProgram->SetDeviceHandleInternal(DevProg);
      DeviceProgramCreated = true;
    }

    if (!DeviceProgramCreated)
      ret = CL_INVALID_VALUE;

    if (CL_SUCCEEDED(ret))
      SetContextDevicesToProgramMappingInternal();
    else {
      LOG_ERROR(TEXT("Failed to create library program, err %d"), ret);
      for (size_t j = 0; j < i; ++j) {
        cl_dev_program DevProg =
            m_ppDevicePrograms[j]->GetDeviceProgramHandle();
        if (nullptr != DevProg)
          Devices[j]->GetDeviceAgent()->clDevReleaseProgram(DevProg);
      }
    }
  } catch (std::bad_alloc &e) {
    LOG_ERROR(TEXT("%s"),
              TEXT("ProgramWithLibraryKernels throws bad_alloc exception"));
    ret = CL_OUT_OF_HOST_MEMORY;
  }

  if (Ret)
    *Ret = ret;
}

ProgramWithLibraryKernels::~ProgramWithLibraryKernels() {}
