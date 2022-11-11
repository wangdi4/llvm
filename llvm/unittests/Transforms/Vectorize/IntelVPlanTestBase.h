//===- llvm/unittest/Transforms/Vectorize/IntelVPlanTestBase.h ------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file defines an IntelVPlanTestBase class, which provides helpers to
/// parse a LLVM IR string and create VPlans given a loop entry block.
//===----------------------------------------------------------------------===//
#ifndef LLVM_UNITTESTS_TRANSFORMS_VECTORIZE_INTELVPLANTESTBASE_H
#define LLVM_UNITTESTS_TRANSFORMS_VECTORIZE_INTELVPLANTESTBASE_H

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelLoopVectorizationLegality.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelLoopVectorizationPlanner.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPOCodeGen.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCFGBuilder.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/SourceMgr.h"
#include "gtest/gtest.h"

namespace llvm {
namespace vpo {

/// Helper class to create a module from an assembly string and VPlans for a
/// given loop entry block.
class VPlanTestBase : public testing::Test {
protected:
  std::unique_ptr<LLVMContext> Ctx;
  std::unique_ptr<Module> M;
  std::unique_ptr<LoopInfo> LI;
  std::unique_ptr<DominatorTree> DT;
  std::unique_ptr<TargetLibraryInfoImpl> TLIImpl;
  std::unique_ptr<TargetLibraryInfo> TLI;
  std::unique_ptr<TargetTransformInfo> TTI;
  std::unique_ptr<AssumptionCache> AC;
  std::unique_ptr<DataLayout> DL;
  std::unique_ptr<ScalarEvolution> SE;
  std::unique_ptr<PredicatedScalarEvolution> PSE;
  std::unique_ptr<VPOVectorizationLegality> Legal;
  std::unique_ptr<VPExternalValues> Externals;
  std::unique_ptr<VPUnlinkedInstructions> UVPI;
  std::unique_ptr<LoopVectorizationPlanner> LVP;

  VPlanTestBase() : Ctx(new LLVMContext) {}

  Module &parseModule(const char *ModuleString) {
    SMDiagnostic Err;
    M = parseAssemblyString(ModuleString, Err, *Ctx);
    EXPECT_TRUE(M);
    return *M;
  }

  void doAnalysis(Function &F, BasicBlock *LoopHeader) {
    DL.reset(new DataLayout(M.get()));
    DT.reset(new DominatorTree(F));
    LI.reset(new LoopInfo(*DT));
    TLIImpl.reset(new TargetLibraryInfoImpl());
    TLI.reset(new TargetLibraryInfo(*TLIImpl));
    TTI.reset(new TargetTransformInfo(*DL.get()));
    AC.reset(new AssumptionCache(F));
    auto *Loop = LI->getLoopFor(LoopHeader);
    SE.reset(new ScalarEvolution(F, *TLI, *AC, *DT, *LI));
    if (Loop) {
      PSE.reset(new PredicatedScalarEvolution(*SE, *Loop));
      Legal.reset(new VPOVectorizationLegality(Loop, *PSE, &F));
    }
    Externals.reset(new VPExternalValues(Ctx.get(), DL.get()));
    UVPI.reset(new VPUnlinkedInstructions());
    LVP.reset(new LoopVectorizationPlanner(
        nullptr /* no WRLp */, Loop, LI.get(), TLI.get(), TTI.get(), DL.get(),
        DT.get(), Legal.get(), nullptr /* no VLSA */));
  }

  std::unique_ptr<VPlanNonMasked> buildHCFG(BasicBlock *LoopHeader) {
    auto F = LoopHeader->getParent();
    doAnalysis(*F, LoopHeader);

    // Needed for induction importing
    Legal.get()->canVectorize(*DT, nullptr /* use auto induction detection */);

    auto Plan = std::make_unique<VPlanNonMasked>(*Externals, *UVPI);
    VPlanHCFGBuilder HCFGBuilder(LI->getLoopFor(LoopHeader), LI.get(), *DL,
                                 nullptr /*WRLp */, Plan.get(), Legal.get(),
                                 SE.get());
    HCFGBuilder.buildHierarchicalCFG();
    Plan->setVPSE(
        std::make_unique<VPlanScalarEvolutionLLVM>(*SE, *LI->begin(),
                                                  *Plan->getLLVMContext(),
                                                  DL.get()));
    auto &VPSE =
        *static_cast<VPlanScalarEvolutionLLVM *>(Plan.get()->getVPSE());
    Plan->setVPVT(
        std::make_unique<VPlanValueTrackingLLVM>(VPSE, *DL, &*AC, &*DT));

    LVP->runInitialVecSpecificTransforms(Plan.get());
    LVP->createLiveInOutLists(*Plan.get());

    return Plan;
  }

  std::unique_ptr<VPlanNonMasked> buildFCFG(Function *F) {
    doAnalysis(*F, &F->getEntryBlock());
    auto Plan = std::make_unique<VPlanNonMasked>(*Externals, *UVPI);
    VPlanFunctionCFGBuilder FCFGBuilder(Plan.get(), *F);
    FCFGBuilder.buildCFG();
    return Plan;
  }

  std::unique_ptr<VPOCodeGen> getVPOCodeGen(BasicBlock *LoopHeader, unsigned VF,
                                            unsigned UF) {
    OptReportBuilder ORBuilder; // Unsetup ORBuilder
    auto Plan = buildHCFG(LoopHeader);
    auto VPOCG = std::make_unique<VPOCodeGen>(
        LI->getLoopFor(LoopHeader), *Ctx.get(), *PSE.get(), LI.get(), DT.get(),
        TLI.get(), TTI.get(), VF, UF, Legal.get(), nullptr /*VLSA*/, Plan.get(),
        ORBuilder);
    return VPOCG;
  }
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_UNITTESTS_TRANSFORMS_VECTORIZE_INTELVPLANTESTBASE_H
