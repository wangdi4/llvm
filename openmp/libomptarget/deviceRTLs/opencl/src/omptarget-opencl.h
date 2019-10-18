#if INTEL_COLLAB
//===--- omptarget-opencl.h - header for the OpenCL device RTL ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains declaration/definition of types, functions, and macros
//
//===----------------------------------------------------------------------===//

#ifndef OMPTARGET_OPENCL_H
#define OMPTARGET_OPENCL_H

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

#define KMP_PAUSE() // We don't have any candidates for this.


///
/// Device information
///

#ifndef KMP_DEVICE_GEN9_GT4
#define KMP_DEVICE_GEN9_GT4
#endif

/// Constants to be used when allocating internal data structures.
/// TODO: numbers may be too conservative (or large).
#ifdef KMP_DEVICE_GEN9_GT2 // 3 subslices
#define KMP_MAX_GROUP_SIZE 256
#define KMP_MAX_NUM_GROUPS (24 * 7)
#define KMP_MAX_LOG_NUM_GROUPS 16
#define KMP_MAX_GLOBAL_SIZE KMP_MAX_GROUP_SIZE * KMP_MAX_NUM_GROUPS
#define KMP_MAX_DATA_SIZE 16 // quad
#define KMP_ATOMIC_FIXED4_SUPPORTED 1
#define KMP_ATOMIC_FIXED8_SUPPORTED 1
#define KMP_USE_BARRIER_COUNTING

#elif defined(KMP_DEVICE_GEN9_GT3) // 6 subslices
#define KMP_MAX_GROUP_SIZE 256
#define KMP_MAX_NUM_GROUPS (48 * 7)
#define KMP_MAX_LOG_NUM_GROUPS 16
#define KMP_MAX_GLOBAL_SIZE KMP_MAX_GROUP_SIZE * KMP_MAX_NUM_GROUPS
#define KMP_MAX_DATA_SIZE 16 // quad
#define KMP_ATOMIC_FIXED4_SUPPORTED 1
#define KMP_ATOMIC_FIXED8_SUPPORTED 1
#define KMP_USE_BARRIER_COUNTING

#elif defined(KMP_DEVICE_GEN9_GT4) // 9 subslices
#define KMP_MAX_GROUP_SIZE 256
#define KMP_MAX_NUM_GROUPS (72 * 7)
#define KMP_MAX_LOG_NUM_GROUPS 16
#define KMP_MAX_GLOBAL_SIZE KMP_MAX_GROUP_SIZE * KMP_MAX_NUM_GROUPS
#define KMP_MAX_DATA_SIZE 16 // quad
#define KMP_ATOMIC_FIXED4_SUPPORTED 1
#define KMP_ATOMIC_FIXED8_SUPPORTED 1
#define KMP_USE_BARRIER_COUNTING

#elif defined(KMP_DEVICE_GEN12)
// TODO
#else
#error Unsupported device!!!
#endif

/// Static parameters used in RTL
#define KMP_ACTIVE_PARALLEL_BUMP 128  // used for tracking active level
#define KMP_MAX_PARALLEL_LEVEL 8      // used for task object allocation
#define KMP_MAX_SHAREDS 64            // used for data sharing

/// Enable extensions if available
#if KMP_ATOMIC_FIXED8_SUPPORTED
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
#endif

/// OP definitions for atomic/reduction entries
#define OP_MIN(X, Y, DT) ((X) < (Y) ? (X) : (Y))
#define OP_MAX(X, Y, DT) ((X) > (Y) ? (X) : (Y))
#define TO_LOGIC(X, DT) ((X) != (DT)0 ? 1 : 0)
#define OP_OR(X, Y, DT) ((DT)(TO_LOGIC(X, DT) || TO_LOGIC(Y, DT)))
#define OP_AND(X, Y, DT) ((DT)(TO_LOGIC(X, DT) && TO_LOGIC(Y, DT)))
#define OP_ADD(X, Y, DT) ((X) + (Y))
#define OP_SUB(X, Y, DT) ((X) - (Y))
#define OP_MUL(X, Y, DT) ((X) * (Y))
#define OP_DIV(X, Y, DT) ((X) / (Y))
#define OP_ORB(X, Y, DT) ((X) | (Y))
#define OP_ANDB(X, Y, DT) ((X) & (Y))
#define OP_XOR(X, Y, DT) ((X) ^ (Y))
#define OP_SHL(X, Y, DT) ((X) << (Y))
#define OP_SHR(X, Y, DT) ((X) >> (Y))


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

