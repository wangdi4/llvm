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

#if !KMP_ASSUME_SIMPLE_SPMD_MODE
/// Initialize global kernel parameters -- not used now
EXTERN void __kmpc_kernel_init_params(void *params) {
  // TODO: future parameters
}

/// Initialize kernel execution -- only master calls this
EXTERN void __kmpc_kernel_init(int thread_limit, short needs_rtl) {
  KMP_ASSERT(needs_rtl,
             "__kmpc_kernel_init() expects device RTL is required");

  __omp_spirv_global_data.assume_simple_spmd_mode = 0;

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
  __omp_spirv_global_data.assume_simple_spmd_mode = 0;

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
#endif // !KMP_ASSUME_SIMPLE_SPMD_MODE

// Barrier for entire team
void __kmpc_barrier() {
  __kmp_team_barrier();
}

#if !KMP_ASSUME_SIMPLE_SPMD_MODE
void __kmpc_init_runtime() {
  __kmp_init_locals();
}
#endif // !KMP_ASSUME_SIMPLE_SPMD_MODE

/// Named barrier support. These are empty intentionally for BE that do not
/// replace them with intrinsics.
EXTERN void __kmpc_nbarrier_init(uint nbarrier_count) {}

EXTERN void __kmpc_nbarrier_wait(uint nbarrier_id) {}

EXTERN void __kmpc_nbarrier_signal(
    uint nbarrier_id, uint num_producers, uint num_consumers, uint op_type,
    uint fence_type) {}


///
/// Support for critical section
///

// Requires correctly initialized "*name" (=0).
EXTERN void __kmpc_critical(kmp_critical_name *name) {
  __kmp_acquire_lock((int *)name);
}

/// Begin critical section with hint -- hint is ignored
EXTERN void __kmpc_critical_with_hint(kmp_critical_name *name, uint hint) {
  __kmp_acquire_lock((int *)name);
}

EXTERN void __kmpc_end_critical(kmp_critical_name *name) {
  __kmp_release_lock((int *)name);
}

/// Begin critical section with explicit SIMD
EXTERN void __kmpc_critical_simd(kmp_critical_name *name) {
  __kmp_acquire_lock_simd((int *)name);
}

/// Begin critical section with hint, explicit SIMD -- hint is ignored
EXTERN void __kmpc_critical_with_hint_simd(kmp_critical_name *name, uint hint) {
  __kmp_acquire_lock_simd((int *)name);
}

/// End critical section with explicit SIMD
EXTERN void __kmpc_end_critical_simd(kmp_critical_name *name) {
  __kmp_release_lock_simd((int *)name);
}


///
/// Other __kmpc_* entries
///

EXTERN int __kmpc_master() {
  return __kmpc_masked(NULL, 0, 0);
}

EXTERN void __kmpc_end_master() {
  // nothing to be done
}

EXTERN int __kmpc_masked(ident_t *loc, int gtid, int filter) {
#if KMP_ASSUME_SIMPLE_SPMD_MODE
  return ((int)__kmp_get_local_id() == filter) ? KMP_TRUE : KMP_FALSE;
#else
  return (__kmp_get_omp_thread_id(__kmp_is_spmd_mode()) == filter);
#endif
}

EXTERN void __kmpc_end_masked(ident_t *loc, int gtid) {
  // nothing to be done
}

EXTERN int __kmpc_single(ident_t *loc, int gtid) {
  // This is a conforming implementation.
  return __kmpc_master();
}

EXTERN void __kmpc_end_single(ident_t *loc, int gtid) {
  // nothing to be done
}

EXTERN uint __kmpc_global_thread_num(ident_t *loc) {
  return 0;
}


///
/// Support for generic/hierarchical target region
///

/// Check if the current work belong to the master sub group
EXTERN int __kmpc_master_sub_group() {
  if (__spirv_BuiltInSubgroupId == __spirv_BuiltInNumSubgroups - 1)
    return KMP_TRUE;
  else
    return KMP_FALSE;
}

/// Check if the current work is the leader of the master sub group
EXTERN int __kmpc_master_sub_group_leader() {
  if (__kmpc_master_sub_group() &&
      __spirv_BuiltInSubgroupLocalInvocationId == 0)
    return KMP_TRUE;
  else
    return KMP_FALSE;
}

/// Check if current work is the active sub group leader
EXTERN int __kmpc_active_sub_group_leader() {
  if (__kmp_get_active_sub_group_leader_id() ==
      __spirv_BuiltInSubgroupLocalInvocationId)
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
#if HAVE_FP64_SUPPORT
KMPC_REDUCTION(OP_ADD, add, double)
#endif // HAVE_FP64_SUPPORT


///
/// Dynamic memory allocation support
///

INLINE void *__kmp_alloc_fetch_add(size_t align, size_t size) {
  if (size == 0)
    return NULL;

  if (align == 0)
    align = KMP_MAX_ALIGNMENT;

  size_t alloc_size = (align - 1) + size;

  uintptr_t mem = atomic_fetch_add(
      (volatile atomic_uintptr_t *)(&__omp_spirv_program_data.dyna_mem_cur),
      (ptrdiff_t)alloc_size);

  if (mem + alloc_size <= __omp_spirv_program_data.dyna_mem_ub) {
    uintptr_t aligned = (((uintptr_t)mem - 1) + align) & (~(align - 1));
    return (void *)aligned;
  }

  PRINT0("Warning: __kmp_alloc returns NULL (out of memory)\n");
  return NULL;
}

/// 64-bit memory block descriptor stores block usage for 32 blocks, where each
/// two bits store the state of a single block as follows.
/// 0x3: block itself is used as a single allocation
/// 0x2: block is lower/upper bound of an allocation that use multiple blocks
/// 0x1: block is busy (part of multi-block allocation but it is not boundary)
/// 0x0: block is free
#define KMP_MEM_NUM_BLOCKS_PER_DESC 32
#define KMP_MEM_NUM_DESCS_PER_COUNTER 32
#define KMP_MEM_NUM_BLOCKS_PER_COUNTER \
  (KMP_MEM_NUM_BLOCKS_PER_DESC * KMP_MEM_NUM_DESCS_PER_COUNTER)
#define KMP_MEM_DESC_BUSY_SBLOCK ((ulong)0x3)
#define KMP_MEM_DESC_BUSY_MBOUND ((ulong)0x2)
#define KMP_MEM_DESC_BUSY_MBLOCK ((ulong)0x1)
#define KMP_UINT32_MAX (~((uint)0))

INLINE uint __kmp_claim_blocks(global kmp_mem_heap_t *heap, uint desc_id,
                               uint num_blocks) {
  // The last descriptor may have less number of blocks
  uint block_ub = 0;
  if (heap->num_block_desc == desc_id + 1)
    block_ub = heap->num_blocks % KMP_MEM_NUM_BLOCKS_PER_DESC;
  if (block_ub == 0)
    block_ub = KMP_MEM_NUM_BLOCKS_PER_DESC;

  // Prepare block mask for the requested number of blocks
  ulong block_mask = 0;
  if (num_blocks < KMP_MEM_NUM_BLOCKS_PER_DESC)
    block_mask = ((ulong)1 << (2 * num_blocks)) - 1;
  else
    block_mask = ~block_mask;

  // Prepare desired bits for the requested number of blocks
  ulong desired_bits = 0;
  if (num_blocks == 1) {
    desired_bits = KMP_MEM_DESC_BUSY_SBLOCK;
  } else {
    desired_bits = KMP_MEM_DESC_BUSY_MBOUND << 2;
    for (uint i = 1; i < num_blocks - 1; i++)
      desired_bits = (desired_bits | KMP_MEM_DESC_BUSY_MBLOCK) << 2;
    desired_bits = desired_bits | KMP_MEM_DESC_BUSY_MBOUND;
  }

  volatile global atomic_ulong *desc =
      (volatile global atomic_ulong *)&heap->block_desc[desc_id];

  // Try to claim free blocks with CAS on the free descriptor bits
  for (uint i = 0; i < block_ub; i += num_blocks) {
    if (block_ub - i < num_blocks)
      break;
    if ((atomic_load(desc) & block_mask) == 0) {
      ulong expected = atomic_load(desc) & ~block_mask;
      ulong desired = expected | desired_bits;
      if (atomic_compare_exchange_strong(desc, &expected, desired))
        return i + desc_id * KMP_MEM_NUM_BLOCKS_PER_DESC;
    }
    desired_bits <<= (2 * num_blocks);
    block_mask <<= (2 * num_blocks);
  }
  return KMP_UINT32_MAX;
}

INLINE void *__kmp_alloc(size_t align, size_t size) {
  global static volatile atomic_uint total_num_allocs = ATOMIC_VAR_INIT(0);
  // Use this number to distribute block search
  uint num_allocs = atomic_fetch_add(&total_num_allocs, 1U);
  void *ret = NULL;
  global kmp_mem_pool_t *pool =
      (global kmp_mem_pool_t *)__omp_spirv_program_data.dyna_mem_pool;
  if (!pool)
    return ret;

  for (uint i = 0; i < pool->num_heaps; i++) {
    global kmp_mem_heap_t *heap = &pool->heap_desc[i];
    if (heap->max_size < size)
      continue; // Requires heap with bigger block size
    uint num_blocks = (size + heap->block_size - 1) / heap->block_size;
    uint block_start = num_allocs % heap->num_block_desc;
    for (uint j = block_start; j < heap->num_block_desc + block_start;) {
      uint k = j % heap->num_block_desc;
      uint counter_id = k / KMP_MEM_NUM_DESCS_PER_COUNTER;
      volatile atomic_uint *counter =
          (volatile atomic_uint *)&heap->block_counter[counter_id];
      if (j % KMP_MEM_NUM_DESCS_PER_COUNTER == 0) {
        // Skip descriptors that are fully used.
        uint remaining_blocks =
            KMP_MEM_NUM_BLOCKS_PER_COUNTER - atomic_load(counter);
        if (remaining_blocks < num_blocks) {
          j += KMP_MEM_NUM_DESCS_PER_COUNTER;
          continue;
        }
      }
      uint block_id = __kmp_claim_blocks(heap, k, num_blocks);
      if (block_id != KMP_UINT32_MAX) {
        ret = (void *)(heap->alloc_base + block_id * heap->block_size);
        atomic_fetch_add(counter, num_blocks);
        return ret;
      }
      j++;
    }
  }
  return ret;
}

INLINE void __kmp_dealloc(void *ptr) {
  uintptr_t ptrint = (uintptr_t)ptr;
  kmp_mem_pool_t *pool =
      (kmp_mem_pool_t *)__omp_spirv_program_data.dyna_mem_pool;
  if (!pool || !ptrint)
    return;

  for (uint i = 0; i < pool->num_heaps; i++) {
    kmp_mem_heap_t *heap = (kmp_mem_heap_t *)&pool->heap_desc[i];
    if (ptrint < heap->alloc_base)
      return; // Invalid pointer range
    uintptr_t heap_ub = heap->alloc_base + pool->heap_size;
    if (ptrint >= heap_ub)
      continue; // Memory does not belong to this heap

    // Obtain block descriptor ID and offset
    uint block_id = (ptrint - heap->alloc_base) / heap->block_size;
    uint desc_id = block_id / KMP_MEM_NUM_BLOCKS_PER_DESC;
    uint desc_offset = block_id % KMP_MEM_NUM_BLOCKS_PER_DESC;

    // Prepare desired block mask while checking the descriptor encoding
    volatile atomic_ulong *desc =
        (volatile atomic_ulong *)&heap->block_desc[desc_id];
    ulong desc_val = atomic_load(desc);
    desc_val >>= (2 * desc_offset);
    bool bounded = false;
    ulong desired_mask = 0;
    uint num_blocks = 0;
    while (true) {
      ulong mask = desc_val & 0x3;
      if (!mask)
        break; // Unused block
      desired_mask = (desired_mask << 2) | 0x3;
      num_blocks++;
      if (mask == KMP_MEM_DESC_BUSY_SBLOCK) {
        break; // Single-block allocation
      } else if (mask == KMP_MEM_DESC_BUSY_MBOUND) {
        if (bounded)
          break; // Multi-block allocation with matching bound
        bounded = true;
      }
      desc_val >>= 2;
    }
    desired_mask = ~(desired_mask << (2 * desc_offset));

    // We need to use this loop since we would like to update partial bits of
    // the descriptor.
    bool done = false;
    do {
      ulong expected = atomic_load(desc);
      ulong desired = (expected & desired_mask);
      done = atomic_compare_exchange_strong(desc, &expected, desired);
    } while (!done);
    uint counter_id = desc_id / KMP_MEM_NUM_DESCS_PER_COUNTER;
    volatile atomic_uint *counter =
        (volatile atomic_uint *)&heap->block_counter[counter_id];
    atomic_fetch_sub(counter, num_blocks);
    break;
  }
}

EXTERN void *__kmpc_alloc(int gtid, size_t size, omp_allocator_handle_t al) {
  return __kmpc_aligned_alloc(gtid, 0, size, al);
}

EXTERN void *__kmpc_aligned_alloc(int gtid, size_t align, size_t size,
                                  omp_allocator_handle_t al) {
  // Use available allocator. We expect only one of them is enabled.
  if (__omp_spirv_program_data.dyna_mem_cur)
    return __kmp_alloc_fetch_add(align, size);
  else if (__omp_spirv_program_data.dyna_mem_pool)
    return __kmp_alloc(align, size);
  return NULL;
}

EXTERN void __kmpc_free(int gtid, void *ptr, omp_allocator_handle_t al) {
  if (__omp_spirv_program_data.dyna_mem_pool)
    __kmp_dealloc(ptr);
}

#endif // INTEL_COLLAB
