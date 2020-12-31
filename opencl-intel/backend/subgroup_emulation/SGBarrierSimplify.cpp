//=----------------------- SGBarrierSimplify.cpp -*- C++ -*------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "SGBarrierSimplify.h"

#include "InitializePasses.h"
#include "OCLPassSupport.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "sg-barrier-simplify"

namespace intel {
OCL_INITIALIZE_PASS_BEGIN(SGBarrierSimplify, DEBUG_TYPE,
                          "Remove duplicate barriers and split barrier BB",
                          false, false)
OCL_INITIALIZE_PASS_DEPENDENCY_INTEL(SGSizeAnalysis)
OCL_INITIALIZE_PASS_END(SGBarrierSimplify, DEBUG_TYPE,
                        "Remove duplicate barriers and split barrier BB", false,
                        false)

char SGBarrierSimplify::ID = 0;

bool SGBarrierSimplify::removeRedundantBarriers(Function *F) {
  LLVM_DEBUG(dbgs() << "Removing redundant barriers for function "
                    << F->getName() << "\n");

  bool Changed = true;
  bool ModuleChanged = false;

  InstVec DummyBarriersToBeRemoved;
  InstVec BarriersToBeRemoved;

  // Collect sub_group_barrier / dummy_sg_barrier calls.
  auto &Barriers = Helper.getBarriersForFunction(F);
  auto &DummyBarriers = Helper.getDummyBarriersForFunction(F);

  while (Changed) {
    for (auto *Barrier : Barriers) {
      auto *I = cast<Instruction>(Barrier->getNextNode());
      if (!Barriers.count(I))
        continue;
      // Remove the later barrier because we may encounter this case.
      // dummy_sg_barrier
      // sg_barrier
      // sg_barrier
      // If we remove the former one, it will be pushed twice which will
      // cause segfault.
      BarriersToBeRemoved.push_back(I);
    }

    for (auto *DummyBarrier : DummyBarriers) {
      auto *I = cast<Instruction>(DummyBarrier->getNextNode());
      if (Barriers.count(I))
        BarriersToBeRemoved.push_back(I);
      else if (DummyBarriers.count(I) || Utils.isDummyBarrierCall(I) ||
               Utils.isBarrierCall(I))
        DummyBarriersToBeRemoved.push_back(DummyBarrier);
    }

    Changed = !BarriersToBeRemoved.empty() || !DummyBarriersToBeRemoved.empty();

    if (Changed) {
      Helper.removeBarriers(BarriersToBeRemoved);
      Helper.removeDummyBarriers(DummyBarriersToBeRemoved);
      BarriersToBeRemoved.clear();
      DummyBarriersToBeRemoved.clear();
      ModuleChanged = true;
    }
  }

  // Don't remove the ending dummy_sg_barrier here, since the return value
  // may be widened later, and then we may need to restore the dummy/barrier
  // pair to fill the widened return value. Example: return sub_group_all(x).
  return ModuleChanged;
}

// For the barrier - dummybarrier pair, there shouldn't be any SG barrier or
// dummy_sg_barrier between them. So we can search such pair from every call of
// WG sync function and remove all encountered sub_group_barrier(s) /
// dummy_sg_barrier(s).
bool SGBarrierSimplify::simplifyCallRegion(Function *F) {
  if (!Utils.getAllFunctionsWithSynchronization().count(F))
    return false;
  InstVec Barriers, DummyBarriers;
  for (User *U : F->users()) {
    if (CallInst *CI = dyn_cast<CallInst>(U)) {
      LLVM_DEBUG(dbgs() << "Simplify call region for" << *CI << "\n");
      Instruction *Prev = CI->getPrevNode();
      Instruction *Next = CI->getNextNode();
      while (!Utils.isBarrierCall(Prev)) {
        if (Helper.isDummyBarrier(Prev))
          DummyBarriers.push_back(Prev);
        if (Helper.isBarrier(Prev))
          Barriers.push_back(Prev);
        Prev = Prev->getPrevNode();
      }
      while (!Utils.isDummyBarrierCall(Next)) {
        if (Helper.isDummyBarrier(Next))
          DummyBarriers.push_back(Next);
        if (Helper.isBarrier(Next))
          Barriers.push_back(Next);
        Next = Next->getNextNode();
      }
    }
  }
  Helper.removeDummyBarriers(DummyBarriers);
  Helper.removeBarriers(Barriers);
  return !Barriers.empty() || !DummyBarriers.empty();
}

bool SGBarrierSimplify::simplifyDummyRegion(Function *F) {
  InstVec Barriers, DummyBarriers;
  BarrierUtils Utils;
  Utils.init(F->getParent());
  auto DummyRegion = Utils.findDummyRegion(*F);
  for (auto &Inst : DummyRegion) {
    if (Helper.isDummyBarrier(&Inst))
      DummyBarriers.push_back(&Inst);
    if (Helper.isBarrier(&Inst))
      Barriers.push_back(&Inst);
  }
  Helper.removeDummyBarriers(DummyBarriers);
  Helper.removeBarriers(Barriers);
  return !Barriers.empty() || !DummyBarriers.empty();
}

bool SGBarrierSimplify::splitBarrierBB(Function *F) {
  LLVM_DEBUG(dbgs() << "Spliting Barrier BB for function " << F->getName()
                    << "\n");
  // Collect sub_group_barrier / dummy_sg_barrier calls again
  // since some have been removed.
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

bool SGBarrierSimplify::runOnModule(Module &M) {
  Helper.initialize(M);
  FuncSet FunctionsToHandle = Helper.getAllFunctionsNeedEmulation();

  if (FunctionsToHandle.empty())
    return false;

  // Initialize barrier utils.
  Utils.init(&M);

  bool Changed = false;

  SizeAnalysis = &getAnalysis<SGSizeAnalysis>();

  for (auto *F : FunctionsToHandle) {
    Changed |= simplifyCallRegion(F);
    Changed |= removeRedundantBarriers(F);
    Changed |= simplifyDummyRegion(F);
  }
  for (auto *F : FunctionsToHandle)
    Changed |= splitBarrierBB(F);
  return Changed;
}

} // namespace intel

extern "C" {
llvm::Pass *createSGBarrierSimplifyPass() {
  return new intel::SGBarrierSimplify();
}
}
