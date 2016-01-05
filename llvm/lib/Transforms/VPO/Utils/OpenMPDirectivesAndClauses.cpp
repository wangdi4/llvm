//=- OpenMPDirectivesAndClauses.cpp - Table of OpenMP directives -*- C++ -*-==//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file provides a set of utilities for generating strings for OpenMP
/// directives and qualifiers. These are used to create Metadata that can be
/// attached to directive intrinsics, etc.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Support/Debug.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

#define DEBUG_TYPE "vpo-utils"

using namespace llvm;
using namespace llvm::vpo;

std::unordered_map<int, StringRef> VPOUtils::DirectiveStrings = {
  { DIR_OMP_PARALLEL,
    "DIR.OMP.PARALLEL" },
  { DIR_OMP_END_PARALLEL,
    "DIR.OMP.END.PARALLEL" },
  { DIR_OMP_PARALLEL_LOOP,
    "DIR.OMP.PARALLEL.LOOP" },
  { DIR_OMP_END_PARALLEL_LOOP,
    "DIR.OMP.END.PARALLEL.LOOP" },
  { DIR_OMP_LOOP_SIMD,
    "DIR.OMP.LOOP.SIMD" },
  { DIR_OMP_END_LOOP_SIMD,
    "DIR.OMP.END.LOOP.SIMD" },
  { DIR_OMP_PARALLEL_LOOP_SIMD,
    "DIR.OMP.PARALLEL.LOOP.SIMD" },
  { DIR_OMP_END_PARALLEL_LOOP_SIMD,
    "DIR.OMP.END.PARALLEL.LOOP.SIMD" },
  { DIR_OMP_SECTIONS,
    "DIR.OMP.SECTIONS" },
  { DIR_OMP_END_SECTIONS,
    "DIR.OMP.END.SECTIONS" },
  { DIR_OMP_PARALLEL_SECTIONS,
    "DIR.OMP.PARALLEL.SECTIONS" },
  { DIR_OMP_END_PARALLEL_SECTIONS,
    "DIR.OMP.END.PARALLEL.SECTIONS" },
  { DIR_OMP_WORKSHARE,
    "DIR.OMP.WORKSHARE" },
  { DIR_OMP_END_WORKSHARE,
    "DIR.OMP.END.WORKSHARE" },
  { DIR_OMP_PARALLEL_WORKSHARE,
    "DIR.OMP.PARALLEL.WORKSHARE" },
  { DIR_OMP_END_PARALLEL_WORKSHARE,
    "DIR.OMP.END.PARALLEL.WORKSHARE" },
  { DIR_OMP_SECTION,
    "DIR.OMP.SECTION" },
  { DIR_OMP_SINGLE,
    "DIR.OMP.SINGLE" },
  { DIR_OMP_END_SINGLE,
    "DIR.OMP.END.SINGLE" },
  { DIR_OMP_TASK,
    "DIR.OMP.TASK" },
  { DIR_OMP_END_TASK,
    "DIR.OMP.END.TASK" },
  { DIR_OMP_MASTER,
    "DIR.OMP.MASTER" },
  { DIR_OMP_END_MASTER,
    "DIR.OMP.END.MASTER" },
  { DIR_OMP_CRITICAL,
    "DIR.OMP.CRITICAL" },
  { DIR_OMP_END_CRITICAL,
    "DIR.OMP.END.CRITICAL" },
  { DIR_OMP_BARRIER,
    "DIR.OMP.BARRIER" },
  { DIR_OMP_TASKWAIT,
    "DIR.OMP.TASKWAIT" },
  { DIR_OMP_TASKYIELD,
    "DIR.OMP.TASKYIELD" },
  { DIR_OMP_ATOMIC,
    "DIR.OMP.ATOMIC" },
  { DIR_OMP_END_ATOMIC,
    "DIR.OMP.END.ATOMIC" },
  { DIR_OMP_FLUSH,
    "DIR.OMP.FLUSH" },
  { DIR_OMP_ORDERED,
    "DIR.OMP.ORDERED" },
  { DIR_OMP_SIMD,
    "DIR.OMP.SIMD" },
  { DIR_OMP_END_SIMD,
    "DIR.OMP.END.SIMD" },
  { DIR_OMP_TASKLOOP,
    "DIR.OMP.TASKLOOP" },
  { DIR_OMP_END_TASKLOOP,
    "DIR.OMP.END.TASKLOOP" },
  { DIR_OMP_TASKLOOP_SIMD,
    "DIR.OMP.TASKLOOP.SIMD" },
  { DIR_OMP_END_TASKLOOP_SIMD,
    "DIR.OMP.END.TASKLOOP.SIMD" },
  { DIR_OMP_TARGET,
    "DIR.OMP.TARGET" },
  { DIR_OMP_END_TARGET,
    "DIR.OMP.END.TARGET" },
  { DIR_OMP_TARGET_DATA,
    "DIR.OMP.TARGET.DATA" },
  { DIR_OMP_END_TARGET_DATA,
    "DIR.OMP.END.TARGET.DATA" },
  { DIR_OMP_TARGET_UPDATE,
    "DIR.OMP.TARGET.UPDATE" },
  { DIR_OMP_END_TARGET_UPDATE,
    "DIR.OMP.END.TARGET.UPDATE" },
  { DIR_OMP_TEAMS,
    "DIR.OMP.TEAMS" },
  { DIR_OMP_END_TEAMS,
    "DIR.OMP.END.TEAMS" },
  { DIR_OMP_TEAMS_DISTRIBUTE,
    "DIR.OMP.TEAMS.DISTRIBUTE" },
  { DIR_OMP_END_TEAMS_DISTRIBUTE,
    "DIR.OMP.END.TEAMS.DISTRIBUTE" },
  { DIR_OMP_TEAMS_SIMD,
    "DIR.OMP.TEAMS.SIMD" },
  { DIR_OMP_END_TEAMS_SIMD,
    "DIR.OMP.END.TEAMS.SIMD" },
  { DIR_OMP_TEAMS_DISTRIBUTE_SIMD,
    "DIR.OMP.TEAMS.DISTRIBUTE.SIMD" },
  { DIR_OMP_END_TEAMS_DISTRIBUTE_SIMD,
    "DIR.OMP.END.TEAMS.DISTRIBUTE.SIMD" },
  { DIR_OMP_DISTRIBUTE,
    "DIR.OMP.DISTRIBUTE" },
  { DIR_OMP_END_DISTRIBUTE,
    "DIR.OMP.END.DISTRIBUTE" },
  { DIR_OMP_DISTRIBUTE_PARLOOP,
    "DIR.OMP.DISTRIBUTE.PARLOOP" },
  { DIR_OMP_END_DISTRIBUTE_PARLOOP,
    "DIR.OMP.END.DISTRIBUTE.PARLOOP" },
  { DIR_OMP_DISTRIBUTE_SIMD,
    "DIR.OMP.DISTRIBUTE.SIMD" },
  { DIR_OMP_END_DISTRIBUTE_SIMD,
    "DIR.OMP.END.DISTRIBUTE.SIMD" },
  { DIR_OMP_DISTRIBUTE_PARLOOP_SIMD,
    "DIR.OMP.DISTRIBUTE.PARLOOP.SIMD" },
  { DIR_OMP_END_DISTRIBUTE_PARLOOP_SIMD,
    "DIR.OMP.END.DISTRIBUTE.PARLOOP.SIMD" },
  { DIR_OMP_TARGET_ENTER_DATA,
    "DIR.OMP.TARGET.ENTER.DATA" },
  { DIR_OMP_TARGET_EXIT_DATA,
    "DIR.OMP.TARGET.EXIT.DATA" },
  { DIR_OMP_TARGET_TEAMS,
    "DIR.OMP.TARGET.TEAMS" },
  { DIR_OMP_END_TARGET_TEAMS,
    "DIR.OMP.END.TARGET.TEAMS" },
  { DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP,
    "DIR.OMP.TEAMS.DISTRIBUTE.PARLOOP" },
  { DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP,
    "DIR.OMP.END.TEAMS.DISTRIBUTE.PARLOOP" },
  { DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
    "DIR.OMP.TEAMS.DISTRIBUTE.PARLOOP.SIMD" },
  { DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
    "DIR.OMP.END.TEAMS.DISTRIBUTE.PARLOOP.SIMD" },
  { DIR_OMP_TARGET_TEAMS_DISTRIBUTE,
    "DIR.OMP.TARGET.TEAMS.DISTRIBUTE" },
  { DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE,
    "DIR.OMP.END.TARGET.TEAMS.DISTRIBUTE" },
  { DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP,
    "DIR.OMP.TARGET.TEAMS.DISTRIBUTE.PARLOOP" },
  { DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP,
    "DIR.OMP.END.TARGET.TEAMS.DISTRIBUTE.PARLOOP" },
  { DIR_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD,
    "DIR.OMP.TARGET.TEAMS.DISTRIBUTE.SIMD" },
  { DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_SIMD,
    "DIR.OMP.END.TARGET.TEAMS.DISTRIBUTE.SIMD" },
  { DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
    "DIR.OMP.TARGET.TEAMS.DISTRIBUTE.PARLOOP.SIMD" },
  { DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD,
    "DIR.OMP.END.TARGET.TEAMS.DISTRIBUTE.PARLOOP.SIMD" },
  { DIR_OMP_CANCEL,
    "DIR.OMP.CANCEL" },
  { DIR_OMP_CANCELLATION_POINT,
    "DIR.OMP.CANCELLATION.POINT" },
  { DIR_QUAL_LIST_END,
    "DIR.QUAL.LIST.END" }
};

