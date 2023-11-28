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

namespace llvm {
namespace vpo {
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

  template <class T> void setMergeId(T *VPVal, unsigned Id) {
    VPVal->MergeId = Id;
  }

  template <class T> Value *getUnderlying(T *VPVal) {
    return VPVal->getUnderlyingValue();
  }

  template <class T> void setUnderlying(T *VPVal, Value *NewUnderlying) {
    VPVal->setUnderlyingValue(*NewUnderlying);
  }

  SmallVectorImpl<std::unique_ptr<VPLiveInValue>> *getLiveIns() {
    return &(Plan->LiveInValues);
  }

  SmallVectorImpl<std::unique_ptr<VPLiveOutValue>> *getLiveOuts() {
    return &(Plan->LiveOutValues);
  }

  SmallVectorImpl<std::unique_ptr<VPExternalUse>> *getExtUses() {
    return &(Plan->getExternals().VPExternalUses);
  }

  SmallVectorImpl<std::unique_ptr<VPExternalDef>> *getExtDefs() {
    return &(Plan->getExternals().VPExternalDefs);
  }

  std::unique_ptr<VPlanVerifier> Verifier;
  std::unique_ptr<VPlanNonMasked> Plan;
  VPBuilder Builder;
};
} // namespace vpo
} // namespace llvm

namespace {
const char *SmallLoopIR = R"(
    define void @foo(i64 %N, ptr %Idx) {
      entry:
        %outer.idx = load i64, ptr %Idx
        br label %for.body
      for.body:
        %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
        %indvars.iv.next = add i64 %indvars.iv, 1
        %exitcond = icmp ne i64 %indvars.iv.next, %N
        br i1 %exitcond, label %for.body, label %for.end
      for.end:
        ret void
      }
    )";

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
  Module &M = parseModule(SmallLoopIR);

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

TEST_F(VPlanVerifierTestBase, EntryExitPredicates) {
  Module &M = parseModule(R"(
    define void @foo(i64 %N, ptr %Idx) {
      entry:
        %outer.idx = load i64, ptr %Idx
        br label %for.body.a
      for.body.a:
        %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body.b ]
        %indvars.iv.next = add i64 %indvars.iv, 1
        br label %for.body.b
      for.body.b:
        %exitcond = icmp ne i64 %indvars.iv.next, %N
        br i1 %exitcond, label %for.body.a, label %for.end
      for.end:
        br label %exit
      exit:
        ret void
      }
    )");

  Function *Func = M.getFunction("foo");
  BasicBlock *LoopHeader = Func->getEntryBlock().getSingleSuccessor();
  getHCFGPlan(LoopHeader);

  VPLoop *Lp = *Plan->getVPLoopInfo()->begin();
  assert(Lp);

  VPBasicBlock *Header = Lp->getHeader();
  VPBasicBlock *Exit = Lp->getExitingBlock();
  assert(Header && Exit && Header != Exit);

  Verifier->verifyVPlan(Plan.get(), VPlanVerifier::SkipDA);

  auto *I32Ty = IntegerType::get(*Ctx, 32);

  // Set exit predicate, but not entry
  Builder.setInsertPointFirstNonPhi(Exit);
  Exit->setBlockPredicate(Builder.createPred(new VPValue(I32Ty)));
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get(), VPlanVerifier::SkipDA),
               "both be predicated or neither");

  // Remove exit predicate and re-verify to ensure it passes
  Exit->setBlockPredicate(nullptr);
  Verifier->verifyVPlan(Plan.get(), VPlanVerifier::SkipDA);

  // Set header predicate without an exit predicate
  Builder.setInsertPointFirstNonPhi(Header);
  Header->setBlockPredicate(Builder.createPred(new VPValue(I32Ty)));
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get(), VPlanVerifier::SkipDA),
               "both be predicated or neither");

  // Set exit predicate to different value than entry predicate
  Builder.setInsertPointFirstNonPhi(Exit);
  Exit->setBlockPredicate(Builder.createPred(new VPValue(I32Ty)));
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get(), VPlanVerifier::SkipDA),
               "do not have the same predicate");

  // Set matching predicate for entry and exit
  Exit->setBlockPredicate(Header->getBlockPredicate());
  Verifier->verifyVPlan(Plan.get(), VPlanVerifier::SkipDA);
}

TEST_F(VPlanVerifierTestBase, ExtUseTest) {
  Module &M = parseModule(SmallLoopIR);

  Function *Func = M.getFunction("foo");
  BasicBlock *LoopHeader = Func->getEntryBlock().getSingleSuccessor();
  getHCFGPlan(LoopHeader);
  Verifier->verifyVPlan(Plan.get());

  VPExternalValues *Exts = &Plan->getExternals();
  VPExternalUse *First = *Exts->externalUses().begin();
  Type *Ty = First->getType();
  auto *NewExtUse = Exts->createVPExternalUseNoIR(Ty);

  // NewExtUse will be in index 1 -> mismatch between merge id
  setMergeId<VPExternalUse>(NewExtUse, 0);
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get()),
               "index and merge ID do not match");

  // Set back to index 1, should pass
  setMergeId<VPExternalUse>(NewExtUse, 1);
  Verifier->verifyVPlan(Plan.get());

  // Change the first ext use's underlying to a value, exactly which doesn't
  // matter as long as it's a phi
  setUnderlying<VPExternalUse>(First, &*LoopHeader->begin());
  Verifier->verifyVPlan(Plan.get());

  // Set the same value as the underlying for the new ext use, should fail
  // with duplicate/repeated external use
  setUnderlying<VPExternalUse>(NewExtUse, &*LoopHeader->begin());
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get()), "Repeated VPExternalUse");
}

TEST_F(VPlanVerifierTestBase, LiveInOutTest) {
  Module &M = parseModule(SmallLoopIR);

  Function *Func = M.getFunction("foo");
  BasicBlock *LoopHeader = Func->getEntryBlock().getSingleSuccessor();
  getHCFGPlan(LoopHeader);
  Verifier->verifyVPlan(Plan.get());

  auto &LiveIns = *getLiveIns();
  assert(LiveIns.size() == 1);
  Type *Ty = LiveIns[0]->getType();

  // Sizes of external use and livein list don't match
  LiveIns.emplace_back(new VPLiveInValue(1, Ty));
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get()), "exceeds");

  VPExternalValues *Exts = &Plan->getExternals();
  // Create ext use to pad out the list so the sizes match
  Exts->createVPExternalUseNoIR(Ty);

  LiveIns.pop_back();
  LiveIns.emplace_back(new VPLiveInValue(3, Ty));
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get()),
               "Live in index and merge ID do not match");

  // LiveIns may have null values but LiveOuts should not
  getLiveOuts()->emplace_back(nullptr);
  EXPECT_DEATH(Verifier->verifyVPlan(Plan.get()), "Null live out");
}
} // namespace
