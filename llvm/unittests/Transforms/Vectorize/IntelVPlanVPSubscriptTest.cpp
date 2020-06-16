//===-- llvm/unittest/Transforms/Vectorize/IntelVPlanVPSubscriptTest.cpp --===//
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
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanBuilder.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVerifier.h"
#include "IntelVPlanTestBase.h"
#include "gtest/gtest.h"

namespace llvm {
namespace vpo {
namespace {
class VPlanVPSubscriptTest : public VPlanTestBase {};

TEST_F(VPlanVPSubscriptTest, TestVPSubscript) {
  const char *ModuleString =
      "define void @f(i32* %A, i64 %N, i64* %Idx) {\n"
      "entry:\n"
      "  %outer.idx = load i64, i64* %Idx\n"
      "  br label %for.body\n"
      "for.body:\n"
      "  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]\n"
      "  %indvars.iv.next = add i64 %indvars.iv, 1\n"
      "  %exitcond = icmp ne i64 %indvars.iv.next, %N\n"
      "  br i1 %exitcond, label %for.body, label %for.end\n"
      "for.end:\n"
      "  ret void\n"
      "}\n";

  Module &M = parseModule(ModuleString);

  Function *F = M.getFunction("f");
  BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
  auto Plan = buildHCFG(LoopHeader);

  VPBuilder Builder;
  VPLoop *OuterMostVPL = *(Plan->getVPLoopInfo())->begin();
  VPBasicBlock *VPLHeader = cast<VPBasicBlock>(OuterMostVPL->getHeader());
  Builder.setInsertPointFirstNonPhi(VPLHeader);

  VPValue *Base = Plan->getVPExternalDef(F->arg_begin());

  Type *Ty32 = Type::getInt32Ty(*Plan->getLLVMContext());
  VPConstant *Lower1 = Plan->getVPConstant(ConstantInt::get(Ty32, 0));
  VPConstant *Stride1 = Plan->getVPConstant(ConstantInt::get(Ty32, 1));
  VPValue *Idx1 = &*VPLHeader->getVPPhis().begin();
  VPConstant *Lower2 = Plan->getVPConstant(ConstantInt::get(Ty32, 2));
  VPConstant *Stride2 = Plan->getVPConstant(ConstantInt::get(Ty32, 3));
  VPValue *Idx2 = Plan->getVPExternalDef(&*F->getEntryBlock().begin());
  Type *BaseTy = Base->getType();

  // Create a new single-dimension VPSubscriptInst.
  Builder.createSubscriptInst(BaseTy, 0, Lower1, Stride1, Base, Idx1);

  // Create a new multi-dimension VPSubscriptInst.
  Builder.createSubscriptInst(BaseTy, 2, {Lower2, Lower1}, {Stride2, Stride1},
                              Base, {Idx2, Idx1});

  // Create a new multi-dimension VPSubscriptInst with struct offsets and
  // dimension type information.
  Builder.createSubscriptInst(
      BaseTy, 2, {Lower2, Lower1}, {Stride2, Stride1}, Base, {Idx2, Idx1},
      {{1 /*Dimension*/, {1, 0}}, {0 /*Dimension*/, {0}}},
      {{1 /*Dimension*/, BaseTy}, {0 /*Dimension*/, BaseTy}});

  // Verify loops after adding subscript VPInstruction.
  VPlanVerifier Verifier(*Plan->getDataLayout());
  Plan.get()->computeDT();
  Verifier.verifyLoops(Plan.get(), *Plan->getDT(), Plan->getVPLoopInfo());
}
} // namespace
} // namespace vpo
} // namespace llvm
