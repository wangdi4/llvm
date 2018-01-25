//===- CSAStreamingMemoryPrep.cpp - IR prepass for streaming ---*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass re-expresses memory loads in loops in a way more amenable to
// optimization in the streaming memory conversion pass.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "csa-streamem"
#define PASS_DESC "Prepare IR for streaming memory conversion"

static cl::opt<bool> EnableStreamingMemoryPrep(
    "csa-enable-streammem-prep", cl::Hidden,
    cl::desc("CSA Specific: enable streaming memory prepare pass"));

namespace llvm {
  void initializeCSAStreamingMemoryPrepPass(PassRegistry&);
}

namespace {
struct CSAStreamingMemoryPrep : public FunctionPass {
  static char ID;

  explicit CSAStreamingMemoryPrep() : FunctionPass(ID) {
    initializeCSAStreamingMemoryPrepPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;
  bool runOnLoop(Loop *L);

  ScalarEvolution *SE;
};
} // namespace

char CSAStreamingMemoryPrep::ID = 0;
INITIALIZE_PASS_BEGIN(CSAStreamingMemoryPrep, "csa-streammem-prep", PASS_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(CSAStreamingMemoryPrep, "csa-streammem-prep", PASS_DESC, false,
                    false)

Pass *llvm::createCSAStreamingMemoryPrepPass() { return new CSAStreamingMemoryPrep(); }

bool CSAStreamingMemoryPrep::runOnFunction(Function &F) {
  bool Changed = false;

  if (skipFunction(F))
    return Changed;

  if (!EnableStreamingMemoryPrep)
    return Changed;

  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

  for (auto &L : LI->getLoopsInPreorder()) {
    Changed |= runOnLoop(L);
  }

  return Changed;
}

bool CSAStreamingMemoryPrep::runOnLoop(Loop *L) {
  bool Changed = false;

  SmallPtrSet<Instruction *, 8> loop_indexes;
  for (auto &BB : L->blocks()) {
    for (auto &I : *BB) {
      Value *pointer;
      if (auto SI = dyn_cast<StoreInst>(&I))
        pointer = SI->getPointerOperand();
      else if (auto LI = dyn_cast<LoadInst>(&I))
        pointer = LI->getPointerOperand();
      else
        continue;

      if (auto I = dyn_cast<Instruction>(pointer))
        loop_indexes.insert(I);
    }
  }

  // Expand the SCEV indices in non-canonical mode. The use of non-canonical
  // mode causes the expander to create PHI nodes rather than relying on the
  // canonical induction variable. This causes the backend to emit multiple
  // STRIDE patterns, rather than math off of the SEQ* index variable.
  SCEVExpander expand(*SE, SE->getDataLayout(), "streammemprep");
  expand.disableCanonicalMode();
  for (auto index : loop_indexes) {
    // Only consider affine expressions that are used in this loop.
    const SCEV *se_index = SE->getSCEV(index);
    if (SE->getLoopDisposition(se_index, L) != ScalarEvolution::LoopComputable)
      continue;
    auto affineSCEV = dyn_cast<SCEVAddRecExpr>(se_index);
    if (!affineSCEV)
      continue;

    expand.setInsertPoint(index);
    Value *expansion = expand.expandCodeFor(se_index, index->getType());
    if (expansion != index) {
      index->replaceAllUsesWith(expansion);
      index->eraseFromParent();
    }
  }

  return Changed;
}
