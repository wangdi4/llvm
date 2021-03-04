//===------------ Intel_VPOParoptSharedPrivatization.cpp ------------------===//
//
//   Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements optimization pass that privatizes shared items in work
/// regions where it is safe to do so.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptSharedPrivatization.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-shared-privatization"
#define PASS_NAME "VPO Paropt Shared Privatization Pass"

static bool privatizeSharedItems(Function &F, WRegionInfo &WI,
                                 OptimizationRemarkEmitter &ORE) {
  bool Changed = false;

  // Walk the W-Region Graph top-down, and create W-Region List
  WI.buildWRGraph();

  if (WI.WRGraphIsEmpty()) {
    LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Shared Privatization \n");
    return Changed;
  }

  LLVM_DEBUG(WI.print(dbgs()));
  LLVM_DEBUG(dbgs() << PASS_NAME << " for Function: ");
  LLVM_DEBUG(dbgs().write_escaped(F.getName()) << '\n');

  VPOParoptTransform VP(nullptr, &F, &WI, WI.getDomTree(), WI.getLoopInfo(),
                        WI.getSE(), WI.getTargetTransformInfo(),
                        WI.getAssumptionCache(), WI.getTargetLibraryInfo(),
                        WI.getAliasAnalysis(), OmpNoFECollapse,
                        OptReportVerbosity::None, ORE, 2, false);

  Changed |= VP.privatizeSharedItems();

  return Changed;
}

PreservedAnalyses
VPOParoptSharedPrivatizationPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);
  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);

  PreservedAnalyses PA;

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  if (!privatizeSharedItems(F, WI, ORE))
    PA = PreservedAnalyses::all();
  else
    PA = PreservedAnalyses::none();
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");

  return PA;
}

namespace {

class VPOParoptSharedPrivatization : public FunctionPass {
public:
  static char ID;

  VPOParoptSharedPrivatization() : FunctionPass(ID) {
    initializeVPOParoptSharedPrivatizationPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
    auto &ORE = getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();

    LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
    bool Changed = privatizeSharedItems(F, WI, ORE);
    LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WRegionInfoWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
  }
};

} // end anonymous namespace

