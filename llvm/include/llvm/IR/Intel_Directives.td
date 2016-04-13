//==- Intel_Directives.td - Defines all parallelization and -*- tablegen -*-==//
//                          vectorization directives and clauses.
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the classes used to represent parallelization and 
/// vectorization directives and clauses. Each different type of directive and
/// clause is then instantiated accordingly. TableGen parses these records and
/// generates enums for both classes, as well as tables which provide an
/// enum->string and string->enum mapping. The enums and tables are used to
/// generate Metadata strings that are used as arguments to llvm.directive
/// intrinsics. These intrinsics are used to mark parallel/vector constructs
/// and behavior.
///
// ===--------------------------------------------------------------------=== //

// The Directive class represents all parallel/vector constructs that can be
// represented within LLVM IR. For example, the "#pragma omp parallel"
// directive for OpenMP.
class Directive;

// The Clause class represents additional information provided to Directives
// that can be used to control parallel/vector execution. For example,
// "#pragma omp parallel" could be modified to include the "if (expr)" clause
// to determine if the region should be executed in parallel or not.
class Clause;

// For additional information on the supported Directives and Clauses, please
// refer to the following documentation. Currently, this file defines only
// the directives and clauses associated with OpenMP, but will be extended in
// the future for other parallel/vector programming models and APIs.
//
// http://openmp.org/wp/openmp-specifications

// *** Begin OpenMP Directives ***

def DIR_OMP_PARALLEL : Directive;
def DIR_OMP_END_PARALLEL : Directive;

def DIR_OMP_PARALLEL_LOOP : Directive;
def DIR_OMP_END_PARALLEL_LOOP : Directive;

def DIR_OMP_LOOP_SIMD : Directive;
def DIR_OMP_END_LOOP_SIMD : Directive;

def DIR_OMP_PARALLEL_LOOP_SIMD : Directive;
def DIR_OMP_END_PARALLEL_LOOP_SIMD : Directive;

def DIR_OMP_SECTIONS : Directive;
def DIR_OMP_END_SECTIONS : Directive;

def DIR_OMP_PARALLEL_SECTIONS : Directive;
def DIR_OMP_END_PARALLEL_SECTIONS : Directive;

def DIR_OMP_WORKSHARE : Directive;
def DIR_OMP_END_WORKSHARE : Directive;

def DIR_OMP_PARALLEL_WORKSHARE : Directive;
def DIR_OMP_END_PARALLEL_WORKSHARE : Directive;

def DIR_OMP_SECTION : Directive;
def DIR_OMP_END_SECTION : Directive;

def DIR_OMP_SINGLE : Directive;
def DIR_OMP_END_SINGLE : Directive;

def DIR_OMP_TASK : Directive;
def DIR_OMP_END_TASK : Directive;

def DIR_OMP_MASTER : Directive;
def DIR_OMP_END_MASTER : Directive;

def DIR_OMP_CRITICAL : Directive;
def DIR_OMP_END_CRITICAL : Directive;

def DIR_OMP_BARRIER : Directive;

def DIR_OMP_TASKWAIT : Directive;

def DIR_OMP_TASKYIELD : Directive;

def DIR_OMP_ATOMIC : Directive;
def DIR_OMP_END_ATOMIC : Directive;

def DIR_OMP_FLUSH : Directive;

// def DIR_OMP_THREADPRIVATE : Directive;

def DIR_OMP_ORDERED : Directive;
def DIR_OMP_END_ORDERED : Directive;

def DIR_OMP_SIMD : Directive;
def DIR_OMP_END_SIMD : Directive;

def DIR_OMP_TASKLOOP : Directive;
def DIR_OMP_END_TASKLOOP : Directive;

def DIR_OMP_TASKLOOP_SIMD : Directive;
def DIR_OMP_END_TASKLOOP_SIMD : Directive;

def DIR_OMP_TARGET : Directive;
def DIR_OMP_END_TARGET : Directive;

def DIR_OMP_TARGET_DATA : Directive;
def DIR_OMP_END_TARGET_DATA : Directive;

def DIR_OMP_TARGET_UPDATE : Directive;
def DIR_OMP_END_TARGET_UPDATE : Directive;

def DIR_OMP_TEAMS : Directive;
def DIR_OMP_END_TEAMS : Directive;

def DIR_OMP_TEAMS_DISTRIBUTE : Directive;
def DIR_OMP_END_TEAMS_DISTRIBUTE : Directive;

def DIR_OMP_TEAMS_SIMD : Directive;
def DIR_OMP_END_TEAMS_SIMD : Directive;

def DIR_OMP_TEAMS_DISTRIBUTE_SIMD : Directive;
def DIR_OMP_END_TEAMS_DISTRIBUTE_SIMD : Directive;

def DIR_OMP_DISTRIBUTE : Directive;
def DIR_OMP_END_DISTRIBUTE : Directive;

def DIR_OMP_DISTRIBUTE_PARLOOP : Directive;
def DIR_OMP_END_DISTRIBUTE_PARLOOP : Directive;

def DIR_OMP_DISTRIBUTE_SIMD : Directive;
def DIR_OMP_END_DISTRIBUTE_SIMD : Directive;