/// Schedule type
typedef enum kmp_sched {
  kmp_sched_static_chunk = 33,
  kmp_sched_static_nochunk = 34,
  kmp_sched_dynamic = 35,
  kmp_sched_guided = 36,
  kmp_sched_runtime = 37,
  kmp_sched_auto = 38,

  kmp_sched_static_balanced_chunk = 45,

  kmp_sched_static_ordered = 65,
  kmp_sched_static_nochunk_ordered = 66,
  kmp_sched_dynamic_ordered = 67,
  kmp_sched_guided_ordered = 68,
  kmp_sched_runtime_ordered = 69,
  kmp_sched_auto_ordered = 70,

  kmp_sched_distr_static_chunk = 91,
  kmp_sched_distr_static_nochunk = 92,
  kmp_sched_distr_static_chunk_sched_static_chunkone = 93,

  kmp_sched_default = kmp_sched_static_nochunk,
  kmp_sched_unorderd_first = kmp_sched_static_chunk,
  kmp_sched_unordered_last = kmp_sched_auto,
  kmp_sched_ordered_first = kmp_sched_static_ordered,
  kmp_sched_ordered_last = kmp_sched_auto_ordered,
  kmp_sched_distribute_first = kmp_sched_distr_static_chunk,
  kmp_sched_distribute_last =
      kmp_sched_distr_static_chunk_sched_static_chunkone,

  kmp_sched_modifier_monotonic = (1 << 29),
  kmp_sched_modifier_nonmonotonic = (1 << 30),

#define SCHEDULE_WITHOUT_MODIFIERS(s)                                          \
  (enum kmp_sched)(                                                            \
      (s) & ~(kmp_sched_modifier_nonmonotonic | kmp_sched_modifier_monotonic))
} kmp_sched_t;

typedef enum omp_sched_t {
  omp_sched_static = 0x1,
  omp_sched_dynamic = 0x2,
  omp_sched_guided = 0x3,
  omp_sched_auto = 0x4,
} omp_sched_t;

///
/// Task state
///

enum {
  TASK_SCHED_MASK = (0x1 | 0x2 | 0x3), // for runtime schedule
  TASK_IN_PARALLEL = 0x10, // has encountered at least one parallel region
  TASK_IMPLICIT = 0x20, // is an implicit task in a parallel region
  TASK_IN_DEEP_PARALLEL = 0x40, // has encountered at least two parallel region
};

typedef struct kmp_task_state {
  struct {
    long loop_ub;
    long next_lb;
    long chunk;
    long stride;
    kmp_sched_t schedule;
  } loop_data;
  struct {
    uchar flags;
    uchar unused;
    ushort num_threads;
    ushort thread_limit;
    ushort thread_id;
    ushort num_team_threads;
    ulong runtime_chunk_size;
  } task_data;
  struct kmp_task_state *prev;
} kmp_task_state_t;

/// Team state
typedef struct kmp_team_state {
  kmp_task_state_t level_zero_task;
  kmp_task_state_t work_parallel; // work for active parallel
} kmp_team_state_t;

/// Thread states
typedef struct kmp_thread_state {
  kmp_team_state_t team;
  kmp_task_state_t level_one_task[KMP_MAX_GROUP_SIZE];
  // TODO: We do not have dynamic memory allocation, so we need to set limit on
  // maximum parallel level to allocate task data for serialized region.
  kmp_task_state_t serialized_task[KMP_MAX_PARALLEL_LEVEL][KMP_MAX_GROUP_SIZE];
  // TODO: llvm-spirv -r crashes when this is declared as kmp_task_state_t*
  void *top_level_task[KMP_MAX_GROUP_SIZE];
  union {
    ushort num_threads[KMP_MAX_GROUP_SIZE];
    ushort simd_limit[KMP_MAX_GROUP_SIZE];
  } next_region;
  kmp_sched_t schedule[KMP_MAX_GROUP_SIZE];
  long chunk[KMP_MAX_GROUP_SIZE];
  long loop_ub[KMP_MAX_GROUP_SIZE];
  long next_lb[KMP_MAX_GROUP_SIZE];
  long stride[KMP_MAX_GROUP_SIZE];
  long count;
} kmp_thread_state_t;

