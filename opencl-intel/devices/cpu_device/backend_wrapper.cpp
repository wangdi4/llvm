// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

namespace Intel {
namespace OpenCL {
namespace CPUDevice {

using namespace DeviceBackend;

OpenCLBackendWrapper::OpenCLBackendWrapper(void)
    : // ALERT!!! DK!!! Backend sometimes corrups heap on Linux if it unloads in
      // parallel with shutdown
      m_targetDev(CPU_DEVICE) {}

cl_dev_err_code
OpenCLBackendWrapper::Init(const ICLDevBackendOptions *pBackendOptions,
                           DeviceMode dev) {
  m_targetDev = dev;
  return InitDeviceBackend(pBackendOptions);
}

void OpenCLBackendWrapper::Terminate() { TerminateDeviceBackend(); }

ICLDevBackendServiceFactory *OpenCLBackendWrapper::GetBackendFactory() {
  return GetDeviceBackendFactory();
}

} // namespace CPUDevice
} // namespace OpenCL
} // namespace Intel