std::unordered_map<int, StringRef> VPOUtils::ClauseStrings = {
  { QUAL_OMP_DEFAULT_NONE,
    "QUAL.OMP.DEFAULT.NONE" },
  { QUAL_OMP_DEFAULT_SHARED,
    "QUAL.OMP.DEFAULT.SHARED" },
  { QUAL_OMP_DEFAULT_PRIVATE,
    "QUAL.OMP.DEFAULT.PRIVATE" },
  { QUAL_OMP_DEFAULT_FIRSTPRIVATE,
    "QUAL.OMP.DEFAULT.FIRSTPRIVATE" },
  { QUAL_OMP_MERGEABLE,
    "QUAL.OMP.MERGEABLE" },
  { QUAL_OMP_NOWAIT,
    "QUAL.OMP.NOWAIT" },
  { QUAL_OMP_NOGROUP,
    "QUAL.OMP.NOGROUP" },
  { QUAL_OMP_UNTIED,
    "QUAL.OMP.UNTIED" },
  { QUAL_OMP_READ,
    "QUAL.OMP.READ" },
  { QUAL_OMP_READ_SEQ_CST,
    "QUAL.OMP.READ.SEQ.CST" },
  { QUAL_OMP_WRITE,
    "QUAL.OMP.WRITE" },
  { QUAL_OMP_WRITE_SEQ_CST,
    "QUAL.OMP.WRITE.SEQ.CST" },
  { QUAL_OMP_UPDATE,
    "QUAL.OMP.UPDATE" },
  { QUAL_OMP_UPDATE_SEQ_CST,
    "QUAL.OMP.UPDATE.SEQ.CST" },
  { QUAL_OMP_CAPTURE,
    "QUAL.OMP.CAPTURE" },
  { QUAL_OMP_CAPTURE_SEQ_CST,
    "QUAL.OMP.CAPTURE.SEQ.CST" },
  { QUAL_OMP_SCHEDULE_AUTO,
    "QUAL.OMP.SCHEDULE.AUTO" },
  { QUAL_OMP_SCHEDULE_RUNTIME,
    "QUAL.OMP.SCHEDULE.RUNTIME" },
  { QUAL_OMP_PROC_BIND_MASTER,
    "QUAL.OMP.PROC.BIND.MASTER" },
  { QUAL_OMP_PROC_BIND_CLOSE,
    "QUAL.OMP.PROC.BIND.CLOSE" },
  { QUAL_OMP_PROC_BIND_SPREAD,
    "QUAL.OMP.PROC.BIND.SPREAD" },

  { QUAL_OMP_IF,
    "QUAL.OMP.IF" },
  { QUAL_OMP_COLLAPSE,
    "QUAL.OMP.COLLAPSE" },
  { QUAL_OMP_NUM_THREADS,
    "QUAL.OMP.NUM.THREADS" },
  { QUAL_OMP_ORDERED,
    "QUAL.OMP.ORDERED" },
  { QUAL_OMP_SAFELEN,
    "QUAL.OMP.SAFELEN" },
  { QUAL_OMP_SIMDLEN,
    "QUAL.OMP.SIMDLEN" },
  { QUAL_OMP_FINAL,
    "QUAL.OMP.FINAL" },
  { QUAL_OMP_GRAINSIZE,
    "QUAL.OMP.GRAINSIZE" },
  { QUAL_OMP_NUM_TASKS,
    "QUAL.OMP.NUM.TASKS" },
  { QUAL_OMP_PRIORITY,
    "QUAL.OMP.PRIORITY" },
  { QUAL_OMP_NUM_TEAMS,
    "QUAL.OMP.NUM.TEAMS" },
  { QUAL_OMP_THREAD_LIMIT,
    "QUAL.OMP.THREAD.LIMIT" },
  { QUAL_OMP_DIST_SCHEDULE_STATIC,
    "QUAL.OMP.DIST.SCHEDULE.STATIC" },
  { QUAL_OMP_SCHEDULE_STATIC,
    "QUAL.OMP.SCHEDULE.STATIC" },
  { QUAL_OMP_SCHEDULE_DYNAMIC,
    "QUAL.OMP.SCHEDULE.DYNAMIC" },
  { QUAL_OMP_SCHEDULE_GUIDED,
    "QUAL.OMP.SCHEDULE.GUIDED" },

  { QUAL_OMP_SHARED,
    "QUAL.OMP.SHARED" },
  { QUAL_OMP_PRIVATE,
    "QUAL.OMP.PRIVATE" },
  { QUAL_OMP_FIRSTPRIVATE,
    "QUAL.OMP.FIRSTPRIVATE" },
  { QUAL_OMP_LASTPRIVATE,
    "QUAL.OMP.LASTPRIVATE" },
  { QUAL_OMP_COPYIN,
    "QUAL.OMP.COPYIN" },
  { QUAL_OMP_COPYPRIVATE,
    "QUAL.OMP.COPYPRIVATE" },
  { QUAL_OMP_REDUCTION_ADD,
    "QUAL.OMP.REDUCTION.ADD" },
  { QUAL_OMP_REDUCTION_SUB,
    "QUAL.OMP.REDUCTION.SUB" },
  { QUAL_OMP_REDUCTION_MUL,
    "QUAL.OMP.REDUCTION.MUL" },
  { QUAL_OMP_REDUCTION_AND,
    "QUAL.OMP.REDUCTION.AND" },
  { QUAL_OMP_REDUCTION_OR,
    "QUAL.OMP.REDUCTION.OR" },
  { QUAL_OMP_REDUCTION_XOR,
    "QUAL.OMP.REDUCTION.XOR" },
  { QUAL_OMP_REDUCTION_BAND,
    "QUAL.OMP.REDUCTION.BAND" },
  { QUAL_OMP_REDUCTION_BOR,
    "QUAL.OMP.REDUCTION.BOR" },
  { QUAL_OMP_REDUCTION_UDR,
    "QUAL.OMP.REDUCTION.UDR" },
  { QUAL_OMP_TO,
    "QUAL.OMP.TO" },
  { QUAL_OMP_FROM,
    "QUAL.OMP.FROM" },
  { QUAL_OMP_LINEAR,
    "QUAL.OMP.LINEAR" },
  { QUAL_OMP_ALIGNED,
    "QUAL.OMP.ALIGNED" },
  { QUAL_OMP_FLUSH,
    "QUAL.OMP.FLUSH" },
  { QUAL_OMP_THREADPRIVATE,
    "QUAL.OMP.THREADPRIVATE" },
  { QUAL_OMP_MAP_TO,
    "QUAL.OMP.MAP.TO" },
  { QUAL_OMP_MAP_FROM,
    "QUAL.OMP.MAP.FROM" },
  { QUAL_OMP_MAP_TOFROM,
    "QUAL.OMP.MAP.TOFROM" },
  { QUAL_OMP_DEPEND_IN,
    "QUAL.OMP.DEPEND.IN" },
  { QUAL_OMP_DEPEND_OUT,
    "QUAL.OMP.DEPEND.OUT" },
  { QUAL_OMP_DEPEND_INOUT,
    "QUAL.OMP.DEPEND.INOUT" },
  { QUAL_OMP_NAME,
    "QUAL.OMP.NAME" },
  { QUAL_LIST_END,         // TBD: SIMD Function cloning emits DIR.QUAL.LIST.END
                           // For now changed the string to match.
    "DIR.QUAL.LIST.END" } 
};