/// For outlined parallel regions
typedef void *kmp_work_fn_t;

/// Data-sharing support. The list cannot grow dynamically.
typedef struct kmp_shared_data {
  void *shareds[KMP_MAX_SHAREDS];
  uint num_shareds;
} kmp_shared_data_t;

/// Local state
/// We need to use an array of local states for each work group
typedef struct kmp_local_state {
  uchar parallel_level[KMP_MAX_GROUP_SIZE];
  kmp_thread_state_t *thread_state;
  kmp_work_fn_t work_fn;
  uint execution_flags;
  ushort thread_limit;
  ushort team_threads;
  ushort num_threads;
  kmp_shared_data_t shared_data;
  kmp_barrier_counting_t work_barrier;
} kmp_local_state_t;

/// Global state
typedef struct kmp_global_state {
  kmp_barrier_t g_barrier;              // global barrier
  int assume_simple_spmd_mode;     // assume simple SPMD mode
} kmp_global_state_t;


///
/// Types from host runtime
///

typedef int kmp_critical_name[8];

typedef struct ident {
  char bytes[24];
} ident_t;


///
/// Program-scope global data
///

/// Global state
EXTERN kmp_global_state_t GLOBAL;

/// Per-team state
EXTERN kmp_local_state_t LOCALS[KMP_MAX_NUM_GROUPS];

/// Per-thread state for all work groups
EXTERN kmp_thread_state_t THREADS[KMP_MAX_NUM_GROUPS];

///
/// RTL init/fini routines
///

/// Initialize information maintained by RTL
EXTERN void __kmpc_kernel_init_params(void *params);

/// Initialize a kernel execution
EXTERN void __kmpc_kernel_init(int thread_limit, short needs_rtl);

/// Finalize a kernel execution
EXTERN void __kmpc_kernel_fini(short is_rtl_initialized);

/// Initialize a SPMD kernel execution
EXTERN void __kmpc_spmd_kernel_init(int thread_limit, short needs_rtl,
                                    short needs_data_sharing);

/// Finalize a SPMD kernel execution
EXTERN void __kmpc_spmd_kernel_fini(short needs_rtl);

/// Return if we are in SPMD execution mode
EXTERN char __kmpc_is_spmd_exec_mode(void);


///
/// Parallel regions
///

/// Prepare a parallel region -- called by master
EXTERN void __kmpc_kernel_prepare_parallel(void *work_fn,
                                           short is_rtl_initialized);

/// Initialize a parallel region -- called by workers
EXTERN bool __kmpc_kernel_parallel(void **work_fn, short is_rtl_initialized);

/// Finalize a parallel region -- called by workers
EXTERN void __kmpc_kernel_end_parallel(void);

/// Initialize a serialized parallel region
EXTERN void __kmpc_serialized_parallel(ident_t *loc, uint tid);

/// Finalize a serialized parallel region
EXTERN void __kmpc_end_serialized_parallel(ident_t *loc, uint tid);

/// Return the current parallel level (counting active & inactive regions)
EXTERN short __kmpc_parallel_level(ident_t *loc, uint gtid);

/// Push num_threads for the next parallel region
EXTERN void __kmpc_push_num_threads(ident_t *loc, int tid, int num_threads);

/// Push simd_limit for the next region
EXTERN void __kmpc_push_simd_limit(ident_t *loc, int tid, int simd_limit);

/// Init sharing variables
EXTERN void __kmpc_init_sharing_variables(void);

/// Begin sharing variables
EXTERN void __kmpc_begin_sharing_variables(void ***shareds, size_t num_shareds);

/// End sharing variables
EXTERN void __kmpc_end_sharing_variables(void);

/// Return the list of shared variables
EXTERN void __kmpc_get_shared_variables(void ***shareds);

///
/// Loop scheduling
///

/// Static init with 4-byte signed loop bounds
EXTERN void __kmpc_for_static_init_4(ident_t *loc, int tid, int schedtype,
    int *plastiter, int *plower, int *puppper, int *pstride, int incr,
    int chunk);

