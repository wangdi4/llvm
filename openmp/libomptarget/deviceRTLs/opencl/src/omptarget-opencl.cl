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

/// Team barrier
INLINE void __kmp_team_barrier(int global_id) {
  // We need to implement this if we allow single-team-multi-group mapping
  // TODO: use global_id to access the barrier object for the team.
}

void __kmpc_barrier() {
  // Built-in work group barrier
  work_group_barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
}

///
/// Atomics
///
/// TODO: relax memory order later (current: seq_cst)

/// OP definitions
#define OP_MIN(X, Y, DT) ((X) < (Y) ? (X) : (Y))
#define OP_MAX(X, Y, DT) ((X) > (Y) ? (X) : (Y))
#define TO_LOGIC(X, DT) ((X) != (DT)0 ? 1 : 0)
#define OP_OR(X, Y, DT) ((DT)(TO_LOGIC(X, DT) || TO_LOGIC(Y, DT)))
#define OP_AND(X, Y, DT) ((DT)(TO_LOGIC(X, DT) && TO_LOGIC(Y, DT)))

/// Fallback implementation
// TODO: lock is not working at work-item level, so we don't have any way to
// implement this correctly only in software.
#define KMPC_ATOMIC_IMPL_FALLBACK(DATANAME, DATATYPE, OPNAME, OP)              \
  /* __kmpc_atomic_DATANAME_OPNAME(*lhs, rhs) */                               \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    printf("Device does not support this atomic data type: %s\n", #DATATYPE);  \
  }

/// Fallback for binary and/or
#define KMPC_ATOMIC_IMPL_FALLBACK_B(DATANAME, DATATYPE, OPNAME, OP)            \
  /* __kmpc_atomic_DATANAME_OPNAMEb(*lhs, rhs) */                              \
  KMPC_ATOMIC_FN(DATANAME, OPNAME##b, DATATYPE) {                              \
    printf("Device does not support this atomic data type: %s\n", #DATATYPE);  \
  }

/// Fallback for capture atomics
#define KMPC_ATOMIC_IMPL_FALLBACK_CPT(DATANAME, DATATYPE, OPNAME, OP)          \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    printf("Device does not support this atomic data type: %s\n", #DATATYPE);  \
    return *lhs;                                                               \
  }

/// Fallback for binary and/or capture atomics
#define KMPC_ATOMIC_IMPL_FALLBACK_B_CPT(DATANAME, DATATYPE, OPNAME, OP)        \
  /* __kmpc_atomic_DATANAME_OPNAMEb_cpt(*lhs, rhs, flag) */                    \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME##b, DATATYPE) {                          \
    printf("Device does not support this atomic data type: %s\n", #DATATYPE);  \
    return *lhs;                                                               \
  }

/// Use intrinsics
#define KMPC_ATOMIC_IMPL_INTRINSIC(DATANAME, DATATYPE, OPNAME)                 \
  /* __kmpc_atomic_DATANAME_OPNAME(*lhs, rhs) */                               \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    atomic_fetch_##OPNAME((atomic_##DATATYPE *)lhs, rhs);                      \
  }

/// Use intrinsics for binary and/or
#define KMPC_ATOMIC_IMPL_INTRINSIC_B(DATANAME, DATATYPE, OPNAME)               \
  /* __kmpc_atomic_DATANAME_OPNAMEb(*lhs, rhs) */                              \
  KMPC_ATOMIC_FN(DATANAME, OPNAME##b, DATATYPE) {                              \
    atomic_fetch_##OPNAME((atomic_##DATATYPE *)lhs, rhs);                      \
  }

/// Use intrinsics for capture atomics
#define KMPC_ATOMIC_IMPL_INTRINSIC_CPT(DATANAME, DATATYPE, OPNAME, OP)         \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    DATATYPE captured = atomic_fetch_##OPNAME((atomic_##DATATYPE *)lhs, rhs);  \
    if (flag)                                                                  \
      captured = captured OP rhs;                                              \
    return captured;                                                           \
  }

