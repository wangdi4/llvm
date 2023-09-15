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

#ifndef OPENCL_BACKEND_WRAPPER_H
#define OPENCL_BACKEND_WRAPPER_H

#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "cl_dynamic_lib.h"
#include "common_dev_limits.h"

namespace Intel {
namespace OpenCL {
namespace CPUDevice {
/**
 * @brief This class hides the internals of loading and calling OCL CPU Backend
 *
 */
class OpenCLBackendWrapper {
public:
  OpenCLBackendWrapper();
  /**
   * Calls the OCLCpuBackend Init function
   */
  cl_dev_err_code
  Init(const DeviceBackend::ICLDevBackendOptions *pBackendOptions,
       DeviceMode dev);

  /**
   * Calls the OCLCpuBackend Terminate function
   */
  void Terminate();

  /**
   * Calls the OCLCpuBackend 'GetBackendFactor' method (returning the factory
   * inteface)
   */
  DeviceBackend::ICLDevBackendServiceFactory *GetBackendFactory();

private:
  DeviceMode m_targetDev;
};
} // namespace CPUDevice
} // namespace OpenCL
} // namespace Intel

#endif // OPENCL_BACKEND_WRAPPER_H
