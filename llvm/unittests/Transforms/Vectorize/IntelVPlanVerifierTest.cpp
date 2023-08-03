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
  void getHCFGPlan(BasicBlock *BB) {
    Plan = buildHCFG(BB);
    Verifier = std::make_unique<VPlanVerifier>(nullptr, *DL);
    VPLoopInfo *VPLI = Plan->getVPLoopInfo();
    assert(VPLI);

    Plan->computeDT();
    Plan->computePDT();
    Plan->setVPlanDA(std::make_unique<VPlanDivergenceAnalysis>());
    Plan->getVPlanDA()->compute(
        Plan.get(), *VPLI->begin(), Plan->getVPLoopInfo(), nullptr /*VPVT*/,
        *Plan->getDT(), *Plan->getPDT(), false /*Not in LCSSA form.*/);
  }

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

// Test for DT check failure discovered with Fortran dope vector and private
// final cond instructions in VPlan driver
TEST_F(VPlanVerifierTestBase, DTUpdateTest) {
  Module &M = parseModule(R"(
  define dso_local void @foo() {
  entry:
    %a = add i64 0, 1
    br label %retblock
  retblock:
    ret void
  })");

  Function *Func = M.getFunction("foo");
  ASSERT_TRUE(Func);
  Plan = buildFCFG(Func);
  ASSERT_TRUE(DL);
  Verifier = std::make_unique<VPlanVerifier>(nullptr, *DL);

  // Need VPLI and Dom Trees for this test, DA unnecessary
  Plan->setVPLoopInfo(std::make_unique<VPLoopInfo>());
  Plan->computeDT();
  Plan->computePDT();

  VPLoopInfo *VPLI = Plan->getVPLoopInfo();
  VPDominatorTree *DT = Plan->getDT();
  VPPostDominatorTree *PDT = Plan->getPDT();
  ASSERT_TRUE(VPLI && DT && PDT);

  VPBasicBlock *Entry = &Plan->getEntryBlock();
  auto InstIt = Entry->front().getIterator();
  VPBasicBlock *VPBB1 = VPBlockUtils::splitBlock(Entry, InstIt, VPLI, DT, PDT);
  VPBasicBlock *VPBB2 =
      VPBlockUtils::splitBlock(VPBB1, std::next(InstIt), VPLI, DT, PDT);

  VPValue *Cond = new VPValue(IntegerType::get(*Ctx, 32));
  Entry->setTerminator(VPBB1, VPBB2, Cond);

  // verifyVPlan runs the dom tree verify functions
  // Run those functions directly to make sure
  ASSERT_FALSE(DT->verify());
  ASSERT_FALSE(PDT->verify());
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get(), VPlanVerifier::SkipLoopInfo),
               "Dominator Tree failed to verify");

  VPBlockUtils::updateDomTrees(VPBB1, VPBB2, Entry);

  // Should not fail after updating the dom trees
  ASSERT_TRUE(DT->verify());
  ASSERT_TRUE(PDT->verify());
  Verifier->verifyVPlan(Plan.get(), VPlanVerifier::SkipLoopInfo);
}

// Test for DA failures being caught by the verifier
// Tests include missing shapes and differing shapes between
// stored shape and the result of a fresh calculation
TEST_F(VPlanVerifierTestBase, DATests) {
  Module &M = parseModule(R"(
    define void @foo(i64 %N, i64* %Idx) {
      entry:
        %outer.idx = load i64, i64* %Idx
        br label %for.body
      for.body:
        %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
        %indvars.iv.next = add i64 %indvars.iv, 1
        %exitcond = icmp ne i64 %indvars.iv.next, %N
        br i1 %exitcond, label %for.body, label %for.end
      for.end:
        ret void
      }
    )");

  Function *Func = M.getFunction("foo");
  BasicBlock *LoopHeader = Func->getEntryBlock().getSingleSuccessor();
  getHCFGPlan(LoopHeader);
  auto *DA = Plan->getVPlanDA();

  // Create a new instruction without a DA shape and insert
  Type *Ty32 = Type::getInt32Ty(*Plan->getLLVMContext());
  Builder.setInsertPointFirstNonPhi(
      (*Plan->getVPLoopInfo()->begin())->getHeader());
  VPInstruction *CmpInst = Builder.createCmpInst(
      CmpInst::ICMP_NE, Plan->getVPConstant(ConstantInt::get(Ty32, 0)),
      Plan->getVPConstant(ConstantInt::get(Ty32, 1)), "UniformCmpInst");

  // Run verifier after inserting an instruction with undefined shape
  ASSERT_TRUE(DA->getVectorShape(*CmpInst).isUndefined());
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get()), "Shape has not been defined");

  // Give instruction the correct shape. Verifier should pass.
  DA->updateDivergence(*CmpInst);
  ASSERT_TRUE(DA->getVectorShape(*CmpInst).isUniform());
  Verifier->verifyVPlan(Plan.get(), VPlanVerifier::CheckDAShapes);

  // Set incorrect shape; expected shape is uniform. Should assert.
  DA->markDivergent(*CmpInst);
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get(), VPlanVerifier::CheckDAShapes),
               "Recalculated shape for DA is different");
}
} // namespace
