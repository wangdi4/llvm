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
  std::unique_ptr<VPlanValueTrackingLLVM> VPVT;
  std::unique_ptr<VPlanPeelingAnalysis> VPPA;

  void buildVPlanFromString(const char* ModuleString) {
    Module &M = parseModule(ModuleString);
    FuncFoo = M.getFunction("foo");
    BasicBlock *LoopHeader = FuncFoo->getEntryBlock().getSingleSuccessor();
    Plan = buildHCFG(LoopHeader);
  }

  void setupPeelingAnalysis(VPlanPeelingCostModel &CM) {
    VPSE = std::make_unique<VPlanScalarEvolutionLLVM>(*SE, *LI->begin());
    VPVT = std::make_unique<VPlanValueTrackingLLVM>(*VPSE, *DL, &*AC, &*DT);
    VPPA = std::make_unique<VPlanPeelingAnalysis>(CM, *VPSE, *VPVT, *DL);
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

TEST_F(VPlanPeelingAnalysisTest, NoPeeling_Misalign) {
  // No peeling, since there's no properly aligned memory references in the loop.
  buildVPlanFromString(
    "define void @foo(i16* %buf1, i32* %buf2, i32* %buf3, i32* %buf4, i64* %buf5) {\n"
    "entry:\n"
    "  %buf1.asInt = ptrtoint i16* %buf1 to i64\n"
    "  %buf2.asInt = ptrtoint i32* %buf2 to i64\n"
    "  %buf3.asInt = ptrtoint i32* %buf3 to i64\n"
    "  %buf4.asInt = ptrtoint i32* %buf4 to i64\n"
    "  %buf5.asInt = ptrtoint i64* %buf5 to i64\n"
    "  %tmp1 = and i64 %buf1.asInt, -1024\n"
    "  %tmp2 = and i64 %buf2.asInt, -1024\n"
    "  %tmp3 = and i64 %buf3.asInt, -1024\n"
    "  %tmp4 = and i64 %buf4.asInt, -1024\n"
    "  %tmp5 = and i64 %buf5.asInt, -1024\n"
    "  %ptr1.asInt = or i64 %tmp1, 3\n"
    "  %ptr2.asInt = or i64 %tmp2, 5\n"
    "  %ptr3.asInt = or i64 %tmp3, 6\n"
    "  %ptr4.asInt = or i64 %tmp4, 7\n"
    "  %ptr5.asInt = or i64 %tmp5, 10\n"
    "  %ptr1 = inttoptr i64 %ptr1.asInt to i16*\n"
    "  %ptr2 = inttoptr i64 %ptr2.asInt to i32*\n"
    "  %ptr3 = inttoptr i64 %ptr3.asInt to i32*\n"
    "  %ptr4 = inttoptr i64 %ptr4.asInt to i32*\n"
    "  %ptr5 = inttoptr i64 %ptr5.asInt to i64*\n"
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %count16 = trunc i64 %counter to i16\n"
    "  %count32 = trunc i64 %counter to i32\n"
    "  %p1 = getelementptr inbounds i16, i16* %ptr1, i64 %counter\n"
    "  %p2 = getelementptr inbounds i32, i32* %ptr2, i64 %counter\n"
    "  %p3 = getelementptr inbounds i32, i32* %ptr3, i64 %counter\n"
    "  %p4 = getelementptr inbounds i32, i32* %ptr4, i64 %counter\n"
    "  %p5 = getelementptr inbounds i64, i64* %ptr5, i64 %counter\n"
    "  store i16 %count16, i16* %p1, align 1\n"
    "  store i32 %count32, i32* %p2, align 1\n"
    "  store i32 %count32, i32* %p3, align 1\n"
    "  store i32 %count32, i32* %p4, align 1\n"
    "  store i64 %counter, i64* %p5, align 1\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, 10240\n"
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

TEST_F(VPlanPeelingAnalysisTest, DynamicPeeling_Cost) {
  buildVPlanFromString(
    "define void @foo(i32 *%buf, i64 %x) {\n"
    "entry:\n"
    "  %offset0 = mul i64 %x, 16\n"
    "  %offset1 = mul i64 %x, 32\n"
    "  %offset2 = mul i64 %x, 64\n"
    "  %offset3 = add i64 %offset0, 3\n"
    "  %offset4 = add i64 %offset1, 5\n"
    "  %offset5 = add i64 %offset2, 7\n"
    // %ptr1 = %buf + 128 * %x
    "  %ptr1 = getelementptr inbounds i32, i32* %buf, i64 %offset1\n"
    // %ptr2 = %buf + 256 * %x
    "  %ptr2 = getelementptr inbounds i32, i32* %buf, i64 %offset2\n"
    // %ptr3 = %buf + 64 * %x + 12
    "  %ptr3 = getelementptr inbounds i32, i32* %buf, i64 %offset3\n"
    // %ptr4 = %buf + 128 * %x + 20
    "  %ptr4 = getelementptr inbounds i32, i32* %buf, i64 %offset4\n"
    // %ptr5 = %buf + 256 * %x + 28
    "  %ptr5 = getelementptr inbounds i32, i32* %buf, i64 %offset5\n"
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %count32 = trunc i64 %counter to i32\n"
    "  %p1 = getelementptr inbounds i32, i32* %ptr1, i64 %counter\n"
    "  %p2 = getelementptr inbounds i32, i32* %ptr2, i64 %counter\n"
    "  %p3 = getelementptr inbounds i32, i32* %ptr3, i64 %counter\n"
    "  %p4 = getelementptr inbounds i32, i32* %ptr4, i64 %counter\n"
    "  %p5 = getelementptr inbounds i32, i32* %ptr5, i64 %counter\n"
    "  store i32 %count32, i32* %p1\n"
    "  store i32 %count32, i32* %p2\n"
    "  store i32 %count32, i32* %p3\n"
    "  store i32 %count32, i32* %p4\n"
    "  store i32 %count32, i32* %p5\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, 10240\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n");

  VPlanPeelingCostModelLog CM;
  setupPeelingAnalysis(CM);

  // Check that the dynamic variant beats the static one.
  std::unique_ptr<VPlanPeelingVariant> PV4 = VPPA->selectBestPeelingVariant(4);
  EXPECT_TRUE(isa<VPlanDynamicPeeling>(*PV4));

  // VF = 4.
  // Best alignment:
  //   %ptr1: 4 ->  4 (cost -= 0)
  //   %ptr2: 4 ->  4 (cost -= 0)
  //   %ptr3: 4 -> 16 (cost -= 2)
  //   %ptr4: 4 ->  8 (cost -= 1)
  //   %ptr5: 4 -> 16 (cost -= 2)
  Optional<std::pair<VPlanDynamicPeeling, int>> P4 =
      VPPA->selectBestDynamicPeelingVariant(4);
  EXPECT_EQ(P4->second, 5);

  std::string S4;
  raw_string_ostream OS4(S4);
  VPSE->toSCEV(P4->first.invariantBase())->print(OS4);
  EXPECT_EQ(S4, "(12 + (64 * %x) + %buf)");
  EXPECT_EQ(P4->first.targetAlignment(), 16);

  // VF = 16.
  // Best alignment:
  //   %ptr1: 4 -> 64 (cost -= 4)
  //   %ptr2: 4 -> 64 (cost -= 4)
  //   %ptr3: 4 ->  4 (cost -= 0)
  //   %ptr4: 4 ->  4 (cost -= 0)
  //   %ptr5: 4 ->  4 (cost -= 0)
  Optional<std::pair<VPlanDynamicPeeling, int>> P16 =
      VPPA->selectBestDynamicPeelingVariant(16);
  EXPECT_EQ(P16->second, 8);

  std::string S16;
  raw_string_ostream OS16(S16);
  VPSE->toSCEV(P16->first.invariantBase())->print(OS16);
  EXPECT_EQ(S16, "((128 * %x) + %buf)<nsw>");
  EXPECT_EQ(P16->first.targetAlignment(), 64);
}

TEST_F(VPlanPeelingAnalysisTest, StaticPeeling_LowPeelCount) {
  // Static peeling with lower PeelCount should be preferred to peeling with
  // higher PeelCount if the profit is the same (same access type).
  buildVPlanFromString(
    "define void @foo(i32* %buf1, i32* %buf2, i32* %buf3) {\n"
    "entry:\n"
    "  %buf1.asInt = ptrtoint i32* %buf1 to i64\n"
    "  %buf2.asInt = ptrtoint i32* %buf2 to i64\n"
    "  %buf3.asInt = ptrtoint i32* %buf3 to i64\n"
    "  %tmp1 = and i64 %buf1.asInt, -64\n"
    "  %tmp2 = and i64 %buf2.asInt, -64\n"
    "  %tmp3 = and i64 %buf3.asInt, -64\n"
    "  %ptr1.asInt = or i64 %tmp1, 52\n"
    "  %ptr2.asInt = or i64 %tmp2, 12\n"
    "  %ptr3.asInt = or i64 %tmp3, 24\n"
    "  %ptr1 = inttoptr i64 %ptr1.asInt to i32*\n" /* %ptr1 ≡ 52 (mod 64) */
    "  %ptr2 = inttoptr i64 %ptr2.asInt to i32*\n" /* %ptr2 ≡ 12 (mod 64) */
    "  %ptr3 = inttoptr i64 %ptr3.asInt to i32*\n" /* %ptr3 ≡ 24 (mod 64) */
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %count32 = trunc i64 %counter to i32\n"
    "  %p1 = getelementptr inbounds i32, i32* %ptr1, i64 %counter\n"
    "  %p2 = getelementptr inbounds i32, i32* %ptr2, i64 %counter\n"
    "  %p3 = getelementptr inbounds i32, i32* %ptr3, i64 %counter\n"
    "  store i32 %count32, i32* %p1\n"
    "  store i32 %count32, i32* %p2\n"
    "  store i32 %count32, i32* %p3\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, 10240\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n");

  VPlanPeelingCostModelSimple CM(*DL);
  setupPeelingAnalysis(CM);

  std::unique_ptr<VPlanPeelingVariant> PV4 = VPPA->selectBestPeelingVariant(4);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV4));
  VPlanStaticPeeling &SP4 = cast<VPlanStaticPeeling>(*PV4);
  EXPECT_EQ(SP4.peelCount(), 1); // Peel to align %p2 by 16.

  std::unique_ptr<VPlanPeelingVariant> PV8 = VPPA->selectBestPeelingVariant(8);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV8));
  VPlanStaticPeeling &SP8 = cast<VPlanStaticPeeling>(*PV8);
  EXPECT_EQ(SP8.peelCount(), 2); // Peel to align %p3 by 32.

  std::unique_ptr<VPlanPeelingVariant> PV16 = VPPA->selectBestPeelingVariant(16);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV16));
  VPlanStaticPeeling &SP16 = cast<VPlanStaticPeeling>(*PV16);
  EXPECT_EQ(SP16.peelCount(), 3); // Peel to align %p1 by 64.
}

