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
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

#define DEBUG_TYPE "VPOIntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOUtils::stripDirectives(WRegionNode *WRN) {
  bool success = true;
  BasicBlock *EntryBB = WRN->getEntryBBlock();
  BasicBlock *ExitBB = WRN->getExitBBlock();

  // Under the new region representation:
  //   %1 = call token @llvm.directive.region.entry() [...]
  //     ...
  //   call void @llvm.directive.region.exit(token %1) [...]
  // We have to remove the END dir before the BEGIN dir. If not, when removing
  // the BEGIN it will first remove the END (which is a use of the token
  // defined by the BEGIN intrinsic) and then later stripDirectives(*ExitBB)
  // would return false because there's nothing left in the ExitBB to remove.
  success = success && VPOUtils::stripDirectives(*ExitBB);
  success = success && VPOUtils::stripDirectives(*EntryBB);
  return success;
}

bool VPOUtils::stripDirectives(BasicBlock &BB) {
  SmallVector<Instruction *, 4> IntrinsicsToRemove;

  for (Instruction &I : BB) {
    if (VPOAnalysisUtils::isIntelDirectiveOrClause(&I))
      IntrinsicsToRemove.push_back(&I);
  }

  // Remove the directive intrinsics.
  // SimplifyCFG will remove any blocks that become empty.
  unsigned Idx = 0;
  for (Idx = 0; Idx < IntrinsicsToRemove.size(); ++Idx) {
    Instruction *I = IntrinsicsToRemove[Idx];
    // Under the region representation, the BEGIN directive writes to a token
    // that is used by the matching END directive. Therefore, before removing
    // I, we must first remove all its uses, if any. Failing to do that
    // will result in this assertion: "Uses remain when a value is destroyed!"
    for (User *U : I->users())
      if (Instruction *UI = dyn_cast<Instruction>(U)) {
        UI->eraseFromParent();
      }
    I->eraseFromParent();
  }

  // Returns true if any elimination happens.
  return Idx > 0;
}

bool VPOUtils::stripDirectives(Function &F) {
  bool changed = false;

  for (BasicBlock &BB: F) {
    changed |= stripDirectives(BB);
  }

  return changed;
}

bool VPOUtils::stripPrivateClauses(WRegionNode *WRN) {
  BasicBlock *EntryBB = WRN->getEntryBBlock();
  return VPOUtils::stripPrivateClauses(*EntryBB);
}

bool VPOUtils::stripPrivateClauses(BasicBlock &BB) {
  SmallVector<Instruction *, 4> IntrinsicsToRemove;

  for (Instruction &I : BB) {
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(&I);
    if (Call) {
      Intrinsic::ID Id = Call->getIntrinsicID();
      if (Id == Intrinsic::directive_region_entry) {
        // TODO: add support for this representation
        DEBUG(dbgs() << "** WARNING: stripPrivateClauses() support for the "
                     << "OperandBundle representation will be done later.\n");
      }
      else if (Id == Intrinsic::intel_directive_qual_opndlist) {
        StringRef ClauseString = VPOAnalysisUtils::getDirOrClauseString(Call);
        ClauseSpecifier ClauseInfo(ClauseString);
        int ClauseID = ClauseInfo.getId();
        if (ClauseID == QUAL_OMP_PRIVATE)
          IntrinsicsToRemove.push_back(&I);
      }
    }
  }

  unsigned Idx = 0;
  for (Idx = 0; Idx < IntrinsicsToRemove.size(); ++Idx) {
    Instruction *I = IntrinsicsToRemove[Idx];
    I->eraseFromParent();
  }
  return Idx > 0;
}

CallInst *VPOUtils::createMaskedGatherCall(Value *VecPtr,
                                           IRBuilder<> &Builder,
                                           unsigned Alignment,
                                           Value *Mask,
                                           Value *PassThru) {
  auto NewCallInst = Builder.CreateMaskedGather(VecPtr, Alignment, Mask,
                                                PassThru);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedScatterCall(Value *VecPtr,
                                            Value *VecData,
                                            IRBuilder<> &Builder,
                                            unsigned Alignment,
                                            Value *Mask) {
  auto NewCallInst = Builder.CreateMaskedScatter(VecData, VecPtr, Alignment,
                                                 Mask);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedLoadCall(Value *VecPtr,
                                         IRBuilder<> &Builder,
                                         unsigned Alignment,
                                         Value *Mask,
                                         Value *PassThru) {
  auto NewCallInst = Builder.CreateMaskedLoad(VecPtr, Alignment, Mask,
                                               PassThru);
  return NewCallInst;
}

CallInst *VPOUtils::createMaskedStoreCall(Value *VecPtr,
                                          Value *VecData,
                                          IRBuilder<> &Builder,
                                          unsigned Alignment,
                                          Value *Mask) {
  auto NewCallInst = Builder.CreateMaskedStore(VecData, VecPtr, Alignment,
                                                Mask);
  return NewCallInst;
}
