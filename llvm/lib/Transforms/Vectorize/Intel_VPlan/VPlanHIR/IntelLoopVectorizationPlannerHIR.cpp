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

static cl::opt<bool>
    EnableInMemoryEntities("vplan-enable-inmemory-entities", cl::init(false),
                           cl::Hidden, cl::desc("Enable in memory entities."));

bool LoopVectorizationPlannerHIR::executeBestPlan(VPOCodeGenHIR *CG, unsigned UF) {
  assert(BestVF != 1 && "Non-vectorized loop should be handled elsewhere!");
  VPlanVector *Plan = getVPlanForVF(BestVF);
  assert(Plan && "Unexpected null VPlan");

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

  // Temporary, until CFG merge is implemented. Replace VPLiveInValue-s by
  // original incoming values.
  VPLiveInOutCreator LICreator(*Plan);
  LICreator.restoreLiveIns();

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

std::shared_ptr<VPlanVector> LoopVectorizationPlannerHIR::buildInitialVPlan(
    VPExternalValues &Ext, VPUnlinkedInstructions &UVPI, std::string VPlanName,
    ScalarEvolution *SE) {
  // Create new empty VPlan
  std::shared_ptr<VPlanVector> SharedPlan =
      std::make_shared<VPlanNonMasked>(Ext, UVPI);
  VPlanVector *Plan = SharedPlan.get();
  Plan->setName(VPlanName);

  // Disable SOA-analysis for HIR.
  Plan->disableSOAAnalysis();

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
  Plan->disableActiveLaneInstructions();

  return SharedPlan;
}

bool LoopVectorizationPlannerHIR::canProcessLoopBody(const VPlanVector &Plan,
                                                     const VPLoop &Loop) const {
  // TODO: Privates are not being imported from HIRLegality to VPEntities, hence
  // below checks for in-memory entities will not capture OMP SIMD private
  // construct. Remove this check after importing is implemented.
  if (HIRLegality->getPrivates().size() > 0)
    return false;

  if (EnableInMemoryEntities)
    return true;

  const VPLoopEntityList *LE = Plan.getLoopEntities(&Loop);

  // Entities code and CG need to be uplifted to handle vector type inductions
  // and reductions.
  for (auto *BB : Loop.blocks())
    for (VPInstruction &Inst : *BB)
      if (LE->getReduction(&Inst) || LE->getInduction(&Inst))
        if (isa<VectorType>(Inst.getType())) {
          LLVM_DEBUG(dbgs() << "LVP: Vector type reduction/induction currently"
                            << " not supported.\n"
                            << Inst << "\n");
          return false;
        }

  // HIR-CG is not setup to deal with instructions related to in-memory entities
  // such as VPAllocatePrivate. Check and bail out for any in-memory entities.
  // Walking the VPlan instructions will not work as this check is done before
  // we insert entity related instructions.
  return !LE->hasInMemoryEntity();
}

unsigned LoopVectorizationPlannerHIR::getLoopUnrollFactor(bool *Forced) {
  bool ForcedValue = false;
  unsigned UF = LoopVectorizationPlanner::getLoopUnrollFactor(&ForcedValue);

  if (!ForcedValue) {
    UF = TheLoop->getUnrollPragmaCount();

    if (UF > 0)
      ForcedValue = true;
    else {
      // getUnrollPragmaCount() returns negative value, which means no
      // #pragma unroll N is specified for TheLoop.  Leave ForcedValue
      // to be false then.
      // Capture UF that could be specified internally by other LoopOpt
      // transforms.
      UF = TheLoop->getForcedVectorUnrollFactor();
      if (UF == 0)
        UF = 1;
    }
  }

  if (Forced)
    *Forced = ForcedValue;

  return UF;
}

void LoopVectorizationPlannerHIR::emitVecSpecifics(VPlanVector *Plan) {
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLoop *CandidateLoop = *VPLInfo->begin();

  // The multi-exit loops are processed in a special way
  if (!CandidateLoop->getUniqueExitBlock())
    return;

  auto *PreHeader = CandidateLoop->getLoopPreheader();
  assert(PreHeader && "Single pre-header is expected!");

  VPBuilderHIR Builder;
  Builder.setInsertPointFirstNonPhi(PreHeader);
  VPValue *OrigTC;
  VPInstruction *Cond;
  std::tie(OrigTC, Cond) = getLoopUpperBound(CandidateLoop);
  assert((OrigTC && Cond) && "A normalized loop expected");
  auto *VTC = Builder.create<VPVectorTripCountCalculation>(
      TheLoop, "vector.trip.count", OrigTC);
  Cond->replaceUsesOfWith(OrigTC, VTC);
}