TEST_F(VPlanPeelingAnalysisTest, StaticPeeling_StoreVsLoad) {
  // Peeling for the store must be preferred even if it results in higher
  // PeelCount.
  buildVPlanFromString(
    "define void @foo(i16* %buf1, i16* %buf2) {\n"
    "entry:\n"
    "  %buf1.asInt = ptrtoint i16* %buf1 to i64\n"
    "  %buf2.asInt = ptrtoint i16* %buf2 to i64\n"
    "  %tmp1 = and i64 %buf1.asInt, -64\n"
    "  %tmp2 = and i64 %buf2.asInt, -64\n"
    "  %ptr1.asInt = or i64 %tmp1, 62\n"
    "  %ptr2.asInt = or i64 %tmp2,  2\n"
    "  %ptr1 = inttoptr i64 %ptr1.asInt to i16*\n" /* %ptr1 ≡ 62 (mod 64) */
    "  %ptr2 = inttoptr i64 %ptr2.asInt to i16*\n" /* %ptr2 ≡ 2 (mod 64) */
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %p1 = getelementptr inbounds i16, i16* %ptr1, i64 %counter\n"
    "  %val = load i16, i16* %p1\n"
    "  %p2 = getelementptr inbounds i16, i16* %ptr2, i64 %counter\n"
    "  store i16 %val, i16* %p2\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, 10240\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n");

  VPlanPeelingCostModelSimple CM(*DL);
  setupPeelingAnalysis(CM);

  std::unique_ptr<VPlanPeelingVariant> PV2 = VPPA->selectBestPeelingVariant(2);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV2));
  VPlanStaticPeeling &SP2 = cast<VPlanStaticPeeling>(*PV2);
  EXPECT_EQ(SP2.peelCount(), 1); // Make store aligned by 4

  std::unique_ptr<VPlanPeelingVariant> PV4 = VPPA->selectBestPeelingVariant(4);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV4));
  VPlanStaticPeeling &SP4 = cast<VPlanStaticPeeling>(*PV4);
  EXPECT_EQ(SP4.peelCount(), 3); // Make store aligned by 8

  std::unique_ptr<VPlanPeelingVariant> PV8 = VPPA->selectBestPeelingVariant(8);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV8));
  VPlanStaticPeeling &SP8 = cast<VPlanStaticPeeling>(*PV8);
  EXPECT_EQ(SP8.peelCount(), 7); // Make store aligned by 16

  std::unique_ptr<VPlanPeelingVariant> PV16 = VPPA->selectBestPeelingVariant(16);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV16));
  VPlanStaticPeeling &SP16 = cast<VPlanStaticPeeling>(*PV16);
  EXPECT_EQ(SP16.peelCount(), 15); // Make store aligned by 32

  std::unique_ptr<VPlanPeelingVariant> PV32 = VPPA->selectBestPeelingVariant(32);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV32));
  VPlanStaticPeeling &SP32 = cast<VPlanStaticPeeling>(*PV32);
  EXPECT_EQ(SP32.peelCount(), 31); // Make store aligned by 64
}

