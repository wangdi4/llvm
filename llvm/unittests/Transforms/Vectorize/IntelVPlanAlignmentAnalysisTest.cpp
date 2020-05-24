//===- IntelVPlanAlignmentAnalysisTest.cpp ----------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAlignmentAnalysis.h"

#include "IntelVPlanTestBase.h"

#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

namespace {

class VPlanPeelingVariantTest : public ::testing::Test {};

TEST_F(VPlanPeelingVariantTest, StaticPeelingClass) {
  VPlanStaticPeeling StaticPeeling(42);
  VPlanPeelingVariant *Obfuscated = &StaticPeeling;
  EXPECT_TRUE(isa<VPlanStaticPeeling>(Obfuscated));
  EXPECT_FALSE(isa<VPlanDynamicPeeling>(Obfuscated));
  EXPECT_EQ(cast<VPlanStaticPeeling>(Obfuscated), &StaticPeeling);
  EXPECT_EQ(dyn_cast<VPlanDynamicPeeling>(Obfuscated), nullptr);
}

TEST_F(VPlanPeelingVariantTest, DynamicPeelingClass) {
  VPlanDynamicPeeling DynamicPeeling(nullptr, {nullptr, 1}, Align{2});
  VPlanPeelingVariant *Obfuscated = &DynamicPeeling;
  EXPECT_TRUE(isa<VPlanDynamicPeeling>(Obfuscated));
  EXPECT_FALSE(isa<VPlanStaticPeeling>(Obfuscated));
  EXPECT_EQ(dyn_cast<VPlanDynamicPeeling>(Obfuscated), &DynamicPeeling);
  EXPECT_EQ(dyn_cast<VPlanStaticPeeling>(Obfuscated), nullptr);
}

TEST_F(VPlanPeelingVariantTest, DynamicPeelingParameters) {
  auto mkPeeling = [](int Step, Align TargetAlignment) -> VPlanDynamicPeeling {
    return VPlanDynamicPeeling(nullptr, {nullptr, Step}, TargetAlignment);
  };

  auto P1 = mkPeeling(1, Align{4});
  EXPECT_EQ(P1.requiredAlignment(), 1);
  EXPECT_EQ(P1.multiplier(), 3);

  auto P2 = mkPeeling(2, Align{16});
  EXPECT_EQ(P2.requiredAlignment(), 2);
  EXPECT_EQ(P2.multiplier(), 7);

  auto P3 = mkPeeling(3, Align{8});
  EXPECT_EQ(P3.requiredAlignment(), 1);
  EXPECT_EQ(P3.multiplier(), 5);

  auto P4 = mkPeeling(12, Align{16});
  EXPECT_EQ(P4.requiredAlignment(), 4);
  EXPECT_EQ(P4.multiplier(), 1);

  auto P5 = mkPeeling(18, Align{8});
  EXPECT_EQ(P5.requiredAlignment(), 2);
  EXPECT_EQ(P5.multiplier(), 3);
}

class VPlanPeelingAnalysisTest : public vpo::VPlanTestBase {
protected:
  Function *FuncFoo;
  std::unique_ptr<VPlan> Plan;
  std::unique_ptr<VPlanScalarEvolutionLLVM> VPSE;
  std::unique_ptr<VPlanPeelingAnalysis> VPPA;

  void buildVPlanFromString(const char* ModuleString) {
    Module &M = parseModule(ModuleString);
    FuncFoo = M.getFunction("foo");
    BasicBlock *LoopHeader = FuncFoo->getEntryBlock().getSingleSuccessor();
    Plan = buildHCFG(LoopHeader);
  }

  void setupPeelingAnalysis(VPlanPeelingCostModel &CM) {
    VPSE = std::make_unique<VPlanScalarEvolutionLLVM>(*SE, *LI->begin());
    VPPA = std::make_unique<VPlanPeelingAnalysis>(CM, *VPSE, *DL);
    VPPA->collectMemrefs(*Plan);
  }
};

// According to this cost model, profit from alignment increases by one every
// time alignment doubles. Such cost model doesn't make practical value but it
// is useful for testing purposes.
class VPlanPeelingCostModelLog final : public VPlanPeelingCostModel {
public:
  int getCost(VPInstruction *Mrf, int VF, Align Alignment) override {
    return -Log2(Alignment);
  }
};

TEST_F(VPlanPeelingAnalysisTest, NoPeeling_NoUnitStride) {
  // No peeling, since there's no unit-strided store or load in the loop.
  buildVPlanFromString(
    "define void @foo(i32* %dst, i32* %src, i64 %size) {\n"
    "entry:\n"
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %counter_times_two = mul nsw nuw i64 %counter, 2\n"
    "  %src.ptr = getelementptr inbounds i32, i32* %src, i64 %counter_times_two\n"
    "  %src.val = load i32, i32* %src.ptr, align 4\n"
    "  %dst.val = add i32 %src.val, 42\n"
    "  %dst.ptr = getelementptr inbounds i32, i32* %dst, i64 %counter_times_two\n"
    "  store i32 %dst.val, i32* %dst.ptr, align 4\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, %size\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n");

  VPlanPeelingCostModelLog CM;
  setupPeelingAnalysis(CM);

  int VFs[] = {2, 4, 8, 16, 32, 64};
  for (auto VF : VFs) {
    std::unique_ptr<VPlanPeelingVariant> PV = VPPA->selectBestPeelingVariant(VF);
    ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV));
    VPlanStaticPeeling &SP = cast<VPlanStaticPeeling>(*PV);
    EXPECT_EQ(SP.peelCount(), 0);
  }
}

