//===- Intel_HeteroArchOpt.cpp - Hetero Arch (such as ADL) Optimization ---===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass implements loop level multi version optimization for hybrid
// processor.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86TargetMachine.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

#define DEBUG_TYPE "x86-hetero-arch-opt"

static cl::opt<bool> DisableHeteroArchOpt(
    "disable-x86-hetero-arch-opt", cl::Hidden,
    cl::desc("Disable X86 Hetero Architecture Optimization."), cl::init(false));

// Defines preferable loop height to clone. If it's possible, this pass will try
// to clone loop candidate's ancestor loop with loop height equals to this value
// rather than directly clone loop candidates to leverage cpu core detection
// cost and detection interval. This pass may clone loop candidate's ancestor
// with height not equals to this value. The priority(dsc) is:
// 1. Ancestor height is greater than or equals to this value.
// 2. Ancestor height is closer to this value.
static cl::opt<uint32_t> PreferCloneLoopHeight("hetero-arch-clone-loop-height",
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

class X86HeteroArchOpt : public FunctionPass {
public:
  static char ID;

  X86HeteroArchOpt() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<TargetPassConfig>();
    AU.addRequiredID(LoopSimplifyID);
    AU.addRequiredID(LCSSAID);
    AU.addPreserved<TargetTransformInfoWrapperPass>();
  }

  bool runOnFunction(Function &F) override;

private:
  bool optLoop();

  // Data need to be cleaned over each function.
  void cleanup() {
    NoCloneLoops.clear();
    CloneExitBlocks.clear();
  }

  bool isTerminatorBB(const BasicBlock *BB) {
    return succ_begin(BB) == succ_end(BB);
  }

  // Populate loop candidates which contains flollowing instructions:
  // 1. masked_gather intrinsic that won't be scalarized for core.
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

  // Loops that are not cloneable due to token type.
  DenseSet<const Loop *> NoCloneLoops;

  // Exit blocks that should also be cloned when loop is cloned.
  DenseMap<Loop *, SmallVector<BasicBlock *, 2>> CloneExitBlocks;
};

} // end anonymous namespace

char X86HeteroArchOpt::ID = 0;
char &llvm::X86HeteroArchOptID = X86HeteroArchOpt::ID;

INITIALIZE_PASS_BEGIN(X86HeteroArchOpt, DEBUG_TYPE, "Hetero Arch Optimization",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(LCSSAWrapperPass)
INITIALIZE_PASS_END(X86HeteroArchOpt, DEBUG_TYPE, "Hetero Arch Optimization",
                    false, false)

FunctionPass *llvm::createX86HeteroArchOptPass() {
  return new X86HeteroArchOpt();
}

bool X86HeteroArchOpt::runOnFunction(Function &F) {
  if (DisableHeteroArchOpt || skipFunction(F))
    return false;

  // This opt will bloat code.
  if (F.hasOptSize())
    return false;

  StringRef CPU = F.getFnAttribute("target-cpu").getValueAsString();
  const X86Subtarget *ST = getAnalysis<TargetPassConfig>()
                               .getTM<X86TargetMachine>()
                               .getSubtargetImpl(F);
  bool ShouldOptimize = ST->hasFastCoreType() || CPU == "core-avx2";
  if (!ShouldOptimize)
    return false;

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  CurFn = &F;

  // Dominator tree and loop info is NOT up to date after this optimization.
  bool Changed = optLoop();
  cleanup();
  return Changed;
}

bool X86HeteroArchOpt::optLoop() {
  SmallVector<LoopCand> LoopCandidates;
  for (auto *Lp : *LI)
    scanLoopCandidates(Lp, LoopCandidates);
  return createLoopMultiVersion(LoopCandidates);
}

unsigned
X86HeteroArchOpt::scanLoopCandidates(Loop *L,
                                     SmallVector<LoopCand> &LoopCandidates) {
  unsigned MaxHeight = 0;
  for (Loop *SubLoop : *L)
    MaxHeight =
        std::max(scanLoopCandidates(SubLoop, LoopCandidates), MaxHeight);
  MaxHeight++;

  LoopCand LC(L, MaxHeight);
  for (BasicBlock *BB : L->blocks()) {
    bool InSubLoop = LI->getLoopFor(BB) != L;
    for (Instruction &I : *BB) {
      // LCSSA won't handle tokens defined inside loop but with loop outside
      // user. We can't use tokens in PHI so we mark this loop as not cloneable.
      // An exception is if the token is only used in exit blocks and those exit
      // blocks has no successor and all of predecessors are in loop. In this
      // case, we can safely clone blocks of loop as well as those exit blocks.
      if (I.getType()->isTokenTy()) {
        for (User *U : I.users()) {
          Instruction *UI = cast<Instruction>(U);
          BasicBlock *UBB = UI->getParent();
          if (L->contains(UI))
            continue;

          // We don't need to check if UBB is dominated by loop header here
          // since cloneLoop works only if loop is in simplify form.
          SmallVector<BasicBlock *> ExitBlocks;
          L->getUniqueExitBlocks(ExitBlocks);
          if (isTerminatorBB(UBB) &&
              (find(ExitBlocks, UBB) != ExitBlocks.end())) {
            CloneExitBlocks[L].push_back(UBB);
            continue;
          }

          NoCloneLoops.insert(L);
          break;
        }
        continue;
      }

      // Skip BB belonging to subloops.
      if (InSubLoop)
        continue;

      IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I);
      if (II && II->getIntrinsicID() == Intrinsic::masked_gather &&
          !TTI->shouldScalarizeMaskedGather(II))
        LC.CandInsts.push_back(II);
    }
  }

  if (LC.CandInsts.size())
    LoopCandidates.push_back(std::move(LC));

  return MaxHeight;
}

