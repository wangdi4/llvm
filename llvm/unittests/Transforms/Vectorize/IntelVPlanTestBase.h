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
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
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
  std::unique_ptr<AssumptionCache> AC;
  std::unique_ptr<DataLayout> DL;
  std::unique_ptr<ScalarEvolution> SE;
  std::unique_ptr<PredicatedScalarEvolution> PSE;
  std::unique_ptr<VPOVectorizationLegality> Legal;
  std::unique_ptr<VPExternalValues> Externals;

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
    AC.reset(new AssumptionCache(F));
    auto Loop = LI->getLoopFor(LoopHeader);
    SE.reset(new ScalarEvolution(F, *TLI, *AC, *DT, *LI));
    PSE.reset(new PredicatedScalarEvolution(*SE, *Loop));
    Legal.reset(new VPOVectorizationLegality(Loop, *PSE, &F));
    Externals.reset(new VPExternalValues(Ctx.get(), DL.get()));
  }

  std::unique_ptr<VPlan> buildHCFG(BasicBlock *LoopHeader) {
    auto F = LoopHeader->getParent();
    doAnalysis(*F, LoopHeader);

    auto Plan = std::make_unique<VPlan>(*Externals);
    VPlanHCFGBuilder HCFGBuilder(LI->getLoopFor(LoopHeader), LI.get(), *DL,
                                 nullptr /*WRLp */, Plan.get(), Legal.get());
    HCFGBuilder.buildHierarchicalCFG();
    return Plan;
  }
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_UNITTESTS_TRANSFORMS_VECTORIZE_INTELVPLANTESTBASE_H
