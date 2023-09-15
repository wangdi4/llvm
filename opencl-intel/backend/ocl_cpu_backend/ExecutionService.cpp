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

#include "ExecutionService.h"
#include <assert.h>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

ExecutionService::ExecutionService(const ICLDevBackendOptions *pOptions) {
  // obtain Device Command Manager
  assert(pOptions && "pOptions are NULL");
}

size_t ExecutionService::GetTargetMachineDescriptionSize() const {
  assert(false && "NotImplemented");
  return 0;
}

cl_dev_err_code
ExecutionService::GetTargetMachineDescription(void * /*pTargetDescription*/,
                                              size_t /*descriptionSize*/) const

{
  assert(false && "NotImplemented");
  return CL_DEV_NOT_SUPPORTED;
}

void ExecutionService::Release() { delete this; }

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
