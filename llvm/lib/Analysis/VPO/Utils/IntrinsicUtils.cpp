#if INTEL_COLLAB
//==-- IntrinsicUtils.cpp - Utilities for VPO related intrinsics -*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// VPO Analysis utilities for the llvm.directive.region.entry/exit intrinsics.
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
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"

#define DEBUG_TYPE "VPOIntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOAnalysisUtils::isRegionDirective(Intrinsic::ID Id) {
  return  Id == Intrinsic::directive_region_entry ||
          Id == Intrinsic::directive_region_exit;
}

bool VPOAnalysisUtils::isRegionDirective(const Instruction *I, bool *IsEntry) {
  if (I) {
    const IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
    if (Call) {
      Intrinsic::ID Id = Call->getIntrinsicID();
      if (IsEntry)
        *IsEntry = Id == Intrinsic::directive_region_entry;
      return VPOAnalysisUtils::isRegionDirective(Id);
    }
  }
  return false;
}

StringRef VPOAnalysisUtils::getRegionDirectiveString(const Instruction *I,
                                                     bool *IsEntry) {
  StringRef DirString;  // ctor initializes its data to nullptr
  if (VPOAnalysisUtils::isRegionDirective(I, IsEntry)) {
    const IntrinsicInst *Call = cast<IntrinsicInst>(I);
    if (Call->getNumOperandBundles() > 0) {
      // First operand bundle has the directive name
      OperandBundleUse BU = Call->getOperandBundleAt(0);
      DirString = BU.getTagName();
    }
  }
  return DirString;
}

int VPOAnalysisUtils::getRegionDirectiveID(const Instruction *I,
                                           bool *IsEntry) {
  StringRef DirString = VPOAnalysisUtils::getRegionDirectiveString(I, IsEntry);
  return VPOAnalysisUtils::getDirectiveID(DirString);
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
static bool verifyBBWithDirective(BasicBlock &BB, bool DoAssert) {
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

/// \brief Auxiliary function for verifyBB to verify BBlocks that should not
/// contain any OpenMP directives.
static bool verifyBBWithoutDirective(BasicBlock &BB, bool DoAssert) {
  // We already verified that the first instruction is not a directive.
  // Verify that there are no directives in the rest of the BBlock.
  for (BasicBlock::iterator I = ++BB.begin(), E = BB.end(); I != E; ++I) {
    if (VPOAnalysisUtils::isOpenMPDirective(&*I)) {
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

  if (isOpenMPDirective(FirstInstr)) {
    return verifyBBWithDirective(BB, DoAssert);
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

/// Returns true if \p V is an operand to a "QUAL.OMP.JUMP.TO.END.IF" clause.
///
/// An example of such \p V is `%t`:
/// \code
///   %t = alloca i1
///   %1 = begin_region[... "QUAL.OMP.JUMP.TO.END.IF"(i1* %t)]
///   %t.load = load volatile i1, i1* %t
///   %cmp = icmp ne i1 %.load, false
///   br i1 %cmp, %END, %CONTINUE
///
///   CONTINUE:
///   ...
///
///   END:
///   end_region(%1)
/// \endcode
bool VPOAnalysisUtils::seenOnJumpToEndIfClause(Value *V) {
  // A "JUMP.TO.END.IF" operand is an alloca of type I1.
  auto *AI = dyn_cast_or_null<AllocaInst>(V);
  if (!AI)
    return false;

  Type *AllocTy = AI->getAllocatedType();
  if (!AllocTy->isIntegerTy(1))
    return false;

  // Find a directive that may potentially have V as a jump-to-end-if operand.
  auto findFirstPotentialDirectiveUserOfV = [&V]() -> CallInst * {
    for (User *U : V->users()) {
      if (!isa<CallInst>(U) && !isa<LoadInst>(U) && !isa<CastInst>(U))
        // Jump-to-end-if operands cannot have uses other than these.
        return nullptr;

      if (CallInst *CI = dyn_cast<CallInst>(U))
        if (VPOAnalysisUtils::isOpenMPDirective(CI) &&
            CI->getNumOperandBundles() > 0)
          return CI;
    }
    return nullptr;
  };

  CallInst *CI = findFirstPotentialDirectiveUserOfV();
  if (!CI)
    return false;

  StringRef JumpToEndIfClauseString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_JUMP_TO_END_IF);

  for (unsigned I = 0; I < CI->getNumOperandBundles(); ++I) {
    OperandBundleUse BU = CI->getOperandBundleAt(I);
    if (BU.getTagName() != JumpToEndIfClauseString)
      continue;

    assert(BU.Inputs.size() == 1 && "Expected a single operand in a "
                                    "'QUAL.OMP.JUMP.TO.END.IF' bundle.");
    Value *JumpToEndIfOpnd = BU.Inputs[0];
    if (JumpToEndIfOpnd == V)
      return true;
  }
  return false;
}
#endif // INTEL_COLLAB
