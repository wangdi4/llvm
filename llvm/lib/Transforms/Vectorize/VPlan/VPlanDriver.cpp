//===-- VPlanDriver.cpp -----------------------------------------------------===//
//
//   Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan vectorizer driver pass.
//
//===----------------------------------------------------------------------===//

#include "LoopVectorizationCodeGen.h"
#include "LoopVectorizationPlanner.h"
#include "VPOLoopAdapters.h"
#include "VPlanPredicator.h"
#include "VolcanoOpenCL.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Vectorize.h"

#define DEBUG_TYPE "VPlanDriver"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

// static cl::opt<bool>
//    DisableVPODirectiveCleanup("disable-vpo-directive-cleanup",
//    cl::init(false),
//                               cl::Hidden,
//                               cl::desc("Disable VPO directive cleanup"));

static cl::opt<bool> DisableCodeGen(
    "disable-vplan-codegen", cl::init(false), cl::Hidden,
    cl::desc(
        "Disable VPO codegen, when true, the pass stops at VPlan creation"));

static cl::opt<unsigned>
    VPlanDefaultVF("vplan-default-vf", cl::init(4),
                   cl::desc("Default VPlan vectorization factor"));

static cl::opt<bool> VPlanConstrStressTest(
    "vplan-build-stress-test", cl::init(false),
    cl::desc("Construct VPlan for every loop (stress testing)"));

static cl::opt<bool> VPlanStressOnlyInnermost(
    "vplan-build-stress-only-innermost", cl::init(false),
    cl::desc("When stress testing is enable, construct VPlan only for "
             "innermost loops"));

static cl::opt<bool>
    VPlanForceBuild("vplan-force-build", cl::init(false),
                    cl::desc("Construct VPlan even if loop is not supported "
                             "(only for development)"));
static cl::opt<bool>
    DisableVPlanPredicator("disable-vplan-predicator", cl::init(false),
                           cl::Hidden, cl::desc("Disable VPlan predicator."));

static cl::opt<unsigned> VPlanVectCand(
    "vplan-build-vect-candidates", cl::init(0),
    cl::desc(
        "Construct VPlan for vectorization candidates (CG stress testing)"));

STATISTIC(CandLoopsVectorized, "Number of candidate loops vectorized");

namespace {

// class VPODirectiveCleanup : public FunctionPass {
// public:
//  static char ID; // Pass identification, replacement for typeid
//
//  VPODirectiveCleanup() : FunctionPass(ID) {
//    initializeVPODirectiveCleanupPass(*PassRegistry::getPassRegistry());
//  }
//  bool runOnFunction(Function &Fn) override;
//  //  void getAnalysisUsage(AnalysisUsage &AU) const override;
//};

template <class LoopType> class VPlanDriverBase : public FunctionPass {

protected:
  // Hold information regarding explicit vectorization in LLVM-IR.
  WRegionInfo *WR;

  /// Handle to Target Information
  TargetTransformInfo *TTI;
  TargetLibraryInfo *TLI;

  VPlanDriverBase(char &ID) : FunctionPass(ID){};

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool processFunction(Function &Fn, WRegionCollection::InputIRKind IR);
  // TODO: Try to refactor at least part of it.
  virtual bool processLoop(LoopType *Lp, unsigned VF, Function &Fn,
                           WRNVecLoopNode *WRLp = 0) = 0;

  // VPlan Driver running modes
  bool runStandardMode(Function &Fn, WRegionCollection::InputIRKind IR);
  bool runCGStressTestMode(Function &Fn);
  bool runConstructStressTestMode(Function &Fn);

  // TODO: Move isSupported to Legality class.
  virtual bool isSupported(LoopType *Lp) = 0;
  virtual void collectAllLoops(SmallVectorImpl<LoopType *> &Loops) = 0;
  virtual bool isVPlanCandidate(LoopType *Lp) = 0;
};

class VPlanDriver : public VPlanDriverBase<Loop> {

private:
  LoopInfo *LI;
  ScalarEvolution *SE;
  DominatorTree *DT;
  AssumptionCache *AC;

  bool processLoop(Loop *Lp, unsigned VF, Function &Fn,
                   WRNVecLoopNode *WRLp = 0) override;

