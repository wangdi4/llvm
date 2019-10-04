//===- JumpThreading.cpp - Thread control through conditional blocks ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Jump Threading pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/JumpThreading.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/GuardUtils.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/Intel_AggInline.h"          // INTEL
#include "llvm/Analysis/Intel_Andersens.h"          // INTEL
#include "llvm/Analysis/Intel_WP.h"                 // INTEL
#include "llvm/Analysis/LazyValueInfo.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"   // INTEL
#include "llvm/Analysis/Loads.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"      // INTEL
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/BlockFrequency.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>

using namespace llvm;
using namespace jumpthreading;

#define DEBUG_TYPE "jump-threading"

STATISTIC(NumThreads, "Number of jumps threaded");
STATISTIC(NumFolds,   "Number of terminators folded");
STATISTIC(NumDupes,   "Number of branch blocks duplicated to eliminate phi");

static cl::opt<unsigned>
BBDuplicateThreshold("jump-threading-threshold",
          cl::desc("Max block size to duplicate for jump threading"),
          cl::init(6), cl::Hidden);

static cl::opt<unsigned>
ImplicationSearchThreshold(
  "jump-threading-implication-search-threshold",
  cl::desc("The number of predecessors to search for a stronger "
           "condition to use to thread over a weaker condition"),
  cl::init(3), cl::Hidden);

static cl::opt<bool> PrintLVIAfterJumpThreading(
    "print-lvi-after-jump-threading",
    cl::desc("Print the LazyValueInfo cache after JumpThreading"), cl::init(false),
    cl::Hidden);

static cl::opt<bool> ThreadAcrossLoopHeaders(
    "jump-threading-across-loop-headers",
    cl::desc("Allow JumpThreading to thread across loop headers, for testing"),
    cl::init(false), cl::Hidden);

#if INTEL_CUSTOMIZATION
static cl::opt<bool>
JumpThreadLoopHeader("jump-thread-loop-header",
                     cl::desc("Jump thread through loop header blocks"),
                     cl::init(true), cl::Hidden);

static cl::opt<bool>
DistantJumpThreading("distant-jump-threading",
          cl::desc("Perform jump threading across larger-than-BB regions"),
          cl::init(true), cl::Hidden);

static cl::opt<bool>
ConservativeJumpThreading("conservative-jump-threading",
          cl::desc("Use conservative heuristics for loop headers and multi-BB "
                   "thread regions"),
          cl::init(true), cl::Hidden);
#endif // INTEL_CUSTOMIZATION

namespace {

  /// This pass performs 'jump threading', which looks at blocks that have
  /// multiple predecessors and multiple successors.  If one or more of the
  /// predecessors of the block can be proven to always jump to one of the
  /// successors, we forward the edge from the predecessor to the successor by
  /// duplicating the contents of this block.
  ///
  /// An example of when this can occur is code like this:
  ///
  ///   if () { ...
  ///     X = 4;
  ///   }
  ///   if (X < 3) {
  ///
  /// In this case, the unconditional branch at the end of the first if can be
  /// revectored to the false side of the second if.
  class JumpThreading : public FunctionPass {
    JumpThreadingPass Impl;

  public:
    static char ID; // Pass identification

    JumpThreading(int T = -1, bool AllowCFGSimps = true) :              // INTEL
      FunctionPass(ID), Impl(T, AllowCFGSimps) {                        // INTEL
      initializeJumpThreadingPass(*PassRegistry::getPassRegistry());
    }

    bool runOnFunction(Function &F) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addPreserved<DominatorTreeWrapperPass>();
      AU.addRequired<AAResultsWrapperPass>();
      AU.addRequired<LazyValueInfoWrapperPass>();
      AU.addPreserved<LazyValueInfoWrapperPass>();
      AU.addPreserved<GlobalsAAWrapperPass>();
      AU.addPreserved<AndersensAAWrapperPass>();                        // INTEL
      AU.addPreserved<InlineAggressiveWrapperPass>();                   // INTEL
      AU.addPreserved<WholeProgramWrapperPass>();                       // INTEL
      AU.addRequired<TargetLibraryInfoWrapperPass>();
      AU.addRequired<TargetTransformInfoWrapperPass>();                 // INTEL
    }

    void releaseMemory() override { Impl.releaseMemory(); }
  };

} // end anonymous namespace

char JumpThreading::ID = 0;

INITIALIZE_PASS_BEGIN(JumpThreading, "jump-threading",
                "Jump Threading", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LazyValueInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(JumpThreading, "jump-threading",
                "Jump Threading", false, false)

// Public interface to the Jump Threading pass
FunctionPass *llvm::createJumpThreadingPass(int Threshold,              // INTEL
                                            bool AllowCFGSimps) {       // INTEL
  return new JumpThreading(Threshold, AllowCFGSimps);                   // INTEL
}                                                                       // INTEL

JumpThreadingPass::JumpThreadingPass(int T, bool AllowCFGSimps) {       // INTEL
  DoCFGSimplifications = AllowCFGSimps;                                 // INTEL
  BBDupThreshold = (T == -1) ? BBDuplicateThreshold : unsigned(T);
}

// Update branch probability information according to conditional
// branch probability. This is usually made possible for cloned branches
// in inline instances by the context specific profile in the caller.
// For instance,
//
//  [Block PredBB]
//  [Branch PredBr]
//  if (t) {
//     Block A;
//  } else {
//     Block B;
//  }
//
//  [Block BB]
//  cond = PN([true, %A], [..., %B]); // PHI node
//  [Branch CondBr]
//  if (cond) {
//    ...  // P(cond == true) = 1%
//  }
//
//  Here we know that when block A is taken, cond must be true, which means
//      P(cond == true | A) = 1
//
//  Given that P(cond == true) = P(cond == true | A) * P(A) +
//                               P(cond == true | B) * P(B)
//  we get:
//     P(cond == true ) = P(A) + P(cond == true | B) * P(B)
//
//  which gives us:
//     P(A) is less than P(cond == true), i.e.
//     P(t == true) <= P(cond == true)
//
//  In other words, if we know P(cond == true) is unlikely, we know
//  that P(t == true) is also unlikely.
//
static void updatePredecessorProfileMetadata(PHINode *PN, BasicBlock *BB) {
  BranchInst *CondBr = dyn_cast<BranchInst>(BB->getTerminator());
  if (!CondBr)
    return;

  BranchProbability BP;
  uint64_t TrueWeight, FalseWeight;
  if (!CondBr->extractProfMetadata(TrueWeight, FalseWeight))
    return;

  // Returns the outgoing edge of the dominating predecessor block
  // that leads to the PhiNode's incoming block:
  auto GetPredOutEdge =
      [](BasicBlock *IncomingBB,
         BasicBlock *PhiBB) -> std::pair<BasicBlock *, BasicBlock *> {
    auto *PredBB = IncomingBB;
    auto *SuccBB = PhiBB;
    SmallPtrSet<BasicBlock *, 16> Visited;
    while (true) {
      BranchInst *PredBr = dyn_cast<BranchInst>(PredBB->getTerminator());
      if (PredBr && PredBr->isConditional())
        return {PredBB, SuccBB};
      Visited.insert(PredBB);
      auto *SinglePredBB = PredBB->getSinglePredecessor();
      if (!SinglePredBB)
        return {nullptr, nullptr};

      // Stop searching when SinglePredBB has been visited. It means we see
      // an unreachable loop.
      if (Visited.count(SinglePredBB))
        return {nullptr, nullptr};

      SuccBB = PredBB;
      PredBB = SinglePredBB;
    }
  };

  for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
    Value *PhiOpnd = PN->getIncomingValue(i);
    ConstantInt *CI = dyn_cast<ConstantInt>(PhiOpnd);

    if (!CI || !CI->getType()->isIntegerTy(1))
      continue;

    BP = (CI->isOne() ? BranchProbability::getBranchProbability(
                            TrueWeight, TrueWeight + FalseWeight)
                      : BranchProbability::getBranchProbability(
                            FalseWeight, TrueWeight + FalseWeight));

    auto PredOutEdge = GetPredOutEdge(PN->getIncomingBlock(i), BB);
    if (!PredOutEdge.first)
      return;

    BasicBlock *PredBB = PredOutEdge.first;
    BranchInst *PredBr = dyn_cast<BranchInst>(PredBB->getTerminator());
    if (!PredBr)
      return;

    uint64_t PredTrueWeight, PredFalseWeight;
    // FIXME: We currently only set the profile data when it is missing.
    // With PGO, this can be used to refine even existing profile data with
    // context information. This needs to be done after more performance
    // testing.
    if (PredBr->extractProfMetadata(PredTrueWeight, PredFalseWeight))
      continue;

    // We can not infer anything useful when BP >= 50%, because BP is the
    // upper bound probability value.
    if (BP >= BranchProbability(50, 100))
      continue;

    SmallVector<uint32_t, 2> Weights;
    if (PredBr->getSuccessor(0) == PredOutEdge.second) {
      Weights.push_back(BP.getNumerator());
      Weights.push_back(BP.getCompl().getNumerator());
    } else {
      Weights.push_back(BP.getCompl().getNumerator());
      Weights.push_back(BP.getNumerator());
    }
    PredBr->setMetadata(LLVMContext::MD_prof,
                        MDBuilder(PredBr->getParent()->getContext())
                            .createBranchWeights(Weights));
  }
}

/// runOnFunction - Toplevel algorithm.
bool JumpThreading::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;
  auto TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
#if INTEL_CUSTOMIZATION
  // If we need structured CFGs, disable jump threading, since it generally
  // tends to destructure them.
  auto TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  if (TTI->needsStructuredCFG())
    return false;
#endif
  // Get DT analysis before LVI. When LVI is initialized it conditionally adds
  // DT if it's available.
  auto DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto LVI = &getAnalysis<LazyValueInfoWrapperPass>().getLVI();
  auto AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  DomTreeUpdater DTU(*DT, DomTreeUpdater::UpdateStrategy::Lazy);
  std::unique_ptr<BlockFrequencyInfo> BFI;
  std::unique_ptr<BranchProbabilityInfo> BPI;
  bool HasProfileData = F.hasProfileData();
  if (HasProfileData) {
    LoopInfo LI{DominatorTree(F)};
    BPI.reset(new BranchProbabilityInfo(F, LI, TLI));
    BFI.reset(new BlockFrequencyInfo(F, *BPI, LI));
  }

  bool Changed = Impl.runImpl(F, TLI, LVI, AA, &DTU, HasProfileData,
                              std::move(BFI), std::move(BPI));
  if (PrintLVIAfterJumpThreading) {
    dbgs() << "LVI for function '" << F.getName() << "':\n";
    LVI->printLVI(F, *DT, dbgs());
  }
  return Changed;
}

PreservedAnalyses JumpThreadingPass::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  // Get DT analysis before LVI. When LVI is initialized it conditionally adds
  // DT if it's available.
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &LVI = AM.getResult<LazyValueAnalysis>(F);
  auto &AA = AM.getResult<AAManager>(F);
  DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);

  std::unique_ptr<BlockFrequencyInfo> BFI;
  std::unique_ptr<BranchProbabilityInfo> BPI;
  if (F.hasProfileData()) {
    LoopInfo LI{DominatorTree(F)};
    BPI.reset(new BranchProbabilityInfo(F, LI, &TLI));
    BFI.reset(new BlockFrequencyInfo(F, *BPI, LI));
  }

  bool Changed = runImpl(F, &TLI, &LVI, &AA, &DTU, HasProfileData,
                         std::move(BFI), std::move(BPI));

  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<GlobalsAA>();
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<LazyValueAnalysis>();
  PA.preserve<InlineAggAnalysis>();   // INTEL
  PA.preserve<AndersensAA>();         // INTEL
  PA.preserve<WholeProgramAnalysis>();// INTEL
  return PA;
}

bool JumpThreadingPass::runImpl(Function &F, TargetLibraryInfo *TLI_,
                                LazyValueInfo *LVI_, AliasAnalysis *AA_,
                                DomTreeUpdater *DTU_, bool HasProfileData_,
                                std::unique_ptr<BlockFrequencyInfo> BFI_,
                                std::unique_ptr<BranchProbabilityInfo> BPI_) {
  LLVM_DEBUG(dbgs() << "Jump threading on function '" << F.getName() << "'\n");
  TLI = TLI_;
  LVI = LVI_;
  AA = AA_;
  DTU = DTU_;
  BFI.reset();
  BPI.reset();
  // When profile data is available, we need to update edge weights after
  // successful jump threading, which requires both BPI and BFI being available.
  HasProfileData = HasProfileData_;
  auto *GuardDecl = F.getParent()->getFunction(
      Intrinsic::getName(Intrinsic::experimental_guard));
  HasGuards = GuardDecl && !GuardDecl->use_empty();
  if (HasProfileData) {
    BPI = std::move(BPI_);
    BFI = std::move(BFI_);
  }

  // JumpThreading must not processes blocks unreachable from entry. It's a
  // waste of compute time and can potentially lead to hangs.
  SmallPtrSet<BasicBlock *, 16> Unreachable;
  assert(DTU && "DTU isn't passed into JumpThreading before using it.");
  assert(DTU->hasDomTree() && "JumpThreading relies on DomTree to proceed.");
  DominatorTree &DT = DTU->getDomTree();
  for (auto &BB : F)
    if (!DT.isReachableFromEntry(&BB))
      Unreachable.insert(&BB);

  if (!ThreadAcrossLoopHeaders)
    FindLoopHeaders(F);

  bool EverChanged = false;
  bool Changed;
  do {
    Changed = false;
    for (auto &BB : F) {
      if (Unreachable.count(&BB))
        continue;
      while (ProcessBlock(&BB)) // Thread all of the branches we can over BB.
        Changed = true;
      // Stop processing BB if it's the entry or is now deleted. The following
      // routines attempt to eliminate BB and locating a suitable replacement
      // for the entry is non-trivial.
      if (&BB == &F.getEntryBlock() || DTU->isBBPendingDeletion(&BB))
        continue;

      if (pred_empty(&BB)) {
        // When ProcessBlock makes BB unreachable it doesn't bother to fix up
        // the instructions in it. We must remove BB to prevent invalid IR.
        LLVM_DEBUG(dbgs() << "  JT: Deleting dead block '" << BB.getName()
                          << "' with terminator: " << *BB.getTerminator()
                          << '\n');
        LoopHeaders.erase(&BB);
        CountableLoopLatches.erase(&BB);   // INTEL
        CountableLoopHeaders.erase(&BB);   // INTEL
        LVI->eraseBlock(&BB);
        DeleteDeadBlock(&BB, DTU);
        Changed = true;
        continue;
      }

      // ProcessBlock doesn't thread BBs with unconditional TIs. However, if BB
      // is "almost empty", we attempt to merge BB with its sole successor.
      auto *BI = dyn_cast<BranchInst>(BB.getTerminator());
      if (BI && BI->isUnconditional() &&
          DoCFGSimplifications &&                                       // INTEL
          // The terminator must be the only non-phi instruction in BB.
          BB.getFirstNonPHIOrDbg()->isTerminator() &&
          // Don't alter Loop headers and latches to ensure another pass can
          // detect and transform nested loops later.
          !LoopHeaders.count(&BB) && !LoopHeaders.count(BI->getSuccessor(0)) &&
          TryToSimplifyUncondBranchFromEmptyBlock(&BB, DTU)) {
        // BB is valid for cleanup here because we passed in DTU. F remains
        // BB's parent until a DTU->getDomTree() event.
        LVI->eraseBlock(&BB);
        CountableLoopLatches.erase(&BB); // INTEL
        CountableLoopHeaders.erase(&BB); // INTEL
        Changed = true;
      }
    }
    EverChanged |= Changed;
  } while (Changed);

  LoopHeaders.clear();
  CountableLoopLatches.clear(); // INTEL
  CountableLoopHeaders.clear(); // INTEL
  // Flush only the Dominator Tree.
  DTU->getDomTree();
  LVI->enableDT();
  return EverChanged;
}

// Replace uses of Cond with ToVal when safe to do so. If all uses are
// replaced, we can remove Cond. We cannot blindly replace all uses of Cond
// because we may incorrectly replace uses when guards/assumes are uses of
// of `Cond` and we used the guards/assume to reason about the `Cond` value
// at the end of block. RAUW unconditionally replaces all uses
// including the guards/assumes themselves and the uses before the
// guard/assume.
static void ReplaceFoldableUses(Instruction *Cond, Value *ToVal) {
  assert(Cond->getType() == ToVal->getType());
  auto *BB = Cond->getParent();
  // We can unconditionally replace all uses in non-local blocks (i.e. uses
  // strictly dominated by BB), since LVI information is true from the
  // terminator of BB.
  replaceNonLocalUsesWith(Cond, ToVal);
  for (Instruction &I : reverse(*BB)) {
    // Reached the Cond whose uses we are trying to replace, so there are no
    // more uses.
    if (&I == Cond)
      break;
    // We only replace uses in instructions that are guaranteed to reach the end
    // of BB, where we know Cond is ToVal.
    if (!isGuaranteedToTransferExecutionToSuccessor(&I))
      break;
    I.replaceUsesOfWith(Cond, ToVal);
  }
  if (Cond->use_empty() && !Cond->mayHaveSideEffects())
    Cond->eraseFromParent();
}

