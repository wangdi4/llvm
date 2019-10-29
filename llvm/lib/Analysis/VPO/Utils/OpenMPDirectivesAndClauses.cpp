#if INTEL_COLLAB
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

ClauseSpecifier::ClauseSpecifier(StringRef Name)
    : FullName(Name), IsArraySection(false), IsByRef(false), IsNonPod(false),
      IsUnsigned(false), IsConditional(false), IsScheduleMonotonic(false),
      IsScheduleNonmonotonic(false), IsScheduleSimd(false),
      IsMapAggrHead(false), IsMapAggr(false) {
  StringRef Base;  // BaseName
  StringRef Mod;   // Modifier

  // Split Name into the BaseName and Modifier substrings
  SmallVector<StringRef, 2> SubString;
  Name.split(SubString, ":");
  int NumSubStrings = SubString.size();
  assert((NumSubStrings==1 || NumSubStrings==2) &&
         "Unexpected number of substrings in a clause specifier");

  // Save the BaseName and Modifier substrings
  Base = SubString[0];
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
    llvm_unreachable("String is not an OpenMP clause name");

  // If Modifier exists, then split it into its component substrings, and
  // update ClauseSpecifier's properties accordingly
  if (NumSubStrings == 2) {
    SmallVector<StringRef, 2> ModSubString;
    Mod.split(ModSubString, ".");
    unsigned NumberOfModifierStrings = ModSubString.size();

    if (VPOAnalysisUtils::isScheduleClause(getId())) {
      for (unsigned i=0; i < NumberOfModifierStrings; i++) {
        if (ModSubString[i] == "MONOTONIC")
          setIsScheduleMonotonic();
        else if (ModSubString[i] == "NONMONOTONIC")
          setIsScheduleNonmonotonic();
        else if (ModSubString[i] == "SIMD")
          setIsScheduleSimd();
        else
          llvm_unreachable("Unknown modifier string for the SCHEDULE clause");
      }
    } else
      for (unsigned i=0; i < NumberOfModifierStrings; i++) {
        if (ModSubString[i] == "ARRSECT")
          setIsArraySection();
        else if (ModSubString[i] == "BYREF")
          setIsByRef();
        else if (ModSubString[i] == "NONPOD")
          setIsNonPod();
        else if (ModSubString[i] == "UNSIGNED")     // for reduction clause
          setIsUnsigned();
        else if (ModSubString[i] == "CONDITIONAL")  // for lastprivate clause
          setIsConditional();
        else if (ModSubString[i] == "AGGRHEAD") // map chain head
          setIsMapAggrHead();
        else if (ModSubString[i] == "AGGR") // map chain (not head)
          setIsMapAggr();
        else
          llvm_unreachable("Unknown modifier string for clause");
      }
  }

  LLVM_DEBUG(dbgs() << "=== ClauseInfo: " << Base);
  LLVM_DEBUG(dbgs() << "  ID: " << getId());
  LLVM_DEBUG(dbgs() << "  Modifier: \"" << Mod << "\"");
  LLVM_DEBUG(dbgs() << "  ArrSect: " << getIsArraySection());
  LLVM_DEBUG(dbgs() << "  ByRef: " << getIsByRef());
  LLVM_DEBUG(dbgs() << "  NonPod: " << getIsNonPod());
  LLVM_DEBUG(dbgs() << "  Monotonic: " << getIsScheduleMonotonic());
  LLVM_DEBUG(dbgs() << "  Nonmonotonic: " << getIsScheduleNonmonotonic());
  LLVM_DEBUG(dbgs() << "  Simd: " << getIsScheduleSimd());
  LLVM_DEBUG(dbgs() << "\n");
}

