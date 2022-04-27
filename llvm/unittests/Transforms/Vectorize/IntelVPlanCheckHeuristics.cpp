#if INTEL_FEATURE_SW_ADVANCED
//===-- llvm/unittest/Transforms/Vectorize/IntelVPlanCheckHeuristics.cpp --===//
//
//   Copyright (C) Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCostModelHeuristics.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCostModel.h"
#include "IntelVPlanTestBase.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "gtest/gtest.h"

using namespace llvm::vpo;
namespace {

static std::unique_ptr<TargetMachine>
createTargetMachine(std::string CPUStr, std::string FeaturesStr) {
  LLVMInitializeX86TargetInfo();
  LLVMInitializeX86Target();
  LLVMInitializeX86TargetMC();

  auto TT(Triple::normalize("x86_64--"));
  std::string Error;

  const Target *TheTarget = TargetRegistry::lookupTarget(TT, Error);
  return std::unique_ptr<TargetMachine>(
      TheTarget->createTargetMachine(TT, CPUStr, FeaturesStr, TargetOptions(),
                                     None, None, CodeGenOpt::Default));
}

class VPlanCheckPsadbwHeuristicTest : public VPlanTestBase {};

TEST_F(VPlanCheckPsadbwHeuristicTest, TestCheckPsadbwHeuristic) {
  const char *ModuleString = R"(
@a = external local_unnamed_addr global [1024 x i8], align 16
@b = external local_unnamed_addr global [1024 x i8], align 16
define dso_local i32 @foo(i32 %t) {
entry:
  br label %for.body

for.cond.cleanup:
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %s.010 = phi i32 [ %t, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i8], [1024 x i8]* @a, i64 0, i64 %indvars.iv
  %val1 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %val1 to i32
  %arrayidx2 = getelementptr inbounds [1024 x i8], [1024 x i8]* @b, i64 0, i64 %indvars.iv
  %val2 = load i8, i8* %arrayidx2, align 1
  %conv3 = zext i8 %val2 to i32
  %sub = sub nsw i32 %conv, %conv3
  %abs = call i32 @llvm.abs.i32(i32 %sub, i1 true)
  %add = add nuw nsw i32 %abs, %s.010
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}
declare i32 @llvm.abs.i32(i32, i1))";

  Module &M = parseModule(ModuleString);

  Function *F = M.getFunction("foo");
  EXPECT_TRUE(F);
  BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
  auto Plan = buildHCFG(LoopHeader);
  VPLoop *OuterMostVPL = *(Plan->getVPLoopInfo())->begin();
  Plan->setVPlanDA(std::make_unique<VPlanDivergenceAnalysis>());
  auto *DA = Plan->getVPlanDA();
  Plan->computeDT();
  Plan->computePDT();
  DA->compute(Plan.get(), OuterMostVPL, Plan->getVPLoopInfo(),
              *Plan->getDT(), *Plan->getPDT(), false /*Not in LCSSA form.*/);

  auto TM = createTargetMachine("skx", "");
  auto TTIPass =
    createTargetTransformInfoWrapperPass(TM->getTargetIRAnalysis());
  const TargetTransformInfo &TTI =
    (static_cast<TargetTransformInfoWrapperPass *>(TTIPass))->getTTI(*F);
  auto DL = std::make_unique<DataLayout>(&M);

  LoopVectorizationPlanner LVP(nullptr /* WRL */,
                               nullptr /* Loop */,
                               nullptr /* LoopInfo */,
                               nullptr /* LibraryInfo */,
                               &TTI, DL.get(),
                               nullptr /* DominatorTree */,
                               nullptr /* legality */,
                               nullptr /* VLSA*/);

  // Base CM is expected to be created.
  TM.get()->Options.IntelAdvancedOptim = false;
  std::unique_ptr<VPlanCostModelInterface> CMBase =
    LVP.createCostModel(Plan.get(), 1);

  // Full CM is expected to be created.
  TM.get()->Options.IntelAdvancedOptim = true;
  std::unique_ptr<VPlanCostModelInterface> CMFull =
    LVP.createCostModel(Plan.get(), 1);

  VPInstructionCost BaseCost = CMBase.get()->getCost();
  VPInstructionCost FullCost = CMFull.get()->getCost();
  // psadbw heuristics should be a part of Full CM and expected to trigger on
  // input in this test. The output Cost for Full CM is expected to be reduced
  // by the heuristic.
  EXPECT_LT(FullCost, BaseCost);
}
} // namespace
#endif // INTEL_FEATURE_SW_ADVANCED
