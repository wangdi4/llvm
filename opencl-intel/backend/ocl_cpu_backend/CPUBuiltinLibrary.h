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

#include "BuiltinModules.h"
#include "cl_cpu_detect.h"
#include "cl_dev_backend_api.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class CPUBuiltinLibrary : public BuiltinLibrary {
public:
  CPUBuiltinLibrary(const Intel::OpenCL::Utils::CPUDetect *cpuId,
                    bool useDynamicSvmlLibrary = true)
      : BuiltinLibrary(cpuId), m_useDynamicSvmlLibrary(useDynamicSvmlLibrary) {}

  virtual void Load() override;

private:
  bool m_useDynamicSvmlLibrary;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