TEST_F(VPlanPeelingAnalysisTest, StaticPeeling_DoubleLoad) {
  // Two loads (i8 and i32) can be aligned simultaneously. So, a static peeling
  // to align both loads should be preferred to aligning the single i32 store.
  buildVPlanFromString(
    "define void @foo(i8* %buf1, i8* %buf2, i8* %buf3) {\n"
    "entry:\n"
    "  %buf1.asInt = ptrtoint i8* %buf1 to i64\n"
    "  %buf2.asInt = ptrtoint i8* %buf2 to i64\n"
    "  %buf3.asInt = ptrtoint i8* %buf3 to i64\n"
    "  %tmp1 = and i64 %buf1.asInt, -64\n"
    "  %tmp2 = and i64 %buf2.asInt, -64\n"
    "  %tmp3 = and i64 %buf3.asInt, -64\n"
    "  %ptr1.asInt = or i64 %tmp1, 14\n"
    "  %ptr2.asInt = or i64 %tmp2, 14\n"
    "  %ptr3.asInt = or i64 %tmp3, 11\n"
    "  %ptr1 = inttoptr i64 %ptr1.asInt to i8*\n" // %ptr1 ≡ 14 (mod 64)
    "  %ptr2 = inttoptr i64 %ptr2.asInt to i8*\n" // %ptr2 ≡ 14 (mod 64)
    "  %ptr3 = inttoptr i64 %ptr3.asInt to i8*\n" // %ptr3 ≡ 11 (mod 64)
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %p1 = getelementptr inbounds i8, i8* %ptr1, i64 %counter\n"
    "  %p2 = getelementptr inbounds i8, i8* %ptr2, i64 %counter\n"
    "  %v1 = load i8, i8* %p1\n"
    "  %v2 = load i8, i8* %p2\n"
    "  %v3 = add i8 %v1, %v2\n"
    "  %p3 = getelementptr inbounds i8, i8* %ptr3, i64 %counter\n"
    "  store i8 %v3, i8* %p3\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, 10240\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n");

  VPlanPeelingCostModelSimple CM(*DL);
  setupPeelingAnalysis(CM);

  std::unique_ptr<VPlanPeelingVariant> PV2 = VPPA->selectBestPeelingVariant(2);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV2));
  VPlanStaticPeeling &SP2 = cast<VPlanStaticPeeling>(*PV2);
  EXPECT_EQ(SP2.peelCount(), 0); // Make loads aligned by 2.

  std::unique_ptr<VPlanPeelingVariant> PV4 = VPPA->selectBestPeelingVariant(4);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV4));
  VPlanStaticPeeling &SP4 = cast<VPlanStaticPeeling>(*PV4);
  EXPECT_EQ(SP4.peelCount(), 2); // Make loads aligned by 4.

  std::unique_ptr<VPlanPeelingVariant> PV8 = VPPA->selectBestPeelingVariant(8);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV8));
  VPlanStaticPeeling &SP8 = cast<VPlanStaticPeeling>(*PV8);
  EXPECT_EQ(SP8.peelCount(), 2); // Make loads aligned by 8.

  std::unique_ptr<VPlanPeelingVariant> PV16 = VPPA->selectBestPeelingVariant(16);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV16));
  VPlanStaticPeeling &SP16 = cast<VPlanStaticPeeling>(*PV16);
  EXPECT_EQ(SP16.peelCount(), 2); // Make loads aligned by 16.

  std::unique_ptr<VPlanPeelingVariant> PV32 = VPPA->selectBestPeelingVariant(32);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV32));
  VPlanStaticPeeling &SP32 = cast<VPlanStaticPeeling>(*PV32);
  EXPECT_EQ(SP32.peelCount(), 18); // Make loads aligned by 32.

  std::unique_ptr<VPlanPeelingVariant> PV64 = VPPA->selectBestPeelingVariant(64);
  ASSERT_TRUE(isa<VPlanStaticPeeling>(*PV64));
  VPlanStaticPeeling &SP64 = cast<VPlanStaticPeeling>(*PV64);
  EXPECT_EQ(SP64.peelCount(), 50); // Make loads aligned by 64.
}