/// Use intrinsics for capture atomics with generic OP
#define KMPC_ATOMIC_IMPL_INTRINSIC_BASE_CPT(DATANAME, DATATYPE, OPNAME, OP)    \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    DATATYPE captured = atomic_fetch_##OPNAME((atomic_##DATATYPE *)lhs, rhs);  \
    if (flag)                                                                  \
      captured = OP(captured, rhs, DATATYPE);                                  \
    return captured;                                                           \
  }

/// Use intrinsics for binary and/or capture atomics
#define KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(DATANAME, DATATYPE, OPNAME, OP)       \
  /* __kmpc_atomic_DATANAME_OPNAMEb_cpt(*lhs, rhs, flag) */                    \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME##b, DATATYPE) {                          \
    DATATYPE captured = atomic_fetch_##OPNAME((atomic_##DATATYPE *)lhs, rhs);  \
    if (flag)                                                                  \
      captured = captured OP rhs;                                              \
    return captured;                                                           \
  }

/// Use cmpxchg
#define KMPC_ATOMIC_IMPL_CMPXCHG(DATANAME, DATATYPE, OPNAME, OP)               \
  /* __kmpc_atomic_DATANAME_OPNAME(*lhs, rhs) */                               \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    atomic_##DATATYPE *obj = (atomic_##DATATYPE *)lhs;                         \
    bool done = false;                                                         \
    while (!done) {                                                            \
      DATATYPE prev = atomic_load(obj);                                        \
      DATATYPE next = prev OP rhs;                                             \
      done = atomic_compare_exchange_strong(obj, &prev, next);                 \
    }                                                                          \
  }

/// Use cmpxchg for capture atomics
#define KMPC_ATOMIC_IMPL_CMPXCHG_CPT(DATANAME, DATATYPE, OPNAME, OP)           \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    atomic_##DATATYPE *obj = (atomic_##DATATYPE *)lhs;                         \
    bool done = false;                                                         \
    DATATYPE prev, next;                                                       \
    while (!done) {                                                            \
      prev = atomic_load(obj);                                                 \
      next = prev OP rhs;                                                      \
      done = atomic_compare_exchange_strong(obj, &prev, next);                 \
    }                                                                          \
    return flag ? next : prev;                                                 \
  }

/// Use cmpxchg with generic OP
#define KMPC_ATOMIC_IMPL_CMPXCHG_BASE(DATANAME, DATATYPE, OPNAME, OP)          \
  /* __kmpc_atomic_DATANAME_OPNAME(*lhs, rhs) */                               \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    atomic_##DATATYPE *obj = (atomic_##DATATYPE *)lhs;                         \
    bool done = false;                                                         \
    while (!done) {                                                            \
      DATATYPE prev = atomic_load(obj);                                        \
      DATATYPE next = OP(prev, rhs, DATATYPE);                                 \
      done = atomic_compare_exchange_strong(obj, &prev, next);                 \
    }                                                                          \
  }

/// Use cmpxchg for capture atomics with generic OP
#define KMPC_ATOMIC_IMPL_CMPXCHG_BASE_CPT(DATANAME, DATATYPE, OPNAME, OP)      \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    atomic_##DATATYPE *obj = (atomic_##DATATYPE *)lhs;                         \
    bool done = false;                                                         \
    DATATYPE prev, next;                                                       \
    while (!done) {                                                            \
      prev = atomic_load(obj);                                                 \
      next = OP(prev, rhs, DATATYPE);                                          \
      done = atomic_compare_exchange_strong(obj, &prev, next);                 \
    }                                                                          \
    return flag ? next : prev;                                                 \
  }

/// Use cmpxchg with type cast
#define KMPC_ATOMIC_IMPL_CMPXCHG_CAST(DATANAME, DATATYPE, BASETYPE, OPNAME, OP)\
  /* __kmpc_atomic_DATANAME_OPNAME(*lhs, rhs) */                               \
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

