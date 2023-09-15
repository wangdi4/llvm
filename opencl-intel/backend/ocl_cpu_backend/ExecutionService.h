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

#pragma once

#include "IAbstractBackendFactory.h"
#include "cl_dev_backend_api.h"
#ifdef OCL_DEV_BACKEND_PLUGINS
#include "plugin_manager.h"
#endif

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class Kernel;
class KernelProperties;

class ExecutionService : public ICLDevBackendExecutionService {
public:
  ExecutionService(const ICLDevBackendOptions *pOptions);
  /**
   * @returns the target machine description size in bytes
   */
  virtual size_t GetTargetMachineDescriptionSize() const override;

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
                              size_t descriptionSize) const override;

  /**
   * Releases the Execution Service
   */
  virtual void Release() override;

protected:
  // pointer to the Backend Factory, not owned by this class
  IAbstractBackendFactory *m_pBackendFactory = nullptr;

#ifdef OCL_DEV_BACKEND_PLUGINS
  mutable Intel::OpenCL::PluginManager m_pluginManager;
#endif
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
