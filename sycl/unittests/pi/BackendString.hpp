// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <detail/plugin.hpp>

namespace pi {
<<<<<<< HEAD
inline const char *GetBackendString(sycl::backend backend) {
  switch (backend) {
#define PI_BACKEND_STR(backend_name)                                           \
  case sycl::backend::backend_name:                                            \
    return #backend_name
    PI_BACKEND_STR(hip);
    PI_BACKEND_STR(cuda);
    PI_BACKEND_STR(host);
    PI_BACKEND_STR(opencl);
    PI_BACKEND_STR(level_zero);
#if INTEL_CUSTOMIZATION
    PI_BACKEND_STR(ext_intel_esimd_emulator);
#endif // INTEL_CUSTOMIZATION
#undef PI_BACKEND_STR
  default:
    return "Unknown Plugin";
=======
inline const char *GetBackendString(const sycl::detail::plugin &Plugin) {
  std::stringstream Str;
  for (sycl::backend Backend :
       {sycl::backend::opencl, sycl::backend::ext_oneapi_level_zero,
        sycl::backend::ext_oneapi_cuda, sycl::backend::ext_intel_esimd_emulator,
        sycl::backend::ext_oneapi_hip}) {
    if (Plugin.hasBackend(Backend)) {
      Str << Backend;
    }
>>>>>>> 8d75f1caef354914961bea0d7cac3015918dc84e
  }
  return Str.str().c_str();
}
} // namespace pi