  bool isSupported(Loop *Lp) override;
  void collectAllLoops(SmallVectorImpl<Loop *> &Loops) override;
  bool isVPlanCandidate(Loop *Lp) override;

public:
  static char ID; // Pass identification, replacement for typeid

  VPlanDriver() : VPlanDriverBase<Loop>(ID) {
    initializeVPlanDriverPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &Fn) override;
};

class VPlanDriverHIR : public VPlanDriverBase<HLLoop> {

private:
  HIRFramework *HIRF;
  HIRLoopStatistics *HIRLoopStats;
  // HIRDDAnalysis *DDA;
  // HIRVectVLSAnalysis *VLS;

  bool processLoop(HLLoop *Lp, unsigned VF, Function &Fn,
                   WRNVecLoopNode *WRLp = 0) override;

  bool isSupported(HLLoop *Lp) override {
    if (!Lp->isInnermost() || Lp->getNumExits() != 1)
      return false;

    if (HIRLoopStats->getSelfLoopStatistics(Lp).hasSwitches())
      return false;

    return true;
  };

  void collectAllLoops(SmallVectorImpl<HLLoop *> &Loops) override{
    HIRF->getHLNodeUtils().gatherAllLoops(Loops);
  };

  //TODO
  bool isVPlanCandidate(HLLoop *Lp) override { return false; };

public:
  static char ID; // Pass identification, replacement for typeid

  VPlanDriverHIR() : VPlanDriverBase(ID) {
    initializeVPlanDriverHIRPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &Fn) override;
};

} // anonymous namespace

/// Check whether the edge (\p SrcBB, \p DestBB) is a backedge according to LI.
/// I.e., check if it exists a loop that contains SrcBB and where DestBB is the
/// loop header.
static bool isExpectedBackedge(LoopInfo *LI, BasicBlock *SrcBB,
                               BasicBlock *DestBB) {
  for (Loop *Loop = LI->getLoopFor(SrcBB); Loop; Loop = Loop->getParentLoop()) {
    if (Loop->getHeader() == DestBB)
      return true;
  }

  return false;
}

/// Check if the CFG of \p MF is irreducible.
static bool isIrreducibleCFG(Loop *Lp, LoopInfo *LI) {
  LoopBlocksDFS DFS(Lp);
  DFS.perform(LI);
  SmallPtrSet<const BasicBlock *, 32> VisitedBB;

  for (BasicBlock *BB : make_range(DFS.beginRPO(), DFS.endRPO())) {
    VisitedBB.insert(BB);
    for (BasicBlock *SuccBB : BB->getTerminator()->successors()) {
      // SuccBB hasn't been visited yet
      if (!VisitedBB.count(SuccBB))
        continue;
      // We already visited SuccBB, thus BB->SuccBB must be a backedge.
      // Check that the head matches what we have in the loop information.
      // Otherwise, we have an irreducible graph.
      if (!isExpectedBackedge(LI, BB, SuccBB))
        return true;
    }
  }

  return false;
}

