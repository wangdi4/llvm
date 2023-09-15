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

#ifndef DEBUGGINGSERVICEWRAPPER_H
#define DEBUGGINGSERVICEWRAPPER_H

#include "cl_device_api.h"
#include "icldebuggingservice.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class DebuggingServiceWrapper {
public:
  static DebuggingServiceWrapper &GetInstance() { return instance; }

  DebuggingServiceWrapper();

  // Initialize the service. This can return CL_DEV_SUCCESS also if the
  // DLL was not loaded voluntarily (due to a lacking environment var).
  // In this case all subsequent calls to GetDebuggingService will return
  // NULL and Terminate will be a no-op.
  //
  cl_dev_err_code Init();
  void Terminate();
  ICLDebuggingService *GetDebuggingService();

private:
  cl_dev_err_code LoadDll();
  void UnloadDll();

private:
  static DebuggingServiceWrapper instance;

  bool m_dll_loaded;

  DEBUGGING_SERVICE_INIT_FUNC m_init_func;
  DEBUGGING_SERVICE_TERMINATE_FUNC m_terminate_func;
  DEBUGGING_SERVICE_INSTANCE_FUNC m_instance_func;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // DEBUGGINGSERVICEWRAPPER_H
