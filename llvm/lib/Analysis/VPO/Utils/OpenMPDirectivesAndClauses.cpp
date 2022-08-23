#if INTEL_COLLAB
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
//==- OpenMPDirectivesAndClauses.cpp - Utils for OpenMP directives/clauses -==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// VPO Analysis utilities for handling OpenMP directives and clauses.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Support/Debug.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Analysis/Directives.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"

#define DEBUG_TYPE "vpo-utils"

using namespace llvm;
using namespace llvm::vpo;

namespace {
// Returns a character separating the clause name from the modifiers.
char getClauseSeparator() {
  return ':';
}

// Returns a character separating the modifiers from each other.
char getModifiersSeparator() {
  return '.';
}
} // end anonymous namespace

ClauseSpecifier::ClauseSpecifier(StringRef Name)
    : FullName(Name), IsArraySection(false), IsByRef(false), IsNonPod(false),
#if INTEL_CUSTOMIZATION
      IsF90DopeVector(false), IsF90NonPod(false), IsCptr(false),
      IsWILocal(false), IsAllocatable(false),
#endif // INTEL_CUSTOMIZATION
      IsVarLen(false), IsAggregate(false), IsPointer(false),
      IsFunctionPointer(false), IsPointerToPointer(false), IsScalar(false),
      IsAlways(false), IsClose(false), IsPresent(false), IsUnsigned(false),
      IsComplex(false), IsConditional(false), IsScheduleMonotonic(false),
      IsScheduleNonmonotonic(false), IsScheduleSimd(false),
      IsMapAggrHead(false), IsMapAggr(false), IsMapChainLink(false),
      IsIV(false), IsInitTarget(false), IsInitTargetSync(false),
      IsInitPrefer(false), IsTask(false), IsInscan(false), IsTyped(false) {
  StringRef Base;  // BaseName
  StringRef Mod;   // Modifier
  // Split Name into the BaseName and Modifier substrings
  SmallVector<StringRef, 2> SubString;
  Name.split(SubString, getClauseSeparator());
  int NumSubStrings = SubString.size();
  assert((NumSubStrings==1 || NumSubStrings==2) &&
         "Unexpected number of substrings in a clause specifier");

  // Save the BaseName and Modifier substrings
  Base = SubString[0];
  LLVM_DEBUG(dbgs() << "\nClauseSpecifier: base name = " << Base << "\n");
  setBaseName(Base);
  if (NumSubStrings == 2) {
    Mod = SubString[1];
    setModifier(Mod);
  }

#if 1
  // Temporary hack to accept "QUAL.OMP.LINEAR.VAL" and treat it as
  // "QUAL.OMP.LINEAR". Eventually, Clang will be fixed to never emit
  // "QUAL.OMP.LINEAR.VAL".
  if (Base == "QUAL.OMP.LINEAR.VAL")
    setId(QUAL_OMP_LINEAR);
  else
#endif
  // Get the clause ID corresponding to BaseName
  if (VPOAnalysisUtils::isOpenMPClause(Base))
    setId(VPOAnalysisUtils::getClauseID(Base));
  else
    llvm_unreachable("Base name is not an OpenMP clause name");

  // If Modifier exists, then split it into its component substrings, and
  // update ClauseSpecifier's properties accordingly
  if (NumSubStrings == 2) {
    SmallVector<StringRef, 2> ModSubString;
    Mod.split(ModSubString, getModifiersSeparator());
    unsigned NumberOfModifierStrings = ModSubString.size();

    if (getId() == QUAL_OMP_INIT)
      for (unsigned i = 0; i < NumberOfModifierStrings; i++) {
        LLVM_DEBUG(dbgs() << "ClauseSpecifier: modifier = " << ModSubString[i]
              << "\n");
        if (ModSubString[i] == "TARGET")
          setIsInitTarget();
        else if (ModSubString[i] == "TARGETSYNC")
          setIsInitTargetSync();
        else if (ModSubString[i] == "PREFER")
          setIsInitPrefer();
        else
          llvm_unreachable("Unknown modifier string for the Init clause");
      }
    else if (VPOAnalysisUtils::isScheduleClause(getId()))
      for (unsigned i=0; i < NumberOfModifierStrings; i++) {
        LLVM_DEBUG(dbgs() << "ClauseSpecifier: modifier = " << ModSubString[i]
                          << "\n");
        if (ModSubString[i] == "MONOTONIC")
          setIsScheduleMonotonic();
        else if (ModSubString[i] == "NONMONOTONIC")
          setIsScheduleNonmonotonic();
        else if (ModSubString[i] == "SIMD")
          setIsScheduleSimd();
        else
          llvm_unreachable("Unknown modifier string for the SCHEDULE clause");
      }
    else if (VPOAnalysisUtils::isDefaultmapClause(getId())) {
      assert(NumberOfModifierStrings <= 1 &&
             "DEFAULTMAP cannot specify more than one variable category");
      for (unsigned i=0; i < NumberOfModifierStrings; i++) {
        LLVM_DEBUG(dbgs() << "ClauseSpecifier: modifier = " << ModSubString[i]
                          << "\n");
        if (ModSubString[i] == "AGGREGATE")
          setIsAggregate();
#if INTEL_CUSTOMIZATION
        else if (ModSubString[i] == "ALLOCATABLE")
          setIsAllocatable();
#endif // INTEL_CUSTOMIZATION
        else if (ModSubString[i] == "POINTER")
          setIsPointer();
        else if (ModSubString[i] == "SCALAR")
          setIsScalar();
        else
          llvm_unreachable(
              "Unknown variable category for the DEFAULTMAP clause");
      }
    } else
      for (unsigned i=0; i < NumberOfModifierStrings; i++) {
        LLVM_DEBUG(dbgs() << "ClauseSpecifier: modifier = " << ModSubString[i]
                          << "\n");
        if (ModSubString[i] == "ALWAYS")           // map-type modifier
          setIsAlways();
        else if (ModSubString[i] == "CLOSE")       // map-type modifier
          setIsClose();
        else if (ModSubString[i] == "PRESENT")     // map-type modifier
          setIsPresent();
        else if (ModSubString[i] == "ARRSECT")
          setIsArraySection();
        else if (ModSubString[i] == "BYREF")
          setIsByRef();
        else if (ModSubString[i] == "TYPED")
          setIsTyped();
#if INTEL_CUSTOMIZATION
        else if (ModSubString[i] == "F90_DV")
          setIsF90DopeVector();
        else if (ModSubString[i] == "F90_NONPOD")
          setIsF90NonPod();
        else if (ModSubString[i] == "CPTR")
          setIsCptr();
        else if (ModSubString[i] == "WILOCAL")
          setIsWILocal();
#endif // INTEL_CUSTOMIZATION
        else if (ModSubString[i] == "NONPOD")
          setIsNonPod();
        else if (ModSubString[i] == "VARLEN")
          setIsVarLen();
        else if (ModSubString[i] == "UNSIGNED")    // for reduction clause
          setIsUnsigned();
        else if (ModSubString[i] == "CMPLX")       // for reduction clause
          setIsComplex();
        else if (ModSubString[i] == "TASK")        // for reduction clause
          setIsTask();
        else if (ModSubString[i] == "INSCAN")      // for reduction clause
          setIsInscan();
        else if (ModSubString[i] == "CONDITIONAL") // for lastprivate clause
          setIsConditional();
        else if (ModSubString[i] == "AGGRHEAD")    // map chain head
          setIsMapAggrHead();
        else if (ModSubString[i] == "AGGR")        // map chain (not head)
          setIsMapAggr();
        else if (ModSubString[i] == "CHAIN")       // map chain (not head)
          setIsMapChainLink();
        else if (ModSubString[i] == "IV")          // for linear clause
          setIsIV();
        else if (ModSubString[i] == "PTR_TO_PTR") // For operands like "int *x"
          setIsPointerToPointer(); // (i32**) on linear, is/use_device_ptr etc.
        else if (ModSubString[i] == "FPTR") // map chain whose base is a
          setIsFunctionPointer();           // function pointer
        else
          llvm_unreachable("Unknown modifier string for clause");
      }
  }
}

