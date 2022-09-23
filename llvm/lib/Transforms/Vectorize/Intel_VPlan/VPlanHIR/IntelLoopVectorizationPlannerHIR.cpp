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
#include "../IntelVPlanCFGMerger.h"
#include "../IntelVPlanCallVecDecisions.h"
#include "../IntelVPlanSSADeconstruction.h"
#include "../IntelVPlanVLSTransform.h"
#include "IntelVPOCodeGenHIR.h"
#include "IntelVPlanBuilderHIR.h"
#include "Intel_VPlan/IntelVPTransformLibraryCalls.h"

#define DEBUG_TYPE "LoopVectorizationPlannerHIR"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool> ForceLinearizationHIR("vplan-force-linearization-hir",
                                           cl::init(false), cl::Hidden,
                                           cl::desc("Force CFG linearization"));

static cl::opt<bool>
    EnableInMemoryEntities("vplan-enable-inmemory-entities", cl::init(false),
                           cl::Hidden, cl::desc("Enable in memory entities."));

bool LoopVectorizationPlannerHIR::executeBestPlan(VPOCodeGenHIR *CG,
                                                  unsigned UF) {
  unsigned BestVF = getBestVF();
  assert(BestVF != 1 && "Non-vectorized loop should be handled elsewhere!");
  VPlanVector *Plan = getBestVPlan();
  assert(Plan && "Unexpected null VPlan");

  // Deconstruct SSA for final VPlan that will be lowered to HIR.
  VPlanSSADeconstruction SSADeconstructor(*Plan);
  SSADeconstructor.run();
  VPLAN_DUMP(PrintAfterSSADeconstruction, "SSA deconstruction", Plan);

  // Collect OVLS memrefs and groups for the VF chosen by cost modeling.
  VPlanVLSAnalysis *VLSA = CG->getVLS();
  VLSA->getOVLSMemrefs(Plan, BestVF);

  applyVLSTransform(*Plan, *VLSA, BestVF);

  // Process all loop entities and collect instructions participating in them if
  // needed.
  CG->collectLoopEntityInsts();
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
  std::string Label;
  CallVecDecisions.runForMergedCFG(TLI, TTI);
  Label = "CallVecDecisions analysis for merged CFG";
  VPLAN_DUMP(PrintAfterCallVecDecisions, Label, Plan);

  // Transform lib calls (like 'sincos') that need additional processing.
  VPTransformLibraryCalls VPTransLibCalls(*Plan, *TLI);
  VPTransLibCalls.transform();

  // Compute SVA results for final VPlan which will be used by CG.
  Plan->runSVA(BestVF);
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
  Plan->setOrigLoopNestingLevel(TheLoop->getNestingLevel());

  const Function* F = &TheLoop->getHLNodeUtils().getFunction();
  Plan->setPrintingEnabled(llvm::isFunctionInPrintList(F->getName()));

  // Enable SOA-analysis if enabled in the header.
  if (EnableSOAAnalysisHIR)
    Plan->enableSOAAnalysis();

  // Build hierarchical CFG
  const DDGraph &DDG = DDA->getGraph(TheLoop);

  VPlanHCFGBuilderHIR HCFGBuilder(WRLp, TheLoop, Plan, HIRLegality, DDG);
  if (!HCFGBuilder.buildHierarchicalCFG())
    return nullptr;

  // Search loop representation is not yet explicit and search loop idiom
  // recognition is picky. Avoid any changes in predicator behavior for search
  // loops such as avoiding predicate calculations.
  auto *VPLI = Plan->getVPLoopInfo();
  assert(VPLI->size() == 1 && "Expected 1 loop");
  if ((*VPLI->begin())->getUniqueExitBlock() == nullptr)
    setIsSearchLoop();

  if (ForceLinearizationHIR || isSearchLoop())
    Plan->markFullLinearizationForced();

  return SharedPlan;
}

bool LoopVectorizationPlannerHIR::canProcessLoopBody(const VPlanVector &Plan,
                                                     const VPLoop &Loop) const {
  if (EnableInMemoryEntities)
    return true;

  const VPLoopEntityList *LE = Plan.getLoopEntities(&Loop);
  if (!LE)
    return false;

  for (auto *BB : Loop.blocks())
    for (VPInstruction &Inst : *BB) {
      // Entities code and CG need to be uplifted to handle vector type
      // inductions and reductions.
      if (LE->getReduction(&Inst) || LE->getInduction(&Inst)) {
        if (isa<VectorType>(Inst.getType())) {
          LLVM_DEBUG(dbgs() << "LVP: Vector type reduction/induction currently"
                            << " not supported.\n"
                            << Inst << "\n");
          return false;
        }
      } else if (Loop.isLiveOut(&Inst) && !LE->getPrivate(&Inst) &&
                 !LE->getCompressExpandIdiom(&Inst)) {
        // Some liveouts are left unrecognized due to unvectorizable use-def
        // chains.
        LLVM_DEBUG(dbgs() << "LVP: Unrecognized liveout found.");
        return false;
      }
      // Specialization for handling sincos functions in CG is done based on
      // underlying HIR. Privatization for such sincos cannot be implemented
      // until it is uplifted to be fully VPValue-based.
      if (auto *VPCall = dyn_cast<VPCallInstruction>(&Inst)) {
        auto *CalledF = VPCall->getCalledFunction();
        if (!CalledF)
          continue;

        auto *UnderlyingCall = VPCall->getUnderlyingCallInst();
        if (UnderlyingCall &&
            vpo::VPOAnalysisUtils::isBeginDirective(UnderlyingCall)) {
          LLVM_DEBUG(dbgs() << "LVP: unsupported nested begin directive. "
                            << *UnderlyingCall << "\n");
          return false;
        }
      }
    }

  // Check whether all reductions are supported
  for (auto Red : LE->vpreductions())
    if (Red->getRecurrenceKind() == RecurKind::SelectICmp ||
        Red->getRecurrenceKind() == RecurKind::SelectFCmp)
      return false;

  for (auto Ind : LE->vpinductions()) {
    if (Ind->getKind() == InductionDescriptor::IK_PtrInduction) {
      LLVM_DEBUG(dbgs() << "LVP: Pointer induction currently not supported.\n");
      return false;
    }
  }

  // All checks passed.
  return true;
}