TEST_F(VPlanPeelingAnalysisTest, DynamicPeeling_Store) {
  // Peeling for a store must be preferred to a load of the same type.
  buildVPlanFromString(
    "define void @foo(i32* %dst, i32* %src, i64 %size) {\n"
    "entry:\n"
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %src.ptr = getelementptr inbounds i32, i32* %src, i64 %counter\n"
    "  %src.val = load i32, i32* %src.ptr, align 4\n"
    "  %dst.val = add i32 %src.val, 42\n"
    "  %dst.ptr = getelementptr inbounds i32, i32* %dst, i64 %counter\n"
    "  store i32 %dst.val, i32* %dst.ptr, align 4\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, %size\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n");

  VPlanPeelingCostModelSimple CM(*DL);
  setupPeelingAnalysis(CM);
  VPlanSCEV *DstScev = VPSE->toVPlanSCEV(SE->getSCEV(FuncFoo->getArg(0)));

  std::unique_ptr<VPlanPeelingVariant> PV4 = VPPA->selectBestPeelingVariant(4);
  ASSERT_TRUE(isa<VPlanDynamicPeeling>(*PV4));
  VPlanDynamicPeeling &DP4 = cast<VPlanDynamicPeeling>(*PV4);
  EXPECT_EQ(DP4.memref()->getOpcode(), Instruction::Store);
  EXPECT_EQ(DP4.invariantBase(), DstScev);
  EXPECT_EQ(DP4.requiredAlignment(), 4);
  EXPECT_EQ(DP4.targetAlignment(), 16);
  EXPECT_EQ(DP4.multiplier(), 3);

  std::unique_ptr<VPlanPeelingVariant> PV16 = VPPA->selectBestPeelingVariant(16);
  ASSERT_TRUE(isa<VPlanDynamicPeeling>(*PV16));
  VPlanDynamicPeeling &DP16 = cast<VPlanDynamicPeeling>(*PV16);
  EXPECT_EQ(DP16.memref()->getOpcode(), Instruction::Store);
  EXPECT_EQ(DP16.invariantBase(), DstScev);
  EXPECT_EQ(DP16.requiredAlignment(), 4);
  EXPECT_EQ(DP16.targetAlignment(), 64);
  EXPECT_EQ(DP16.multiplier(), 15);
}

TEST_F(VPlanPeelingAnalysisTest, DynamicPeeling_Load) {
  // The store in the function is not unit-strided, so the cost model has no
  // other choice but to choose the load as peeling target.
  buildVPlanFromString(
    "define void @foo(i32* %dst, i32* %src, i64 %size) {\n"
    "entry:\n"
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %src.ptr = getelementptr inbounds i32, i32* %src, i64 %counter\n"
    "  %src.val = load i32, i32* %src.ptr, align 4\n"
    "  %dst.val = add i32 %src.val, 42\n"
    "  %counter_times_two = mul nsw nuw i64 %counter, 2\n"
    "  %dst.ptr = getelementptr inbounds i32, i32* %dst, i64 %counter_times_two\n"
    "  store i32 %dst.val, i32* %dst.ptr, align 4\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, %size\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n");

  VPlanPeelingCostModelSimple CM(*DL);
  setupPeelingAnalysis(CM);
  VPlanSCEV *SrcScev = VPSE->toVPlanSCEV(SE->getSCEV(FuncFoo->getArg(1)));

  std::unique_ptr<VPlanPeelingVariant> PV4 = VPPA->selectBestPeelingVariant(4);
  ASSERT_TRUE(isa<VPlanDynamicPeeling>(*PV4));
  VPlanDynamicPeeling &DP4 = cast<VPlanDynamicPeeling>(*PV4);
  EXPECT_EQ(DP4.memref()->getOpcode(), Instruction::Load);
  EXPECT_EQ(DP4.invariantBase(), SrcScev);
  EXPECT_EQ(DP4.requiredAlignment(), 4);
  EXPECT_EQ(DP4.targetAlignment(), 16);
  EXPECT_EQ(DP4.multiplier(), 3);

  std::unique_ptr<VPlanPeelingVariant> PV16 = VPPA->selectBestPeelingVariant(16);
  ASSERT_TRUE(isa<VPlanDynamicPeeling>(*PV16));
  VPlanDynamicPeeling &DP16 = cast<VPlanDynamicPeeling>(*PV16);
  EXPECT_EQ(DP16.memref()->getOpcode(), Instruction::Load);
  EXPECT_EQ(DP16.invariantBase(), SrcScev);
  EXPECT_EQ(DP16.requiredAlignment(), 4);
  EXPECT_EQ(DP16.targetAlignment(), 64);
  EXPECT_EQ(DP16.multiplier(), 15);
}

} // namespace
