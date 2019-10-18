// INTEL_COLLAB   -*- tablegen -*-
//==---- Directives.td - Defines all parallelization and -*- tablegen -*----==//
//                          vectorization directives and clauses.
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the classes used to represent directives and clauses.
/// TableGen uses these definitions to generate enums for the directives and
/// clauses, as well as tables that provide enum-to-string and string-to-enum
/// mappings for the directives and clauses.
///
/// The output from TableGen is included into:
///        include/llvm/Analysis/"Directives.h
///        lib/Analysis/Directives.cpp
///
// ===--------------------------------------------------------------------=== //

class Directive;

class Clause;

// =====================================================================
// * Begin OpenMP Directives
// * This section defines directives used by OpenMP
// * (https://www.openmp.org/specifications)
// =====================================================================

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

def "DIR.OMP.TARGET.VARIANT.DISPATCH" : Directive;
def "DIR.OMP.END.TARGET.VARIANT.DISPATCH" : Directive;

def "DIR.OMP.CANCEL" : Directive;
def "DIR.OMP.END.CANCEL" : Directive;

def "DIR.OMP.CANCELLATION.POINT" : Directive;
def "DIR.OMP.END.CANCELLATION.POINT" : Directive;

def "DIR.OMP.GENERICLOOP" : Directive;
def "DIR.OMP.END.GENERICLOOP" : Directive;

// INTEL_CUSTOMIZATION
// Directives used to mark begin/end of a loop determined by VPO vectorizer
// legality analysis to be legal to vectorize.
// These are in the OpenMP section because they need to be processed by
// WRN construction as if they were a SIMD directive.
def "DIR.VPO.AUTO.VEC" : Directive;
def "DIR.VPO.END.AUTO.VEC" : Directive;
//end INTEL_CUSTOMIZATION


// =====================================================================
// * Begin Non-OpenMP Directives
// * This section defines directives unrelated to OpenMP
// =====================================================================

// INTEL_CUSTOMIZATION
def "DIR.PRAGMA.IVDEP" : Directive;
def "DIR.PRAGMA.END.IVDEP" : Directive;
//end INTEL_CUSTOMIZATION


// =====================================================================
// * Begin OpenMP Clauses
// * This section defines qualifiers used by OpenMP
// * (https://www.openmp.org/specifications)
// =====================================================================

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
def "QUAL.OMP.PROC_BIND.MASTER" : Clause;
def "QUAL.OMP.PROC_BIND.CLOSE" : Clause;
def "QUAL.OMP.PROC_BIND.SPREAD" : Clause;
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
def "QUAL.OMP.REDUCTION.ADD" : Clause;
def "QUAL.OMP.REDUCTION.SUB" : Clause;
def "QUAL.OMP.REDUCTION.MUL" : Clause;
def "QUAL.OMP.REDUCTION.AND" : Clause;
def "QUAL.OMP.REDUCTION.OR" : Clause;
def "QUAL.OMP.REDUCTION.BXOR" : Clause;
def "QUAL.OMP.REDUCTION.BAND" : Clause;
def "QUAL.OMP.REDUCTION.BOR" : Clause;
def "QUAL.OMP.REDUCTION.MAX" : Clause;
def "QUAL.OMP.REDUCTION.MIN" : Clause;
def "QUAL.OMP.REDUCTION.UDR" : Clause;
def "QUAL.OMP.INREDUCTION.ADD" : Clause;
def "QUAL.OMP.INREDUCTION.SUB" : Clause;
def "QUAL.OMP.INREDUCTION.MUL" : Clause;
def "QUAL.OMP.INREDUCTION.AND" : Clause;
def "QUAL.OMP.INREDUCTION.OR" : Clause;
def "QUAL.OMP.INREDUCTION.BXOR" : Clause;
def "QUAL.OMP.INREDUCTION.BAND" : Clause;
def "QUAL.OMP.INREDUCTION.BOR" : Clause;
def "QUAL.OMP.INREDUCTION.MAX" : Clause;
def "QUAL.OMP.INREDUCTION.MIN" : Clause;
def "QUAL.OMP.INREDUCTION.UDR" : Clause;
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
def "QUAL.OMP.HINT" : Clause;
def "QUAL.OMP.NAME" : Clause;
def "QUAL.OMP.BIND.TEAMS" : Clause;
def "QUAL.OMP.BIND.PARALLEL" : Clause;
def "QUAL.OMP.BIND.THREAD" : Clause;
def "QUAL.OMP.ORDER.CONCURRENT" : Clause;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
// Support Intel extension for CSA.
def "QUAL.OMP.SA.NUM_WORKERS" : Clause;
def "QUAL.OMP.SA.SCHEDULE.STATIC" : Clause;
def "QUAL.OMP.SA.PIPELINE" : Clause;
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

// *** Begin Auxiliary OpenMP Clauses ***
// These quals don't correspond to actual clauses in the OpenMP spec,
// but are used to carry additional information for OpenMP constructs

def "QUAL.OMP.TARGET.TASK" : Clause;
def "QUAL.OMP.NORMALIZED.IV" : Clause;
def "QUAL.OMP.NORMALIZED.UB" : Clause;
def "QUAL.OMP.CANCELLATION.POINTS" : Clause;
def "QUAL.OMP.OPERAND.ADDR" : Clause;
def "QUAL.OMP.JUMP.TO.END.IF" : Clause;
def "QUAL.OMP.OFFLOAD.ENTRY.IDX" : Clause;


// =====================================================================
// * Begin Non-OpenMP Clauses
// * This section defines qualifiers unrelated to OpenMP
// =====================================================================
// INTEL_CUSTOMIZATION
def "QUAL.PRAGMA.ARRAY" : Clause;
// end INTEL_CUSTOMIZATION

// end INTEL_COLLAB
