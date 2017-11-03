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

def "DIR.OMP.PARALLEL" : Directive;
def "DIR.OMP.END.PARALLEL" : Directive;

def "DIR.OMP.LOOP" : Directive;
def "DIR.OMP.END.LOOP" : Directive;

def "DIR.OMP.PARALLEL.LOOP" : Directive;
def "DIR.OMP.END.PARALLEL.LOOP" : Directive;

def "DIR.OMP.SECTIONS" : Directive;
def "DIR.OMP.END.SECTIONS" : Directive;

def "DIR.OMP.PARALLEL.SECTIONS" : Directive;
def "DIR.OMP.END.PARALLEL.SECTIONS" : Directive;

def "DIR.OMP.WORKSHARE" : Directive;
def "DIR.OMP.END.WORKSHARE" : Directive;

def "DIR.OMP.PARALLEL.WORKSHARE" : Directive;
def "DIR.OMP.END.PARALLEL.WORKSHARE" : Directive;

def "DIR.OMP.SECTION" : Directive;
def "DIR.OMP.END.SECTION" : Directive;

def "DIR.OMP.SINGLE" : Directive;
def "DIR.OMP.END.SINGLE" : Directive;

def "DIR.OMP.TASK" : Directive;
def "DIR.OMP.END.TASK" : Directive;

def "DIR.OMP.MASTER" : Directive;
def "DIR.OMP.END.MASTER" : Directive;

def "DIR.OMP.CRITICAL" : Directive;
def "DIR.OMP.END.CRITICAL" : Directive;

def "DIR.OMP.BARRIER" : Directive;
def "DIR.OMP.END.BARRIER" : Directive;

def "DIR.OMP.TASKWAIT" : Directive;
def "DIR.OMP.END.TASKWAIT" : Directive;

def "DIR.OMP.TASKYIELD" : Directive;
def "DIR.OMP.END.TASKYIELD" : Directive;

def "DIR.OMP.ATOMIC" : Directive;
def "DIR.OMP.END.ATOMIC" : Directive;

def "DIR.OMP.FLUSH" : Directive;
def "DIR.OMP.END.FLUSH" : Directive;

def "DIR.OMP.THREADPRIVATE" : Directive;

def "DIR.OMP.ORDERED" : Directive;
def "DIR.OMP.END.ORDERED" : Directive;

def "DIR.OMP.SIMD" : Directive;
def "DIR.OMP.END.SIMD" : Directive;

def "DIR.OMP.TASKGROUP" : Directive;
def "DIR.OMP.END.TASKGROUP" : Directive;

def "DIR.OMP.TASKLOOP" : Directive;
def "DIR.OMP.END.TASKLOOP" : Directive;

def "DIR.OMP.TARGET" : Directive;
def "DIR.OMP.END.TARGET" : Directive;

def "DIR.OMP.TARGET.DATA" : Directive;
def "DIR.OMP.END.TARGET.DATA" : Directive;

def "DIR.OMP.TARGET.UPDATE" : Directive;
def "DIR.OMP.END.TARGET.UPDATE" : Directive;

def "DIR.OMP.TEAMS" : Directive;
def "DIR.OMP.END.TEAMS" : Directive;

def "DIR.OMP.DISTRIBUTE" : Directive;
def "DIR.OMP.END.DISTRIBUTE" : Directive;

def "DIR.OMP.DISTRIBUTE.PARLOOP" : Directive;
def "DIR.OMP.END.DISTRIBUTE.PARLOOP" : Directive;

def "DIR.OMP.TARGET.ENTER.DATA" : Directive;
def "DIR.OMP.END.TARGET.ENTER.DATA" : Directive;

def "DIR.OMP.TARGET.EXIT.DATA" : Directive;
def "DIR.OMP.END.TARGET.EXIT.DATA" : Directive;

def "DIR.OMP.CANCEL" : Directive;
def "DIR.OMP.END.CANCEL" : Directive;

def "DIR.OMP.CANCELLATION.POINT" : Directive;
def "DIR.OMP.END.CANCELLATION.POINT" : Directive;

// *** Begin OpenMP Clauses ***

