//===- CSACacheLocalizer.cpp - Local cache ID assignment pass --------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------===//
// This pass locates loops bracketed wtih the local cache intrinsics
// Intrinsic::csa_local_cache_region_{begin,end}. It does a post-order
// traversal of the loops and assgins a unique local cache ID to the
// intrinsics and the memory instructions in the loop. The ID will be
// stored in the metadata of an instruction.
//
// This pass is intended to be run right before CSAMemopOrderingPass
// so that the IDs can be used in that pass to determine orderings.
//
//===----------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsCSA.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Metadata.h"
#include "llvm/CodeGen/MachineMemOperand.h" // To get CSA_LOCAL_CACHE_METADATA_KEY.

#include <string>
#include <cassert>

#define DEBUG_TYPE "csa-cache-localizer"
#define PASS_NAME "CSA: Localize Memops to Cache"

using namespace llvm;

namespace {

class CSACacheLocalizer : public FunctionPass {
public:
  static char ID;

  CSACacheLocalizer() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
  }

  bool runOnFunction(Function &) override;

  StringRef getPassName() const override {
    return PASS_NAME;
  }

private:
  const char *Key;
  DominatorTree *DT;
  PostDominatorTree *PDT;

  // This vector contains the csa_local_cache_region_begin intrinsics
  // as we discover them.
  SmallVector<IntrinsicInst *, 4> LCBS;

  static int NextID;

  bool isLocalCacheBegin(Instruction &) const;
  bool isLocalCacheEnd(Instruction &) const;

  bool isMarked(Instruction *) const;
  void setMarker(Instruction *);
  MDNode *getMarker(Instruction *) const;

  // Get the immediate dominating LCB intrinsic in LCBS of the given LCB.
  IntrinsicInst *getDLCB(IntrinsicInst *);
};

} // namespace

char CSACacheLocalizer::ID = 0;
int CSACacheLocalizer::NextID = 1;

INITIALIZE_PASS(CSACacheLocalizer, DEBUG_TYPE, PASS_NAME, false, false)

Pass *llvm::createCSACacheLocalizerPass() {
  return new CSACacheLocalizer();
}

bool CSACacheLocalizer::runOnFunction(Function &F) {

  Key = CSA_LOCAL_CACHE_METADATA_KEY;
  DT  = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (isLocalCacheBegin(I)) {
        auto LCB = cast<IntrinsicInst>(&I);
        if (isMarked(LCB)) continue;

        setMarker(LCB);
        Value* ID = ConstantInt::get(LCB->getContext(), llvm::APInt(32, NextID, false));
        // Update the second argument with the ID.
        LCB->setArgOperand(1, ID);

        auto DLCB = getDLCB(LCB);

        // If Marker is not nullptr, this cache region is nested in another
        // cache region. We get the enclosing LCB's marker to identify
        // the instructions whose marker needs updating.
        auto Marker = DLCB ? getMarker(DLCB) : nullptr;

        LCBS.push_back(LCB);

        for (auto U : LCB->users()) {
          auto LCE = cast<IntrinsicInst>(U);
          setMarker(LCE);
          // Update the second argument with the ID.
          LCE->setArgOperand(1, ID);
          for (auto &BB : F) {
            if (DT->dominates(LCB, &BB) && PDT->dominates(LCE->getParent(), &BB)) {
              for (auto &I : BB) {
                if (I.mayReadOrWriteMemory() &&
                    !isLocalCacheBegin(I) && !isLocalCacheEnd(I) &&
                    DT->dominates(LCB, &I) && PDT->dominates(LCE, &I)) {
                  // If the instruction is not marked, or if it's already marked
                  // by the enclosing local cache region we identified earlier, we
                  // mark it as in the current local cache region.
                  if (!isMarked(&I) || getMarker(&I) == Marker) {
                    setMarker(&I);
                  }
                }
              }
            }
          }
        }
        NextID++;
      }
    }
  }

  // NextID starts at 1 and counts up.
  return (NextID > 1);
}

bool CSACacheLocalizer::isLocalCacheBegin(Instruction &Inst) const {
  IntrinsicInst *const Intr = dyn_cast<IntrinsicInst>(&Inst);
  return (Intr and
          Intr->getIntrinsicID() == Intrinsic::csa_local_cache_region_begin);

}

bool CSACacheLocalizer::isLocalCacheEnd(Instruction &Inst) const {
  IntrinsicInst *const Intr = dyn_cast<IntrinsicInst>(&Inst);
  return (Intr and
          Intr->getIntrinsicID() == Intrinsic::csa_local_cache_region_end);

}

bool CSACacheLocalizer::isMarked(Instruction *Inst) const {
  return (Inst->hasMetadata(Key));
}

void CSACacheLocalizer::setMarker(Instruction *Inst) {
  auto &C = Inst->getContext();
  ConstantInt* ID = ConstantInt::get(C, llvm::APInt(32, NextID, false));
  MDNode* N = MDNode::get(C, ConstantAsMetadata::get(ID));
  Inst->setMetadata(Key, N);
}

MDNode *CSACacheLocalizer::getMarker(Instruction *Inst) const {
  return Inst->getMetadata(Key);
}


IntrinsicInst *CSACacheLocalizer::getDLCB(IntrinsicInst *Intr) {
  IntrinsicInst *DLCB = nullptr;
  for (auto LCB : LCBS) {
    if (DT->dominates(LCB, Intr) && (!DLCB || DT->dominates(DLCB, LCB))) {
      DLCB = LCB;
    }
  }

  return DLCB;
}
