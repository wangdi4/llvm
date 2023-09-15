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

#include "OpenCLBackendWrapper.h"
#include "BackendOptions.h"
#include "SATestException.h"
#include "cl_dynamic_lib.h"
#include <assert.h>

namespace Validation {
OpenCLBackendWrapper *OpenCLBackendWrapper::s_instance = NULL;

void OpenCLBackendWrapper::Init(const BERunOptions &runConfig) {
  GlobalBackendOptions config;
  config.InitFromRunConfiguration(runConfig);

  assert(!s_instance);
  s_instance = new OpenCLBackendWrapper();
  s_instance->InitBackend(&config); // we could probably pass the logger routine
                                    // here, but currently it's not supported
}

OpenCLBackendWrapper &OpenCLBackendWrapper::GetInstance() {
  assert(s_instance);
  return *s_instance;
}

OpenCLBackendWrapper::OpenCLBackendWrapper(void) {}

OpenCLBackendWrapper::~OpenCLBackendWrapper(void) {}

cl_dev_err_code
OpenCLBackendWrapper::InitBackend(const ICLDevBackendOptions *pBackendOptions) {
  return InitDeviceBackend(pBackendOptions);
}

void OpenCLBackendWrapper::Terminate() {
  assert(s_instance);
  s_instance->Release();
  delete s_instance;
  s_instance = NULL;
}

void OpenCLBackendWrapper::Release() { TerminateDeviceBackend(); }

ICLDevBackendServiceFactory *OpenCLBackendWrapper::GetBackendServiceFactory() {
  return GetDeviceBackendFactory();
}

} // namespace Validation