#if INTEL_CUSTOMIZATION
/// getJumpThreadDuplicationCost - Return the cost of duplicating this region to
/// thread across it. Stop scanning the region when passing the threshold.
static unsigned getJumpThreadDuplicationCost(
  const SmallVectorImpl<BasicBlock*> &RegionBlocks,
  const BasicBlock *RegionBottom,
  unsigned Threshold) {
  const Instruction *BBTerm = RegionBottom->getTerminator();

  unsigned Bonus = 0;
  // Threading through a switch statement is particularly profitable.  If this
  // block ends in a switch, decrease its cost to make it more likely to happen.
  if (isa<SwitchInst>(BBTerm))
    Bonus = 6;

  // The same holds for indirect branches, but slightly more so.
  if (isa<IndirectBrInst>(BBTerm))
    Bonus = 8;


  // Bump the threshold up so the early exit from the loop doesn't skip the
  // terminator-based Size adjustment at the end.
  Threshold += Bonus;

  // Sum up the cost of each instruction until we get to the terminator.  Don't
  // include the terminator because the copy won't include it.
  unsigned Size = 0;
  for (auto BB : RegionBlocks) {
    /// Ignore PHI nodes, these will be flattened when duplication happens.
    BasicBlock::const_iterator I(BB->getFirstNonPHI());

    // FIXME: THREADING will delete values that are just used to compute the
    // branch, so they shouldn't count against the duplication cost.

    // Sum up the cost of each instruction.
    for (; I != BB->end(); ++I) {
      // Don't include the terminator in the region bottom, because the copy
      // won't include it.
      if (I->isTerminator() && BB == RegionBottom)
        continue;

      if (IntrinsicUtils::isDirective(const_cast<Instruction *>(&*I)))
        return Threshold + 1;

      // Stop scanning the block if we've reached the threshold.
      if (Size > Threshold)
        return Size;

      // Debugger intrinsics don't incur code size.
      if (isa<DbgInfoIntrinsic>(I)) continue;

      // If this is a pointer->pointer bitcast, it is free.
      if (isa<BitCastInst>(I) && I->getType()->isPointerTy())
        continue;

      // Bail out if this instruction gives back a token type, it is not
      // possible to duplicate it if it is used outside this BB.
      if (I->getType()->isTokenTy() && I->isUsedOutsideOfBlock(BB))
        return ~0U;

      // All other instructions count for at least one unit.
      ++Size;

      // Calls are more expensive.  If they are non-intrinsic calls, we model
      // them as having cost of 4.  If they are a non-vector intrinsic, we model
      // them as having cost of 2 total, and if they are a vector intrinsic, we
      //  model them as having cost 1.
      if (const CallInst *CI = dyn_cast<CallInst>(I)) {
        if (CI->cannotDuplicate() || CI->isConvergent())
          // Blocks with NoDuplicate are modelled as having infinite cost, so
          // they are never duplicated.
          return ~0U;
        else if (!isa<IntrinsicInst>(CI))
          Size += 3;
        else if (!CI->getType()->isVectorTy())
          Size += 1;
      }
    }
  }

  return Size > Bonus ? Size - Bonus : 0;
}

// Approximates checks for common countable loop structure-
// 1) Header has two predecessors.
// 2) Latch ends in a conditional branch.
// 3) Latch condition is a ICmp instruction.
// 4) One of the compare operands looks like an IV.
//
// Example IR-
// latch:
// %inc = add %iv, 1
// %cmp = icmp lt %inc, 5
// br %cmp, %header, %exit
static bool isCountableLoop(const BasicBlock *HeaderBB,
                            const BasicBlock *LatchBB) {

  if (std::distance(pred_begin(HeaderBB), pred_end(HeaderBB)) != 2)
    return false;

  auto *BrTerm = dyn_cast<BranchInst>(LatchBB->getTerminator());

  if (!BrTerm || BrTerm->isUnconditional())
    return false;

  auto *CmpInst = dyn_cast<ICmpInst>(BrTerm->getCondition());

  if (!CmpInst)
    return false;

  for (const Value *Op : CmpInst->operands()) {
    auto *OpInst = dyn_cast<Instruction>(Op);

    if (!OpInst)
      continue;

    auto ParentBB = OpInst->getParent();
    bool DefinedInHeader = (ParentBB == HeaderBB);
    bool DefinedInHeaderOrLatch = (DefinedInHeader || (ParentBB == LatchBB));

    // If instruction is defined in the header or latch and is among the most
    // common instruction types for an IV (add, sub, GEP and phi), assume it
    // is an IV.
    if (DefinedInHeaderOrLatch &&
        ((OpInst->getOpcode() == Instruction::Add) ||
        (OpInst->getOpcode() == Instruction::Sub) ||
        (OpInst->getOpcode() == Instruction::GetElementPtr) ||
        (DefinedInHeader && isa<PHINode>(OpInst))))
      return true;
  }

  return false;
}

#endif // INTEL_CUSTOMIZATION

/// FindLoopHeaders - We do not want jump threading to turn proper loop
/// structures into irreducible loops.  Doing this breaks up the loop nesting
/// hierarchy and pessimizes later transformations.  To prevent this from
/// happening, we first have to find the loop headers.  Here we approximate this
/// by finding targets of backedges in the CFG.
///
/// Note that there definitely are cases when we want to allow threading of
/// edges across a loop header.  For example, threading a jump from outside the
/// loop (the preheader) to an exit block of the loop is definitely profitable.
/// It is also almost always profitable to thread backedges from within the loop
/// to exit blocks, and is often profitable to thread backedges to other blocks
/// within the loop (forming a nested loop).  This simple analysis is not rich
/// enough to track all of these properties and keep it up-to-date as the CFG
/// mutates, so we don't allow any of these transformations.
void JumpThreadingPass::FindLoopHeaders(Function &F) {
  SmallVector<std::pair<const BasicBlock*,const BasicBlock*>, 32> Edges;
  FindFunctionBackedges(F, Edges);

  for (const auto &Edge : Edges)
    LoopHeaders.insert(Edge.second);
#if INTEL_CUSTOMIZATION
  if (F.isPreLoopOpt())
    for (const auto &Edge : Edges)
      if (isCountableLoop(Edge.second, Edge.first)) {
        CountableLoopLatches.insert(Edge.first);
        CountableLoopHeaders.insert(Edge.second);
      }
#endif // INTEL_CUSTOMIZATION
}

/// getKnownConstant - Helper method to determine if we can thread over a
/// terminator with the given value as its condition, and if so what value to
/// use for that. What kind of value this is depends on whether we want an
/// integer or a block address, but an undef is always accepted.
/// Returns null if Val is null or not an appropriate constant.
static Constant *getKnownConstant(Value *Val, ConstantPreference Preference) {
  if (!Val)
    return nullptr;

  // Undef is "known" enough.
  if (UndefValue *U = dyn_cast<UndefValue>(Val))
    return U;

  if (Preference == WantBlockAddress)
    return dyn_cast<BlockAddress>(Val->stripPointerCasts());

  return dyn_cast<ConstantInt>(Val);
}

#if INTEL_CUSTOMIZATION
/// Test whether two ThreadRegions are identical.
static bool matchingRegionInfo(const ThreadRegionInfo &RegionInfo1,
                               const ThreadRegionInfo &RegionInfo2) {
  ThreadRegionInfoIterator I1 = RegionInfo1.begin(), I1E = RegionInfo1.end();
  ThreadRegionInfoIterator I2 = RegionInfo2.begin(), I2E = RegionInfo2.end();

  while (I1 != I1E && I2 != I2E) {
    if (I1->first != I2->first || I1->second != I2->second)
      return false;
    ++I1, ++I2;
  }

  return I1 == I1E && I2 == I2E;
}
#endif // INTEL_CUSTOMIZATION

/// ComputeValueKnownInPredecessors - Given a basic block BB and a value V, see
/// if we can infer that the value is a known ConstantInt/BlockAddress or undef
/// in any of our predecessors.  If so, return the known list of value and pred
/// BB in the result vector.
///
/// This returns true if there were any known values.
bool JumpThreadingPass::ComputeValueKnownInPredecessorsImpl(
    Value *V, BasicBlock *BB, PredValueInfo &Result,
    ThreadRegionInfo &RegionInfo,       // INTEL
    ConstantPreference Preference,
    DenseSet<std::pair<Value *, BasicBlock *>> &RecursionSet,
    Instruction *CxtI) {
  // This method walks up use-def chains recursively.  Because of this, we could
  // get into an infinite loop going around loops in the use-def chain.  To
  // prevent this, keep track of what (value, block) pairs we've already visited
  // and terminate the search if we loop back to them
  if (!RecursionSet.insert(std::make_pair(V, BB)).second)
    return false;

  // If V is a constant, then it is known in all predecessors.
  if (Constant *KC = getKnownConstant(V, Preference)) {
    for (BasicBlock *Pred : predecessors(BB))
      Result.push_back(std::make_pair(KC, Pred));

    RegionInfo.push_back(std::make_pair(BB, BB));                       // INTEL
    return !Result.empty();
  }

  // If V is a non-instruction value, or an instruction in a different block,
  // then it can't be derived from a PHI.
  Instruction *I = dyn_cast<Instruction>(V);
  if (!I || I->getParent() != BB) {

    // Okay, if this is a live-in value, see if it has a known value at the end
    // of any of our predecessors.
    //
    // FIXME: This should be an edge property, not a block end property.
    /// TODO: Per PR2563, we could infer value range information about a
    /// predecessor based on its terminator.
    //
    // FIXME: change this to use the more-rich 'getPredicateOnEdge' method if
    // "I" is a non-local compare-with-a-constant instruction.  This would be
    // able to handle value inequalities better, for example if the compare is
    // "X < 4" and "X < 3" is known true but "X < 4" itself is not available.
    // Perhaps getConstantOnEdge should be smart enough to do this?

    if (DTU->hasPendingDomTreeUpdates())
      LVI->disableDT();
    else
      LVI->enableDT();
    for (BasicBlock *P : predecessors(BB)) {
      // If the value is known by LazyValueInfo to be a constant in a
      // predecessor, use that information to try to thread this block.
      Constant *PredCst = LVI->getConstantOnEdge(V, P, BB, CxtI);
      if (Constant *KC = getKnownConstant(PredCst, Preference))
        Result.push_back(std::make_pair(KC, P));
    }

#if INTEL_CUSTOMIZATION
    if (!Result.empty()) {
      RegionInfo.push_back(std::make_pair(BB, BB));
      return true;
    }

    // If I is a PHI node defined outside BB, we might have a candidate for
    // distant jump threading.
    if (!DistantJumpThreading || !I || !isa<PHINode>(I))
      return false;
#endif // INTEL_CUSTOMIZATION
  }

  /// If I is a PHI node, then we know the incoming values for any constants.
  if (PHINode *PN = dyn_cast<PHINode>(I)) {
    if (DTU->hasPendingDomTreeUpdates())
      LVI->disableDT();
    else
      LVI->enableDT();
    for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
      Value *InVal = PN->getIncomingValue(i);
      if (Constant *KC = getKnownConstant(InVal, Preference)) {
        Result.push_back(std::make_pair(KC, PN->getIncomingBlock(i)));
      } else {
        Constant *CI = LVI->getConstantOnEdge(InVal,
                                              PN->getIncomingBlock(i),
                                              PN->getParent(), CxtI);   // INTEL
        if (Constant *KC = getKnownConstant(CI, Preference))
          Result.push_back(std::make_pair(KC, PN->getIncomingBlock(i)));
      }
    }

#if INTEL_CUSTOMIZATION
    if (!Result.empty()) {
      RegionInfo.push_back(std::make_pair(PN->getParent(), BB));
      return true;
    }

    if (!DistantJumpThreading)
      return false;

    // We failed to find any constant incoming values to PN. Now look back
    // even further. One of the operands to PN might itself be a PHI node
    // with constant incoming values. This has the potential of really exploding
    // compile time, so only search back through one PHI.
    if (RegionInfo.size() > 0)
      return false;
    
    RegionInfo.push_back(std::make_pair(PN->getParent(), BB));
    for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
      Value *InVal = PN->getIncomingValue(i);
      if (PHINode *PN2 = dyn_cast<PHINode>(InVal)) {
        ComputeValueKnownInPredecessors(PN2, PN2->getParent(), Result,
                                        RegionInfo, Preference, CxtI);
        if (!Result.empty()) {
          // ComputeValueKnownInPredecessors will have made PN2's block the
          // bottom of the thread sub-region. We need to change this to be
          // the relevant predecessor of PN's block.
          assert(RegionInfo.size() == 2 && "Unexpected thread region");
          RegionInfo.back().second = PN->getIncomingBlock(i);
          return true;
        }
      }
    }
    RegionInfo.pop_back();

    return false;
