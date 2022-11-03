// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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
#include <BE_DynamicLib.h>

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation {
class BERunOptions;

/// @brief This class hides the internals of loading and calling OCL CPU Backend
//
class OpenCLBackendWrapper {
public:
  static void Init(const BERunOptions &runConfig);
  static OpenCLBackendWrapper &GetInstance();
  static void Terminate();

  void LoadDll();

  cl_dev_err_code InitBackend(const ICLDevBackendOptions *config);

  ICLDevBackendServiceFactory *GetBackendServiceFactory();

  void Release();

private:
  OpenCLBackendWrapper();
  ~OpenCLBackendWrapper();

private:
  Intel::OpenCL::DeviceBackend::Utils::BE_DynamicLib m_dll;
  BACKEND_INIT_FUNCPTR m_funcInit;
  BACKEND_TERMINATE_FUNCPTR m_funcTerminate;
  BACKEND_GETFACTORY_FUNCPTR m_funcGetFactory;

  static OpenCLBackendWrapper *s_instance;
};
} // namespace Validation

#endif // OPENCL_BACKEND_WRAPPER_H
