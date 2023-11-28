//===- IntelVPAssumptionCacheTest.cpp ---------------------------*- C++ -*-===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPAssumptionCache.h"
#include "IntelVPlanTestBase.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

namespace {

class VPAssumptionCacheTest : public VPlanTestBase {
protected:
  Function *FuncFoo;
  std::unique_ptr<VPlanVector> Plan;

  void buildVPlanFromString(const char *ModuleString) {
    Module &M = parseModule(ModuleString);
    FuncFoo = M.getFunction("foo");
    BasicBlock *LoopHeader = FuncFoo->getEntryBlock().getSingleSuccessor();
    Plan = buildHCFG(LoopHeader);
  }

  template <typename InstTy> const InstTy *getFirstVPInstruction() const {
    for (VPInstruction &I : vpinstructions(Plan.get()))
      if (auto *Inst = dyn_cast<InstTy>(&I))
        return Inst;
    return nullptr;
  }

  const VPCallInstruction *getAssumeCallInsideLoop() const {
    const auto *Call = getFirstVPInstruction<VPCallInstruction>();
    assert(Call->getCalledFunction()->getIntrinsicID() == Intrinsic::assume);
    return Call;
  }

  const VPLoadStoreInst *getStoreInst() const {
    const auto *Store = getFirstVPInstruction<VPLoadStoreInst>();
    assert(Store->getOpcode() == Instruction::Store);
    return Store;
  }
};

TEST_F(VPAssumptionCacheTest, AssumeInsidePlan) {
  buildVPlanFromString(R"(
      declare void @llvm.assume(i1)
      define void @foo(ptr %p) {
      entry:
        br label %for.body
      for.body:
        %ind = phi i32 [ 0, %entry ], [ %ind.next, %for.body ]
        call void @llvm.assume(i1 true) [ "align"(ptr %p, i32 16) ]
        store i32 1, ptr %p
        %ind.next = add nuw nsw i32 %ind, 1
        %cond = icmp eq i32 %ind.next, 256
        br i1 %cond, label %for.body, label %exit
      exit:
        ret void
      })");

  const VPCallInstruction *AssumeCall = getAssumeCallInsideLoop();

  const auto Assumes = Plan->getVPAC()->assumptions();
  ASSERT_EQ(Assumes.size(), (size_t)1);
  EXPECT_EQ(cast<VPCallInstruction>(Assumes.front()), AssumeCall);

  const VPValue *ParamVal = getStoreInst()->getOperand(1);

  const auto AffectingAssumes = Plan->getVPAC()->assumptionsFor(ParamVal);
  ASSERT_EQ(AffectingAssumes.size(), (size_t)1);
  EXPECT_EQ(cast<VPCallInstruction>(AffectingAssumes.front()), AssumeCall);
}

TEST_F(VPAssumptionCacheTest, AssumeOutsidePlan) {
  buildVPlanFromString(R"(
      declare void @llvm.assume(i1)
      define void @foo(ptr %p) {
      entry:
        call void @llvm.assume(i1 true) [ "align"(ptr %p, i32 16) ]
        br label %for.body
      for.body:
        %ind = phi i32 [ 0, %entry ], [ %ind.next, %for.body ]
        store i32 1, ptr %p
        %ind.next = add nuw nsw i32 %ind, 1
        %cond = icmp eq i32 %ind.next, 256
        br i1 %cond, label %for.body, label %exit
      exit:
        ret void
      })");

  const auto Assumes = Plan->getVPAC()->assumptions();
  ASSERT_EQ(Assumes.size(), (size_t)1);
  EXPECT_TRUE(isa<AssumeInst>(Assumes.front()));

  const auto *ParamVal = getStoreInst()->getOperand(1);

  const auto AffectingAssumes = Plan->getVPAC()->assumptionsFor(ParamVal);
  ASSERT_EQ(AffectingAssumes.size(), (size_t)1);
  EXPECT_TRUE(isa<AssumeInst>(AffectingAssumes.front()));

  EXPECT_EQ(cast<AssumeInst>(AffectingAssumes.front()),
            cast<AssumeInst>(Assumes.front()));
}

TEST_F(VPAssumptionCacheTest, AssumeOp) {
  buildVPlanFromString(R"(
      declare void @llvm.assume(i1)
      define void @foo(ptr %p) {
      entry:
        br label %for.body
      for.body:
        %ind = phi i32 [ 0, %entry ], [ %ind.next, %for.body ]
        %gep = getelementptr i32, ptr %p, i32 %ind
        %elem = load i32, ptr %p, align 4
        %eq = icmp eq i32 %elem, 0
        call void @llvm.assume(i1 %eq)
        %ind.next = add nuw nsw i32 %ind, 1
        %cond = icmp eq i32 %ind.next, 256
        br i1 %cond, label %for.body, label %exit
      exit:
        ret void
      })");

  const auto Assumes = Plan->getVPAC()->assumptions();
  ASSERT_EQ(Assumes.size(), (size_t)1);

  const auto *AssumeCall = dyn_cast<VPCallInstruction>(Assumes[0]);
  EXPECT_TRUE(AssumeCall);
  EXPECT_EQ(Assumes[0].Index, VPAssumptionCache::ExprResultIdx);

  const auto *AssumeOp = dyn_cast<VPCmpInst>(AssumeCall->getArgOperand(0));
  EXPECT_TRUE(AssumeOp);

  const auto AssumesForCmp = Plan->getVPAC()->assumptionsFor(AssumeOp);
  ASSERT_EQ(AssumesForCmp.size(), (size_t)1);
  EXPECT_EQ(dyn_cast<VPCallInstruction>(AssumesForCmp[0]), AssumeCall);
  EXPECT_EQ(AssumesForCmp[0].Index, VPAssumptionCache::ExprResultIdx);
}

} // namespace
