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
  int ret = __kmp_get_num_groups();
  return ret;
}

EXTERN int omp_get_team_size(int level) {
#if KMP_ASSUME_SIMPLE_SPMD_MODE
  return __kmp_get_local_size();
#else
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
#endif
}

EXTERN int omp_get_thread_num(void) {
#if KMP_ASSUME_SIMPLE_SPMD_MODE
  return __kmp_get_local_id();
#else
  return __kmp_get_omp_thread_id(__kmp_is_spmd_mode());
#endif
}

EXTERN int omp_get_num_threads(void) {
#if KMP_ASSUME_SIMPLE_SPMD_MODE
  size_t group_id = __kmp_get_group_id();
  if (__omp_spirv_program_data.device_type == 0 &&
      group_id < KMP_MAX_SPMD_NUM_GROUPS &&
      __omp_spirv_spmd_num_threads[group_id] > 0)
    return __omp_spirv_spmd_num_threads[group_id];
  else
    return __kmp_get_local_size();
#else
  return __kmp_get_num_omp_threads(__kmp_is_spmd_mode());
#endif
}

EXTERN void omp_set_num_threads(int num_threads) {
  // There is nothing we can do here now
}

EXTERN int omp_get_max_threads(void) {
#if KMP_ASSUME_SIMPLE_SPMD_MODE
  // FIXME: this somehow fixes CMPLRLIBS-33306. Investigate why.
  (void)omp_get_num_threads();
  // Return thread_limit always.
  return __kmp_get_local_size();
#else
  kmp_local_state_t *local_state = __kmp_get_local_state();
  int level = local_state->parallel_level[__kmp_get_local_id()];
  int ret = 1;
  if (level == 0)
    ret = local_state->num_threads;
  return ret;
#endif
}

EXTERN int omp_in_parallel(void) {
#if KMP_ASSUME_SIMPLE_SPMD_MODE
  return 1;
#else
  int level = __kmp_get_local_state()->parallel_level[__kmp_get_local_id()];
  return (level > KMP_ACTIVE_PARALLEL_BUMP);
#endif
}

EXTERN int omp_get_thread_limit(void) {
  return __kmp_get_local_size();
}

EXTERN int omp_get_device_num(void) {
  if (__omp_spirv_program_data.initialized) {
    return __omp_spirv_program_data.device_num;
  } else {
    KMP_UNSUPPORTED("omp_get_device_num()");
    return KMP_UNSPECIFIED;
  }
}

EXTERN int omp_get_num_devices(void) {
  if (__omp_spirv_program_data.initialized) {
    return __omp_spirv_program_data.num_devices;
  } else {
    KMP_UNSUPPORTED("omp_get_num_devices()");
    return KMP_UNSPECIFIED;
  }
}

EXTERN int omp_get_num_procs(void) {
  return __omp_spirv_program_data.total_eus *
      __omp_spirv_program_data.hw_threads_per_eu;
}

EXTERN int omp_get_supported_active_levels(void) {
  return 1;
}

EXTERN void omp_set_affinity_format(const char *fmt) {
  KMP_UNSUPPORTED("omp_set_affinity_format()");
}

EXTERN size_t omp_get_affinity_format(char *buf, size_t size) {
  KMP_UNSUPPORTED("omp_get_affinity_format()");
  return 0;
}

EXTERN void omp_display_affinity(const char *fmt) {
  KMP_UNSUPPORTED("omp_display_affinity()");
}

EXTERN size_t omp_capture_affinity(char *buf, size_t size, const char *fmt) {
  KMP_UNSUPPORTED("omp_capture_affinity()");
  return 0;
}

EXTERN int omp_is_initial_device(void) {
  return KMP_FALSE;
}

EXTERN int omp_get_initial_device(void) {
  KMP_UNSUPPORTED("omp_get_initial_device()");
  return KMP_UNSPECIFIED;
}

EXTERN int omp_pause_resource(omp_pause_resource_t kind, int device_num) {
  KMP_UNSUPPORTED("omp_pause_resource()");
  return -1;
}

EXTERN int omp_pause_resource_all(omp_pause_resource_t kind) {
  KMP_UNSUPPORTED("omp_pause_resource_all()");
  return -1;
}

EXTERN double omp_get_wtime(void) {
  KMP_UNSUPPORTED("omp_get_wtime()");
  return 0.0;
}

EXTERN int omp_get_max_teams(void) {
  // This is what we use in the plugin for non-ND-loop kernels
  return __omp_spirv_program_data.total_eus *
      __omp_spirv_program_data.hw_threads_per_eu;
}

EXTERN int omp_get_teams_thread_limit(void) {
  return __omp_spirv_program_data.teams_thread_limit;
}

EXTERN void *malloc(size_t size) {
  return __kmpc_alloc(0, size, NULL);
}

EXTERN void free(void *ptr) {
  __kmpc_free(0, ptr, NULL);
}

#if !KMP_ASSUME_SIMPLE_SPMD_MODE
// Initialize global barrier
EXTERN void kmp_global_barrier_init(void) {
// TODO: decide default implementation based on performance
#ifdef KMP_GLOBAL_BARRIER_DISSEM
  __kmp_barrier_dissem_init(&__omp_spirv_global_data.g_barrier);
#else
  // nothing to be done
#endif
}

// Global barrier
EXTERN void kmp_global_barrier(void) {
// TODO: decide default implementation based on performance
#ifdef KMP_GLOBAL_BARRIER_DISSEM
  __kmp_barrier_dissem(&__omp_spirv_global_data.g_barrier);
#else
  __kmp_barrier_counting(&__omp_spirv_global_data.g_barrier);
#endif
}
#endif // !KMP_ASSUME_SIMPLE_SPMD_MODE

EXTERN void ompx_nbarrier_init(uint nbarrier_count) {
  __kmpc_nbarrier_init(nbarrier_count);
}

EXTERN void ompx_nbarrier_wait(uint nbarrier_id) {
  __kmpc_nbarrier_wait(nbarrier_id);
}

EXTERN void ompx_nbarrier_signal(
    uint nbarrier_id, uint num_producers, uint num_consumers, uint op_type,
    uint fence_type) {
  __kmpc_nbarrier_signal(nbarrier_id, num_producers, num_consumers, op_type,
                         fence_type);
}

#endif // INTEL_COLLAB
