//===------ Intel_GeneralUtils.cpp - General set of utilities for VPO -----===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file includes a set of utilities that are generally useful for many
/// purposes.
///
// ===--------------------------------------------------------------------=== //

#include <queue>
#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;

template Constant *
IntelGeneralUtils::getConstantValue<int>(Type *Ty, LLVMContext &Context,
                                         int Val);
template Constant *
IntelGeneralUtils::getConstantValue<float>(Type *Ty, LLVMContext &Context,
                                           float Val);
template Constant *
IntelGeneralUtils::getConstantValue<double>(Type *Ty, LLVMContext &Context,
                                            double Val);
template <typename T>
Constant *IntelGeneralUtils::getConstantValue(Type *Ty, LLVMContext &Context,
                                              T Val) {
  Constant *ConstVal = nullptr;

  if (Ty->isIntegerTy(8))
    ConstVal = ConstantInt::get(Type::getInt8Ty(Context), Val);
  else if (Ty->isIntegerTy(16))
    ConstVal = ConstantInt::get(Type::getInt16Ty(Context), Val);
  else if (Ty->isIntegerTy(32))
    ConstVal = ConstantInt::get(Type::getInt32Ty(Context), Val);
  else if (Ty->isIntegerTy(64))
    ConstVal = ConstantInt::get(Type::getInt64Ty(Context), Val);
  else if (Ty->isFloatTy())
    ConstVal = ConstantFP::get(Type::getFloatTy(Context), Val);
  else if (Ty->isDoubleTy())
    ConstVal = ConstantFP::get(Type::getDoubleTy(Context), Val);

  assert(ConstVal && "Could not generate constant for type");

  return ConstVal;
}

Loop *IntelGeneralUtils::getLoopFromLoopInfo(LoopInfo *LI,
                                             BasicBlock *WRNEntryBB) {

  // The loop header BB is the successor BB of the WRN's entry BB
  BasicBlock *LoopHeaderBB = *(succ_begin(WRNEntryBB));
  Loop *Lp = LI->getLoopFor(LoopHeaderBB);
#if 0
  dbgs() << "Checking BB for Loop:\n" << *LoopHeaderBB << "\n";
  if (Lp)
    dbgs() << "Found Loop: " << *Lp << "\n";
#endif
  return Lp;
}

/// \brief This function ensures that EntryBB is the first item in BBSet and
/// ExitBB is the last item in BBSet.
void IntelGeneralUtils::collectBBSet(BasicBlock *EntryBB, BasicBlock *ExitBB,
                                     SmallVectorImpl<BasicBlock *> &BBSet) {

  assert(EntryBB && "no EntryBB");
  assert(ExitBB && "no ExitBB");

  std::queue<BasicBlock *> BBlockQueue;
  BBlockQueue.push(EntryBB);

  while (!BBlockQueue.empty()) {
    BasicBlock *Front = BBlockQueue.front();
    BBlockQueue.pop();

    if (Front != ExitBB) {
      // If 'Front' is not in the BBSet, insert it to the end of BBSet.
      if (std::find(BBSet.begin(), BBSet.end(), Front) == BBSet.end())
        BBSet.push_back(Front);

      for (succ_iterator I = succ_begin(Front), E = succ_end(Front); I != E;
           ++I)
        // Push the successor to the queue only if it is not in the BBSet.
        if (std::find(BBSet.begin(), BBSet.end(), *I) == BBSet.end())
          BBlockQueue.push(*I);
    }
  }

  BBSet.push_back(ExitBB);

  assert((BBSet.front() == EntryBB) &&
         "The first element of BBSet is not EntryBB");
  assert((BBSet.back() == ExitBB) && "The last element of BBSet is not ExitBB");
}
