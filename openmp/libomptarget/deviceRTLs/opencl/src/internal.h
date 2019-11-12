#if INTEL_COLLAB
//===--- internal.h - header that contains internal utility functions -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains implementation of internal utility functions
//
//===----------------------------------------------------------------------===//

#ifndef INTERNAL_H
#define INTERNAL_H

#include "omptarget-opencl.h"

///
/// Basic messaging -- we don't have variadics in OpenCL
///

/// Just print out something if check fails
#define KMP_ASSERT(Check, Message)                                             \
  do {                                                                         \
    if (!(Check))                                                              \
      printf("Assertion Failed: " #Check ", " Message "\n");                   \
  } while (0)

/// Unsupported feature
#define KMP_UNSUPPORTED(Feature)                                               \
  do {                                                                         \
    printf("Device does not support " Feature "\n");                           \
  } while (0)

///
/// Utility functions
///

/// Return linear global id
INLINE size_t __kmp_get_global_id() {
  return get_global_linear_id();
}

/// Return linear local id
INLINE size_t __kmp_get_local_id() {
  return get_local_linear_id();
}

/// Return linear group id
INLINE size_t __kmp_get_group_id() {
  return get_group_id(0) + get_num_groups(0) * get_group_id(1) +
      get_num_groups(0) * get_num_groups(1) * get_group_id(2);
}

/// Return global size
INLINE size_t __kmp_get_global_size() {
  return get_global_size(0) * get_global_size(1) * get_global_size(2);
}

/// Return local size
INLINE size_t __kmp_get_local_size() {
  return get_local_size(0) * get_local_size(1) * get_local_size(2);
}

/// Return number of groups
INLINE size_t __kmp_get_num_groups() {
  return get_num_groups(0) * get_num_groups(1) * get_num_groups(2);
}

/// Return the work id for the master thread
INLINE int __kmp_get_master_id() {
  return get_max_sub_group_size() * (get_num_sub_groups() - 1);
}

/// Return the number of workers
INLINE int __kmp_get_num_workers() {
  return __kmp_get_master_id();
}

/// Lock builtins which are not widened (considered
/// uniform) by the device compiler(s).
EXTERN void __builtin_IB_kmp_acquire_lock(__global int *);
EXTERN void __builtin_IB_kmp_release_lock(__global int *);

/// Acquire lock
INLINE void __kmp_acquire_lock(__global int *lock) {
  __builtin_IB_kmp_acquire_lock(lock);
}

/// Release lock
INLINE void __kmp_release_lock(__global int *lock) {
  __builtin_IB_kmp_release_lock(lock);
}

/// Initialize a single team data
INLINE void __kmp_init_local(kmp_local_state_t *local_state) {
  atomic_init(&local_state->work_barrier.count, 0);
  atomic_init(&local_state->work_barrier.go, 0);
}

/// Access local data
INLINE kmp_local_state_t *__kmp_get_local_state() {
  return &LOCALS[__kmp_get_group_id()];
}

/// Access thread data
INLINE kmp_thread_state_t *__kmp_get_thread_state() {
  return &THREADS[__kmp_get_group_id()];
}

/// Initialize all team data
INLINE void __kmp_init_locals() {
  for (int i = 0; i < KMP_MAX_NUM_GROUPS; ++i)
    __kmp_init_local(&LOCALS[i]);
}

/// Initialize work barrier
INLINE void __kmp_init_work_barrier(kmp_local_state_t *local_state) {
  atomic_init(&local_state->work_barrier.count, 0);
  atomic_init(&local_state->work_barrier.go, 0);
}

/// Set execution flags
INLINE void __kmp_set_execution_flags(uint execution_mode, uint runtime_mode) {
  kmp_local_state_t *local_state = __kmp_get_local_state();
  local_state->execution_flags = execution_mode | runtime_mode;
}

/// Constants for kernel execution mode
typedef enum kmp_exec_mode {
  EXEC_MODE_GENERIC = 0x00u,
  EXEC_MODE_SPMD = 0x01u,
  EXEC_MODE_MASK = 0x01u,
} kmp_exec_mode_t;

/// Constants for runtime state
typedef enum kmp_rtl_state {
  RTL_STATE_INITIALIZED = 0x00u,
  RTL_STATE_UNINITIALIZED = 0x02u,
  RTL_STATE_MASK = 0x02u,
} kmp_rtl_state_t;

/// Check if current mode is generic
INLINE bool __kmp_is_generic_mode() {
  uint flags = __kmp_get_local_state()->execution_flags;
  return (flags & EXEC_MODE_MASK) == EXEC_MODE_GENERIC;
}

/// Check if current mode is spmd
INLINE bool __kmp_is_spmd_mode() {
  uint flags = __kmp_get_local_state()->execution_flags;
  return (flags & EXEC_MODE_MASK) == EXEC_MODE_SPMD;
}

/// Check if runtime is in initialized state
INLINE bool __kmp_is_rtl_initialized() {
  uint flags = __kmp_get_local_state()->execution_flags;
  return (flags & RTL_STATE_MASK) == RTL_STATE_INITIALIZED;
}

/// Check if runtime is in uninitialized state
INLINE bool __kmp_is_rtl_uninitialized() {
  uint flags = __kmp_get_local_state()->execution_flags;
  return (flags & RTL_STATE_MASK) == RTL_STATE_UNINITIALIZED;
}

/// Check if current mode is spmd with compiler data
INLINE bool __kmp_check_spmd_mode(ident_t *loc) {
  if (loc) {
    // TODO: define constants for the flags
  }
  return __kmp_is_spmd_mode();
}

/// Check if current mode is generic mode with compiler data
INLINE bool __kmp_check_generic_mode(ident_t *loc) {
  return !__kmp_check_spmd_mode(loc);
}

/// Check if runtime is in initialized state with compiler data
INLINE bool __kmp_check_rtl_initialized(ident_t *loc) {
  if (loc) {
    // TODO: define constants for the flags
  }
  return __kmp_is_rtl_initialized();
}

/// Check if runtime is in uninitialized state with compiler data
INLINE bool __kmp_check_rtl_uninitialized(ident_t *loc) {
  return !__kmp_check_rtl_initialized(loc);
}

/// Return the shared data info
INLINE kmp_shared_data_t *__kmp_get_shared_data() {
  kmp_local_state_t *local_state = __kmp_get_local_state();
  return &local_state->shared_data;
}

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

/// Do counting barrier with the specified count within a work group
INLINE void __kmp_work_barrier(kmp_barrier_counting_t *bar, int count) {
  sub_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

  if (get_sub_group_local_id() == 0) {
    uint curr_go = atomic_load_explicit(&bar->go, memory_order_acquire,
                                        memory_scope_work_group);
    if (atomic_fetch_add_explicit(&bar->count, 1, memory_order_acq_rel,
                                  memory_scope_work_group) == count - 1) {
      atomic_store_explicit(&bar->count, 0, memory_order_release,
                            memory_scope_work_group);
      atomic_store_explicit(&bar->go, ~curr_go, memory_order_release,
                            memory_scope_work_group);
    } else {
      while (atomic_load_explicit(&bar->go, memory_order_acquire,
                                  memory_scope_work_group) == curr_go)
        KMP_PAUSE();
    }
  }

  sub_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
}

/// Barrier for a work group
INLINE void __kmp_team_barrier() {
  work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
}

INLINE int __kmp_get_logical_thread_id(bool is_spmd_mode) {
  int tid = __kmp_get_local_id();
  if (!is_spmd_mode && tid >= __kmp_get_master_id())
    return 0;
  else
    return tid;
}

INLINE int __kmp_get_num_omp_threads(bool is_spmd_mode) {
  int ret;
  kmp_local_state_t *local_state = __kmp_get_local_state();
  int tid = __kmp_get_logical_thread_id(is_spmd_mode);
  int level = local_state->parallel_level[tid];

  if (level != KMP_ACTIVE_PARALLEL_BUMP + 1) {
    ret = 1;
  } else if (is_spmd_mode) {
    ret = __kmp_get_local_size();
  } else {
    ret = local_state->team_threads;
  }
  return ret;
}

INLINE int __kmp_get_omp_thread_id(bool is_spmd_mode) {
  int ret;
  kmp_local_state_t *local_state = __kmp_get_local_state();
  int tid = __kmp_get_logical_thread_id(is_spmd_mode);
  int level = local_state->parallel_level[tid];

  if (level != KMP_ACTIVE_PARALLEL_BUMP + 1) {
    ret = 0;
  } else if (is_spmd_mode) {
    ret = __kmp_get_local_id();
  } else {
    kmp_thread_state_t *thread_state = __kmp_get_thread_state();
    kmp_task_state_t *task =
        (kmp_task_state_t *)thread_state->top_level_task[tid];
    KMP_ASSERT(task,
               "Invalid task descriptor while retrieving thread ID");
    ret = task->task_data.thread_id;
  }
  return ret;
}


///
/// Accessors/modifiers for kmp_thread_state_t
///

INLINE void __kmp_init_thread_state(kmp_thread_state_t *thread_state, int tid) {
  thread_state->top_level_task[tid] = (kmp_task_state_t *)NULL;
  thread_state->next_region.num_threads[tid] = 0;
}

INLINE void __kmp_set_top_level_task(kmp_thread_state_t *thread_state, int tid,
                                     kmp_task_state_t *new_task) {
  thread_state->top_level_task[tid] = new_task;
}


///
/// Accessors/modifiers for kmp_task_state_t
///

// Only master can call this
INLINE void __kmp_init_level_zero_task(kmp_task_state_t *task) {
  task->task_data.flags = 0;
  task->task_data.thread_id = 0;
  task->task_data.runtime_chunk_size = 1; // TODO: check if this fits
}

// Any work/thread can call this in SPMD mode
INLINE void __kmp_init_level_one_task(kmp_task_state_t *task,
                                      kmp_task_state_t *parent) {
  task->task_data.flags = TASK_IN_PARALLEL | TASK_IMPLICIT;
  task->task_data.thread_id = __kmp_get_local_id();
  task->task_data.runtime_chunk_size = 1; // TODO: check if this fits
  task->prev = parent;
}

INLINE int __kmp_task_is_in_parallel(kmp_task_state_t *task) {
  return task->task_data.flags & TASK_IN_PARALLEL;
}

INLINE int __kmp_task_is_in_deep_parallel(kmp_task_state_t *task) {
  return task->task_data.flags & TASK_IN_DEEP_PARALLEL;
}

INLINE int __kmp_task_is_parallel_construct(kmp_task_state_t *task) {
  return task->task_data.flags & TASK_IMPLICIT;
}

INLINE int __kmp_task_is_task_construct(kmp_task_state_t *task) {
  return !__kmp_task_is_parallel_construct(task);
}

INLINE omp_sched_t __kmp_task_get_runtime_sched(kmp_task_state_t *task) {
  return  (omp_sched_t)((task->task_data.flags & TASK_SCHED_MASK) + 1);
}

INLINE void __kmp_task_set_runtime_sched(kmp_task_state_t *task,
                                         omp_sched_t sched) {
  task->task_data.flags &= ~TASK_SCHED_MASK;
  task->task_data.flags |= ((uchar)sched) - 1;
}

INLINE void __kmp_task_copy_data(kmp_task_state_t *task,
                                 kmp_task_state_t *source) {
  task->task_data = source->task_data;
}

INLINE void __kmp_task_copy_parent(kmp_task_state_t *task,
                                   kmp_task_state_t *source) {
  task->task_data = source->task_data;
  task->prev = source;
}

INLINE void __kmp_task_save_loop_data(kmp_task_state_t *task) {
  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  int tid = task->task_data.thread_id;
  task->loop_data.loop_ub = thread_state->loop_ub[tid];
  task->loop_data.next_lb = thread_state->next_lb[tid];
  task->loop_data.schedule = thread_state->schedule[tid];
  task->loop_data.chunk = thread_state->chunk[tid];
  task->loop_data.stride = thread_state->stride[tid];
}

INLINE void __kmp_task_restore_loop_data(kmp_task_state_t *task) {
  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  int tid = task->task_data.thread_id;
  thread_state->loop_ub[tid] = task->loop_data.loop_ub;
  thread_state->next_lb[tid] = task->loop_data.next_lb;
  thread_state->schedule[tid] = task->loop_data.schedule;
  thread_state->chunk[tid] = task->loop_data.chunk;
  thread_state->stride[tid] = task->loop_data.stride;
}

INLINE void __kmp_init_team_task(kmp_task_state_t *task,
                                 kmp_task_state_t *source) {
  __kmp_task_copy_data(task, source);
  task->prev = source;
  task->task_data.flags |= TASK_IN_PARALLEL | TASK_IMPLICIT;
}

INLINE void __kmp_init_task_from_team(kmp_task_state_t *task,
                                      kmp_task_state_t *team_task) {
  __kmp_task_copy_data(task, team_task);
  task->prev = team_task->prev;
  task->task_data.thread_id = __kmp_get_local_id();
}

INLINE void __kmp_init_team_state(kmp_team_state_t *team) {
  __kmp_init_level_zero_task(&team->level_zero_task);
}


///
/// Others
///

INLINE void __kmp_inc_parallel_level(bool is_active) {
  int tid = __kmp_get_local_id();
  kmp_local_state_t *local_state = __kmp_get_local_state();
  // Update both active/non-active parallel level
  local_state->parallel_level[tid] +=
      1 + (is_active ? KMP_ACTIVE_PARALLEL_BUMP : 0);
}

INLINE void __kmp_dec_parallel_level(bool is_active) {
  int tid = __kmp_get_local_id();
  kmp_local_state_t *local_state = __kmp_get_local_state();
  // Update both active/non-active parallel level
  local_state->parallel_level[tid] -=
      1 + (is_active ? KMP_ACTIVE_PARALLEL_BUMP : 0);
}

INLINE int __kmp_get_level(int tid) {
  kmp_local_state_t *local_state = __kmp_get_local_state();
  int level = local_state->parallel_level[tid] & (KMP_ACTIVE_PARALLEL_BUMP - 1);
  return level;
}

#endif // INTERNAL_H
#endif // INTEL_COLLAB
