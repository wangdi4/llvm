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
#include "llvm/IR/Operator.h"


namespace llvm {

// The function searches nested structures for the first matching type
// in the beginning of a structure.
// For instance if Outer is struct S {struct Q {int i}; float f;};
// And Inner is struct Q, then Inner will be returned as a result.
// The same time search for float should not succeed since "float f"
// is not a 0 offset struct member.
// nullptr is returned if no match found  
static Type* FindFirstZeroOffsetStrucType(Type* Outer, Type* Inner) {
  while(Outer) {
    if (Outer == Inner)
      return Inner;
    if (auto *S = dyn_cast<StructType>(Outer))
      Outer = S->getStructElementType(0);
    else
      return nullptr;
  }
  return nullptr;
}

//
// The function merges 2 types. Using Optional allows to implement
// 3 state logic. When an Optional has no value this denotes any
// possible type. When it has a value and the value is actual type, it
// naturally means that type. If an Optional has a value
// and the value is nullptr this denotes none type, typically
// this is result of a merge of two conflicting types.
//
static Optional<Type *> mergeTypes(const Optional<Type *> &X,
                                   const Optional<Type *> &Y) {
  if (!X.has_value()) {
    return Y;
  }
  if (!Y.has_value()) {
    return X;
  }

  if (!X.value() || !Y.value())
    return nullptr;

  // It may happen that a pointer points to 2 different types
  // for instance for types like struct S { struct Q {int i}; };
  // we need to choose which type to return after a merge, since
  // pointers to &S, &S::Q and &S::Q::i are actually the same.
  // In the current approach, the least type appeared in the search
  // will be result of the merge. Probably it needs to be
  // revised in the future.
  if (X.value() != Y.value()) {
    if (auto *RT = FindFirstZeroOffsetStrucType(X.value(), Y.value()))
      return RT;
    if (auto *RT = FindFirstZeroOffsetStrucType(Y.value(), X.value()))
      return RT;
    return nullptr;
  }
  return X;
}

//
// Return the pointer element type of 'V' that can be inferred by checking the
// types of its uses through various instruction types. 
// The approaches used to infer a type (in the order of priority).
// 1. Direct type inference from an instruction.
// 2. Infer a type from pointer uses.
// 3. Infer a type from defs.
// Result of the function is an Optional with a meaning as it described
// in MergeTypes function
//
static Optional<Type *>
inferPtrElementTypeX(Value *V, DenseMap<const Value *, Optional<Type *>> &M) {
  constexpr auto AnyTy = Optional<Type*>();
  auto IT = M.find(V);
  if (IT != M.end()) {
    return IT->second;
  }
  
  M[V] = AnyTy;

  if (auto AI = dyn_cast<AllocaInst>(V))
    return M[V] = AI->getAllocatedType();

  if (auto SI = dyn_cast<SubscriptInst>(V))
    return M[V] = SI->getElementType();

  auto Ty = AnyTy;

  for (auto &Use : V->uses()) {
    auto *U = Use.getUser();
    auto NTy = AnyTy;
    if (auto GEP = dyn_cast<GEPOperator>(U))
      NTy = GEP->getSourceElementType();
    else if (auto LI = dyn_cast<LoadInst>(U))
      NTy = LI->getType();
    else if (auto SI = dyn_cast<StoreInst>(U)) {
      if (SI->getPointerOperand() != V)
        continue;
      NTy = SI->getValueOperand()->getType();
    } else if (auto SuI = dyn_cast<SubscriptInst>(U)) {
      if (SuI->getPointerOperand() != V)
        continue;
      NTy = SuI->getElementType();
    } else if (auto CB = dyn_cast<CallBase>(U)) {
      auto *CalledF = CB->getCalledFunction();
      if (!CalledF || CalledF->isVarArg())
        continue;
      if (!CB->isArgOperand(&Use))
        continue;
      unsigned ANo = CB->getArgOperandNo(&Use);
      auto *A = CalledF->getArg(ANo);
      NTy = inferPtrElementTypeX(A, M);
    } else
      continue;

    Ty = mergeTypes(Ty, NTy);
    if (Ty.has_value() && Ty.value() == nullptr)
      break;
  }

  if (Ty == AnyTy) {
    if (auto Arg = dyn_cast<Argument>(V)) {
      auto *F = Arg->getParent();
      for (auto *U: F->users()) {
        if (auto *CB = dyn_cast<CallBase>(U)) {
          if (CB->getCalledFunction() == F) {
            auto *ActArg = CB->getArgOperand(Arg->getArgNo());
            auto NTy = inferPtrElementTypeX(ActArg, M);
            Ty = mergeTypes(Ty, NTy);
          }
        }
      }
      // We failed to infer a type from the argument defs.
      // This naturally means that the defs have different types.
      // In this case, no better type can be deduced than "any" type.
      if (Ty.has_value() && Ty.value() == nullptr)
        Ty = AnyTy;
    } else if (auto BC = dyn_cast<BitCastOperator>(V))
    // Failed to infer a BitCast type by uses. In the case if ptr->ptr cast
    // which is actually dead code, we can try to infer type from
    // the BitCast argument.
      if (BC->getSrcTy()->isPointerTy() && BC->getDestTy()->isPointerTy())
        Ty = inferPtrElementTypeX(BC->getOperand(0), M); 
  }

  M[V] = Ty;
  return Ty;
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
  if (Ty->getContext().supportsTypedPointers())
    return Ty->getNonOpaquePointerElementType();
  auto VMap = DenseMap<const Value *, Optional<Type *>>();
  auto RT = inferPtrElementTypeX(&V, VMap);
  return RT.has_value() ? RT.value() : nullptr;
}

} // namespace llvm
