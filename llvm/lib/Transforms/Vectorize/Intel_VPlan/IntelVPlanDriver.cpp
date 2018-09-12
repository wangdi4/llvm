//===-- IntelVPlanDriver.cpp ----------------------------------------------===//
//
//   Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
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

#include "IntelLoopVectorizationCodeGen.h"
#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPOLoopAdapters.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanPredicator.h"
#include "IntelVolcanoOpenCL.h"
#include "VPlanHIR/IntelLoopVectorizationPlannerHIR.h"
#include "VPlanHIR/IntelVPOCodeGenHIR.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Vectorize.h"

#define DEBUG_TYPE "VPlanDriver"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

namespace llvm {
class DemandedBits;
class OptimizationRemarkEmitter;
} // namespace llvm

// static cl::opt<bool>
//    DisableVPODirectiveCleanup("disable-vpo-directive-cleanup",
//    cl::init(false),
//                               cl::Hidden,
//                               cl::desc("Disable VPO directive cleanup"));

static cl::opt<bool> DisableCodeGen(
    "disable-vplan-codegen", cl::init(false), cl::Hidden,
    cl::desc(
        "Disable VPO codegen, when true, the pass stops at VPlan creation"));

// TODO: Unify with LoopVectorize's vplan-build-stress-test?
static cl::opt<bool> VPlanConstrStressTest(
    "vpo-vplan-build-stress-test", cl::init(false),
    cl::desc("Construct VPlan for every loop (stress testing)"));

static cl::opt<bool> VPlanStressOnlyInnermost(
    "vplan-build-stress-only-innermost", cl::init(false),
    cl::desc("When stress testing is enable, construct VPlan only for "
             "innermost loops"));

static cl::opt<bool>
    VPlanForceBuild("vplan-force-build", cl::init(false),
                    cl::desc("Construct VPlan even if loop is not supported "
                             "(only for development)"));

#if INTEL_CUSTOMIZATION
static cl::opt<bool>
    VPlanPrintInit("vplan-print-after-init", cl::init(false),
                   cl::desc("Print plain dump after initial VPlan generated"));
#endif

static cl::opt<unsigned> VPlanVectCand(
    "vplan-build-vect-candidates", cl::init(0),
    cl::desc(
        "Construct VPlan for vectorization candidates (CG stress testing)"));

static cl::list<unsigned> VPlanCostModelPrintAnalysisForVF(
    "vplan-cost-model-print-analysis-for-vf", cl::Hidden, cl::CommaSeparated,
    cl::ZeroOrMore,
    cl::desc("Print detailed VPlan Cost Model Analysis report for the given "
             "VF. For testing/debug purposes only."));

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
  const DataLayout *DL;

  VPlanDriverBase(char &ID) : FunctionPass(ID){};

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool processFunction(Function &Fn, WRegionCollection::InputIRKind IR);
  // TODO: Try to refactor at least part of it.
  virtual bool processLoop(LoopType *Lp, Function &Fn,
                           WRNVecLoopNode *WRLp = 0) = 0;

  // VPlan Driver running modes
  bool runStandardMode(Function &Fn, WRegionCollection::InputIRKind IR);
  bool runCGStressTestMode(Function &Fn);
  bool runConstructStressTestMode(Function &Fn);

  // TODO: Move isSupported to Legality class.
  virtual bool isSupported(LoopType *Lp) = 0;
  virtual void collectAllLoops(SmallVectorImpl<LoopType *> &Loops) = 0;

  // Return true if the given loop is a candidate for VPlan vectorization.
  // Currently this function is used in the LLVM IR path to generate VPlan
  // candidates using LoopVectorizationLegality for stress testing the LLVM IR
  // VPlan implementation.
  virtual bool isVPlanCandidate(Function &Fn, LoopType *Lp) = 0;
};

class VPlanDriver : public VPlanDriverBase<Loop> {

private:
  LoopInfo *LI;
  ScalarEvolution *SE;
  DominatorTree *DT;
  AssumptionCache *AC;
  AliasAnalysis *AA;
  DemandedBits *DB;
  LoopAccessLegacyAnalysis *LAA;
  OptimizationRemarkEmitter *ORE;

