#if INTEL_COLLAB
//===--- abi.c - Entry points for the compiler ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains entry points for the compiler-generated code.
//
//===----------------------------------------------------------------------===//

#include "omptarget-opencl.h"

///
/// Barrier
///

void __kmpc_barrier() {
  // Built-in work group barrier
  work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
}


///
/// Support for critical section
///

// Requires correctly initialized "*name" (=0).
EXTERN void __kmpc_critical(kmp_critical_name *name) {
  __kmp_acquire_lock((atomic_uint *)name);
}

EXTERN void __kmpc_end_critical(kmp_critical_name *name) {
  __kmp_release_lock((atomic_uint *)name);
}


///
/// Other __kmpc_* entries
///

EXTERN int __kmpc_master() {
  return (__kmp_get_local_id() == 0) ? KMP_TRUE : KMP_FALSE;
}

EXTERN void __kmpc_end_master() {
  // nothing to be done
}

#endif // INTEL_COLLAB
