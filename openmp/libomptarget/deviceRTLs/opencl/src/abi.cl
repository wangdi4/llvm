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
  __kmp_acquire_lock((__global int *)name);
}

EXTERN void __kmpc_end_critical(kmp_critical_name *name) {
  __kmp_release_lock((__global int *)name);
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

#if INTEL_CUSTOMIZATION
// Temporary define these builtins here.  Eventually, IGC must define
// them, and we should remove it from here.
#define KMP_LOCK_FREE 0
#define KMP_LOCK_BUSY 1

EXTERN void __builtin_IB_kmp_acquire_lock(__global int *lock)
{
  volatile atomic_uint *lck = (volatile atomic_uint *)lock;
  uint expected = KMP_LOCK_FREE;
  while (atomic_load_explicit(lck, memory_order_relaxed) != KMP_LOCK_FREE ||
         !atomic_compare_exchange_strong_explicit(lck, &expected, KMP_LOCK_BUSY,
                                                  memory_order_acquire,
                                                  memory_order_relaxed)) {
    expected = KMP_LOCK_FREE;
  }
}

EXTERN void __builtin_IB_kmp_release_lock(__global int *lock)
{
  volatile atomic_uint *lck = (volatile atomic_uint *)lock;
  atomic_store_explicit(lck, KMP_LOCK_FREE, memory_order_release);
}

#undef KMP_LOCK_FREE
#undef KMP_LOCK_BUSY
#endif // INTEL_CUSTOMIZATION

#endif // INTEL_COLLAB