bool X86HeteroArchOpt::createLoopMultiVersion(
    SmallVector<LoopCand> &LoopCandidates) {
  // Group ancestor loops belonging to same top level loop.
  MapVector<Loop *, SmallVector<Loop *>> TopLevelLoop2AncLoops;
  DenseMap<Loop *, SmallDenseSet<LoopCand *, 2>> AncLoop2Cands;

  for (auto &LC : LoopCandidates) {
    SmallVector<Loop *, 4> Path;
    for (Loop *L = LC.L; L; L = L->getParentLoop())
      Path.push_back(L);

    Loop *TopLevelLoop = Path.back();
    unsigned PreferIdx =
        LC.MaxHeight < PreferCloneLoopHeight
            ? std::min<unsigned>(PreferCloneLoopHeight - LC.MaxHeight,
                                 LC.L->getLoopDepth() - 1)
            : 0;
    // Sort path so that loop with preferable height comes frist, then loops
    // with greater height in asc, then lower height in dsc.
    std::reverse(Path.begin(), Path.begin() + PreferIdx);
    std::rotate(Path.begin(), Path.begin() + PreferIdx, Path.end());

    // AncLoop is the ancestor of L to be cloned.
    auto AncLoop =
        find_if(Path, [this](const Loop *L) { return !NoCloneLoops.count(L); });
    if (AncLoop != Path.end()) {
      // At this point, AncLoop2Cands doesn't contain all subloop candidates
      // which is inside the corresponding ancestor loop. It'll be filled later.
      TopLevelLoop2AncLoops[TopLevelLoop].push_back(*AncLoop);
      AncLoop2Cands[*AncLoop].insert(&LC);
    } else {
      LLVM_DEBUG(dbgs() << "Can't create multiversion for loop:\n" << *LC.L);
    }
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

bool X86HeteroArchOpt::cloneLoop(Loop *L, ValueToValueMapTy &VMap) {
  assert(!NoCloneLoops.count(L) && "Found not cloneable loop");
  if (!L->isLoopSimplifyForm() || !L->isLCSSAForm(*DT))
    return false;

  // Clone all basicblocks belong to loop L and insert clone one after the
  // original one.
  std::vector<BasicBlock *> OrigBBs = L->getBlocksVector();
  SmallVector<BasicBlock *, 32> CloneBBs;
  OrigBBs.insert(OrigBBs.end(), CloneExitBlocks[L].begin(),
                 CloneExitBlocks[L].end());
  auto InsertPoint = (*OrigBBs.rbegin())->getIterator();
  for (auto RI = OrigBBs.rbegin(), End = OrigBBs.rend(); RI != End; RI++) {
    BasicBlock *CloneBB = CloneBasicBlock(*RI, VMap, ".clone");
    CurFn->insert(InsertPoint, CloneBB);
    VMap[*RI] = CloneBB;
    CloneBBs.push_back(CloneBB);
  }
  remapInstructionsInBlocks(CloneBBs, VMap);

  // Insert cpu core type detection code in preheader.
  BasicBlock *Preheader = L->getLoopPreheader();
  assert(Preheader && "Preheader is expected");
  Preheader->getTerminator()->eraseFromParent();
  IRBuilder<> IRB(Preheader);
  CallInst *CoreType = IRB.CreateIntrinsic(
      Intrinsic::x86_intel_fast_cpuid_coretype, std::nullopt, std::nullopt);
  CoreType->getCalledFunction()->addFnAttr(CurFn->getFnAttribute("target-cpu"));
  // 20H is intel atom. 40H is intel core.
  Value *IsNotAtom = IRB.CreateICmpNE(CoreType, IRB.getInt8(0x20));
  // Origin loop for core and clone loop for atom.
  BasicBlock *OrigHeader = L->getHeader();
  BasicBlock *CloneHeader = cast<BasicBlock>(VMap[OrigHeader]);
  IRB.CreateCondBr(IsNotAtom, OrigHeader, CloneHeader);

  // In LCSSA form, all outside users which use loop inside defs is phi node in
  // exit blocks. We need to add the corresponding clone loop inside defs to
  // those phi nodes.
  SmallVector<BasicBlock *> ExitBlocks;
  L->getUniqueExitBlocks(ExitBlocks);
  for (BasicBlock *ExitBB : ExitBlocks) {
    // Don't need to fix phi for cloned exit basic block.
    if (VMap.count(ExitBB))
      continue;

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
