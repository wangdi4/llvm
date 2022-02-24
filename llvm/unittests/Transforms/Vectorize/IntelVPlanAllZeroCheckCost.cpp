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

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCFGBuilder.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCostModelHeuristics.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCostModel.h"
#include "IntelVPlanTestBase.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "gtest/gtest.h"

namespace llvm {
namespace vpo {
namespace {

static std::unique_ptr<TargetMachine>
createTargetMachine(StringRef CPUStr, StringRef FeaturesStr) {
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

class VPlanAllZeroCheckCostTest : public VPlanTestBase {};

TEST_F(VPlanAllZeroCheckCostTest, AllZeroCheckCost) {

  // Dummy function to build VPlan. We just need an empty function and a single
  // basic block to insert an AllZeroCheck instruction. Had to be done this way
  // because CM needs TTI, which needs a Function pointer.
  const char *ModuleString = R"(
define dso_local void @foo(i32 %t) {
entry:
  ret void
})";

  Module &M = parseModule(ModuleString);

  Function *F = M.getFunction("foo");
  EXPECT_TRUE(F);

  // Construct simple function-based VPlan and run the minimal set of
  // analyses needed.
  auto Externals =
      std::make_unique<VPExternalValues>(&M.getContext(), &M.getDataLayout());
  auto UnlinkedVPInsts = std::make_unique<VPUnlinkedInstructions>();
  auto Plan = std::make_unique<VPlanNonMasked>(*Externals, *UnlinkedVPInsts);
  VPlanFunctionCFGBuilder FunctionCFGBuilder(Plan.get(), *F);
  FunctionCFGBuilder.buildCFG();
  Plan->setName(F->getName());
  Plan->computeDT();
  Plan->computePDT();
  Plan->setVPLoopInfo(std::make_unique<VPLoopInfo>());
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLInfo->analyze(*Plan->getDT());
  // CM runs SVA, which in turn needs DA, which then needs VPLoopInfo.
  Plan->setVPlanDA(std::make_unique<VPlanDivergenceAnalysis>());
  auto *DA = Plan->getVPlanDA();
  DA->compute(Plan.get(), nullptr, Plan->getVPLoopInfo(),
              *Plan->getDT(), *Plan->getPDT(), false /*Not in LCSSA form.*/);
  auto *EntryBB = &Plan->getEntryBlock();
  VPBuilder Builder;
  Builder.setInsertPoint(EntryBB, EntryBB->begin());

  // Create a dummy block-predicate as the operand to AllZeroCheck.
  Type *Ty1 = Type::getInt1Ty(M.getContext());
  auto *Pred = new VPValue(Ty1, nullptr /* no underlying value */);
  auto *AllZeroCheck = cast<VPInstruction>(Builder.createAllZeroCheck(Pred));

  // Create TTI stuff for CM. Cost for AllZeroCheck is the same for all
  // targets, so TM can be for any target. The one selected is arbitrary.
  SmallVector<StringRef> TargetAttrs =
      { "+sse4.2", "+avx", "+avx2", "+avx512f" };
  for (auto TargetAttr : TargetAttrs) {
    auto TM = createTargetMachine("", TargetAttr);
    TM.get()->Options.IntelAdvancedOptim = false;
    auto TTIPass =
      createTargetTransformInfoWrapperPass(TM->getTargetIRAnalysis());
    const TargetTransformInfo &TTI =
      (static_cast<TargetTransformInfoWrapperPass *>(TTIPass))->getTTI(*F);
    LoopVectorizationPlanner LVP(nullptr /* WRL */,
                                 nullptr /* Loop */,
                                 nullptr /* LoopInfo */,
                                 nullptr /* LibraryInfo */,
                                 &TTI, &M.getDataLayout(),
                                 nullptr /* DominatorTree */,
                                 nullptr /* legality */,
                                 nullptr /* VLSA*/);

    SmallVector<unsigned> VFs = { 1, 2, 4, 8, 16, 32 };
    for (auto VF : VFs) {
      // Currently, cost of AllZeroCheck instruction is the same across all targets
      // and VFs. Testing for different ones will catch if something changes in TTI.
      std::unique_ptr<VPlanCostModelInterface> CM =
          LVP.createCostModel(Plan.get(), VF);

      // VPlanCostModelInterface getCost() methods will apply heuristics that need
      // VPLoops. Cast down to VPlanCostModelBase to get cost at TTI level.
      VPInstructionCost AllZeroCheckCost =
          static_cast<VPlanCostModelBase*>(CM.get())->getTTICost(AllZeroCheck);
      EXPECT_EQ(AllZeroCheckCost, 2);
    }
  }
}

} // namespace
} // namespace vpo
} // namespace llvm