StringRef VPOAnalysisUtils::getDirectiveString(Instruction *I){
  StringRef DirString;  // ctor initializes its data to nullptr
  if (I) {
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
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
  return Directives::ClauseIDs.count(ClauseFullName);
}

int VPOAnalysisUtils::getDirectiveID(StringRef DirFullName) {
  if (Directives::DirectiveIDs.count(DirFullName))
    return Directives::DirectiveIDs[DirFullName];
  return -1;
}

int VPOAnalysisUtils::getDirectiveID(Instruction *I) {
  StringRef DirString = VPOAnalysisUtils::getDirectiveString(I);
  return VPOAnalysisUtils::getDirectiveID(DirString);
}

int VPOAnalysisUtils::getClauseID(StringRef ClauseFullName) {
  if (Directives::ClauseIDs.count(ClauseFullName))
    return Directives::ClauseIDs[ClauseFullName];
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
  case DIR_OMP_CRITICAL:
  case DIR_OMP_ATOMIC:
  case DIR_OMP_ORDERED:
  case DIR_OMP_SIMD:
  case DIR_OMP_TASKGROUP:
  case DIR_OMP_TASKLOOP:
  case DIR_OMP_TARGET:
  case DIR_OMP_TARGET_DATA:
  case DIR_OMP_TARGET_VARIANT_DISPATCH:
  case DIR_OMP_TEAMS:
  case DIR_OMP_DISTRIBUTE:
  case DIR_OMP_DISTRIBUTE_PARLOOP:
  case DIR_OMP_GENERICLOOP:
#if INTEL_CUSTOMIZATION
  case DIR_VPO_AUTO_VEC:
  case DIR_PRAGMA_IVDEP:
#endif // INTEL_CUSTOMIZATION
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isBeginDirective(StringRef DirString) {
  int DirID = VPOAnalysisUtils::getDirectiveID(DirString);
  return VPOAnalysisUtils::isBeginDirective(DirID);
}

bool VPOAnalysisUtils::isBeginDirective(Instruction *I) {
  int DirID = VPOAnalysisUtils::getDirectiveID(I);
  return VPOAnalysisUtils::isBeginDirective(DirID);
}

bool VPOAnalysisUtils::isBeginDirective(BasicBlock *BB) {
  return VPOAnalysisUtils::isBeginDirective(&(BB->front()));
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
  case DIR_OMP_END_CRITICAL:
  case DIR_OMP_END_ATOMIC:
  case DIR_OMP_END_ORDERED:
  case DIR_OMP_END_SIMD:
  case DIR_OMP_END_TASKGROUP:
  case DIR_OMP_END_TASKLOOP:
  case DIR_OMP_END_TARGET:
  case DIR_OMP_END_TARGET_DATA:
  case DIR_OMP_END_TARGET_VARIANT_DISPATCH:
  case DIR_OMP_END_TEAMS:
  case DIR_OMP_END_DISTRIBUTE:
  case DIR_OMP_END_DISTRIBUTE_PARLOOP:
  case DIR_OMP_END_GENERICLOOP:
#if INTEL_CUSTOMIZATION
  case DIR_VPO_END_AUTO_VEC:
  case DIR_PRAGMA_END_IVDEP:
#endif // INTEL_CUSTOMIZATION
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
  case DIR_OMP_TEAMS:
    return DIR_OMP_END_TEAMS;
  case DIR_OMP_DISTRIBUTE:
    return DIR_OMP_END_DISTRIBUTE;
  case DIR_OMP_DISTRIBUTE_PARLOOP:
    return DIR_OMP_END_DISTRIBUTE_PARLOOP;

#if INTEL_CUSTOMIZATION
  // Non-OpenMP Directives
  case DIR_VPO_AUTO_VEC:
    return DIR_VPO_END_AUTO_VEC;
  case DIR_PRAGMA_IVDEP:
    return DIR_PRAGMA_END_IVDEP;
#endif // INTEL_CUSTOMIZATION

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

bool VPOAnalysisUtils::isDependClause(int ClauseID) {
  switch(ClauseID) {
    case QUAL_OMP_DEPEND_IN:
    case QUAL_OMP_DEPEND_INOUT:
    case QUAL_OMP_DEPEND_OUT:
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
    return true;
  }
  return false;
}

bool VPOAnalysisUtils::isMapClause(int ClauseID) {
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
    case QUAL_OMP_DEFAULTMAP_TOFROM_SCALAR:
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
    case QUAL_OMP_BIND_TEAMS:
    case QUAL_OMP_BIND_PARALLEL:
    case QUAL_OMP_BIND_THREAD:
    case QUAL_OMP_ORDER_CONCURRENT:
      return 0;

    // Clauses that take one argument
    case QUAL_OMP_SIMDLEN:
    case QUAL_OMP_SAFELEN:
    case QUAL_OMP_COLLAPSE:
    case QUAL_OMP_IF:
    case QUAL_OMP_NAME:
    case QUAL_OMP_NUM_THREADS:
    case QUAL_OMP_FINAL:
    case QUAL_OMP_GRAINSIZE:
    case QUAL_OMP_NUM_TASKS:
    case QUAL_OMP_PRIORITY:
    case QUAL_OMP_NUM_TEAMS:
    case QUAL_OMP_THREAD_LIMIT:
    case QUAL_OMP_DEVICE:
    case QUAL_OMP_OFFLOAD_ENTRY_IDX:
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

#endif // INTEL_COLLAB
