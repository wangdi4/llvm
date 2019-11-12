#if INTEL_COLLAB
//===--- abi-parallel.cl - Entry points for parallel region ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains entry points for parallel region.
//
//===----------------------------------------------------------------------===//

#include "omptarget-opencl.h"
#include "internal.h"

///
/// Parallel regions
///

/// Determine and return num threads with the given input
INLINE ushort __kmp_determine_num_threads(ushort num_threads_clause,
                                          ushort num_threads_icv,
                                          ushort thread_limit) {
  ushort requested = num_threads_icv;
  if (num_threads_clause != 0)
    requested = num_threads_clause;

  ushort available = __kmp_get_num_workers();
  if (thread_limit != 0 && thread_limit < available)
    available = thread_limit;

  ushort num_threads = available;
  if (requested != 0 && requested < num_threads)
    num_threads = requested;

  // Adjust num_threads to multiple of sub group size or 1
  ushort sub_group_size = (ushort)get_max_sub_group_size();
  if (num_threads < sub_group_size) {
    num_threads = 1;
  } else {
    num_threads = (num_threads & ~(sub_group_size - 1));
  }

  return num_threads;
}

/// Prepare a single parallel region (called by master)
EXTERN void __kmpc_kernel_prepare_parallel(void *work_fn,
                                           short is_rtl_initialized) {
  KMP_ASSERT(is_rtl_initialized,
             "Invalid RTL state while scheduling a parallel region");
  kmp_local_state_t *local_state = __kmp_get_local_state();
  KMP_ASSERT(local_state, "Invalid team descriptor");
  local_state->work_fn = work_fn;

  const int tid = 0;
  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  kmp_task_state_t *curr_task =
      (kmp_task_state_t *)thread_state->top_level_task[tid];
  KMP_ASSERT(curr_task, "Invalid task descriptor");

  if (__kmp_task_is_in_parallel(curr_task)) {
    return; // Not sure if this is possible other than invalid code generation
  }

  ushort next_num_threads = thread_state->next_region.num_threads[tid];
  ushort num_threads = __kmp_determine_num_threads(
      next_num_threads, local_state->num_threads, local_state->thread_limit);

  if (next_num_threads != 0) {
    thread_state->next_region.num_threads[tid] = 0;
  }

  KMP_ASSERT(num_threads > 0, "Invalid team size");

  // Copy task state from current task to team's task
  __kmp_init_team_task(&thread_state->team.work_parallel, curr_task);
  local_state->team_threads = num_threads;
}

/// Prepare a single parallel region (called by workers)
EXTERN bool __kmpc_kernel_parallel(void **work_fn, short is_rtl_initialized) {
  KMP_ASSERT(is_rtl_initialized,
             "Invalid RTL state while initializing a parallel region");
  kmp_local_state_t *local_state = __kmp_get_local_state();

  // Return work function
  *work_fn = local_state->work_fn;

  // It was cleared by the master
  if (!*work_fn) {
    return false;
  }

  int tid = __kmp_get_local_id();
  if (tid >= local_state->team_threads) {
    return false; // Team does not require this thread
  }

  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  kmp_task_state_t *task = &thread_state->level_one_task[tid];
  KMP_ASSERT(task, "Invalid task descriptor");

  // Copy from team task and set my top-level task
  __kmp_init_task_from_team(task, &thread_state->team.work_parallel);
  __kmp_set_top_level_task(thread_state, tid, task);
  __kmp_inc_parallel_level(local_state->team_threads != 1);

  return true;
}

/// Finalize parallel region by workers
EXTERN void __kmpc_kernel_end_parallel() {
  KMP_ASSERT(__kmp_is_rtl_initialized(),
             "Invalid RTL state while finalizing a parallel region");
  int tid = __kmp_get_local_id();
  kmp_local_state_t *local_state = __kmp_get_local_state();
  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  kmp_task_state_t *task =
      (kmp_task_state_t *)thread_state->top_level_task[tid];
  __kmp_set_top_level_task(thread_state, tid, task->prev);
  __kmp_dec_parallel_level(local_state->team_threads != 1);
}

