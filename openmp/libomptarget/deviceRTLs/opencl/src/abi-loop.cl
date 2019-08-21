#if INTEL_COLLAB
//===--- abi-loop.cl - Entry points for loop scheduling -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains entry points for loop scheduling.
//
//===----------------------------------------------------------------------===//

#include "omptarget-opencl.h"
#include "internal.h"

///
/// For static scheduling
///

#define FOR_STATIC_CHUNK(DT, PtrLast, PtrLb, PtrUb, PtrStride, Chunk, Id, Num) \
  do {                                                                         \
    *(PtrStride) = (Num) * (Chunk);                                            \
    *(PtrLb) = *(PtrLb) + (Id) * (Chunk);                                      \
    DT oldub = *(PtrUb);                                                       \
    *(PtrUb) = *(PtrLb) + (Chunk) - 1;                                         \
    DT lastchunk = oldub - (oldub % (Chunk));                                  \
    *(PtrLast) = ((lastchunk - *(PtrLb)) % *(PtrStride)) == 0;                 \
  } while (0)

#define FOR_STATIC_NO_CHUNK(DT, PtrLast, PtrLb, PtrUb, PtrStride, PtrChunk,    \
                            Id, Num)                                           \
  do {                                                                         \
    DT LoopSize = *(PtrUb) - *(PtrLb) + 1;                                     \
    *(PtrChunk) = LoopSize / Num;                                              \
    DT leftover = LoopSize - *(PtrChunk) * Num;                                \
    if (Id < leftover) {                                                       \
      (*(PtrChunk))++;                                                         \
      *(PtrLb) = *(PtrLb) + Id * (*(PtrChunk));                                \
    } else {                                                                   \
      *(PtrLb) = *(PtrLb) + Id * (*(PtrChunk)) + leftover;                     \
    }                                                                          \
    DT oldub = *(PtrUb);                                                       \
    *(PtrUb) = *(PtrLb) + *(PtrChunk) - 1;                                     \
    *(PtrLast) = *(PtrLb) <= oldub && oldub <= *(PtrUb);                       \
    *(PtrStride) = LoopSize;                                                   \
  } while (0)

