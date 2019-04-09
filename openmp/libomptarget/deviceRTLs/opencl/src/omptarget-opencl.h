#if INTEL_COLLAB // -*- C++ -*-
//===--- omptarget-opencl.h - header for the OpenCL device RTL ----*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains declaration/definition of types, functions, and macros
//
//===----------------------------------------------------------------------===//

///
/// Misc. definitions
///

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

#define INLINE static inline

#define KMP_TRUE 1
#define KMP_FALSE 0
#define KMP_UNSPECIFIED -1

///
/// Device information
///

#ifndef KMP_DEVICE_GEN9_GT4
#define KMP_DEVICE_GEN9_GT4
#endif

/// Constants to be used when allocating internal data structures.
/// TODO: numbers may be too conservative (or large).
#ifdef KMP_DEVICE_GEN9_GT2 // 3 subslices
#define KMP_MAX_WORKGROUP_SIZE 256
#define KMP_MAX_NUM_GROUPS (24 * 7)
#define KMP_MAX_LOG_NUM_GROUPS 16
#define KMP_MAX_GLOBAL_SIZE KMP_MAX_WORKGROUP_SIZE * KMP_MAX_NUM_GROUPS
#define KMP_MAX_DATA_SIZE 16 // quad
#define KMP_ATOMIC_FIXED4_SUPPORTED 1
#define KMP_ATOMIC_FIXED8_SUPPORTED 0
#define KMP_USE_BARRIER_COUNTING

#elif defined(KMP_DEVICE_GEN9_GT3) // 6 subslices
#define KMP_MAX_WORKGROUP_SIZE 256
#define KMP_MAX_NUM_GROUPS (48 * 7)
#define KMP_MAX_LOG_NUM_GROUPS 16
#define KMP_MAX_GLOBAL_SIZE KMP_MAX_WORKGROUP_SIZE * KMP_MAX_NUM_GROUPS
#define KMP_MAX_DATA_SIZE 16 // quad
#define KMP_ATOMIC_FIXED4_SUPPORTED 1
#define KMP_ATOMIC_FIXED8_SUPPORTED 0
#define KMP_USE_BARRIER_COUNTING

#elif defined(KMP_DEVICE_GEN9_GT4) // 9 subslices
#define KMP_MAX_WORKGROUP_SIZE 256
#define KMP_MAX_NUM_GROUPS (72 * 7)
#define KMP_MAX_LOG_NUM_GROUPS 16
#define KMP_MAX_GLOBAL_SIZE KMP_MAX_WORKGROUP_SIZE * KMP_MAX_NUM_GROUPS
#define KMP_MAX_DATA_SIZE 16 // quad
#define KMP_ATOMIC_FIXED4_SUPPORTED 1
#define KMP_ATOMIC_FIXED8_SUPPORTED 0
#define KMP_USE_BARRIER_COUNTING

#elif defined(KMP_DEVICE_GEN12)
// TODO
#else
#error Unsupported device!!!
#endif

/// Enable extensions if available
#if KMP_ATOMIC_FIXED8_SUPPORTED
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
#endif


///
/// Types
///

typedef struct kmp_barrier_counting {
  atomic_uint count;
  atomic_uint go;
} kmp_barrier_counting_t;

typedef struct kmp_barrier_dissem_node {
  volatile char mine[2][KMP_MAX_LOG_NUM_GROUPS];
  volatile char *partner[2][KMP_MAX_LOG_NUM_GROUPS];
  int parity;
  int sense;
} kmp_barrier_dissem_node_t;

typedef struct kmp_barrier_dissem {
  kmp_barrier_dissem_node_t node[KMP_MAX_NUM_GROUPS];
  int num_rounds;
  atomic_int num_initialized;
} kmp_barrier_dissem_t;

typedef union kmp_barrier {
  kmp_barrier_counting_t counting;
  kmp_barrier_dissem_t dissem;
} kmp_barrier_t;

