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

#ifndef ICLDevBackendExecutionService_H
#define ICLDevBackendExecutionService_H

#include "ICLDevBackendKernel.h"
#include "cl_device_api.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 * This interface class is responsible for moving the stdout buffer to the
 * device manager
 */
class ICLDevBackendDeviceAgentCallback {
public:
  virtual ~ICLDevBackendDeviceAgentCallback() {}

  /**
   * @effects sends a final buffer to be printed
   */
  virtual int Print(const char *buffer, void *pHandle) = 0;
};

/**
 * This interface class is responsible for the execution service for a given
 * kernel it also contains machine detection unit to better know the target
 * specifications
 */
class ICLDevBackendExecutionService {
public:
  virtual ~ICLDevBackendExecutionService() {}

  /**
   * @returns the target machine description size in bytes
   */
  virtual size_t GetTargetMachineDescriptionSize() const = 0;

  /**
   * Gets the target machine description in the already allocated buffer
   *
   * @param pTargetDescription pointer to the allocated buffer to be filled with
   * the target machine description
   * @param descriptionSize the size of the allocated buffer
   *
   * @returns CL_DEV_SUCCESS and the pTargetDescription will be filled with the
   *  description in case of success; otherwise:
   *  CL_DEV_INVALID_VALUE in case pTargetDescription == NULL
   *  CL_DEV_ERROR_FAIL in any other error
   */
  virtual cl_dev_err_code
  GetTargetMachineDescription(void *pTargetDescription,
                              size_t descriptionSize) const = 0;

  /**
   * Releases the Execution Service
   */
  virtual void Release() = 0;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // ICLDevBackendExecutionService_H
