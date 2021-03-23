//===- IntelVPlanAlignmentAnalysisTest.cpp ----------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanTTIWrapper.h"
#include "IntelVPlanTestBase.h"
#include "llvm/ADT/Triple.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

namespace {

static std::unique_ptr<TargetMachine>
createTargetMachine(std::string CPUStr, std::string FeaturesStr) {
  auto TT(Triple::normalize("x86_64--"));
  std::string Error;

  const Target *TheTarget = TargetRegistry::lookupTarget(TT, Error);
  return std::unique_ptr<TargetMachine>(
      TheTarget->createTargetMachine(TT, CPUStr, FeaturesStr, TargetOptions(),
                                     None, None, CodeGenOpt::Default));
}

class VPlanTTIMemrefCostTest : public vpo::VPlanTestBase {
protected:
  const std::string CPUStr = "skx";
  std::string FeaturesStr = "";

  std::unique_ptr<DataLayout> DL;
  std::unique_ptr<TargetMachine> TM;
  std::unique_ptr<VPlanTTIWrapper> VPTTI;

  static unsigned constexpr AS = 0;

protected:
  static void SetUpTestCase() {
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
  }

  void SetUp() override {
    TM = createTargetMachine(CPUStr, FeaturesStr);
    buildVPTTI();
  }

