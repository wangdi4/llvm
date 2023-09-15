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

#include "BackendConfiguration.h"
#include "cl_dev_backend_api.h"
#include <assert.h>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

BackendConfiguration *BackendConfiguration::s_pInstance = nullptr;

void BackendConfiguration::Init() {
  assert(!s_pInstance);
  s_pInstance = new BackendConfiguration();
}

void BackendConfiguration::Terminate() {
  if (nullptr != s_pInstance) {
    delete s_pInstance;
    s_pInstance = nullptr;
  }
}

const BackendConfiguration &BackendConfiguration::GetInstance() {
  assert(s_pInstance);
  return *s_pInstance;
}

GlobalCompilerConfig BackendConfiguration::GetGlobalCompilerConfig(
    const ICLDevBackendOptions *pBackendOptions) const {
  GlobalCompilerConfig config;
  config.LoadConfig();
  config.ApplyRuntimeOptions(pBackendOptions);
  return config;
}

std::unique_ptr<ICompilerConfig> BackendConfiguration::GetCPUCompilerConfig(
    const ICLDevBackendOptions *pBackendOptions, bool SkipBuiltins) const {
  CompilerConfig *config(new CompilerConfig);
  config->LoadConfig();
  config->ApplyRuntimeOptions(pBackendOptions);
  if (SkipBuiltins)
    config->SkipBuiltins();
  return std::unique_ptr<ICompilerConfig>(config);
}
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