typedef struct kmp_global_state {
  kmp_barrier_t g_barrier;              // global barrier
} kmp_global_state_t;


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
  uint work_dim = get_work_dim();
  size_t ret = get_group_id(0);
  if (work_dim == 1)
    return ret;
  ret += get_num_groups(0) * get_group_id(1);
  if (work_dim == 2)
    return ret;
  ret += get_num_groups(0) * get_num_groups(1) * get_group_id(2);
  return (work_dim == 3) ? ret : 0;
}

/// Return global size
INLINE size_t __kmp_get_global_size() {
  uint work_dim = get_work_dim();
  size_t ret = get_global_size(0);
  if (work_dim == 1)
    return ret;
  ret *= get_global_size(1);
  if (work_dim == 2)
    return ret;
  ret *= get_global_size(2);
  return (work_dim == 3) ? ret : 1;
}

/// Return local size
INLINE size_t __kmp_get_local_size() {
  uint work_dim = get_work_dim();
  size_t ret = get_local_size(0);
  if (work_dim == 1)
    return ret;
  ret *= get_local_size(1);
  if (work_dim == 2)
    return ret;
  ret *= get_local_size(2);
  return (work_dim == 3) ? ret : 1;
}

/// Return number of groups
INLINE size_t __kmp_get_num_groups() {
  uint work_dim = get_work_dim();
  size_t ret = get_num_groups(0);
  if (work_dim == 1)
    return ret;
  ret *= get_num_groups(1);
  if (work_dim == 2)
    return ret;
  ret *= get_num_groups(2);
  return (work_dim == 3) ? ret : 1;
}


///
/// Barriers
///

EXTERN void __kmpc_barrier();


///
/// Atomics
///

/// Signature
#define KMPC_ATOMIC_FN(DATANAME, OPTYPE, DATATYPE)                             \
EXTERN void __kmpc_atomic_##DATANAME##_##OPTYPE                                \
  (DATATYPE *lhs, DATATYPE rhs)

/// Signature for capture atomics
#define KMPC_ATOMIC_FN_CPT(DATANAME, OPTYPE, DATATYPE)                         \
EXTERN DATATYPE __kmpc_atomic_##DATANAME##_##OPTYPE##_cpt                      \
  (DATATYPE *lhs, DATATYPE rhs, int flag)

/// 4-byte fixed
KMPC_ATOMIC_FN(fixed4, add, int);
KMPC_ATOMIC_FN(fixed4, sub, int);
KMPC_ATOMIC_FN(fixed4, orb, int);
KMPC_ATOMIC_FN(fixed4, xor, int);
KMPC_ATOMIC_FN(fixed4, andb, int);
KMPC_ATOMIC_FN(fixed4, mul, int);
KMPC_ATOMIC_FN(fixed4, div, int);
KMPC_ATOMIC_FN(fixed4, shl, int);
KMPC_ATOMIC_FN(fixed4, shr, int);
KMPC_ATOMIC_FN(fixed4, min, int);
KMPC_ATOMIC_FN(fixed4, max, int);
KMPC_ATOMIC_FN(fixed4, orl, int);
KMPC_ATOMIC_FN(fixed4, andl, int);

/// 4-byte fixed capture
KMPC_ATOMIC_FN_CPT(fixed4, add, int);
KMPC_ATOMIC_FN_CPT(fixed4, sub, int);
KMPC_ATOMIC_FN_CPT(fixed4, orb, int);
KMPC_ATOMIC_FN_CPT(fixed4, xor, int);
KMPC_ATOMIC_FN_CPT(fixed4, andb, int);
KMPC_ATOMIC_FN_CPT(fixed4, mul, int);
KMPC_ATOMIC_FN_CPT(fixed4, div, int);
KMPC_ATOMIC_FN_CPT(fixed4, shl, int);
KMPC_ATOMIC_FN_CPT(fixed4, shr, int);
KMPC_ATOMIC_FN_CPT(fixed4, min, int);
KMPC_ATOMIC_FN_CPT(fixed4, max, int);
KMPC_ATOMIC_FN_CPT(fixed4, orl, int);
KMPC_ATOMIC_FN_CPT(fixed4, andl, int);

