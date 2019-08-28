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
  PrefetchIntr      = nullptr;

  MemEntry =
    CallInst::Create(Intrinsic::getDeclaration(Mod, Intrinsic::csa_mementry),
                     "memop.0o0", F.getEntryBlock().getFirstNonPHIOrDbg());

  order(F);

  deleteParallelIntrinsics(F);

  expandDataGatedPrefetches(F);

  return true;
}

static bool isOrderableIntrinsic(Intrinsic::ID ID) {
  switch (ID) {
  case Intrinsic::prefetch:
  case Intrinsic::csa_pipeline_depth_token_take:
  case Intrinsic::csa_pipeline_depth_token_return:
    return true;
  default:
    return false;
  }
}

bool CSAMemopOrderingBase::needsOrderingEdges(Instruction &I) const {

  switch (I.getOpcode()) {

  // Calls are always ordered, except for calls to intrinsics. Currently
  // prefetch is the only intrinsic that needs ordering.
  case Instruction::Call: {
    const auto CI = cast<CallInst>(&I);
    if (const auto II = dyn_cast<IntrinsicInst>(CI))
      return isOrderableIntrinsic(II->getIntrinsicID());
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

Value *CSAMemopOrderingBase::toMemOrdValue(Value *V, Instruction *Where) {

  // If the value is a constant, just return <none>.
  if (isa<Constant>(V))
    return NoneVal;

  // Otherwise, get its type. If it's already the right type, no conversion is
  // needed.
  const Type *const Ty = V->getType();
  if (Ty == MemordType)
    return V;

  // Otherwise, check if it's already an integer type. If it is, just truncate
  // it.
  if (Ty->isIntegerTy()) {
    return CastInst::Create(Instruction::Trunc, V, MemordType, "", Where);
  }

  // Otherwise, check if it's a pointer type. If it is, convert it with
  // ptrtoint.
  if (Ty->isPointerTy()) {
    return CastInst::Create(Instruction::PtrToInt, V, MemordType, "", Where);
  }

  // Otherwise, check if it's an aggregate type. If it is, convert its elements
  // and all0 those together.
  if (Ty->isAggregateType()) {
    SmallVector<Value *, 4> All0Inputs;

    if (const auto ArrayTy = dyn_cast<ArrayType>(Ty)) {
      for (int Idx = 0, IdxE = ArrayTy->getNumElements(); Idx != IdxE; ++Idx) {
        Value *const EV = ExtractValueInst::Create(V, Idx, "", Where);
        All0Inputs.push_back(toMemOrdValue(EV, Where));
      }
    } else if (const auto StructTy = dyn_cast<StructType>(Ty)) {
      for (int Idx = 0, IdxE = StructTy->getNumElements(); Idx != IdxE; ++Idx) {
        Value *const EV = ExtractValueInst::Create(V, Idx, "", Where);
        All0Inputs.push_back(toMemOrdValue(EV, Where));
      }
    } else
      llvm_unreachable("Are you sure this is an aggregate type?");

    return createAll0(All0Inputs, Where);
  }

  // Otherwise, bitcast it to an appropriately-sized integer and keep going.
  Type *const IntTy =
    Type::getIntNTy(Ty->getContext(), Ty->getPrimitiveSizeInBits());
  Value *const Bitcasted =
    CastInst::Create(Instruction::BitCast, V, IntTy, "", Where);
  return toMemOrdValue(Bitcasted, Where);
}

void CSAMemopOrderingBase::expandDataGatedPrefetches(Function &F) {
  using std::begin;
  using std::end;
  using std::next;
  for (BasicBlock &BB : F) {
    for (auto II = begin(BB); II != end(BB); ++II) {
      const auto DGPrefetch = dyn_cast<IntrinsicInst>(&*II);
      if (not DGPrefetch)
        continue;
      if (DGPrefetch->getIntrinsicID() != Intrinsic::csa_gated_prefetch)
        continue;

      // Convert the <gate> argument to an ordering signal.
      Value *const Gate =
          toMemOrdValue(DGPrefetch->getArgOperand(0), DGPrefetch);

      // Pull out the prefetch declaration if it hasn't been already for this
      // function.
      if (not PrefetchIntr) {
        PrefetchIntr =
            Intrinsic::getDeclaration(F.getParent(), Intrinsic::prefetch,
                                      DGPrefetch->getArgOperand(1)->getType());
      }

      // Construct the prefetch arguments. These are the same as the data-gated
      // ones, but with the first argument dropped and a 1 added to the end to
      // indicate a data prefetch.
      SmallVector<Value *, 4> PrefetchArgs(next(DGPrefetch->arg_begin()),
                                           DGPrefetch->arg_end());
      PrefetchArgs.push_back(
        ConstantInt::get(PrefetchIntr->getFunctionType()->getParamType(3), 1));

      // Create the prefetch call.
      Instruction *const Prefetch =
        CallInst::Create(PrefetchIntr, PrefetchArgs, "", DGPrefetch);
      Prefetch->setDebugLoc(DGPrefetch->getDebugLoc());

      // Add the ordering edge. The iterator is updated to point at the outord
      // call so it can be advanced to the next instruction in the next
      // iteration.
      Value *const OutOrd = createOrderingEdges(Prefetch, Gate);
      II                  = cast<Instruction>(OutOrd)->getIterator();

      // Now the original call is no longer needed; delete it.
      DGPrefetch->eraseFromParent();
    }
  }
}
