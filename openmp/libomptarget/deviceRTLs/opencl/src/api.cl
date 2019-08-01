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

/// Counting barrier
INLINE void __kmp_barrier_counting(global kmp_barrier_t *barrier) {
  kmp_barrier_counting_t *bar = &barrier->counting;
  if (__kmp_get_local_id() == 0) {
    uint curr_go = atomic_load_explicit(&bar->go, memory_order_acquire);
    if (atomic_fetch_add_explicit(&bar->count, 1, memory_order_acq_rel) ==
        __kmp_get_num_groups() - 1) {
      atomic_store_explicit(&bar->count, 0, memory_order_release);
      atomic_store_explicit(&bar->go, ~curr_go, memory_order_release);
    } else {
      while (atomic_load_explicit(&bar->go, memory_order_acquire) == curr_go)
        continue;
    }
  }

  work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
}

/// Dissemination barrier initializer: compute total rounds, partner location.
INLINE void __kmp_barrier_dissem_init(global kmp_barrier_dissem_t *bar) {
  if (__kmp_get_local_id() == 0) {
    int num_groups = __kmp_get_num_groups();
    int group_id = __kmp_get_group_id();
    kmp_barrier_dissem_node_t *node = &bar->node[group_id];
    node->sense = 1;
    node->parity = 0;
    bar->num_rounds = (int)ceil(log2(num_groups * 1.0));

    for (int i = 0; i < bar->num_rounds; i++) {
      int j = (group_id + (1 << i)) % num_groups;
      node->mine[0][i] = 0;
      node->mine[1][i] = 0;
      node->partner[0][i] = &bar->node[j].mine[0][i];
      node->partner[1][i] = &bar->node[j].mine[1][i];
    }
    // Make sure every node is initialized at this point
    while (atomic_fetch_add_explicit(&bar->num_initialized, 1,
                                     memory_order_acq_rel) < num_groups - 1);
  }

  work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
}

/// Dissemination barrier: initializer must be called before any barrier calls.
INLINE void __kmp_barrier_dissem(global kmp_barrier_t *barrier) {
  global kmp_barrier_dissem_t *bar = &barrier->dissem;
  kmp_barrier_dissem_node_t *node = &bar->node[__kmp_get_group_id()];
  int local_id = __kmp_get_local_id();
  int local_size = __kmp_get_local_size();

  // Utilize the idle work item to wait for partners' arrival in parallel
  for (int i = local_id; i < bar->num_rounds; i += local_size) {
    *node->partner[node->parity][i] = node->sense;
    while (node->mine[node->parity][i] != node->sense);
  }

  work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

  if (local_id == 0) {
    if (node->parity == 1)
      node->sense = !node->sense;
    node->parity = 1 - node->parity;
  }

  work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
}

/// Team barrier
INLINE void __kmp_team_barrier(int global_id) {
  // We need to implement this if we allow single-team-multi-group mapping
  // TODO: use global_id to access the barrier object for the team.
}


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
  return __kmp_get_local_size();
}

EXTERN int omp_get_thread_num(void) {
  return __kmp_get_local_id();
}

EXTERN int omp_get_num_threads(void) {
  // need more information
  return KMP_UNSPECIFIED;
}

EXTERN int omp_get_max_threads(void) {
  return __kmp_get_local_size();
}

EXTERN int omp_in_parallel(void) {
  // need more information
  return KMP_UNSPECIFIED;
}

EXTERN int omp_get_device_num(void) {
  // No built-ins to get this information
  // FIXME: returning 0 for now
  return 0;
}

EXTERN int omp_get_num_devices(void) {
  // Unspecified
  return KMP_UNSPECIFIED;
}

EXTERN int omp_is_initial_device(void) {
  return KMP_FALSE;
}

EXTERN int omp_get_initial_device(void) {
  // Unspecified
  return KMP_UNSPECIFIED;
}

// Initialize global barrier
EXTERN void kmp_global_barrier_init(void) {
// TODO: decide default implementation based on performance
#ifdef KMP_GLOBAL_BARRIER_DISSEM
  __kmp_barrier_dissem_init(&gstate.g_barrier);
#else
  // nothing to be done
#endif
}

// Global barrier
EXTERN void kmp_global_barrier(void) {
// TODO: decide default implementation based on performance
#ifdef KMP_GLOBAL_BARRIER_DISSEM
  __kmp_barrier_dissem(&gstate.g_barrier);
#else
  __kmp_barrier_counting(&gstate.g_barrier);
#endif
}

#endif // INTEL_COLLAB