  void buildVPTTI() {
    Module &M = parseModule(
        "target datalayout = \"e-m:e-i64:64-f80:128-n8:16:32:64-S128\"\n"
        "target triple = \"x86_64-unknown-linux-gnu\"\n"

        "define void @foo(i32* %dst, i32* %src, i64 %size) {\n"
        "entry:\n"
        "  br label %for.body\n"
        "for.body:\n"
        "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
        "  %counter_times_two = mul nsw nuw i64 %counter, 2\n"
        "  %src.ptr = getelementptr inbounds i32, i32* %src, i64 "
        "%counter_times_two\n"
        "  %src.val = load i32, i32* %src.ptr, align 4\n"
        "  %dst.val = add i32 %src.val, 42\n"
        "  %dst.ptr = getelementptr inbounds i32, i32* %dst, i64 "
        "%counter_times_two\n"
        "  store i32 %dst.val, i32* %dst.ptr, align 4\n"
        "  %counter.next = add nsw i64 %counter, 1\n"
        "  %exitcond = icmp sge i64 %counter.next, %size\n"
        "  br i1 %exitcond, label %exit, label %for.body\n"
        "exit:\n"
        "  ret void\n"
        "}\n");

    const Function *Func = M.getFunction("foo");
    EXPECT_TRUE(Func);
    auto TTIPass =
        createTargetTransformInfoWrapperPass(TM->getTargetIRAnalysis());
    const TargetTransformInfo &TTI =
        (static_cast<TargetTransformInfoWrapperPass *>(TTIPass))->getTTI(*Func);
    DL.reset(new DataLayout(&M));
    VPTTI.reset(new VPlanTTIWrapper(TTI, *DL.get()));
  }
};

TEST_F(VPlanTTIMemrefCostTest, CheckTMSetup) {
  // Check the max vector register width to verify TM setup.
  EXPECT_EQ(VPTTI->getRegisterBitWidth(true), 256u);
}

TEST_F(VPlanTTIMemrefCostTest, ScalarLoadCost) {
  // Expect regular cost for scalars.
  const int Expected = 1000;
  const unsigned Alignment = 4;
  auto ScalarType = Type::getInt32Ty(*Ctx);
  auto Actual = VPTTI->getMemoryOpCost(Instruction::Load, ScalarType,
                                       Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

// Check irregular types.

TEST_F(VPlanTTIMemrefCostTest, Check_1xi1) {
  const int Expected = 1000;
  const unsigned Alignment = 4;
  auto Type1xi1 = FixedVectorType::get(Type::getInt1Ty(*Ctx), 1);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Load, Type1xi1, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

TEST_F(VPlanTTIMemrefCostTest, Check1xi8Ptr) {
  const int Expected = 1000;
  const unsigned Alignment = 4;
  const unsigned VF = 1;
  const auto Tyi8Ptr = Type::getInt8Ty(*Ctx);
  auto Ty1xi8Ptr = FixedVectorType::get(Tyi8Ptr->getPointerTo(), VF);
  auto Actual = VPTTI->getMemoryOpCost(Instruction::Store, Ty1xi8Ptr,
                                       Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

TEST_F(VPlanTTIMemrefCostTest, Check1xi8) {
  const int Expected = 1000;
  const unsigned Alignment = 4;
  const unsigned VF = 1;
  const auto Tyi8 = Type::getInt8Ty(*Ctx);
  auto Ty1xi8 = FixedVectorType::get(Tyi8, VF);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Store, Ty1xi8, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

TEST_F(VPlanTTIMemrefCostTest, Check7xi8) {
  // Read 7 x i8 (7 bytes).
  // Reading 7 bytes is less than Alignment, so it is aligned.
  const int Expected = 1000;
  const unsigned Alignment = 8;
  const unsigned VF = 7;
  const auto Tyi8 = Type::getInt8Ty(*Ctx);
  auto Ty7xi8 = FixedVectorType::get(Tyi8, VF);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Store, Ty7xi8, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

TEST_F(VPlanTTIMemrefCostTest, AlignedStore) {
  const int Expected = 1000;
  const unsigned Alignment = 16;
  const unsigned VF = 4;
  // Such store is aligned.
  auto VecTy = FixedVectorType::get(Type::getInt32Ty(*Ctx), VF);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Store, VecTy, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

TEST_F(VPlanTTIMemrefCostTest, UnalignedStoreLowProbility) {
  const int Expected = 1001;
  const unsigned Alignment = 4;
  const unsigned VF = 4;
  // Within the cache line it has low probability to be unaligned.
  auto VecTy = FixedVectorType::get(Type::getInt32Ty(*Ctx), VF);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Store, VecTy, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

TEST_F(VPlanTTIMemrefCostTest, UnalignedStoreHighProbability) {
  const int Expected = 1003;
  const unsigned Alignment = 8;
  const unsigned VF = 16;
  // Within 64 byte cache line it has high probability to be unaligned.
  auto VecTy = FixedVectorType::get(Type::getInt32Ty(*Ctx), VF);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Store, VecTy, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

TEST_F(VPlanTTIMemrefCostTest, Check2PartLoadCost) {
  // Expect double penalty for very wide store.
  const int Expected = 2006;
  const unsigned Alignment = 4;
  const unsigned VF = 32;
  auto VecTy = FixedVectorType::get(Type::getInt32Ty(*Ctx), VF);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Store, VecTy, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

// Common cases

TEST_F(VPlanTTIMemrefCostTest, Align64Size64) {
  const int Expected = 1000;
  const unsigned Alignment = 64;
  const unsigned VF = 16;
  auto VecTy = FixedVectorType::get(Type::getFloatTy(*Ctx), VF);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Store, VecTy, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

TEST_F(VPlanTTIMemrefCostTest, Align32Size32) {
  const int Expected = 1000;
  const unsigned Alignment = 32;
  const unsigned VF = 8;
  auto VecTy = FixedVectorType::get(Type::getFloatTy(*Ctx), VF);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Store, VecTy, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

TEST_F(VPlanTTIMemrefCostTest, Align64Size32) {
  const int Expected = 1000;
  const unsigned Alignment = 64;
  const unsigned VF = 8;
  auto VecTy = FixedVectorType::get(Type::getFloatTy(*Ctx), VF);
  auto Actual =
      VPTTI->getMemoryOpCost(Instruction::Store, VecTy, Align(Alignment), AS);
  EXPECT_EQ(Actual, Expected);
}

// Consistency

TEST_F(VPlanTTIMemrefCostTest, 2xVectorGives2xCost) {
  // Check that cost of <8 x double> is twice of <16 x double>
  const unsigned Alignment = 8;
  auto Ty8xdouble = FixedVectorType::get(Type::getDoubleTy(*Ctx), 8);
  auto Ty16xdouble = FixedVectorType::get(Type::getDoubleTy(*Ctx), 16);
  const auto Ty8xdoubleCost =
    VPTTI->getMemoryOpCost(
      Instruction::Store, Ty8xdouble, Align(Alignment), AS);
  const auto Ty16xdoubleCost =
    VPTTI->getMemoryOpCost(
      Instruction::Store, Ty16xdouble, Align(Alignment), AS);
  EXPECT_TRUE(Ty8xdoubleCost * 2 == Ty16xdoubleCost);
}

} // namespace