void LoopVectorizationPlannerHIR::createLiveInOutLists(VPlanVector &Plan) {
  VPLiveInOutCreator LICreator(Plan);
  LICreator.createInOutValues(TheLoop);
}

unsigned LoopVectorizationPlannerHIR::getLoopUnrollFactor(bool *Forced) {
  bool ForcedValue = false;
  unsigned UF = LoopVectorizationPlanner::getLoopUnrollFactor(&ForcedValue);

  if (!ForcedValue) {
    UF = TheLoop->getUnrollPragmaCount();

    if (UF > 0)
      ForcedValue = true;
    else {
      UF = TheLoop->getInterleavePragmaCount();

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
  }

  if (Forced)
    *Forced = ForcedValue;

  return UF;
}

LoopVectorizationPlanner::PlannerType
LoopVectorizationPlannerHIR::getPlannerType() const {
  if (LightWeightMode)
    return PlannerType::LightWeight;
  return LoopVectorizationPlanner::getPlannerType();
}

bool LoopVectorizationPlannerHIR::unroll(VPlanVector &Plan) {

  bool Result = LoopVectorizationPlanner::unroll(Plan);

  if (Result) {
    TheLoop->removeLoopMetadata("llvm.loop.unroll.count");
    TheLoop->addLoopMetadata(MDNode::get(
        *Plan.getLLVMContext(),
        MDString::get(*Plan.getLLVMContext(), "llvm.loop.unroll.disable")));
  }

  return Result;
}

void LoopVectorizationPlannerHIR::emitPeelRemainderVPLoops(unsigned VF,
                                                           unsigned UF) {
  if (isSearchLoop())
    return;
  assert(getBestVF() > 1 && "Unexpected VF");
  VPlanVector *Plan = getBestVPlan();
  assert(Plan && "No best VPlan found.");

  VPlanCFGMerger CFGMerger(*Plan, VF, UF);

  // Set the flag to indicate if we are dealing with a simple main vector
  // scalar remainder scenario with known trip counts.
  CFGMerger.setIsSimpleConstTCScenario(VecScenario.isSimpleConstTCScenario());

  // Run CFGMerger.
  CFGMerger.createMergedCFG(VecScenario, MergerVPlans, TheLoop);
}

void LoopVectorizationPlannerHIR::createMergerVPlans(
    VPAnalysesFactoryBase &VPAF) {
  // Search loop representation is not yet explicit and search loop idiom
  // recognition is picky. Avoid emitting merged CFG for search loops.
  if (isSearchLoop())
    return;

  assert(MergerVPlans.empty() && "Non-empty list of VPlans");
  assert(getBestVF() > 1 && "Unexpected VF");

  VPlanVector *Plan = getBestVPlan();
  assert(Plan && "No best VPlan found.");

  VPlanCFGMerger::createPlans(*this, VecScenario, MergerVPlans, TheLoop,
                              *Plan, VPAF);
}

void LoopVectorizationPlannerHIR::emitVecSpecifics(VPlanVector *Plan) {
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLoop *CandidateLoop = *VPLInfo->begin();

  // TODO. Modify the loop latch condition classification (in
  // hasLoopNormalizedInduction) to accept HIR-normalized loops. They use 'le'
  // condition which leads to execution of OrigUB + 1 iterations.
  //
  bool ExactUB = false;
  bool HasNormalizedInd = hasLoopNormalizedInduction(CandidateLoop, ExactUB);
  assert(ExactUB && "Exact UB expected for decomposed HLLoops.");
  CandidateLoop->setHasNormalizedInductionFlag(HasNormalizedInd, ExactUB);

  // The multi-exit loops are processed in a special way
  if (!CandidateLoop->getUniqueExitBlock())
    return;

  // TODO: All loops in HIR path are expected to be normalized. Move this
  // assertion to after the call hasLoopNormalizedInduction() when loop entity
  // instructions are supported for search loops.
  assert(HasNormalizedInd && "Expected normalized loop");

  auto *PreHeader = CandidateLoop->getLoopPreheader();
  assert(PreHeader && "Single pre-header is expected!");
  VPBuilderHIR Builder;
  Builder.setInsertPointFirstNonPhi(PreHeader);

  VPValue *OrigTC;
  VPCmpInst *Cond;
  std::tie(OrigTC, Cond) = CandidateLoop->getLoopUpperBound();

  if (auto *Instr = dyn_cast<VPInstruction>(OrigTC)) {
    auto Parent = Instr->getParent();
    assert(
        (Parent == PreHeader || Plan->getDT()->dominates(Parent, PreHeader)) &&
        "Unexpected loop upper bound placement");
    Builder.setInsertPoint(Parent, std::next(Instr->getIterator(), 1));
  }

  auto *VTC = Builder.createHIR<VPVectorTripCountCalculation>(
      TheLoop, "vector.trip.count", OrigTC);
  Cond->replaceUsesOfWith(OrigTC, VTC);
}
