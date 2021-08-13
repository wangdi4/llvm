//===------------------ Intel_OPAnalysisUtils.cpp -------------------------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Analysis routines helpful in working with IR that uses opaque pointers.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OPAnalysisUtils.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"

namespace llvm {

//
// Return the pointer element type of 'V' that can be inferred by checking the
// types of its uses through various instruction types. Return 'nullptr' if no type
// can be inferred or the types inferred are inconsistent.
//
static Type *inferPtrElementTypeX(Value *V) {

  auto ResultType = [](Type *OldTy, Type *NewTy) -> Type * {
    return (!OldTy || NewTy == OldTy) ? NewTy : nullptr;
  };

  Type *STy = nullptr;
  for (User *U : V->users()) {
    Type *NTy = nullptr;
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      NTy = ResultType(STy, GEPI->getSourceElementType());
    } else if (auto LI = dyn_cast<LoadInst>(U)) {
      NTy = ResultType(STy, LI->getType());
    } else if (auto SI = dyn_cast<StoreInst>(U)) {
      if (SI->getPointerOperand() == V)
        NTy = ResultType(STy, SI->getValueOperand()->getType());
    } else if (auto SuI = dyn_cast<SubscriptInst>(U)) {
      if (SuI->getPointerOperand() == V)
        if (Type *LSTy = inferPtrElementTypeX(SuI))
          NTy = ResultType(STy, LSTy);
    }
    if (STy && !NTy)
      return nullptr;
    STy = NTy;
  }
  return STy;
}

//
// If 'Arg' is a pointer type, return its pointer element type if it can be
// inferred by checking the types of its uses through various instruction
// types. Return 'nullptr' if no type can be inferred or the types inferred
// are inconsistent.
//
// NOTE: The use of this replaces the use of
//   Arg.getType()->getPointerElementType()
// which will be removed when the community moves to opaque pointers.
//
Type *inferPtrElementType(Argument &Arg) {
  llvm::Type *Ty = Arg.getType();
  if (!Ty->isPointerTy())
    return nullptr;
  return inferPtrElementTypeX(&Arg);
}

} // namespace llvm
