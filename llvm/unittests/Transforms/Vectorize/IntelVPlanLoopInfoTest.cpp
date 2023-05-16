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
  std::unique_ptr<VPlanVector> Plan;
  Function *Foo;

  VPLoopInfoTest() {
    parseModule(R"(
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
      }
    )");
    Foo = M->getFunction("foo");
    Plan = buildHCFG(Foo->getEntryBlock().getSingleSuccessor());
  }
};

TEST_F(VPLoopInfoTest, getLatchComparison) {
  const VPLoop *MainLoop = Plan->getMainLoop(true);
  ASSERT_TRUE(MainLoop);

  const VPBasicBlock *LatchBB = MainLoop->getLoopLatch();
  ASSERT_TRUE(LatchBB);

  const VPCmpInst *Cmp = MainLoop->getLatchComparison();
  EXPECT_EQ(LatchBB->getCondBit(), Cmp);
}

TEST_F(VPLoopInfoTest, getInduction) {
  const VPLoop *MainLoop = Plan->getMainLoop(true);
  ASSERT_TRUE(MainLoop);

  const VPInductionInit *Ind = MainLoop->getInduction();
  ASSERT_TRUE(Ind);
  ASSERT_TRUE(isa<VPConstantInt>(Ind->getStartVal()));
  ASSERT_EQ(cast<VPConstantInt>(Ind->getStartVal())->getZExtValue(),
            (uint64_t)0);
}
