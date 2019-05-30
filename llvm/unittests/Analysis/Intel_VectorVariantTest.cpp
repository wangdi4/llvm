//===- Intel_VectorVariantTest.cpp - Vector ABI Mangling tests --*- C++ -*-===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VectorVariant.h"

#include "gtest/gtest.h"

using namespace llvm;

namespace {

TEST(VectorManglingTest, Basic) {
  std::string FuncName = "_ZGVcN8v_foo";
  VectorVariant VV(FuncName);

  EXPECT_TRUE(VV.getISA() == VectorVariant::ISAClass::YMM1);
  EXPECT_FALSE(VV.isMasked());
  EXPECT_EQ(VV.getVlen(), 8u);

  auto Params = VV.getParameters();
  EXPECT_EQ(Params.size(), 1u);
  EXPECT_TRUE(Params[0].isVector());
  EXPECT_FALSE(Params[0].isAligned());


  EXPECT_TRUE(FuncName == VV.generateFunctionName("foo"));
}

TEST(VectorManglingTest, VariableStride) {
  std::string FuncName = "_ZGVcN8ls1u_foo";
  VectorVariant VV(FuncName);

  auto Params = VV.getParameters();
  EXPECT_EQ(Params.size(), 2u);
  EXPECT_TRUE(Params[0].isVariableStride());
  EXPECT_EQ(Params[0].getStrideArgumentPosition(), 1);

  EXPECT_TRUE(FuncName == VV.generateFunctionName("foo"));
}

} // anonymous namespace
