//===- Intel_HeteroArchOpt.cpp - Hetero Arch (such as ADL) Optimization ---===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass implements loop level multi version optimization for alderlake.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

#define DEBUG_TYPE "hetero-arch-opt"

// Defines preferable loop height to clone. If the max height of loop candidate
// is less than this value, this pass will try to find its ancestor loop with
// max height equals to this value, or top level loop if its max height is less
// than this value. Then this pass will try to clone this ancestor loop.
static cl::opt<uint32_t>
    HeteroArchCloneLoopHeightThreshold("hetero-arch-clone-loop-height",
                                       cl::init(3), cl::ReallyHidden);

namespace {

struct LoopCand {
  // L is innermost loop which contains CandInsts.
  Loop *L;

  // Max height of L.
  unsigned MaxHeight;

  SmallVector<Instruction *, 8> CandInsts;

  LoopCand(Loop *L, unsigned MaxHeight) : L(L), MaxHeight(MaxHeight) {}
};

class HeteroArchOpt : public FunctionPass {
public:
  static char ID;

  HeteroArchOpt() : FunctionPass(ID) {
    initializeHeteroArchOptPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<TargetTransformInfoWrapperPass>();
  }

  bool runOnFunction(Function &F) override;

private:
  bool optLoop();

  // Populate loop candidates which contains flollowing instructions:
  // 1. masked_gather intrinsic that won't be scalarized for alderlake.
  unsigned scanLoopCandidates(Loop *L, SmallVector<LoopCand> &LoopCandidates);

  // Create loop multi version for LoopCandidates in the following steps:
  // 1. Find appropriate ancestor loops of loop candidates to clone.
  // 2. Clone loops and insert cpu detection code in preheader.
  // 3. Optimize candidate instructions in clone loops.
  bool createLoopMultiVersion(SmallVector<LoopCand> &LoopCandidates);

  // Clone loop L if it is simplify and LCSSA form. The clone loop will be
  // inserted after the original loop. Cpu core type detection code will be
  // inserted into preheader to decide which loop should be run.
  bool cloneLoop(Loop *L, ValueToValueMapTy &VMap);

  LoopInfo *LI = nullptr;
  DominatorTree *DT = nullptr;
  TargetTransformInfo *TTI = nullptr;
  Function *CurFn = nullptr;
};

} // end anonymous namespace

char HeteroArchOpt::ID = 0;
char &llvm::HeteroArchOptID = HeteroArchOpt::ID;

INITIALIZE_PASS(HeteroArchOpt, DEBUG_TYPE, "Hetero Arch Optimization", false,
                false)

FunctionPass *llvm::createHeteroArchOptPass() { return new HeteroArchOpt(); }

bool HeteroArchOpt::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  // This opt will bloat code.
  if (F.hasOptSize())
    return false;

  if (!F.hasFnAttribute("target-cpu"))
    return false;

  StringRef CPU = F.getFnAttribute("target-cpu").getValueAsString();
  if (CPU != "alderlake")
    return false;

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  CurFn = &F;

  // Dominator tree and loop info is NOT up to date after this optimization.
  bool Changed = optLoop();
  return Changed;
}

bool HeteroArchOpt::optLoop() {
  SmallVector<LoopCand> LoopCandidates;
  for (auto *Lp : *LI)
    scanLoopCandidates(Lp, LoopCandidates);
  return createLoopMultiVersion(LoopCandidates);
}

unsigned
HeteroArchOpt::scanLoopCandidates(Loop *L,
                                  SmallVector<LoopCand> &LoopCandidates) {
  unsigned MaxHeight = 0;
  for (Loop *SubLoop : *L)
    MaxHeight =
        std::max(scanLoopCandidates(SubLoop, LoopCandidates), MaxHeight);
  MaxHeight++;

  for (BasicBlock *BB : L->blocks()) {
    // Skip BB belonging to subloops.
    if (LI->getLoopFor(BB) != L)
      continue;

    LoopCand LC(L, MaxHeight);
    for (Instruction &I : *BB) {
      IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I);
      if (II && II->getIntrinsicID() == Intrinsic::masked_gather &&
          !TTI->shouldScalarizeMaskedGather(II))
        LC.CandInsts.push_back(II);
    }

    if (LC.CandInsts.size())
      LoopCandidates.push_back(std::move(LC));
  }
  return MaxHeight;
}

