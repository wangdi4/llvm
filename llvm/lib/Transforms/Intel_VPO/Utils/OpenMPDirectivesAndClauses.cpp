//=- OpenMPDirectivesAndClauses.cpp - Table of OpenMP directives -*- C++ -*-==//
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
/// This file provides a set of utilities for generating strings for OpenMP
/// directives and qualifiers. These are used to create Metadata that can be
/// attached to directive intrinsics, etc.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Support/Debug.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"
#include "llvm/Analysis/Intel_Directives.h"

#define DEBUG_TYPE "vpo-utils"

using namespace llvm;
using namespace llvm::vpo;

StringRef VPOUtils::getDirectiveName(int Id) {
  // skip "DIR_OMP_"
  return IntelDirectives::DirectiveStrings[Id].substr(8);
}

StringRef VPOUtils::getClauseName(int Id) {
  return IntelIntrinsicUtils::getClauseString(Id).substr(9); // skip "QUAL_OMP_"
}

bool VPOUtils::isOpenMPDirective(StringRef DirFullName) {
  return IntelDirectives::DirectiveIDs.count(DirFullName);
}

bool VPOUtils::isOpenMPClause(StringRef ClauseFullName) {
  return IntelDirectives::ClauseIDs.count(ClauseFullName);
}

int VPOUtils::getDirectiveID(StringRef DirFullName) {
  // DEBUG(dbgs() << "DirFullName 1" << DirFullName << "\n" );
  assert(VPOUtils::isOpenMPDirective(DirFullName) && 
         "Directive string not found");
  return IntelDirectives::DirectiveIDs[DirFullName];
}

int VPOUtils::getClauseID(StringRef ClauseFullName) {
  assert(VPOUtils::isOpenMPClause(ClauseFullName) && 
         "Clause string not found");
  return IntelDirectives::ClauseIDs[ClauseFullName];
}

bool VPOUtils::isBeginDirective(StringRef DirString) {
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isBeginDirective(DirID);
}

bool VPOUtils::isBeginDirective(int DirID) {
  switch(DirID) {
  case DIR_OMP_PARALLEL:
  case DIR_OMP_LOOP:
  case DIR_OMP_PARALLEL_LOOP:
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
  case DIR_OMP_TASKGROUP:
  case DIR_OMP_TASKLOOP:
  case DIR_OMP_TARGET:
  case DIR_OMP_TARGET_DATA:
  case DIR_OMP_TEAMS:
  case DIR_OMP_DISTRIBUTE:
  case DIR_OMP_DISTRIBUTE_PARLOOP:
    return true;
  }
  return false;
}

bool VPOUtils::isEndDirective(StringRef DirString) {
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isEndDirective(DirID);
}

bool VPOUtils::isEndDirective(int DirID) {
  switch(DirID) {
  case DIR_OMP_END_PARALLEL:
  case DIR_OMP_END_LOOP:
  case DIR_OMP_END_PARALLEL_LOOP:
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
  case DIR_OMP_END_TASKGROUP:
  case DIR_OMP_END_TASKLOOP:
  case DIR_OMP_END_TARGET:
  case DIR_OMP_END_TARGET_DATA:
  case DIR_OMP_END_TEAMS:
  case DIR_OMP_END_DISTRIBUTE:
  case DIR_OMP_END_DISTRIBUTE_PARLOOP:
    return true;
  }
  return false;
}

bool VPOUtils::isBeginOrEndDirective(StringRef DirString) {
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isBeginOrEndDirective(DirID);
}

bool VPOUtils::isBeginOrEndDirective(int DirID) {
  return VPOUtils::isBeginDirective(DirID) ||
         VPOUtils::isEndDirective(DirID);
}

bool VPOUtils::isStandAloneDirective(StringRef DirString) {
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isStandAloneDirective(DirID);
}

bool VPOUtils::isStandAloneDirective(int DirID) {
  switch(DirID) {
  case DIR_OMP_BARRIER:
  case DIR_OMP_TASKWAIT:
  case DIR_OMP_TASKYIELD:
  case DIR_OMP_FLUSH:
  case DIR_OMP_SECTION:
  case DIR_OMP_TARGET_ENTER_DATA:
  case DIR_OMP_TARGET_EXIT_DATA:
  case DIR_OMP_TARGET_UPDATE:
  case DIR_OMP_CANCEL:
  case DIR_OMP_CANCELLATION_POINT:
  case DIR_OMP_THREADPRIVATE:
    return true;
  }
  return false;
}

bool VPOUtils::isListEndDirective(StringRef DirString) {
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isListEndDirective(DirID);
}

bool VPOUtils::isListEndDirective(int DirID) {
  return DirID == DIR_QUAL_LIST_END;
}

bool VPOUtils::isReductionClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_REDUCTION_ADD:
    case QUAL_OMP_REDUCTION_SUB:
    case QUAL_OMP_REDUCTION_MUL:
    case QUAL_OMP_REDUCTION_AND:
    case QUAL_OMP_REDUCTION_OR:
    case QUAL_OMP_REDUCTION_XOR:
    case QUAL_OMP_REDUCTION_BAND:
    case QUAL_OMP_REDUCTION_BOR:
    case QUAL_OMP_REDUCTION_UDR:
    return true;
  }
  return false;
}

bool VPOUtils::isScheduleClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_DIST_SCHEDULE_STATIC:
    case QUAL_OMP_SCHEDULE_AUTO:
    case QUAL_OMP_SCHEDULE_DYNAMIC:
    case QUAL_OMP_SCHEDULE_GUIDED:
    case QUAL_OMP_SCHEDULE_RUNTIME:
    case QUAL_OMP_SCHEDULE_STATIC:
    return true;
  }
  return false;
}

bool VPOUtils::isMapClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_TO:
    case QUAL_OMP_FROM:
    case QUAL_OMP_MAP_TO:
    case QUAL_OMP_MAP_FROM:
    case QUAL_OMP_MAP_TOFROM:
    case QUAL_OMP_MAP_ALLOC:
    case QUAL_OMP_MAP_RELEASE:
    case QUAL_OMP_MAP_DELETE:
    case QUAL_OMP_MAP_ALWAYS_TO:
    case QUAL_OMP_MAP_ALWAYS_FROM:
    case QUAL_OMP_MAP_ALWAYS_TOFROM:
    case QUAL_OMP_MAP_ALWAYS_ALLOC:
    case QUAL_OMP_MAP_ALWAYS_RELEASE:
    case QUAL_OMP_MAP_ALWAYS_DELETE:
    return true;
  }
  return false;
}