/// For serialized parallel
EXTERN void __kmpc_serialized_parallel(ident_t *loc, uint gtid) {
  __kmp_inc_parallel_level(false); // inactive parallel region

  if (__kmp_check_rtl_uninitialized(loc)) {
    return;
  }

  int tid = __kmp_get_logical_thread_id(__kmp_check_spmd_mode(loc));
  int level = __kmp_get_level(tid);
  KMP_ASSERT(level <= KMP_MAX_PARALLEL_LEVEL,
             "Reached max supported parallel level");
  if (level > KMP_MAX_PARALLEL_LEVEL)
    return;

  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  kmp_task_state_t *task =
      (kmp_task_state_t *)thread_state->top_level_task[tid];
  __kmp_task_save_loop_data(task);

  // We need to use statically allocated task data
  kmp_task_state_t *new_task = &thread_state->serialized_task[level - 1][tid];
  __kmp_task_copy_parent(new_task, task);
  new_task->task_data.thread_id = 0;

  __kmp_set_top_level_task(thread_state, tid, new_task);
}

/// For serialized parallel
EXTERN void __kmpc_end_serialized_parallel(ident_t *loc, uint gtid) {
  __kmp_dec_parallel_level(false);

  if (__kmp_check_rtl_uninitialized(loc)) {
    return;
  }

  int tid = __kmp_get_logical_thread_id(__kmp_check_spmd_mode(loc));
  int level = __kmp_get_level(tid);
  KMP_ASSERT(level <= KMP_MAX_PARALLEL_LEVEL,
             "Reached max supported parallel level");
  if (level > KMP_MAX_PARALLEL_LEVEL)
    return;

  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  kmp_task_state_t *task =
      (kmp_task_state_t *)thread_state->top_level_task[tid];
  __kmp_set_top_level_task(thread_state, tid, task->prev);
  task = (kmp_task_state_t *)thread_state->top_level_task[tid];
  __kmp_task_restore_loop_data(task);
}

/// Return current parallel level
EXTERN short __kmpc_parallel_level(ident_t *loc, uint gtid) {
  int tid = __kmp_get_logical_thread_id(__kmp_check_spmd_mode(loc));
  return __kmp_get_level(tid);
}

/// Push num threads
EXTERN void __kmpc_push_num_threads(ident_t *loc, int tid, int num_threads) {
  KMP_ASSERT(__kmp_check_rtl_initialized(loc),
             "Invalid RTL state while pushing num_threads\n");
  tid = __kmp_get_logical_thread_id(__kmp_check_spmd_mode(loc));
  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  thread_state->next_region.num_threads[tid] = num_threads;
}

/// Push simd limit -- not sure if we need this
EXTERN void __kmpc_push_simd_limit(ident_t *loc, int tid, int simd_limit) {
  KMP_ASSERT(__kmp_check_rtl_initialized(loc),
             "Invalid RTL state while pushing simd_limit\n");
  tid = __kmp_get_logical_thread_id(__kmp_check_spmd_mode(loc));
  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  thread_state->next_region.simd_limit[tid] = simd_limit;
}

/// Init sharing variables
EXTERN void __kmpc_init_sharing_variables() {
  kmp_shared_data_t *shared_data = __kmp_get_shared_data();
  shared_data->num_shareds = 0;
}

/// Begin sharing variables
EXTERN void __kmpc_begin_sharing_variables(void ***shareds,
                                           size_t num_shareds) {
  KMP_ASSERT(num_shareds <= KMP_MAX_SHAREDS,
             "Reached max supported data sharing");
  kmp_shared_data_t *shared_data = __kmp_get_shared_data();
  *shareds = shared_data->shareds;
  shared_data->num_shareds = num_shareds;
}

/// End sharing variables
EXTERN void __kmpc_end_sharing_variables() {
  kmp_shared_data_t *shared_data = __kmp_get_shared_data();
  shared_data->num_shareds = 0;
}

/// Return list of shared variables
EXTERN void __kmpc_get_shared_variables(void ***shareds) {
  kmp_shared_data_t *shared_data = __kmp_get_shared_data();
  *shareds = shared_data->shareds;
}

#endif // INTEL_COLLAB