template <class LoopType>
void VPlanDriverBase<LoopType>::getAnalysisUsage(AnalysisUsage &AU) const {

  // TODO (CMPLRS-44750): Preserve analyses.
  AU.addRequired<WRegionInfo>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

// Common LLVM-IR/HIR high-level implementation to process a function. It gets
// LLVM-IR-HIR common analyses and choose an execution mode.
template <class LoopType>
bool VPlanDriverBase<LoopType>::processFunction(
    Function &Fn, WRegionCollection::InputIRKind IR) {

  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(Fn);
  TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();

  assert(!(VPlanVectCand && VPlanConstrStressTest) &&
         "Stress testing for VPlan "
         "Construction and CG cannot be "
         "enabled at the same time.");

  bool ModifiedFunc = false;

  // Execution modes
  if (VPlanVectCand) {
    ModifiedFunc = runCGStressTestMode(Fn);
  } else if (!VPlanConstrStressTest) {
    ModifiedFunc = runStandardMode(Fn, IR);
  } else {
    ModifiedFunc = runConstructStressTestMode(Fn);
    assert(!ModifiedFunc &&
           "VPlan Construction stress testing can't modify Function!");
  }

  return ModifiedFunc;
}

/// Standard Mode: standard path for (TODO: automatic and) explicit
/// vectorization.
/// Explicit vectorization: it uses WRegion analysis to collect and vectorize
/// all the WRNVecLoopNode's.
template <class LoopType>
bool VPlanDriverBase<LoopType>::runStandardMode(
    Function &Fn, WRegionCollection::InputIRKind IR) {

  DEBUG(dbgs() << "VD: Stardard Vectorization mode\n");

  WR = &getAnalysis<WRegionInfo>();
  WR->buildWRGraph(IR);
  WRContainerImpl *WRGraph = WR->getWRGraph();

  DEBUG(dbgs() << "WD: WRGraph #nodes= " << WRGraph->size() << "\n");

  bool ModifiedFunc = false;
  for (auto WRNode : *WRGraph) {

    if (WRNVecLoopNode *WRLp = dyn_cast<WRNVecLoopNode>(WRNode)) {
      LoopType *Lp = WRLp->getTheLoop<LoopType>();
      //      simplifyLoop(Lp, DT, LI, SE, AC, false /* PreserveLCSSA */);
      //      formLCSSARecursively(*Lp, *DT, LI, SE);

      assert((VPlanForceBuild || isSupported(Lp)) &&
             "Loop is not supported by VPlan");

      // Get vectorization factor
      unsigned VF = VPlanDefaultVF;
      unsigned Simdlen = WRLp->getSimdlen();
      assert(Simdlen <= 64 && "Wrong Simdlen value");
      VF = Simdlen ? Simdlen : VPlanDefaultVF;

      DEBUG(dbgs() << "VD: Starting VPlan for \n");
      DEBUG(WRNode->dump());

      ModifiedFunc |= processLoop(Lp, VF, Fn, WRLp);
    }
  }

  return ModifiedFunc;
}

/// Construction Stress Testing Mode: builds the H-CFG for any loop in the
/// function.
/// TODO: WIP for HIR.
template <class LoopType>
bool VPlanDriverBase<LoopType>::runConstructStressTestMode(Function &Fn) {

  DEBUG(dbgs() << "VD: VPlan Construction Stress Test mode\n");

  SmallVector<LoopType *, 8> Worklist;
  collectAllLoops(Worklist);

  bool ModifiedFunc = false;
  for (LoopType *Lp : Worklist) {
    IRHIRLoopAdapter<LoopType> LpAdapter(Lp);
    // Process only innermost loops if VPlanStressOnlyInnermost is enabled
    if (!VPlanStressOnlyInnermost || LpAdapter.isInnermost()) {
      // simplifyLoop(Lp, DT, LI, SE, AC, false /* PreserveLCSSA */);
      // formLCSSARecursively(*Lp, *DT, LI, SE);
      if (VPlanForceBuild || isSupported(Lp))
        ModifiedFunc |= processLoop(Lp, VPlanDefaultVF, Fn);
    }
  }

  return ModifiedFunc;
}

/// CG Stress Testing Mode: generates vector code for the first VPlanVectCand
/// number of loops marked as vectorizable using LoopVectorize analysis. When
/// debugging vector CG issues, we can do a binary search to find out the
/// problem loop by setting VPlanVectCand appropriately.
template <class LoopType>
bool VPlanDriverBase<LoopType>::runCGStressTestMode(Function &Fn) {

  DEBUG(dbgs() << "VD: VPlan CG Stress Test mode\n");

  SmallVector<LoopType *, 8> Worklist;
  collectAllLoops(Worklist);

  int ModifiedFunc = false;
  for (LoopType *Lp : Worklist) {
    if (CandLoopsVectorized < VPlanVectCand && isVPlanCandidate(Lp)) {
      ModifiedFunc |= processLoop(Lp, VPlanDefaultVF, Fn);
      CandLoopsVectorized++;
    }
  }

  return ModifiedFunc;
}

INITIALIZE_PASS_BEGIN(VPlanDriver, "VPlanDriver", "VPlan Vectorization Driver",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfo)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(VPlanDriver, "VPlanDriver", "VPlan Vectorization Driver",
                    false, false)

char VPlanDriver::ID = 0;

Pass *llvm::createVPlanDriverPass() { return new VPlanDriver(); }

void VPlanDriver::getAnalysisUsage(AnalysisUsage &AU) const {

  // TODO (CMPLRS-44750): We do not preserve LoopInfo as we remove loops, create
  // new loops. Same holds for Scalar Evolution which needs to be computed for
  // newly created loops.
  VPlanDriverBase::getAnalysisUsage(AU);

  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<AssumptionCacheTracker>();
}

bool VPlanDriver::runOnFunction(Function &Fn) {

  DEBUG(dbgs() << "VPlan LLVM-IR Driver for Function: " << Fn.getName()
               << "\n");

  if (skipFunction(Fn))
    return false;

  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  // TODO: LI shouldn't be necessary as we are building VPLoopInfo. Maybe only
  // for debug/stress testing.
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(Fn);

  bool ModifiedFunc =
      VPlanDriverBase::processFunction(Fn, WRegionCollection::LLVMIR);

  // Remove calls to directive intrinsics since the LLVM back end does not know
  // how to translate them.
  VPOUtils::stripDirectives(Fn);

  return ModifiedFunc;
}

bool VPlanDriver::processLoop(Loop *Lp, unsigned VF, Function &Fn,
                              WRNVecLoopNode *WRLp) {
  PredicatedScalarEvolution PSE(*SE, *Lp);
  VPOVectorizationLegality LVL(Lp, PSE, TLI, TTI, &Fn, LI, DT);

  // Send explicit data from WRLoop to the Legality.
  // The decision about possible loop vectorization is based
  // on this data.
  LoopVectorizationPlanner::EnterExplicitData(WRLp, LVL);
  // The function canVectorize() collects information about induction
  // and reduction variables. It also verifies that the loop vectorization
  // is fully supported.
  if (!LVL.canVectorize()) {
    DEBUG(dbgs() << "VD: Not vectorizing: Cannot prove legality.\n");

    // Only bail out if we are generating code, we want to continue if
    // we are only stress testing VPlan builds below.
    if (!VPlanConstrStressTest && !DisableCodeGen)
      return false;
  }

  LoopVectorizationPlanner LVP(WRLp, Lp, LI, SE, TLI, TTI, DT, &LVL);

  LVP.buildInitialVPlans(VF /*MinVF*/, VF /*MaxVF*/);

  // VPlan Predicator
  if (!DisableVPlanPredicator) {
    IntelVPlan *Plan = LVP.getVPlanForVF(VF);
    VPlanPredicator VPP(Plan);
    VPP.predicate();
  }

  // VPlan construction stress test ends here.
  if (VPlanConstrStressTest)
    return false;

  LVP.setBestPlan(VF, 1);

  DEBUG(std::string PlanName; raw_string_ostream RSO(PlanName);
        RSO << "VD: Initial VPlan for VF=" << VF; RSO.flush();
        VPlan *Plan = LVP.getVPlanForVF(VF); Plan->setName(PlanName);
        dbgs() << *Plan);

  bool ModifiedLoop = false;
  if (!DisableCodeGen) {
    if (VPlanVectCand)
      DEBUG(dbgs() << "VD: VPlan Generating code in function: " << Fn.getName()
                   << "\n");

    VPOCodeGen VCodeGen(Lp, PSE, LI, DT, TLI, TTI, VF, 1, &LVL);
#if INTEL_OPENCL
    VCodeGen.initOpenCLScalarSelectSet(volcanoScalarSelect);
#endif
    LVP.executeBestPlan(VCodeGen);
    ModifiedLoop = true;
  }

  return ModifiedLoop;
}

// Auxiliary function that checks only loop-specific constraints. Generic loop
// nest constraints are in 'isSupported' function.
static bool isSupportedRec(Loop *Lp) {

  if (!Lp->getUniqueExitBlock()) {
    DEBUG(dbgs() << "VD: loop form "
                 << "(" << Lp->getName()
                 << ") is not supported: multiple exit blocks.\n");
    return false;
  }

  for (Loop *SubLoop : Lp->getSubLoops()) {
    if (!isSupportedRec(SubLoop))
      return false;
  }

  return true;
}

// Return true if this loop is supported in VPlan
bool VPlanDriver::isSupported(Loop *Lp) {

  // Check for loop specific constraints
  if (!isSupportedRec(Lp)) {
    DEBUG(dbgs() << "VD: loop nest "
                 << "(" << Lp->getName() << ") is not supported.\n");
    return false;
  }

  // Check generic loop nest constraints
  if (isIrreducibleCFG(Lp, LI)) {
    DEBUG(dbgs() << "VD: loop nest "
                 << "(" << Lp->getName()
                 << ") is not supported: irreducible CFG.\n");
    return false;
  }

  for (BasicBlock *BB : Lp->blocks()) {
    // We don't support switch statements inside loops.
    if (!isa<BranchInst>(BB->getTerminator())) {
      DEBUG(dbgs() << "VD: loop nest contains a switch statement.\n");
      return false;
    }
  }

  DEBUG(dbgs() << "VD: loop nest "
               << "(" << Lp->getName() << ") is supported.\n");

  return true;
}

void VPlanDriver::collectAllLoops(SmallVectorImpl<Loop *> &Loops) {

  std::function<void(Loop *)> collectSubLoops = [&](Loop *Lp) {
    Loops.push_back(Lp);
    for (Loop *InnerLp : Lp->getSubLoops())
      collectSubLoops(InnerLp);
  };

  for (Loop *Lp : *LI)
    collectSubLoops(Lp);
}

bool VPlanDriver::isVPlanCandidate(Loop *Lp) {
  MDNode *LoopID = Lp->getLoopID();

  if (LoopID) {
    for (unsigned i = 1, ie = LoopID->getNumOperands(); i < ie; ++i) {
      MDNode *MD = dyn_cast<MDNode>(LoopID->getOperand(i));
      if (MD) {
        const MDString *S = dyn_cast<MDString>(MD->getOperand(0));
        if (S && S->getString().startswith("vplan.vect.candidate")) {
          return true;
        }
      }
    }
  }

  return false;
}

INITIALIZE_PASS_BEGIN(VPlanDriverHIR, "VPlanDriverHIR",
                      "VPlan Vectorization Driver HIR", false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfo)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
// INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
// INITIALIZE_PASS_DEPENDENCY(HIRParser)
// INITIALIZE_PASS_DEPENDENCY(HIRLocalityAnalysis)
// INITIALIZE_PASS_DEPENDENCY(HIRVectVLSAnalysis)
// INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
// INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysis)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(VPlanDriverHIR, "VPlanDriverHIR",
                    "VPlan Vectorization Driver HIR", false, false)