#endif // INTEL_CUSTOMIZATION
  }

  // Handle Cast instructions.  Only see through Cast when the source operand is
  // PHI or Cmp to save the compilation time.
  if (CastInst *CI = dyn_cast<CastInst>(I)) {
    Value *Source = CI->getOperand(0);
    if (!isa<PHINode>(Source) && !isa<CmpInst>(Source))
      return false;
    ComputeValueKnownInPredecessorsImpl(Source, BB, Result, RegionInfo, // INTEL
                                        Preference, RecursionSet, CxtI);// INTEL
    if (Result.empty())
      return false;

    // Convert the known values.
    for (auto &R : Result)
      R.first = ConstantExpr::getCast(CI->getOpcode(), R.first, CI->getType());

    return true;
  }

  // Handle some boolean conditions.
  if (I->getType()->getPrimitiveSizeInBits() == 1) {
    assert(Preference == WantInteger && "One-bit non-integer type?");
    // X | true -> true
    // X & false -> false
    if (I->getOpcode() == Instruction::Or ||
        I->getOpcode() == Instruction::And) {
      PredValueInfoTy LHSVals, RHSVals;
      ThreadRegionInfoTy RegionInfoOp0, RegionInfoOp1;                  // INTEL

      ComputeValueKnownInPredecessorsImpl(I->getOperand(0), BB, LHSVals,
                                          RegionInfoOp0,                    // INTEL
                                      WantInteger, RecursionSet, CxtI);
      ComputeValueKnownInPredecessorsImpl(I->getOperand(1), BB, RHSVals,
                                          RegionInfoOp1,                    // INTEL
                                          WantInteger, RecursionSet, CxtI);

      if (LHSVals.empty() && RHSVals.empty())
        return false;

      ConstantInt *InterestingVal;
      if (I->getOpcode() == Instruction::Or)
        InterestingVal = ConstantInt::getTrue(I->getContext());
      else
        InterestingVal = ConstantInt::getFalse(I->getContext());

      SmallPtrSet<BasicBlock*, 4> LHSKnownBBs;

      // Scan for the sentinel.  If we find an undef, force it to the
      // interesting value: x|undef -> true and x&undef -> false.
      for (const auto &LHSVal : LHSVals)
        if (LHSVal.first == InterestingVal || isa<UndefValue>(LHSVal.first)) {
          Result.emplace_back(InterestingVal, LHSVal.second);
          LHSKnownBBs.insert(LHSVal.second);
        }

#if INTEL_CUSTOMIZATION
      // It is possible that there are known values for both operand 0 and
      // operand 1 but where the top of the thread region is different. Don't
      // attempt to handle this case. Instead, arbitrarily use operand 0's
      // known values when the thread region is different.
      if (!Result.empty() &&
          !matchingRegionInfo(RegionInfoOp0, RegionInfoOp1)) {
        RegionInfo.insert(RegionInfo.end(), RegionInfoOp0.begin(),
                          RegionInfoOp0.end());
        return true;
      }
#endif // INTEL_CUSTOMIZATION

      for (const auto &RHSVal : RHSVals)
        if (RHSVal.first == InterestingVal || isa<UndefValue>(RHSVal.first)) {
          // If we already inferred a value for this block on the LHS, don't
          // re-add it.
          if (!LHSKnownBBs.count(RHSVal.second))
            Result.emplace_back(InterestingVal, RHSVal.second);
        }

      if (!Result.empty())                                              // INTEL
        RegionInfo.insert(RegionInfo.end(), RegionInfoOp1.begin(),      // INTEL
                          RegionInfoOp1.end());                         // INTEL

      return !Result.empty();
    }

    // Handle the NOT form of XOR.
    if (I->getOpcode() == Instruction::Xor &&
        isa<ConstantInt>(I->getOperand(1)) &&
        cast<ConstantInt>(I->getOperand(1))->isOne()) {
      ComputeValueKnownInPredecessorsImpl(I->getOperand(0), BB, Result,
                                          RegionInfo,                       // INTEL
                                          WantInteger, RecursionSet, CxtI);
      if (Result.empty())
        return false;

      // Invert the known values.
      for (auto &R : Result)
        R.first = ConstantExpr::getNot(R.first);

      return true;
    }

  // Try to simplify some other binary operator values.
  } else if (BinaryOperator *BO = dyn_cast<BinaryOperator>(I)) {
    assert(Preference != WantBlockAddress
            && "A binary operator creating a block address?");

    ThreadRegionInfoTy RegionInfoOp0;                                   // INTEL
    if (ConstantInt *CI = dyn_cast<ConstantInt>(BO->getOperand(1))) {
      PredValueInfoTy LHSVals;
      ComputeValueKnownInPredecessorsImpl(BO->getOperand(0), BB, LHSVals,
                                          RegionInfoOp0,                    // INTEL
                                          WantInteger, RecursionSet, CxtI);

      // Try to use constant folding to simplify the binary operator.
      for (const auto &LHSVal : LHSVals) {
        Constant *V = LHSVal.first;
        Constant *Folded = ConstantExpr::get(BO->getOpcode(), V, CI);

        if (Constant *KC = getKnownConstant(Folded, WantInteger))
          Result.push_back(std::make_pair(KC, LHSVal.second));
      }
    }

    if (!Result.empty())                                                // INTEL
      RegionInfo.insert(RegionInfo.end(), RegionInfoOp0.begin(),        // INTEL
                        RegionInfoOp0.end());                           // INTEL

    return !Result.empty();
  }

  // Handle compare with phi operand, where the PHI is defined in this block.
  if (CmpInst *Cmp = dyn_cast<CmpInst>(I)) {
    assert(Preference == WantInteger && "Compares only produce integers");
    Type *CmpType = Cmp->getType();
    Value *CmpLHS = Cmp->getOperand(0);
    Value *CmpRHS = Cmp->getOperand(1);
    CmpInst::Predicate Pred = Cmp->getPredicate();

    PHINode *PN = dyn_cast<PHINode>(CmpLHS);
    if (!PN)
      PN = dyn_cast<PHINode>(CmpRHS);
    if (PN && PN->getParent() == BB) {
      const DataLayout &DL = PN->getModule()->getDataLayout();
      // We can do this simplification if any comparisons fold to true or false.
      // See if any do.
      if (DTU->hasPendingDomTreeUpdates())
        LVI->disableDT();
      else
        LVI->enableDT();
      for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
        BasicBlock *PredBB = PN->getIncomingBlock(i);
        Value *LHS, *RHS;
        if (PN == CmpLHS) {
          LHS = PN->getIncomingValue(i);
          RHS = CmpRHS->DoPHITranslation(BB, PredBB);
        } else {
          LHS = CmpLHS->DoPHITranslation(BB, PredBB);
          RHS = PN->getIncomingValue(i);
        }

#if INTEL_CUSTOMIZATION
        // When BB is a loop header, LHS can be derived from a Value, %V,
        // computed in BB during a prior iteration of the loop. We have to be
        // careful to avoid trying to simplify a comparison based on a RHS also
        // derived from %V but from the current loop iteration. It is
        // computationally expensive to detect this situation, because %V can be
        // arbitrarily far back in the expression tree for LHS. So
        // conservatively suppress this optimization when BB is a loop header.
        //
        // Note that the problem is provably confined to loop headers. In order
        // for LHS to be derived from %V defined in BB, BB must dominate PredBB,
        // which makes PredBB-->BB a backedge and BB a member of LoopHeaders.
        //
        // Caution! This requires LoopHeaders to be kept up-to-date for
        // correctness. Practically speaking, this isn't a new requirement
        // given the number of failures that occured when first allowing jump
        // threading across loop headers. But this check makes the requirement
        // explicit and intentional.
        if (isa<Instruction>(LHS) && isa<Instruction>(RHS) &&
            LoopHeaders.count(BB))
          continue;
#endif // INTEL_CUSTOMIZATION

        Value *Res = SimplifyCmpInst(Pred, LHS, RHS, {DL});
        if (!Res) {
          if (!isa<Constant>(RHS))
            continue;

          // getPredicateOnEdge call will make no sense if LHS is defined in BB.
          auto LHSInst = dyn_cast<Instruction>(LHS);
          if (LHSInst && LHSInst->getParent() == BB)
            continue;

          LazyValueInfo::Tristate
            ResT = LVI->getPredicateOnEdge(Pred, LHS,
                                           cast<Constant>(RHS), PredBB, BB,
                                           CxtI ? CxtI : Cmp);
          if (ResT == LazyValueInfo::Unknown)
            continue;
          Res = ConstantInt::get(Type::getInt1Ty(LHS->getContext()), ResT);
        }

        if (Constant *KC = getKnownConstant(Res, WantInteger))
          Result.push_back(std::make_pair(KC, PredBB));
      }

      if (!Result.empty())                                              // INTEL
        RegionInfo.push_back(std::make_pair(BB, BB));                   // INTEL

      return !Result.empty();
    }

    // If comparing a live-in value against a constant, see if we know the
    // live-in value on any predecessors.
    if (isa<Constant>(CmpRHS) && !CmpType->isVectorTy()) {
      Constant *CmpConst = cast<Constant>(CmpRHS);

      if (!isa<Instruction>(CmpLHS) ||
          cast<Instruction>(CmpLHS)->getParent() != BB) {
        if (DTU->hasPendingDomTreeUpdates())
          LVI->disableDT();
        else
          LVI->enableDT();
        for (BasicBlock *P : predecessors(BB)) {
          // If the value is known by LazyValueInfo to be a constant in a
          // predecessor, use that information to try to thread this block.
          LazyValueInfo::Tristate Res =
            LVI->getPredicateOnEdge(Pred, CmpLHS,
                                    CmpConst, P, BB, CxtI ? CxtI : Cmp);
          if (Res == LazyValueInfo::Unknown)
            continue;

          Constant *ResC = ConstantInt::get(CmpType, Res);
          Result.push_back(std::make_pair(ResC, P));
        }

        if (!Result.empty())                                            // INTEL
          RegionInfo.push_back(std::make_pair(BB, BB));                 // INTEL

        return !Result.empty();
      }

      // InstCombine can fold some forms of constant range checks into
      // (icmp (add (x, C1)), C2). See if we have we have such a thing with
      // x as a live-in.
      {
        using namespace PatternMatch;

        Value *AddLHS;
        ConstantInt *AddConst;
        if (isa<ConstantInt>(CmpConst) &&
            match(CmpLHS, m_Add(m_Value(AddLHS), m_ConstantInt(AddConst)))) {
          if (!isa<Instruction>(AddLHS) ||
              cast<Instruction>(AddLHS)->getParent() != BB) {
            if (DTU->hasPendingDomTreeUpdates())
              LVI->disableDT();
            else
              LVI->enableDT();
            for (BasicBlock *P : predecessors(BB)) {
              // If the value is known by LazyValueInfo to be a ConstantRange in
              // a predecessor, use that information to try to thread this
              // block.
              ConstantRange CR = LVI->getConstantRangeOnEdge(
                  AddLHS, P, BB, CxtI ? CxtI : cast<Instruction>(CmpLHS));
              // Propagate the range through the addition.
              CR = CR.add(AddConst->getValue());

              // Get the range where the compare returns true.
              ConstantRange CmpRange = ConstantRange::makeExactICmpRegion(
                  Pred, cast<ConstantInt>(CmpConst)->getValue());

              Constant *ResC;
              if (CmpRange.contains(CR))
                ResC = ConstantInt::getTrue(CmpType);
              else if (CmpRange.inverse().contains(CR))
                ResC = ConstantInt::getFalse(CmpType);
              else
                continue;

              Result.push_back(std::make_pair(ResC, P));
            }

            if (!Result.empty())                                        // INTEL
              RegionInfo.push_back(std::make_pair(BB, BB));             // INTEL

            return !Result.empty();
          }
        }
      }

      // Try to find a constant value for the LHS of a comparison,
      // and evaluate it statically if we can.
      PredValueInfoTy LHSVals;
      ThreadRegionInfoTy RegionInfoOp0;                                     // INTEL
      ComputeValueKnownInPredecessorsImpl(I->getOperand(0), BB, LHSVals,
                                          RegionInfoOp0,                    // INTEL
                                          WantInteger, RecursionSet, CxtI);

      for (const auto &LHSVal : LHSVals) {
        Constant *V = LHSVal.first;
        Constant *Folded = ConstantExpr::getCompare(Pred, V, CmpConst);
        if (Constant *KC = getKnownConstant(Folded, WantInteger))
          Result.push_back(std::make_pair(KC, LHSVal.second));
      }

      if (!Result.empty())                                              // INTEL
        RegionInfo.insert(RegionInfo.end(), RegionInfoOp0.begin(),      // INTEL
                          RegionInfoOp0.end());                         // INTEL

      return !Result.empty();
    }
  }

  if (SelectInst *SI = dyn_cast<SelectInst>(I)) {
    // Handle select instructions where at least one operand is a known constant
    // and we can figure out the condition value for any predecessor block.
    Constant *TrueVal = getKnownConstant(SI->getTrueValue(), Preference);
    Constant *FalseVal = getKnownConstant(SI->getFalseValue(), Preference);
    PredValueInfoTy Conds;
    ThreadRegionInfoTy RegionInfoOp0;                                   // INTEL
    if ((TrueVal || FalseVal) &&
        ComputeValueKnownInPredecessorsImpl(SI->getCondition(), BB, Conds,
                                            RegionInfoOp0,                  // INTEL
                                            WantInteger, RecursionSet, CxtI)) {
      for (auto &C : Conds) {
        Constant *Cond = C.first;

        // Figure out what value to use for the condition.
        bool KnownCond;
        if (ConstantInt *CI = dyn_cast<ConstantInt>(Cond)) {
          // A known boolean.
          KnownCond = CI->isOne();
        } else {
          assert(isa<UndefValue>(Cond) && "Unexpected condition value");
          // Either operand will do, so be sure to pick the one that's a known
          // constant.
          // FIXME: Do this more cleverly if both values are known constants?
          KnownCond = (TrueVal != nullptr);
        }

        // See if the select has a known constant value for this predecessor.
        if (Constant *Val = KnownCond ? TrueVal : FalseVal)
          Result.push_back(std::make_pair(Val, C.second));
      }

      if (!Result.empty())                                              // INTEL
        RegionInfo.insert(RegionInfo.end(), RegionInfoOp0.begin(),      // INTEL
                          RegionInfoOp0.end());                         // INTEL

      return !Result.empty();
    }
  }

  // If all else fails, see if LVI can figure out a constant value for us.
  if (DTU->hasPendingDomTreeUpdates())
    LVI->disableDT();
  else
    LVI->enableDT();
  Constant *CI = LVI->getConstant(V, BB, CxtI);
  if (Constant *KC = getKnownConstant(CI, Preference)) {
    for (BasicBlock *Pred : predecessors(BB))
      Result.push_back(std::make_pair(KC, Pred));
  }

  if (!Result.empty())                                                  // INTEL
    RegionInfo.push_back(std::make_pair(BB, BB));                       // INTEL

  return !Result.empty();
}

/// GetBestDestForBranchOnUndef - If we determine that the specified block ends
/// in an undefined jump, decide which block is best to revector to.
///
/// Since we can pick an arbitrary destination, we pick the successor with the
/// fewest predecessors.  This should reduce the in-degree of the others.
static unsigned GetBestDestForJumpOnUndef(BasicBlock *BB) {
  Instruction *BBTerm = BB->getTerminator();
  unsigned MinSucc = 0;
  BasicBlock *TestBB = BBTerm->getSuccessor(MinSucc);
  // Compute the successor with the minimum number of predecessors.
  unsigned MinNumPreds = pred_size(TestBB);
  for (unsigned i = 1, e = BBTerm->getNumSuccessors(); i != e; ++i) {
    TestBB = BBTerm->getSuccessor(i);
    unsigned NumPreds = pred_size(TestBB);
    if (NumPreds < MinNumPreds) {
      MinSucc = i;
      MinNumPreds = NumPreds;
    }
  }

  return MinSucc;
}

static bool hasAddressTakenAndUsed(BasicBlock *BB) {
  if (!BB->hasAddressTaken()) return false;

  // If the block has its address taken, it may be a tree of dead constants
  // hanging off of it.  These shouldn't keep the block alive.
  BlockAddress *BA = BlockAddress::get(BB);
  BA->removeDeadConstantUsers();
  return !BA->use_empty();
}

/// ProcessBlock - If there are any predecessors whose control can be threaded
/// through to a successor, transform them now.
bool JumpThreadingPass::ProcessBlock(BasicBlock *BB) {
  // If the block is trivially dead, just return and let the caller nuke it.
  // This simplifies other transformations.
  if (DTU->isBBPendingDeletion(BB) ||
      (pred_empty(BB) && BB != &BB->getParent()->getEntryBlock()))
    return false;

  // If this block has a single predecessor, and if that pred has a single
  // successor, merge the blocks.  This encourages recursive jump threading
  // because now the condition in this block can be threaded through
  // predecessors of our predecessor block.
  if (BasicBlock *SinglePred = BB->getSinglePredecessor()) {
    const Instruction *TI = SinglePred->getTerminator();
    if (!TI->isExceptionalTerminator() && TI->getNumSuccessors() == 1 &&
        DoCFGSimplifications &&                                         // INTEL
        SinglePred != BB && !hasAddressTakenAndUsed(BB)) {
      // If SinglePred was a loop header, BB becomes one.
      if (LoopHeaders.erase(SinglePred))
        LoopHeaders.insert(BB);

      CountableLoopLatches.erase(SinglePred);   // INTEL
      CountableLoopHeaders.erase(SinglePred);   // INTEL
      LVI->eraseBlock(SinglePred);
      MergeBasicBlockIntoOnlyPred(BB, DTU);

      // Now that BB is merged into SinglePred (i.e. SinglePred Code followed by
      // BB code within one basic block `BB`), we need to invalidate the LVI
      // information associated with BB, because the LVI information need not be
      // true for all of BB after the merge. For example,
      // Before the merge, LVI info and code is as follows:
      // SinglePred: <LVI info1 for %p val>
      // %y = use of %p
      // call @exit() // need not transfer execution to successor.
      // assume(%p) // from this point on %p is true
      // br label %BB
      // BB: <LVI info2 for %p val, i.e. %p is true>
      // %x = use of %p
      // br label exit
      //
      // Note that this LVI info for blocks BB and SinglPred is correct for %p
      // (info2 and info1 respectively). After the merge and the deletion of the
      // LVI info1 for SinglePred. We have the following code:
      // BB: <LVI info2 for %p val>
      // %y = use of %p
      // call @exit()
      // assume(%p)
      // %x = use of %p <-- LVI info2 is correct from here onwards.
      // br label exit
      // LVI info2 for BB is incorrect at the beginning of BB.

      // Invalidate LVI information for BB if the LVI is not provably true for
      // all of BB.
      if (!isGuaranteedToTransferExecutionToSuccessor(BB))
        LVI->eraseBlock(BB);
      return true;
    }
  }

  if (TryToUnfoldSelectInCurrBB(BB))
    return true;

  // Look if we can propagate guards to predecessors.
  if (HasGuards && ProcessGuards(BB))
    return true;

  // What kind of constant we're looking for.
  ConstantPreference Preference = WantInteger;

  // Look to see if the terminator is a conditional branch, switch or indirect
  // branch, if not we can't thread it.
  Value *Condition;
  Instruction *Terminator = BB->getTerminator();
  if (BranchInst *BI = dyn_cast<BranchInst>(Terminator)) {
    // Can't thread an unconditional jump.
    if (BI->isUnconditional()) return false;
    Condition = BI->getCondition();
  } else if (SwitchInst *SI = dyn_cast<SwitchInst>(Terminator)) {
    Condition = SI->getCondition();
  } else if (IndirectBrInst *IB = dyn_cast<IndirectBrInst>(Terminator)) {
    // Can't thread indirect branch with no successors.
    if (IB->getNumSuccessors() == 0) return false;
    Condition = IB->getAddress()->stripPointerCasts();
    Preference = WantBlockAddress;
  } else {
    return false; // Must be an invoke or callbr.
  }

  // Run constant folding to see if we can reduce the condition to a simple
  // constant.
  if (Instruction *I = dyn_cast<Instruction>(Condition)) {
    Value *SimpleVal =
        ConstantFoldInstruction(I, BB->getModule()->getDataLayout(), TLI);
    if (SimpleVal) {
      I->replaceAllUsesWith(SimpleVal);
      if (isInstructionTriviallyDead(I, TLI))
        I->eraseFromParent();
      Condition = SimpleVal;
    }
  }

  // If the terminator is branching on an undef, we can pick any of the
  // successors to branch to.  Let GetBestDestForJumpOnUndef decide.
  if (isa<UndefValue>(Condition)) {
    unsigned BestSucc = GetBestDestForJumpOnUndef(BB);
    std::vector<DominatorTree::UpdateType> Updates;

    // Fold the branch/switch.
    Instruction *BBTerm = BB->getTerminator();
    Updates.reserve(BBTerm->getNumSuccessors());
    for (unsigned i = 0, e = BBTerm->getNumSuccessors(); i != e; ++i) {
      if (i == BestSucc) continue;
      BasicBlock *Succ = BBTerm->getSuccessor(i);
      Succ->removePredecessor(BB, true);
      Updates.push_back({DominatorTree::Delete, BB, Succ});
    }

    LLVM_DEBUG(dbgs() << "  In block '" << BB->getName()
                      << "' folding undef terminator: " << *BBTerm << '\n');
    BranchInst::Create(BBTerm->getSuccessor(BestSucc), BBTerm);
    BBTerm->eraseFromParent();
    DTU->applyUpdatesPermissive(Updates);
    return true;
  }

  // If the terminator of this block is branching on a constant, simplify the
  // terminator to an unconditional branch.  This can occur due to threading in
  // other blocks.
  if (getKnownConstant(Condition, Preference)) {
    LLVM_DEBUG(dbgs() << "  In block '" << BB->getName()
                      << "' folding terminator: " << *BB->getTerminator()
                      << '\n');
    ++NumFolds;
    ConstantFoldTerminator(BB, true, nullptr, DTU);
    return true;
  }

  Instruction *CondInst = dyn_cast<Instruction>(Condition);

  // All the rest of our checks depend on the condition being an instruction.
  if (!CondInst) {
    // FIXME: Unify this with code below.
    if (ProcessThreadableEdges(Condition, BB, Preference, Terminator))
      return true;
    return false;
  }

  if (CmpInst *CondCmp = dyn_cast<CmpInst>(CondInst)) {
    // If we're branching on a conditional, LVI might be able to determine
    // it's value at the branch instruction.  We only handle comparisons
    // against a constant at this time.
    // TODO: This should be extended to handle switches as well.
    BranchInst *CondBr = dyn_cast<BranchInst>(BB->getTerminator());
    Constant *CondConst = dyn_cast<Constant>(CondCmp->getOperand(1));
    if (CondBr && CondConst) {
      // We should have returned as soon as we turn a conditional branch to
      // unconditional. Because its no longer interesting as far as jump
      // threading is concerned.
      assert(CondBr->isConditional() && "Threading on unconditional terminator");

      if (DTU->hasPendingDomTreeUpdates())
        LVI->disableDT();
      else
        LVI->enableDT();
      LazyValueInfo::Tristate Ret =
        LVI->getPredicateAt(CondCmp->getPredicate(), CondCmp->getOperand(0),
                            CondConst, CondBr);
      if (Ret != LazyValueInfo::Unknown) {
        unsigned ToRemove = Ret == LazyValueInfo::True ? 1 : 0;
        unsigned ToKeep = Ret == LazyValueInfo::True ? 0 : 1;
        BasicBlock *ToRemoveSucc = CondBr->getSuccessor(ToRemove);
        ToRemoveSucc->removePredecessor(BB, true);
        BranchInst *UncondBr =
          BranchInst::Create(CondBr->getSuccessor(ToKeep), CondBr);
        UncondBr->setDebugLoc(CondBr->getDebugLoc());
        CondBr->eraseFromParent();
        if (CondCmp->use_empty())
          CondCmp->eraseFromParent();
        // We can safely replace *some* uses of the CondInst if it has
        // exactly one value as returned by LVI. RAUW is incorrect in the
        // presence of guards and assumes, that have the `Cond` as the use. This
        // is because we use the guards/assume to reason about the `Cond` value
        // at the end of block, but RAUW unconditionally replaces all uses
        // including the guards/assumes themselves and the uses before the
        // guard/assume.
        else if (CondCmp->getParent() == BB) {
          auto *CI = Ret == LazyValueInfo::True ?
            ConstantInt::getTrue(CondCmp->getType()) :
            ConstantInt::getFalse(CondCmp->getType());
          ReplaceFoldableUses(CondCmp, CI);
        }
        DTU->applyUpdatesPermissive(
            {{DominatorTree::Delete, BB, ToRemoveSucc}});
        return true;
      }

      // We did not manage to simplify this branch, try to see whether
      // CondCmp depends on a known phi-select pattern.
      if (TryToUnfoldSelect(CondCmp, BB))
        return true;
    }
  }

  if (SwitchInst *SI = dyn_cast<SwitchInst>(BB->getTerminator()))
    if (TryToUnfoldSelect(SI, BB))
      return true;

  // Check for some cases that are worth simplifying.  Right now we want to look
  // for loads that are used by a switch or by the condition for the branch.  If
  // we see one, check to see if it's partially redundant.  If so, insert a PHI
  // which can then be used to thread the values.
  Value *SimplifyValue = CondInst;
  if (CmpInst *CondCmp = dyn_cast<CmpInst>(SimplifyValue))
    if (isa<Constant>(CondCmp->getOperand(1)))
      SimplifyValue = CondCmp->getOperand(0);

  // TODO: There are other places where load PRE would be profitable, such as
  // more complex comparisons.
  if (LoadInst *LoadI = dyn_cast<LoadInst>(SimplifyValue))
    if (SimplifyPartiallyRedundantLoad(LoadI))
      return true;

  // Before threading, try to propagate profile data backwards:
  if (PHINode *PN = dyn_cast<PHINode>(CondInst))
    if (PN->getParent() == BB && isa<BranchInst>(BB->getTerminator()))
      updatePredecessorProfileMetadata(PN, BB);

  // Handle a variety of cases where we are branching on something derived from
  // a PHI node in the current block.  If we can prove that any predecessors
  // compute a predictable value based on a PHI node, thread those predecessors.
  if (ProcessThreadableEdges(CondInst, BB, Preference, Terminator))
    return true;

  // If this is an otherwise-unfoldable branch on a phi node in the current
  // block, see if we can simplify.
  if (PHINode *PN = dyn_cast<PHINode>(CondInst))
    if (PN->getParent() == BB && isa<BranchInst>(BB->getTerminator()))
      return ProcessBranchOnPHI(PN);

  // If this is an otherwise-unfoldable branch on a XOR, see if we can simplify.
  if (CondInst->getOpcode() == Instruction::Xor &&
      CondInst->getParent() == BB && isa<BranchInst>(BB->getTerminator()))
    return ProcessBranchOnXOR(cast<BinaryOperator>(CondInst));

  // Search for a stronger dominating condition that can be used to simplify a
  // conditional branch leaving BB.
  if (ProcessImpliedCondition(BB))
    return true;

  return false;
}