/// Use cmpxchg with type cast for capture atomics
#define KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(DATANAME, DATATYPE, BASETYPE,        \
                                          OPNAME, OP)                          \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
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
    return flag ? next.data : prev.data;                                       \
  }

/// Use cmpxchg with type cast, generic OP
#define KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE(DATANAME, DATATYPE, BASETYPE,       \
                                           OPNAME, OP)                         \
  /* __kmpc_atomic_DATANAME_OPNAME(*lhs, rhs, flag) */                         \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    union {                                                                    \
      BASETYPE base;                                                           \
      DATATYPE data;                                                           \
    } prev, next;                                                              \
    atomic_##BASETYPE *obj = (atomic_##BASETYPE *)lhs;                         \
    bool done = false;                                                         \
    while (!done) {                                                            \
      prev.base = atomic_load(obj);                                            \
      next.data = OP(prev.data, rhs, DATATYPE);                                \
      done = atomic_compare_exchange_strong(obj, &prev.base, next.base);       \
    }                                                                          \
  }

/// Use cmpxchg with type cast, generic OP for capture atomics
#define KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE_CPT(DATANAME, DATATYPE,             \
                                               BASETYPE, OPNAME, OP)           \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    union {                                                                    \
      BASETYPE base;                                                           \
      DATATYPE data;                                                           \
    } prev, next;                                                              \
    atomic_##BASETYPE *obj = (atomic_##BASETYPE *)lhs;                         \
    bool done = false;                                                         \
    while (!done) {                                                            \
      prev.base = atomic_load(obj);                                            \
      next.data = OP(prev.data, rhs, DATATYPE);                                \
      done = atomic_compare_exchange_strong(obj, &prev.base, next.base);       \
    }                                                                          \
    return flag ? next.data : prev.data;                                       \
  }

/// 4-byte fixed atomics
#if KMP_ATOMIC_FIXED4_SUPPORTED
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4, int, add)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4, int, sub)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed4, int, or)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4, int, xor)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed4, int, and)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4, int, min)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4, int, max)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, shr, >>)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE(fixed4, int, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE(fixed4, int, andl, OP_AND)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4, int, add, +)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4, int, sub, -)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed4, int, or, |)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4, int, xor, ^)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed4, int, and, &)
KMPC_ATOMIC_IMPL_INTRINSIC_BASE_CPT(fixed4, int, min, OP_MIN)
KMPC_ATOMIC_IMPL_INTRINSIC_BASE_CPT(fixed4, int, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, shr, >>)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE_CPT(fixed4, int, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE_CPT(fixed4, int, andl, OP_AND)
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
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4, int, add, +)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4, int, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_B_CPT(fixed4, int, or, |)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4, int, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK_B_CPT(fixed4, int, and, &)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4, int, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4, int, div, /)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4, int, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4, int, shr, >>)
#endif

/// 4-byte unsigned fixed atomics
#if KMP_ATOMIC_FIXED4_SUPPORTED
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4u, uint, add)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4u, uint, sub)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed4u, uint, or)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4u, uint, xor)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed4u, uint, and)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4u, uint, min)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed4u, uint, max)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, shr, >>)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE(fixed4u, uint, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE(fixed4u, uint, andl, OP_AND)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4u, uint, add, +)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4u, uint, sub, -)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed4u, uint, or, |)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4u, uint, xor, ^)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed4u, uint, and, &)
KMPC_ATOMIC_IMPL_INTRINSIC_BASE_CPT(fixed4u, uint, min, OP_MIN)
KMPC_ATOMIC_IMPL_INTRINSIC_BASE_CPT(fixed4u, uint, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, shr, >>)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE_CPT(fixed4u, uint, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE_CPT(fixed4u, uint, andl, OP_AND)
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
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4u, uint, add, +)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4u, uint, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_B_CPT(fixed4u, uint, or, |)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4u, uint, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK_B_CPT(fixed4u, uint, and, &)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4u, uint, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4u, uint, div, /)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4u, uint, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed4u, uint, shr, >>)
#endif

