#if INTEL_COLLAB
//==-- IntrinsicUtils.cpp - Utilities for VPO related intrinsics -*- C++ -*-==//
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
/// This file provides a set of utilities for VPO-based intrinsic function
/// calls. E.g., directives that mark the beginning and end of SIMD and
/// parallel regions.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"

#define DEBUG_TYPE "VPOIntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOAnalysisUtils::isRegionDirective(Intrinsic::ID Id) {
  return  Id == Intrinsic::directive_region_entry ||
          Id == Intrinsic::directive_region_exit;
}

bool VPOAnalysisUtils::isRegionDirective(Instruction *I) {
  if (I) {
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
    if (Call) {
      Intrinsic::ID Id = Call->getIntrinsicID();
      return VPOAnalysisUtils::isRegionDirective(Id);
    }
  }
  return false;
}

StringRef VPOAnalysisUtils::getRegionDirectiveString(Instruction *I) {
  StringRef DirString;  // ctor initializes its data to nullptr
  if (VPOAnalysisUtils::isRegionDirective(I)) {
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
    if (Call->getNumOperandBundles() > 0) {
      // First operand bundle has the directive name
      OperandBundleUse BU = Call->getOperandBundleAt(0);
      DirString = BU.getTagName();
    }
  }
  return DirString;
}

int VPOAnalysisUtils::getRegionDirectiveID(Instruction *I) {
  StringRef DirString = VPOAnalysisUtils::getRegionDirectiveString(I);
  return VPOAnalysisUtils::getDirectiveID(DirString);
}

bool VPOAnalysisUtils::isIntelDirective(Instruction *I, bool doClauses) {
  if (I) {
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
    if (Call) {
      Intrinsic::ID Id = Call->getIntrinsicID();
      // Is it an intel_directive?
      if (VPOAnalysisUtils::isIntelDirective(Id) ||
          (doClauses && VPOAnalysisUtils::isIntelClause(Id)))
        return true;

      // Is it a directive_region_entry/exit?
      StringRef DirString = VPOAnalysisUtils::getRegionDirectiveString(I);
      return VPOAnalysisUtils::isOpenMPDirective(DirString);
    }
  }
  return false;
}

bool VPOAnalysisUtils::isIntelDirective(Intrinsic::ID Id) {
  return Id == Intrinsic::intel_directive;
}

bool VPOAnalysisUtils::isIntelClause(Intrinsic::ID Id) {
  return (Id == Intrinsic::intel_directive_qual ||
          Id == Intrinsic::intel_directive_qual_opnd ||
          Id == Intrinsic::intel_directive_qual_opndlist);
}

bool VPOAnalysisUtils::isIntelDirectiveOrClause(Instruction *I) {
  return VPOAnalysisUtils::isIntelDirective(I, true);
}

bool VPOAnalysisUtils::isIntelDirectiveOrClause(Intrinsic::ID Id) {
  return isIntelDirective(Id) || isIntelClause(Id);
}

StringRef
VPOAnalysisUtils::getDirectiveMetadataString(const IntrinsicInst *Call) {
  assert(isIntelDirectiveOrClause(Call->getIntrinsicID()) &&
         "Expected a call to an llvm.intel.directive* intrinsic");

  MDString *OperandMDStr = nullptr;
  Value *Operand = Call->getArgOperand(0);
  assert(isa<MetadataAsValue>(Operand) && "Call operand 0 is not metadata.");
  MetadataAsValue *OperandMDVal = cast<MetadataAsValue>(Operand);
  Metadata *MD = OperandMDVal->getMetadata();

  if (isa<MDNode>(MD)) {
    MDNode *OperandNode = cast<MDNode>(MD);
    Metadata *OperandNodeMD = OperandNode->getOperand(0);
    assert(isa<MDString>(OperandNodeMD) && "Metadata is not an MD string.");
    OperandMDStr = cast<MDString>(OperandNodeMD);
  } else if (isa<MDString>(MD))
    OperandMDStr = cast<MDString>(MD);
  else
    llvm_unreachable("Unexpected Call operand.");

  StringRef DirectiveStr = OperandMDStr->getString();

  return DirectiveStr;
}

