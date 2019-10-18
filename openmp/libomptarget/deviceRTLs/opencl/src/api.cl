#if INTEL_COLLAB
//===--- api.c - Runtime library routines ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains API routines for application writers.
//
//===----------------------------------------------------------------------===//

#include "omptarget-opencl.h"
#include "internal.h"

///
/// OpenMP* RTL routines
///
/// TODO: current NDRange mapping is subject to change.
///
/// For now, we assume an OMP team can have at most one work group.
///

EXTERN int omp_get_team_num(void) {
  return __kmp_get_group_id();
}

EXTERN int omp_get_num_teams(void) {
  return __kmp_get_num_groups();
}

EXTERN int omp_get_team_size(int level) {
  bool is_spmd_mode = __kmp_is_spmd_mode();
  if (is_spmd_mode)
    return level == 1 ? __kmp_get_local_size() : 1;

  int tid = __kmp_get_logical_thread_id(is_spmd_mode);
  kmp_local_state_t *local_state = __kmp_get_local_state();
  uint par_level = local_state->parallel_level[tid];

  // We are either in the level-1 active parallel region or
  // in its nested serialized region
  if (level == 1 && par_level > KMP_ACTIVE_PARALLEL_BUMP)
    return local_state->team_threads;

  // Level-0 is serial region
  if (level == 0)
    return 1;

  // We are in nested inactive parallel regions with level-1 being inactive
  if (level > 0 && par_level < KMP_ACTIVE_PARALLEL_BUMP && level <= par_level)
    return 1;

  // We are in nested inactive parallel regions with level-1 being active
  if (level > 1 && par_level > KMP_ACTIVE_PARALLEL_BUMP &&
      level <= (par_level - KMP_ACTIVE_PARALLEL_BUMP))
    return 1;

  // Gray area
  return KMP_UNSPECIFIED;
}

EXTERN int omp_get_thread_num(void) {
  if (GLOBAL.assume_simple_spmd_mode)
    return __kmp_get_local_id();

  return __kmp_get_omp_thread_id(__kmp_is_spmd_mode());
}

EXTERN int omp_get_num_threads(void) {
  if (GLOBAL.assume_simple_spmd_mode)
    return __kmp_get_local_size();

  return __kmp_get_num_omp_threads(__kmp_is_spmd_mode());
}

EXTERN int omp_get_max_threads(void) {
  if (GLOBAL.assume_simple_spmd_mode)
    return 1;

  kmp_local_state_t *local_state = __kmp_get_local_state();
  int level = local_state->parallel_level[__kmp_get_local_id()];
  int ret = 1;
  if (level == 0)
    ret = local_state->num_threads;
  return ret;
}

EXTERN int omp_in_parallel(void) {
  if (GLOBAL.assume_simple_spmd_mode)
    return 1;

  int level = __kmp_get_local_state()->parallel_level[__kmp_get_local_id()];
  return (level > KMP_ACTIVE_PARALLEL_BUMP);
}

EXTERN int omp_get_device_num(void) {
  KMP_UNSUPPORTED("omp_get_device_num()");
  // No built-ins to get this information
  return KMP_UNSPECIFIED;
}

EXTERN int omp_get_num_devices(void) {
  KMP_UNSUPPORTED("omp_get_num_devices()");
  return KMP_UNSPECIFIED;
}

EXTERN int omp_get_num_procs(void) {
  if (GLOBAL.assume_simple_spmd_mode || __kmp_is_spmd_mode())
    return __kmp_get_local_size();
  return __kmp_get_num_workers();
}

EXTERN int omp_is_initial_device(void) {
  return KMP_FALSE;
}

EXTERN int omp_get_initial_device(void) {
  KMP_UNSUPPORTED("omp_get_initial_device()");
  return KMP_UNSPECIFIED;
}

// Initialize global barrier
EXTERN void kmp_global_barrier_init(void) {
// TODO: decide default implementation based on performance
#ifdef KMP_GLOBAL_BARRIER_DISSEM
  __kmp_barrier_dissem_init(&GLOBAL.g_barrier);
#else
  // nothing to be done
#endif
}

// Global barrier
EXTERN void kmp_global_barrier(void) {
// TODO: decide default implementation based on performance
#ifdef KMP_GLOBAL_BARRIER_DISSEM
  __kmp_barrier_dissem(&GLOBAL.g_barrier);
#else
  __kmp_barrier_counting(&GLOBAL.g_barrier);
#endif
}

#endif // INTEL_COLLAB
