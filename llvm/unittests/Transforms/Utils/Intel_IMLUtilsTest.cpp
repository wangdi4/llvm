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
public:
  IMLUtilsTest() : testing::Test() {
    Context.setOpaquePointers(true);
    HalfTy = Type::getHalfTy(Context);
    FloatTy = Type::getFloatTy(Context);
    DoubleTy = Type::getDoubleTy(Context);
  }
protected:
  LLVMContext Context;

  Type *HalfTy;
  Type *FloatTy;
  Type *DoubleTy;
};

TEST_F(IMLUtilsTest, DetermineOCLSVMLCallConv) {
  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__ocl_svml_g9_cvtfptoi64rtpsatf3",
                FunctionType::get(
                    VectorType::get(Type::getInt64Ty(Context), 3, false),
                    {VectorType::get(FloatTy, 3, false)}, false))
                .value(),
            CallingConv::SVML_Unified_256);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_x0_erff1", FunctionType::get(FloatTy, {FloatTy}, false))
          .value(),
      CallingConv::SVML_Unified_512);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_e9_log2f1", FunctionType::get(FloatTy, {FloatTy}, false))
          .value(),
      CallingConv::SVML_Unified_256);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__ocl_svml_l9_asinh4",
                FunctionType::get(VectorType::get(DoubleTy, 4, false),
                                  {VectorType::get(DoubleTy, 4, false)}, false))
                .value(),
            CallingConv::SVML_Unified_256);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__ocl_svml_l9_asinh8",
                FunctionType::get(VectorType::get(DoubleTy, 8, false),
                                  {VectorType::get(DoubleTy, 8, false)}, false))
                .value(),
            CallingConv::SVML_Unified_256);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_h8_cos4",
          FunctionType::get(
              StructType::get(Context, {VectorType::get(DoubleTy, 2, false),
                                        VectorType::get(DoubleTy, 2, false)}),
              {StructType::get(Context, {VectorType::get(DoubleTy, 2, false),
                                         VectorType::get(DoubleTy, 2, false)})},
              false))
          .value(),
      CallingConv::SVML_Unified);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__ocl_svml_z0_log1pf2",
                FunctionType::get(VectorType::get(FloatTy, 8, false),
                                  {VectorType::get(FloatTy, 8, false)}, false))
                .value(),
            CallingConv::SVML_Unified_512);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__ocl_svml_e9_sincosf2",
                FunctionType::get(VectorType::get(FloatTy, 2, false),
                                  {VectorType::get(FloatTy, 2, false),
                                   PointerType::getUnqual(Context)},
                                  false))
                .value(),
            CallingConv::SVML_Unified_256);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__ocl_svml_b3_asinh4",
                FunctionType::get(VectorType::get(DoubleTy, 4, false),
                                  {VectorType::get(DoubleTy, 4, false)}, false))
                .value(),
            CallingConv::SVML_Unified_512);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_00_cos4",
          FunctionType::get(VectorType::get(DoubleTy, 4, false),
                            {VectorType::get(DoubleTy, 4, false)}, false)),
      None);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__ocl_svml_shared_acospif3",
          FunctionType::get(VectorType::get(FloatTy, 3, false),
                            {VectorType::get(FloatTy, 3, false)}, false)),
      None);
}

TEST_F(IMLUtilsTest, DetermineCSVMLCallConv) {
  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__svml_sincoss8_br_x1",
          FunctionType::get(
              StructType::get(Context, {VectorType::get(HalfTy, 8, false),
                                        VectorType::get(HalfTy, 8, false)}),
              {VectorType::get(HalfTy, 8, false)}, false))
          .value(),
      CallingConv::SVML_Unified);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "__svml_sind1_br",
          FunctionType::get(
              StructType::get(Context, {VectorType::get(DoubleTy, 2, false),
                                        VectorType::get(DoubleTy, 2, false)}),
              {VectorType::get(DoubleTy, 2, false)}, false))
          .value(),
      CallingConv::SVML_Unified);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__svml_clog4_mask_chosen_core_func_init_internal",
                FunctionType::get(
                    VectorType::get(DoubleTy, 8, false),
                    {VectorType::get(DoubleTy, 8, false),
                     VectorType::get(Type::getInt1Ty(Context), 16, false),
                     VectorType::get(DoubleTy, 8, false)},
                    false))
                .value(),
            CallingConv::SVML_Unified_512);

  EXPECT_EQ(getSVMLCallingConvByNameAndType(
                "__svml_powrf4",
                FunctionType::get(VectorType::get(FloatTy, 4, false),
                                  {VectorType::get(FloatTy, 4, false),
                                   VectorType::get(FloatTy, 4, false)},
                                  false))
                .value(),
            CallingConv::SVML_Unified);

  EXPECT_EQ(
      getSVMLCallingConvByNameAndType(
          "_svml_fdimf16_z0",
          FunctionType::get(VectorType::get(FloatTy, 16, false),
                            {VectorType::get(FloatTy, 16, false)}, false)),
      None);
}

}