StringRef
VPOAnalysisUtils::getScheduleModifierMDString(const IntrinsicInst *Call) {
  assert(isIntelDirectiveOrClause(Call->getIntrinsicID()) &&
         "Expected a call to an llvm.intel.directive* intrinsic");

  // The llvm intrin reprsenting a schedule clause has 3 arguments:
  // arg 0: Metadata string "QUAL.OMP.SCHEDULE.<SchduleKind>"
  // arg 1: Metadata string for schedule modifiers
  // arg 2: Value for the chunk size
  //
  // This util returns the string from arg 1.

  Value *Operand = Call->getArgOperand(1);

  return VPOAnalysisUtils::getScheduleModifierMDString(Operand);
}

/// \brief 'Modifier' has the MDString for the schedule modifier.
/// Extract and return the string. E.g.: "MODIFIERNONE" and "SIMD.MONOTONIC".
StringRef VPOAnalysisUtils::getScheduleModifierMDString(Value *Modifier) {

  MDString *OperandMDStr = nullptr;
  assert(isa<MetadataAsValue>(Modifier) && "Modifier is not metadata.");
  MetadataAsValue *OperandMDVal = cast<MetadataAsValue>(Modifier);
  Metadata *MD = OperandMDVal->getMetadata();

  if (isa<MDNode>(MD)) {
    MDNode *OperandNode = cast<MDNode>(MD);
    Metadata *OperandNodeMD = OperandNode->getOperand(0);
    assert(isa<MDString>(OperandNodeMD) && "Metadata is not an MD string.");
    OperandMDStr = cast<MDString>(OperandNodeMD);
  } else if (isa<MDString>(MD))
    OperandMDStr = cast<MDString>(MD);
  else
    llvm_unreachable("Unexpected schedule modifier Value.");

  StringRef ModifierStr = OperandMDStr->getString();
  return ModifierStr;
}


/// \brief Auxiliary function for verifyBB to print out messages.
static void verifyBBError(StringRef Msg, BasicBlock &BB, bool DoAssert) {
  LLVM_DEBUG(dbgs() << "\n\n === verifyBB error: " << Msg << ": \n\n"
                    << BB << "\n");
  if (DoAssert)
    llvm_unreachable(Msg.data());
}

/// \brief Auxiliary function for verifyBB to verify BBlocks with directives
/// represented with directive.region.entry/exit intrinsics.
static bool verifyBBWithRegionDirective(BasicBlock &BB, bool DoAssert) {
  // We already verified that the first instruction is a
  // directive.region.entry/exit intrinsic

  // 1. Verify that the BBlock only has FirstInstr and a terminator.
  Instruction *FirstInstr = &(BB.front());
  Instruction *T = BB.getTerminator();
  if (FirstInstr -> getNextNode() != T) {
    StringRef Msg = "The directive.region.entry/exit intrinsic must be the "
                    "only instruction besides the terminator in the BBlock";
    verifyBBError(Msg, BB, DoAssert);
    return false;
  }

  // 2. If FirstInstr is a BEGIN directive, verify that
  //    2.1 It has one and only one use
  //    2.2 The use is an END directive
  //    2.3 The END directive matches with the BEGIN directive
  int BeginDirID = VPOAnalysisUtils::getDirectiveID(FirstInstr);
  if (VPOAnalysisUtils::isBeginDirective(BeginDirID) ||
      VPOAnalysisUtils::isStandAloneBeginDirective(BeginDirID)) {

    // 2.1 Exactly one use?
    auto Users = FirstInstr -> users();
    int NumUsers = std::distance(Users.begin(), Users.end());
    if (NumUsers != 1) {
      StringRef Msg = "The directive.region.entry call must have exactly ONE "
                    "use, which is its corresponding directive.region.exit";
      verifyBBError(Msg, BB, DoAssert);
      return false;
    }

    // 2.2 Is the use an END directive?
    Instruction *EndDir = dyn_cast<Instruction>(*(Users.begin()));
    int EndDirID = VPOAnalysisUtils::getDirectiveID(EndDir);
    if (!VPOAnalysisUtils::isEndDirective(EndDirID) &&
        !VPOAnalysisUtils::isStandAloneEndDirective(EndDirID)) {
      StringRef Msg = "The use of the directive.region.entry call must be an "
                      "END directive";
      verifyBBError(Msg, BB, DoAssert);
      return false;
    }

    // 2.3 Does the END directive match with the BEGIN directive?
    if (VPOAnalysisUtils::getMatchingEndDirective(BeginDirID) != EndDirID) {
      StringRef Msg = "The directive.region.entry call must have a matching "
                      "END directive";
      verifyBBError(Msg, BB, DoAssert);
      return false;
    }
  }
  return true;
}

