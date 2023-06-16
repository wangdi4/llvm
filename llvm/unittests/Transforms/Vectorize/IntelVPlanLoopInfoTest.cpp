//===- llvm/unittest/Transforms/Vectorize/IntelVPlanLoopInfoTest.cpp ------===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanTestBase.h"

#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

class VPLoopInfoTest : public VPlanTestBase {
protected:
  std::unique_ptr<VPlanVector> buildVPlanFromString(const char *S) {
    parseModule(S);
    Function *Foo = M->getFunction("foo");
    return buildHCFG(Foo->getEntryBlock().getSingleSuccessor());
  }
};

// Check both possible induction update orders, i.e:
//   <bin-op> <iv>, <step> and
//   <bin-op> <step>, <iv>.
const char *const ModuleStrings[2] = {
    R"(
    define void @foo() {
      entry:
        br label %for.body

      for.body:
        %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
        %iv.next = add nuw nsw i64 %iv, 1
        %exitcond = icmp ugt i64 %iv.next, 1024
        br i1 %exitcond, label %for.exit, label %for.body

      for.exit:
        ret void
    })",
    R"(
    define void @foo() {
      entry:
        br label %for.body

      for.body:
        %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
        %iv.next = add nuw nsw i64 1, %iv
        %exitcond = icmp ugt i64 %iv.next, 1024
        br i1 %exitcond, label %for.exit, label %for.body

      for.exit:
        ret void
    })"};

TEST_F(VPLoopInfoTest, getLatchComparison) {
  for (const char *S : ModuleStrings) {
    const auto Plan = buildVPlanFromString(S);
    const VPLoop *MainLoop = Plan->getMainLoop(true);
    ASSERT_TRUE(MainLoop);

    const VPBasicBlock *LatchBB = MainLoop->getLoopLatch();
    ASSERT_TRUE(LatchBB);

    const VPCmpInst *Cmp = MainLoop->getLatchComparison();
    EXPECT_EQ(LatchBB->getCondBit(), Cmp);
  }
}

TEST_F(VPLoopInfoTest, getInduction) {
  for (const char *S : ModuleStrings) {
    const auto Plan = buildVPlanFromString(S);
    const VPLoop *MainLoop = Plan->getMainLoop(true);
    ASSERT_TRUE(MainLoop);

    const VPInductionInit *InductionInit = MainLoop->getInductionInit();
    ASSERT_TRUE(InductionInit);
    ASSERT_TRUE(isa<VPConstantInt>(InductionInit->getStartVal()));
    ASSERT_EQ(cast<VPConstantInt>(InductionInit->getStartVal())->getZExtValue(),
              (uint64_t)0);

    const VPPHINode *InductionPHI = MainLoop->getInductionPHI();
    ASSERT_TRUE(InductionPHI);
    ASSERT_EQ(InductionPHI->getIncomingValue(MainLoop->getLoopPreheader()),
              InductionInit);
  }
}

TEST_F(VPLoopInfoTest, getInductionWithUnroll) {
  for (const char *S : ModuleStrings) {
    const auto Plan = buildVPlanFromString(S);
    Plan->setVPlanDA(std::make_unique<VPlanDivergenceAnalysis>());
    Plan->computeDA();
    VPlanLoopUnroller Unroller(*Plan.get(), 4);
    Unroller.run();

    const VPLoop *MainLoop = Plan->getMainLoop(true);
    ASSERT_TRUE(MainLoop);

    const VPInductionInit *InductionInit = MainLoop->getInductionInit();
    ASSERT_TRUE(InductionInit);
    ASSERT_TRUE(isa<VPConstantInt>(InductionInit->getStartVal()));
    ASSERT_EQ(cast<VPConstantInt>(InductionInit->getStartVal())->getZExtValue(),
              (uint64_t)0);

    const VPPHINode *InductionPHI = MainLoop->getInductionPHI();
    ASSERT_TRUE(InductionPHI);
    ASSERT_EQ(InductionPHI->getIncomingValue(MainLoop->getLoopPreheader()),
              InductionInit);
  }
}
