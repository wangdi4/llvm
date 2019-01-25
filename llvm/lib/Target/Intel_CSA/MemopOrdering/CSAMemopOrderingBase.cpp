//===-- CSAMemopOrderingBase.cpp - Common memop ordering base ---*- C++ -*-===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file implements a base class for CSA memop ordering passes.
///
///===---------------------------------------------------------------------===//

#include "CSAMemopOrderingBase.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"

#include <iterator>

using namespace llvm;

#define DEBUG_TYPE "csa-memop-ordering"

// Memory ordering statistics.
STATISTIC(MemopCount, "Number of memory operations ordered");
STATISTIC(MergeCount, "Number of merges inserted");
STATISTIC(PHICount, "Number of phi nodes inserted");

void CSAMemopOrderingBase::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<AAResultsWrapperPass>();
  AU.addPreserved<StackProtector>();
}

bool CSAMemopOrderingBase::runOnFunction(Function &F) {
  AA                = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  Module *const Mod = F.getParent();
  MemordType        = Type::getInt1Ty(F.getContext());
  NoneVal           = ConstantInt::getFalse(F.getContext());
  InordIntr         = Intrinsic::getDeclaration(Mod, Intrinsic::csa_inord);
  OutordIntr        = Intrinsic::getDeclaration(Mod, Intrinsic::csa_outord);
  All0Intr          = Intrinsic::getDeclaration(Mod, Intrinsic::csa_all0);

  MemEntry =
    CallInst::Create(Intrinsic::getDeclaration(Mod, Intrinsic::csa_mementry),
                     "memop.0o0", F.getEntryBlock().getFirstNonPHIOrDbg());

  order(F);

  deleteParallelIntrinsics(F);

  return true;
}

bool CSAMemopOrderingBase::needsOrderingEdges(Instruction &I) const {

  switch (I.getOpcode()) {

  // Calls are always ordered, except for calls to intrinsics. Currently
  // prefetch is the only intrinsic that needs ordering.
  case Instruction::Call: {
    const auto CI = cast<CallInst>(&I);
    if (const auto II = dyn_cast<IntrinsicInst>(CI))
      return II->getIntrinsicID() == Intrinsic::prefetch;
    return not CI->isInlineAsm();
  }

  // Loads of constant memory are _not_ ordered, since they don't really need
  // ordering and don't have attached chain operands in instruction selection.
  // Other loads are always ordered.
  case Instruction::Load: {
    const auto LI = cast<LoadInst>(&I);
    const bool IsConstantLoad =
      LI->isSimple() and AA->pointsToConstantMemory(LI->getPointerOperand());
    return not IsConstantLoad;
  }

  // These instructions are always ordered.
  case Instruction::Store:
  case Instruction::AtomicCmpXchg:
  case Instruction::Fence:
  case Instruction::AtomicRMW:
  case Instruction::Ret:
    return true;

  // Nothing else should be ordered.
  default:
    return false;
  }
}

PHINode *CSAMemopOrderingBase::createPHI(BasicBlock *BB, const Twine &Name) {
  ++PHICount;
  return PHINode::Create(MemordType, pred_size(BB), Name, BB->getFirstNonPHI());
}

Value *CSAMemopOrderingBase::createOrderingEdges(Instruction *I, Value *Inord,
                                                 const Twine &Name) {
  if (Inord)
    CallInst::Create(InordIntr, Inord, "", I);
  if (isa<ReturnInst>(I))
    return nullptr;
  ++MemopCount;
  return CallInst::Create(OutordIntr, Name, I->getNextNode());
}

Value *CSAMemopOrderingBase::createAll0(ArrayRef<Value *> Inputs,
                                        Instruction *Where, const Twine &Name) {
  using std::begin;
  using std::end;
  using std::remove;
  SmallVector<Value *, 4> NeededInputs(begin(Inputs), end(Inputs));
  NeededInputs.erase(remove(begin(NeededInputs), end(NeededInputs), NoneVal),
                     end(NeededInputs));
  switch (NeededInputs.size()) {
  case 0:
    return NoneVal;
  case 1:
    return NeededInputs.front();
  default:
    ++MergeCount;
    return CallInst::Create(All0Intr, NeededInputs, Name, Where);
  }
}

static BasicBlock::iterator recursivelyDeleteWithAllUsers(Instruction *I) {
  while (not I->user_empty())
    recursivelyDeleteWithAllUsers(I->user_back());
  return I->eraseFromParent();
}

void CSAMemopOrderingBase::deleteParallelIntrinsics(Function &F) {
  using std::begin;
  using std::end;
  for (BasicBlock &BB : F) {
    for (auto IIt = begin(BB); IIt != end(BB);) {
      IntrinsicInst *const II = dyn_cast<IntrinsicInst>(&*IIt);
      if (II and II->getIntrinsicID() == Intrinsic::csa_parallel_region_entry) {
        IIt = recursivelyDeleteWithAllUsers(II);
      } else {
        ++IIt;
      }
    }
  }
}