def DIR_OMP_DISTRIBUTE_PARLOOP_SIMD : Directive;
def DIR_OMP_END_DISTRIBUTE_PARLOOP_SIMD : Directive;

def DIR_OMP_TARGET_ENTER_DATA : Directive;
def DIR_OMP_TARGET_EXIT_DATA : Directive;

def DIR_OMP_TARGET_TEAMS : Directive;
def DIR_OMP_END_TARGET_TEAMS : Directive;

def DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP : Directive;
def DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP : Directive;

def DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP_SIMD : Directive;
def DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP_SIMD : Directive;

def DIR_OMP_TARGET_TEAMS_DISTRIBUTE : Directive;
def DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE : Directive;

def DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP : Directive;
def DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP : Directive;

def DIR_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD : Directive;
def DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_SIMD : Directive;

def DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD : Directive;
def DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD : Directive;

def DIR_OMP_CANCEL : Directive;
def DIR_OMP_CANCELLATION_POINT : Directive;

// *** Begin OpenMP Clauses ***

def QUAL_OMP_DEFAULT_NONE : Clause;
def QUAL_OMP_DEFAULT_SHARED : Clause;
def QUAL_OMP_DEFAULT_PRIVATE : Clause;
def QUAL_OMP_DEFAULT_FIRSTPRIVATE : Clause;
def QUAL_OMP_MERGEABLE : Clause;
def QUAL_OMP_NOWAIT : Clause;
def QUAL_OMP_NOGROUP : Clause;
def QUAL_OMP_UNTIED : Clause;
def QUAL_OMP_READ : Clause;
def QUAL_OMP_READ_SEQ_CST : Clause;
def QUAL_OMP_WRITE : Clause;
def QUAL_OMP_WRITE_SEQ_CST : Clause;
def QUAL_OMP_UPDATE : Clause;
def QUAL_OMP_UPDATE_SEQ_CST : Clause;
def QUAL_OMP_CAPTURE : Clause;
def QUAL_OMP_CAPTURE_SEQ_CST : Clause;
def QUAL_OMP_SCHEDULE_AUTO : Clause;
def QUAL_OMP_SCHEDULE_RUNTIME : Clause;
def QUAL_OMP_PROC_BIND_MASTER : Clause;
def QUAL_OMP_PROC_BIND_CLOSE : Clause;
def QUAL_OMP_PROC_BIND_SPREAD : Clause;
def QUAL_OMP_IF : Clause;
def QUAL_OMP_COLLAPSE : Clause;
def QUAL_OMP_NUM_THREADS : Clause;
def QUAL_OMP_ORDERED : Clause;
def QUAL_OMP_SAFELEN : Clause;
def QUAL_OMP_SIMDLEN : Clause;
def QUAL_OMP_FINAL : Clause;
def QUAL_OMP_GRAINSIZE : Clause;
def QUAL_OMP_NUM_TASKS : Clause;
def QUAL_OMP_PRIORITY : Clause;
def QUAL_OMP_NUM_TEAMS : Clause;
def QUAL_OMP_THREAD_LIMIT : Clause;
def QUAL_OMP_DIST_SCHEDULE_STATIC : Clause;
def QUAL_OMP_SCHEDULE_STATIC : Clause;
def QUAL_OMP_SCHEDULE_DYNAMIC : Clause;
def QUAL_OMP_SCHEDULE_GUIDED : Clause;
def QUAL_OMP_SHARED : Clause;
def QUAL_OMP_PRIVATE : Clause;
def QUAL_OMP_FIRSTPRIVATE : Clause;
def QUAL_OMP_LASTPRIVATE : Clause;
def QUAL_OMP_COPYIN : Clause;
def QUAL_OMP_COPYPRIVATE : Clause;
def QUAL_OMP_REDUCTION : Clause;
def QUAL_OMP_REDUCTION_ADD : Clause;
def QUAL_OMP_REDUCTION_SUB : Clause;
def QUAL_OMP_REDUCTION_MUL : Clause;
def QUAL_OMP_REDUCTION_AND : Clause;
def QUAL_OMP_REDUCTION_OR : Clause;
def QUAL_OMP_REDUCTION_XOR : Clause;
def QUAL_OMP_REDUCTION_BAND : Clause;
def QUAL_OMP_REDUCTION_BOR : Clause;
def QUAL_OMP_REDUCTION_UDR : Clause;
def QUAL_OMP_TO : Clause;
def QUAL_OMP_FROM : Clause;
def QUAL_OMP_LINEAR : Clause;
def QUAL_OMP_UNIFORM : Clause;
def QUAL_OMP_ALIGNED : Clause;
def QUAL_OMP_FLUSH : Clause;
def QUAL_OMP_THREADPRIVATE : Clause;
def QUAL_OMP_MAP_TO : Clause;
def QUAL_OMP_MAP_FROM : Clause;
def QUAL_OMP_MAP_TOFROM : Clause;
def QUAL_OMP_DEPEND_IN : Clause;
def QUAL_OMP_DEPEND_OUT : Clause;
def QUAL_OMP_DEPEND_INOUT : Clause;
def QUAL_OMP_NAME : Clause;