  bool processLoop(Loop *Lp, Function &Fn, WRNVecLoopNode *WRLp = 0) override;

  bool isSupported(Loop *Lp) override;
  void collectAllLoops(SmallVectorImpl<Loop *> &Loops) override;
  bool isVPlanCandidate(Function &Fn, Loop *Lp) override;

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
  HIRDDAnalysis *DDA;
  // HIRVectVLSAnalysis *VLS;
  LoopOptReportBuilder LORBuilder;

  bool processLoop(HLLoop *Lp, Function &Fn, WRNVecLoopNode *WRLp = 0) override;

  bool isSupported(HLLoop *Lp) override {
    if (!Lp->isInnermost())
      return false;

    if (HIRLoopStats->getSelfLoopStatistics(Lp).hasSwitches())
      return false;

    return true;
  };

  void collectAllLoops(SmallVectorImpl<HLLoop *> &Loops) override {
    HIRF->getHLNodeUtils().gatherAllLoops(Loops);
  };

  bool isVPlanCandidate(Function &Fn, HLLoop *Lp) override {
    // This function is only used in the LLVM-IR path to generate VPlan
    // candidates.
    return false;
  };

public:
  static char ID; // Pass identification, replacement for typeid

  VPlanDriverHIR() : VPlanDriverBase(ID) {
    initializeVPlanDriverHIRPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// \brief Overrides FunctionPass's printer pass to return one which prints
  /// HIR instead of LLVM IR.
  FunctionPass *createPrinterPass(raw_ostream &OS,
                                  const std::string &Banner) const override {
    return createHIRPrinterPass(OS, Banner);
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  bool runOnFunction(Function &Fn) override;
};

// FIXME: \p VF is the single VF that we have VPlan for. That should be changed
// in the future and the argument won't be required.
template <typename CostModelTy = VPlanCostModel>
void printCostModelAnalysisIfRequested(LoopVectorizationPlanner &LVP,
                                       const TargetTransformInfo *TTI,
                                       const DataLayout *DL) {
  for (unsigned VFRequested : VPlanCostModelPrintAnalysisForVF) {
    if (!LVP.hasVPlanForVF(VFRequested)) {
      errs() << "VPlan for VF = " << VFRequested << " was not constructed\n";
      continue;
    }
    VPlan *Plan = LVP.getVPlanForVF(VFRequested);
    CostModelTy CM(Plan, VFRequested, TTI, DL);

    // If different stages in VPlanDriver were proper passes under pass manager
    // control it would have been opt's output stream (via "-o" switch). As it
    // is not so, just pass stdout so that we would not be required to redirect
    // stderr to Filecheck.
    CM.print(outs());
  }
}

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
    for (BasicBlock *SuccBB : successors(BB)) {
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
  AU.addRequired<WRegionInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

// Common LLVM-IR/HIR high-level implementation to process a function. It gets
// LLVM-IR-HIR common analyses and choose an execution mode.
template <class LoopType>
bool VPlanDriverBase<LoopType>::processFunction(
    Function &Fn, WRegionCollection::InputIRKind IR) {

  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(Fn);

  // We cannot rely on compiler driver not invoking vectorizer for
  // non-vector targets. Ensure vectorizer won't cause any issues for
  // such targets.
  if (TTI->getRegisterBitWidth(true) == 0)
    return false;

  TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  DL = &Fn.getParent()->getDataLayout();

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

  LLVM_DEBUG(dbgs() << "VD: Stardard Vectorization mode\n");

  WR = &getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
  WR->buildWRGraph(IR);
  WRContainerImpl *WRGraph = WR->getWRGraph();

  LLVM_DEBUG(dbgs() << "WD: WRGraph #nodes= " << WRGraph->size() << "\n");

  bool ModifiedFunc = false;
  for (auto WRNode : *WRGraph) {

    if (WRNVecLoopNode *WRLp = dyn_cast<WRNVecLoopNode>(WRNode)) {
      LoopType *Lp = WRLp->getTheLoop<LoopType>();
      //      simplifyLoop(Lp, DT, LI, SE, AC, false /* PreserveLCSSA */);
      //      formLCSSARecursively(*Lp, *DT, LI, SE);

      assert((VPlanForceBuild || isSupported(Lp)) &&
             "Loop is not supported by VPlan");

      LLVM_DEBUG(dbgs() << "VD: Starting VPlan for \n");
      LLVM_DEBUG(WRNode->dump());

      ModifiedFunc |= processLoop(Lp, Fn, WRLp);
    }
  }

  return ModifiedFunc;
}

/// Construction Stress Testing Mode: builds the H-CFG for any loop in the
/// function.
/// TODO: WIP for HIR.
template <class LoopType>
bool VPlanDriverBase<LoopType>::runConstructStressTestMode(Function &Fn) {

  LLVM_DEBUG(dbgs() << "VD: VPlan Construction Stress Test mode\n");

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
        ModifiedFunc |= processLoop(Lp, Fn);
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

  LLVM_DEBUG(dbgs() << "VD: VPlan CG Stress Test mode\n");

  SmallVector<LoopType *, 8> Worklist;
  collectAllLoops(Worklist);

  int ModifiedFunc = false;
  for (LoopType *Lp : Worklist) {
    if (CandLoopsVectorized < VPlanVectCand && isVPlanCandidate(Fn, Lp)) {
      ModifiedFunc |= processLoop(Lp, Fn);
      CandLoopsVectorized++;
    }
  }

  return ModifiedFunc;
}

INITIALIZE_PASS_BEGIN(VPlanDriver, "VPlanDriver", "VPlan Vectorization Driver",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)

INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DemandedBitsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopAccessLegacyAnalysis)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)

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

  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<DemandedBitsWrapperPass>();
  AU.addRequired<LoopAccessLegacyAnalysis>();
  AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
}