StringRef VPOAnalysisUtils::getDirectiveString(const Instruction *I) {
  StringRef DirString;  // ctor initializes its data to nullptr
  if (I) {
    const IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
    if (Call) {
      DirString = VPOAnalysisUtils::getRegionDirectiveString(I);
    }
  }
  return DirString;
}

StringRef VPOAnalysisUtils::getDirectiveString(int Id) {
  assert(Directives::DirectiveStrings.count(Id) &&
         "Can't find a string for directive id");
  return Directives::DirectiveStrings[Id];
}

StringRef VPOAnalysisUtils::getClauseString(int Id) {
  assert(Directives::ClauseStrings.count(Id) &&
         "Can't find a string for clause id");
  return Directives::ClauseStrings[Id];
}

std::string VPOAnalysisUtils::getTypedClauseString(int Id) {
  return VPOAnalysisUtils::getClauseString(Id).str() + ":TYPED";
}

StringRef VPOAnalysisUtils::getOmpDirectiveName(int Id) {
  // skip "DIR_OMP_"
  return VPOAnalysisUtils::getDirectiveString(Id).substr(8);
}

StringRef VPOAnalysisUtils::getOmpClauseName(int Id) {
  // Handle special cases first: REDUCTION, DEPEND, MAP:
  if (VPOAnalysisUtils::isDependClause(Id))
    return "DEPEND";
  if (VPOAnalysisUtils::isMapClause(Id))
    return "MAP";
  if (VPOAnalysisUtils::isReductionClause(Id))
    return "REDUCTION";
  if (VPOAnalysisUtils::isScheduleClause(Id))
    return "SCHEDULE";

  // Regular cases: just skip "QUAL_OMP_"
  return VPOAnalysisUtils::getClauseString(Id).substr(9);
}