/// 4-byte float atomics
#if KMP_ATOMIC_FIXED4_SUPPORTED
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, add, +)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, sub, -)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE(float4, float, int, min, OP_MIN)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE(float4, float, int, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE(float4, float, int, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE(float4, float, int, andl, OP_AND)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, add, +)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, sub, -)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE_CPT(float4, float, int, min, OP_MIN)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE_CPT(float4, float, int, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE_CPT(float4, float, int, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE_CPT(float4, float, int, andl, OP_AND)
#else
KMPC_ATOMIC_IMPL_FALLBACK(float4, float, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(float4, float, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK(float4, float, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(float4, float, div, /)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float4, float, add, +)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float4, float, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float4, float, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float4, float, div, /)
#endif

/// 8-byte fixed atomics
#if KMP_ATOMIC_FIXED8_SUPPORTED
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8, long, add)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8, long, sub)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed8, long, or)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8, long, xor)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed8, long, and)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8, long, min)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8, long, max)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8, long, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8, long, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8, long, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8, long, shr, >>)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE(fixed8, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE(fixed8, long, andl, OP_AND)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed8, long, add, +)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed8, long, sub, -)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed8, long, or, |)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed8, long, xor, ^)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed8, long, and, &)
KMPC_ATOMIC_IMPL_INTRINSIC_BASE_CPT(fixed8, long, min, OP_MIN)
KMPC_ATOMIC_IMPL_INTRINSIC_BASE_CPT(fixed8, long, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed8, long, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed8, long, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed8, long, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed8, long, shr, >>)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE_CPT(fixed8, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE_CPT(fixed8, long, andl, OP_AND)
#else
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed8, long, or, |)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed8, long, and, &)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, min, OP_MIN)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, max, OP_MAX)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, andl, OP_AND)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, div, /)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8, long, shr, >>)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, add, +)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_B_CPT(fixed8, long, or, |)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK_B_CPT(fixed8, long, and, &)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, min, OP_MIN)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, max, OP_MAX)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, andl, OP_AND)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, div, /)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8, long, shr, >>)
#endif

/// 8-byte unsigned fixed atomics
#if KMP_ATOMIC_FIXED8_SUPPORTED
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8u, ulong, add)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8u, ulong, sub)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed8u, ulong, or)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8u, ulong, xor)
KMPC_ATOMIC_IMPL_INTRINSIC_B(fixed8u, ulong, and)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8u, ulong, min)
KMPC_ATOMIC_IMPL_INTRINSIC(fixed8u, ulong, max)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8u, ulong, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8u, ulong, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8u, ulong, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed8u, ulong, shr, >>)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE(fixed8u, ulong, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE(fixed8u, ulong, andl, OP_AND)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed8u, ulong, add, +)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed8u, ulong, sub, -)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed8u, ulong, or, |)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed8u, ulong, xor, ^)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed8u, ulong, and, &)
KMPC_ATOMIC_IMPL_INTRINSIC_BASE_CPT(fixed8u, ulong, min, OP_MIN)
KMPC_ATOMIC_IMPL_INTRINSIC_BASE_CPT(fixed8u, ulong, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed8u, ulong, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed8u, ulong, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed8u, ulong, shl, <<)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed8u, ulong, shr, >>)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE_CPT(fixed8u, ulong, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_BASE_CPT(fixed8u, ulong, andl, OP_AND)
#else
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed8u, ulong, or, |)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK_B(fixed8u, ulong, and, &)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, min, OP_MIN)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, max, OP_MAX)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, orl, OP_OR)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, andl, OP_AND)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, div, /)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK(fixed8u, ulong, shr, >>)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, add, +)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_B_CPT(fixed8u, ulong, or, |)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, xor, ^)
KMPC_ATOMIC_IMPL_FALLBACK_B_CPT(fixed8u, ulong, and, &)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, min, OP_MIN)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, max, OP_MAX)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, orl, OP_OR)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, andl, OP_AND)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, div, /)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, shl, <<)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(fixed8u, ulong, shr, >>)
#endif

