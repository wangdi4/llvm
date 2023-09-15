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

#include "BE_DynamicLib.h"
#include "ocl_string_exception.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#include <stdio.h>
#endif

using namespace std;
using namespace Intel::OpenCL::Utils;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
namespace Utils {

BE_DynamicLib::BE_DynamicLib(void) {}

BE_DynamicLib::~BE_DynamicLib(void) { Close(); }

void BE_DynamicLib::Load(const char *pLibName) {
  try {
    Intel::OpenCL::Utils::OclDynamicLib::Load(pLibName);
  } catch (ocl_string_exception &errMsg) {
    throw Intel::OpenCL::DeviceBackend::Exceptions::DynamicLibException(
        errMsg.what());
  }
}

ptrdiff_t BE_DynamicLib::GetFuncPtr(const char *funcName) {
  return GetFunctionPtrByName(funcName);
}

} // namespace Utils
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
