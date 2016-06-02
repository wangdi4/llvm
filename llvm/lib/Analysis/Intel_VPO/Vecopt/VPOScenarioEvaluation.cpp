//===-- VPOScenarioEvaluation.cpp -----------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the VPO vectorizer engine that drives the exploration
// of different vectorization alternatives.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOScenarioEvaluation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRVLSClient.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrVisitor.h"

#define DEBUG_TYPE "VPOScenarioEvaluation"

static cl::opt<unsigned> DefaultVF("default-vpo-vf", cl::init(4),
                                   cl::desc("Default vector length"));

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

// Given a region that contains loops/loop-nest, decide which of the loops
// (or combinations of loops) to vectorize, and how.
// Currently, a limited version is implemented:
// Given a region that contains a single innermost loop, consider different
// Vectorization facotrs (VFs) for vectorizing it.
//
// Current initial flow:
//
// BestCost = scalarCost;
// BestCand = nullptr (scalar version);
//
// foreach Aloop in Wrn {                       //currently only one Aloop
//   |[DataDepInfo] = getDataDepInfo(Aloop);
//   |[memrefs] = gatherMemrefsInLoop(Aloop);
//   |VF-Candidates = getPossibleVFs(Aloop);
//   |
//   |foreach VF in VF-Candidates {
//   |  |  VectContextBase VC = setVecContext(Aloop,VF);
//   |  |
//   |  |  VLSInfo = getVLSInfoForCandidate([VC],[DataDepInfo],[memrefs]); (*)
//   |  |  MemAccessInfo = VLSInfo->analyzeVLSMemrefsInLoop();             (*)
//   |  |  Grps = VLSInfo->analyzeVLSGroupsInLoop(MemAccessInfo);          (*)
//   |  |
//   |  |  int Cost = CM.getCost(VC,Grps);
//   |  |  If (Cost < BestCost){
//   |  |    BestCost <-- Cost
//   |  |    BestCand <-- VC;
//   |  |  }
//   |  +}
//   +}
//
// (*) TODO: Compute VLS groups and Memory accesses conservatively in a VF
// agnostic way, and then refine per VF if needed (ideally most processing
// happens at loop level, and is insensitive to the actual VF).
//
// CHECKME: A different flow for different optimization levels?
//
// Implementation stages (for innermost loop vectorization):
// Step0 [current]: Incorporate the VLS Memref analysis and VLS Group analysis
//        into the vectorizer.
//        "evaluate" a single given/default VF candidate (dummy evaluation);
//        No AVR changes.
//        No Compile-time considerations.
//        No changes in the behavior of the vectorizer.
// Step1: Really evaluate a single given/default VF candidate (via TTI costs);
//        Still no AVR changes.
//        Still no Compile-time considerations.
//        VLSGroups ignored in cost analysis.
//        Given the default VF, the Vectorizer may now decide not to vectorize.
// Step2: Evaluate several VF candidates;
//        Still no Compile-time considerations.
//        Still no AVR changes.
// Step3: Refine the skeleton: Optimize (minimize) processing across VF
//        candidates (for Compile time).
// Step4: Take into account VLSGroups in cost evaluation.
// Step5: Prepare mechanism to allow changing the AVR.
// Step6: Incorporate passes that may change the AVR.
// Step7: ...
VPOVecContextBase *VPOScenarioEvaluationBase::getBestCandidate(AVRWrn *AWrn) {

  setAWrn(AWrn);

#if 1
  // FORNOW: An AVRWrn node is expected to have only one AVRLoop child
  AVRLoop *AvrLoop = nullptr;
  for (auto Itr = AWrn->child_begin(), End = AWrn->child_end(); Itr != End;
       ++Itr) {
    if (AVRLoop *TempALoop = dyn_cast<AVRLoop>(Itr)) {
      if (AvrLoop)
        return nullptr;

      AvrLoop = TempALoop;
    }
  }

  // Check that we have an AVRLoop
  if (!AvrLoop)
    return nullptr;
#endif

  // Loop over search space of candidates within AWrn. In the future this will
  // examine all candidate ALoops (and combinations thereof) within the AWrn.
  // FORNOW: we expect to encounter only a single ALoop in AWrn.
  // So FORNOW: A region has a single innermost loop, and therefore a single
  // scenario: Scenario == a single AvrLoop considered for vectoriztion

  VPOVecContextBase *VectCand;

  // TODO: CostOfBestScenario = 0;
  // TODO: foreach Scenario:
  {
    // TODO: ScenarioCost = 0;
    // TODO: foreach AVRLoop candidate in the sceanrio:
    {
      setALoop(AvrLoop);

      // Get the scalar cost for this loop.
      //
      // FORNOW: Calculate cost only for the candidate AvrLoop. We assume that
      // the rest of the code in the region will remain the same between the
      // scalar and vector versions.
      int BestCostForALoop = CM.getCost(ALoop, 1);
      DEBUG(errs() << "Scalar Cost for candidate Loop = " << BestCostForALoop
                   << "\n");

      // VectCand represents the best way to vectorize this loop, including
      // compared to leaving it scalar (in which case VectCand in null);
      VectCand = nullptr;

#ifdef USEALOOP
      VectCand = processLoop(AvrLoop, &BestCostForALoop);
#else
      VectCand = processLoop(AWrn, &BestCostForALoop);
#endif

      // TODO: Cache the best result so far for this loop (best VF, and
      // corresponding best cost), add it to the overall cost for this scenario,
      // and move on to the next Loop in the region for this scenario.
      // ScenarioCost += BestCostForALoop;

    } // End iterating over all loops in the scenario
    // TODO: Keep track of best scenario so far.
    // CostOfBestScenario = min(CostOfBestScenario, ScenarioCost);

  } // End iterating over all scenarios for the region

  // Currently VectCands represents a single loop (the only loop in the
  // scenario). If best thing for this loop is to leave it scalar, VectCand is
  // nullptr. In the future this may be coded directly in the AVR, for all the
  // loops of this region, according to the best scenario.
  return VectCand;
}