StringRef VPOAnalysisUtils::getReductionOpName(int Id) {
  assert (VPOAnalysisUtils::isReductionClause(Id) &&
          "Expected a QUAL_OMP_REDUCTION_<OP> clause");

  // skip "QUAL_OMP_REDUCTION_"
  return VPOAnalysisUtils::getClauseString(Id).substr(19);
}

bool VPOAnalysisUtils::isOpenMPDirective(Instruction *I) {
  StringRef DirString = VPOAnalysisUtils::getRegionDirectiveString(I);
  return VPOAnalysisUtils::isOpenMPDirective(DirString);
}

bool VPOAnalysisUtils::isOpenMPDirective(StringRef DirFullName) {
  return Directives::DirectiveIDs.count(DirFullName);
}

bool VPOAnalysisUtils::isOpenMPClause(StringRef ClauseFullName) {
  size_t SeparatorPos = ClauseFullName.find(getClauseSeparator());
  StringRef BaseName = ClauseFullName.take_front(SeparatorPos);

  return Directives::ClauseIDs.count(BaseName);
}

int VPOAnalysisUtils::getDirectiveID(StringRef DirFullName) {
  if (Directives::DirectiveIDs.count(DirFullName))
    return Directives::DirectiveIDs[DirFullName];
  return -1;
}

int VPOAnalysisUtils::getDirectiveID(const Instruction *I) {
  StringRef DirString = VPOAnalysisUtils::getDirectiveString(I);
  return VPOAnalysisUtils::getDirectiveID(DirString);
}

int VPOAnalysisUtils::getClauseID(StringRef ClauseFullName) {
  size_t SeparatorPos = ClauseFullName.find(getClauseSeparator());
  StringRef BaseName = ClauseFullName.take_front(SeparatorPos);

  if (Directives::ClauseIDs.count(BaseName))
    return Directives::ClauseIDs[BaseName];
  return -1;
}

