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

class VPlanPeelingAnalysisTest : public vpo::VPlanTestBase {};

TEST_F(VPlanPeelingAnalysisTest, NoPeeling) {
  const char *ModuleString =
    "declare void @bar(i32)\n"
    "define void @foo(i32* %src, i64 %size) {\n"
    "entry:\n"
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %src.ptr = getelementptr inbounds i32, i32* %src, i64 %counter\n"
    "  %src.val = load i32, i32* %src.ptr, align 4\n"
    "  call void @bar(i32 %src.val)\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, %size\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n";

  Module &M = parseModule(ModuleString);
  Function *F = M.getFunction("foo");
  BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
  std::unique_ptr<VPlan> Plan = buildHCFG(LoopHeader);

  VPlanScalarEvolutionLLVM VPSE(*SE, *LI->begin());
  VPlanPeelingAnalysis VPPA(VPSE);
  VPPA.collectMemrefs(*Plan);

  std::unique_ptr<VPlanPeelingVariant> PV4 = VPPA.selectBestPeelingVariant(4);
  std::unique_ptr<VPlanPeelingVariant> PV16 = VPPA.selectBestPeelingVariant(16);

  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV4));
  VPlanStaticPeeling &SP4 = cast<VPlanStaticPeeling>(*PV4);
  EXPECT_EQ(SP4.peelCount(), 0);

  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV16));
  VPlanStaticPeeling &SP16 = cast<VPlanStaticPeeling>(*PV16);
  EXPECT_EQ(SP16.peelCount(), 0);
}

TEST_F(VPlanPeelingAnalysisTest, DynamicPeeling) {
  const char *ModuleString =
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
      "}\n";

  Module &M = parseModule(ModuleString);
  Function *F = M.getFunction("foo");
  BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
  std::unique_ptr<VPlan> Plan = buildHCFG(LoopHeader);

  VPlanScalarEvolutionLLVM VPSE(*SE, *LI->begin());
  VPlanSCEV *DstScev = VPSE.toVPlanSCEV(SE->getSCEV(F->getArg(0)));

  VPlanPeelingAnalysis VPPA(VPSE);
  VPPA.collectMemrefs(*Plan);

  std::unique_ptr<VPlanPeelingVariant> PV4 = VPPA.selectBestPeelingVariant(4);
  std::unique_ptr<VPlanPeelingVariant> PV16 = VPPA.selectBestPeelingVariant(16);

  ASSERT_TRUE(isa<VPlanDynamicPeeling>(*PV4));
  VPlanDynamicPeeling &DP4 = cast<VPlanDynamicPeeling>(*PV4);
  EXPECT_EQ(DP4.memref()->getOpcode(), Instruction::Store);
  EXPECT_EQ(DP4.invariantBase(), DstScev);
  EXPECT_EQ(DP4.requiredAlignment(), 4);
  EXPECT_EQ(DP4.targetAlignment(), 16);
  EXPECT_EQ(DP4.multiplier(), 3);

  ASSERT_TRUE(isa<VPlanDynamicPeeling>(*PV16));
  VPlanDynamicPeeling &DP16 = cast<VPlanDynamicPeeling>(*PV16);
  EXPECT_EQ(DP16.memref()->getOpcode(), Instruction::Store);
  EXPECT_EQ(DP16.invariantBase(), DstScev);
  EXPECT_EQ(DP16.requiredAlignment(), 4);
  EXPECT_EQ(DP16.targetAlignment(), 64);
  EXPECT_EQ(DP16.multiplier(), 15);
}

} // namespace
