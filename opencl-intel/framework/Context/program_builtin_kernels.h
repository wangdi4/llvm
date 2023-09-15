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

#pragma once
#include "program.h"
#include <string>

namespace Intel {
namespace OpenCL {
namespace Framework {

class ProgramWithBuiltInKernels : public Program {
public:
  PREPARE_SHARED_PTR(ProgramWithBuiltInKernels)

  static SharedPtr<ProgramWithBuiltInKernels>
  Allocate(SharedPtr<Context> pContext, cl_uint uiNumDevices,
           SharedPtr<FissionableDevice> *pDevices, const char *szKernelNames,
           cl_int *piRet) {
    return SharedPtr<ProgramWithBuiltInKernels>(new ProgramWithBuiltInKernels(
        pContext, uiNumDevices, pDevices, szKernelNames, piRet));
  }

protected:
  ProgramWithBuiltInKernels(SharedPtr<Context> pContext, cl_uint uiNumDevices,
                            SharedPtr<FissionableDevice> *pDevices,
                            const char *szKernelNames, cl_int *piRet);

  virtual ~ProgramWithBuiltInKernels();

  std::string m_szKernelNames;
};
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
