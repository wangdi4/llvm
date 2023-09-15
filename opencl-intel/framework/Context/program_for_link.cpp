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

#include "program_for_link.h"
#include "Context.h"
#include "cl_logger.h"
#include "cl_shared_ptr.hpp"
#include "cl_sys_defines.h"
#include "kernel.h"
#include "sampler.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramForLink::ProgramForLink(SharedPtr<Context> pContext,
                               cl_uint uiNumDevices,
                               SharedPtr<FissionableDevice> *pDevices,
                               cl_int *piRet)
    : Program(pContext) {
  cl_int ret = CL_SUCCESS;
  m_szNumAssociatedDevices = uiNumDevices;

  try {
    m_ppDevicePrograms.resize(m_szNumAssociatedDevices);

    for (size_t i = 0; i < m_szNumAssociatedDevices; ++i) {
      std::unique_ptr<DeviceProgram> &pDevProgram = m_ppDevicePrograms[i];
      pDevProgram.reset(new DeviceProgram());

      pDevProgram->SetDevice(pDevices[i]);
      pDevProgram->SetHandle(GetHandle());
      pDevProgram->SetContext(pContext->GetHandle());
    }
  } catch (std::bad_alloc &e) {
    ret = CL_OUT_OF_HOST_MEMORY;
  }

  if (piRet) {
    *piRet = ret;
  }
}

ProgramForLink::~ProgramForLink() {}