bool VPOAnalysisUtils::isBeginDirective(int DirID) {
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
  case DIR_OMP_MASKED:
  case DIR_OMP_CRITICAL:
  case DIR_OMP_ATOMIC:
  case DIR_OMP_ORDERED:
  case DIR_OMP_SIMD:
  case DIR_OMP_TASKGROUP:
  case DIR_OMP_TASKLOOP:
  case DIR_OMP_TARGET:
  case DIR_OMP_TARGET_DATA:
  case DIR_OMP_TARGET_VARIANT_DISPATCH:
  case DIR_OMP_DISPATCH:
  case DIR_OMP_TEAMS:
  case DIR_OMP_DISTRIBUTE:
  case DIR_OMP_DISTRIBUTE_PARLOOP:
  case DIR_OMP_GENERICLOOP:
  case DIR_OMP_SCOPE:
  case DIR_OMP_TILE:
#if INTEL_CUSTOMIZATION
  case DIR_VPO_AUTO_VEC:
  case DIR_PRAGMA_IVDEP:
  case DIR_PRAGMA_BLOCK_LOOP:
  case DIR_PRAGMA_DISTRIBUTE_POINT:
#endif // INTEL_CUSTOMIZATION
  case DIR_VPO_GUARD_MEM_MOTION:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isBeginDirective(StringRef DirString) {
  int DirID = VPOAnalysisUtils::getDirectiveID(DirString);
  return VPOAnalysisUtils::isBeginDirective(DirID);
}

bool VPOAnalysisUtils::isBeginDirective(const Instruction *I) {
  int DirID = VPOAnalysisUtils::getDirectiveID(I);
  return VPOAnalysisUtils::isBeginDirective(DirID);
}

bool VPOAnalysisUtils::isBeginDirective(BasicBlock *BB) {
  return VPOAnalysisUtils::isBeginDirective(&(BB->front()));
}

bool VPOAnalysisUtils::isBeginLoopDirective(int DirID) {
  switch(DirID) {
  case DIR_OMP_LOOP:
  case DIR_OMP_PARALLEL_LOOP:
  case DIR_OMP_SIMD:
  case DIR_OMP_TASKLOOP:
  case DIR_OMP_DISTRIBUTE:
  case DIR_OMP_DISTRIBUTE_PARLOOP:
  case DIR_OMP_GENERICLOOP:
  case DIR_OMP_TILE:
  case DIR_PRAGMA_BLOCK_LOOP:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isBeginLoopDirective(StringRef DirString) {
  int DirID = VPOAnalysisUtils::getDirectiveID(DirString);
  return VPOAnalysisUtils::isBeginLoopDirective(DirID);
}

bool VPOAnalysisUtils::isBeginLoopDirective(Instruction *I) {
  int DirID = VPOAnalysisUtils::getDirectiveID(I);
  return VPOAnalysisUtils::isBeginLoopDirective(DirID);
}

bool VPOAnalysisUtils::isEndDirective(int DirID) {
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
  case DIR_OMP_END_MASKED:
  case DIR_OMP_END_CRITICAL:
  case DIR_OMP_END_ATOMIC:
  case DIR_OMP_END_ORDERED:
  case DIR_OMP_END_SIMD:
  case DIR_OMP_END_TASKGROUP:
  case DIR_OMP_END_TASKLOOP:
  case DIR_OMP_END_TARGET:
  case DIR_OMP_END_TARGET_DATA:
  case DIR_OMP_END_TARGET_VARIANT_DISPATCH:
  case DIR_OMP_END_DISPATCH:
  case DIR_OMP_END_TEAMS:
  case DIR_OMP_END_DISTRIBUTE:
  case DIR_OMP_END_DISTRIBUTE_PARLOOP:
  case DIR_OMP_END_GENERICLOOP:
  case DIR_OMP_END_SCOPE:
  case DIR_OMP_END_TILE:
  case DIR_OMP_END_SCAN:
#if INTEL_CUSTOMIZATION
  case DIR_VPO_END_AUTO_VEC:
  case DIR_PRAGMA_END_IVDEP:
  case DIR_PRAGMA_END_BLOCK_LOOP:
  case DIR_PRAGMA_END_DISTRIBUTE_POINT:
#endif // INTEL_CUSTOMIZATION
  case DIR_VPO_END_GUARD_MEM_MOTION:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isEndDirective(StringRef DirString) {
  int DirID = VPOAnalysisUtils::getDirectiveID(DirString);
  return VPOAnalysisUtils::isEndDirective(DirID);
}

bool VPOAnalysisUtils::isEndDirective(Instruction *I) {
  int DirID = VPOAnalysisUtils::getDirectiveID(I);
  return VPOAnalysisUtils::isEndDirective(DirID);
}

bool VPOAnalysisUtils::isEndDirective(BasicBlock *BB) {
  return VPOAnalysisUtils::isEndDirective(&(BB->front()));
}

bool VPOAnalysisUtils::isEndLoopDirective(int DirID) {
  switch(DirID) {
  case DIR_OMP_END_LOOP:
  case DIR_OMP_END_PARALLEL_LOOP:
  case DIR_OMP_END_SIMD:
  case DIR_OMP_END_TASKLOOP:
  case DIR_OMP_END_DISTRIBUTE:
  case DIR_OMP_END_DISTRIBUTE_PARLOOP:
  case DIR_OMP_END_GENERICLOOP:
  case DIR_OMP_END_TILE:
  case DIR_PRAGMA_END_BLOCK_LOOP:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isEndLoopDirective(StringRef DirString) {
  int DirID = VPOAnalysisUtils::getDirectiveID(DirString);
  return VPOAnalysisUtils::isEndLoopDirective(DirID);
}

bool VPOAnalysisUtils::isEndLoopDirective(Instruction *I) {
  int DirID = VPOAnalysisUtils::getDirectiveID(I);
  return VPOAnalysisUtils::isEndLoopDirective(DirID);
}

bool VPOAnalysisUtils::isBeginOrEndDirective(int DirID) {
  return VPOAnalysisUtils::isBeginDirective(DirID) ||
         VPOAnalysisUtils::isEndDirective(DirID);
}

bool VPOAnalysisUtils::isBeginOrEndDirective(StringRef DirString) {
  int DirID = VPOAnalysisUtils::getDirectiveID(DirString);
  return VPOAnalysisUtils::isBeginOrEndDirective(DirID);
}

bool VPOAnalysisUtils::isBeginOrEndDirective(Instruction *I) {
  int DirID = VPOAnalysisUtils::getDirectiveID(I);
  return VPOAnalysisUtils::isBeginOrEndDirective(DirID);
}

bool VPOAnalysisUtils::isBeginOrEndDirective(BasicBlock *BB) {
  return VPOAnalysisUtils::isBeginOrEndDirective(&(BB->front()));
}

bool VPOAnalysisUtils::isStandAloneBeginDirective(int DirID) {
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
  case DIR_OMP_INTEROP:
  case DIR_OMP_PREFETCH:
  case DIR_OMP_SCAN:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isStandAloneBeginDirective(StringRef DirString) {
  int DirID = VPOAnalysisUtils::getDirectiveID(DirString);
  return VPOAnalysisUtils::isStandAloneBeginDirective(DirID);
}

bool VPOAnalysisUtils::isStandAloneBeginDirective(Instruction *I) {
  int DirID = VPOAnalysisUtils::getDirectiveID(I);
  return VPOAnalysisUtils::isStandAloneBeginDirective(DirID);
}

bool VPOAnalysisUtils::isStandAloneBeginDirective(BasicBlock *BB) {
  return VPOAnalysisUtils::isStandAloneBeginDirective(&(BB->front()));
}

bool VPOAnalysisUtils::isStandAloneEndDirective(int DirID) {
  switch(DirID) {
  case DIR_OMP_END_BARRIER:
  case DIR_OMP_END_TASKWAIT:
  case DIR_OMP_END_TASKYIELD:
  case DIR_OMP_END_FLUSH:
  case DIR_OMP_END_SECTION:
  case DIR_OMP_END_TARGET_ENTER_DATA:
  case DIR_OMP_END_TARGET_EXIT_DATA:
  case DIR_OMP_END_TARGET_UPDATE:
  case DIR_OMP_END_CANCEL:
  case DIR_OMP_END_CANCELLATION_POINT:
  case DIR_OMP_END_INTEROP:
  case DIR_OMP_END_PREFETCH:
  case DIR_OMP_END_SCAN:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isStandAloneEndDirective(StringRef DirString) {
  int DirID = VPOAnalysisUtils::getDirectiveID(DirString);
  return VPOAnalysisUtils::isStandAloneEndDirective(DirID);
}

bool VPOAnalysisUtils::isStandAloneEndDirective(Instruction *I) {
  int DirID = VPOAnalysisUtils::getDirectiveID(I);
  return VPOAnalysisUtils::isStandAloneEndDirective(DirID);
}

bool VPOAnalysisUtils::isStandAloneEndDirective(BasicBlock *BB) {
  return VPOAnalysisUtils::isStandAloneEndDirective(&(BB->front()));
}

Instruction * VPOAnalysisUtils::getEndRegionDir(Instruction *BeginDir) {
  assert(VPOAnalysisUtils::isRegionDirective(BeginDir) &&
         "getEndRegionDir: expected BeginDir to be a REGION directive");
  assert((VPOAnalysisUtils::isBeginDirective(BeginDir) ||
          VPOAnalysisUtils::isStandAloneBeginDirective(BeginDir)) &&
         "getEndRegionDir: expected BeginDir to be a BEGIN directive");
  assert(BeginDir->getNumUses() == 1 &&
         "getEndRegionDir: there must be exactly 1 use of the BEGIN dir");
  User *U = *(BeginDir->user_begin());
  Instruction *EndDir = dyn_cast<Instruction>(U);
  assert(EndDir && (VPOAnalysisUtils::isEndDirective(EndDir) ||
                    VPOAnalysisUtils::isStandAloneEndDirective(EndDir)) &&
           "getEndRegionDir: the use is not an END directive");
  return EndDir;
}

BasicBlock * VPOAnalysisUtils::getEndRegionDirBB(Instruction *BeginDir) {
  Instruction *EndDir = VPOAnalysisUtils::getEndRegionDir(BeginDir);
  BasicBlock  *EndBB  = EndDir->getParent();
  return EndBB;
}

int VPOAnalysisUtils::getMatchingEndDirective(int DirID) {
  switch(DirID) {
  case DIR_OMP_PARALLEL:
    return DIR_OMP_END_PARALLEL;
  case DIR_OMP_GENERICLOOP:
    return DIR_OMP_END_GENERICLOOP;
  case DIR_OMP_LOOP:
    return DIR_OMP_END_LOOP;
  case DIR_OMP_PARALLEL_LOOP:
    return DIR_OMP_END_PARALLEL_LOOP;
  case DIR_OMP_SECTIONS:
    return DIR_OMP_END_SECTIONS;
  case DIR_OMP_PARALLEL_SECTIONS:
    return DIR_OMP_END_PARALLEL_SECTIONS;
  case DIR_OMP_WORKSHARE:
    return DIR_OMP_END_WORKSHARE;
  case DIR_OMP_PARALLEL_WORKSHARE:
    return DIR_OMP_END_PARALLEL_WORKSHARE;
  case DIR_OMP_SINGLE:
    return DIR_OMP_END_SINGLE;
  case DIR_OMP_TASK:
    return DIR_OMP_END_TASK;
  case DIR_OMP_MASTER:
    return DIR_OMP_END_MASTER;
  case DIR_OMP_MASKED:
    return DIR_OMP_END_MASKED;
  case DIR_OMP_CRITICAL:
    return DIR_OMP_END_CRITICAL;
  case DIR_OMP_ATOMIC:
    return DIR_OMP_END_ATOMIC;
  case DIR_OMP_ORDERED:
    return DIR_OMP_END_ORDERED;
  case DIR_OMP_SIMD:
    return DIR_OMP_END_SIMD;
  case DIR_OMP_TASKGROUP:
    return DIR_OMP_END_TASKGROUP;
  case DIR_OMP_TASKLOOP:
    return DIR_OMP_END_TASKLOOP;
  case DIR_OMP_TARGET:
    return DIR_OMP_END_TARGET;
  case DIR_OMP_TARGET_DATA:
    return DIR_OMP_END_TARGET_DATA;
  case DIR_OMP_TARGET_VARIANT_DISPATCH:
    return DIR_OMP_END_TARGET_VARIANT_DISPATCH;
  case DIR_OMP_DISPATCH:
    return DIR_OMP_END_DISPATCH;
  case DIR_OMP_TEAMS:
    return DIR_OMP_END_TEAMS;
  case DIR_OMP_DISTRIBUTE:
    return DIR_OMP_END_DISTRIBUTE;
  case DIR_OMP_DISTRIBUTE_PARLOOP:
    return DIR_OMP_END_DISTRIBUTE_PARLOOP;
  case DIR_OMP_SCOPE:
      return DIR_OMP_END_SCOPE;
  case DIR_OMP_TILE:
      return DIR_OMP_END_TILE;

#if INTEL_CUSTOMIZATION
  // Non-OpenMP Directives
  case DIR_VPO_AUTO_VEC:
    return DIR_VPO_END_AUTO_VEC;
  case DIR_PRAGMA_IVDEP:
    return DIR_PRAGMA_END_IVDEP;
  case DIR_PRAGMA_BLOCK_LOOP:
    return DIR_PRAGMA_END_BLOCK_LOOP;
  case DIR_PRAGMA_DISTRIBUTE_POINT:
    return DIR_PRAGMA_END_DISTRIBUTE_POINT;
#endif // INTEL_CUSTOMIZATION

  case DIR_VPO_GUARD_MEM_MOTION:
    return DIR_VPO_END_GUARD_MEM_MOTION;

  // StandAlone Directives
  case DIR_OMP_BARRIER:
    return DIR_OMP_END_BARRIER;
  case DIR_OMP_TASKWAIT:
    return DIR_OMP_END_TASKWAIT;
  case DIR_OMP_TASKYIELD:
    return DIR_OMP_END_TASKYIELD;
  case DIR_OMP_FLUSH:
    return DIR_OMP_END_FLUSH;
  case DIR_OMP_SECTION:
    return DIR_OMP_END_SECTION;
  case DIR_OMP_TARGET_ENTER_DATA:
    return DIR_OMP_END_TARGET_ENTER_DATA;
  case DIR_OMP_TARGET_EXIT_DATA:
    return DIR_OMP_END_TARGET_EXIT_DATA;
  case DIR_OMP_TARGET_UPDATE:
    return DIR_OMP_END_TARGET_UPDATE;
  case DIR_OMP_CANCEL:
    return DIR_OMP_END_CANCEL;
  case DIR_OMP_CANCELLATION_POINT:
    return DIR_OMP_END_CANCELLATION_POINT;
  case DIR_OMP_INTEROP:
      return DIR_OMP_END_INTEROP;
  case DIR_OMP_PREFETCH:
      return DIR_OMP_END_PREFETCH;
  case DIR_OMP_SCAN:
      return DIR_OMP_END_SCAN;
  }
  return -1;
}

bool VPOAnalysisUtils::isBeginDirectiveOfRegionsNeedingOutlining(
    StringRef DirString) {
  return VPOAnalysisUtils::isBeginDirectiveOfRegionsNeedingOutlining(
      VPOAnalysisUtils::getDirectiveID(DirString));
}

bool VPOAnalysisUtils::isBeginDirectiveOfRegionsNeedingOutlining(int DirID) {
  switch (DirID) {
  case DIR_OMP_PARALLEL:
  case DIR_OMP_PARALLEL_LOOP:
  case DIR_OMP_PARALLEL_SECTIONS:
  case DIR_OMP_PARALLEL_WORKSHARE:
  case DIR_OMP_DISTRIBUTE_PARLOOP:
  case DIR_OMP_TASK:
  case DIR_OMP_TASKLOOP:
  case DIR_OMP_TEAMS:
  case DIR_OMP_TARGET:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isEndDirectiveOfRegionsNeedingOutlining(
    StringRef DirString) {
  return VPOAnalysisUtils::isEndDirectiveOfRegionsNeedingOutlining(
      VPOAnalysisUtils::getDirectiveID(DirString));
}

bool VPOAnalysisUtils::isEndDirectiveOfRegionsNeedingOutlining(int DirID) {
  switch (DirID) {
  case DIR_OMP_END_PARALLEL:
  case DIR_OMP_END_PARALLEL_LOOP:
  case DIR_OMP_END_PARALLEL_SECTIONS:
  case DIR_OMP_END_PARALLEL_WORKSHARE:
  case DIR_OMP_END_DISTRIBUTE_PARLOOP:
  case DIR_OMP_END_TASK:
  case DIR_OMP_END_TASKLOOP:
  case DIR_OMP_END_TEAMS:
  case DIR_OMP_END_TARGET:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isDependClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_DEPEND_IN:
    case QUAL_OMP_DEPEND_INOUT:
    case QUAL_OMP_DEPEND_OUT:
    case QUAL_OMP_DEPARRAY:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isInReductionClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_INREDUCTION_ADD:
    case QUAL_OMP_INREDUCTION_SUB:
    case QUAL_OMP_INREDUCTION_MUL:
    case QUAL_OMP_INREDUCTION_AND:
    case QUAL_OMP_INREDUCTION_OR:
    case QUAL_OMP_INREDUCTION_BXOR:
    case QUAL_OMP_INREDUCTION_BAND:
    case QUAL_OMP_INREDUCTION_BOR:
#if INTEL_CUSTOMIZATION
    case QUAL_OMP_INREDUCTION_EQV:
    case QUAL_OMP_INREDUCTION_NEQV:
#endif // INTEL_CUSTOMIZATION
    case QUAL_OMP_INREDUCTION_MAX:
    case QUAL_OMP_INREDUCTION_MIN:
    case QUAL_OMP_INREDUCTION_UDR:
      return true;
  }
  return false;
}

bool VPOAnalysisUtils::isReductionClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_REDUCTION_ADD:
    case QUAL_OMP_REDUCTION_SUB:
    case QUAL_OMP_REDUCTION_MUL:
    case QUAL_OMP_REDUCTION_AND:
    case QUAL_OMP_REDUCTION_OR:
    case QUAL_OMP_REDUCTION_BXOR:
    case QUAL_OMP_REDUCTION_BAND:
    case QUAL_OMP_REDUCTION_BOR:
#if INTEL_CUSTOMIZATION
    case QUAL_OMP_REDUCTION_EQV:
    case QUAL_OMP_REDUCTION_NEQV:
#endif // INTEL_CUSTOMIZATION
    case QUAL_OMP_REDUCTION_MAX:
    case QUAL_OMP_REDUCTION_MIN:
    case QUAL_OMP_REDUCTION_UDR:
      return true;
  }
  return false;
}

bool VPOAnalysisUtils::isScheduleClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_DIST_SCHEDULE_STATIC:
    case QUAL_OMP_SCHEDULE_AUTO:
    case QUAL_OMP_SCHEDULE_DYNAMIC:
    case QUAL_OMP_SCHEDULE_GUIDED:
    case QUAL_OMP_SCHEDULE_RUNTIME:
    case QUAL_OMP_SCHEDULE_STATIC:
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    case QUAL_OMP_SA_SCHEDULE_STATIC:
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isMapClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_TO:
    case QUAL_OMP_FROM:
    case QUAL_OMP_MAP:
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

bool VPOAnalysisUtils::isDefaultmapClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_DEFAULTMAP_ALLOC:
    case QUAL_OMP_DEFAULTMAP_TO:
    case QUAL_OMP_DEFAULTMAP_FROM:
    case QUAL_OMP_DEFAULTMAP_TOFROM:
    case QUAL_OMP_DEFAULTMAP_FIRSTPRIVATE:
    case QUAL_OMP_DEFAULTMAP_NONE:
    case QUAL_OMP_DEFAULTMAP_DEFAULT:
    case QUAL_OMP_DEFAULTMAP_PRESENT:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isBindClause(int ClauseID) {
  switch (ClauseID) {
    case QUAL_OMP_BIND_PARALLEL:
    case QUAL_OMP_BIND_THREAD:
    case QUAL_OMP_BIND_TEAMS:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isBindClause(StringRef ClauseFullName) {
  int ClauseID = VPOAnalysisUtils::getClauseID(ClauseFullName);
  return isBindClause(ClauseID);
}

/// \brief Return 0, 1, or 2:
///   0 for clauses that take no arguments
///   1 for clauses that take exactly 1 argument
///   2 for anything else (such as list type arguments)
unsigned VPOAnalysisUtils::getClauseType(int ClauseID) {
  switch(ClauseID) {

    // Clauses that take no arguments
    case QUAL_OMP_DEFAULT_NONE:
    case QUAL_OMP_DEFAULT_SHARED:
    case QUAL_OMP_DEFAULT_PRIVATE:
    case QUAL_OMP_DEFAULT_FIRSTPRIVATE:
    case QUAL_OMP_DEFAULTMAP_ALLOC:
    case QUAL_OMP_DEFAULTMAP_TO:
    case QUAL_OMP_DEFAULTMAP_FROM:
    case QUAL_OMP_DEFAULTMAP_TOFROM:
    case QUAL_OMP_DEFAULTMAP_FIRSTPRIVATE:
    case QUAL_OMP_DEFAULTMAP_NONE:
    case QUAL_OMP_DEFAULTMAP_DEFAULT:
    case QUAL_OMP_DEFAULTMAP_PRESENT:
    case QUAL_OMP_NOWAIT:
    case QUAL_OMP_UNTIED:
    case QUAL_OMP_READ_SEQ_CST:
    case QUAL_OMP_READ:
    case QUAL_OMP_WRITE_SEQ_CST:
    case QUAL_OMP_WRITE:
    case QUAL_OMP_UPDATE_SEQ_CST:
    case QUAL_OMP_UPDATE:
    case QUAL_OMP_CAPTURE_SEQ_CST:
    case QUAL_OMP_CAPTURE:
    case QUAL_OMP_MERGEABLE:
    case QUAL_OMP_NOGROUP:
    case QUAL_OMP_PROC_BIND_MASTER:
    case QUAL_OMP_PROC_BIND_CLOSE:
    case QUAL_OMP_PROC_BIND_SPREAD:
    case QUAL_OMP_ORDERED_THREADS:
    case QUAL_OMP_ORDERED_SIMD:
    case QUAL_OMP_CANCEL_PARALLEL:
    case QUAL_OMP_CANCEL_LOOP:
    case QUAL_OMP_CANCEL_SECTIONS:
    case QUAL_OMP_CANCEL_TASKGROUP:
    case QUAL_OMP_TARGET_TASK:
    case QUAL_OMP_IMPLICIT:
    case QUAL_OMP_BIND_TEAMS:
    case QUAL_OMP_BIND_PARALLEL:
    case QUAL_OMP_BIND_THREAD:
    case QUAL_OMP_ORDER_CONCURRENT:
    case QUAL_OMP_OFFLOAD_KNOWN_NDRANGE:
    case QUAL_OMP_OFFLOAD_HAS_TEAMS_REDUCTION:
#if INTEL_CUSTOMIZATION
    case QUAL_EXT_DO_CONCURRENT:
#endif // INTEL_CUSTOMIZATION
      return 0;

    // Clauses that take one argument
    case QUAL_OMP_SIMDLEN:
    case QUAL_OMP_SAFELEN:
    case QUAL_OMP_COLLAPSE:
    case QUAL_OMP_IF:
    case QUAL_OMP_NAME:
    case QUAL_OMP_HINT:
    case QUAL_OMP_NOCONTEXT:
    case QUAL_OMP_NOVARIANTS:
    case QUAL_OMP_NUM_THREADS:
    case QUAL_OMP_FINAL:
    case QUAL_OMP_GRAINSIZE:
    case QUAL_OMP_NUM_TASKS:
    case QUAL_OMP_PRIORITY:
    case QUAL_OMP_DEVICE:
    case QUAL_OMP_FILTER:
    case QUAL_OMP_OFFLOAD_ENTRY_IDX:
    case QUAL_OMP_USE:
    case QUAL_OMP_DESTROY:
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    case QUAL_OMP_SA_NUM_WORKERS:
    case QUAL_OMP_SA_PIPELINE:
#endif // INTEL_FEATURE_CSA
#endif //INTEL_CUSTOMIZATION
      return 1;
  }
  return 2; //everything else
}

// True if the directive supports the private clause.
bool VPOAnalysisUtils::supportsPrivateClause(int DirID) {
  switch (DirID) {
  case DIR_OMP_PARALLEL:
  case DIR_OMP_LOOP:
  case DIR_OMP_PARALLEL_LOOP:
  case DIR_OMP_SECTIONS:
  case DIR_OMP_PARALLEL_SECTIONS:
  case DIR_OMP_PARALLEL_WORKSHARE:
  case DIR_OMP_SINGLE:
  case DIR_OMP_TASK:
  case DIR_OMP_SIMD:
  case DIR_OMP_TASKLOOP:
  case DIR_OMP_TARGET:
  case DIR_OMP_TEAMS:
  case DIR_OMP_DISTRIBUTE:
  case DIR_OMP_DISTRIBUTE_PARLOOP:
  case DIR_OMP_GENERICLOOP:
  case DIR_OMP_SCOPE:
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::supportsPrivateClause(StringRef DirString) {
  return VPOAnalysisUtils::supportsPrivateClause(
      VPOAnalysisUtils::getDirectiveID(DirString));
}

bool VPOAnalysisUtils::supportsPrivateClause(Instruction *I) {
  int DirID = VPOAnalysisUtils::getDirectiveID(I);
  return VPOAnalysisUtils::supportsPrivateClause(DirID);
}

bool VPOAnalysisUtils::supportsPrivateClause(BasicBlock *BB) {
  return VPOAnalysisUtils::supportsPrivateClause(&(BB->front()));
}

/// Returns pointer to directive instruction if this bblock contains known loop
/// begin/end directive. \p BeginDir flag indicates whether to look for begin or
/// end directive.
template <bool BeginDir = true>
static Instruction *getLoopDirective(BasicBlock *BB) {
  for (auto &Inst : *BB) {
    if (BeginDir ? VPOAnalysisUtils::isBeginLoopDirective(&Inst)
                 : VPOAnalysisUtils::isEndLoopDirective(&Inst))
      return &Inst;
  }
  return nullptr;
}

/// Traces a chain of single predecessor/successor bblocks starting from \p BB
/// and looks for loop begin/end directive. Returns the directive instruction
/// pointer. \p BeginDir flag indicates whether to look for begin or
/// end directive and whether to loop upward or downward the chain of BBs.
template <bool BeginDir = true>
static Instruction *findLoopDirective(BasicBlock *BB) {
  for (; BB != nullptr;) {
    if (auto *Inst = getLoopDirective<BeginDir>(BB))
      return Inst;
    BB = BeginDir ? BB->getSinglePredecessor() : BB->getSingleSuccessor();
  }
  return nullptr;
}

/// Returns begin loop directive instruction if it exists.
Instruction *VPOAnalysisUtils::getBeginLoopDirective(const Loop &Lp) {
  return findLoopDirective<true>(Lp.getLoopPreheader());
}

/// Returns end loop directive instruction if it exists.
Instruction *VPOAnalysisUtils::getEndLoopDirective(const Loop &Lp) {
  // TODO: This will not work for multi-exit loops. We need to get the exiting
  // block corresponding to the loop latch.
  return findLoopDirective<false>(Lp.getExitBlock());
}

#endif // INTEL_COLLAB