/// 8-byte float atomics
#if KMP_ATOMIC_FIXED8_SUPPORTED
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float8, double, long, add, +)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float8, double, long, sub, -)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float8, double, long, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float8, double, long, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE(float8, double, long, min, OP_MIN)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE(float8, double, long, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE(float8, double, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE(float8, double, long, andl, OP_AND)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float8, double, long, add, +)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float8, double, long, sub, -)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float8, double, long, mul, *)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float8, double, long, div, /)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE_CPT(float8, double, long, min, OP_MIN)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE_CPT(float8, double, long, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE_CPT(float8, double, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_BASE_CPT(float8, double, long, andl, OP_AND)
#else
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, add, +)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, div, /)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, min, OP_MIN)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, max, OP_MAX)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, orl, OP_OR)
KMPC_ATOMIC_IMPL_FALLBACK(float8, double, andl, OP_AND)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float8, double, add, +)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float8, double, sub, -)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float8, double, mul, *)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float8, double, div, /)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float8, double, min, OP_MIN)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float8, double, max, OP_MAX)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float8, double, orl, OP_OR)
KMPC_ATOMIC_IMPL_FALLBACK_CPT(float8, double, andl, OP_AND)
#endif


// Needed to use this code due to a compiler issue with casting
#define KMP_ATOMIC_LOAD_EXPLICIT(dtype, ptr, ret, order)                       \
  switch (order) {                                                             \
  case memory_order_relaxed:                                                   \
    *((dtype *)ret) = atomic_load_explicit((volatile atomic_##dtype *)ptr,     \
                                           memory_order_relaxed);              \
    break;                                                                     \
  case memory_order_acquire:                                                   \
    *((dtype *)ret) = atomic_load_explicit((volatile atomic_##dtype *)ptr,     \
                                           memory_order_acquire);              \
    break;                                                                     \
  case memory_order_release:                                                   \
    *((dtype *)ret) = atomic_load_explicit((volatile atomic_##dtype *)ptr,     \
                                           memory_order_release);              \
    break;                                                                     \
  case memory_order_acq_rel:                                                   \
    *((dtype *)ret) = atomic_load_explicit((volatile atomic_##dtype *)ptr,     \
                                           memory_order_acq_rel);              \
    break;                                                                     \
  case memory_order_seq_cst:                                                   \
  default:                                                                     \
    *((dtype *)ret) = atomic_load_explicit((volatile atomic_##dtype *)ptr,     \
                                           memory_order_seq_cst);              \
    break;                                                                     \
  }

EXTERN void __kmpc_atomic_load(size_t size, void *ptr, void *ret, int order) {
  if (size <= sizeof(uint)) {
    KMP_ATOMIC_LOAD_EXPLICIT(uint, ptr, ret, order);
  } else if (size <= sizeof(ulong)) {
#if KMP_ATOMIC_FIXED8_SUPPORTED
    KMP_ATOMIC_LOAD_EXPLICIT(ulong, ptr, ret, order);
#else
    printf("WARNING: Device does not support 64-bit atomics\n");
#endif
  }
}

#define KMP_ATOMIC_STORE_EXPLICIT(dtype, ptr, val, order)                      \
  switch (order) {                                                             \
  case memory_order_relaxed:                                                   \
    atomic_store_explicit((volatile atomic_##dtype *)ptr, *(dtype *)val,       \
                          memory_order_relaxed);                               \
    break;                                                                     \
  case memory_order_acquire:                                                   \
    atomic_store_explicit((volatile atomic_##dtype *)ptr, *(dtype *)val,       \
                          memory_order_acquire);                               \
    break;                                                                     \
  case memory_order_release:                                                   \
    atomic_store_explicit((volatile atomic_##dtype *)ptr, *(dtype *)val,       \
                          memory_order_release);                               \
    break;                                                                     \
  case memory_order_acq_rel:                                                   \
    atomic_store_explicit((volatile atomic_##dtype *)ptr, *(dtype *)val,       \
                          memory_order_acq_rel);                               \
    break;                                                                     \
  case memory_order_seq_cst:                                                   \
  default:                                                                     \
    atomic_store_explicit((volatile atomic_##dtype *)ptr, *(dtype *)val,       \
                          memory_order_seq_cst);                               \
    break;                                                                     \
  }

EXTERN void __kmpc_atomic_store(size_t size, void *ptr, void *val, int order) {
  if (size <= sizeof(uint)) {
    KMP_ATOMIC_STORE_EXPLICIT(uint, ptr, val, order);
  } else if (size <= sizeof(ulong)) {
#if KMP_ATOMIC_FIXED8_SUPPORTED
    KMP_ATOMIC_STORE_EXPLICIT(ulong, ptr, val, order);
#else
    printf("WARNING: Device does not support 64-bit atomics\n");
#endif
  }
}

// Failure order is set to relaxed as it cannot be stronger than success order.
#define KMP_ATOMIC_COMPXCHG_EXPLICIT(dtype, ret, ptr, expected, desired,       \
                                     success_order, failure_order)             \
  switch (success_order) {                                                     \
  case memory_order_relaxed:                                                   \
    ret = atomic_compare_exchange_strong_explicit((atomic_##dtype *)ptr,       \
        (dtype *)expected, *(dtype *)desired, memory_order_relaxed,            \
        memory_order_relaxed);                                                 \
    break;                                                                     \
  case memory_order_acquire:                                                   \
    ret = atomic_compare_exchange_strong_explicit((atomic_##dtype *)ptr,       \
        (dtype *)expected, *(dtype *)desired, memory_order_acquire,            \
        memory_order_relaxed);                                                 \
    break;                                                                     \
  case memory_order_release:                                                   \
    ret = atomic_compare_exchange_strong_explicit((atomic_##dtype *)ptr,       \
        (dtype *)expected, *(dtype *)desired, memory_order_release,            \
        memory_order_relaxed);                                                 \
    break;                                                                     \
  case memory_order_acq_rel:                                                   \
    ret = atomic_compare_exchange_strong_explicit((atomic_##dtype *)ptr,       \
        (dtype *)expected, *(dtype *)desired, memory_order_acq_rel,            \
        memory_order_relaxed);                                                 \
    break;                                                                     \
  case memory_order_seq_cst:                                                   \
  default:                                                                     \
    ret = atomic_compare_exchange_strong_explicit((atomic_##dtype *)ptr,       \
        (dtype *)expected, *(dtype *)desired, memory_order_seq_cst,            \
        memory_order_relaxed);                                                 \
    break;                                                                     \
  }

EXTERN bool __kmpc_atomic_compare_exchange(size_t size, void *ptr,
                                           void *expected, void *desired,
                                           int success_order,
                                           int failure_order) {
  bool ret = false;
  if (size <= sizeof(uint)) {
    KMP_ATOMIC_COMPXCHG_EXPLICIT(uint, ret, ptr, expected, desired,
                                 success_order, failure_order);
  } else if (size <= sizeof(ulong)) {
#if KMP_ATOMIC_FIXED8_SUPPORTED
    KMP_ATOMIC_COMPXCHG_EXPLICIT(ulong, ret, ptr, expected, desired,
                                 success_order, failure_order);
#else
    printf("WARNING: Device does not support 64-bit atomics\n");
#endif
  }
  return ret;
}

///
/// Other __kmpc_* entries
///

EXTERN int __kmpc_master() {
  return (__kmp_get_local_id() == 0) ? KMP_TRUE : KMP_FALSE;
}

EXTERN void __kmpc_end_master() {
  // nothing to be done
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