/// Static init with 4-byte unsigned loop bounds
EXTERN void __kmpc_for_static_init_4u(ident_t *loc, int tid, int schedtype,
    int *plastiter, uint *plower, uint *puppper, int *pstride, int incr,
    int chunk);

/// Static init with 8-byte signed loop bounds
EXTERN void __kmpc_for_static_init_8(ident_t *loc, int tid, int schedtype,
    int *plastiter, long *plower, long *puppper, long *pstride, long incr,
    long chunk);

/// Static init with 8-byte unsigned loop bounds
EXTERN void __kmpc_for_static_init_8u(ident_t *loc, int tid, int schedtype,
    int *plastiter, ulong *plower, ulong *puppper, long *pstride, long incr,
    long chunk);

/// Static init with 4-byte signed loop bounds in SPMD mode
EXTERN void __kmpc_for_static_init_4_spmd(ident_t *loc, int tid, int schedtype,
    int *plastiter, int *plower, int *puppper, int *pstride, int incr,
    int chunk);

/// Static init with 4-byte unsigned loop bounds in SPMD mode
EXTERN void __kmpc_for_static_init_4u_spmd(ident_t *loc, int tid, int schedtype,
    int *plastiter, uint *plower, uint *puppper, int *pstride, int incr,
    int chunk);

/// Static init with 8-byte signed loop bounds in SPMD mode
EXTERN void __kmpc_for_static_init_8_spmd(ident_t *loc, int tid, int schedtype,
    int *plastiter, long *plower, long *puppper, long *pstride, long incr,
    long chunk);

/// Static init with 8-byte unsigned loop bounds in SPMD mode
EXTERN void __kmpc_for_static_init_8u_spmd(ident_t *loc, int tid, int schedtype,
    int *plastiter, ulong *plower, ulong *puppper, long *pstride, long incr,
    long chunk);

/// Static init with 4-byte signed loop bounds in generic mode
EXTERN void __kmpc_for_static_init_4_generic(ident_t *loc, int tid,
    int schedtype, int *plastiter, int *plower, int *puppper, int *pstride,
    int incr, int chunk);

/// Static init with 4-byte unsigned loop bounds in generic mode
EXTERN void __kmpc_for_static_init_4u_generic(ident_t *loc, int tid,
    int schedtype, int *plastiter, uint *plower, uint *puppper, int *pstride,
    int incr, int chunk);

/// Static init with 8-byte signed loop bounds in generic mode
EXTERN void __kmpc_for_static_init_8_generic(ident_t *loc, int tid,
    int schedtype, int *plastiter, long *plower, long *puppper, long *pstride,
    long incr, long chunk);

/// Static init with 8-byte unsigned loop bounds in generic mode
EXTERN void __kmpc_for_static_init_8u_generic(ident_t *loc, int tid,
    int schedtype, int *plastiter, ulong *plower, ulong *puppper, long *pstride,
    long incr, long chunk);

/// Finalize static scheduling
EXTERN void __kmpc_for_static_fini(ident_t *loc, int tid);


///
/// Barriers
///

/// Barrier for active workers
EXTERN void __kmpc_work_barrier();

/// Barrier for entire work group
EXTERN void __kmpc_barrier();

/// Runtime initializer
EXTERN void __kmpc_init_runtime();


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
/// Support for critical section
///

EXTERN void __kmpc_critical(kmp_critical_name *);
EXTERN void __kmpc_end_critical(kmp_critical_name *);


///
/// Other __kmpc_* entries
///

EXTERN int __kmpc_master();
EXTERN void __kmpc_end_master();

/// Check if the current work item belongs to the master sub group
EXTERN int __kmpc_master_sub_group();

/// Check if the current work item is the leader of the master sub group
EXTERN int __kmpc_master_sub_group_leader();


///
/// Support for reduction
///

EXTERN void __kmpc_reduction_add_int(const uint id, const uint size,
                                     void *local_result, void *output);
EXTERN void __kmpc_reduction_add_long(const uint id, const uint size,
                                      void *local_result, void *output);
EXTERN void __kmpc_reduction_add_float(const uint id, const uint size,
                                       void *local_result, void *output);
EXTERN void __kmpc_reduction_add_double(const uint id, const uint size,
                                        void *local_result, void *output);


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
#endif // OMPTARGET_OPENCL_H
#endif // INTEL_COLLAB
