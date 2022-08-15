//===------------------ Intel_OPAnalysisUtils.cpp -------------------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
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

OpaquePointerTypeMapper::OpaquePointerTypeMapper(Module &M) {
  auto updateType = [](TypeAggregator& A, const Type* Ty) {
    if (!A.has_value()) {
      A = Ty;
      return;
    }
    if (!A.value())
      return;
    if (A.value() != Ty) {
      A = nullptr;
      return;
    }
  };

  for (auto &F : M.functions()) {
    for (auto &I : instructions(F)) {
      auto *Ty = inferPtrElementType(I);
      if (!Ty)
        continue;
      updateType(PtrETypeMap[&I], Ty);
      for (auto &Use : I.uses()) {
        auto *User = Use.getUser();
        // Propagate types to functions' dummy args.
        if (auto *CB = dyn_cast<CallBase>(User)) {
          auto *CalledF = CB->getCalledFunction();
          if (!CalledF || CalledF->isVarArg())
            continue;
          if(!CB->isArgOperand(&Use))
            continue;
          unsigned ANo = CB->getArgOperandNo(&Use);
          auto *A = CalledF->getArg(ANo);
          updateType(PtrETypeMap[A], Ty);
        }
      }
    }
  }
}

const Type *OpaquePointerTypeMapper::getPointerElementType(const Value *V) const {
  auto IT = PtrETypeMap.find(V);
  if (IT == PtrETypeMap.end())
    return nullptr;
  if (!IT->second.hasValue())
    return nullptr;
  return IT->second.value();
}

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
  if (auto AI = dyn_cast<AllocaInst>(V))
    return AI->getAllocatedType();
  for (User *U : V->users()) {
    Type *NTy = nullptr;
    if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      NTy = ResultType(STy, GEPI->getSourceElementType());
    } else if (auto LI = dyn_cast<LoadInst>(U)) {
      NTy = ResultType(STy, LI->getType());
    } else if (auto SI = dyn_cast<StoreInst>(U)) {
      if (SI->getPointerOperand() != V)
        continue;
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
Type *inferPtrElementType(Value &V) {
  llvm::Type *Ty = V.getType();
  if (!Ty->isPointerTy())
    return nullptr;
  return inferPtrElementTypeX(&V);
}

} // namespace llvm
