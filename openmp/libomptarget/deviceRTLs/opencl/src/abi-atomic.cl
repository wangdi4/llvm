#if INTEL_COLLAB
//===--- abi-atomic.cl - Atomic operation support -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains entry points for supporting atomic operations.
//
//===----------------------------------------------------------------------===//

#include "omptarget-opencl.h"

///
/// Atomics
///
/// TODO: relax memory order later (current: seq_cst)

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

/// Use intrinsics for 8-byte data
#define KMPC_ATOMIC_IMPL_INTRINSIC8(DATANAME, DATATYPE, OPNAME)                \
  /* __kmpc_atomic_DATANAME_OPNAME(*lhs, rhs) */                               \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    atom_##OPNAME((volatile global long *)lhs, rhs);                           \
  }

/// Use intrinsics for binary and/or
#define KMPC_ATOMIC_IMPL_INTRINSIC_B(DATANAME, DATATYPE, OPNAME)               \
  /* __kmpc_atomic_DATANAME_OPNAMEb(*lhs, rhs) */                              \
  KMPC_ATOMIC_FN(DATANAME, OPNAME##b, DATATYPE) {                              \
    atomic_fetch_##OPNAME((atomic_##DATATYPE *)lhs, rhs);                      \
  }

/// Use intrinsics for 8-byte binary and/or
#define KMPC_ATOMIC_IMPL_INTRINSIC8_B(DATANAME, DATATYPE, OPNAME)              \
  /* __kmpc_atomic_DATANAME_OPNAMEb(*lhs, rhs) */                              \
  KMPC_ATOMIC_FN(DATANAME, OPNAME##b, DATATYPE) {                              \
    atom_##OPNAME((volatile global long *)lhs, rhs);                           \
  }

/// Use intrinsics for capture atomics with generic OP
#define KMPC_ATOMIC_IMPL_INTRINSIC_CPT(DATANAME, DATATYPE, OPNAME, OP)         \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    DATATYPE captured = atomic_fetch_##OPNAME((atomic_##DATATYPE *)lhs, rhs);  \
    if (flag)                                                                  \
      captured = OP(captured, rhs, DATATYPE);                                  \
    return captured;                                                           \
  }

/// Use intrinsics for 8-byte capture atomics with generic OP
#define KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(DATANAME, DATATYPE, OPNAME, OP)        \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    DATATYPE captured = atom_##OPNAME((volatile global long*)lhs, rhs);        \
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
      captured = OP(captured, rhs, DATATYPE);                                  \
    return captured;                                                           \
  }

/// Use intrinsics for 8-byte binary and/or capture atomics
#define KMPC_ATOMIC_IMPL_INTRINSIC8_B_CPT(DATANAME, DATATYPE, OPNAME, OP)      \
  /* __kmpc_atomic_DATANAME_OPNAMEb_cpt(*lhs, rhs, flag) */                    \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME##b, DATATYPE) {                          \
    DATATYPE captured = atom_##OPNAME((volatile global long *)lhs, rhs);       \
    if (flag)                                                                  \
      captured = OP(captured, rhs, DATATYPE);                                  \
    return captured;                                                           \
  }

/// Use cmpxchg with generic OP
#define KMPC_ATOMIC_IMPL_CMPXCHG(DATANAME, DATATYPE, OPNAME, OP)               \
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

/// Use cmpxchg with generic OP for 8-byte data
#define KMPC_ATOMIC_IMPL_CMPXCHG8(DATANAME, DATATYPE, OPNAME, OP)              \
  /* __kmpc_atomic_DATANAME_OPNAME(*lhs, rhs) */                               \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    volatile global long *obj = (volatile global long *)lhs;                   \
    bool done = false;                                                         \
    while (!done) {                                                            \
      DATATYPE prev = atom_add(obj, 0);                                        \
      DATATYPE next = OP(prev, rhs, DATATYPE);                                 \
      done = ((long)prev == atom_cmpxchg(obj, prev, next));                    \
    }                                                                          \
  }

/// Use cmpxchg for capture atomics with generic OP
#define KMPC_ATOMIC_IMPL_CMPXCHG_CPT(DATANAME, DATATYPE, OPNAME, OP)           \
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

