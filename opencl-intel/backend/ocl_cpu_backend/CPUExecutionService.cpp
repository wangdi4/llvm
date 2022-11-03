// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#include "CPUExecutionService.h"
#include "CPUDeviceBackendFactory.h"
#include "Kernel.h"
#include "KernelProperties.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

CPUExecutionService::CPUExecutionService(const ICLDevBackendOptions *pOptions)
    : ExecutionService(pOptions) {
  m_pBackendFactory = CPUDeviceBackendFactory::GetInstance();
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
