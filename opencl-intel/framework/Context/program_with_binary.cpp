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

#include "program_with_binary.h"
#include "Context.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithBinary::ProgramWithBinary(SharedPtr<Context> pContext,
                                     cl_uint uiNumDevices,
                                     SharedPtr<FissionableDevice> *pDevices,
                                     const size_t *pszLengths,
                                     const unsigned char **pBinaries,
                                     cl_int *piBinaryStatus, cl_int *piRet)
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

      cl_int *piBinStatus =
          (nullptr == piBinaryStatus) ? nullptr : piBinaryStatus + i;
      ret = pDevProgram->SetBinary(pszLengths[i], pBinaries[i], piBinStatus);
      if (CL_FAILED(ret)) {
        break;
      }

      // if device is custom then set binary to custom
      if (pDevices[i]->GetRootDevice()->GetDeviceType() ==
          CL_DEVICE_TYPE_CUSTOM) {
        pDevProgram->SetStateInternal(DEVICE_PROGRAM_CUSTOM_BINARY);
      } else {
        switch (pDevProgram->GetBinaryTypeInternal()) {
        case CL_PROGRAM_BINARY_TYPE_INTERMEDIATE:
        case CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT:
        case CL_PROGRAM_BINARY_TYPE_LIBRARY:
          pDevProgram->SetStateInternal(DEVICE_PROGRAM_LOADED_IR);
          break;
        case CL_PROGRAM_BINARY_TYPE_EXECUTABLE:
          pDevProgram->SetStateInternal(DEVICE_PROGRAM_LINKED);
          break;
        default:
          ret = CL_INVALID_BINARY;
          break;
        }
      }
    }
  } catch (std::bad_alloc &e) {
    ret = CL_OUT_OF_HOST_MEMORY;
  }

  if (piRet) {
    *piRet = ret;
  }
}

ProgramWithBinary::~ProgramWithBinary() {}
