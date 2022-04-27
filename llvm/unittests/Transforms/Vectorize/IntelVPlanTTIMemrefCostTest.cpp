//===- IntelVPlanAlignmentAnalysisTest.cpp ----------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCostModel.h"
#include "IntelVPlanTestBase.h"
#include "llvm/ADT/Triple.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "gtest/gtest.h"

using namespace llvm;
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

class VPlanTTIMemrefCostTest : public vpo::VPlanTestBase {
protected:
  std::unique_ptr<TargetMachine> TM;
  std::unique_ptr<VPlanNonMasked> Plan;
  std::unique_ptr<VPlanCostModelInterface> CM;
  SmallVector<const VPLoadStoreInst *, 16> VPInsts;

protected:
  void SetUp() override {
    Module &M = parseModule(R"(
        define void @foo(i32* %p32, i1* %p1, i8** %pp8, i8* %p8,
                         double %dval, double* %pd) {
        entry:
          %lane = call i32 @llvm.vplan.laneid()
          %gep0 = getelementptr inbounds i32, i32* %p32, i32 %lane
          %val0 = load i32, i32* %gep0, align 1
          %gep1 = getelementptr inbounds i1, i1* %p1, i32 %lane
          %val1 = load i1,  i1*  %gep1, align 1
          %gep2 = getelementptr inbounds i8*, i8** %pp8, i32 %lane
          %val2 = load i8*, i8** %gep2, align 1
          %gep3 = getelementptr inbounds i8, i8* %p8, i32 %lane
          %val3 = load i8,  i8*  %gep3, align 1
          store i32 %val0,  i32* %gep0, align 1
          %gep4 = getelementptr inbounds double, double* %pd, i32 %lane
          store double %dval, double* %gep4, align 1
          ret void
        }

        declare i32 @llvm.vplan.laneid())"
    );

    Function *Func = M.getFunction("foo");
    EXPECT_TRUE(Func);
    Plan = buildFCFG(Func);

    Plan->setVPLoopInfo(std::make_unique<VPLoopInfo>());
    Plan->setVPlanDA(std::make_unique<VPlanDivergenceAnalysis>());
    auto *DA = Plan->getVPlanDA();
    auto *VPLInfo = Plan->getVPLoopInfo();

    Plan->computeDT();
    Plan->computePDT();
    VPLInfo->analyze(*Plan->getDT());
    DA->compute(Plan.get(), nullptr /* CandidateLoop */, Plan->getVPLoopInfo(),
                *Plan->getDT(), *Plan->getPDT(), false /*Not in LCSSA form.*/);

    TM = createTargetMachine("skx", "");
    auto TTIPass =
      createTargetTransformInfoWrapperPass(TM->getTargetIRAnalysis());
    const TargetTransformInfo &TTI =
      (static_cast<TargetTransformInfoWrapperPass *>(TTIPass))->getTTI(*Func);

    LoopVectorizationPlanner LVP(nullptr /* WRL */,
                                 nullptr /* Loop */,
                                 nullptr /* LoopInfo */,
                                 nullptr /* LibraryInfo */,
                                 &TTI, DL.get(),
                                 nullptr /* DominatorTree */,
                                 nullptr /* legality */,
                                 nullptr /* VLSA*/);

    CM = LVP.createCostModel(Plan.get(), 1);
    for (const VPInstruction &VPInst : Plan->front())
      if (auto VPLoadStore = dyn_cast<VPLoadStoreInst>(&VPInst))
        VPInsts.push_back(VPLoadStore);
  }
};

TEST_F(VPlanTTIMemrefCostTest, ScalarLoadCost) {
  // Expect regular cost for scalars.
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[0], Align(4), 1 /* VF */), 1);
}

// Check irregular types.

TEST_F(VPlanTTIMemrefCostTest, Check_1xi1) {
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[1], Align(4), 1 /* VF */), 1);
}

TEST_F(VPlanTTIMemrefCostTest, Check1xi8Ptr) {
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[2], Align(4), 1 /* VF */), 1);
}

TEST_F(VPlanTTIMemrefCostTest, Check1xi8) {
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[3], Align(4), 1 /* VF */), 1);
}

TEST_F(VPlanTTIMemrefCostTest, Check7xi8) {
  // Read 7 x i8 (7 bytes).
  // Reading 7 bytes is less than Alignment, so it is aligned.
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[3], Align(8), 7 /* VF */), 1);
}

TEST_F(VPlanTTIMemrefCostTest, AlignedStore) {
  // Such store is aligned.
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[4], Align(16), 4 /* VF */), 1);
}

TEST_F(VPlanTTIMemrefCostTest, UnalignedStoreLowProbility) {
  // Within the cache line it has low probability to be unaligned
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[4], Align(4), 4 /* VF */), 1.1875);
}

TEST_F(VPlanTTIMemrefCostTest, UnalignedStoreHighProbability) {
  // Within 64 byte cache line it has high probability to be unaligned.
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[4], Align(8), 16 /* VF */), 1.875);
}

TEST_F(VPlanTTIMemrefCostTest, Check2PartLoadCost) {
  // Expect double penalty for very wide store.
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[4], Align(4), 32 /* VF */), 3.875);
}

// Common cases

TEST_F(VPlanTTIMemrefCostTest, Align64Size64) {
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[4], Align(64), 16 /* VF */), 1);
}

TEST_F(VPlanTTIMemrefCostTest, Align32Size32) {
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[4], Align(32), 8 /* VF */), 1);
}

TEST_F(VPlanTTIMemrefCostTest, Align64Size32) {
  EXPECT_EQ(CM->getLoadStoreCost(VPInsts[4], Align(64), 8 /* VF */), 1);
}

// Consistency

TEST_F(VPlanTTIMemrefCostTest, 2xVectorGives2xCost) {
  // Check that cost of <8 x double> is twice of <16 x double>
  const auto Ty8xdoubleCost =
    CM->getLoadStoreCost(VPInsts[5], Align(8), 8  /* VF */);
  const auto Ty16xdoubleCost =
    CM->getLoadStoreCost(VPInsts[5], Align(8), 16  /* VF */);
  EXPECT_TRUE(Ty8xdoubleCost * 2 == Ty16xdoubleCost);
}

} // namespace
