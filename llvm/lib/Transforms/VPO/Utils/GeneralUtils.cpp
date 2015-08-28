//====-- GeneralUtils.cpp - General set of utilities for VPO -*- C++ -*---====//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/IR/Constants.h"

using namespace llvm;
using namespace llvm::vpo;

template Constant* VPOUtils::getConstantValue<int>(Type *Ty,
                                                   LLVMContext &Context,
                                                   int Val);

template Constant* VPOUtils::getConstantValue<float>(Type *Ty,
                                                      LLVMContext &Context,
                                                      float Val);

template Constant* VPOUtils::getConstantValue<double>(Type *Ty,
                                                      LLVMContext &Context,
                                                      double Val);

template <typename T>
Constant* VPOUtils::getConstantValue(Type *Ty, LLVMContext &Context, T Val)
{
  Constant *ConstVal = nullptr;

  if (Ty->isIntegerTy(8)) {
    ConstVal = ConstantInt::get(Type::getInt8Ty(Context), Val);
  } else if (Ty->isIntegerTy(16)) {
    ConstVal = ConstantInt::get(Type::getInt16Ty(Context), Val);
  } else if (Ty->isIntegerTy(32)) {
    ConstVal = ConstantInt::get(Type::getInt32Ty(Context), Val);
  } else if (Ty->isIntegerTy(64)) {
    ConstVal = ConstantInt::get(Type::getInt64Ty(Context), Val);
  } else if (Ty->isFloatTy()) {
    ConstVal = ConstantFP::get(Type::getFloatTy(Context), Val);
  } else if (Ty->isDoubleTy()) {
    ConstVal = ConstantFP::get(Type::getDoubleTy(Context), Val);
  }

  assert (ConstVal && "Could not generate constant for type");

  return ConstVal;
}
