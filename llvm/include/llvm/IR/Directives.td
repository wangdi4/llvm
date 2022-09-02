#if INTEL_COLLAB //  -*- tablegen -*-
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
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

def "DIR.OMP.MASKED" : Directive;
def "DIR.OMP.END.MASKED" : Directive;

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

def "DIR.OMP.DISPATCH" : Directive;
def "DIR.OMP.END.DISPATCH" : Directive;

def "DIR.OMP.CANCEL" : Directive;
def "DIR.OMP.END.CANCEL" : Directive;

def "DIR.OMP.CANCELLATION.POINT" : Directive;
def "DIR.OMP.END.CANCELLATION.POINT" : Directive;

def "DIR.OMP.GENERICLOOP" : Directive;
def "DIR.OMP.END.GENERICLOOP" : Directive;

def "DIR.OMP.INTEROP"     : Directive;
def "DIR.OMP.END.INTEROP" : Directive;

def "DIR.OMP.PREFETCH"     : Directive;
def "DIR.OMP.END.PREFETCH" : Directive;

def "DIR.OMP.SCOPE" : Directive;
def "DIR.OMP.END.SCOPE" : Directive;

def "DIR.OMP.TILE" : Directive;
def "DIR.OMP.END.TILE" : Directive;

def "DIR.OMP.SCAN" : Directive;
def "DIR.OMP.END.SCAN" : Directive;

#if INTEL_CUSTOMIZATION
// Directives used to mark begin/end of a loop determined by VPO vectorizer
// legality analysis to be legal to vectorize.
// These are in the OpenMP section because they need to be processed by
// WRN construction as if they were a SIMD directive.
def "DIR.VPO.AUTO.VEC" : Directive;
def "DIR.VPO.END.AUTO.VEC" : Directive;
#endif // INTEL_CUSTOMIZATION


// =====================================================================
// * Begin Non-OpenMP Directives
// * This section defines directives unrelated to OpenMP
// =====================================================================

#if INTEL_CUSTOMIZATION
def "DIR.PRAGMA.IVDEP" : Directive;
def "DIR.PRAGMA.END.IVDEP" : Directive;
def "DIR.PRAGMA.BLOCK_LOOP" : Directive;
def "DIR.PRAGMA.END.BLOCK_LOOP" : Directive;
def "DIR.PRAGMA.DISTRIBUTE_POINT" : Directive;
def "DIR.PRAGMA.END.DISTRIBUTE_POINT" : Directive;
#endif // INTEL_CUSTOMIZATION

def "DIR.VPO.GUARD.MEM.MOTION" : Directive;
def "DIR.VPO.END.GUARD.MEM.MOTION" : Directive;

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
def "QUAL.OMP.INIT" : Clause;
def "QUAL.OMP.DESTROY" : Clause;
def "QUAL.OMP.USE" : Clause;
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
#if INTEL_CUSTOMIZATION
def "QUAL.OMP.REDUCTION.EQV" : Clause;
def "QUAL.OMP.REDUCTION.NEQV" : Clause;
#endif // INTEL_CUSTOMIZATION
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
#if INTEL_CUSTOMIZATION
def "QUAL.OMP.INREDUCTION.EQV" : Clause;
def "QUAL.OMP.INREDUCTION.NEQV" : Clause;
#endif // INTEL_CUSTOMIZATION
def "QUAL.OMP.INREDUCTION.MAX" : Clause;
def "QUAL.OMP.INREDUCTION.MIN" : Clause;
def "QUAL.OMP.INREDUCTION.UDR" : Clause;
def "QUAL.OMP.TO" : Clause;
def "QUAL.OMP.FROM" : Clause;
def "QUAL.OMP.LINEAR" : Clause;
def "QUAL.OMP.UNIFORM" : Clause;
def "QUAL.OMP.ALIGNED" : Clause;
def "QUAL.OMP.NONTEMPORAL" : Clause;
def "QUAL.OMP.FLUSH" : Clause;
def "QUAL.OMP.THREADPRIVATE" : Clause;
def "QUAL.OMP.FILTER" : Clause;
def "QUAL.OMP.DEVICE" : Clause;
def "QUAL.OMP.SUBDEVICE" : Clause;
def "QUAL.OMP.IS_DEVICE_PTR" : Clause;
def "QUAL.OMP.HAS_DEVICE_ADDR" : Clause;
def "QUAL.OMP.USE_DEVICE_PTR" : Clause;
def "QUAL.OMP.USE_DEVICE_ADDR" : Clause;
def "QUAL.OMP.DEFAULTMAP.ALLOC" : Clause;
def "QUAL.OMP.DEFAULTMAP.TO" : Clause;
def "QUAL.OMP.DEFAULTMAP.FROM" : Clause;
def "QUAL.OMP.DEFAULTMAP.TOFROM" : Clause;
def "QUAL.OMP.DEFAULTMAP.FIRSTPRIVATE" : Clause;
def "QUAL.OMP.DEFAULTMAP.NONE" : Clause;
def "QUAL.OMP.DEFAULTMAP.DEFAULT" : Clause;
def "QUAL.OMP.DEFAULTMAP.PRESENT" : Clause;
def "QUAL.OMP.MAP" : Clause;
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
def "QUAL.OMP.DEPARRAY" : Clause;
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
def "QUAL.OMP.ALLOCATE" : Clause;
def "QUAL.OMP.NOCONTEXT" : Clause;
def "QUAL.OMP.NOVARIANTS" : Clause;
def "QUAL.OMP.DATA" : Clause;
def "QUAL.OMP.SIZES" : Clause;
def "QUAL.OMP.INTEROP" : Clause;
def "QUAL.OMP.INCLUSIVE" : Clause;
def "QUAL.OMP.EXCLUSIVE" : Clause;
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
def "QUAL.OMP.IMPLICIT" : Clause;
def "QUAL.OMP.DISPATCH.CALL" : Clause;
def "QUAL.OMP.NORMALIZED.IV" : Clause;
def "QUAL.OMP.NORMALIZED.UB" : Clause;
def "QUAL.OMP.LIVEIN" : Clause;
def "QUAL.OMP.CANCELLATION.POINTS" : Clause;
def "QUAL.OMP.OPERAND.ADDR" : Clause;
def "QUAL.OMP.JUMP.TO.END.IF" : Clause;
def "QUAL.OMP.OFFLOAD.ENTRY.IDX" : Clause;
def "QUAL.OMP.OFFLOAD.NDRANGE" : Clause;
def "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE" : Clause;
def "QUAL.OMP.OFFLOAD.HAS.TEAMS.REDUCTION" : Clause;

// =====================================================================
// * Begin Non-OpenMP Clauses
// * This section defines qualifiers unrelated to OpenMP
// =====================================================================
#if INTEL_CUSTOMIZATION
def "QUAL.PRAGMA.ARRAY" : Clause;

// Fortran DO_CONCURRENT Tag on OMP Target/Teams/GenericLoop construct
def "QUAL.EXT.DO.CONCURRENT" : Clause;
#endif // INTEL_CUSTOMIZATION


#endif // INTEL_COLLAB