StringMap<int> VPOUtils::DirectiveIDs;

StringMap<int> VPOUtils::ClauseIDs;

StringRef VPOUtils::getDirectiveString(int Id)
{
  return VPOUtils::DirectiveStrings[Id];
}

StringRef VPOUtils::getClauseString(int Id)
{
  return VPOUtils::ClauseStrings[Id];
}

StringRef VPOUtils::getDirectiveName(int Id)
{
  return VPOUtils::DirectiveStrings[Id].substr(8); // skip "DIR_OMP_"
}

StringRef VPOUtils::getClauseName(int Id)
{
  return VPOUtils::ClauseStrings[Id].substr(9); // skip "QUAL_OMP_"
}

void VPOUtils::initDirectiveAndClauseStringMap()
{
  int Id; // an enum in OMP_DIRECTIVES or OMP_CLAUSES

  // Initialize mapping from Directive string to ID.
  // First enum in OMP_DIRECTIVES starts with 0
  DirectiveIDs.clear();
  for (Id=0; Id <= DIR_QUAL_LIST_END; ++Id) {
    StringRef S = VPOUtils::getDirectiveString(Id);
    DirectiveIDs.insert(std::make_pair(S, Id));
  }

  // Initialize mapping from Clause string to ID.
  // First enum in OMP_CLAUSES starts with 0
  ClauseIDs.clear();
  for (Id=0; Id <= QUAL_LIST_END; ++Id) {
    StringRef S = VPOUtils::getClauseString(Id);
    ClauseIDs.insert(std::make_pair(S, Id));
  }
}

