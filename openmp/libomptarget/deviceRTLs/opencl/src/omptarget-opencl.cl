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

///
/// Global state
///
/// TODO: barrier is bound to team, so we need per-team barrier object if there
/// is a team that maps to multiple work groups.

kmp_global_state_t gstate = {
  .g_barrier = {{ATOMIC_VAR_INIT(0), ATOMIC_VAR_INIT(0)}},
};

#endif // INTEL_COLLAB