char VPOParoptSharedPrivatization::ID = 0;
INITIALIZE_PASS_BEGIN(VPOParoptSharedPrivatization, DEBUG_TYPE, PASS_NAME,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_END(VPOParoptSharedPrivatization, DEBUG_TYPE, PASS_NAME, false,
                    false)

FunctionPass *llvm::createVPOParoptSharedPrivatizationPass() {
  return new VPOParoptSharedPrivatization();
}

bool VPOParoptTransform::privatizeSharedItems(WRegionNode *W) {
  if (!W->canHaveShared() || !W->needsOutlining())
    return false;

  W->populateBBSet();

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::privatizeSharedItems: "
                    << W->getName() << "\n");

  // Returns true if all value V users in given blocks are load instructions
  // or bitcasts/GEPs that are used by the loads.
  auto allUsersAreLoads = [](Value *V,
                              const SmallPtrSetImpl<BasicBlock *> &BBs) {
    // Predicate to check if given value is an instruction from BBs.
    auto IsFromBBs = [&BBs](Value *V) {
      if (auto *I = dyn_cast<Instruction>(V))
        return BBs.contains(I->getParent());
      return false;
    };

    // Verify that all users of alloca instruction in collected blocks are
    // either loads or bitcasts/GEPs used by the loads.
    SmallVector<Value *, 8u> Worklist;
    copy_if(V->users(), std::back_inserter(Worklist), IsFromBBs);
    while (!Worklist.empty()) {
      Value *I = Worklist.pop_back_val();
      if (isa<BitCastInst>(I) || isa<GetElementPtrInst>(I)) {
        copy_if(I->users(), std::back_inserter(Worklist), IsFromBBs);
        continue;
      }
      if (!isa<LoadInst>(I))
        return false;
    }
    return true;
  };

#ifndef NDEBUG
  auto reportSkipped = [](Value *V, const Twine &Msg) {
    dbgs() << "skipping '" << V->getName() << "' - " << Msg << "\n";
  };
#endif // NDEBUG

  auto IsPrivatizationCandidate =
      [&](AllocaInst *AI, const SmallPtrSetImpl<BasicBlock *> &BBs) {
        // Do not attempt to promote arrays or structures.
        if (AI->isArrayAllocation() ||
            !AI->getAllocatedType()->isSingleValueType()) {
          LLVM_DEBUG(reportSkipped(AI, "not a single value type"));
          return false;
        }
        Optional<TypeSize> Size =
            AI->getAllocationSizeInBits(F->getParent()->getDataLayout());
        if (!Size) {
          LLVM_DEBUG(reportSkipped(AI, "unknown size"));
          return false;
        }

        // Check if item's memory is modified inside the region. If not then it
        // should be safe to privatize it.
        MemoryLocation Loc{AI, LocationSize::precise(*Size)};
        if (any_of(BBs, [&](BasicBlock *BB) {
              for (const Instruction &I : *BB) {
                // Ignore fences when checking if memory location is modified by
                // the instruction since fences do not really modify it though
                // AA assumes they do.
                if (isa<FenceInst>(&I))
                  continue;
                if (isModOrRefSet(intersectModRef(AA->getModRefInfo(&I, Loc),
                                                  ModRefInfo::Mod))) {
                  LLVM_DEBUG(reportSkipped(AI, "is modified by");
                             dbgs() << I << "\n";);
                  return true;
                }
              }
              return false;
            }))
          return false;
        return true;
      };

  auto ContainsValue = [](auto *C, Value *V) {
    return any_of(C->items(), [V](auto *I) { return I->getOrig() == V; });
  };

  // Returns true if value V is private in the work region W.
  auto IsWRNPrivate = [&ContainsValue](WRegionNode *W, Value *V) {
    if (PrivateClause *C = W->getPrivIfSupported())
      if (ContainsValue(C, V))
        return true;
    if (FirstprivateClause *C = W->getFprivIfSupported())
      if (ContainsValue(C, V))
        return true;
    return false;
  };

  // Returns true if value V is captured by any nested 'omp task' construct.
  auto IsCapturedByNestedTask = [&, W](Value *V) {
    SmallVector<WRegionNode *, 8u> Worklist{W};
    do {
      WRegionNode *W = Worklist.pop_back_val();
      for (WRegionNode *CW : W->getChildren()) {
        if (IsWRNPrivate(CW, V))
          continue;
        if (CW->getIsTask() && ContainsValue(&CW->getShared(), V)) {
          LLVM_DEBUG(reportSkipped(V, "is captured by a nested task"));
          return true;
        }
        Worklist.push_back(CW);
      }
    } while (!Worklist.empty());
    return false;
  };

  // Collects set of work region blocks where we will be checking if item's
  // memory is modified. This set does not include blocks with directives (all
  // directives have their own basic blocks now) and nested regions where item
  // is private.
  auto FindWRNBlocks = [&, W](Value *V) {
    // Build set of work region blocks where we will be checking if item's
    // memory is modified. Exclude blocks with directives (all directives have
    // their own basic blocks now).
    SmallPtrSet<BasicBlock *, 16u> BBs;
    for (BasicBlock *BB : W->blocks())
      if (!VPOAnalysisUtils::isBeginOrEndDirective(BB))
        BBs.insert(BB);

    // Then exclude nested regions where shared item will be privatized. Any
    // modifications in these regions should not inhibit privatization because
    // it will be done on a private instance.
    SmallVector<WRegionNode *, 8u> Worklist{W};
    do {
      WRegionNode *W = Worklist.pop_back_val();
      for (WRegionNode *CW : W->getChildren()) {
        if (IsWRNPrivate(CW, V)) {
          for_each(CW->blocks(), [&BBs](BasicBlock *BB) { BBs.erase(BB); });
          continue;
        }
        Worklist.push_back(CW);
      }
    } while (!Worklist.empty());

    return BBs;
  };

  // Replaces all uses of value From with To within work region W.
  auto ReplaceWRNUsesOfWith = [W](Value *From, Value *To) {
    SmallVector<Instruction *, 8> Users;
    if (WRegionUtils::findUsersInRegion(W, From, &Users))
      for (auto *User : Users)
        User->replaceUsesOfWith(From, To);
  };

  // Find "shared" candidates that can be privatized.
  SmallVector<AllocaInst*, 8> ToPrivatize;
  for (SharedItem *I : W->getShared().items()) {
    if (auto *AI = dyn_cast<AllocaInst>(I->getOrig())) {
      // Do not do privatization for a shared item if it is captured by a nested
      // task.
      //
      //  PARALLEL SHARED(X)         =>      PARALLEL FIRSTPRIVATE(X)
      //    TASK SHARED(X)                     TASK SHARED(X)
      //
      // Such transformation is illegal since it may cause thread executing task
      // to access X allocated on the stack of thread that created the task when
      // its stack frame is freed.
      if (IsCapturedByNestedTask(AI))
        continue;

      auto BBs = FindWRNBlocks(AI);

      if (!IsPrivatizationCandidate(AI, BBs) || !allUsersAreLoads(AI, BBs))
        continue;

      ToPrivatize.push_back(AI);
      continue;
    }

    // Check if shared item is a bitcasted alloca instruction that can be
    // privatized. If so move bitcast into the work region and change shared
    // item to alloca instruction.
    if (auto *BCI = dyn_cast<BitCastInst>(I->getOrig()))
      if (auto *AI = dyn_cast<AllocaInst>(BCI->getOperand(0))) {

        // TODO: If the BC's region is nested, we cannot make the alloca
        // live-into the inner region without fixing up all the outer
        // regions with map and/or explicit shared. Just disable for now.
        if (W->getParent() && W->needsOutlining())
          continue;

        if (IsCapturedByNestedTask(BCI))
          continue;

        auto BBs = FindWRNBlocks(AI);

        if (!IsPrivatizationCandidate(AI, BBs) || !allUsersAreLoads(BCI, BBs))
          continue;

        // Change shared value to alloca instruction.
        auto *EntryDir = cast<IntrinsicInst>(W->getEntryDirective());
        EntryDir->replaceUsesOfWith(BCI, AI);
        I->setOrig(AI);

        // And move bitcast into the region.
        BasicBlock *EntrySuccessor = W->getEntryBBlock()->getSingleSuccessor();
        assert(EntrySuccessor && "Entry block must have a single successor");
        auto *NewBCI = cast<BitCastInst>(BCI->clone());
        NewBCI->insertBefore(EntrySuccessor->getFirstNonPHI());

        ReplaceWRNUsesOfWith(BCI, NewBCI);

        // Add alloca to the privatization list.
        ToPrivatize.push_back(AI);
        continue;
      }

    LLVM_DEBUG(reportSkipped(I->getOrig(), "not an local pointer"));
  }

  if (ToPrivatize.empty())
    return false;

  // Create separate block for alloca and load/store instructions.
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewBB = SplitBlock(EntryBB, EntryBB->getTerminator(), DT, LI);
  Instruction *InsPt = NewBB->getTerminator();

  // Create private instances for variables collected earlier.
  for (AllocaInst* AI : ToPrivatize) {
    LLVM_DEBUG(dbgs() << "privatizing '" << AI->getName() << "'\n");

    // Allocate space for the private copy.
    auto *NewAI = cast<AllocaInst>(AI->clone());
    NewAI->setName(AI->getName() + ".fp");
    NewAI->insertBefore(InsPt);

    // Copy variable value from the original location to the private instance.
    new StoreInst(
        new LoadInst(AI->getAllocatedType(), AI, AI->getName() + ".v", InsPt),
        NewAI, InsPt);

    // And replace all uses of the original variable in the region with the
    // private one.
    ReplaceWRNUsesOfWith(AI, NewAI);
  }

  // Need to refresh BBSet because CFG has been changed.
  W->populateBBSet(/*Always=*/true);

  return true;
}

bool VPOParoptTransform::privatizeSharedItems() {
  bool NeedTID, NeedBID;
  gatherWRegionNodeList(NeedTID, NeedBID);

  bool Changed = false;
  for (auto *W : WRegionList) {
    switch (W->getWRegionKindID()) {
    case WRegionNode::WRNTeams:
    case WRegionNode::WRNParallel:
      Changed |= privatizeSharedItems(W);
      break;
    case WRegionNode::WRNParallelSections:
    case WRegionNode::WRNParallelLoop:
    case WRegionNode::WRNDistributeParLoop:
      Changed |= privatizeSharedItems(W);
      break;
    }
  }
  return Changed;
}