/// Use cmpxchg for capture atomics with generic OP for 8-byte data
#define KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(DATANAME, DATATYPE, OPNAME, OP)          \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    volatile global long *obj = (volatile global long *)lhs;                   \
    bool done = false;                                                         \
    DATATYPE prev, next;                                                       \
    while (!done) {                                                            \
      prev = atom_add(obj, 0);                                                 \
      next = OP(prev, rhs, DATATYPE);                                          \
      done = ((long)prev == atom_cmpxchg(obj, prev, next));                    \
    }                                                                          \
    return flag ? next : prev;                                                 \
  }

/// Use cmpxchg with type cast, generic OP
#define KMPC_ATOMIC_IMPL_CMPXCHG_CAST(DATANAME, DATATYPE, BASETYPE, OPNAME,    \
                                      OP)                                      \
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

/// Use cmpxchg with type cast, generic OP for 8-byte data
#define KMPC_ATOMIC_IMPL_CMPXCHG8_CAST(DATANAME, DATATYPE, BASETYPE, OPNAME,   \
                                       OP)                                     \
  /* __kmpc_atomic_DATANAME_OPNAME(*lhs, rhs, flag) */                         \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    union {                                                                    \
      long base;                                                               \
      DATATYPE data;                                                           \
    } prev, next;                                                              \
    volatile global long *obj = (volatile global long *)lhs;                   \
    bool done = false;                                                         \
    while (!done) {                                                            \
      prev.base = atom_add(obj, 0);                                            \
      next.data = OP(prev.data, rhs, DATATYPE);                                \
      done = (prev.base == atom_cmpxchg(obj, prev.base, next.base));           \
    }                                                                          \
  }

/// Use cmpxchg with type cast, generic OP for capture atomics
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
      next.data = OP(prev.data, rhs, DATATYPE);                                \
      done = atomic_compare_exchange_strong(obj, &prev.base, next.base);       \
    }                                                                          \
    return flag ? next.data : prev.data;                                       \
  }

