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
#include "llvm/Transforms/Utils/Intel_OpenMPDirectivesAndClauses.h"
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"

#define DEBUG_TYPE "vpo-utils"

using namespace llvm;
using namespace llvm::vpo;

StringMap<int> VPOUtils::DirectiveIDs;

StringMap<int> VPOUtils::ClauseIDs;

StringRef VPOUtils::getDirectiveName(int Id) {
  // skip "DIR_OMP_"
  return IntelOpenMPDirectivesAndClauses::DirectiveStrings[Id].substr(8);
}

StringRef VPOUtils::getClauseName(int Id) {
  return IntelIntrinsicUtils::getClauseString(Id).substr(9); // skip "QUAL_OMP_"
}

void VPOUtils::initDirectiveAndClauseStringMap() {

  if (!DirectiveIDs.empty()) // All maps are already initialized
    return;

  // Initialize mapping from Directive string to ID.
  // First enum in OMP_DIRECTIVES starts with 0
  for (int Id = 0; Id <= DIR_QUAL_LIST_END; ++Id) {
    StringRef S = IntelIntrinsicUtils::getDirectiveString(Id);
    DirectiveIDs[S] = Id;
  }

  // Initialize mapping from Clause string to ID.
  // First enum in OMP_CLAUSES starts with 0
  for (int Id = 0; Id <= QUAL_LIST_END; ++Id) {
    StringRef S = IntelIntrinsicUtils::getClauseString(Id);
    ClauseIDs[S] = Id;
  }
}

bool VPOUtils::isOpenMPDirective(StringRef DirFullName) {
  return VPOUtils::DirectiveIDs.count(DirFullName);
}

bool VPOUtils::isOpenMPClause(StringRef ClauseFullName) {
  return VPOUtils::ClauseIDs.count(ClauseFullName);
}

int VPOUtils::getDirectiveID(StringRef DirFullName) {
  // DEBUG(dbgs() << "DirFullName 1" << DirFullName << "\n" );
  assert(VPOUtils::isOpenMPDirective(DirFullName) && 
         "Directive string not found");
  return VPOUtils::DirectiveIDs[DirFullName];
}

int VPOUtils::getClauseID(StringRef ClauseFullName) {
  assert(VPOUtils::isOpenMPClause(ClauseFullName) && 
         "Clause string not found");
  return VPOUtils::ClauseIDs[ClauseFullName];
}

bool VPOUtils::isBeginDirective(StringRef DirString) {
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isBeginDirective(DirID);
}

bool VPOUtils::isBeginDirective(int DirID) {
  switch(DirID) {
  case DIR_OMP_PARALLEL:
  case DIR_OMP_PARALLEL_LOOP:
  case DIR_OMP_LOOP_SIMD:
  case DIR_OMP_PARALLEL_LOOP_SIMD:
  case DIR_OMP_SECTION:
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

bool VPOUtils::isEndDirective(StringRef DirString) {
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isEndDirective(DirID);
}

bool VPOUtils::isEndDirective(int DirID) {
  switch(DirID) {
  case DIR_OMP_END_PARALLEL:
  case DIR_OMP_END_PARALLEL_LOOP:
  case DIR_OMP_END_LOOP_SIMD:
  case DIR_OMP_END_PARALLEL_LOOP_SIMD:
  case DIR_OMP_END_SECTION:
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

bool VPOUtils::isBeginOrEndDirective(StringRef DirString) {
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isBeginOrEndDirective(DirID);
}

bool VPOUtils::isBeginOrEndDirective(int DirID) {
  return VPOUtils::isBeginDirective(DirID) ||
         VPOUtils::isEndDirective(DirID);
}

bool VPOUtils::isSoloDirective(StringRef DirString) {
  int DirID = VPOUtils::getDirectiveID(DirString);
  return VPOUtils::isSoloDirective(DirID);
}

bool VPOUtils::isSoloDirective(int DirID) {
  switch(DirID) {
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

