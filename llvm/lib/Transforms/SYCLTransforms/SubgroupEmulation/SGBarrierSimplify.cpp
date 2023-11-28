//== SGBarrierSimplify.cpp - Simplify subgroup barrier ----------- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGBarrierSimplify.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGSizeAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-sg-emu-barrier-simplify"

PreservedAnalyses SGBarrierSimplifyPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<SGSizeAnalysisPass>();
  return PA;
}

bool SGBarrierSimplifyPass::runImpl(Module &M) {
  Helper.initialize(M);

  FuncSet FunctionsToHandle = Helper.getAllFunctionsNeedEmulation();

  if (FunctionsToHandle.empty())
    return false;

  // Initialize barrier utils.
  Utils.init(&M);

  bool Changed = false;

  for (auto *F : FunctionsToHandle) {
    Changed |= simplifyCallRegion(F);
    Changed |= removeRedundantBarriers(F);
    Changed |= simplifyDummyRegion(F);
  }

  for (auto *F : FunctionsToHandle)
    Changed |= splitBarrierBB(F);

  return Changed;
}

// For the barrier - dummybarrier pair, there shouldn't be any SG barrier or
// dummy_sg_barrier between them. So we can search such pair from every call of
// WG sync function and remove all encountered sub_group_barrier(s) /
// dummy_sg_barrier(s).
bool SGBarrierSimplifyPass::simplifyCallRegion(Function *F) {
  if (!Utils.getAllFunctionsWithSynchronization().count(F))
    return false;

  InstVec Barriers;
  InstVec DummyBarriers;
  for (User *U : F->users()) {
    CallInst *CI = dyn_cast<CallInst>(U);
    if (!CI)
      continue;

    LLVM_DEBUG(dbgs() << "Simplify call region for " << *CI << "\n");

    Instruction *Prev = CI->getPrevNode();
    while (!Utils.isBarrierCall(Prev)) {
      if (Helper.isDummyBarrier(Prev))
        DummyBarriers.push_back(Prev);
      if (Helper.isBarrier(Prev))
        Barriers.push_back(Prev);
      Prev = Prev->getPrevNode();
    }

    Instruction *Next = CI->getNextNode();
    while (!Utils.isDummyBarrierCall(Next)) {
      if (Helper.isDummyBarrier(Next))
        DummyBarriers.push_back(Next);
      if (Helper.isBarrier(Next))
        Barriers.push_back(Next);
      Next = Next->getNextNode();
    }
  }

  Helper.removeBarriers(Barriers);
  Helper.removeDummyBarriers(DummyBarriers);

  return !Barriers.empty() || !DummyBarriers.empty();
}

bool SGBarrierSimplifyPass::removeRedundantBarriers(Function *F) {
  LLVM_DEBUG(dbgs() << "Remove redundant barriers for function " << F->getName()
                    << "\n");

  bool Changed = false;

  // Collect sub_group_barrier / dummy_sg_barrier calls.
  auto &Barriers = Helper.getBarriersForFunction(F);
  auto &DummyBarriers = Helper.getDummyBarriersForFunction(F);

  InstVec BarriersToRemove;
  InstVec DummyBarriersToRemove;

  while (true) {
    for (auto *Barrier : Barriers) {
      auto *I = cast<Instruction>(Barrier->getNextNode());
      if (!Barriers.count(I))
        continue;
      // Remove the later barrier because we may encounter following case:
      //   dummy_sg_barrier
      //   sg_barrier
      //   sg_barrier
      // If we remove the previous one, it will be pushed twice which will cause
      // segfault.
      BarriersToRemove.push_back(I);
    }

    for (auto *DummyBarrier : DummyBarriers) {
      auto *I = cast<Instruction>(DummyBarrier->getNextNode());
      if (Barriers.count(I))
        BarriersToRemove.push_back(I);
      else if (DummyBarriers.count(I) || Utils.isDummyBarrierCall(I) ||
               Utils.isBarrierCall(I))
        DummyBarriersToRemove.push_back(DummyBarrier);
    }

    if (!BarriersToRemove.empty())
      Helper.removeBarriers(BarriersToRemove);

    if (!DummyBarriersToRemove.empty())
      Helper.removeDummyBarriers(DummyBarriersToRemove);

    if (BarriersToRemove.empty() && DummyBarriersToRemove.empty())
      break;

    BarriersToRemove.clear();
    DummyBarriersToRemove.clear();

    Changed = true;
  }

  // Don't remove the ending dummy_sg_barrier here, since the return value may
  // be widened later, and then we may need to restore the dummy/barrier pair to
  // fill the widened return value. Example: return sub_group_all(x).
  return Changed;
}

bool SGBarrierSimplifyPass::simplifyDummyRegion(Function *F) {
  InstVec Barriers;
  InstVec DummyBarriers;
  auto DummyRegion = Utils.findDummyRegion(*F);
  for (auto &I : DummyRegion) {
    if (Helper.isBarrier(&I))
      Barriers.push_back(&I);
    if (Helper.isDummyBarrier(&I))
      DummyBarriers.push_back(&I);
  }

  if (!Barriers.empty())
    Helper.removeBarriers(Barriers);
  if (!DummyBarriers.empty())
    Helper.removeDummyBarriers(DummyBarriers);

  return !Barriers.empty() || !DummyBarriers.empty();
}

bool SGBarrierSimplifyPass::splitBarrierBB(Function *F) {
  LLVM_DEBUG(dbgs() << "Splitting Barrier BB for function " << F->getName()
                    << "\n");

  // Collect sub_group_barrier / dummy_sg_barrier calls again since some may be
  // removed.
  auto &Barriers = Helper.getBarriersForFunction(F);
  auto &DummyBarriers = Helper.getDummyBarriersForFunction(F);

  bool Changed = false;

  for (auto *Barrier : Barriers) {
    auto *BB = Barrier->getParent();
    BasicBlock::iterator It{Barrier};
    if (BB->begin() == It)
      continue;
    BB->splitBasicBlock(It, "sg.barrier.bb.");
    Changed = true;
  }

  for (auto *DummyBarrier : DummyBarriers) {
    auto *BB = DummyBarrier->getParent();
    BasicBlock::iterator It{DummyBarrier};
    if (BB->begin() == It)
      continue;
    BB->splitBasicBlock(It, "sg.dummy.bb.");
    Changed = true;
  }

  return Changed;
}