bool JumpThreadingPass::ProcessImpliedCondition(BasicBlock *BB) {
  auto *BI = dyn_cast<BranchInst>(BB->getTerminator());
  if (!BI || !BI->isConditional())
    return false;

  Value *Cond = BI->getCondition();
  BasicBlock *CurrentBB = BB;
  BasicBlock *CurrentPred = BB->getSinglePredecessor();
  unsigned Iter = 0;

  auto &DL = BB->getModule()->getDataLayout();

  while (CurrentPred && Iter++ < ImplicationSearchThreshold) {
    auto *PBI = dyn_cast<BranchInst>(CurrentPred->getTerminator());
    if (!PBI || !PBI->isConditional())
      return false;
    if (PBI->getSuccessor(0) != CurrentBB && PBI->getSuccessor(1) != CurrentBB)
      return false;

    bool CondIsTrue = PBI->getSuccessor(0) == CurrentBB;
    Optional<bool> Implication =
        isImpliedCondition(PBI->getCondition(), Cond, DL, CondIsTrue);
    if (Implication) {
      BasicBlock *KeepSucc = BI->getSuccessor(*Implication ? 0 : 1);
      BasicBlock *RemoveSucc = BI->getSuccessor(*Implication ? 1 : 0);
      RemoveSucc->removePredecessor(BB);
      BranchInst *UncondBI = BranchInst::Create(KeepSucc, BI);
      UncondBI->setDebugLoc(BI->getDebugLoc());
      BI->eraseFromParent();
      DTU->applyUpdatesPermissive({{DominatorTree::Delete, BB, RemoveSucc}});
      return true;
    }
    CurrentBB = CurrentPred;
    CurrentPred = CurrentBB->getSinglePredecessor();
  }

  return false;
}

/// Return true if Op is an instruction defined in the given block.
static bool isOpDefinedInBlock(Value *Op, BasicBlock *BB) {
  if (Instruction *OpInst = dyn_cast<Instruction>(Op))
    if (OpInst->getParent() == BB)
      return true;
  return false;
}

/// SimplifyPartiallyRedundantLoad - If LoadI is an obviously partially
/// redundant load instruction, eliminate it by replacing it with a PHI node.
/// This is an important optimization that encourages jump threading, and needs
/// to be run interlaced with other jump threading tasks.
bool JumpThreadingPass::SimplifyPartiallyRedundantLoad(LoadInst *LoadI) {
  // Don't hack volatile and ordered loads.
  if (!LoadI->isUnordered()) return false;

  // If the load is defined in a block with exactly one predecessor, it can't be
  // partially redundant.
  BasicBlock *LoadBB = LoadI->getParent();
  if (LoadBB->getSinglePredecessor())
    return false;

  // If the load is defined in an EH pad, it can't be partially redundant,
  // because the edges between the invoke and the EH pad cannot have other
  // instructions between them.
  if (LoadBB->isEHPad())
    return false;

  Value *LoadedPtr = LoadI->getOperand(0);

  // If the loaded operand is defined in the LoadBB and its not a phi,
  // it can't be available in predecessors.
  if (isOpDefinedInBlock(LoadedPtr, LoadBB) && !isa<PHINode>(LoadedPtr))
    return false;

  // Scan a few instructions up from the load, to see if it is obviously live at
  // the entry to its block.
  BasicBlock::iterator BBIt(LoadI);
  bool IsLoadCSE;
  if (Value *AvailableVal = FindAvailableLoadedValue(
          LoadI, LoadBB, BBIt, DefMaxInstsToScan, AA, &IsLoadCSE)) {
    // If the value of the load is locally available within the block, just use
    // it.  This frequently occurs for reg2mem'd allocas.

    if (IsLoadCSE) {
      LoadInst *NLoadI = cast<LoadInst>(AvailableVal);
      combineMetadataForCSE(NLoadI, LoadI, false);
    };

    // If the returned value is the load itself, replace with an undef. This can
    // only happen in dead loops.
    if (AvailableVal == LoadI)
      AvailableVal = UndefValue::get(LoadI->getType());
    if (AvailableVal->getType() != LoadI->getType())
      AvailableVal = CastInst::CreateBitOrPointerCast(
          AvailableVal, LoadI->getType(), "", LoadI);
    LoadI->replaceAllUsesWith(AvailableVal);
    LoadI->eraseFromParent();
    return true;
  }

  // Otherwise, if we scanned the whole block and got to the top of the block,
  // we know the block is locally transparent to the load.  If not, something
  // might clobber its value.
  if (BBIt != LoadBB->begin())
    return false;

  // If all of the loads and stores that feed the value have the same AA tags,
  // then we can propagate them onto any newly inserted loads.
  AAMDNodes AATags;
  LoadI->getAAMetadata(AATags);

  SmallPtrSet<BasicBlock*, 8> PredsScanned;

  using AvailablePredsTy = SmallVector<std::pair<BasicBlock *, Value *>, 8>;

  AvailablePredsTy AvailablePreds;
  BasicBlock *OneUnavailablePred = nullptr;
  SmallVector<LoadInst*, 8> CSELoads;

  // If we got here, the loaded value is transparent through to the start of the
  // block.  Check to see if it is available in any of the predecessor blocks.
  for (BasicBlock *PredBB : predecessors(LoadBB)) {
    // If we already scanned this predecessor, skip it.
    if (!PredsScanned.insert(PredBB).second)
      continue;

    BBIt = PredBB->end();
    unsigned NumScanedInst = 0;
    Value *PredAvailable = nullptr;
    // NOTE: We don't CSE load that is volatile or anything stronger than
    // unordered, that should have been checked when we entered the function.
    assert(LoadI->isUnordered() &&
           "Attempting to CSE volatile or atomic loads");
    // If this is a load on a phi pointer, phi-translate it and search
    // for available load/store to the pointer in predecessors.
    Value *Ptr = LoadedPtr->DoPHITranslation(LoadBB, PredBB);
    PredAvailable = FindAvailablePtrLoadStore(
        Ptr, LoadI->getType(), LoadI->isAtomic(), PredBB, BBIt,
        DefMaxInstsToScan, AA, &IsLoadCSE, &NumScanedInst);

    // If PredBB has a single predecessor, continue scanning through the
    // single predecessor.
    BasicBlock *SinglePredBB = PredBB;
    while (!PredAvailable && SinglePredBB && BBIt == SinglePredBB->begin() &&
           NumScanedInst < DefMaxInstsToScan) {
      SinglePredBB = SinglePredBB->getSinglePredecessor();
      if (SinglePredBB) {
        BBIt = SinglePredBB->end();
        PredAvailable = FindAvailablePtrLoadStore(
            Ptr, LoadI->getType(), LoadI->isAtomic(), SinglePredBB, BBIt,
            (DefMaxInstsToScan - NumScanedInst), AA, &IsLoadCSE,
            &NumScanedInst);
      }
    }

    if (!PredAvailable) {
      OneUnavailablePred = PredBB;
      continue;
    }

    if (IsLoadCSE)
      CSELoads.push_back(cast<LoadInst>(PredAvailable));

    // If so, this load is partially redundant.  Remember this info so that we
    // can create a PHI node.
    AvailablePreds.push_back(std::make_pair(PredBB, PredAvailable));
  }

  // If the loaded value isn't available in any predecessor, it isn't partially
  // redundant.
  if (AvailablePreds.empty()) return false;

  // Okay, the loaded value is available in at least one (and maybe all!)
  // predecessors.  If the value is unavailable in more than one unique
  // predecessor, we want to insert a merge block for those common predecessors.
  // This ensures that we only have to insert one reload, thus not increasing
  // code size.
  BasicBlock *UnavailablePred = nullptr;

  // If the value is unavailable in one of predecessors, we will end up
  // inserting a new instruction into them. It is only valid if all the
  // instructions before LoadI are guaranteed to pass execution to its
  // successor, or if LoadI is safe to speculate.
  // TODO: If this logic becomes more complex, and we will perform PRE insertion
  // farther than to a predecessor, we need to reuse the code from GVN's PRE.
  // It requires domination tree analysis, so for this simple case it is an
  // overkill.
  if (PredsScanned.size() != AvailablePreds.size() &&
      !isSafeToSpeculativelyExecute(LoadI))
    for (auto I = LoadBB->begin(); &*I != LoadI; ++I)
      if (!isGuaranteedToTransferExecutionToSuccessor(&*I))
        return false;

  // If there is exactly one predecessor where the value is unavailable, the
  // already computed 'OneUnavailablePred' block is it.  If it ends in an
  // unconditional branch, we know that it isn't a critical edge.
  if (PredsScanned.size() == AvailablePreds.size()+1 &&
      OneUnavailablePred->getTerminator()->getNumSuccessors() == 1) {
    UnavailablePred = OneUnavailablePred;
  } else if (PredsScanned.size() != AvailablePreds.size()) {
    // Otherwise, we had multiple unavailable predecessors or we had a critical
    // edge from the one.
    SmallVector<BasicBlock*, 8> PredsToSplit;
    SmallPtrSet<BasicBlock*, 8> AvailablePredSet;

    for (const auto &AvailablePred : AvailablePreds)
      AvailablePredSet.insert(AvailablePred.first);

    // Add all the unavailable predecessors to the PredsToSplit list.
    for (BasicBlock *P : predecessors(LoadBB)) {
      // If the predecessor is an indirect goto, we can't split the edge.
      // Same for CallBr.
      if (isa<IndirectBrInst>(P->getTerminator()) ||
          isa<CallBrInst>(P->getTerminator()))
        return false;

      if (!AvailablePredSet.count(P))
        PredsToSplit.push_back(P);
    }

    // Split them out to their own block.
    UnavailablePred = SplitBlockPreds(LoadBB, PredsToSplit, "thread-pre-split");
  }

  // If the value isn't available in all predecessors, then there will be
  // exactly one where it isn't available.  Insert a load on that edge and add
  // it to the AvailablePreds list.
  if (UnavailablePred) {
    assert(UnavailablePred->getTerminator()->getNumSuccessors() == 1 &&
           "Can't handle critical edge here!");
    LoadInst *NewVal = new LoadInst(
        LoadI->getType(), LoadedPtr->DoPHITranslation(LoadBB, UnavailablePred),
        LoadI->getName() + ".pr", false, LoadI->getAlignment(),
        LoadI->getOrdering(), LoadI->getSyncScopeID(),
        UnavailablePred->getTerminator());
    NewVal->setDebugLoc(LoadI->getDebugLoc());
    if (AATags)
      NewVal->setAAMetadata(AATags);

    AvailablePreds.push_back(std::make_pair(UnavailablePred, NewVal));
  }

  // Now we know that each predecessor of this block has a value in
  // AvailablePreds, sort them for efficient access as we're walking the preds.
  array_pod_sort(AvailablePreds.begin(), AvailablePreds.end());

  // Create a PHI node at the start of the block for the PRE'd load value.
  pred_iterator PB = pred_begin(LoadBB), PE = pred_end(LoadBB);
  PHINode *PN = PHINode::Create(LoadI->getType(), std::distance(PB, PE), "",
                                &LoadBB->front());
  PN->takeName(LoadI);
  PN->setDebugLoc(LoadI->getDebugLoc());

  // Insert new entries into the PHI for each predecessor.  A single block may
  // have multiple entries here.
  for (pred_iterator PI = PB; PI != PE; ++PI) {
    BasicBlock *P = *PI;
    AvailablePredsTy::iterator I =
        llvm::lower_bound(AvailablePreds, std::make_pair(P, (Value *)nullptr));

    assert(I != AvailablePreds.end() && I->first == P &&
           "Didn't find entry for predecessor!");

    // If we have an available predecessor but it requires casting, insert the
    // cast in the predecessor and use the cast. Note that we have to update the
    // AvailablePreds vector as we go so that all of the PHI entries for this
    // predecessor use the same bitcast.
    Value *&PredV = I->second;
    if (PredV->getType() != LoadI->getType())
      PredV = CastInst::CreateBitOrPointerCast(PredV, LoadI->getType(), "",
                                               P->getTerminator());

    PN->addIncoming(PredV, I->first);
  }

  for (LoadInst *PredLoadI : CSELoads) {
    combineMetadataForCSE(PredLoadI, LoadI, true);
  }

  LoadI->replaceAllUsesWith(PN);
  LoadI->eraseFromParent();

  return true;
}

/// FindMostPopularDest - The specified list contains multiple possible
/// threadable destinations.  Pick the one that occurs the most frequently in
/// the list.
static BasicBlock *
FindMostPopularDest(BasicBlock *BB,
                    const ThreadRegionInfo &RegionInfo,                 // INTEL
                    const SmallVectorImpl<std::pair<BasicBlock *,
                                          BasicBlock *>> &PredToDestList) {
  assert(!PredToDestList.empty());

  // Determine popularity.  If there are multiple possible destinations, we
  // explicitly choose to ignore 'undef' destinations.  We prefer to thread
  // blocks with known and real destinations to threading undef.  We'll handle
  // them later if interesting.
  DenseMap<BasicBlock*, unsigned> DestPopularity;
  for (const auto &PredToDest : PredToDestList)
    if (PredToDest.second)
      DestPopularity[PredToDest.second]++;

  if (DestPopularity.empty())
    return nullptr;

#if INTEL_CUSTOMIZATION
  // Avoid picking a block in the thread region if there are any other
  // available choices, since thread-to-self is disallowed. At this point, we
  // don't know all the blocks in the region, but check the ones we do know,
  // i.e. the blocks that begin & end each sub-region.
  for (auto SubRegion : RegionInfo) {
    DestPopularity[SubRegion.first] = 0;
    DestPopularity[SubRegion.second] = 0;
  }
#endif // INTEL_CUSTOMIZATION

  // Find the most popular dest.
  DenseMap<BasicBlock*, unsigned>::iterator DPI = DestPopularity.begin();
  BasicBlock *MostPopularDest = DPI->first;
  unsigned Popularity = DPI->second;
  SmallVector<BasicBlock*, 4> SamePopularity;

  for (++DPI; DPI != DestPopularity.end(); ++DPI) {
    // If the popularity of this entry isn't higher than the popularity we've
    // seen so far, ignore it.
    if (DPI->second < Popularity)
      ; // ignore.
    else if (DPI->second == Popularity) {
      // If it is the same as what we've seen so far, keep track of it.
      SamePopularity.push_back(DPI->first);
    } else {
      // If it is more popular, remember it.
      SamePopularity.clear();
      MostPopularDest = DPI->first;
      Popularity = DPI->second;
    }
  }

  // Okay, now we know the most popular destination.  If there is more than one
  // destination, we need to determine one.  This is arbitrary, but we need
  // to make a deterministic decision.  Pick the first one that appears in the
  // successor list.
  if (!SamePopularity.empty()) {
    SamePopularity.push_back(MostPopularDest);
    Instruction *TI = BB->getTerminator();
    for (unsigned i = 0; ; ++i) {
      assert(i != TI->getNumSuccessors() && "Didn't find any successor!");

      if (!is_contained(SamePopularity, TI->getSuccessor(i)))
        continue;

      MostPopularDest = TI->getSuccessor(i);
      break;
    }
  }

  // Okay, we have finally picked the most popular destination.
  return MostPopularDest;
}