bool VPlanDriver::runOnFunction(Function &Fn) {
  if (skipFunction(Fn))
    return false;

  LLVM_DEBUG(dbgs() << "VPlan LLVM-IR Driver for Function: " << Fn.getName()
                    << "\n");

  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  // TODO: LI shouldn't be necessary as we are building VPLoopInfo. Maybe only
  // for debug/stress testing.
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(Fn);

  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  DB = &getAnalysis<DemandedBitsWrapperPass>().getDemandedBits();
  ORE = &getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();
  LAA = &getAnalysis<LoopAccessLegacyAnalysis>();

  bool ModifiedFunc =
      VPlanDriverBase::processFunction(Fn, WRegionCollection::LLVMIR);

  // Remove calls to directive intrinsics since the LLVM back end does not know
  // how to translate them.
  VPOUtils::stripDirectives(Fn);

  return ModifiedFunc;
}

bool VPlanDriver::processLoop(Loop *Lp, Function &Fn, WRNVecLoopNode *WRLp) {
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
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: Cannot prove legality.\n");

    // Only bail out if we are generating code, we want to continue if
    // we are only stress testing VPlan builds below.
    if (!VPlanConstrStressTest && !DisableCodeGen)
      return false;
  }

  LoopVectorizationPlanner LVP(WRLp, Lp, LI, SE, TLI, TTI, DL, DT, &LVL);

  LVP.buildInitialVPlans();
  printCostModelAnalysisIfRequested(LVP, TTI, DL);

  // VPlan Predicator
  LVP.predicate();

  // VPlan construction stress test ends here.
  if (VPlanConstrStressTest)
    return false;

  assert((WRLp || VPlanVectCand) && "WRLp can be null in stress testing only!");

  unsigned VF = LVP.selectBestPlan();

  LLVM_DEBUG(std::string PlanName; raw_string_ostream RSO(PlanName);
             RSO << "VD: Initial VPlan for VF=" << VF; RSO.flush();
             VPlan *Plan = LVP.getVPlanForVF(VF); Plan->setName(PlanName);
             dbgs() << *Plan);

#if INTEL_CUSTOMIZATION
  if (VPlanPrintInit) {
    VPlan *Plan = LVP.getVPlanForVF(VF);
    errs() << "Print initial VPlan for VF=" << VF << "\n";
    Plan->dump(errs());
  }