/// 4-byte unsigned fixed
KMPC_ATOMIC_FN(fixed4u, add, uint);
KMPC_ATOMIC_FN(fixed4u, sub, uint);
KMPC_ATOMIC_FN(fixed4u, orb, uint);
KMPC_ATOMIC_FN(fixed4u, xor, uint);
KMPC_ATOMIC_FN(fixed4u, andb, uint);
KMPC_ATOMIC_FN(fixed4u, mul, uint);
KMPC_ATOMIC_FN(fixed4u, div, uint);
KMPC_ATOMIC_FN(fixed4u, shl, uint);
KMPC_ATOMIC_FN(fixed4u, shr, uint);
KMPC_ATOMIC_FN(fixed4u, min, uint);
KMPC_ATOMIC_FN(fixed4u, max, uint);
KMPC_ATOMIC_FN(fixed4u, orl, uint);
KMPC_ATOMIC_FN(fixed4u, andl, uint);

/// 4-byte unsigned fixed capture
KMPC_ATOMIC_FN_CPT(fixed4u, add, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, sub, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, orb, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, xor, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, andb, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, mul, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, div, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, shl, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, shr, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, min, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, max, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, orl, uint);
KMPC_ATOMIC_FN_CPT(fixed4u, andl, uint);

/// 4-byte float
KMPC_ATOMIC_FN(float4, add, float);
KMPC_ATOMIC_FN(float4, sub, float);
KMPC_ATOMIC_FN(float4, mul, float);
KMPC_ATOMIC_FN(float4, div, float);
KMPC_ATOMIC_FN(float4, min, float);
KMPC_ATOMIC_FN(float4, max, float);
KMPC_ATOMIC_FN(float4, orl, float);
KMPC_ATOMIC_FN(float4, andl, float);

/// 4-byte float capture
KMPC_ATOMIC_FN_CPT(float4, add, float);
KMPC_ATOMIC_FN_CPT(float4, sub, float);
KMPC_ATOMIC_FN_CPT(float4, mul, float);
KMPC_ATOMIC_FN_CPT(float4, div, float);
KMPC_ATOMIC_FN_CPT(float4, min, float);
KMPC_ATOMIC_FN_CPT(float4, max, float);
KMPC_ATOMIC_FN_CPT(float4, orl, float);
KMPC_ATOMIC_FN_CPT(float4, andl, float);

/// 8-byte fixed
KMPC_ATOMIC_FN(fixed8, add, long);
KMPC_ATOMIC_FN(fixed8, sub, long);
KMPC_ATOMIC_FN(fixed8, orb, long);
KMPC_ATOMIC_FN(fixed8, xor, long);
KMPC_ATOMIC_FN(fixed8, andb, long);
KMPC_ATOMIC_FN(fixed8, mul, long);
KMPC_ATOMIC_FN(fixed8, div, long);
KMPC_ATOMIC_FN(fixed8, shl, long);
KMPC_ATOMIC_FN(fixed8, shr, long);
KMPC_ATOMIC_FN(fixed8, min, long);
KMPC_ATOMIC_FN(fixed8, max, long);
KMPC_ATOMIC_FN(fixed8, orl, long);
KMPC_ATOMIC_FN(fixed8, andl, long);

/// 8-byte fixed capture
KMPC_ATOMIC_FN_CPT(fixed8, add, long);
KMPC_ATOMIC_FN_CPT(fixed8, sub, long);
KMPC_ATOMIC_FN_CPT(fixed8, orb, long);
KMPC_ATOMIC_FN_CPT(fixed8, xor, long);
KMPC_ATOMIC_FN_CPT(fixed8, andb, long);
KMPC_ATOMIC_FN_CPT(fixed8, mul, long);
KMPC_ATOMIC_FN_CPT(fixed8, div, long);
KMPC_ATOMIC_FN_CPT(fixed8, shl, long);
KMPC_ATOMIC_FN_CPT(fixed8, shr, long);
KMPC_ATOMIC_FN_CPT(fixed8, min, long);
KMPC_ATOMIC_FN_CPT(fixed8, max, long);
KMPC_ATOMIC_FN_CPT(fixed8, orl, long);
KMPC_ATOMIC_FN_CPT(fixed8, andl, long);