char VPlanDriverHIR::ID = 0;

Pass *llvm::createVPlanDriverHIRPass() { return new VPlanDriverHIR(); }

void VPlanDriverHIR::getAnalysisUsage(AnalysisUsage &AU) const {

  VPlanDriverBase::getAnalysisUsage(AU);

  // HIR path does not work without setPreservesAll
  AU.setPreservesAll(); // TODO: ?

  //  AU.addRequired<LoopInfoWrapperPass>();
  //  AU.addRequired<ScalarEvolutionWrapperPass>();

  AU.addRequiredTransitive<HIRFramework>();
  AU.addRequiredTransitive<HIRLoopStatistics>();
  //  AU.addRequiredTransitive<HIRParser>();
  //  AU.addRequiredTransitive<HIRLocalityAnalysis>();
  //  AU.addRequiredTransitive<HIRDDAnalysis>();
  //  AU.addRequiredTransitive<HIRSafeReductionAnalysis>();
  //  AU.addRequired<HIRVectVLSAnalysis>();
}

bool VPlanDriverHIR::runOnFunction(Function &Fn) {

  DEBUG(dbgs() << "VPlan HIR Driver for Function: " << Fn.getName() << "\n");
  
  HIRF = &getAnalysis<HIRFramework>();
  HIRLoopStats = &getAnalysis<HIRLoopStatistics>();
  // DDA = &getAnalysis<HIRDDAnalysis>();
  // VLS = &getAnalysis<HIRVectVLSAnalysis>();

  return VPlanDriverBase::processFunction(Fn, WRegionCollection::HIR);
}