/// Use cmpxchg with type cast, generic OP for 8-byte capture atomics
#define KMPC_ATOMIC_IMPL_CMPXCHG8_CAST_CPT(DATANAME, DATATYPE, BASETYPE,       \
                                           OPNAME, OP)                         \
  /* __kmpc_atomic_DATANAME_OPNAME_cpt(*lhs, rhs, flag) */                     \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    union {                                                                    \
      long base;                                                               \
      DATATYPE data;                                                           \
    } prev, next;                                                              \
    volatile global long *obj = (volatile global long *)lhs;                   \
    bool done = false;                                                         \
    while (!done) {                                                            \
      prev.base = atom_add(obj, 0);                                            \
      next.data = OP(prev.data, rhs, DATATYPE);                                \
      done = (prev.base == atom_cmpxchg(obj, prev.base, next.base));           \
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
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, shl, OP_SHL)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, shr, OP_SHR)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4, int, andl, OP_AND)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4, int, add, OP_ADD)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4, int, sub, OP_SUB)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed4, int, or, OP_ORB)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4, int, xor, OP_XOR)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed4, int, and, OP_ANDB)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4, int, min, OP_MIN)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4, int, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, shl, OP_SHL)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, shr, OP_SHR)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4, int, andl, OP_AND)
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
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, shl, OP_SHL)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, shr, OP_SHR)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG(fixed4u, uint, andl, OP_AND)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4u, uint, add, OP_ADD)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4u, uint, sub, OP_SUB)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed4u, uint, or, OP_ORB)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4u, uint, xor, OP_XOR)
KMPC_ATOMIC_IMPL_INTRINSIC_B_CPT(fixed4u, uint, and, OP_ANDB)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4u, uint, min, OP_MIN)
KMPC_ATOMIC_IMPL_INTRINSIC_CPT(fixed4u, uint, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, shl, OP_SHL)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, shr, OP_SHR)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_CPT(fixed4u, uint, andl, OP_AND)
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
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, add, OP_ADD)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, sub, OP_SUB)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, min, OP_MIN)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST(float4, float, int, andl, OP_AND)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, add, OP_ADD)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, sub, OP_SUB)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, min, OP_MIN)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG_CAST_CPT(float4, float, int, andl, OP_AND)
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
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8, long, add)
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8, long, sub)
KMPC_ATOMIC_IMPL_INTRINSIC8_B(fixed8, long, or)
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8, long, xor)
KMPC_ATOMIC_IMPL_INTRINSIC8_B(fixed8, long, and)
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8, long, min)
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8, long, max)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8, long, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8, long, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8, long, shl, OP_SHL)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8, long, shr, OP_SHR)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8, long, andl, OP_AND)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8, long, add, OP_ADD)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8, long, sub, OP_SUB)
KMPC_ATOMIC_IMPL_INTRINSIC8_B_CPT(fixed8, long, or, OP_ORB)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8, long, xor, OP_XOR)
KMPC_ATOMIC_IMPL_INTRINSIC8_B_CPT(fixed8, long, and, OP_ANDB)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8, long, min, OP_MIN)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8, long, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8, long, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8, long, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8, long, shl, OP_SHL)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8, long, shr, OP_SHR)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8, long, andl, OP_AND)
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
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8u, ulong, add)
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8u, ulong, sub)
KMPC_ATOMIC_IMPL_INTRINSIC8_B(fixed8u, ulong, or)
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8u, ulong, xor)
KMPC_ATOMIC_IMPL_INTRINSIC8_B(fixed8u, ulong, and)
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8u, ulong, min)
KMPC_ATOMIC_IMPL_INTRINSIC8(fixed8u, ulong, max)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8u, ulong, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8u, ulong, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8u, ulong, shl, OP_SHL)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8u, ulong, shr, OP_SHR)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8u, ulong, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG8(fixed8u, ulong, andl, OP_AND)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8u, ulong, add, OP_ADD)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8u, ulong, sub, OP_SUB)
KMPC_ATOMIC_IMPL_INTRINSIC8_B_CPT(fixed8u, ulong, or, OP_ORB)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8u, ulong, xor, OP_XOR)
KMPC_ATOMIC_IMPL_INTRINSIC8_B_CPT(fixed8u, ulong, and, OP_ANDB)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8u, ulong, min, OP_MIN)
KMPC_ATOMIC_IMPL_INTRINSIC8_CPT(fixed8u, ulong, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8u, ulong, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8u, ulong, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8u, ulong, shl, OP_SHL)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8u, ulong, shr, OP_SHR)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8u, ulong, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG8_CPT(fixed8u, ulong, andl, OP_AND)
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
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST(float8, double, long, add, OP_ADD)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST(float8, double, long, sub, OP_SUB)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST(float8, double, long, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST(float8, double, long, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST(float8, double, long, min, OP_MIN)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST(float8, double, long, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST(float8, double, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST(float8, double, long, andl, OP_AND)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST_CPT(float8, double, long, add, OP_ADD)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST_CPT(float8, double, long, sub, OP_SUB)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST_CPT(float8, double, long, mul, OP_MUL)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST_CPT(float8, double, long, div, OP_DIV)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST_CPT(float8, double, long, min, OP_MIN)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST_CPT(float8, double, long, max, OP_MAX)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST_CPT(float8, double, long, orl, OP_OR)
KMPC_ATOMIC_IMPL_CMPXCHG8_CAST_CPT(float8, double, long, andl, OP_AND)
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
  if (size == sizeof(uint)) {
    KMP_ATOMIC_LOAD_EXPLICIT(uint, ptr, ret, order);
#if KMP_ATOMIC_FIXED8_SUPPORTED
  } else if (size == sizeof(ulong)) {
    *(long *)ret = atom_add((volatile global long *)ptr, 0);
#endif
  } else {
    printf("WARNING: Device does not support %zu-bit atomics\n", 8 * size);
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
  if (size == sizeof(uint)) {
    KMP_ATOMIC_STORE_EXPLICIT(uint, ptr, val, order);
#if KMP_ATOMIC_FIXED8_SUPPORTED
  } else if (size == sizeof(ulong)) {
    atom_xchg((volatile global long *)ptr, *(long *)val);
#endif
  } else {
    printf("WARNING: Device does not support %zu-bit atomics\n", 8 * size);
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
  bool ret = true; // avoid common-case hanging
  if (size == sizeof(uint)) {
    KMP_ATOMIC_COMPXCHG_EXPLICIT(uint, ret, ptr, expected, desired,
                                 success_order, failure_order);
#if KMP_ATOMIC_FIXED8_SUPPORTED
  } else if (size == sizeof(ulong)) {
    long old_exp = *(long *)expected;
    long old_val = atom_cmpxchg((volatile global long *)ptr, old_exp,
                                *(long *)desired);
    if (old_val != old_exp) {
      *(long *)expected = old_val;
      ret = false;
    }
#endif
  } else {
    printf("WARNING: Device does not support %zu-bit atomics\n", 8 * size);
  }
  return ret;
}

#endif // INTEL_COLLAB
