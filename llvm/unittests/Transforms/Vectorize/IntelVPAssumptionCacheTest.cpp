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
      define void @foo(i32* %p) {
      entry:
        br label %for.body
      for.body:
        %ind = phi i32 [ 0, %entry ], [ %ind.next, %for.body ]
        call void @llvm.assume(i1 true) [ "align"(i32* %p, i32 16) ]
        store i32 1, i32* %p
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
      define void @foo(i32* %p) {
      entry:
        call void @llvm.assume(i1 true) [ "align"(i32* %p, i32 16) ]
        br label %for.body
      for.body:
        %ind = phi i32 [ 0, %entry ], [ %ind.next, %for.body ]
        store i32 1, i32* %p
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

} // namespace
