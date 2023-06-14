//===- IntelVPlanVerifierTest.cpp ----------------------*- C++ -*-===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanTestBase.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

namespace {
// Test base including verifier and builder utilities
class VPlanVerifierTestBase : public VPlanTestBase {
protected:
  std::unique_ptr<VPlanVerifier> Verifier;
  std::unique_ptr<VPlanNonMasked> Plan;
  VPBuilder Builder;
};

// Test for SSA verification
TEST_F(VPlanVerifierTestBase, VerifySSATest) {
  Module &M = parseModule(R"(
  define dso_local void @foo() {
  entry:
    br label %retblock
  retblock:
    ret void
  })");

  Function *Func = M.getFunction("foo");
  ASSERT_TRUE(Func);
  Plan = buildFCFG(Func);
  ASSERT_TRUE(DL);

  Verifier = std::make_unique<VPlanVerifier>(nullptr, *DL);

  auto *Ty = IntegerType::get(*Ctx, 32);
  VPBasicBlock *Entry = &Plan->getEntryBlock();
  VPInstruction *Inst1 =
      new VPInstruction(VPInstruction::Abs, Ty, {new VPValue(Ty)});
  VPInstruction *Inst2 = new VPInstruction(VPInstruction::Abs, Ty, {Inst1});

  // Insert the instructions in the wrong order to break SSA
  Builder.setInsertPoint(&Entry->front());
  Builder.insert(Inst2);
  Builder.insert(Inst1);

  // DT is needed for the SSA check
  Plan->computeDT();
  ASSERT_TRUE(Plan->getDT());

  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get()), "does not dominate all uses");
}

} // namespace