bool HeteroArchOpt::createLoopMultiVersion(
    SmallVector<LoopCand> &LoopCandidates) {
  // Group ancestor loops belonging to same top level loop.
  MapVector<Loop *, SmallVector<Loop *>> TopLevelLoop2AncLoops;
  DenseMap<Loop *, SmallDenseSet<LoopCand *, 2>> AncLoop2Cands;
  for (auto &LC : LoopCandidates) {
    // AncLoop is the ancestor of L to be cloned.
    Loop *AncLoop = LC.L;
    if (LC.MaxHeight < HeteroArchCloneLoopHeightThreshold) {
      unsigned RecedeLevel =
          std::min<unsigned>(HeteroArchCloneLoopHeightThreshold - LC.MaxHeight,
                             LC.L->getLoopDepth() - 1);
      while (RecedeLevel--)
        AncLoop = AncLoop->getParentLoop();
    }
    Loop *TopLevelLoop = AncLoop;
    while (Loop *ParentLoop = TopLevelLoop->getParentLoop())
      TopLevelLoop = ParentLoop;
    TopLevelLoop2AncLoops[TopLevelLoop].push_back(AncLoop);

    // At this point, AncLoop2Cands doesn't contain all subloop candidates which
    // is inside the corresponding ancestor loop. It'll be filled later.
    AncLoop2Cands[AncLoop].insert(&LC);
  }

  // Sort it with lower depth comes first.
  llvm::for_each(TopLevelLoop2AncLoops, [](auto &KV) {
    llvm::sort(KV.second, [](Loop *A, Loop *B) {
      return A->getLoopDepth() < B->getLoopDepth();
    });
  });

  // Ancestors loops inside top level loops may interleave with each other. This
  // step remove ancestor loops which are inside another ancestor loop.
  for (auto &KV : TopLevelLoop2AncLoops) {
    SmallVector<Loop *> &AncLoops = KV.second;
    for (unsigned I = 0; I != AncLoops.size(); I++) {
      unsigned J = I + 1;
      Loop *UpperLoop = AncLoops[I];
      for (unsigned K = J, E = AncLoops.size(); K != E; K++) {
        Loop *LowerLoop = AncLoops[K];
        if (UpperLoop->contains(LowerLoop)) {
          // Populate loop candidates of lower loop to upper loop.
          auto &LowerLoopCands = AncLoop2Cands[LowerLoop];
          AncLoop2Cands[UpperLoop].insert(LowerLoopCands.begin(),
                                          LowerLoopCands.end());
          if (UpperLoop != LowerLoop)
            AncLoop2Cands.erase(LowerLoop);

          continue;
        }
        AncLoops[J++] = LowerLoop;
      }
      AncLoops.truncate(J);
    }
  }

  // Clone loops and optimize candidate instructions.
  bool Changed = false;
  ValueToValueMapTy VMap;
  for (auto &KV : TopLevelLoop2AncLoops) {
    for (Loop *AncLoop : KV.second) {
      if (!cloneLoop(AncLoop, VMap))
        continue;

      Changed = true;
      for (LoopCand *LC : AncLoop2Cands[AncLoop]) {
        for (Instruction *I : LC->CandInsts) {
          // Add metadata as scalarize hint for clone gather intrinsics.
          IntrinsicInst *CloneGather = cast<IntrinsicInst>(VMap[I]);
          assert(CloneGather->getIntrinsicID() == Intrinsic::masked_gather);
          CloneGather->setMetadata("hetero.arch.opt.disable.gather",
                                   MDNode::get(CloneGather->getContext(), {}));
          assert(TTI->shouldScalarizeMaskedGather(CloneGather));
        }
      }
    }
  }
  return Changed;
}

bool HeteroArchOpt::cloneLoop(Loop *L, ValueToValueMapTy &VMap) {
  if (!L->isLoopSimplifyForm() || !L->isLCSSAForm(*DT))
    return false;

  // Clone all basicblocks belong to loop L and insert clone one after the
  // original one.
  ArrayRef<BasicBlock *> OrigBBs = L->getBlocks();
  SmallVector<BasicBlock *, 32> CloneBBs;
  auto InsertPoint = (*OrigBBs.rbegin())->getIterator();
  for (auto RI = OrigBBs.rbegin(), End = OrigBBs.rend(); RI != End; RI++) {
    BasicBlock *CloneBB = CloneBasicBlock(*RI, VMap, ".clone");
    CurFn->getBasicBlockList().insertAfter(InsertPoint, CloneBB);
    VMap[*RI] = CloneBB;
    CloneBBs.push_back(CloneBB);
  }
  remapInstructionsInBlocks(CloneBBs, VMap);

  // Insert cpu core type detection code in preheader.
  BasicBlock *Preheader = L->getLoopPreheader();
  assert(Preheader && "Preheader is expected");
  Preheader->getTerminator()->eraseFromParent();
  IRBuilder<> IRB(Preheader);
  CallInst *CoreType =
      IRB.CreateIntrinsic(Intrinsic::x86_intel_fast_cpuid_coretype, None, None);
  CoreType->getCalledFunction()->addFnAttr(CurFn->getFnAttribute("target-cpu"));
  // 20H is intel atom. 40H is intel core.
  Value *IsCore = IRB.CreateICmpEQ(CoreType, IRB.getInt8(0x40));
  // Origin loop for core and clone loop for atom.
  BasicBlock *OrigHeader = L->getHeader();
  BasicBlock *CloneHeader = cast<BasicBlock>(VMap[OrigHeader]);
  IRB.CreateCondBr(IsCore, OrigHeader, CloneHeader);

  // In LCSSA form, all outside users which use loop inside defs is phi node in
  // exit blocks. We need to add the corresponding clone loop inside defs to
  // those phi nodes.
  SmallVector<BasicBlock *> ExitBlocks;
  L->getUniqueExitBlocks(ExitBlocks);
  for (BasicBlock *ExitBB : ExitBlocks) {
    for (PHINode &ExitPHI : ExitBB->phis()) {
      SmallVector<std::pair<Value *, BasicBlock *>, 4> Incomings;
      for (unsigned I = 0, E = ExitPHI.getNumIncomingValues(); I != E; I++)
        Incomings.push_back(std::make_pair(ExitPHI.getIncomingValue(I),
                                           ExitPHI.getIncomingBlock(I)));

      // In loop simplify form, all exit blocks are dominated by header.
      // Therefore incoming basicblock(IBB) must be inside the original loop.
      // LCSSA gurantee incoming value(IValue) can't come from another clone
      // loop. This implies if IValue is in VMap, this IValue must be def inside
      // this loop. Otherwise it must be def outside any clone loop.
      for (auto &VB : Incomings) {
        Value *IValue = VB.first, *IBB = VB.second;
        Value *CloneIValue =
            VMap.count(IValue) ? cast<Value>(VMap[IValue]) : IValue;
        ExitPHI.addIncoming(CloneIValue, cast<BasicBlock>(VMap[IBB]));
      }
    }
  }
  return true;
}
