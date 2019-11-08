//===-- IntelLoopVectorizationPlannerHIR.cpp ------------------------------===//
//
//   Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements LoopVectorizationPlannerHIR.
///
//===----------------------------------------------------------------------===//

#include "IntelLoopVectorizationPlannerHIR.h"
#include "../IntelVPlanCallVecDecisions.h"
#include "../IntelVPlanSSADeconstruction.h"
#include "IntelVPOCodeGenHIR.h"
#include "IntelVPlanBuilderHIR.h"

#define DEBUG_TYPE "LoopVectorizationPlannerHIR"

using namespace llvm;
using namespace llvm::vpo;

cl::opt<uint64_t>
    VPlanDefaultEstTripHIR("vplan-default-est-trip-hir", cl::init(300),
                           cl::desc("Default estimated trip count"));

static cl::opt<bool> ForceLinearizationHIR("vplan-force-linearization-hir",
                                           cl::init(false), cl::Hidden,
                                           cl::desc("Force CFG linearization"));

bool LoopVectorizationPlannerHIR::executeBestPlan(VPOCodeGenHIR *CG, unsigned UF) {
  assert(BestVF != 1 && "Non-vectorized loop should be handled elsewhere!");
  VPlan *Plan = getVPlanForVF(BestVF);
  assert(Plan && "VPlan not found!");

  // Deconstruct SSA for final VPlan that will be lowered to HIR.
  VPlanSSADeconstruction SSADeconstructor(*Plan);
  SSADeconstructor.run();
  VPLAN_DUMP(PrintAfterSSADeconstruction, "SSA deconstruction", Plan);

  // Collect OVLS memrefs and groups for the VF chosen by cost modeling.
  VPlanVLSAnalysis *VLSA = CG->getVLS();
  VLSA->getOVLSMemrefs(Plan, BestVF);

  // Process all loop entities and create refs for them if needed.
  CG->createAndMapLoopEntityRefs(BestVF);
  // Set hoist loop for reductions.
  CG->setRednHoistPtForVectorLoop();

  bool VecLoopsInit = CG->initializeVectorLoop(BestVF, UF);
  if (!VecLoopsInit)
    return false;

  // Run CallVecDecisions analysis for final VPlan which will be used by CG.
  VPlanCallVecDecisions CallVecDecisions(*Plan);
  CallVecDecisions.run(BestVF, TLI, TTI);
  std::string Label("CallVecDecisions analysis for VF=" +
                    std::to_string(BestVF));
  VPLAN_DUMP(PrintAfterCallVecDecisions, Label, Plan);

  // Compute SVA results for final VPlan which will be used by CG.
  Plan->runSVA(BestVF, TLI);
  VPLAN_DUMP(PrintSVAResults, "ScalVec analysis", Plan);

  Plan->executeHIR(CG);
  CG->finalizeVectorLoop();
  return true;
}

std::shared_ptr<VPlan> LoopVectorizationPlannerHIR::buildInitialVPlan(
    unsigned StartRangeVF, unsigned &EndRangeVF, LLVMContext *Context,
    const DataLayout *DL) {
  // Create new empty VPlan
  std::shared_ptr<VPlan> SharedPlan = std::make_shared<VPlan>(Context, DL);
  VPlan *Plan = SharedPlan.get();

  // Build hierarchical CFG
  const DDGraph &DDG = DDA->getGraph(TheLoop);

  VPlanHCFGBuilderHIR HCFGBuilder(WRLp, TheLoop, Plan, HIRLegality, DDG);
  HCFGBuilder.buildHierarchicalCFG();

  // Search loop representation is not yet explicit and search loop idiom
  // recognition is picky. Avoid any changes in predicator behavior for search
  // loops such as avoiding predicate calculations.
  auto *VPLI = Plan->getVPLoopInfo();
  assert(VPLI->size() == 1 && "Expected 1 loop");
  bool SearchLoop = (*VPLI->begin())->getUniqueExitBlock() == nullptr;

  if (ForceLinearizationHIR || SearchLoop)
    Plan->markFullLinearizationForced();

  return SharedPlan;
}

