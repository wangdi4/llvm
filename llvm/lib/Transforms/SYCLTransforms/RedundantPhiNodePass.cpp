//==--- RedundantPhiNodePass.cpp - Remove redundant Phi nodes ------ C++ -*-==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/RedundantPhiNodePass.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"

using namespace llvm;

extern cl::opt<bool> SkipNonBarrierFunction;

#define DEBUG_TYPE "sycl-kernel-redundant-phi-node"

PreservedAnalyses RedundantPhiNode::run(Function &F,
                                        FunctionAnalysisManager &) {
  if (!runImpl(F))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool RedundantPhiNode::runImpl(Function &F) {
  // Skip non-barrier functions to reduce compile time
  if (SkipNonBarrierFunction) {
    BarrierUtils Utils;
    Utils.init(F.getParent());
    CompilationUtils::FuncSet FS = Utils.getAllFunctionsWithSynchronization();
    if (!FS.count(&F)) {
      LLVM_DEBUG(
          dbgs() << "Skip non-barrier function in RedundantPhiNode pass: "
                 << F.getName() << "\n");
      return false;
    }
  }

  SmallVector<Instruction *, 8> InstsToRemove;
  for (auto &BB : F) {
    for (auto &I : BB) {
      PHINode *PhiInst = dyn_cast<PHINode>(&I);
      if (!PhiInst) {
        // No more PhiNode in this BasicBlock
        break;
      }
      if (PhiInst->getNumIncomingValues() == 1) {
        PhiInst->replaceAllUsesWith(PhiInst->getIncomingValue(0));
        InstsToRemove.push_back(PhiInst);
        continue;
      }
      // Found a PHINode
      assert(PhiInst->getNumIncomingValues() == 2 &&
             "assume PhiCanon pass handled such PHINode!");
      if (PhiInst->getIncomingValue(0) == PhiInst->getIncomingValue(1)) {
        // It is a Redundant PHINode add it to container for handling
        PhiInst->replaceAllUsesWith(PhiInst->getIncomingValue(0));
        InstsToRemove.push_back(PhiInst);
      }
    }
  }
  // Remove all redundant Phi nodes
  for (Instruction *I : InstsToRemove) {
    assert(I && "remove instruction container contains non instruction!");
    I->eraseFromParent();
  }
  // The module was changed only if there were instructions to remove
  return !InstsToRemove.empty();
}