/// 8-byte unsigned fixed
KMPC_ATOMIC_FN(fixed8u, add, ulong);
KMPC_ATOMIC_FN(fixed8u, sub, ulong);
KMPC_ATOMIC_FN(fixed8u, orb, ulong);
KMPC_ATOMIC_FN(fixed8u, xor, ulong);
KMPC_ATOMIC_FN(fixed8u, andb, ulong);
KMPC_ATOMIC_FN(fixed8u, mul, ulong);
KMPC_ATOMIC_FN(fixed8u, div, ulong);
KMPC_ATOMIC_FN(fixed8u, shl, ulong);
KMPC_ATOMIC_FN(fixed8u, shr, ulong);
KMPC_ATOMIC_FN(fixed8u, min, ulong);
KMPC_ATOMIC_FN(fixed8u, max, ulong);
KMPC_ATOMIC_FN(fixed8u, orl, ulong);
KMPC_ATOMIC_FN(fixed8u, andl, ulong);

/// 8-byte unsigned fixed capture
KMPC_ATOMIC_FN_CPT(fixed8u, add, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, sub, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, orb, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, xor, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, andb, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, mul, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, div, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, shl, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, shr, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, min, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, max, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, orl, ulong);
KMPC_ATOMIC_FN_CPT(fixed8u, andl, ulong);

/// 8-byte float
KMPC_ATOMIC_FN(float8, add, double);
KMPC_ATOMIC_FN(float8, sub, double);
KMPC_ATOMIC_FN(float8, mul, double);
KMPC_ATOMIC_FN(float8, div, double);
KMPC_ATOMIC_FN(float8, min, double);
KMPC_ATOMIC_FN(float8, max, double);
KMPC_ATOMIC_FN(float8, orl, double);
KMPC_ATOMIC_FN(float8, andl, double);

/// 8-byte float capture
KMPC_ATOMIC_FN_CPT(float8, add, double);
KMPC_ATOMIC_FN_CPT(float8, sub, double);
KMPC_ATOMIC_FN_CPT(float8, mul, double);
KMPC_ATOMIC_FN_CPT(float8, div, double);
KMPC_ATOMIC_FN_CPT(float8, min, double);
KMPC_ATOMIC_FN_CPT(float8, max, double);
KMPC_ATOMIC_FN_CPT(float8, orl, double);
KMPC_ATOMIC_FN_CPT(float8, andl, double);

/// TODO: more data types

///
/// Support for generalized atomics code generation
///

EXTERN void __kmpc_atomic_load(size_t, void *, void *, int);
EXTERN void __kmpc_atomic_store(size_t, void *, void *, int);
EXTERN bool __kmpc_atomic_compare_exchange(size_t, void *, void *, void *, int,
                                           int);

///
/// Other __kmpc_* entries
///

EXTERN int __kmpc_master();
EXTERN void __kmpc_end_master();


///
/// OpenMP* RTL routines
///

EXTERN int omp_get_team_num(void);

EXTERN int omp_get_num_teams(void);

EXTERN int omp_get_team_size(int);

EXTERN int omp_get_thread_num(void);

EXTERN int omp_get_num_threads(void);

EXTERN int omp_in_parallel(void);

EXTERN int omp_get_max_threads(void);

EXTERN int omp_get_device_num(void);

EXTERN int omp_get_num_devices(void);

EXTERN int omp_is_initial_device(void);

EXTERN int omp_get_initial_device(void);

EXTERN void kmp_global_barrier_init(void);

EXTERN void kmp_global_barrier(void);

///
/// Device runtime initialization
///
/// Some of the global states
#endif // INTEL_COLLAB
