//==--- glibc_wrapper.cpp - wrappers for Glibc internal functions ----------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "wrapper.h"

<<<<<<< HEAD
#if INTEL_COLLAB
#if OMP_LIBDEVICE
#pragma omp declare target
#endif  // OMP_LIBDEVICE
#endif  // INTEL_COLLAB

=======
#ifdef __SPIR__
>>>>>>> 857ee511bf4053f1f0cdc7f0d2b41fd6273926e0
DEVICE_EXTERN_C
void __assert_fail(const char *expr, const char *file, unsigned int line,
                   const char *func) {
  __devicelib_assert_fail(
      expr, file, line, func, __spirv_GlobalInvocationId_x(),
      __spirv_GlobalInvocationId_y(), __spirv_GlobalInvocationId_z(),
      __spirv_LocalInvocationId_x(), __spirv_LocalInvocationId_y(),
      __spirv_LocalInvocationId_z());
}
<<<<<<< HEAD

#if INTEL_COLLAB
#if OMP_LIBDEVICE
#pragma omp end declare target
#endif  // OMP_LIBDEVICE
#endif  // INTEL_COLLAB
=======
#endif // __SPIR__
>>>>>>> 857ee511bf4053f1f0cdc7f0d2b41fd6273926e0