bool VPlanDriverHIR::processLoop(HLLoop *Lp, unsigned VF, Function &Fn,
                                 WRNVecLoopNode *WRLp) {

  // TODO: Do we need legality check in HIR?. If we reach this point, the loop
  // either has been marked with SIMD directive by 'HIR Vec Directive Insertion
  // Pass' or we are a stress testing mode.
  // VPOVectorizationLegality LVL(Lp, PSE, TLI, TTI, &Fn, LI, DT);

  // Send explicit data from WRLoop to the Legality.
  // The decision about possible loop vectorization is based
  // on this data.
  // TODO: EnterExplicitData works with Values. This is weird. Please, revisit.
  // LoopVectorizationPlanner::EnterExplicitData(WRLp, LVL);

  //TODO: No Legal for HIR.
  LoopVectorizationPlannerHIR LVP(WRLp, Lp, TLI, TTI, nullptr /*Legal*/);

  LVP.buildInitialVPlans(VF /*MinVF*/, VF /*MaxVF*/);

  // VPlan construction stress test ends here.
  // TODO: Move after predication.
  if (VPlanConstrStressTest)
    return false;

  // VPlan Predicator
  if (!DisableVPlanPredicator) {
    IntelVPlan *Plan = LVP.getVPlanForVF(VF);
    VPlanPredicator VPP(Plan);
    VPP.predicate();
  }

  LVP.setBestPlan(VF, 1);

  // Set the final name for this initial VPlan.
  std::string PlanName;
  raw_string_ostream RSO(PlanName);
  RSO << "Initial VPlan for VF=" << VF;
  RSO.flush();
  VPlan *Plan = LVP.getVPlanForVF(VF);
  Plan->setName(PlanName);

  DEBUG(dbgs() << "VD:\n" << *Plan);

  bool ModifiedLoop = false;
  if (!DisableCodeGen) {
    //    if (VPlanVectCand)
    //      errs() << "VD: VPlan Generating code in function: " << Fn.getName()
    //             << "\n";
    //
    //    VPOCodeGen VCodeGen(Lp, PSE, LI, DT, TLI, TTI, VF, 1, &LVL);
    //#if INTEL_OPENCL
    //    VCodeGen.initOpenCLScalarSelectSet(volcanoScalarSelect);
    //#endif
    //    LVP.executeBestPlan(VCodeGen);
    //    ModifiedLoop = true;
  }

  return ModifiedLoop;
}