/// \brief Auxiliary function for verifyBB to verify BBlocks with directives
/// represented with llvm.intel.directive intrinsics.
static bool verifyBBWithIntelDirective(BasicBlock &BB, bool DoAssert) {
  // We already verified that the first instruction is an llvm.intel.directive
  // intrinsic.
  Instruction *FirstInstr = &(BB.front());

  // 1. Verify that FirstInstr is not DIR.QUAL.LIST.END. This can happen as
  //    a result of bad CFG restructuring, because DIR.QUAL.LIST.END is
  //    categorized as a directive (ie, isIntelDirective() is true).
  if (VPOAnalysisUtils::isListEndDirective(FirstInstr)) {
    StringRef Msg = "A BBlock cannot begin with a DIR.QUAL.LIST.END marker";
    verifyBBError(Msg, BB, DoAssert);
    return false;
  }

  // 2. Verify that the instruction before the terminator is the
  //    DIR.QUAL.LIST.END marker.
  Instruction *T = BB.getTerminator();
  Instruction *ListEnd = T -> getPrevNode();
  if (!VPOAnalysisUtils::isListEndDirective(ListEnd)) {
    StringRef Msg = "A BBlock with an OpenMP directive must close with a "
		    "DIR.QUAL.LIST.END marker";
    verifyBBError(Msg, BB, DoAssert);
    return false;
  }

  // 3. Verify that there are no other llvm.intel.directive intrinsics in
  //    the BBlock between FirstInstr and the DIR.QUAL.LIST.END.
  //    This situation could be the result of merging two EntryBBs of
  //    perfectly nested constructs.
  for (Instruction *I = FirstInstr -> getNextNode(); I && I != ListEnd;
		    I = I -> getNextNode()) {
    if (VPOAnalysisUtils::isIntelDirective(I)) {
      StringRef Msg = "More than one OpenMP directive found in BBlock";
      verifyBBError(Msg, BB, DoAssert);
      return false;
    }
  }

  return true;
}

/// \brief Auxiliary function for verifyBB to verify BBlocks that should not
/// contain any OpenMP directives.
static bool verifyBBWithoutDirective(BasicBlock &BB, bool DoAssert) {
  // We already verified that the first instruction is not a directive.
  // Verify that there are no directives in the rest of the BBlock.
  // The search below includes the first instruction because we want to verify
  // that it is not a clause (an llvm.intel.directive.qual* intrinsic), which
  // would be an error.
  for (Instruction &I : BB) {
    if (VPOAnalysisUtils::isIntelDirectiveOrClause(&I)) {
      StringRef Msg = "An OpenMP directive must be the first instruction of "
                      "the BBlock";
      verifyBBError(Msg, BB, DoAssert);
      return false;
    }
  }
  return true;
}


/// \brief Verify if a BBlock breaks any rules regarding how OpenMP directives
/// are represented. Return true if no errors are found. If an error is found:
///   - If DEBUG is on, it prints a message to dbgs().
///   - If DoAssert is true, it aborts after finding an error.
///   - If DoAssert if false, return false.
bool VPOAnalysisUtils::verifyBB(BasicBlock &BB, bool DoAssert) {
  Instruction *FirstInstr = &(BB.front());

  if (isIntelDirective(FirstInstr)) {
    if (isRegionDirective(FirstInstr))
      return verifyBBWithRegionDirective(BB, DoAssert);
    else
      return verifyBBWithIntelDirective(BB, DoAssert);
  }

  return verifyBBWithoutDirective(BB, DoAssert);
}

/// \brief \returns true if the function has the string attribute
/// "may-have-openmp-directive" set to "true" (Moved here from GeneralUtils.cpp
/// to make it part of Analysis library and avoid circular dependence)
bool VPOAnalysisUtils::mayHaveOpenmpDirective(Function &F) {
  return F.getFnAttribute("may-have-openmp-directive").getValueAsString()
                                                                  == "true";
}

/// \brief \returns !mayHaveOpenmpDirective(F). This is mainly used in
/// passes required by OpenMP that would otherwise be skipped at -O0. (Moved
/// here from GeneralUtils.cpp to make it part of Analysis library and avoid
/// circular dependence)
bool VPOAnalysisUtils::skipFunctionForOpenmp(Function &F) {
  return !mayHaveOpenmpDirective(F);
}
#endif // INTEL_COLLAB