bool JumpThreadingPass::ProcessThreadableEdges(Value *Cond, BasicBlock *BB,
                                               ConstantPreference Preference,
                                               Instruction *CxtI) {
  // If threading this would thread across a loop header, don't even try to
  // thread the edge.
  if (LoopHeaders.count(BB) && !JumpThreadLoopHeader)                   // INTEL
    return false;

  PredValueInfoTy PredValues;
  // bool Changed = false;                                              // INTEL
  ThreadRegionInfoTy RegionInfo;                                        // INTEL
  if (!ComputeValueKnownInPredecessors(Cond, BB, PredValues,            // INTEL
                                       RegionInfo,                      // INTEL
                                       Preference, CxtI))               // INTEL
    return false;

  assert(!PredValues.empty() && !RegionInfo.empty() &&                  // INTEL
         "ComputeValueKnownInPredecessors returned true with no "       // INTEL
         "values or regions");                                          // INTEL

  LLVM_DEBUG(dbgs() << "IN BB: " << *BB;
             for (const auto &PredValue : PredValues) {
               dbgs() << "  BB '" << BB->getName()
                      << "': FOUND condition = " << *PredValue.first
                      << " for pred '" << PredValue.second->getName() << "'.\n";
  });

  // Decide what we want to thread through.  Convert our list of known values to
  // a list of known destinations for each pred.  This also discards duplicate
  // predecessors and keeps track of the undefined inputs (which are represented
  // as a null dest in the PredToDestList).
  SmallPtrSet<BasicBlock*, 16> SeenPreds;
  SmallVector<std::pair<BasicBlock*, BasicBlock*>, 16> PredToDestList;

  BasicBlock *OnlyDest = nullptr;
  BasicBlock *MultipleDestSentinel = (BasicBlock*)(intptr_t)~0ULL;
  Constant *OnlyVal = nullptr;
  Constant *MultipleVal = (Constant *)(intptr_t)~0ULL;
  bool ThreadingBackedge = false;    // INTEL

  for (const auto &PredValue : PredValues) {
    BasicBlock *Pred = PredValue.second;
    if (!SeenPreds.insert(Pred).second)
      continue;  // Duplicate predecessor entry.

    Constant *Val = PredValue.first;

    BasicBlock *DestBB;
    if (isa<UndefValue>(Val))
      DestBB = nullptr;
    else if (BranchInst *BI = dyn_cast<BranchInst>(BB->getTerminator())) {
      assert(isa<ConstantInt>(Val) && "Expecting a constant integer");
      DestBB = BI->getSuccessor(cast<ConstantInt>(Val)->isZero());
    } else if (SwitchInst *SI = dyn_cast<SwitchInst>(BB->getTerminator())) {
      assert(isa<ConstantInt>(Val) && "Expecting a constant integer");
      DestBB = SI->findCaseValue(cast<ConstantInt>(Val))->getCaseSuccessor();
    } else {
      assert(isa<IndirectBrInst>(BB->getTerminator())
              && "Unexpected terminator");
      assert(isa<BlockAddress>(Val) && "Expecting a constant blockaddress");
      DestBB = cast<BlockAddress>(Val)->getBasicBlock();
    }

    // If we have exactly one destination, remember it for efficiency below.
    if (PredToDestList.empty()) {
      OnlyDest = DestBB;
      OnlyVal = Val;
    } else {
      if (OnlyDest != DestBB)
        OnlyDest = MultipleDestSentinel;
      // It possible we have same destination, but different value, e.g. default
      // case in switchinst.
      if (Val != OnlyVal)
        OnlyVal = MultipleVal;
    }

    // If the predecessor ends with an indirect goto, we can't change its
    // destination. Same for CallBr.
    if (isa<IndirectBrInst>(Pred->getTerminator()) ||
        isa<CallBrInst>(Pred->getTerminator()))
      continue;

#if INTEL_CUSTOMIZATION
    if (CountableLoopLatches.count(BB) && CountableLoopHeaders.count(DestBB))
      ThreadingBackedge = true;
#endif // INTEL_CUSTOMIZATION
    PredToDestList.push_back(std::make_pair(Pred, DestBB));
  }

  // If all edges were unthreadable, we fail.
  if (PredToDestList.empty())
    return false;

#if INTEL_CUSTOMIZATION
  // Allow threading of backedge only if we can thread all predecessors of latch
  // otherwise we may turn countable loop into non-countable loop by adding
  // additional backedges to it.
  if (ThreadingBackedge) {
    if (PredToDestList.size() != (unsigned)std::distance(pred_begin(BB),
                                                         pred_end(BB)))
      return false;
  }
#endif // INTEL_CUSTOMIZATION

  // If all the predecessors go to a single known successor, we want to fold,
  // not thread. By doing so, we do not need to duplicate the current block and
  // also miss potential opportunities in case we dont/cant duplicate.
  if (OnlyDest && OnlyDest != MultipleDestSentinel &&                  // INTEL
      RegionInfo.size() == 1 &&                                        // INTEL
      RegionInfo.back().first == RegionInfo.back().second) {           // INTEL
    if (BB->hasNPredecessors(PredToDestList.size())) {
      bool SeenFirstBranchToOnlyDest = false;
      std::vector <DominatorTree::UpdateType> Updates;
      Updates.reserve(BB->getTerminator()->getNumSuccessors() - 1);
      for (BasicBlock *SuccBB : successors(BB)) {
        if (SuccBB == OnlyDest && !SeenFirstBranchToOnlyDest) {
          SeenFirstBranchToOnlyDest = true; // Don't modify the first branch.
        } else {
          SuccBB->removePredecessor(BB, true); // This is unreachable successor.
          Updates.push_back({DominatorTree::Delete, BB, SuccBB});
        }
      }

      // Finally update the terminator.
      Instruction *Term = BB->getTerminator();
      BranchInst::Create(OnlyDest, Term);
      Term->eraseFromParent();
      DTU->applyUpdatesPermissive(Updates);

      // If the condition is now dead due to the removal of the old terminator,
      // erase it.
      if (auto *CondInst = dyn_cast<Instruction>(Cond)) {
        if (CondInst->use_empty() && !CondInst->mayHaveSideEffects())
          CondInst->eraseFromParent();
        // We can safely replace *some* uses of the CondInst if it has
        // exactly one value as returned by LVI. RAUW is incorrect in the
        // presence of guards and assumes, that have the `Cond` as the use. This
        // is because we use the guards/assume to reason about the `Cond` value
        // at the end of block, but RAUW unconditionally replaces all uses
        // including the guards/assumes themselves and the uses before the
        // guard/assume.
        else if (OnlyVal && OnlyVal != MultipleVal &&
                 CondInst->getParent() == BB)
          ReplaceFoldableUses(CondInst, OnlyVal);
      }
      return true;
    }
  }

  // Determine which is the most common successor.  If we have many inputs and
  // this block is a switch, we want to start by threading the batch that goes
  // to the most popular destination first.  If we only know about one
  // threadable destination (the common case) we can avoid this.
  BasicBlock *MostPopularDest = OnlyDest;

  if (MostPopularDest == MultipleDestSentinel) {
    // Remove any loop headers from the Dest list, ThreadEdge conservatively
    // won't process them, but we might have other destination that are eligible
    // and we still want to process.
    erase_if(PredToDestList,
             [&](const std::pair<BasicBlock *, BasicBlock *> &PredToDest) {
               return LoopHeaders.count(PredToDest.second) != 0;
             });

    if (PredToDestList.empty())
      return false;

    MostPopularDest = FindMostPopularDest(BB, RegionInfo,               // INTEL
                                          PredToDestList);              // INTEL
  }

  // Now that we know what the most popular destination is, factor all
  // predecessors that will jump to it into a single predecessor.
  SmallVector<BasicBlock*, 16> PredsToFactor;
  for (const auto &PredToDest : PredToDestList)
    if (PredToDest.second == MostPopularDest) {
      BasicBlock *Pred = PredToDest.first;

      // This predecessor may be a switch or something else that has multiple
      // edges to the block.  Factor each of these edges by listing them
      // according to # occurrences in PredsToFactor.
      for (BasicBlock *Succ : successors(Pred))
        if (Succ == RegionInfo.back().first)         // INTEL
          PredsToFactor.push_back(Pred);
    }

  // If the threadable edges are branching on an undefined value, we get to pick
  // the destination that these predecessors should get to.
  if (!MostPopularDest)
    MostPopularDest = BB->getTerminator()->
                            getSuccessor(GetBestDestForJumpOnUndef(BB));

  // Ok, try to thread it!
  return ThreadEdge(RegionInfo, PredsToFactor, MostPopularDest);    // INTEL
}

/// ProcessBranchOnPHI - We have an otherwise unthreadable conditional branch on
/// a PHI node in the current block.  See if there are any simplifications we
/// can do based on inputs to the phi node.
bool JumpThreadingPass::ProcessBranchOnPHI(PHINode *PN) {
  BasicBlock *BB = PN->getParent();

  // TODO: We could make use of this to do it once for blocks with common PHI
  // values.
  SmallVector<BasicBlock*, 1> PredBBs;
  PredBBs.resize(1);

  // If any of the predecessor blocks end in an unconditional branch, we can
  // *duplicate* the conditional branch into that block in order to further
  // encourage jump threading and to eliminate cases where we have branch on a
  // phi of an icmp (branch on icmp is much better).
  for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
    BasicBlock *PredBB = PN->getIncomingBlock(i);
    if (BranchInst *PredBr = dyn_cast<BranchInst>(PredBB->getTerminator()))
      if (PredBr->isUnconditional()) {
        PredBBs[0] = PredBB;
        // Try to duplicate BB into PredBB.
        if (DuplicateCondBranchOnPHIIntoPred(BB, PredBBs))
          return true;
      }
  }

  return false;
}

/// ProcessBranchOnXOR - We have an otherwise unthreadable conditional branch on
/// a xor instruction in the current block.  See if there are any
/// simplifications we can do based on inputs to the xor.
bool JumpThreadingPass::ProcessBranchOnXOR(BinaryOperator *BO) {
  BasicBlock *BB = BO->getParent();

  // If either the LHS or RHS of the xor is a constant, don't do this
  // optimization.
  if (isa<ConstantInt>(BO->getOperand(0)) ||
      isa<ConstantInt>(BO->getOperand(1)))
    return false;

  // If the first instruction in BB isn't a phi, we won't be able to infer
  // anything special about any particular predecessor.
  if (!isa<PHINode>(BB->front()))
    return false;

  // If this BB is a landing pad, we won't be able to split the edge into it.
  if (BB->isEHPad())
    return false;

  // If we have a xor as the branch input to this block, and we know that the
  // LHS or RHS of the xor in any predecessor is true/false, then we can clone
  // the condition into the predecessor and fix that value to true, saving some
  // logical ops on that path and encouraging other paths to simplify.
  //
  // This copies something like this:
  //
  //  BB:
  //    %X = phi i1 [1],  [%X']
  //    %Y = icmp eq i32 %A, %B
  //    %Z = xor i1 %X, %Y
  //    br i1 %Z, ...
  //
  // Into:
  //  BB':
  //    %Y = icmp ne i32 %A, %B
  //    br i1 %Y, ...

  PredValueInfoTy XorOpValues;
  ThreadRegionInfoTy RegionInfo;                                        // INTEL
  bool isLHS = true;
  if (!ComputeValueKnownInPredecessors(BO->getOperand(0), BB, XorOpValues,
                                       RegionInfo,                      // INTEL
                                       WantInteger, BO)) {
    assert(XorOpValues.empty());
    if (!ComputeValueKnownInPredecessors(BO->getOperand(1), BB, XorOpValues,
                                         RegionInfo,                    // INTEL
                                         WantInteger, BO))
      return false;
    isLHS = false;
  }

#if INTEL_CUSTOMIZATION
  // Distant jump threading doesn't currently support this transformation.
  if (RegionInfo.size() != 1 ||
      RegionInfo.back().first != RegionInfo.back().second)
    return false;
#endif // INTEL_CUSTOMIZATION

  assert(!XorOpValues.empty() &&
         "ComputeValueKnownInPredecessors returned true with no values");

  // Scan the information to see which is most popular: true or false.  The
  // predecessors can be of the set true, false, or undef.
  unsigned NumTrue = 0, NumFalse = 0;
  for (const auto &XorOpValue : XorOpValues) {
    if (isa<UndefValue>(XorOpValue.first))
      // Ignore undefs for the count.
      continue;
    if (cast<ConstantInt>(XorOpValue.first)->isZero())
      ++NumFalse;
    else
      ++NumTrue;
  }

  // Determine which value to split on, true, false, or undef if neither.
  ConstantInt *SplitVal = nullptr;
  if (NumTrue > NumFalse)
    SplitVal = ConstantInt::getTrue(BB->getContext());
  else if (NumTrue != 0 || NumFalse != 0)
    SplitVal = ConstantInt::getFalse(BB->getContext());

  // Collect all of the blocks that this can be folded into so that we can
  // factor this once and clone it once.
  SmallVector<BasicBlock*, 8> BlocksToFoldInto;
  for (const auto &XorOpValue : XorOpValues) {
    if (XorOpValue.first != SplitVal && !isa<UndefValue>(XorOpValue.first))
      continue;

    BlocksToFoldInto.push_back(XorOpValue.second);
  }

  // If we inferred a value for all of the predecessors, then duplication won't
  // help us.  However, we can just replace the LHS or RHS with the constant.
  if (BlocksToFoldInto.size() ==
      cast<PHINode>(BB->front()).getNumIncomingValues()) {
    if (!SplitVal) {
      // If all preds provide undef, just nuke the xor, because it is undef too.
      BO->replaceAllUsesWith(UndefValue::get(BO->getType()));
      BO->eraseFromParent();
    } else if (SplitVal->isZero()) {
      // If all preds provide 0, replace the xor with the other input.
      BO->replaceAllUsesWith(BO->getOperand(isLHS));
      BO->eraseFromParent();
    } else {
      // If all preds provide 1, set the computed value to 1.
      BO->setOperand(!isLHS, SplitVal);
    }

    return true;
  }

  // Try to duplicate BB into PredBB.
  return DuplicateCondBranchOnPHIIntoPred(BB, BlocksToFoldInto);
}

/// AddPHINodeEntriesForMappedBlock - We're adding 'NewPred' as a new
/// predecessor to the PHIBB block.  If it has PHI nodes, add entries for
/// NewPred using the entries from OldPred (suitably mapped).
static void AddPHINodeEntriesForMappedBlock(BasicBlock *PHIBB,
                                            BasicBlock *OldPred,
                                            BasicBlock *NewPred,
                                     DenseMap<Instruction*, Value*> &ValueMap) {
  for (PHINode &PN : PHIBB->phis()) {
    // Ok, we have a PHI node.  Figure out what the incoming value was for the
    // DestBlock.
    Value *IV = PN.getIncomingValueForBlock(OldPred);
#if !INTEL_CUSTOMIZATION
    // This code isn't needed with the Intel customizations, because we always
    // run the SSAUpdater to resolve cross-BB references.
    // Remap the value if necessary.
    if (Instruction *Inst = dyn_cast<Instruction>(IV)) {
      DenseMap<Instruction*, Value*>::iterator I = ValueMap.find(Inst);
      if (I != ValueMap.end())
        IV = I->second;
    }
#endif // !INTEL_CUSTOMIZATION

    PN.addIncoming(IV, NewPred);
  }
}

#if INTEL_CUSTOMIZATION
/// We intend to thread an edge into the region across a group of blocks to an
/// outgoing edge of the region. In order to do this, we have to duplicate all
/// the code along every path from the top to the bottom of every sub-region.
/// This function returns the set of blocks along those paths in RegionBlocks.
/// The sub-regions are structured such that the top is guaranteed to dominate
/// the bottom.
///
/// NOTE: It's possible that distant jump threading has started from a block
/// that is no longer reachable from the function entry due to earlier
/// threading. It may have traversed phis that have taken it back into the
/// reachable part of the CFG. When this occurs this function will not be able
/// to reach sub-region top from the sub-region bottom. If this occurs this
/// function returns false to indicate that we should abort threading the
/// current edge.
static bool collectThreadRegionBlocks(const ThreadRegionInfo &RegionInfo,
                                   SmallVectorImpl<BasicBlock*> &RegionBlocks) {
  SmallVector<BasicBlock*, 16> WorkStack;
  SmallPtrSet<BasicBlock*, 16> Visited;

  // Collect the blocks within each sub-region. This is just a DFS walk backward
  // from BBBottom to BBTop.
  for (auto SubRegion : RegionInfo) {
    BasicBlock *BBTop = SubRegion.first;
    BasicBlock *BBBottom = SubRegion.second;

    // If we already visited BBBottom in a previous subregion, we need to stop
    // because the remapping logic can't handle the same block being in two
    // subregions. It may be possible to thread such a case, but it would
    // probably require duplicating the shared block.
    if (!Visited.insert(BBBottom).second)
      return false;

    WorkStack.push_back(BBBottom);

    while (!WorkStack.empty()) {
      BasicBlock *BB = WorkStack.back();
      WorkStack.pop_back();
      RegionBlocks.push_back(BB);

      if (BB == BBTop)
        continue;

      for (BasicBlock *Pred : predecessors(BB))
        if (Visited.insert(Pred).second)
          WorkStack.push_back(Pred);
    }

    // If we didn't visit the top of the sub-region then part of the subregion
    // is unreachable and we shouldn't try to thread.
    if (!Visited.count(BBTop))
      return false;
  }

  return true;
}

