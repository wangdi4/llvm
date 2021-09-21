//===--------------- Intel_VPOParoptTargetInline.cpp ----------------------===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements a module pass which adds alwaysinline attribute to all
/// functions called from OpenMP target regions.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptTargetInline.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-target-inline"

static bool forceInlineToTarget(Module &M) {
  SmallSetVector<Function *, 8> ForceInline;

  // Walk instructions in [BI, BE) range looking for function calls. BI and BE
  // must be in the same basic block.
  auto FindCalleesInBlock = [&ForceInline](BasicBlock::const_iterator BI,
                                           BasicBlock::const_iterator BE) {
    for (auto BT = BI->getParent()->end(); BI != BE && BI != BT; ++BI)
      if (const auto *CB = dyn_cast<CallBase>(BI))
        if (Function *F = CB->getCalledFunction())
          if (!F->isDeclaration())
            ForceInline.insert(F);
  };

  for (Function &F : M)
    for (Instruction &I : instructions(F)) {
      if (VPOAnalysisUtils::getDirectiveID(&I) != DIR_OMP_TARGET)
        continue;

      Instruction *Entry = &I;
      BasicBlock *EntryBB = Entry->getParent();
      Instruction *Exit = VPOAnalysisUtils::getEndRegionDir(&I);
      BasicBlock *ExitBB = Exit->getParent();

      // If entry/exit pair is in the same block, just check instructions
      // between entry and exit.
      if (EntryBB == ExitBB) {
        FindCalleesInBlock(++Entry->getIterator(), Exit->getIterator());
        continue;
      }

      // Othewrwise first find callees in entry and exit blocks.
      FindCalleesInBlock(++Entry->getIterator(), EntryBB->end());
      FindCalleesInBlock(ExitBB->begin(), Exit->getIterator());

      // Then collect BBSet for this region and check it for calls excluding
      // entry/exit blocks.
      SmallVector<BasicBlock *, 8> BBSet;
      GeneralUtils::collectBBSet(EntryBB, ExitBB, BBSet);
      BBSet.pop_back();
      for (BasicBlock *BB : drop_begin(BBSet))
        FindCalleesInBlock(BB->begin(), BB->end());
    }

  bool Changed = false;
  while (!ForceInline.empty()) {
    Function *F = ForceInline.pop_back_val();

    // Add alwaysinline attribute to the function unless it has noinline.
    if (!F->hasFnAttribute(Attribute::NoInline) &&
        !F->hasFnAttribute(Attribute::AlwaysInline)) {
      F->addFnAttr(Attribute::AlwaysInline);
      Changed = true;
    }

    // And recursively find function calls in this function.
    for (BasicBlock &BB : *F)
      FindCalleesInBlock(BB.begin(), BB.end());
  }

  return Changed;
}

PreservedAnalyses VPOParoptTargetInlinePass::run(Module &M,
                                                 ModuleAnalysisManager &AM) {
  forceInlineToTarget(M);
  return PreservedAnalyses::all();
}

namespace {

struct VPOParoptTargetInline : public ModulePass {
  static char ID;
  VPOParoptTargetInline() : ModulePass(ID) {
    initializeVPOParoptTargetInlinePass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override { return forceInlineToTarget(M); }
};

} // namespace

char VPOParoptTargetInline::ID = 0;
INITIALIZE_PASS(VPOParoptTargetInline, DEBUG_TYPE,
                "Force inlining to target regions", false, false)

ModulePass *llvm::createVPOParoptTargetInlinePass() {
  return new VPOParoptTargetInline;
}