// FORNOW: A trivial implementation.
// TODO: Should really be based on the data-types used in the loop and the
// vector regsiters supported by the target.
void VPOScenarioEvaluationBase::findVFCandidates(
    VFsVector &VFCandidates) const {
  int VF = 0;
  int MinVF, MaxVF;
  VF = AWrn->getSimdVectorLength();
  DEBUG(errs() << "VF = " << VF << " DefaultVF = " << DefaultVF << "\n");
  // FORNOW: Assume the default vectorization factor when VF is 0
  // FORNOW: We consider a single candidate.
  // CHECKME: DefaultVF OK for LLVMIR path?
  if (VF == 0) {
    MinVF = DefaultVF;
    MaxVF = DefaultVF;
  } else {
    MinVF = VF;
    MaxVF = VF;
  }

  // FORNOW: We consider a single VF.
  assert(MinVF == MaxVF && "No support for multiple VF candidates.");
  // (FORNOW: iterate once:)
  for (VF = MinVF; VF <= MaxVF; VF *= 2) {
    VFCandidates.push_back(VF);
  }
}

#ifdef USEALOOP
VPOVecContextBase *
VPOScenarioEvaluationBase::processLoop(AVRLoop *ALoop, int *BestCostForALoop) {
#else
VPOVecContextBase *
VPOScenarioEvaluationBase::processLoop(AVRWrn *AvrWrn, int *BestCostForALoop) {
#endif
  DEBUG(errs() << "Process Loop\n");

// Place holder for loop-level, VF-agnostic passes.
//
#ifdef USEALOOP
  setLoop(ALoop);
#else
  setLoop(AvrWrn);
#endif

  // Obtain data-dependence information and gather memory references.
  //
  // CHECKME: Currently the results of these analyses are kept under the covers
  // at the level of the derived implementation. We may prefer passing them
  // here explicitly. However this will require introducing base-level
  // abstractions to be passed around instead of holding on to the derived-level
  // data-structured already at hand. To be revisited.
  getDataDepInfoForLoop();
  gatherMemrefsInLoop();

  // Identify VF candidates
  //
  VFsVector VFCandidates;
  findVFCandidates(VFCandidates);

  // Evaluate each VF candidate
  //
  VPOVecContextBase *BestCand = nullptr;
  for (auto &VF : VFCandidates) {

    DEBUG(errs() << "Evaluate candidate with VF = " << VF << "\n");
    // Currently VecContext is used to hold underlying-IR level information
    // required for some of the analyses in processCandidates (namely, for VLS
    // grouping).
    VPOVecContextBase &VC = setVecContext(VF);
    int Cost = processCandidate(ALoop, VF, VC);

    if (Cost < *BestCostForALoop) {
      *BestCostForALoop = Cost;
      BestCand = &VC;
      DEBUG(errs() << "New Best Candidate Cost = " << *BestCostForALoop
                   << " for VF = " << VF << " \n");
    }
  }

  // Clear data-structures for this loop
  resetLoopInfo(); 

  // If best candidate is the original scalar one, nullptr is returned.
  return BestCand;
}

// TODO: Ideally we have very few VF sensitive adjusments to make.
// ProcessCandidate will be as much as possible just a getCost call.
int VPOScenarioEvaluationBase::processCandidate(AVRLoop *ALoop, unsigned int VF,
                                                VPOVecContextBase &VC) {

  // Place holder for VF-specific passes.
  //

  // Memrefs Analysis:
  //
  // Analyze the access patterns of the Loop memory-references.
  // Returns an interface that can answer questions about memrefs, such as
  // their stride, and distance from one another, relative to a given context
  // (that includes the loop, DataDepsInfo, and the VF; these are
  // provided under the covers when we set up the VLSInfo below).
  //
  // TODO: VLS grouping is currently the only user of the results of
  // Memrefs analysis. Idiom recognition, CostModel, and CodeGen should also
  // use thes results of Memref Analysis.
  //
  // TODO: Memrefs analysis is largely VF-independent. In some cases SLEV
  // analysis can refine memref information for specific VFs. Move this to be
  // processed once per loop and refine per VF only if necessary.
  // In any case, no need to invalidate and recompute all memory-access
  // information from cratch for each candidate.
  //
  // TODO: An AVRLoop may contain very many memrefs, many of which cannot be
  // grouped together. In order to avoid redundant queries, better break the
  // memrefs into subsets of memrefs that can safely be grouped together.
  //
  // FIXME?: Under the covers this is dependent on the memrefs having been
  // gathered (at the underlying IR level); May want to expose this explicitly
  // here. However this will require introducing a base-level abstraction for
  // the memrefs. Revisit this based on how we want to handle memrefs in VPO in
  // general.
  VPOVLSInfoBase *VLSInfo = getVLSInfoForCandidate();
  OVLSMemrefVector VLSMrfs;
  VLSInfo->analyzeVLSMemrefsInLoop(VLSMrfs);

  // VLS-Groups Analysis:
  //
  // Find groups of neighboring memory-references to be used by the cost model.
  //
  // FIXME?: under the convers VLS is dependent on the DDG (and the Vector
  // Context). May want to expose this explicitly here (passing around
  // base-class objects) rather than keeping the derived-class objects
  // internally.
  OVLSGroupVector VLSGrps;
  VLSInfo->analyzeVLSGroupsInLoop(VLSMrfs, VLSGrps);

  // Calculate the cost of the current candidate
  //
  // FORNOW: The VLS Groups are not yet used by the CostModel.
  int Cost = CM.getCost(ALoop, VF);

  // Cleaups.
  // FORNOW: Release the groups and memrefs.  TODO (save compile time): Keep
  // around the Mrfs and Grps of the best candidate. Also can keep the Mrfs
  // and Grps across different Candidates, as they are usually/largely not
  // invalidated by the changing VF.
  for (OVLSMemref *Memref : VLSMrfs) {
    delete Memref;
  }
  VLSMrfs.clear();
  for (OVLSGroup *Grp : VLSGrps) {
    delete Grp;
  }
  VLSGrps.clear();
  delete VLSInfo;

  return Cost;
}

// CostModel Visit routine.
// FORNOW: Only Assigns are supported.
// TODO: Consider reductions, Calls, ...
// TODO: Move to a different module.
void VPOCostGatherer::visit(AVRLoop *Loop) {
  DEBUG(errs() << "visiting loop!\n");
}
// TODO: If this is an inner-loop inside the ALoop being vectorized: multiply
// by the iteration-count
void VPOCostGatherer::postVisit(AVRLoop *Loop) {
  DEBUG(errs() << "visited loop!\n");
}
void VPOCostGatherer::visit(AVRAssign *Assign) {
  DEBUG(errs() << "visiting assign!\n");
}
// TODO: Take blend cost into account if masked.
void VPOCostGatherer::postVisit(AVRAssign *Assign) {
  DEBUG(errs() << "visited assign!\n");
}
void VPOCostGatherer::visit(AVRExpression *Expr) {
  DEBUG(errs() << "visiting expr!\n");
  if (Expr->isLHSExpr()) {
    // We assume that there is no operation here, just a Value
  }
}
void VPOCostGatherer::postVisit(AVRExpression *Expr) {
  if (Expr->isLHSExpr()) {
    // FORNOW: Not contributing any cost
    DEBUG(errs() << "visited expr: LHS: no cost contributed!\n");
  } else {
    DEBUG(errs() << "visited expr: RHS: add cost of operation!\n");
    // FORNOW: Using a dummy cost. TODO: Get it from TTI.
    LoopBodyCost += 10;
  }
}
// TODO: Contrinute the cost of producing this value if not already
// available, according to the SLEV property.
void VPOCostGatherer::visit(AVRValue *AValue) {
  DEBUG(errs() << "visiting value!\n");
}
void VPOCostGatherer::postVisit(AVRValue *AValue) {}

// Dummy implementation; FORNOW just make sure the vector version will be
// selected over the scalar one.
// CHECKME: getCost() operates on a single AvrLoop. May be called several times
// per scenario, if the region contains several candidate AvrLoops.
// TODO: What additional information will the costModel need?:
// - a Map of Memrefs to the VLS Group they belong to (if any).
// - ?
int VPOCostModelBase::getCost(AVRLoop *ALoop, unsigned int VF) const {
  DEBUG(errs() << "getCost: VF = " << VF << "\n");
  int Cost;

  DEBUG(ALoop->dump(PrintDataType));
  VPOCostGatherer CostGatherer;
  AVRVisitor<VPOCostGatherer> AVisitor(CostGatherer);
  AVisitor.visit(ALoop, true, true, true);

  unsigned int LoopBodyCost = CostGatherer.getLoopBodyCost();
  unsigned int OutOfLoopCost = CostGatherer.getOutOfLoopCost();
  DEBUG(errs() << "LoopBodyCost = " << LoopBodyCost
               << " OutOfLoopCost = " << OutOfLoopCost << "\n");
  // FIXME: Take the LoopCount if available.
  // TODO: Consider remainder-loop etc. costs...
  Cost = (LoopBodyCost * 100 / VF) + OutOfLoopCost;
  return Cost;
}
