//===------- Intel_IMFUtils.cpp - Utilites for Intel Math Libraries -------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// Implementation of IML utilities.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Utils/Intel_IMLUtils.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

bool llvm::isSVMLCallingConv(CallingConv::ID CC) {
  return CC == CallingConv::SVML || CC == CallingConv::SVML_AVX ||
         CC == CallingConv::SVML_AVX512 || CC == CallingConv::SVML_Unified ||
         CC == CallingConv::SVML_Unified_256 ||
         CC == CallingConv::SVML_Unified_512;
}

VectorType *llvm::getVectorTypeForCSVMLFunction(FunctionType *FT) {
  Type *CallRetType = FT->getReturnType();
  auto *RetStructTy = dyn_cast_or_null<StructType>(CallRetType);
  // For structure return types, return the first structure element as
  // a vector type.
  if (RetStructTy && RetStructTy->getNumElements())
    return dyn_cast<VectorType>(RetStructTy->getElementType(0));
  else
    return dyn_cast<VectorType>(CallRetType);
}

static CallingConv::ID getCSVMLCallingConvByVectorSize(unsigned Size) {
  assert(isPowerOf2_32(Size) && Size <= 512 && "Invalid vector size");
  if (Size <= 128)
    return CallingConv::SVML_Unified;
  else if (Size == 256)
    return CallingConv::SVML_Unified_256;
  else if (Size == 512)
    return CallingConv::SVML_Unified_512;
  else
    llvm_unreachable("Invalid vector size");
}

CallingConv::ID llvm::getSVMLCallingConvByNameAndType(StringRef FnName,
                                                      FunctionType *FT) {
  if (FnName.startswith("__svml_"))
    return getCSVMLCallingConvByVectorSize(
        getVectorTypeForCSVMLFunction(FT)->getPrimitiveSizeInBits());

  llvm_unreachable("Invalid function name for SVML function");
}

CallingConv::ID llvm::getLegacyCSVMLCallingConvFromUnified(CallingConv::ID CC) {
  switch (CC) {
    case CallingConv::SVML_Unified:
      return CallingConv::SVML;
    case CallingConv::SVML_Unified_256:
      return CallingConv::SVML_AVX;
    case CallingConv::SVML_Unified_512:
      return CallingConv::SVML_AVX512;
    default:
      llvm_unreachable("Expect one of unified SVML calling conventions");
  }
}