#endif

  bool ModifiedLoop = false;
  if (!DisableCodeGen) {
    if (VPlanVectCand)
      LLVM_DEBUG(dbgs() << "VD: VPlan Generating code in function: "
                        << Fn.getName() << "\n");

    VPOCodeGen VCodeGen(Lp, PSE, LI, DT, TLI, TTI, VF, 1, &LVL);
    VCodeGen.initOpenCLScalarSelectSet(volcanoScalarSelect);
    if (VF != 1) {
      LVP.executeBestPlan(VCodeGen);
      ModifiedLoop = true;
    }
  }

  return ModifiedLoop;
}

// Auxiliary function that checks only loop-specific constraints. Generic loop
// nest constraints are in 'isSupported' function.
static bool isSupportedRec(Loop *Lp) {

  if (!Lp->getUniqueExitBlock()) {
    LLVM_DEBUG(dbgs() << "VD: loop form "
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

  // When running directly from opt there is no guarantee that the loop is in
  // LCSSA form because we no longer apply this transformation from within
  // VPlanDriver. The reasoning behind this is due to the idea that we want to
  // perform all the enabling transformations before VPlanDriver so that the
  // only modifications to the underlying LLVM IR are done as a result of
  // vectorization.
  assert(Lp->isRecursivelyLCSSAForm(*DT, *LI) && "Loop is not in LCSSA form!");

  // Check for loop specific constraints
  if (!isSupportedRec(Lp)) {
    LLVM_DEBUG(dbgs() << "VD: loop nest "
                      << "(" << Lp->getName() << ") is not supported.\n");
    return false;
  }

  // Check generic loop nest constraints
  if (isIrreducibleCFG(Lp, LI)) {
    LLVM_DEBUG(dbgs() << "VD: loop nest "
                      << "(" << Lp->getName()
                      << ") is not supported: irreducible CFG.\n");
    return false;
  }

  for (BasicBlock *BB : Lp->blocks()) {
    // We don't support switch statements inside loops.
    if (!isa<BranchInst>(BB->getTerminator())) {
      LLVM_DEBUG(dbgs() << "VD: loop nest contains a switch statement.\n");
      return false;
    }
  }

  LLVM_DEBUG(dbgs() << "VD: loop nest "
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

INITIALIZE_PASS_BEGIN(VPlanDriverHIR, "VPlanDriverHIR",
                      "VPlan Vectorization Driver HIR", false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
// INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
// INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
// INITIALIZE_PASS_DEPENDENCY(HIRVectVLSAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)
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

  AU.addRequired<HIRFrameworkWrapperPass>();
  AU.addRequired<HIRLoopStatisticsWrapperPass>();
  //  AU.addRequiredTransitive<HIRLoopLocalityWrapperPass>();
  AU.addRequired<HIRDDAnalysisWrapperPass>();
  AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
  //  AU.addRequired<HIRVectVLSAnalysis>();
  AU.addRequired<OptReportOptionsPass>();
}

bool VPlanDriverHIR::runOnFunction(Function &Fn) {
  if (skipFunction(Fn))
    return false;

  LLVM_DEBUG(dbgs() << "VPlan HIR Driver for Function: " << Fn.getName()
                    << "\n");

  HIRF = &getAnalysis<HIRFrameworkWrapperPass>().getHIR();
  HIRLoopStats = &getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS();
  DDA = &getAnalysis<HIRDDAnalysisWrapperPass>().getDDA();
  // VLS = &getAnalysis<HIRVectVLSAnalysis>();
  LORBuilder.setup(Fn.getContext(),
                   getAnalysis<OptReportOptionsPass>().getVerbosity());

  return VPlanDriverBase::processFunction(Fn, WRegionCollection::HIR);
}

bool VPlanDriverHIR::processLoop(HLLoop *Lp, Function &Fn,
                                 WRNVecLoopNode *WRLp) {
  // TODO: How do we allow stress-testing for HIR path?
  assert(WRLp && "WRLp should be non-null!");

  // TODO: Do we need legality check in HIR?. If we reach this point, the loop
  // either has been marked with SIMD directive by 'HIR Vec Directive Insertion
  // Pass' or we are in stress testing mode.
  // VPOVectorizationLegality LVL(Lp, PSE, TLI, TTI, &Fn, LI, DT);

  // Send explicit data from WRLoop to the Legality.
  // The decision about possible loop vectorization is based
  // on this data.
  // TODO: EnterExplicitData works with Values. This is weird. Please, revisit.
  // LoopVectorizationPlanner::EnterExplicitData(WRLp, LVL);

  HLLoop *HLoop = WRLp->getTheLoop<HLLoop>();
  assert(HLoop && "Expected HIR Loop.");
  assert(HLoop->getParentRegion() && "Expected parent HLRegion.");

  const DDGraph &DDG = DDA->getGraph(HLoop);

  // TODO: No Legal for HIR.
  LoopVectorizationPlannerHIR LVP(WRLp, Lp, TLI, TTI, DL, nullptr /*Legal*/,
                                  DDG);

  LVP.buildInitialVPlans();

  printCostModelAnalysisIfRequested<VPlanCostModelProprietary>(LVP, TTI, DL);

  // VPlan construction stress test ends here.
  // TODO: Move after predication.
  if (VPlanConstrStressTest)
    return false;

  // VPlan Predicator
  LVP.predicate();

  // TODO: don't force vectorization if getIsAutoVec() is set to true.
  unsigned VF = LVP.selectBestPlan<VPlanCostModelProprietary>();

  // Set the final name for this initial VPlan.
  std::string PlanName;
  raw_string_ostream RSO(PlanName);
  RSO << "Initial VPlan for VF=" << VF;
  RSO.flush();
  VPlan *Plan = LVP.getVPlanForVF(VF);
  Plan->setName(PlanName);

  LLVM_DEBUG(dbgs() << "VD:\n" << *Plan);

#if INTEL_CUSTOMIZATION
  if (VPlanPrintInit) {
    errs() << "Print initial VPlan for VF=" << VF << "\n";
    Plan->dump(errs());
  }
#endif

  bool ModifiedLoop = false;
  if (!DisableCodeGen) {
    HIRSafeReductionAnalysis *SRA;
    SRA = &getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR();
    bool IsSearchLoop = VPlanIdioms::isAnySearchLoop(Plan, VF, true);
    VPOCodeGenHIR VCodeGen(TLI, SRA, Fn, Lp, LORBuilder, WRLp, IsSearchLoop);
    bool LoopIsHandled = (VF != 1 && VCodeGen.loopIsHandled(Lp, VF));

    // Erase intrinsics before and after the loop if we either vectorized the
    // loop or if this loop is an auto vectorization candidate. SIMD Intrinsics
    // are left around for loops that are not vectorized.
    if (LoopIsHandled || WRLp->getIsAutoVec())
      VCodeGen.eraseLoopIntrins();

    if (LoopIsHandled) {
      CandLoopsVectorized++;
      LVP.executeBestPlan(&VCodeGen);
      ModifiedLoop = true;
    }
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

// IMPORTANT -- keep this function at the end of the file until VPO and
// LoopVectorization legality can be merged.
#undef LoopVectorizationLegality
#include "llvm/Transforms/Vectorize/LoopVectorizationLegality.h"
bool VPlanDriver::isVPlanCandidate(Function &Fn, Loop *Lp) {
  // Only consider inner loops
  if (!Lp->empty())
    return false;

  PredicatedScalarEvolution PSE(*SE, *Lp);
  LoopVectorizationRequirements Requirements(*ORE);
  LoopVectorizeHints Hints(Lp, true, *ORE);
  std::function<const LoopAccessInfo &(Loop &)> GetLAA =
      [&](Loop &L) -> const LoopAccessInfo & { return LAA->getInfo(&L); };
  LoopVectorizationLegality LVL(Lp, PSE, DT, TLI, AA, &Fn, &GetLAA, LI, ORE,
                                &Requirements, &Hints, DB, AC);

  if (!LVL.canVectorize(false /* EnableVPlanNativePath */))
    return false;

  // No induction - bail out for now
  if (!LVL.getPrimaryInduction())
    return false;

  // Bail out if any runtime checks are needed
  auto LAI = &GetLAA(*Lp);
  if (LAI->getNumRuntimePointerChecks())
    return false;

  return true;
}