/// \p OldBB is a block in the thread region, and \p OldSucc is one of its
/// original successors. This routine determines whether the
/// <tt>OldBB->OldSucc</tt> CFG edge in the new thread region needs to target
/// OldSucc or its corresponding new BB.
static bool shouldRemapTarget(BasicBlock *OldBB,
                  BasicBlock *OldSucc,
                  const ThreadRegionInfo &RegionInfo,
                  const DenseMap<BasicBlock*, BasicBlock*> &BlockMapping) {
  DenseMap<BasicBlock*, BasicBlock*>::const_iterator I =
    BlockMapping.find(OldSucc);

  // If OldSucc is not part of the region, then of course we don't remap it.
  if (I == BlockMapping.end())
    return false;

  // If OldSucc is the top of a sub-region, the only edge that can target
  // its corresponding new BB is the one from the previous sub-region.
  bool FoundOldSucc = false;
  for (auto SubRegion : RegionInfo) {
    if (FoundOldSucc)
      return SubRegion.second == OldBB;
    if (OldSucc == SubRegion.first)
      FoundOldSucc = true;
  }

  // If FoundOldSucc is true at this point, it means OldBB-->OldSucc is an edge
  // from inside the region back up to the top of the region. We don't want to
  // remap those. If FoundOldSucc is false, it means that OldSucc isn't the top
  // of a thread region, so we should remap all its predecessors.
  return !FoundOldSucc;
}

/// Determine whether \p OldBB is the top of a thread sub-region. If so, return
/// its predecessor BB through which we are threading. If not, return nullptr.
/// \p PredBB is the predecessor block for the entire thread region.
static BasicBlock* getSubRegionPred(BasicBlock *OldBB,
                                    BasicBlock *PredBB,
                                    const ThreadRegionInfo &RegionInfo) {
  bool FoundOldBB = false;
  for (auto SubRegion : RegionInfo) {
    if (FoundOldBB)
      return SubRegion.second;
    if (OldBB == SubRegion.first)
      FoundOldBB = true;
  }

  // If FoundOldBB is true here, it means that OldBB is the top of the entire
  // thread region.
  if (FoundOldBB)
    return PredBB;

  return nullptr;
}

/// ThreadEdge was significantly modified to support distant jump threading.
/// Not every line was changed, but the entire routine is under
/// INTEL_CUSTOMIZATION, because any community changes to this routine will need
/// to be manually merged.
///
/// ThreadEdge - We have decided that it is safe and profitable to factor the
/// blocks in PredBBs to one predecessor, then thread an edge from it to SuccBB
/// across a region of blocks.  Transform the IR to reflect this change.
bool JumpThreadingPass::ThreadEdge(const ThreadRegionInfo &RegionInfo,
                                   const SmallVectorImpl<BasicBlock*> &PredBBs,
                                   BasicBlock *SuccBB) {
  BasicBlock *RegionTop = RegionInfo.back().first;
  BasicBlock *RegionBottom = RegionInfo.front().second;
  SmallVector<BasicBlock*, 16> RegionBlocks;
  bool ThreadingLoopHeader = false;

  // If threading this would thread into a loop header don't thread the edge.
  // See the comments above FindLoopHeaders for justifications and caveats.
  if (LoopHeaders.count(SuccBB) && !JumpThreadLoopHeader) {
    LLVM_DEBUG(dbgs() << "  Not threading to dest loop header BB '"
                      << SuccBB->getName()
                      << "' - it might create an irreducible loop!\n");
    return false;
  }

  if (!collectThreadRegionBlocks(RegionInfo, RegionBlocks))
    return false;

  for (auto BB : RegionBlocks) {
    // Avoid threading back to a block in the region. This is a hacky way to
    // prevent the situation where we repeatedly thread across a loop header
    // effectively performing loop unrolling. This is not a general solution.
    // You can easily get into the massive unrolling situation in spite of this
    // check. In order to enable threading across loop headers by default, we
    // need a more robust fix for this problem.
    if (BB == SuccBB) {
      LLVM_DEBUG(dbgs() << "  Not threading across BB '"
                        << RegionBottom->getName()
                        << "' - would thread to self!\n");
      return false;
    }

    // Also avoid thread regions that have internal edges back to the top of
    // the region since threading to SuccBB is only valid when entering
    // RegionTop via PredBB.
    if (BB != RegionBottom)
      for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI)
        if (*SI == RegionTop) {
          LLVM_DEBUG(dbgs()
                     << "  Not threading across BB '" << RegionBottom->getName()
                     << "' - internal edge back to RegionTop!\n");
          return false;
        }

    // If threading this would thread across a loop header, don't thread the
    // edge. See the comments above FindLoopHeaders for justifications and
    // caveats.
    if (LoopHeaders.count(BB)) {
      if (!JumpThreadLoopHeader) {
        LLVM_DEBUG(dbgs() << "  Not threading across loop header BB '"
                          << BB->getName() << "' to dest BB '"
                          << SuccBB->getName()
                          << "' - it might create an irreducible loop!\n");
        return false;
      }
      ThreadingLoopHeader = true;

      if (BlockThreadCount[RegionBottom] >= MaxThreadsPerBlock) {
        LLVM_DEBUG(dbgs() << "  Not threading across loop header BB '"
                          << BB->getName()
                          << "' - max thread count reached!\n");
        return false;
      }
    }

    // We are using CountableLoopHeaders here instead because-
    // 1) They are only populated when function has pre_loopopt attribute.
    // 2) This function seems to be conservatively adding some bblocks which
    //    are not true loop headers in LoopHeaders.
    if (CountableLoopHeaders.count(BB)
        && isa<SwitchInst>(BB->getTerminator())) {
      LLVM_DEBUG(
          dbgs()
          << "  Not threading across loop header BB '" << BB->getName()
          << "' - header has switch terminator and threading may convert it "
          << "into a latch. Such loops aren't supported by loopopt!\n");
      return false;
    }
  }

  unsigned JumpThreadCost = getJumpThreadDuplicationCost(RegionBlocks,
                                                         RegionBottom,
                                                         BBDupThreshold);
  if (JumpThreadCost > BBDupThreshold) {
    LLVM_DEBUG(dbgs() << "  Not threading BB '" << RegionBottom->getName()
                      << "' - Cost is too high: " << JumpThreadCost << "\n");
    return false;
  }

  if (ConservativeJumpThreading) {
    // Only allow multi-BB thread regions when threading across switches.
    if (RegionBlocks.size() != 1 &&
        !isa<SwitchInst>(RegionBottom->getTerminator())) {
      LLVM_DEBUG(
          dbgs()
          << "  Not threading BB '" << RegionBottom->getName()
          << "' - Using conservative heuristics for distant threading.\n");
      return false;
    }

    // Only thread across loop headers when threading across switches or
    // threading to return blocks, which are known to exit the loop.
    if (ThreadingLoopHeader) {
      if (!isa<ReturnInst>(SuccBB->getTerminator()) &&
          !isa<SwitchInst>(RegionBottom->getTerminator())) {
        LLVM_DEBUG(
            dbgs() << "  Not threading BB '" << RegionBottom->getName()
                   << "' - Using conservative heuristics loop headers.\n");
        return false;
      }
    }
  }

  // And finally, do it!  Start by factoring the predecessors if needed.
  BasicBlock *PredBB;
  if (PredBBs.size() == 1)
    PredBB = PredBBs[0];
  else {
    LLVM_DEBUG(dbgs() << "  Factoring out " << PredBBs.size()
                      << " common predecessors.\n");
    PredBB = SplitBlockPreds(RegionTop, PredBBs, ".thr_comm");
  }

  // And finally, do it!
  LLVM_DEBUG(dbgs() << "  Threading edge from '" << PredBB->getName()
                    << "' to '" << SuccBB->getName() << "' with cost: "
                    << JumpThreadCost << ", across blocks:\n    ";
             for (auto BB : RegionBlocks)
               dbgs() << " " << BB->getName();
             dbgs() << "\n  Ending with" << *RegionBottom << "\n";);

  if (DTU->hasPendingDomTreeUpdates())
    LVI->disableDT();
  else
    LVI->enableDT();
  // FIXME: This LVI update is not optimal. Removing the PredBB-->RegionTop
  //   edge can make overdefined values computable in any block in the region,
  //   not just RegionBottom. We can generalize the LVI->threadEdge algorithm
  //   to support larger-than-BB thread regions.
  LVI->threadEdge(PredBB, RegionBottom, SuccBB);

  DenseMap<Instruction*, Value*> ValueMapping;
  DenseMap<BasicBlock*, BasicBlock*> BlockMapping;
  SmallVector<DominatorTree::UpdateType, 20> DTUpdates;

  for (auto OldBB : RegionBlocks) {
    BasicBlock *NewBB = BasicBlock::Create(OldBB->getContext(),
                                           OldBB->getName()+".thread",
                                           OldBB->getParent(), RegionTop);
    NewBB->moveAfter(PredBB);
    BlockMapping[OldBB] = NewBB;

    // Disallow threading across loop headers for newly created blocks. This is
    // an added safety measure to prevent out of control jump threading.
    BlockThreadCount[NewBB] = MaxThreadsPerBlock;

    // We are going to have to map operands from their original block 'OldBB' to
    // the new copy of the block 'NewBB'.
    BasicBlock::iterator BI = OldBB->begin();
    BasicBlock::iterator BE = OldBB->end();

    // Clone the instructions of OldBB into NewBB, keeping track of
    // the mapping. Delay remapping until all blocks in the region have been
    // cloned. That ensures all required cross-block mappings are available.
    // In RegionBottom, stop cloning at the terminator instruction.
    for (; BI != BE && (!BI->isTerminator() || OldBB != RegionBottom);
         ++BI) {
      Instruction *New = BI->clone();
      New->setName(BI->getName());
      NewBB->getInstList().push_back(New);
      ValueMapping[&*BI] = New;
    }
  }

  // Remap operands to patch up intra-thread-region references.
  for (auto OldBB : RegionBlocks) {
    BasicBlock *NewBB = BlockMapping[OldBB];
    BasicBlock *SubRegionPred = getSubRegionPred(OldBB, PredBB, RegionInfo);

    for (auto &New : *NewBB) {
      if (PHINode *PN = dyn_cast<PHINode>(&New)) {
        if (SubRegionPred) {
          // For region tops, reduce the PHIs to a single incoming value from
          // SubRegionPred. It is necessary to preserve the PHI, even though it
          // looks degenerate, so that we can run the SSAResolver separately
          // for the PHI and its operand.
          for (int i = PN->getNumIncomingValues() - 1; i >= 0; --i)
            if (PN->getIncomingBlock(i) != SubRegionPred)
              PN->removeIncomingValue(i);
            else if (OldBB != RegionTop)
              PN->setIncomingBlock(i, BlockMapping[PN->getIncomingBlock(i)]);
        }
        else {
          // Update intra-region PHI nodes. Any incoming value from a block
          // outside the region can be ignored, because those edges weren't
          // copied to the new threaded code. That includes edges from
          // RegionBottom, since the only edge from RegionBottom that gets
          // copied to the threaded region is RegionBottom-->SuccBB.
          for (int i = PN->getNumIncomingValues() - 1; i >= 0; --i) {
            DenseMap<BasicBlock*, BasicBlock*>::iterator I;
            I = BlockMapping.find(PN->getIncomingBlock(i));
            if (I != BlockMapping.end() &&
                PN->getIncomingBlock(i) != RegionBottom)
              PN->setIncomingBlock(i, BlockMapping[PN->getIncomingBlock(i)]);
            else
              PN->removeIncomingValue(i, /*DeletePHIIfEmpty*/false);
          }
        }

        // Since all PHI operands are cross-block references, there is no
        // sense trying to remap its operands. We'll always use SSAResolver to
        // rewrite its operands.
        continue;
      }

      for (unsigned i = 0, e = New.getNumOperands(); i != e; ++i)
        if (Instruction *Inst = dyn_cast<Instruction>(New.getOperand(i))) {
          // Only remap operands from the same basic block. We know these
          // operands should reference the new version of Inst. For any cross-BB
          // references, we use SSAUpdater to figure out what instruction(s)
          // can reach the use.
          if (Inst->getParent() == OldBB) {
            DenseMap<Instruction*, Value*>::iterator I;
            I = ValueMapping.find(Inst);
            assert (I != ValueMapping.end() &&
                    "Expected to find a mapping for Inst");
            New.setOperand(i, I->second);
          }
        }
        else if (BasicBlock *DestBB = dyn_cast<BasicBlock>(New.getOperand(i))) {
          BasicBlock *NewDestBB = DestBB;
          if (shouldRemapTarget(OldBB, DestBB, RegionInfo, BlockMapping)) {
            NewDestBB = BlockMapping[DestBB];
            New.setOperand(i, NewDestBB);
          }

          if (New.isTerminator())
            // We used to apply DT updates here, which was not correct,
            // because the updates may be submitted only after IR changes.
            DTUpdates.push_back({DominatorTree::Insert, NewBB, NewDestBB});

          // If we are threading across a loop header, we have to update the
          // LoopHeaders set. To do this precisely, we would need to re-run
          // FindLoopHeaders. Instead, we conservatively add any block that
          // *might* be a loop header in the new CFG, which means any block that
          // is a target of a new CFG edge. We catch most of them here.
          if (ThreadingLoopHeader)
            LoopHeaders.insert(DestBB);
        }
    }
  }

  // We didn't copy the terminator from RegionBottom over to its NewBB,
  // because there is now an unconditional jump to SuccBB. Insert the
  // unconditional jump.
  BranchInst *NewBI = BranchInst::Create(SuccBB, BlockMapping[RegionBottom]);
  NewBI->setDebugLoc(RegionBottom->getTerminator()->getDebugLoc());

  // Add the remaining loop header candidates here.
  if (ThreadingLoopHeader) {
    LoopHeaders.insert(SuccBB);
    LoopHeaders.insert(BlockMapping[RegionTop]);
  }

  // Check to see if any blocks that exit the thread region have PHI nodes.
  // If so, we need to add entries to the PHI nodes for the corresponding NewBB
  // now.
  for (auto OldBB : RegionBlocks) {
    // Ignore RegionBottom in this loop and handle it specially afterward. We
    // only need to update its SuccBB successor.
    if (OldBB == RegionBottom)
      continue;
    succ_iterator SI = succ_begin(OldBB), SE = succ_end(OldBB);
    for (; SI != SE; ++SI)
      if (!shouldRemapTarget(OldBB, *SI, RegionInfo, BlockMapping))
        AddPHINodeEntriesForMappedBlock(*SI, OldBB, BlockMapping[OldBB],
                                        ValueMapping);
  }
  AddPHINodeEntriesForMappedBlock(SuccBB, RegionBottom,
                                  BlockMapping[RegionBottom], ValueMapping);

  // This piece of code used to come after the SSA rewrite that happens next.
  // We need to replace the Pred-->BB CFG arc with Pred-->NewBB before we do
  // the SSA rewrite in order to correctly handle threading across loop
  // headers. For example,
  //
  // BB:
  //     %x = phi i32 [ %y, PredBB ] ...
  //     %a = ... %x ...
  //     %y = ...
  //     %b = ... %y ...
  //
  // NewBB will look like this after cloning, phi translation, & remapping
  // operands and prior to the SSA rewrite:
  //
  // NewBB:
  //     %a.1 = ... %y ...
  //     %y.1 = ...
  //     %b.1 = ... %y.1 ...
  //
  // Based on the CFG, the SSAUpdater code needs to rewrite the use of %y to
  // use the original value (%y), the cloned value (%y.1), or some phi derived
  // value. Without the PredBB-->NewBB edge, the SSAUpdater thinks NewBB has
  // no predecessors and replaces %y with undef.
  //
  // This situation is only possible when BB is a loop header.
  // Proof: As we remap operands in NewBB, uses of instructions from BB get
  //        remapped to use the corresponding instructions from NewBB. The
  //        exception is when the use comes from phi translation, i.e. when
  //        the incoming phi value from PredBB is an instruction from BB. In
  //        order for that to happen, BB must dominate PredBB. That makes
  //        PredBB-->BB a backedge, which puts BB in the LoopHeaders set.
  //
  // Update the terminator of PredBB to jump to the new thread head instead of
  // RegionTop.  This eliminates predecessors from RegionTop, which requires us
  // to simplify any PHI nodes in RegionTop.
  Instruction *PredTerm = PredBB->getTerminator();
  for (unsigned i = 0, e = PredTerm->getNumSuccessors(); i != e; ++i)
    if (PredTerm->getSuccessor(i) == RegionTop) {
      RegionTop->removePredecessor(PredBB, true);
      PredTerm->setSuccessor(i, BlockMapping[RegionTop]);
    }

  // Enqueue required DT updates.
  DTUpdates.append(
      {{DominatorTree::Insert, BlockMapping[RegionBottom], SuccBB},
       {DominatorTree::Insert, PredBB, BlockMapping[RegionTop]},
       {DominatorTree::Delete, PredBB, RegionTop}});
  DTU->applyUpdatesPermissive(DTUpdates);

  // Apply all updates we queued with DTU and get the updated Dominator Tree.
  DominatorTree *DT = &DTU->getDomTree();
  (void)DT;


  // If there were values defined in the region that are used outside the
  // region, then we now have to update all uses of the value to use either the
  // original value, the cloned value, or some PHI derived value. This can
  // require arbitrary PHI insertion, which we are prepared to do. Clean these
  // up now.
  SSAUpdater SSAUpdate;
  SmallVector<Use*, 16> UsesToRename;

  for (auto OldBB : RegionBlocks) {
    for (auto &I : *OldBB) {
      UsesToRename.clear();
      // Scan all uses of this instruction to see if it is used outside of its
      // block, and if so, record them in UsesToRename.
      for (Use &U : I.uses()) {
        Instruction *User = cast<Instruction>(U.getUser());
        // LLORG version uses "DT->dominates(&I, U)" as the right part of the
        // condition below becase it LLORG uses SSAUpdaterBulk. In xmain we
        // update values one by one (no bulk update) so the check is different.
        //
        // TODO: Verify that there are no hidden bugs due to that check here.
        // The test (PR37745.ll) passes in xmain but the issue might be just
        // hidden due to some reason.
        if (!isa<PHINode>(User) && User->getParent() == OldBB)
          continue;

        UsesToRename.push_back(&U);
      }

      // If there are no uses outside the block, we're done with this
      // instruction.
      if (UsesToRename.empty())
        continue;

      LLVM_DEBUG(dbgs() << "JT: Renaming non-local uses of: " << I << "\n");

      // We found a use of I outside the region.  Rename all uses of I that are
      // outside the region to be uses of the appropriate PHI node etc.  Seed
      // ValuesInBlocks with the two values we know.
      SSAUpdate.Initialize(I.getType(), I.getName());
      SSAUpdate.AddAvailableValue(OldBB, &I);

      // It is possible to have Phis in the exisiting block that are created
      // by the SSAUpdate due to the new live-ins from the threaded region
      // and these Phis might not be required in the threaded-block. Therefore,
      // we might not have mapped values for these Phis in ValueMapping.
      // For reference: CMPLRS-4877; test_4877 in intel_loop_headers.ll;
      if (ValueMapping.find(&I) != ValueMapping.end())
        SSAUpdate.AddAvailableValue(BlockMapping[OldBB], ValueMapping[&I]);

      for (auto U : UsesToRename)
        SSAUpdate.RewriteUse(*U);
      LLVM_DEBUG(dbgs() << "\n");
    }
  }

  // At this point, the IR is fully up to date and consistent.  Do a quick scan
  // over the new instructions and zap any that are constants or dead.  This
  // frequently happens because of phi translation.
  for (auto OldBB : RegionBlocks)
    SimplifyInstructionsInBlock(BlockMapping[OldBB], TLI);

  // Update the edge weights and block frequencies.
  UpdateRegionBlockFreqAndEdgeWeight(PredBB, SuccBB, RegionInfo, RegionBlocks,
                                     BlockMapping);

  // Threaded an edge!
  ++BlockThreadCount[RegionBottom];
  ++NumThreads;
  return true;
}
#endif // INTEL_CUSTOMIZATION

