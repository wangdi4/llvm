//===-------- DTransOpUtils.cpp - Utilities for DTrans opaque pointers ----===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
// General utilities for DTrans opaque pointer classes.
///
// ===--------------------------------------------------------------------=== //

#include "Intel_DTrans/Analysis/DTransTypes.h"

namespace llvm {
namespace dtransOP {

bool areOpaquePtrsEnabled(LLVMContext &Ctx) {
  llvm::Type *I8Ptr = llvm::Type::getInt8Ty(Ctx)->getPointerTo();
  llvm::Type *I16Ptr = llvm::Type::getInt16Ty(Ctx)->getPointerTo();
  return I8Ptr == I16Ptr;
}

bool hasPointerType(DTransType *Ty) {
  if (Ty->isPointerTy())
    return true;

  if (auto *ArTy = dyn_cast<DTransArrayType>(Ty))
    return hasPointerType(ArTy->getArrayElementType());
  if (auto *VecTy = dyn_cast<DTransVectorType>(Ty))
    return hasPointerType(VecTy->getVectorElementType());

  if (auto *STy = dyn_cast<DTransStructType>(Ty)) {
    // Check inside of literal structures because those cannot be referenced by
    // name. However, do not look inside non-literal structures because those
    // will be referenced by their name.
    if (STy->isLiteralStruct()) {
      for (auto &FieldMember : STy->elements()) {
        DTransType *FieldTy = FieldMember.getType();
        assert(FieldTy && "Metadata reader had ambiguous types");
        if (hasPointerType(FieldTy))
          return true;
      }
    }
  }

  if (auto *FuncTy = dyn_cast<DTransFunctionType>(Ty)) {
    DTransType *RetTy = FuncTy->getReturnType();
    assert(RetTy && "Metadata reader had ambiguous types");
    if (hasPointerType(RetTy))
      return true;

    unsigned NumParams = FuncTy->getNumArgs();
    for (unsigned Idx = 0; Idx < NumParams; ++Idx) {
      DTransType *ArgTy = FuncTy->getArgType(Idx);
      assert(ArgTy && "Metadata reader had ambiguous types");
      if (hasPointerType(ArgTy))
        return true;
    }
  }

  return false;
}

DTransType *unwrapDTransType(DTransType *Ty) {
  DTransType *BaseTy = Ty;
  while (BaseTy->isPointerTy() || BaseTy->isArrayTy() || BaseTy->isVectorTy()) {
    if (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();
    else if (BaseTy->isArrayTy())
      BaseTy = cast<DTransArrayType>(BaseTy)->getElementType();
    else if (BaseTy->isVectorTy())
      BaseTy = cast<DTransVectorType>(BaseTy)->getElementType();
  }

  return BaseTy;
}

} // end namespace dtransOP
} // end namespace llvm