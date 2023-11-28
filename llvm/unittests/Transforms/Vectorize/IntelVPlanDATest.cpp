//===-- llvm/unittest/Transforms/Vectorize/IntelVPlanDATest.cpp------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===---------------------------------------------------------------------===//

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanBuilder.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanTestBase.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "gtest/gtest.h"
#include <array>
#include <memory>

namespace llvm {
namespace vpo {
namespace {
class VPlanVPDATest : public VPlanTestBase {};

TEST_F(VPlanVPDATest, TestVPlanDAPartialUpdate) {
  std::array<StringRef, 1> ModuleStrings = {{
      R"(
    define void @da_test(ptr %A, i64 %N, ptr %Idx) {
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
    )"}};

  for (const auto ModuleString : ModuleStrings) {
    Module &M = parseModule(ModuleString.data());
    Function *F = M.getFunction("da_test");
    BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
    auto Plan = buildHCFG(LoopHeader);
    VPLoop *OuterMostVPL = *(Plan->getVPLoopInfo())->begin();
    Plan->setVPlanDA(std::make_unique<VPlanDivergenceAnalysis>());
    auto *DA = Plan->getVPlanDA();
    Plan->computeDT();
    Plan->computePDT();
    DA->compute(Plan.get(), OuterMostVPL, Plan->getVPLoopInfo(),
                nullptr /*VPVT*/, *Plan->getDT(), *Plan->getPDT(),
                false /*Not in LCSSA form.*/);

    VPBuilder Builder;
    VPBasicBlock *VPLHeader = cast<VPBasicBlock>(OuterMostVPL->getHeader());
    Builder.setInsertPointFirstNonPhi(VPLHeader);

    // Construct the two operands for the first CMP instruction.
    Type *Ty32 = Type::getInt32Ty(*Plan->getLLVMContext());
    VPConstant *Op1 = Plan->getVPConstant(ConstantInt::get(Ty32, 0));
    VPConstant *Op2 = Plan->getVPConstant(ConstantInt::get(Ty32, 1));

    Builder.setInsertPointFirstNonPhi(VPLHeader);
    // Create a Cmp instruction made up of two operands Op1 and Op2.
    VPInstruction *CmpInst1 =
        Builder.createCmpInst(CmpInst::ICMP_NE, Op1, Op2, "UniformCmpInst");

    // Cast the result to i32 type.
    VPValue *CastInst = cast<VPValue>(
        Builder.createNaryOp(Instruction::BitCast, Ty32, {CmpInst1}));

    VPConstant *Op3 = Plan->getVPConstant(ConstantInt::get(Ty32, -10));
    // Create a Cmp instruction made up of CmpInst1 and Op3.
    VPInstruction *CmpInst2 = Builder.createCmpInst(CmpInst::ICMP_NE, CastInst,
                                                    Op3, "DependentCmpInst");

    // Updating the divergence of the first instruction, CmpInst1, should
    // trigger the update of the second(dependent) instruction CmpInst2. Just
    // compare that the shapes of the two instructions are as expected.
    DA->updateDivergence(*CmpInst1);
    auto Shape1 = DA->getVectorShape(*CmpInst1);
    auto Shape2 = DA->getVectorShape(*CmpInst2);
    EXPECT_EQ(Shape1.isUniform(), true);
    EXPECT_EQ(Shape2.isUniform(), true);
  }
}

} // namespace
} // namespace vpo
} // namespace llvm