/// Create a new basic block that will be the predecessor of BB and successor of
/// all blocks in Preds. When profile data is available, update the frequency of
/// this new block.
BasicBlock *JumpThreadingPass::SplitBlockPreds(BasicBlock *BB,
                                               ArrayRef<BasicBlock *> Preds,
                                               const char *Suffix) {
  SmallVector<BasicBlock *, 2> NewBBs;

  // Collect the frequencies of all predecessors of BB, which will be used to
  // update the edge weight of the result of splitting predecessors.
  DenseMap<BasicBlock *, BlockFrequency> FreqMap;
  if (HasProfileData)
    for (auto Pred : Preds)
      FreqMap.insert(std::make_pair(
          Pred, BFI->getBlockFreq(Pred) * BPI->getEdgeProbability(Pred, BB)));

  // In the case when BB is a LandingPad block we create 2 new predecessors
  // instead of just one.
  if (BB->isLandingPad()) {
    std::string NewName = std::string(Suffix) + ".split-lp";
    SplitLandingPadPredecessors(BB, Preds, Suffix, NewName.c_str(), NewBBs);
  } else {
    NewBBs.push_back(SplitBlockPredecessors(BB, Preds, Suffix));
  }

  std::vector<DominatorTree::UpdateType> Updates;
  Updates.reserve((2 * Preds.size()) + NewBBs.size());
  for (auto NewBB : NewBBs) {
    BlockFrequency NewBBFreq(0);
    Updates.push_back({DominatorTree::Insert, NewBB, BB});
    for (auto Pred : predecessors(NewBB)) {
      Updates.push_back({DominatorTree::Delete, Pred, BB});
      Updates.push_back({DominatorTree::Insert, Pred, NewBB});
      if (HasProfileData) // Update frequencies between Pred -> NewBB.
        NewBBFreq += FreqMap.lookup(Pred);
    }
    if (HasProfileData) // Apply the summed frequency to NewBB.
      BFI->setBlockFreq(NewBB, NewBBFreq.getFrequency());
  }

  DTU->applyUpdatesPermissive(Updates);
  return NewBBs[0];
}

bool JumpThreadingPass::doesBlockHaveProfileData(BasicBlock *BB) {
  const Instruction *TI = BB->getTerminator();
  assert(TI->getNumSuccessors() > 1 && "not a split");

  MDNode *WeightsNode = TI->getMetadata(LLVMContext::MD_prof);
  if (!WeightsNode)
    return false;

  MDString *MDName = cast<MDString>(WeightsNode->getOperand(0));
  if (MDName->getString() != "branch_weights")
    return false;

  // Ensure there are weights for all of the successors. Note that the first
  // operand to the metadata node is a name, not a weight.
  return WeightsNode->getNumOperands() == TI->getNumSuccessors() + 1;
}

#if INTEL_CUSTOMIZATION
/// This routine was significantly refactored to support multi-BB thread
/// regions. Update the block frequencies and edge weights for all new blocks
/// in the region. Also, update (reduce) the block frequencies of the original
/// blocks, and update the edge frequencies out of RegionBottom, since the
/// RegionBottom->SuccBB edge weight needs to be reduced.
void JumpThreadingPass::UpdateRegionBlockFreqAndEdgeWeight(BasicBlock *PredBB,
                                                           BasicBlock *SuccBB,
                       const ThreadRegionInfo &RegionInfo,
                       const SmallVectorImpl<BasicBlock*> &RegionBlocks,
                       DenseMap<BasicBlock*, BasicBlock*> &BlockMapping) {
  if (!HasProfileData)
    return;

  assert(BFI && BPI && "BFI & BPI should have been created here");

  BasicBlock *RegionTop = RegionInfo.back().first;
  BasicBlock *RegionBottom = RegionInfo.front().second;
  DenseMap<BasicBlock*, int> BlockPredCount;

  // Initialize BlockPredCount for each new block.
  for (auto BB : RegionBlocks) {
    BasicBlock *NewBB = BlockMapping[BB];
    BlockPredCount[NewBB] = std::distance(pred_begin(NewBB), pred_end(NewBB));
  }

  // Seed the algorithm by computing the block Freq of RegionTop and adding
  // it to the ready block list. The ready block list holds the *original*
  // blocks whose corresponding new block frequencies have been finalized.
  BasicBlock *NewRegionTop = BlockMapping[RegionTop];
  SmallVector<BasicBlock*, 16> ReadyBlocks;
  auto NewRegionTopFreq =
    BFI->getBlockFreq(PredBB) * BPI->getEdgeProbability(PredBB, NewRegionTop);
  BFI->setBlockFreq(NewRegionTop, NewRegionTopFreq.getFrequency());
  ReadyBlocks.push_back(RegionTop);

  while (!ReadyBlocks.empty()) {
    BasicBlock *BB = ReadyBlocks.back();
    BasicBlock *NewBB = BlockMapping[BB];
    ReadyBlocks.pop_back();

    // Since NewBB is partially replacing BB, we have to update the block
    // frequency of BB.
    auto BBOrigFreq = BFI->getBlockFreq(BB);
    auto NewBBFreq = BFI->getBlockFreq(NewBB);
    auto BBNewFreq = BBOrigFreq - NewBBFreq;
    BFI->setBlockFreq(BB, BBNewFreq.getFrequency());

    // The bottom block in the region needs special treatment. We don't have to
    // worry about the outgoing edge weights for NewBB since it now ends in an
    // unconditional branch, but we do need to adjust the edge weights on exit
    // from BB, since the BB->SuccBB edge frequency dropped by the NewBB->SuccBB
    // edge frequency.
    if (BB == RegionBottom) {
      // Collect updated outgoing edges' frequencies from BB and use them to
      // update edge probabilities.
      SmallVector<uint64_t, 4> BBSuccFreq;
      for (BasicBlock *Succ : successors(BB)) {
        auto SuccFreq = BBOrigFreq * BPI->getEdgeProbability(BB, Succ);
        if (Succ == SuccBB)
          SuccFreq -= NewBBFreq;
        BBSuccFreq.push_back(SuccFreq.getFrequency());
      }

      uint64_t MaxBBSuccFreq =
        *std::max_element(BBSuccFreq.begin(), BBSuccFreq.end());

      SmallVector<BranchProbability, 4> BBSuccProbs;
      if (MaxBBSuccFreq == 0)
        BBSuccProbs.assign(BBSuccFreq.size(),
                           {1, static_cast<unsigned>(BBSuccFreq.size())});
      else {
        for (uint64_t Freq : BBSuccFreq)
          BBSuccProbs.push_back(
            BranchProbability::getBranchProbability(Freq, MaxBBSuccFreq));
        // Normalize edge probabilities so that they sum up to one.
        BranchProbability::normalizeProbabilities(BBSuccProbs.begin(),
                                                  BBSuccProbs.end());
      }

      // Update edge probabilities in BPI.
      for (int I = 0, E = BBSuccProbs.size(); I < E; I++)
        BPI->setEdgeProbability(BB, I, BBSuccProbs[I]);

      // Update the profile metadata as well.
      //
      // Don't do this if the profile of the transformed blocks was statically
      // estimated.  (This could occur despite the function having an entry
      // frequency in completely cold parts of the CFG.)
      //
      // In this case we don't want to suggest to subsequent passes that the
      // calculated weights are fully consistent.  Consider this graph:
      //
      //                 check_1
      //             50% /  |
      //             eq_1   | 50%
      //                 \  |
      //                 check_2
      //             50% /  |
      //             eq_2   | 50%
      //                 \  |
      //                 check_3
      //             50% /  |
      //             eq_3   | 50%
      //                 \  |
      //
      // Assuming the blocks check_* all compare the same value against 1, 2
      // and 3, the overall probabilities are inconsistent; the total
      // probability that the value is either 1, 2 or 3 is 150%.
      //
      // As a consequence if we thread eq_1 -> check_2 to check_3,
      // check_2->check_3 becomes 0%.  This is even worse if the edge whose
      // probability becomes 0% is the loop exit edge.  Then based solely on
      // static estimation we would assume the loop was extremely hot.
      //
      // FIXME this locally as well so that BPI and BFI are consistent as well.
      // We shouldn't make edges extremely likely or unlikely based solely on
      // static estimation.
      if (BBSuccProbs.size() >= 2 && doesBlockHaveProfileData(BB)) {
        SmallVector<uint32_t, 4> Weights;
        for (auto Prob : BBSuccProbs)
          Weights.push_back(Prob.getNumerator());

        auto TI = BB->getTerminator();
        TI->setMetadata(
          LLVMContext::MD_prof,
          MDBuilder(BB->getContext()).createBranchWeights(Weights));
      }

      continue;
    }

    unsigned SuccIndex = 0;
    for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE;
         ++SI, ++SuccIndex) {
      // The outgoing edge weights for NewBB are copied from BB.
      BPI->setEdgeProbability(NewBB, SuccIndex,
                              BPI->getEdgeProbability(BB, SuccIndex));

      // Adjust the block frequency of the successor if it's part of the region.
      if (BlockMapping.find(*SI) == BlockMapping.end())
        continue;

      BasicBlock *NewSucc = BlockMapping[*SI];
      auto EdgeFreq =
        BFI->getBlockFreq(NewBB) * BPI->getEdgeProbability(BB, SuccIndex);
      auto NewSuccFreq = BFI->getBlockFreq(NewSucc) + EdgeFreq;
      BFI->setBlockFreq(NewSucc, NewSuccFreq.getFrequency());

      if (--BlockPredCount[NewSucc] == 0)
        ReadyBlocks.push_back(*SI);
    }
  }
}
#endif // INTEL_CUSTOMIZATION

/// DuplicateCondBranchOnPHIIntoPred - PredBB contains an unconditional branch
/// to BB which contains an i1 PHI node and a conditional branch on that PHI.
/// If we can duplicate the contents of BB up into PredBB do so now, this
/// improves the odds that the branch will be on an analyzable instruction like
/// a compare.
bool JumpThreadingPass::DuplicateCondBranchOnPHIIntoPred(
    BasicBlock *BB, const SmallVectorImpl<BasicBlock *> &PredBBs) {
  assert(!PredBBs.empty() && "Can't handle an empty set");

  // If BB is a loop header, then duplicating this block outside the loop would
  // cause us to transform this into an irreducible loop, don't do this.
  // See the comments above FindLoopHeaders for justifications and caveats.
  if (LoopHeaders.count(BB)) {
    LLVM_DEBUG(dbgs() << "  Not duplicating loop header '" << BB->getName()
                      << "' into predecessor block '" << PredBBs[0]->getName()
                      << "' - it might create an irreducible loop!\n");
    return false;
  }

#if INTEL_CUSTOMIZATION
  // Initially, I enabled this transformation for loop headers under
  // JumpThreadLoopHeader, but that caused out-of-control jump threading,
  // because we were failing to properly update the LoopHeaders set after making
  // this transformation (see cq379894). We may choose to enable this later as
  // it should be fairly simple to add the necessary BB(s) to LoopHeaders, but
  // since it isn't needed for CoreMark, I am simply disabling this for
  // LoopHeaders for now.
  //
  // Another problem with performing this optimization on a loop header is
  // that the transformed code can end up identical to the original code,
  // which would prevent the optimization pass from ever converging! This
  // happens in cases like this:
  //
  //    bb1:
  //      br label %bb2
  //    bb2:
  //      %x = phi i1 [ %x, %bb2 ], [ true, %bb1 ]
  //      br i1 %x, label %bb2, label %bb3
  //
  // After duplicating bb2 into bb1 and doing PHI translation & instruction
  // simplification, bb1 ends up looking exactly the same. So suppress this
  // optimization on single basic block loops for safety.
  pred_iterator PB = pred_begin(BB), PE = pred_end(BB);
  for (pred_iterator PI = PB; PI != PE; ++PI)
    if (*PI == BB) {
      LLVM_DEBUG(
          dbgs() << "  Not duplicating BB '" << BB->getName()
                 << "' into predecessor block '" << PredBBs[0]->getName()
                 << "' - it might prevent jump threading from converging!\n");
      return false;
    }
#endif // INTEL_CUSTOMIZATION

  SmallVector<BasicBlock*, 1> RegionBlocks;                             // INTEL
  RegionBlocks.push_back(BB);                                           // INTEL
  unsigned DuplicationCost =                                            // INTEL
    getJumpThreadDuplicationCost(RegionBlocks, BB, BBDupThreshold);     // INTEL
  if (DuplicationCost > BBDupThreshold) {
    LLVM_DEBUG(dbgs() << "  Not duplicating BB '" << BB->getName()
                      << "' - Cost is too high: " << DuplicationCost << "\n");
    return false;
  }

  // And finally, do it!  Start by factoring the predecessors if needed.
  std::vector<DominatorTree::UpdateType> Updates;
  BasicBlock *PredBB;
  if (PredBBs.size() == 1)
    PredBB = PredBBs[0];
  else {
    LLVM_DEBUG(dbgs() << "  Factoring out " << PredBBs.size()
                      << " common predecessors.\n");
    PredBB = SplitBlockPreds(BB, PredBBs, ".thr_comm");
  }
  Updates.push_back({DominatorTree::Delete, PredBB, BB});

  // Okay, we decided to do this!  Clone all the instructions in BB onto the end
  // of PredBB.
  LLVM_DEBUG(dbgs() << "  Duplicating block '" << BB->getName()
                    << "' into end of '" << PredBB->getName()
                    << "' to eliminate branch on phi.  Cost: "
                    << DuplicationCost << " block is:" << *BB << "\n");

  // Unless PredBB ends with an unconditional branch, split the edge so that we
  // can just clone the bits from BB into the end of the new PredBB.
  BranchInst *OldPredBranch = dyn_cast<BranchInst>(PredBB->getTerminator());

  if (!OldPredBranch || !OldPredBranch->isUnconditional()) {
    BasicBlock *OldPredBB = PredBB;
    PredBB = SplitEdge(OldPredBB, BB);
    Updates.push_back({DominatorTree::Insert, OldPredBB, PredBB});
    Updates.push_back({DominatorTree::Insert, PredBB, BB});
    Updates.push_back({DominatorTree::Delete, OldPredBB, BB});
    OldPredBranch = cast<BranchInst>(PredBB->getTerminator());
  }

  // We are going to have to map operands from the original BB block into the
  // PredBB block.  Evaluate PHI nodes in BB.
  DenseMap<Instruction*, Value*> ValueMapping;

  BasicBlock::iterator BI = BB->begin();
  for (; PHINode *PN = dyn_cast<PHINode>(BI); ++BI)
    ValueMapping[PN] = PN->getIncomingValueForBlock(PredBB);
  // Clone the non-phi instructions of BB into PredBB, keeping track of the
  // mapping and using it to remap operands in the cloned instructions.
  for (; BI != BB->end(); ++BI) {
    Instruction *New = BI->clone();

    // Remap operands to patch up intra-block references.
    for (unsigned i = 0, e = New->getNumOperands(); i != e; ++i)
      if (Instruction *Inst = dyn_cast<Instruction>(New->getOperand(i))) {
        DenseMap<Instruction*, Value*>::iterator I = ValueMapping.find(Inst);
        if (I != ValueMapping.end())
          New->setOperand(i, I->second);
      }

    // If this instruction can be simplified after the operands are updated,
    // just use the simplified value instead.  This frequently happens due to
    // phi translation.
    if (Value *IV = SimplifyInstruction(
            New,
            {BB->getModule()->getDataLayout(), TLI, nullptr, nullptr, New})) {
      ValueMapping[&*BI] = IV;
      if (!New->mayHaveSideEffects()) {
        New->deleteValue();
        New = nullptr;
      }
    } else {
      ValueMapping[&*BI] = New;
    }
    if (New) {
      // Otherwise, insert the new instruction into the block.
      New->setName(BI->getName());
      PredBB->getInstList().insert(OldPredBranch->getIterator(), New);
      // Update Dominance from simplified New instruction operands.
      for (unsigned i = 0, e = New->getNumOperands(); i != e; ++i)
        if (BasicBlock *SuccBB = dyn_cast<BasicBlock>(New->getOperand(i)))
          Updates.push_back({DominatorTree::Insert, PredBB, SuccBB});
    }
  }

  // Check to see if the targets of the branch had PHI nodes. If so, we need to
  // add entries to the PHI nodes for branch from PredBB now.
  BranchInst *BBBranch = cast<BranchInst>(BB->getTerminator());
  AddPHINodeEntriesForMappedBlock(BBBranch->getSuccessor(0), BB, PredBB,
                                  ValueMapping);
  AddPHINodeEntriesForMappedBlock(BBBranch->getSuccessor(1), BB, PredBB,
                                  ValueMapping);

  // If there were values defined in BB that are used outside the block, then we
  // now have to update all uses of the value to use either the original value,
  // the cloned value, or some PHI derived value.  This can require arbitrary
  // PHI insertion, of which we are prepared to do, clean these up now.
  SSAUpdater SSAUpdate;
  SmallVector<Use*, 16> UsesToRename;
  for (Instruction &I : *BB) {
    // Scan all uses of this instruction to see if it is used outside of its
    // block, and if so, record them in UsesToRename.
    for (Use &U : I.uses()) {
      Instruction *User = cast<Instruction>(U.getUser());
      if (PHINode *UserPN = dyn_cast<PHINode>(User)) {
        if (UserPN->getIncomingBlock(U) == BB)
          continue;
      } else if (User->getParent() == BB)
        continue;

      UsesToRename.push_back(&U);
    }

    // If there are no uses outside the block, we're done with this instruction.
    if (UsesToRename.empty())
      continue;

    LLVM_DEBUG(dbgs() << "JT: Renaming non-local uses of: " << I << "\n");

    // We found a use of I outside of BB.  Rename all uses of I that are outside
    // its block to be uses of the appropriate PHI node etc.  See ValuesInBlocks
    // with the two values we know.
    SSAUpdate.Initialize(I.getType(), I.getName());
    SSAUpdate.AddAvailableValue(BB, &I);
    SSAUpdate.AddAvailableValue(PredBB, ValueMapping[&I]);

    while (!UsesToRename.empty())
      SSAUpdate.RewriteUse(*UsesToRename.pop_back_val());
    LLVM_DEBUG(dbgs() << "\n");
  }

  // PredBB no longer jumps to BB, remove entries in the PHI node for the edge
  // that we nuked.
  BB->removePredecessor(PredBB, true);

  // Remove the unconditional branch at the end of the PredBB block.
  OldPredBranch->eraseFromParent();
  DTU->applyUpdatesPermissive(Updates);

  ++NumDupes;
  return true;
}

