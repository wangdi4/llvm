//==-- fallback-cstring.cpp - fallback implementation of C string functions--=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "wrapper.h"

#ifdef __SPIR__
#if INTEL_COLLAB
#if OMP_LIBDEVICE
#pragma omp declare target
#endif  // OMP_LIBDEVICE
#endif  // INTEL_COLLAB
void *__devicelib_memcpy(void *dest, const void *src, size_t n) {
  return __builtin_memcpy(dest, src, n);
}
#if INTEL_COLLAB
#if OMP_LIBDEVICE
#pragma omp end declare target
#endif  // OMP_LIBDEVICE
#endif  // INTEL_COLLAB
#endif // __SPIR__
