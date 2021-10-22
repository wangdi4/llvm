//===--------------- Intel_IMFUtilsTest.cpp - IMLUtils Tests --------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Utils/Intel_IMLUtils.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

#include "gtest/gtest.h"

using namespace llvm;

namespace {

class IMLUtilsTest : public testing::Test {
};

TEST_F(IMLUtilsTest, DetermineOCLSVMLCallConv) {
  LLVMContext Context;
  Context.enableOpaquePointers();
  Type *FloatTy = Type::getFloatTy(Context);
  Type *DoubleTy = Type::getDoubleTy(Context);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__ocl_svml_g9_cvtfptoi64rtpsatf3",
                FunctionType::get(
                    VectorType::get(Type::getInt64Ty(Context), 3, false),
                    {VectorType::get(FloatTy, 3, false)}, false)),
            CallingConv::SVML_Unified_256);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_x0_erff1", FunctionType::get(FloatTy, {FloatTy}, false)),
      CallingConv::SVML_Unified_512);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_e9_log2f1", FunctionType::get(FloatTy, {FloatTy}, false)),
      CallingConv::SVML_Unified_256);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_l9_asinh4",
          FunctionType::get(VectorType::get(DoubleTy, 4, false),
                            {VectorType::get(DoubleTy, 4, false)}, false)),
      CallingConv::SVML_Unified_256);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_l9_asinh8",
          FunctionType::get(VectorType::get(DoubleTy, 8, false),
                            {VectorType::get(DoubleTy, 8, false)}, false)),
      CallingConv::SVML_Unified_256);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_h8_cos4",
          FunctionType::get(
              StructType::get(Context, {VectorType::get(DoubleTy, 2, false),
                                        VectorType::get(DoubleTy, 2, false)}),
              {StructType::get(Context, {VectorType::get(DoubleTy, 2, false),
                                         VectorType::get(DoubleTy, 2, false)})},
              false)),
      CallingConv::SVML_Unified);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__ocl_svml_z0_log1pf2",
                FunctionType::get(VectorType::get(FloatTy, 8, false),
                                  {VectorType::get(FloatTy, 8, false)}, false)),
            CallingConv::SVML_Unified_512);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__ocl_svml_e9_sincosf2",
                FunctionType::get(VectorType::get(FloatTy, 2, false),
                                  {VectorType::get(FloatTy, 2, false),
                                   PointerType::getUnqual(Context)},
                                  false)),
            CallingConv::SVML_Unified_256);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_b3_asinh4",
          FunctionType::get(VectorType::get(DoubleTy, 4, false),
                            {VectorType::get(DoubleTy, 4, false)}, false)),
      CallingConv::SVML_Unified_512);
}

}
