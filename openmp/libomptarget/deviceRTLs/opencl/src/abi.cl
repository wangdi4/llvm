#if INTEL_COLLAB
//===--- abi.cl - Entry points for the compiler ---------------------------===//
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
#include "internal.h"

///
/// Entries for kernel configuration
///

/// Initialize global kernel parameters -- not used now
EXTERN void __kmpc_kernel_init_params(void *params) {
  // TODO: future parameters
}

/// Initialize kernel execution -- only master calls this
EXTERN void __kmpc_kernel_init(int thread_limit, short needs_rtl) {
  KMP_ASSERT(needs_rtl,
             "__kmpc_kernel_init() expects device RTL is required");

  GLOBAL.assume_simple_spmd_mode = 0;

  __kmp_set_execution_flags(EXEC_MODE_GENERIC, RTL_STATE_INITIALIZED);

  kmp_local_state_t *local_state = __kmp_get_local_state();

  // Initialize parallel level
  for (int i = 0; i < __kmp_get_local_size(); ++i)
    local_state->parallel_level[i] = 0;

  KMP_ASSERT(__kmp_get_local_id() == __kmp_get_master_id(),
             "Only the master can call __kmpc_kernel_init()");

  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  int tid = __kmp_get_logical_thread_id(false);

  __kmp_init_thread_state(thread_state, tid);

  kmp_team_state_t *team = &thread_state->team;
  __kmp_init_team_state(team);
  __kmp_set_top_level_task(thread_state, tid, &team->level_zero_task);

  local_state->num_threads = __kmp_get_num_workers();
  local_state->thread_limit = thread_limit;
  __kmp_init_work_barrier(local_state);
}

/// Finalize kernel execution -- only master calls this
EXTERN void __kmpc_kernel_fini(short is_rtl_initialized) {
  KMP_ASSERT(is_rtl_initialized,
             "Reached __kmpc_kernel_fini() without initializing RTL");
  __kmp_get_local_state()->work_fn = 0;
}

/// Initialize SPMD kernel execution -- every work/thread should call this
EXTERN void __kmpc_spmd_kernel_init(int thread_limit, short needs_rtl,
                                    short needs_data_sharing) {
  GLOBAL.assume_simple_spmd_mode = 0;

  __kmp_set_execution_flags(EXEC_MODE_SPMD,
      needs_rtl ? RTL_STATE_INITIALIZED : RTL_STATE_UNINITIALIZED);

  int tid = __kmp_get_local_id();
  kmp_local_state_t *local_state = __kmp_get_local_state();
  local_state->parallel_level[tid] = 1;
  if (__kmp_get_local_size() > 1)
    local_state->parallel_level[tid] += KMP_ACTIVE_PARALLEL_BUMP;

  if (!needs_rtl) {
    __kmp_team_barrier();
    return;
  }

  // Simple target region with nested parallel region requires RTL

  kmp_thread_state_t *thread_state = __kmp_get_thread_state();
  if (tid == 0) {
    __kmp_init_team_state(&thread_state->team);
  }
  __kmp_team_barrier();

  kmp_task_state_t *new_task = &thread_state->level_one_task[tid];
  KMP_ASSERT(new_task, "Invalid level-0 task data");

  __kmp_init_level_one_task(new_task, &thread_state->team.level_zero_task);
  __kmp_set_top_level_task(thread_state, tid, new_task);

  if (needs_data_sharing) {
    // not sure if we need this
  }
}

EXTERN void __kmpc_spmd_kernel_fini(short needs_rtl) {
  // nothing to be done for now
}

EXTERN char __kmpc_is_spmd_exec_mode() {
  return __kmp_is_spmd_mode();
}


///
/// Barrier
///

/// Barrier for a subset of a team -- when all workers are active
void __kmpc_work_barrier() {
  kmp_local_state_t *local_state = __kmp_get_local_state();
  if (__kmp_is_spmd_mode()) {
    // Use fast barrier
    __kmp_team_barrier();
  } else {
    // Use counting barrier
    int num_sub_group = local_state->team_threads / get_max_sub_group_size();
    if (num_sub_group > 0)
      __kmp_work_barrier(&local_state->work_barrier, num_sub_group);
  }
}

// Barrier for entire team
void __kmpc_barrier() {
  __kmp_team_barrier();
}

void __kmpc_init_runtime() {
  __kmp_init_locals();
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
  if (GLOBAL.assume_simple_spmd_mode)
    return (__kmp_get_local_id() == 0) ? KMP_TRUE : KMP_FALSE;

  return (__kmp_get_omp_thread_id(__kmp_is_spmd_mode()) == 0);
}

EXTERN void __kmpc_end_master() {
  // nothing to be done
}

EXTERN uint __kmpc_global_thread_num(void *loc) {
  return 0;
}

///
/// Support for generic/hierarchical target region
///

/// Check if the current work belong to the master sub group
EXTERN int __kmpc_master_sub_group() {
  if (get_sub_group_id() == get_num_sub_groups() - 1)
    return KMP_TRUE;
  else
    return KMP_FALSE;
}

/// Check if the current work is the leader of the master sub group
EXTERN int __kmpc_master_sub_group_leader() {
  if (__kmpc_master_sub_group() && get_sub_group_local_id() == 0)
    return KMP_TRUE;
  else
    return KMP_FALSE;
}


///
/// Support for reduction
///

#define KMPC_REDUCTION(OP, OPNAME, DATATYPE)                                   \
  EXTERN void __kmpc_reduction_##OPNAME##_##DATATYPE(const uint Id,            \
      const uint Size, void *LocalResult, void *Output) {                      \
    KMP_ASSERT((Size & (Size - 1)) == 0,                                       \
               "__kmpc_reduction*() operation requires power of 2 team size"); \
    work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);            \
    DATATYPE *lhs, rhs;                                                        \
    for (uint stride = Size / 2; stride > 0; stride /= 2) {                    \
      work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);          \
      if (Id < stride) {                                                       \
        lhs = &((DATATYPE *)LocalResult)[Id];                                  \
        rhs = ((DATATYPE *)LocalResult)[Id + stride];                          \
        *lhs = OP(*lhs, rhs, DATATYPE);                                        \
      }                                                                        \
    }                                                                          \
    if (Id == 0) {                                                             \
      lhs = (DATATYPE *)Output;                                                \
      rhs = *(DATATYPE *)LocalResult;                                          \
      *lhs = OP(*lhs, rhs, DATATYPE);                                          \
    }                                                                          \
  }

KMPC_REDUCTION(OP_ADD, add, int)
KMPC_REDUCTION(OP_ADD, add, long)
KMPC_REDUCTION(OP_ADD, add, float)
KMPC_REDUCTION(OP_ADD, add, double)


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
