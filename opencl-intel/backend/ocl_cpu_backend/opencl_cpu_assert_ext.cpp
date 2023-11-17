//==--- opencl_cpu_assert_ext.cpp - assert support for SYCL  -------------==//
//
// Copyright 2023 Intel Corporation.
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

#include "ICLDevBackendServiceFactory.h"
#include <stdio.h>

using namespace std;

// This definition is from fallback-cassert.cpp under libdevice.
static const char assert_fmt[] =
    "%s:%d: %s: global id: [%lu,%lu,%lu], local id: [%lu,%lu,%lu] "
    "Assertion `%s` failed.\n";

/*
 * This definition is from wrapper.h under libdevice.
 */
extern "C" LLVM_BACKEND_API int
__devicelib_assert_fail(const char *expr, const char *file, int32_t line,
                        const char *func, uint64_t gid0, uint64_t gid1,
                        uint64_t gid2, uint64_t lid0, uint64_t lid1,
                        uint64_t lid2) {
  // print out error message and then abort.
  fprintf(stderr, assert_fmt, file, line, func, gid0, gid1, gid2, lid0, lid1,
          lid2, expr);
  fflush(stderr);
  abort();
  return 0;
}

/*
 * This function is added to align with fallback implementation of assert.
 * For the direct support from backend, it's no-ops. we directly return.
 */
extern "C" LLVM_BACKEND_API void __devicelib_assert_read(void *) { return; }
