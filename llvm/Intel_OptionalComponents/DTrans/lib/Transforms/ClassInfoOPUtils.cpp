//==ClassInfoOPUtils.cpp - Utilities for SOAToAOS, MemManage and CodeAlign  ==//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file has utility routines related to classes.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ClassInfoOPUtils.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"

namespace llvm {

namespace dtransOP {

// Get class type of the given function if there is one.
DTransStructType *getClassType(const Function *F,
                               TypeMetadataReader &MDReader) {
  if (F->arg_size() < 1)
    return nullptr;
  // Get class type from "this" pointer that is passed as 1st
  // argument.
  auto *DFnTy =
      dyn_cast_or_null<DTransFunctionType>(MDReader.getDTransTypeFromMD(F));
  if (!DFnTy)
    return nullptr;

  if (auto *PTy = dyn_cast<DTransPointerType>(DFnTy->getArgType(0)))
    if (auto *STy = dyn_cast<DTransStructType>(PTy->getPointerElementType()))
      return STy;
  return nullptr;
}

// Returns true if Ty is pointer to pointer to a function.
bool isPtrToVFTable(DTransType *Ty) {
  DTransType *ETy = nullptr;
  if (auto *PPETy = dyn_cast_or_null<DTransPointerType>(Ty))
    if (auto *PETy =
            dyn_cast<DTransPointerType>(PPETy->getPointerElementType()))
      ETy = PETy->getPointerElementType();
  if (!ETy || !ETy->isFunctionTy())
    return false;
  return true;
}

// Returns field type of DTy struct if it has only one field.
DTransType *getSOASimpleBaseType(DTransType *DTy) {
  assert(isa<DTransStructType>(DTy) && "Expected StructType");
  auto *STy = cast<DTransStructType>(DTy);
  if (STy->getNumFields() != 1)
    return nullptr;
  return STy->getFieldType(0);
}

DTransStructType *getValidStructTy(DTransType *Ty) {
  DTransStructType *STy = dyn_cast_or_null<DTransStructType>(Ty);
  if (!STy)
    return nullptr;
  auto *StructT = cast<StructType>(STy->getLLVMType());
  if (StructT->isLiteral() || !StructT->isSized())
    return nullptr;
  return STy;
}

// Returns type of pointee if 'Ty' is pointer.
DTransType *getPointeeType(DTransType *Ty) {
  if (auto *PTy = dyn_cast_or_null<DTransPointerType>(Ty))
    return PTy->getPointerElementType();
  return nullptr;
}

// Returns true if 'Ty' is potential padding field that
// is created to fill gaps in structs.
bool isPotentialPaddingField(DTransType *Ty) {
  ArrayType *ATy = dyn_cast<ArrayType>(Ty->getLLVMType());
  if (!ATy || !ATy->getElementType()->isIntegerTy(8))
    return false;
  return true;
}

} // namespace dtransOP

} // namespace llvm