bool VPOUtils::isOpenMPDirective(StringRef DirFullName)
{

  if (VPOUtils::DirectiveIDs.count(DirFullName)) {
    return true;
  }
  return false;
}

bool VPOUtils::isOpenMPClause(StringRef ClauseFullName)
{
  if (VPOUtils::ClauseIDs.count(ClauseFullName)) {
    return true;
  }
  return false;
}

int VPOUtils::getDirectiveID(StringRef DirFullName)
{
  // DEBUG(dbgs() << "DirFullName 1" << DirFullName << "\n" );
  assert(VPOUtils::isOpenMPDirective(DirFullName) && 
         "Directive string not found");
  return VPOUtils::DirectiveIDs[DirFullName];
}

int VPOUtils::getClauseID(StringRef ClauseFullName)
{
  assert(VPOUtils::isOpenMPClause(ClauseFullName) && 
         "Clause string not found");
  return VPOUtils::ClauseIDs[ClauseFullName];
}

bool VPOUtils::isBeginDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isBeginDirective(DirID);
}

bool VPOUtils::isBeginDirective(
  int DirID
)
{
  switch(DirID) {
    case DIR_OMP_PARALLEL:
    case DIR_OMP_PARALLEL_LOOP:
    case DIR_OMP_LOOP_SIMD:
    case DIR_OMP_PARALLEL_LOOP_SIMD:
    case DIR_OMP_SECTIONS:
    case DIR_OMP_PARALLEL_SECTIONS:
    case DIR_OMP_WORKSHARE:
    case DIR_OMP_PARALLEL_WORKSHARE:
    case DIR_OMP_SINGLE:
    case DIR_OMP_TASK:
    case DIR_OMP_MASTER:
    case DIR_OMP_CRITICAL:
    case DIR_OMP_ATOMIC:
    case DIR_OMP_ORDERED:
    case DIR_OMP_SIMD:
    case DIR_OMP_TASKLOOP:
    case DIR_OMP_TASKLOOP_SIMD:
    case DIR_OMP_TARGET:
    case DIR_OMP_TARGET_DATA:
    case DIR_OMP_TARGET_UPDATE:
    case DIR_OMP_TEAMS:
    case DIR_OMP_TEAMS_DISTRIBUTE:
    case DIR_OMP_TEAMS_SIMD:
    case DIR_OMP_TEAMS_DISTRIBUTE_SIMD:
    case DIR_OMP_DISTRIBUTE:
    case DIR_OMP_DISTRIBUTE_PARLOOP:
    case DIR_OMP_DISTRIBUTE_SIMD:
    case DIR_OMP_DISTRIBUTE_PARLOOP_SIMD:
    case DIR_OMP_TARGET_TEAMS:
    case DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP:
    case DIR_OMP_TEAMS_DISTRIBUTE_PARLOOP_SIMD:
    case DIR_OMP_TARGET_TEAMS_DISTRIBUTE:
    case DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP:
    case DIR_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD:
    case DIR_OMP_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD:
      return true;
  }
  return false;
}


