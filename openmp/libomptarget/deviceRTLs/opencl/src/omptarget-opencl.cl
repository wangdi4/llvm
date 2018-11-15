#if INTEL_COLLAB
//===--- omptarget-opencl.cl - OpenMP device runtime for OpenCL -*--- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
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


///
/// Barrier
///

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

/// Global barrier to be used internally.
INLINE void __kmp_global_barrier() {
  if (__kmp_get_num_groups() == 1) {
    work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    return;
  }
  __kmp_barrier_counting(&gstate.g_barrier);
}

/// Team barrier
INLINE void __kmp_team_barrier(int global_id) {
  // We need to implement this if we allow single-team-multi-group mapping
  // TODO: use global_id to access the barrier object for the team.
}

void __kmpc_barrier(ident_t *id, int global_id) {
  // This should work fine as long as team is mapped to at most one work group.
  __kmp_global_barrier();
}

///
/// Atomics
///
/// TODO: relax memory order later (current: seq_cst)

/// Fallback implementation
// TODO: lock is not working at work-item level, so we don't have any way to
// implement this correctly only in software.
#define KMPC_ATOMIC_IMPL_FALLBACK(DATANAME, DATATYPE, OPNAME, OP)              \
  /* __kmpc_atomic_DATANAME_OPNAME(ident_t *id, int global_id, lhs, rhs) */    \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    *lhs = *lhs OP rhs;                                                        \
    printf("Device does not support this atomic data type: %s\n", #DATATYPE);  \
  }

/// Fallback for binary and/or
#define KMPC_ATOMIC_IMPL_FALLBACK_B(DATANAME, DATATYPE, OPNAME, OP)            \
  /* __kmpc_atomic_DATANAME_OPNAME(ident_t *id, int global_id, lhs, rhs) */    \
  KMPC_ATOMIC_FN_B(DATANAME, OPNAME, DATATYPE) {                               \
    *lhs = *lhs OP rhs;                                                        \
  }

/// Use intrinsics
#define KMPC_ATOMIC_IMPL_INTRINSIC(DATANAME, DATATYPE, OPNAME)                 \
  /* __kmpc_atomic_DATANAME_OPNAME(ident_t *id, int global_id, lhs, rhs) */    \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    atomic_fetch_##OPNAME((atomic_##DATATYPE *)lhs, rhs);                      \
  }

/// Use intrinsics for binary and/or
#define KMPC_ATOMIC_IMPL_INTRINSIC_B(DATANAME, DATATYPE, OPNAME)               \
  /* __kmpc_atomic_DATANAME_OPNAME(ident_t *id, int global_id, lhs, rhs) */    \
  KMPC_ATOMIC_FN_B(DATANAME, OPNAME, DATATYPE) {                               \
    atomic_fetch_##OPNAME((atomic_##DATATYPE *)lhs, rhs);                      \
  }

/// Use cmpxchg
#define KMPC_ATOMIC_IMPL_CMPXCHG(DATANAME, DATATYPE, OPNAME, OP)               \
  /* __kmpc_atomic_DATANAME_OPNAME(ident_t *id, int global_id, lhs, rhs) */    \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    atomic_##DATATYPE *obj = (atomic_##DATATYPE *)lhs;                         \
    bool done = false;                                                         \
    while (!done) {                                                            \
      DATATYPE prev = atomic_load(obj);                                        \
      DATATYPE next = prev OP rhs;                                             \
      done = atomic_compare_exchange_strong(obj, &prev, next);                 \
    }                                                                          \
  }

/// Use cmpxchg with type cast
#define KMPC_ATOMIC_IMPL_CMPXCHG_CAST(DATANAME, DATATYPE, BASETYPE, OPNAME, OP)\
  /* __kmpc_atomic_DATANAME_OPNAME(ident_t *id, int global_id, lhs, rhs) */    \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    union {                                                                    \
      BASETYPE base;                                                           \
      DATATYPE data;                                                           \
    } prev, next;                                                              \
    atomic_##BASETYPE *obj = (atomic_##BASETYPE *)lhs;                         \
    bool done = false;                                                         \
    while (!done) {                                                            \
      prev.base = atomic_load(obj);                                            \
      next.data = prev.data OP rhs;                                            \
      done = atomic_compare_exchange_strong(obj, &prev.base, next.base);       \
    }                                                                          \
  }

/// 4-byte fixed atomics
#if KMP_ATOMIC_FIXED4_SUPPORTED
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4, int, add)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4, int, sub)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed4, int, or)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4, int, xor)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed4, int, and)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, shr, >>)
#else
KMPC_ATOMIC_IMPL_FALLBACK(fixed4, int, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4, int, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed4, int, or, |)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4, int, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed4, int, and, &)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4, int, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4, int, div, /)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4, int, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4, int, shr, >>)
#endif

/// 4-byte unsigned fixed atomics
#if KMP_ATOMIC_FIXED4_SUPPORTED
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4u, uint, add)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4u, uint, sub)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed4u, uint, or)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4u, uint, xor)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed4u, uint, and)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, shr, >>)
#else
KMPC_ATOMIC_IMPL_FALLBACK(fixed4u, uint, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4u, uint, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed4u, uint, or, |)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4u, uint, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed4u, uint, and, &)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4u, uint, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4u, uint, div, /)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4u, uint, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK(fixed4u, uint, shr, >>)
#endif

/// 4-byte float atomics
#if KMP_ATOMIC_FIXED4_SUPPORTED
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, add, +)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, sub, -)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, div, /)
#else
KMPC_ATOMIC_IMPL_FALLBACK(float4, float, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(float4, float, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK(float4, float, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(float4, float, div, /)
#endif

/// 8-byte fixed atomics
#if KMP_ATOMIC_FIXED8_SUPPORTED
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8, long, add)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8, long, sub)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed8, long, or)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8, long, xor)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed8, long, and)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8, long, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8, long, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8, long, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8, long, shr, >>)
#else
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed8, long, or, |)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed8, long, and, &)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, div, /)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, shr, >>)
#endif

/// 8-byte unsigned fixed atomics
#if KMP_ATOMIC_FIXED8_SUPPORTED
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8u, ulong, add)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8u, ulong, sub)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed8u, ulong, or)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8u, ulong, xor)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed8u, ulong, and)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8u, ulong, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8u, ulong, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8u, ulong, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8u, ulong, shr, >>)
#else
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, or, |)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, and, &)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, div, /)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, shr, >>)
#endif

/// 8-byte float atomics
#if KMP_ATOMIC_FIXED8_SUPPORTED
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float8, double, long, add, +)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float8, double, long, sub, -)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float8, double, long, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float8, double, long, div, /)
#else
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, div, /)
#endif


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
#endif // INTEL_COLLAB
