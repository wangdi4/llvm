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
#include "CompilerConfig.h"
#include <memory>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class ICLDevBackendOptions;

//*****************************************************************************
// Represents the global backend configuration.
// It is a singletinon that must be initialized explicitly.
class BackendConfiguration {
public:
  /**
   * Status initialization. Must be called once, in single threaded
   * environment.
   */
  static void Init();
  /**
   * Termination. Must be called once, in single threaded environment
   */
  static void Terminate();
  /**
   * Singleton instance getter.
   */
  static const BackendConfiguration &GetInstance();
  /**
   * Returns the global compiler configuration.
   */
  GlobalCompilerConfig
  GetGlobalCompilerConfig(const ICLDevBackendOptions *pBackendOptions) const;
  /**
   * Returns the CPU compiler instance configuration.
   */
  std::unique_ptr<ICompilerConfig>
  GetCPUCompilerConfig(const ICLDevBackendOptions *pBackendOptions,
                       bool SkipBuiltins = false) const;

private:
  BackendConfiguration() {}
  ~BackendConfiguration() {}

  static BackendConfiguration *s_pInstance;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
