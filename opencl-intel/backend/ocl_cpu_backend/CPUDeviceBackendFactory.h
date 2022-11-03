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

#pragma once
#include "IAbstractBackendFactory.h"
#include "cl_dev_backend_api.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 * This class is used to generate the suitable objects for CPU device
 */
class CPUDeviceBackendFactory : public IAbstractBackendFactory {
protected:
  CPUDeviceBackendFactory(){};
  virtual ~CPUDeviceBackendFactory(){};

public:
  static void Init();
  static void Terminate();

  static CPUDeviceBackendFactory *GetInstance();

  virtual Program *CreateProgram() override;
  virtual Kernel *CreateKernel() override;

  virtual Kernel *CreateKernel(const std::string &name,
                               const std::vector<KernelArgument> &args,
                               const std::vector<unsigned int> &memArgs,
                               KernelProperties *pProps) override;

  virtual KernelProperties *CreateKernelProperties() override;
  virtual KernelJITProperties *CreateKernelJITProperties() override;
  virtual IKernelJITContainer *CreateKernelJITContainer() override;

protected:
  static CPUDeviceBackendFactory *s_pInstance;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
