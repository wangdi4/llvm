#if INTEL_COLLAB
//===--- omptarget-opencl.cl - OpenMP device runtime for OpenCL -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains OpenMP device runtime code for OpenCL devices
//
//===----------------------------------------------------------------------===//

#include "omptarget-opencl.h"
#include "internal.h"

///
/// Runtime data stored in global address space
///

kmp_global_state_t GLOBAL = {
  .g_barrier = {{ATOMIC_VAR_INIT(0u), ATOMIC_VAR_INIT(0u)}},
  .assume_simple_spmd_mode = 1
};

kmp_local_state_t LOCALS[KMP_MAX_NUM_GROUPS];

kmp_thread_state_t THREADS[KMP_MAX_NUM_GROUPS];

#endif // INTEL_COLLAB
