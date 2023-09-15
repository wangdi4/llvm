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

#pragma once

#include "cl_device_api.h"
#include "cl_dynamic_lib.h"
#include <stdexcept>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

namespace Exceptions {
class DynamicLibException : public std::runtime_error {
public:
  virtual ~DynamicLibException() throw() {}
  DynamicLibException(std::string dllname) : runtime_error(dllname) {}
};

} // namespace Exceptions

namespace Utils {

class BE_DynamicLib : public Intel::OpenCL::Utils::OclDynamicLib {
public:
  BE_DynamicLib(void);
  ~BE_DynamicLib(void);

  // Loads a dynamically link library into process address space
  // Input
  //    pLibName  - A pointer to null terminated string that
  // describes library file name
  void Load(const char *pLibName);

  // Returns the pointer to exported function within a loaded module
  ptrdiff_t GetFuncPtr(const char *funcName);
};

} // namespace Utils
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