// INITIALIZE_PASS_BEGIN(VPODirectiveCleanup, "VPODirectiveCleanup",
//                      "VPO Directive Cleanup", false, false)
// INITIALIZE_PASS_END(VPODirectiveCleanup, "VPODirectiveCleanup",
//                    "VPO Directive Cleanup", false, false)
//
// char VPODirectiveCleanup::ID = 0;
//
////FunctionPass *llvm::createVPODirectiveCleanupPass() {
//  return new VPODirectiveCleanup();
//}
//
// void VPODirectiveCleanup::getAnalysisUsage(AnalysisUsage &AU) const {
//}
//
// bool VPODirectiveCleanup::runOnFunction(Function &Fn) {
//
//  // Skip if disabled
//  if (DisableVPODirectiveCleanup) {
//    return false;
//  }
//
//  // Remove calls to directive intrinsics since the LLVM back end does not
//  know
//  // how to translate them.
//  if (!VPOUtils::stripDirectives(Fn)) {
//    // If nothing happens, simply return.
//    return false;
//  }
//
//  // Set up a function pass manager so that we can run some cleanup transforms
//  // on the LLVM IR after code gen.
//  Module *Md = Fn.getParent();
//  legacy::FunctionPassManager FPM(Md);
//
//  // It is possible that stripDirectives call
//  // eliminates all instructions in a basic block except for the branch
//  // instruction. Use CFG simplify to eliminate them.
//  FPM.add(createCFGSimplificationPass());
//  FPM.run(Fn);
//
//  return true;
//}

