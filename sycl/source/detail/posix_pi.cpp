// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//==---------------- posix_pi.cpp ------------------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <sycl/detail/defines.hpp>
#include <sycl/detail/pi.hpp>

#include <dlfcn.h>
#include <string>
#include <iostream>

namespace sycl {
__SYCL_INLINE_VER_NAMESPACE(_V1) {
namespace detail {
namespace pi {

void *loadOsLibrary(const std::string &PluginPath) {
  // TODO: Check if the option RTLD_NOW is correct. Explore using
  // RTLD_DEEPBIND option when there are multiple plugins.
  void *so = dlopen(PluginPath.c_str(), RTLD_NOW);
  if (!so && trace(TraceLevel::PI_TRACE_ALL)) {
    char *Error = dlerror();
    std::cerr << "SYCL_PI_TRACE[-1]: dlopen(" << PluginPath << ") failed with <"
              << (Error ? Error : "unknown error") << ">" << std::endl;
  }
  return so;
}

int unloadOsLibrary(void *Library) { return dlclose(Library); }

void *getOsLibraryFuncAddress(void *Library, const std::string &FunctionName) {
  return dlsym(Library, FunctionName.c_str());
}

} // namespace pi
} // namespace detail
} // __SYCL_INLINE_VER_NAMESPACE(_V1)
} // namespace sycl