TEST_F(VPlanPeelingAnalysisTest, StaticPeeling_Cost1) {
  // Several different memrefs can be partially aligned simultaneously (fully or
  // partically) at the right static peeling variant. The test case crafted in
  // such a way that with small VF it is most profitable to align pointers %p1,
  // %p2, %p3, and with large VF it is more profitable to align pointers %p4,
  // %p5, %p6.
  buildVPlanFromString(
    "define void @foo( i8* %buf1, i16* %buf2, i32* %buf3,\n"
    "                 i16* %buf4, i32* %buf5, i64* %buf6) {\n"
    "entry:\n"
    "  %buf1.asInt = ptrtoint  i8* %buf1 to i64\n"
    "  %buf2.asInt = ptrtoint i16* %buf2 to i64\n"
    "  %buf3.asInt = ptrtoint i32* %buf3 to i64\n"
    "  %buf4.asInt = ptrtoint i16* %buf4 to i64\n"
    "  %buf5.asInt = ptrtoint i32* %buf5 to i64\n"
    "  %buf6.asInt = ptrtoint i64* %buf6 to i64\n"
    "  %tmp1 = and i64 %buf1.asInt, -1024\n"
    "  %tmp2 = and i64 %buf2.asInt, -1024\n"
    "  %tmp3 = and i64 %buf3.asInt, -1024\n"
    "  %tmp4 = and i64 %buf4.asInt, -1024\n"
    "  %tmp5 = and i64 %buf5.asInt, -1024\n"
    "  %tmp6 = and i64 %buf6.asInt, -1024\n"
    "  %ptr1.asInt = or i64 %tmp1, 556\n" // = 512 + 44
    "  %ptr2.asInt = or i64 %tmp2, 260\n" // = 256 + 4
    "  %ptr3.asInt = or i64 %tmp3, 664\n" // = 512 + 128 + 24
    "  %ptr4.asInt = or i64 %tmp4, 418\n" // = 256 + 128 + 34
    "  %ptr5.asInt = or i64 %tmp5, 636\n" // = 512 + 64 + 60
    "  %ptr6.asInt = or i64 %tmp6, 840\n" // = 512 + 256 + 72
    "  %ptr1 = inttoptr i64 %ptr1.asInt to i8*\n"
    "  %ptr2 = inttoptr i64 %ptr2.asInt to i16*\n"
    "  %ptr3 = inttoptr i64 %ptr3.asInt to i32*\n"
    "  %ptr4 = inttoptr i64 %ptr4.asInt to i16*\n"
    "  %ptr5 = inttoptr i64 %ptr5.asInt to i32*\n"
    "  %ptr6 = inttoptr i64 %ptr6.asInt to i64*\n"
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %counte8 = trunc i64 %counter to i8\n"
    "  %count16 = trunc i64 %counter to i16\n"
    "  %count32 = trunc i64 %counter to i32\n"
    "  %p1 = getelementptr inbounds  i8,  i8* %ptr1, i64 %counter\n"
    "  %p2 = getelementptr inbounds i16, i16* %ptr2, i64 %counter\n"
    "  %p3 = getelementptr inbounds i32, i32* %ptr3, i64 %counter\n"
    "  %p4 = getelementptr inbounds i16, i16* %ptr4, i64 %counter\n"
    "  %p5 = getelementptr inbounds i32, i32* %ptr5, i64 %counter\n"
    "  %p6 = getelementptr inbounds i64, i64* %ptr6, i64 %counter\n"
    "  store  i8 %counte8,  i8* %p1\n"
    "  store i16 %count16, i16* %p2\n"
    "  store i32 %count32, i32* %p3\n"
    "  store i16 %count16, i16* %p4\n"
    "  store i32 %count32, i32* %p5\n"
    "  store i64 %counter, i64* %p6\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, 10240\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n");

  // Displacements of pointers *in elements* (ignoring insignificant
  // higher-order bits):
  //   %p1 = 12 (mod 16)
  //   %p2 =  2 (mod 16)
  //   %p3 =  6 (mod 16)
  //   %p4 =  1 (mod 16)
  //   %p5 = 15 (mod 16)
  //   %p6 =  9 (mod 16)

  VPlanPeelingCostModelLog CM;
  setupPeelingAnalysis(CM);

  // In this test we check for exact profit values to make sure that the
  // algorithm captures correctly all the interconnections between alignments of
  // various memrefs.

  // VF = 2. Best PeelCount = 0.
  // Alignment:
  //   %p1: 1 -> 2 (cost -= 1)
  //   %p2: 2 -> 4 (cost -= 1)
  //   %p3: 4 -> 8 (cost -= 1)
  //   %p4: 2 -> 2 (cost -= 0)
  //   %p5: 4 -> 4 (cost -= 0)
  //   %p6: 8 -> 8 (cost -= 0)
  std::pair<VPlanStaticPeeling, int> P2 =
      VPPA->selectBestStaticPeelingVariant(2);
  EXPECT_EQ(P2.first.peelCount(), 0);
  EXPECT_EQ(P2.second, 3);

  // VF = 4. Best PeelCount = 2.
  // Alignment:
  //   %p1: 1 ->  2 (cost -= 1)
  //   %p2: 2 ->  8 (cost -= 2)
  //   %p3: 4 -> 16 (cost -= 2)
  //   %p4: 2 ->  2 (cost -= 0)
  //   %p5: 4 ->  4 (cost -= 0)
  //   %p6: 8 ->  8 (cost -= 0)
  std::pair<VPlanStaticPeeling, int> P4 =
      VPPA->selectBestStaticPeelingVariant(4);
  EXPECT_EQ(P4.first.peelCount(), 2);
  EXPECT_EQ(P4.second, 5);

  // VF = 8. Best PeelCount = 7.
  // Alignment:
  //  %p1: 1 ->  1 (cost -= 0)
  //  %p2: 2 ->  2 (cost -= 0)
  //  %p3: 4 ->  4 (cost -= 0)
  //  %p4: 2 -> 16 (cost -= 3)
  //  %p5: 4 ->  8 (cost -= 1)
  //  %p6: 8 -> 64 (cost -= 3)
  std::pair<VPlanStaticPeeling, int> P8 =
      VPPA->selectBestStaticPeelingVariant(8);
  EXPECT_EQ(P8.first.peelCount(), 7);
  EXPECT_EQ(P8.second, 7);

  // VF = 16. Best PeelCount = 7.
  // Alignment:
  //  %p1: 1 ->   1 (cost -= 0)
  //  %p2: 2 ->   2 (cost -= 0)
  //  %p3: 4 ->   4 (cost -= 0)
  //  %p4: 2 ->  16 (cost -= 3)
  //  %p5: 4 ->   8 (cost -= 1)
  //  %p6: 8 -> 128 (cost -= 4)
  std::pair<VPlanStaticPeeling, int> P16 =
      VPPA->selectBestStaticPeelingVariant(16);
  EXPECT_EQ(P16.first.peelCount(), 7);
  EXPECT_EQ(P16.second, 8);
}