#define FOR_STATIC_INIT(DT, ST, Id, SchedType, PtrLastIter, PtrLower,          \
                        PtrUpper, PtrStride, Chunk, IsSPMDMode)                \
  do {                                                                         \
    ST num_omp_threads = __kmp_get_num_omp_threads(IsSPMDMode);                \
    KMP_ASSERT(Id < num_omp_threads,                                           \
               "Invalid thread ID in static loop scheduling");                 \
    int lastiter = 0;                                                          \
    DT lb = *(PtrLower);                                                       \
    DT ub = *(PtrUpper);                                                       \
    ST stride = *(PtrStride);                                                  \
                                                                               \
    switch (SCHEDULE_WITHOUT_MODIFIERS(SchedType)) {                           \
    case kmp_sched_static_chunk: {                                             \
      if (Chunk > 0) {                                                         \
        FOR_STATIC_CHUNK(DT, &lastiter, &lb, &ub, &stride, Chunk, Id,          \
                         num_omp_threads);                                     \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    case kmp_sched_static_balanced_chunk: {                                    \
      if (Chunk > 0) {                                                         \
        DT tripcount = ub - lb + 1;                                            \
        DT span = (tripcount + num_omp_threads - 1) / num_omp_threads;         \
        Chunk = (span + Chunk - 1) & ~(Chunk - 1);                             \
        KMP_ASSERT(ub >= lb,                                                   \
                   "Invalid loop bounds in static loop scheduling");           \
        DT old_ub = ub;                                                        \
        FOR_STATIC_CHUNK(DT, &lastiter, &lb, &ub, &stride, Chunk, Id,          \
                         num_omp_threads);                                     \
        if (ub > old_ub)                                                       \
          ub = old_ub;                                                         \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    case kmp_sched_static_nochunk: {                                           \
      FOR_STATIC_NO_CHUNK(DT, &lastiter, &lb, &ub, &stride, &Chunk, Id,        \
                          num_omp_threads);                                    \
      break;                                                                   \
    }                                                                          \
    case kmp_sched_distr_static_chunk: {                                       \
      if (Chunk > 0) {                                                         \
        FOR_STATIC_CHUNK(DT, &lastiter, &lb, &ub, &stride, Chunk,              \
                         __kmp_get_group_id(), __kmp_get_num_groups());        \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    case kmp_sched_distr_static_nochunk: {                                     \
      FOR_STATIC_NO_CHUNK(DT, &lastiter, &lb, &ub, &stride, &Chunk,            \
                          __kmp_get_group_id(), __kmp_get_num_groups());       \
      break;                                                                   \
    }                                                                          \
    case kmp_sched_distr_static_chunk_sched_static_chunkone: {                 \
      FOR_STATIC_CHUNK(DT, &lastiter, &lb, &ub, &stride, Chunk,                \
                       num_omp_threads * __kmp_get_group_id() + Id,            \
                       __kmp_get_num_groups() * num_omp_threads);              \
      break;                                                                   \
    }                                                                          \
    default : {                                                                \
      /* Default to static chunk */                                            \
      FOR_STATIC_CHUNK(DT, &lastiter, &lb, &ub, &stride, Chunk, Id,            \
                       num_omp_threads);                                       \
      break;                                                                   \
    }                                                                          \
    }                                                                          \
    *(PtrLastIter) = lastiter;                                                 \
    *(PtrLower) = lb;                                                          \
    *(PtrUpper) = ub;                                                          \
    *(PtrStride) = stride;                                                     \
  } while (0)

EXTERN void __kmpc_for_static_init_4(ident_t *loc, int gtid, int schedtype,
                                     int *plastiter, int *plower, int *pupper,
                                     int *pstride, int incr, int chunk) {
  FOR_STATIC_INIT(int, int, gtid, schedtype, plastiter, plower, pupper, pstride,
                  chunk, __kmp_check_spmd_mode(loc));
}

EXTERN void __kmpc_for_static_init_4u(ident_t *loc, int gtid, int schedtype,
                                      int *plastiter, uint *plower,
                                      uint *pupper, int *pstride, int incr,
                                      int chunk) {
  FOR_STATIC_INIT(uint, int, gtid, schedtype, plastiter, plower, pupper,
                  pstride, chunk, __kmp_check_spmd_mode(loc));

}

EXTERN void __kmpc_for_static_init_8(ident_t *loc, int gtid, int schedtype,
                                     int *plastiter, long *plower, long *pupper,
                                     long *pstride, long incr, long chunk) {
  FOR_STATIC_INIT(long, long, gtid, schedtype, plastiter, plower, pupper,
                  pstride, chunk, __kmp_check_spmd_mode(loc));
}

EXTERN void __kmpc_for_static_init_8u(ident_t *loc, int gtid, int schedtype,
                                      int *plastiter, ulong *plower,
                                      ulong *pupper, long *pstride, long incr,
                                      long chunk) {
  FOR_STATIC_INIT(ulong, long, gtid, schedtype, plastiter, plower, pupper,
                  pstride, chunk, __kmp_check_spmd_mode(loc));
}

EXTERN void __kmpc_for_static_init_4_spmd(ident_t *loc, int gtid, int schedtype,
                                          int *plastiter, int *plower,
                                          int *pupper, int *pstride, int incr,
                                          int chunk) {
  FOR_STATIC_INIT(int, int, gtid, schedtype, plastiter, plower, pupper, pstride,
                  chunk, true);
}

EXTERN void __kmpc_for_static_init_4u_spmd(ident_t *loc, int gtid,
                                           int schedtype, int *plastiter,
                                           uint *plower, uint *pupper,
                                           int *pstride, int incr, int chunk) {
  FOR_STATIC_INIT(uint, int, gtid, schedtype, plastiter, plower, pupper,
                  pstride, chunk, true);
}

EXTERN void __kmpc_for_static_init_8_spmd(ident_t *loc, int gtid, int schedtype,
                                          int *plastiter, long *plower,
                                          long *pupper, long *pstride,
                                          long incr, long chunk) {
  FOR_STATIC_INIT(long, long, gtid, schedtype, plastiter, plower, pupper,
                  pstride, chunk, true);
}

EXTERN void __kmpc_for_static_init_8u_spmd(ident_t *loc, int gtid,
                                           int schedtype, int *plastiter,
                                           ulong *plower, ulong *pupper,
                                           long *pstride, long incr,
                                           long chunk) {
  FOR_STATIC_INIT(ulong, long, gtid, schedtype, plastiter, plower, pupper,
                  pstride, chunk, true);
}

EXTERN void __kmpc_for_static_init_4_generic(ident_t *loc, int gtid,
                                             int schedtype, int *plastiter,
                                             int *plower, int *pupper,
                                             int *pstride, int incr,
                                             int chunk) {
  FOR_STATIC_INIT(int, int, gtid, schedtype, plastiter, plower, pupper, pstride,
                  chunk, false);
}

EXTERN void __kmpc_for_static_init_4u_generic(ident_t *loc, int gtid,
                                              int schedtype, int *plastiter,
                                              uint *plower, uint *pupper,
                                              int *pstride, int incr,
                                              int chunk) {
  FOR_STATIC_INIT(uint, int, gtid, schedtype, plastiter, plower, pupper,
                  pstride, chunk, false);
}

EXTERN void __kmpc_for_static_init_8_generic(ident_t *loc, int gtid,
                                             int schedtype, int *plastiter,
                                             long *plower, long *pupper,
                                             long *pstride, long incr,
                                             long chunk) {
  FOR_STATIC_INIT(long, long, gtid, schedtype, plastiter, plower, pupper,
                  pstride, chunk, false);
}

EXTERN void __kmpc_for_static_init_8u_generic(ident_t *loc, int gtid,
                                              int schedtype, int *plastiter,
                                              ulong *plower, ulong *pupper,
                                              long *pstride, long incr,
                                              long chunk) {
  FOR_STATIC_INIT(ulong, long, gtid, schedtype, plastiter, plower, pupper,
                  pstride, chunk, false);
}

EXTERN void __kmpc_for_static_fini(ident_t *loc, int gtid) {
  // nothing to be done
}

#endif // INTEL_COLLAB
