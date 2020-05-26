//===-- llvm/unittest/Transforms/Vectorize/IntelVPlanCheckNodeParent.cpp --===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.h"
#include "IntelVPlanTestBase.h"
#include "gtest/gtest.h"

namespace llvm {
namespace vpo {
namespace {
class VPlanCheckNodeParentTest : public VPlanTestBase {};

TEST_F(VPlanCheckNodeParentTest, TestVPSubscript) {
  const char *ModuleString =
      "define void @f() {\n"
      "entry:\n"
      "  br label %for.body\n"
      "for.body:\n"
      "  %phi = phi i32 [ 0, %entry ], [ %induction, %latch ]\n"
      "  %induction = add nsw i32 %phi, 1\n"
      "  br label %latch\n"
      "latch:\n"
      "  %bottom_test = icmp eq i32 %induction, 128\n"
      "  br i1 %bottom_test, label %for.body, label %end\n"
      "end:\n"
      "  ret void\n"
      "}\n";

  Module &M = parseModule(ModuleString);

  Function *F = M.getFunction("f");
  BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
  auto Plan = buildHCFG(LoopHeader);

  for (auto &VPBB : *Plan) {
    EXPECT_EQ(VPBB.getParent(), Plan.get());
    for (auto &VPInst : *&VPBB)
      EXPECT_EQ(VPInst.getParent(), &VPBB);
  }
}
} // namespace
} // namespace vpo
} // namespace llvm