TEST_F(VPlanPeelingAnalysisTest, StaticPeeling_Cost2) {
  // This test mostly copies the previous one. The only difference are gaps in
  // known bits for pointers. For example, instead of
  //     %tmp1 = and i64 %buf1.asInt, -1024
  // in this test we have
  //     %tmp1 = and i64 %buf1.asInt, (-1024 + 4)
  // which leaves the 3rd bit in an unknown state. As a result, the maximum
  // static alignment for %p1 is 4.
  buildVPlanFromString(
    "define void @foo( i8* %buf1, i16* %buf2, i32* %buf3,\n"
    "                 i16* %buf4, i32* %buf5, i64* %buf6) {\n"
    "entry:\n"
    "  %buf1.asInt = ptrtoint  i8* %buf1 to i64\n"
    "  %buf2.asInt = ptrtoint i16* %buf2 to i64\n"
    "  %buf3.asInt = ptrtoint i32* %buf3 to i64\n"
    "  %buf4.asInt = ptrtoint i16* %buf4 to i64\n"
    "  %buf5.asInt = ptrtoint i32* %buf5 to i64\n"
    "  %buf6.asInt = ptrtoint i64* %buf6 to i64\n"
    "  %tmp1 = and i64 %buf1.asInt, -1020\n" // = -1024 + 4
    "  %tmp2 = and i64 %buf2.asInt,   -32\n"
    "  %tmp3 = and i64 %buf3.asInt,  -992\n" // = -1024 + 32
    "  %tmp4 = and i64 %buf4.asInt,    -4\n"
    "  %tmp5 = and i64 %buf5.asInt, -1008\n" // = -1024 + 16
    "  %tmp6 = and i64 %buf6.asInt,  -768\n" // = -1024 + 256
    "  %ptr1.asInt = or i64 %tmp1,  1\n"
    "  %ptr2.asInt = or i64 %tmp2,  6\n"
    "  %ptr3.asInt = or i64 %tmp3,  4\n"
    "  %ptr4.asInt = or i64 %tmp4,  2\n"
    "  %ptr5.asInt = or i64 %tmp5,  0\n"
    "  %ptr6.asInt = or i64 %tmp6, 72\n"
    "  %ptr1 = inttoptr i64 %ptr1.asInt to i8*\n"
    "  %ptr2 = inttoptr i64 %ptr2.asInt to i16*\n"
    "  %ptr3 = inttoptr i64 %ptr3.asInt to i32*\n"
    "  %ptr4 = inttoptr i64 %ptr4.asInt to i16*\n"
    "  %ptr5 = inttoptr i64 %ptr5.asInt to i32*\n"
    "  %ptr6 = inttoptr i64 %ptr6.asInt to i64*\n"
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    "  %counte8 = trunc i64 %counter to i8\n"
    "  %count16 = trunc i64 %counter to i16\n"
    "  %count32 = trunc i64 %counter to i32\n"
    "  %p1 = getelementptr inbounds  i8,  i8* %ptr1, i64 %counter\n"
    "  %p2 = getelementptr inbounds i16, i16* %ptr2, i64 %counter\n"
    "  %p3 = getelementptr inbounds i32, i32* %ptr3, i64 %counter\n"
    "  %p4 = getelementptr inbounds i16, i16* %ptr4, i64 %counter\n"
    "  %p5 = getelementptr inbounds i32, i32* %ptr5, i64 %counter\n"
    "  %p6 = getelementptr inbounds i64, i64* %ptr6, i64 %counter\n"
    "  store  i8 %counte8,  i8* %p1\n"
    "  store i16 %count16, i16* %p2\n"
    "  store i32 %count32, i32* %p3\n"
    "  store i16 %count16, i16* %p4\n"
    "  store i32 %count32, i32* %p5\n"
    "  store i64 %counter, i64* %p6\n"
    "  %counter.next = add nsw i64 %counter, 1\n"
    "  %exitcond = icmp sge i64 %counter.next, 10240\n"
    "  br i1 %exitcond, label %exit, label %for.body\n"
    "exit:\n"
    "  ret void\n"
    "}\n");

  // Displacements of pointers *in elements* (modulo base varies due to gaps in
  // known bits):
  //   %p1 = 1 (mod  4)
  //   %p2 = 3 (mod 16)
  //   %p3 = 1 (mod  8)
  //   %p4 = 1 (mod  2)
  //   %p5 = 0 (mod  4)
  //   %p6 = 9 (mod 32)

  VPlanPeelingCostModelLog CM;
  setupPeelingAnalysis(CM);

  // In this test we check for exact profit values to make sure that the
  // algorithm captures correctly all the interconnections between alignments of
  // various memrefs.

  // VF = 2. Best PeelCount = 1.
  // Alignment:
  //  %p1: 1 ->  2 (cost -= 1)
  //  %p2: 2 ->  4 (cost -= 1)
  //  %p3: 4 ->  8 (cost -= 1)
  //  %p4: 2 ->  4 (cost -= 1)
  //  %p5: 4 ->  4 (cost -= 0)
  //  %p6: 8 -> 16 (cost -= 1)
  std::pair<VPlanStaticPeeling, int> P2 =
      VPPA->selectBestStaticPeelingVariant(2);
  EXPECT_EQ(P2.first.peelCount(), 1);
  EXPECT_EQ(P2.second, 5);

  // VF = 4. Best PeelCount = 3.
  // Alignment:
  //  %p1: 1 ->  4 (cost -= 2)
  //  %p2: 2 ->  4 (cost -= 1)
  //  %p3: 4 -> 16 (cost -= 2)
  //  %p4: 2 ->  8 (cost -= 1)
  //  %p5: 4 ->  4 (cost -= 0)
  //  %p6: 8 -> 32 (cost -= 2)
  std::pair<VPlanStaticPeeling, int> P4 =
      VPPA->selectBestStaticPeelingVariant(4);
  EXPECT_EQ(P4.first.peelCount(), 3);
  EXPECT_EQ(P4.second, 8);

  // VF = 8. Best PeelCount = 7.
  // Alignment:
  //  %p1: 1 ->  4 (cost -= 2)
  //  %p2: 2 ->  4 (cost -= 1)
  //  %p3: 4 -> 32 (cost -= 3)
  //  %p4: 2 ->  8 (cost -= 1)
  //  %p5: 4 ->  4 (cost -= 0)
  //  %p6: 8 -> 64 (cost -= 3)
  std::pair<VPlanStaticPeeling, int> P8 =
      VPPA->selectBestStaticPeelingVariant(8);
  EXPECT_EQ(P8.first.peelCount(), 7);
  EXPECT_EQ(P8.second, 10);

  // VF = 16. Best PeelCount = 7.
  // Alignment:
  //  %p1: 1 ->   4 (cost -= 2)
  //  %p2: 2 ->   4 (cost -= 1)
  //  %p3: 4 ->  32 (cost -= 3)
  //  %p4: 2 ->   8 (cost -= 1)
  //  %p5: 4 ->   4 (cost -= 0)
  //  %p6: 8 -> 128 (cost -= 4)
  std::pair<VPlanStaticPeeling, int> P16 =
      VPPA->selectBestStaticPeelingVariant(16);
  EXPECT_EQ(P16.first.peelCount(), 7);
  EXPECT_EQ(P16.second, 11);

  // VF = 32. Best PeelCount = 23.
  // Alignment:
  //  %p1: 1 ->   4 (cost -= 2)
  //  %p2: 2 ->   4 (cost -= 1)
  //  %p3: 4 ->  32 (cost -= 3)
  //  %p4: 2 ->   8 (cost -= 1)
  //  %p5: 4 ->   4 (cost -= 0)
  //  %p6: 8 -> 256 (cost -= 5)
  std::pair<VPlanStaticPeeling, int> P32 =
      VPPA->selectBestStaticPeelingVariant(32);
  EXPECT_EQ(P32.first.peelCount(), 23);
  EXPECT_EQ(P32.second, 12);

  // VF = 64. Best PeelCount = 23.
  // Alignment:
  //  %p1: 1 ->   4 (cost -= 2)
  //  %p2: 2 ->   4 (cost -= 1)
  //  %p3: 4 ->  32 (cost -= 3)
  //  %p4: 2 ->   8 (cost -= 1)
  //  %p5: 4 ->   4 (cost -= 0)
  //  %p6: 8 -> 256 (cost -= 5)
  std::pair<VPlanStaticPeeling, int> P64 =
      VPPA->selectBestStaticPeelingVariant(64);
  EXPECT_EQ(P64.first.peelCount(), 23);
  EXPECT_EQ(P64.second, 12);
}

} // namespace