bool VPOUtils::isEndDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isEndDirective(DirID);
}

bool VPOUtils::isEndDirective(
  int DirID
)
{
  switch(DirID) {
    case DIR_OMP_END_PARALLEL:
    case DIR_OMP_END_PARALLEL_LOOP:
    case DIR_OMP_END_LOOP_SIMD:
    case DIR_OMP_END_PARALLEL_LOOP_SIMD:
    case DIR_OMP_END_SECTIONS:
    case DIR_OMP_END_PARALLEL_SECTIONS:
    case DIR_OMP_END_WORKSHARE:
    case DIR_OMP_END_PARALLEL_WORKSHARE:
    case DIR_OMP_END_SINGLE:
    case DIR_OMP_END_TASK:
    case DIR_OMP_END_MASTER:
    case DIR_OMP_END_CRITICAL:
    case DIR_OMP_END_ATOMIC:
    case DIR_OMP_END_ORDERED:
    case DIR_OMP_END_SIMD:
    case DIR_OMP_END_TASKLOOP:
    case DIR_OMP_END_TASKLOOP_SIMD:
    case DIR_OMP_END_TARGET:
    case DIR_OMP_END_TARGET_DATA:
    case DIR_OMP_END_TARGET_UPDATE:
    case DIR_OMP_END_TEAMS:
    case DIR_OMP_END_TEAMS_DISTRIBUTE:
    case DIR_OMP_END_TEAMS_SIMD:
    case DIR_OMP_END_TEAMS_DISTRIBUTE_SIMD:
    case DIR_OMP_END_DISTRIBUTE:
    case DIR_OMP_END_DISTRIBUTE_PARLOOP:
    case DIR_OMP_END_DISTRIBUTE_SIMD:
    case DIR_OMP_END_DISTRIBUTE_PARLOOP_SIMD:
    case DIR_OMP_END_TARGET_TEAMS:
    case DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP:
    case DIR_OMP_END_TEAMS_DISTRIBUTE_PARLOOP_SIMD:
    case DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE:
    case DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP:
    case DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_SIMD:
    case DIR_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARLOOP_SIMD:
      return true;
  }
  return false;
}

bool VPOUtils::isBeginOrEndDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isBeginOrEndDirective(DirID);
}

bool VPOUtils::isBeginOrEndDirective(
  int DirID
)
{
  return VPOUtils::isBeginDirective(DirID) ||
         VPOUtils::isEndDirective(DirID);
}

bool VPOUtils::isSoloDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isSoloDirective(DirID);
}

bool VPOUtils::isSoloDirective(
  int DirID
)
{
  switch(DirID) {
    case DIR_OMP_SECTION:
    case DIR_OMP_BARRIER:
    case DIR_OMP_TASKWAIT:
    case DIR_OMP_TASKYIELD:
    case DIR_OMP_FLUSH:
    case DIR_OMP_TARGET_ENTER_DATA:
    case DIR_OMP_TARGET_EXIT_DATA:
    case DIR_OMP_CANCEL:
    case DIR_OMP_CANCELLATION_POINT:
      return true;
  }
  return false;
}

bool VPOUtils::isListEndDirective(
  StringRef DirString
)
{
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isListEndDirective(DirID);
}

bool VPOUtils::isListEndDirective(
  int DirID
)
{
  return DirID == DIR_QUAL_LIST_END;
}