def "QUAL.OMP.DEFAULT.NONE" : Clause;
def "QUAL.OMP.DEFAULT.SHARED" : Clause;
def "QUAL.OMP.DEFAULT.PRIVATE" : Clause;
def "QUAL.OMP.DEFAULT.FIRSTPRIVATE" : Clause;
def "QUAL.OMP.MERGEABLE" : Clause;
def "QUAL.OMP.NOWAIT" : Clause;
def "QUAL.OMP.NOGROUP" : Clause;
def "QUAL.OMP.UNTIED" : Clause;
def "QUAL.OMP.READ" : Clause;
def "QUAL.OMP.READ.SEQ_CST" : Clause;
def "QUAL.OMP.WRITE" : Clause;
def "QUAL.OMP.WRITE.SEQ_CST" : Clause;
def "QUAL.OMP.UPDATE" : Clause;
def "QUAL.OMP.UPDATE.SEQ_CST" : Clause;
def "QUAL.OMP.CAPTURE" : Clause;
def "QUAL.OMP.CAPTURE.SEQ_CST" : Clause;
def "QUAL.OMP.PROC.BIND.MASTER" : Clause;
def "QUAL.OMP.PROC.BIND.CLOSE" : Clause;
def "QUAL.OMP.PROC.BIND.SPREAD" : Clause;
def "QUAL.OMP.IF" : Clause;
def "QUAL.OMP.COLLAPSE" : Clause;
def "QUAL.OMP.NUM_THREADS" : Clause;
def "QUAL.OMP.ORDERED" : Clause;
def "QUAL.OMP.SAFELEN" : Clause;
def "QUAL.OMP.SIMDLEN" : Clause;
def "QUAL.OMP.FINAL" : Clause;
def "QUAL.OMP.GRAINSIZE" : Clause;
def "QUAL.OMP.NUM_TASKS" : Clause;
def "QUAL.OMP.PRIORITY" : Clause;
def "QUAL.OMP.NUM_TEAMS" : Clause;
def "QUAL.OMP.THREAD_LIMIT" : Clause;
def "QUAL.OMP.DIST_SCHEDULE.STATIC" : Clause;
def "QUAL.OMP.SCHEDULE.STATIC" : Clause;
def "QUAL.OMP.SCHEDULE.DYNAMIC" : Clause;
def "QUAL.OMP.SCHEDULE.GUIDED" : Clause;
def "QUAL.OMP.SCHEDULE.AUTO" : Clause;
def "QUAL.OMP.SCHEDULE.RUNTIME" : Clause;
def "QUAL.OMP.SHARED" : Clause;
def "QUAL.OMP.PRIVATE" : Clause;
def "QUAL.OMP.FIRSTPRIVATE" : Clause;
def "QUAL.OMP.LASTPRIVATE" : Clause;
def "QUAL.OMP.COPYIN" : Clause;
def "QUAL.OMP.COPYPRIVATE" : Clause;
def "QUAL.OMP.REDUCTION" : Clause;
def "QUAL.OMP.REDUCTION.ADD" : Clause;
def "QUAL.OMP.REDUCTION.SUB" : Clause;
def "QUAL.OMP.REDUCTION.MUL" : Clause;
def "QUAL.OMP.REDUCTION.AND" : Clause;
def "QUAL.OMP.REDUCTION.OR" : Clause;
def "QUAL.OMP.REDUCTION.XOR" : Clause;
def "QUAL.OMP.REDUCTION.BAND" : Clause;
def "QUAL.OMP.REDUCTION.BOR" : Clause;
def "QUAL.OMP.REDUCTION.MAX" : Clause;
def "QUAL.OMP.REDUCTION.MIN" : Clause;
def "QUAL.OMP.REDUCTION.UDR" : Clause;
def "QUAL.OMP.TO" : Clause;
def "QUAL.OMP.FROM" : Clause;
def "QUAL.OMP.LINEAR" : Clause;
def "QUAL.OMP.UNIFORM" : Clause;
def "QUAL.OMP.ALIGNED" : Clause;
def "QUAL.OMP.FLUSH" : Clause;
def "QUAL.OMP.THREADPRIVATE" : Clause;
def "QUAL.OMP.DEVICE" : Clause;
def "QUAL.OMP.IS_DEVICE_PTR" : Clause;
def "QUAL.OMP.USE_DEVICE_PTR" : Clause;
def "QUAL.OMP.DEFAULTMAP.TOFROM.SCALAR" : Clause;
def "QUAL.OMP.MAP.TO" : Clause;
def "QUAL.OMP.MAP.FROM" : Clause;
def "QUAL.OMP.MAP.TOFROM" : Clause;
def "QUAL.OMP.MAP.ALLOC" : Clause;
def "QUAL.OMP.MAP.RELEASE" : Clause;
def "QUAL.OMP.MAP.DELETE" : Clause;
def "QUAL.OMP.MAP.ALWAYS.TO" : Clause;
def "QUAL.OMP.MAP.ALWAYS.FROM" : Clause;
def "QUAL.OMP.MAP.ALWAYS.TOFROM" : Clause;
def "QUAL.OMP.MAP.ALWAYS.ALLOC" : Clause;
def "QUAL.OMP.MAP.ALWAYS.RELEASE" : Clause;
def "QUAL.OMP.MAP.ALWAYS.DELETE" : Clause;
def "QUAL.OMP.DEPEND.IN" : Clause;
def "QUAL.OMP.DEPEND.OUT" : Clause;
def "QUAL.OMP.DEPEND.INOUT" : Clause;
def "QUAL.OMP.DEPEND.SOURCE" : Clause;
def "QUAL.OMP.DEPEND.SINK" : Clause;
def "QUAL.OMP.ORDERED.THREADS" : Clause;
def "QUAL.OMP.ORDERED.SIMD" : Clause;
def "QUAL.OMP.CANCEL.PARALLEL" : Clause;
def "QUAL.OMP.CANCEL.LOOP" : Clause;
def "QUAL.OMP.CANCEL.SECTIONS" : Clause;
def "QUAL.OMP.CANCEL.TASKGROUP" : Clause;
def "QUAL.OMP.NAME" : Clause;
