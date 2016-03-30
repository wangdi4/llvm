//===--- Intel_OpenMPDirectivesAndClauses.h - Class definition -*- C++ -*--===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// Contains the enumerations for OpenMP directives and clauses.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_UTILS_INTEL_OPENMPDIRECTIVESANDCLAUSES_H
#define LLVM_TRANSFORM_UTILS_INTEL_OPENMPDIRECTIVESANDCLAUSES_H

#include <unordered_map>

namespace llvm {

class StringRef;

typedef enum OMP_DIRECTIVES {
      DIR_OMP_PARALLEL = 0,
      DIR_OMP_END_PARALLEL,
      DIR_OMP_PARALLEL_LOOP,
      DIR_OMP_END_PARALLEL_LOOP,
      DIR_OMP_LOOP_SIMD,
      DIR_OMP_END_LOOP_SIMD,
      DIR_OMP_PARALLEL_LOOP_SIMD,
      DIR_OMP_END_PARALLEL_LOOP_SIMD,
      DIR_OMP_SECTIONS,
      DIR_OMP_END_SECTIONS,
      DIR_OMP_PARALLEL_SECTIONS,
      DIR_OMP_END_PARALLEL_SECTIONS,
      DIR_OMP_WORKSHARE,
      DIR_OMP_END_WORKSHARE,
      DIR_OMP_PARALLEL_WORKSHARE,
      DIR_OMP_END_PARALLEL_WORKSHARE,
      DIR_OMP_SECTION,
      DIR_OMP_END_SECTION,
      DIR_OMP_SINGLE,
      DIR_OMP_END_SINGLE,
      DIR_OMP_TASK,
      DIR_OMP_END_TASK,
      DIR_OMP_MASTER,
      DIR_OMP_END_MASTER,
      DIR_OMP_CRITICAL,
      DIR_OMP_END_CRITICAL,
      DIR_OMP_BARRIER,
      DIR_OMP_TASKWAIT,
      DIR_OMP_TASKYIELD,
      DIR_OMP_ATOMIC,
      DIR_OMP_END_ATOMIC,
      DIR_OMP_FLUSH,
      // DIR_OMP_THREADPRIVATE,  // we should need this too
      DIR_OMP_ORDERED,
      DIR_OMP_END_ORDERED,
      DIR_OMP_SIMD,
      DIR_OMP_END_SIMD,
      DIR_OMP_TASKLOOP,
      DIR_OMP_END_TASKLOOP,
      DIR_OMP_TASKLOOP_SIMD,
      DIR_OMP_END_TASKLOOP_SIMD,
      DIR_OMP_TARGET,
      DIR_OMP_END_TARGET,
      DIR_OMP_TARGET_DATA,
      DIR_OMP_END_TARGET_DATA,
      DIR_OMP_TARGET_UPDATE,
      DIR_OMP_END_TARGET_UPDATE,
      DIR_OMP_TEAMS,
      DIR_OMP_END_TEAMS,
      DIR_OMP_TEAMS_DISTRIBUTE,
      DIR_OMP_END_TEAMS_DISTRIBUTE,
      DIR_OMP_TEAMS_SIMD,
      DIR_OMP_END_TEAMS_SIMD,
      DIR_OMP_TEAMS_DISTRIBUTE_SIMD,
      DIR_OMP_END_TEAMS_DISTRIBUTE_SIMD,
      DIR_OMP_DISTRIBUTE,
      DIR_OMP_END_DISTRIBUTE,
      DIR_OMP_DISTRIBUTE_PARLOOP,
      DIR_OMP_END_DISTRIBUTE_PARLOOP,
      DIR_OMP_DISTRIBUTE_SIMD,
      DIR_OMP_END_DISTRIBUTE_SIMD,
      DIR_OMP_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_END_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_TARGET_ENTER_DATA,
      DIR_OMP_TARGET_EXIT_DATA,
      DIR_OMP_TARGET_TEAMS,
      DIR_OMP_END_TARGET_TEAMS,
      DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP,
      DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP,
      DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_TARGET_TEAMS_DISTRIBUTE,
      DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE,
      DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP,
      DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP,
      DIR_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD,
      DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_SIMD,
      DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
      DIR_OMP_CANCEL,
      DIR_OMP_CANCELLATION_POINT,
      DIR_QUAL_LIST_END              // must be last; marks end of DIR
} OMP_DIRECTIVES;

typedef enum OMP_CLAUSES {

  // Clauses with no argument (ie, "DirQual")

      QUAL_OMP_DEFAULT_NONE=0,
      QUAL_OMP_DEFAULT_SHARED,
      QUAL_OMP_DEFAULT_PRIVATE,
      QUAL_OMP_DEFAULT_FIRSTPRIVATE,
      QUAL_OMP_NOWAIT,
      QUAL_OMP_MERGEABLE,
      QUAL_OMP_NOGROUP,
      QUAL_OMP_UNTIED,
      QUAL_OMP_READ,
      QUAL_OMP_READ_SEQ_CST,
      QUAL_OMP_WRITE,
      QUAL_OMP_WRITE_SEQ_CST,
      QUAL_OMP_UPDATE,
      QUAL_OMP_UPDATE_SEQ_CST,
      QUAL_OMP_CAPTURE,
      QUAL_OMP_CAPTURE_SEQ_CST,
      QUAL_OMP_SCHEDULE_AUTO,
      QUAL_OMP_SCHEDULE_RUNTIME,
      QUAL_OMP_PROC_BIND_MASTER,
      QUAL_OMP_PROC_BIND_CLOSE,
      QUAL_OMP_PROC_BIND_SPREAD,

  // Clauses with one argument (ie, "DirQualOpnd")

      QUAL_OMP_IF,
      QUAL_OMP_COLLAPSE,
      QUAL_OMP_NUM_THREADS,
      QUAL_OMP_ORDERED,
      QUAL_OMP_SAFELEN,
      QUAL_OMP_SIMDLEN,
      QUAL_OMP_FINAL,
      QUAL_OMP_GRAINSIZE,
      QUAL_OMP_NUM_TASKS,
      QUAL_OMP_PRIORITY,
      QUAL_OMP_NUM_TEAMS,
      QUAL_OMP_THREAD_LIMIT,
      QUAL_OMP_DIST_SCHEDULE_STATIC,
      QUAL_OMP_SCHEDULE_STATIC,      // default chunk_size=0 (static even)
      QUAL_OMP_SCHEDULE_DYNAMIC,     // default chunk_size=1
      QUAL_OMP_SCHEDULE_GUIDED,      // default chunk_size=1

  // Clauses with list-type arguments (ie, "DirQualOpndList")

      QUAL_OMP_SHARED,
      QUAL_OMP_PRIVATE,
      QUAL_OMP_FIRSTPRIVATE,
      QUAL_OMP_LASTPRIVATE,
      QUAL_OMP_COPYIN,
      QUAL_OMP_COPYPRIVATE,
      QUAL_OMP_REDUCTION,    // must be first REDUCTION op in enum
      QUAL_OMP_REDUCTION_ADD,
      QUAL_OMP_REDUCTION_SUB,
      QUAL_OMP_REDUCTION_MUL,
      QUAL_OMP_REDUCTION_AND,
      QUAL_OMP_REDUCTION_OR,
      QUAL_OMP_REDUCTION_XOR,
      QUAL_OMP_REDUCTION_BAND,
      QUAL_OMP_REDUCTION_BOR,
      QUAL_OMP_REDUCTION_UDR,   // must be last REDUCTION op in enum
      QUAL_OMP_TO,
      QUAL_OMP_FROM,
      QUAL_OMP_LINEAR,
      QUAL_OMP_UNIFORM,         // not yet supported in OpenMP, but needed for
                                // vector function to simd loop transform.
      QUAL_OMP_ALIGNED,
      QUAL_OMP_FLUSH,
      QUAL_OMP_THREADPRIVATE,
      QUAL_OMP_MAP_TO,
      QUAL_OMP_MAP_FROM,
      QUAL_OMP_MAP_TOFROM,
      QUAL_OMP_DEPEND_IN,
      QUAL_OMP_DEPEND_OUT,
      QUAL_OMP_DEPEND_INOUT,
      QUAL_OMP_NAME,            // for critical section name (a string)
      QUAL_LIST_END             // must be last (dummy placeholder only)
} OMP_CLAUSES;

class IntelOpenMPDirectivesAndClauses {

public:
  // Map OMP_DIRECTIVES to StringRefs
  static std::unordered_map<int, StringRef> DirectiveStrings;

  // Map OMP_CLAUSES to StringRefs
  static std::unordered_map<int, StringRef> ClauseStrings;

};

} // end llvm namespace

#endif // LLVM_TRANSFORM_UTILS_INTEL_OPENMPDIRECTIVESANDCLAUSES_H
