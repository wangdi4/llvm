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
#include "cl_shared_ptr.hpp"
#include "program.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

class ProgramWithBinary : public Program {
public:
  PREPARE_SHARED_PTR(ProgramWithBinary)

  static SharedPtr<ProgramWithBinary>
  Allocate(SharedPtr<Context> pContext, cl_uint uiNumDevices,
           SharedPtr<FissionableDevice> *pDevices, const size_t *pszLengths,
           const unsigned char **pBinaries, cl_int *piBinaryStatus,
           cl_int *piRet) {
    return SharedPtr<ProgramWithBinary>(
        new ProgramWithBinary(pContext, uiNumDevices, pDevices, pszLengths,
                              pBinaries, piBinaryStatus, piRet));
  }

protected:
  ProgramWithBinary(SharedPtr<Context> pContext, cl_uint uiNumDevices,
                    SharedPtr<FissionableDevice> *pDevices,
                    const size_t *pszLengths, const unsigned char **pBinaries,
                    cl_int *piBinaryStatus, cl_int *piRet);

  virtual ~ProgramWithBinary();
};
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