// Pred is a predecessor of BB with an unconditional branch to BB. SI is
// a Select instruction in Pred. BB has other predecessors and SI is used in
// a PHI node in BB. SI has no other use.
// A new basic block, NewBB, is created and SI is converted to compare and 
// conditional branch. SI is erased from parent.
void JumpThreadingPass::UnfoldSelectInstr(BasicBlock *Pred, BasicBlock *BB,
                                          SelectInst *SI, PHINode *SIUse,
                                          unsigned Idx) {
  // Expand the select.
  //
  // Pred --
  //  |    v
  //  |  NewBB
  //  |    |
  //  |-----
  //  v
  // BB
  BranchInst *PredTerm = dyn_cast<BranchInst>(Pred->getTerminator());
  BasicBlock *NewBB = BasicBlock::Create(BB->getContext(), "select.unfold",
                                         BB->getParent(), BB);
  // Move the unconditional branch to NewBB.
  PredTerm->removeFromParent();
  NewBB->getInstList().insert(NewBB->end(), PredTerm);
  // Create a conditional branch and update PHI nodes.
  BranchInst::Create(NewBB, BB, SI->getCondition(), Pred);
  SIUse->setIncomingValue(Idx, SI->getFalseValue());
  SIUse->addIncoming(SI->getTrueValue(), NewBB);

  // The select is now dead.
  SI->eraseFromParent();
  DTU->applyUpdatesPermissive({{DominatorTree::Insert, NewBB, BB},
                               {DominatorTree::Insert, Pred, NewBB}});

  // Update any other PHI nodes in BB.
  for (BasicBlock::iterator BI = BB->begin();
       PHINode *Phi = dyn_cast<PHINode>(BI); ++BI)
    if (Phi != SIUse)
      Phi->addIncoming(Phi->getIncomingValueForBlock(Pred), NewBB);
}

bool JumpThreadingPass::TryToUnfoldSelect(SwitchInst *SI, BasicBlock *BB) {
  PHINode *CondPHI = dyn_cast<PHINode>(SI->getCondition());

  if (!CondPHI || CondPHI->getParent() != BB)
    return false;

  for (unsigned I = 0, E = CondPHI->getNumIncomingValues(); I != E; ++I) {
    BasicBlock *Pred = CondPHI->getIncomingBlock(I);
    SelectInst *PredSI = dyn_cast<SelectInst>(CondPHI->getIncomingValue(I));

    // The second and third condition can be potentially relaxed. Currently
    // the conditions help to simplify the code and allow us to reuse existing
    // code, developed for TryToUnfoldSelect(CmpInst *, BasicBlock *)
    if (!PredSI || PredSI->getParent() != Pred || !PredSI->hasOneUse())
      continue;

    BranchInst *PredTerm = dyn_cast<BranchInst>(Pred->getTerminator());
    if (!PredTerm || !PredTerm->isUnconditional())
      continue;

    UnfoldSelectInstr(Pred, BB, PredSI, CondPHI, I);
    return true;
  }
  return false;
}

/// TryToUnfoldSelect - Look for blocks of the form
/// bb1:
///   %a = select
///   br bb2
///
/// bb2:
///   %p = phi [%a, %bb1] ...
///   %c = icmp %p
///   br i1 %c
///
/// And expand the select into a branch structure if one of its arms allows %c
/// to be folded. This later enables threading from bb1 over bb2.
bool JumpThreadingPass::TryToUnfoldSelect(CmpInst *CondCmp, BasicBlock *BB) {
  BranchInst *CondBr = dyn_cast<BranchInst>(BB->getTerminator());
  PHINode *CondLHS = dyn_cast<PHINode>(CondCmp->getOperand(0));
  Constant *CondRHS = cast<Constant>(CondCmp->getOperand(1));

  if (!CondBr || !CondBr->isConditional() || !CondLHS ||
      CondLHS->getParent() != BB)
    return false;

  for (unsigned I = 0, E = CondLHS->getNumIncomingValues(); I != E; ++I) {
    BasicBlock *Pred = CondLHS->getIncomingBlock(I);
    SelectInst *SI = dyn_cast<SelectInst>(CondLHS->getIncomingValue(I));

    // Look if one of the incoming values is a select in the corresponding
    // predecessor.
    if (!SI || SI->getParent() != Pred || !SI->hasOneUse())
      continue;

    BranchInst *PredTerm = dyn_cast<BranchInst>(Pred->getTerminator());
    if (!PredTerm || !PredTerm->isUnconditional())
      continue;

    // Now check if one of the select values would allow us to constant fold the
    // terminator in BB. We don't do the transform if both sides fold, those
    // cases will be threaded in any case.
    if (DTU->hasPendingDomTreeUpdates())
      LVI->disableDT();
    else
      LVI->enableDT();
    LazyValueInfo::Tristate LHSFolds =
        LVI->getPredicateOnEdge(CondCmp->getPredicate(), SI->getOperand(1),
                                CondRHS, Pred, BB, CondCmp);
    LazyValueInfo::Tristate RHSFolds =
        LVI->getPredicateOnEdge(CondCmp->getPredicate(), SI->getOperand(2),
                                CondRHS, Pred, BB, CondCmp);
    if ((LHSFolds != LazyValueInfo::Unknown ||
         RHSFolds != LazyValueInfo::Unknown) &&
        LHSFolds != RHSFolds) {
      UnfoldSelectInstr(Pred, BB, SI, CondLHS, I);
      return true;
    }
  }
  return false;
}

/// TryToUnfoldSelectInCurrBB - Look for PHI/Select or PHI/CMP/Select in the
/// same BB in the form
/// bb:
///   %p = phi [false, %bb1], [true, %bb2], [false, %bb3], [true, %bb4], ...
///   %s = select %p, trueval, falseval
///
/// or
///
/// bb:
///   %p = phi [0, %bb1], [1, %bb2], [0, %bb3], [1, %bb4], ...
///   %c = cmp %p, 0
///   %s = select %c, trueval, falseval
///
/// And expand the select into a branch structure. This later enables
/// jump-threading over bb in this pass.
///
/// Using the similar approach of SimplifyCFG::FoldCondBranchOnPHI(), unfold
/// select if the associated PHI has at least one constant.  If the unfolded
/// select is not jump-threaded, it will be folded again in the later
/// optimizations.
bool JumpThreadingPass::TryToUnfoldSelectInCurrBB(BasicBlock *BB) {
  // If threading this would thread across a loop header, don't thread the edge.
  // See the comments above FindLoopHeaders for justifications and caveats.
  if (LoopHeaders.count(BB))
    return false;

  for (BasicBlock::iterator BI = BB->begin();
       PHINode *PN = dyn_cast<PHINode>(BI); ++BI) {
    // Look for a Phi having at least one constant incoming value.
    if (llvm::all_of(PN->incoming_values(),
                     [](Value *V) { return !isa<ConstantInt>(V); }))
      continue;

    auto isUnfoldCandidate = [BB](SelectInst *SI, Value *V) {
      // Check if SI is in BB and use V as condition.
      if (SI->getParent() != BB)
        return false;
      Value *Cond = SI->getCondition();
      return (Cond && Cond == V && Cond->getType()->isIntegerTy(1));
    };

    SelectInst *SI = nullptr;
    for (Use &U : PN->uses()) {
      if (ICmpInst *Cmp = dyn_cast<ICmpInst>(U.getUser())) {
        // Look for a ICmp in BB that compares PN with a constant and is the
        // condition of a Select.
        if (Cmp->getParent() == BB && Cmp->hasOneUse() &&
            isa<ConstantInt>(Cmp->getOperand(1 - U.getOperandNo())))
          if (SelectInst *SelectI = dyn_cast<SelectInst>(Cmp->user_back()))
            if (isUnfoldCandidate(SelectI, Cmp->use_begin()->get())) {
              SI = SelectI;
              break;
            }
      } else if (SelectInst *SelectI = dyn_cast<SelectInst>(U.getUser())) {
        // Look for a Select in BB that uses PN as condition.
        if (isUnfoldCandidate(SelectI, U.get())) {
          SI = SelectI;
          break;
        }
      }
    }

    if (!SI)
      continue;
    // Expand the select.
    Instruction *Term =
        SplitBlockAndInsertIfThen(SI->getCondition(), SI, false);
    BasicBlock *SplitBB = SI->getParent();
    BasicBlock *NewBB = Term->getParent();
    PHINode *NewPN = PHINode::Create(SI->getType(), 2, "", SI);
    NewPN->addIncoming(SI->getTrueValue(), Term->getParent());
    NewPN->addIncoming(SI->getFalseValue(), BB);
    SI->replaceAllUsesWith(NewPN);
    SI->eraseFromParent();
    // NewBB and SplitBB are newly created blocks which require insertion.
    std::vector<DominatorTree::UpdateType> Updates;
    Updates.reserve((2 * SplitBB->getTerminator()->getNumSuccessors()) + 3);
    Updates.push_back({DominatorTree::Insert, BB, SplitBB});
    Updates.push_back({DominatorTree::Insert, BB, NewBB});
    Updates.push_back({DominatorTree::Insert, NewBB, SplitBB});
    // BB's successors were moved to SplitBB, update DTU accordingly.
    for (auto *Succ : successors(SplitBB)) {
      Updates.push_back({DominatorTree::Delete, BB, Succ});
      Updates.push_back({DominatorTree::Insert, SplitBB, Succ});
    }
    DTU->applyUpdatesPermissive(Updates);
    return true;
  }
  return false;
}

/// Try to propagate a guard from the current BB into one of its predecessors
/// in case if another branch of execution implies that the condition of this
/// guard is always true. Currently we only process the simplest case that
/// looks like:
///
/// Start:
///   %cond = ...
///   br i1 %cond, label %T1, label %F1
/// T1:
///   br label %Merge
/// F1:
///   br label %Merge
/// Merge:
///   %condGuard = ...
///   call void(i1, ...) @llvm.experimental.guard( i1 %condGuard )[ "deopt"() ]
///
/// And cond either implies condGuard or !condGuard. In this case all the
/// instructions before the guard can be duplicated in both branches, and the
/// guard is then threaded to one of them.
bool JumpThreadingPass::ProcessGuards(BasicBlock *BB) {
  using namespace PatternMatch;

  // We only want to deal with two predecessors.
  BasicBlock *Pred1, *Pred2;
  auto PI = pred_begin(BB), PE = pred_end(BB);
  if (PI == PE)
    return false;
  Pred1 = *PI++;
  if (PI == PE)
    return false;
  Pred2 = *PI++;
  if (PI != PE)
    return false;
  if (Pred1 == Pred2)
    return false;

  // Try to thread one of the guards of the block.
  // TODO: Look up deeper than to immediate predecessor?
  auto *Parent = Pred1->getSinglePredecessor();
  if (!Parent || Parent != Pred2->getSinglePredecessor())
    return false;

  if (auto *BI = dyn_cast<BranchInst>(Parent->getTerminator()))
    for (auto &I : *BB)
      if (isGuard(&I) && ThreadGuard(BB, cast<IntrinsicInst>(&I), BI))
        return true;

  return false;
}

/// Try to propagate the guard from BB which is the lower block of a diamond
/// to one of its branches, in case if diamond's condition implies guard's
/// condition.
bool JumpThreadingPass::ThreadGuard(BasicBlock *BB, IntrinsicInst *Guard,
                                    BranchInst *BI) {
  assert(BI->getNumSuccessors() == 2 && "Wrong number of successors?");
  assert(BI->isConditional() && "Unconditional branch has 2 successors?");
  Value *GuardCond = Guard->getArgOperand(0);
  Value *BranchCond = BI->getCondition();
  BasicBlock *TrueDest = BI->getSuccessor(0);
  BasicBlock *FalseDest = BI->getSuccessor(1);

  auto &DL = BB->getModule()->getDataLayout();
  bool TrueDestIsSafe = false;
  bool FalseDestIsSafe = false;

  // True dest is safe if BranchCond => GuardCond.
  auto Impl = isImpliedCondition(BranchCond, GuardCond, DL);
  if (Impl && *Impl)
    TrueDestIsSafe = true;
  else {
    // False dest is safe if !BranchCond => GuardCond.
    Impl = isImpliedCondition(BranchCond, GuardCond, DL, /* LHSIsTrue */ false);
    if (Impl && *Impl)
      FalseDestIsSafe = true;
  }

  if (!TrueDestIsSafe && !FalseDestIsSafe)
    return false;

  BasicBlock *PredUnguardedBlock = TrueDestIsSafe ? TrueDest : FalseDest;
  BasicBlock *PredGuardedBlock = FalseDestIsSafe ? TrueDest : FalseDest;

  ValueToValueMapTy UnguardedMapping, GuardedMapping;
  Instruction *AfterGuard = Guard->getNextNode();
  SmallVector<BasicBlock*, 1> RegionBlocks;                             // INTEL
  RegionBlocks.push_back(BB);                                           // INTEL
  unsigned Cost =                                                       // INTEL
      getJumpThreadDuplicationCost(RegionBlocks, BB, BBDupThreshold);   // INTEL
  if (Cost > BBDupThreshold)
    return false;
  // Duplicate all instructions before the guard and the guard itself to the
  // branch where implication is not proved.
  BasicBlock *GuardedBlock = DuplicateInstructionsInSplitBetween(
      BB, PredGuardedBlock, AfterGuard, GuardedMapping, *DTU);
  assert(GuardedBlock && "Could not create the guarded block?");
  // Duplicate all instructions before the guard in the unguarded branch.
  // Since we have successfully duplicated the guarded block and this block
  // has fewer instructions, we expect it to succeed.
  BasicBlock *UnguardedBlock = DuplicateInstructionsInSplitBetween(
      BB, PredUnguardedBlock, Guard, UnguardedMapping, *DTU);
  assert(UnguardedBlock && "Could not create the unguarded block?");
  LLVM_DEBUG(dbgs() << "Moved guard " << *Guard << " to block "
                    << GuardedBlock->getName() << "\n");
  // Some instructions before the guard may still have uses. For them, we need
  // to create Phi nodes merging their copies in both guarded and unguarded
  // branches. Those instructions that have no uses can be just removed.
  SmallVector<Instruction *, 4> ToRemove;
  for (auto BI = BB->begin(); &*BI != AfterGuard; ++BI)
    if (!isa<PHINode>(&*BI))
      ToRemove.push_back(&*BI);

  Instruction *InsertionPoint = &*BB->getFirstInsertionPt();
  assert(InsertionPoint && "Empty block?");
  // Substitute with Phis & remove.
  for (auto *Inst : reverse(ToRemove)) {
    if (!Inst->use_empty()) {
      PHINode *NewPN = PHINode::Create(Inst->getType(), 2);
      NewPN->addIncoming(UnguardedMapping[Inst], UnguardedBlock);
      NewPN->addIncoming(GuardedMapping[Inst], GuardedBlock);
      NewPN->insertBefore(InsertionPoint);
      Inst->replaceAllUsesWith(NewPN);
    }
    Inst->eraseFromParent();
  }
  return true;
}
