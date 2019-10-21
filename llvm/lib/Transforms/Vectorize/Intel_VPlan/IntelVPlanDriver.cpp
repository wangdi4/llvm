//===-- IntelVPlanDriver.cpp ----------------------------------------------===//
//
//   Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "IntelVPlanDriver.h"
#include "IntelLoopVectorizationCodeGen.h"
#include "IntelLoopVectorizationLegality.h"
#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanPredicator.h"
#include "IntelVolcanoOpenCL.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Vectorize.h"
#if INTEL_CUSTOMIZATION
#include "IntelVPlanCostModelProprietary.h"
#include "VPlanHIR/IntelLoopVectorizationPlannerHIR.h"
#include "VPlanHIR/IntelVPOCodeGenHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#endif // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "VPlanDriver"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt; // INTEL

static cl::opt<bool> DisableCodeGen(
    "disable-vplan-codegen", cl::init(false), cl::Hidden,
    cl::desc(
        "Disable VPO codegen, when true, the pass stops at VPlan creation"));

// TODO: Unify with LoopVectorize's vplan-build-stress-test?
cl::opt<bool> VPlanConstrStressTest(
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

// New predicator is used in the LLVM IR path when this flag is true. This flag
// is currently NFC for the HIR path.
static cl::opt<bool>
    EnableNewVPlanPredicator("enable-new-vplan-predicator", cl::init(true),
                             cl::Hidden,
                             cl::desc("Enable New VPlan predicator."));
#endif //INTEL_CUSTOMIZATION

static cl::opt<unsigned> VPlanVectCand(
    "vplan-build-vect-candidates", cl::init(0),
    cl::desc(
        "Construct VPlan for vectorization candidates (CG stress testing)"));

STATISTIC(CandLoopsVectorized, "Number of candidate loops vectorized");

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

#if INTEL_CUSTOMIZATION
// Common LLVM-IR/HIR high-level implementation to process a function. It gets
// LLVM-IR-HIR common analyses and choose an execution mode.
template <typename Loop>
#endif // INTEL_CUSTOMIZATION
bool VPlanDriver::processFunction(Function &Fn) {

  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(Fn);

  // We cannot rely on compiler driver not invoking vectorizer for
  // non-vector targets. Ensure vectorizer won't cause any issues for
  // such targets.
  if (TTI->getRegisterBitWidth(true) == 0)
    return false;

  TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(Fn);
  DL = &Fn.getParent()->getDataLayout();

  assert(!(VPlanVectCand && VPlanConstrStressTest) &&
         "Stress testing for VPlan "
         "Construction and CG cannot be "
         "enabled at the same time.");

  bool ModifiedFunc = false;

// Execution modes
#if INTEL_CUSTOMIZATION
  if (VPlanVectCand)
    ModifiedFunc = runCGStressTestMode<Loop>(Fn);
  else if (!VPlanConstrStressTest)
    ModifiedFunc = runStandardMode<Loop>(Fn);
  else {
    ModifiedFunc = runConstructStressTestMode<Loop>(Fn);
    assert(!ModifiedFunc &&
           "VPlan Construction stress testing can't modify Function!");
  }
#else  // INTEL_CUSTOMIZATION
  if (VPlanVectCand)
    ModifiedFunc = runCGStressTestMode<Loop>(Fn);
  else if (!VPlanConstrStressTest)
    ModifiedFunc = runStandardMode(Fn);
  else {
    ModifiedFunc = runConstructStressTestMode(Fn);
    assert(!ModifiedFunc &&
           "VPlan Construction stress testing can't modify Function!");
  }
#endif // INTEL_CUSTOMIZATION
  return ModifiedFunc;
}

/// Standard Mode: standard path for (TODO: automatic and) explicit
/// vectorization.
/// Explicit vectorization: it uses WRegion analysis to collect and vectorize
/// all the WRNVecLoopNode's.
#if INTEL_CUSTOMIZATION
template <typename Loop>
#endif // INTEL_CUSTOMIZATION
bool VPlanDriver::runStandardMode(Function &Fn) {

  LLVM_DEBUG(dbgs() << "VD: Stardard Vectorization mode\n");

  WR = &getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
#if INTEL_CUSTOMIZATION
  IRKind IR = IRKind::LLVMIR;
  if (std::is_same<Loop, HLLoop>::value)
    IR = IRKind::HIR;
  WR->buildWRGraph(IR);
#else
  WR->buildWRGraph();
#endif // INTEL_CUSTOMIZATION
  WRContainerImpl *WRGraph = WR->getWRGraph();

  LLVM_DEBUG(dbgs() << "WD: WRGraph #nodes= " << WRGraph->size() << "\n");

  bool ModifiedFunc = false;
  for (auto WRNode : *WRGraph) {

    if (WRNVecLoopNode *WRLp = dyn_cast<WRNVecLoopNode>(WRNode)) {
      Loop *Lp = WRLp->getTheLoop<Loop>();
      //      simplifyLoop(Lp, DT, LI, SE, AC, false /* PreserveLCSSA */);
      //      formLCSSARecursively(*Lp, *DT, LI, SE);

      if (!Lp) {
        LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop was optimized out.\n");
        continue;
      }

      if (!VPlanForceBuild && !isSupported(Lp)) {
        LLVM_DEBUG(dbgs() << "Bailing out: Loop is not supported!\n");
        continue;
      }

      LLVM_DEBUG(dbgs() << "VD: Starting VPlan for \n");
      LLVM_DEBUG(WRNode->dump());

      ModifiedFunc |= processLoop(Lp, Fn, WRLp);
    }
  }

  return ModifiedFunc;
}

/// Construction Stress Testing Mode: builds the H-CFG for any loop in the
/// function.
#if INTEL_CUSTOMIZATION
/// TODO: WIP for HIR.
template <typename Loop>
#endif //INTEL_CUSTOMIZATION
bool VPlanDriver::runConstructStressTestMode(Function &Fn) {

  LLVM_DEBUG(dbgs() << "VD: VPlan Construction Stress Test mode\n");

  SmallVector<Loop *, 8> Worklist;
  collectAllLoops(Worklist);

  bool ModifiedFunc = false;
  for (Loop *Lp : Worklist) {
    HIRLoopAdapter<Loop> LpAdapter(Lp);
    // Process only innermost loops if VPlanStressOnlyInnermost is enabled
    if (!VPlanStressOnlyInnermost || LpAdapter.isInnermost()) {
      // simplifyLoop(Lp, DT, LI, SE, AC, false /* PreserveLCSSA */);
      // formLCSSARecursively(*Lp, *DT, LI, SE);
      if (VPlanForceBuild || isSupported(Lp))
        ModifiedFunc |= processLoop(Lp, Fn, nullptr /*No WRegion*/);
    }
  }

  return ModifiedFunc;
}

/// CG Stress Testing Mode: generates vector code for the first VPlanVectCand
/// number of loops marked as vectorizable using LoopVectorize analysis. When
/// debugging vector CG issues, we can do a binary search to find out the
/// problem loop by setting VPlanVectCand appropriately.
#if INTEL_CUSTOMIZATION
template <typename Loop>
#endif //INTEL_CUSTOMIZATION
bool VPlanDriver::runCGStressTestMode(Function &Fn) {

  LLVM_DEBUG(dbgs() << "VD: VPlan CG Stress Test mode\n");

  SmallVector<Loop *, 8> Worklist;
  collectAllLoops(Worklist);

  int ModifiedFunc = false;
  for (Loop *Lp : Worklist) {
    if (CandLoopsVectorized < VPlanVectCand && isVPlanCandidate(Fn, Lp)) {
      ModifiedFunc |= processLoop(Lp, Fn, nullptr /* No WRegion */);
    }
  }

  return ModifiedFunc;
}

/// Function to add vectorization related remarks for loops created by given
/// codegen object \p VCodeGen
#if INTEL_CUSTOMIZATION
// TODO: Change VPOCodeGenType. This cannot be used in the open sourcing patches
template <class VPOCodeGenType, typename Loop>
#else
template <class VPOCodeGenType>
#endif //INTEL_CUSTOMIZATION
void VPlanDriver::addOptReportRemarks(VPlanOptReportBuilder &VPORBuilder,
                                      VPOCodeGenType *VCodeGen) {
  // The new vectorized loop is stored in MainLoop
  Loop *MainLoop = VCodeGen->getMainLoop();

  // Adds remark LOOP WAS VECTORIZED
  VPORBuilder.addRemark(MainLoop, OptReportVerbosity::Low, 15300);
  // Add remark about VF
  VPORBuilder.addRemark(MainLoop, OptReportVerbosity::Low, 15305,
                        Twine(VCodeGen->getVF()).str());

  // If remainder loop was generated for MainLoop, report that it is currently
  // not vectorized
  if (VCodeGen->getNeedRemainderLoop()) {
    Loop *RemLoop = VCodeGen->getRemainderLoop();
    VPORBuilder.addRemark(RemLoop, OptReportVerbosity::Medium, 15441, "");
  }
}

// Definitions of addRemark functions in VPlanOptReportBuilder
template <typename... Args>
void VPlanOptReportBuilder::addRemark(HLLoop *Lp,
                                      OptReportVerbosity::Level Verbosity,
                                      unsigned MsgID, Args &&... args) {
  LORBuilder(*Lp).addRemark(Verbosity, loopopt::OptReportDiag::getMsg(MsgID),
                            std::forward<Args>(args)...);
}

template <typename... Args>
void VPlanOptReportBuilder::addRemark(Loop *Lp,
                                      OptReportVerbosity::Level Verbosity,
                                      unsigned MsgID, Args &&... args) {
  // For LLVM-IR Loop, LORB needs a valid LoopInfo object
  assert(LI && "LoopInfo for opt-report builder is null.");
  LORBuilder(*Lp, *LI).addRemark(Verbosity,
                                 loopopt::OptReportDiag::getMsg(MsgID),
                                 std::forward<Args>(args)...);
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
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)

INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(VPlanDriver, "VPlanDriver", "VPlan Vectorization Driver",
                    false, false)

char VPlanDriver::ID = 0;

Pass *llvm::createVPlanDriverPass() { return new VPlanDriver(); }

void VPlanDriver::getAnalysisUsage(AnalysisUsage &AU) const {

#if INTEL_CUSTOMIZATION
  // TODO (CMPLRS-44750): We do not preserve LoopInfo as we remove loops, create
  // new loops. Same holds for Scalar Evolution which needs to be computed for
  // newly created loops.

  // TODO (CMPLRS-44750): Preserve analyses.
#endif // INTEL_CUSTOMIZATION
  AU.addRequired<WRegionInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();

  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<AssumptionCacheTracker>();

  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<DemandedBitsWrapperPass>();
  AU.addRequired<LoopAccessLegacyAnalysis>();
  AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
  AU.addRequired<OptReportOptionsPass>();

  AU.addPreserved<AndersensAAWrapperPass>();
  AU.addPreserved<GlobalsAAWrapperPass>();
}

bool VPlanDriver::runOnFunction(Function &Fn) {
  if (skipFunction(Fn))
    return false;

#if INTEL_CUSTOMIZATION
  LLVM_DEBUG(dbgs() << "VPlan LLVM-IR Driver for Function: " << Fn.getName()
                    << "\n");
#endif // INTEL_CUSTOMIZATION

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
  LORBuilder.setup(Fn.getContext(),
                   getAnalysis<OptReportOptionsPass>().getVerbosity());

  bool ModifiedFunc = processFunction(Fn);

  return ModifiedFunc;
}

namespace llvm {
namespace vpo {

#if INTEL_CUSTOMIZATION
template <>
bool VPlanDriver::processLoop<llvm::Loop>(Loop *Lp, Function &Fn,
                                          WRNVecLoopNode *WRLp) {
#else
bool VPlanDriver::processLoop(Loop *Lp, Function &Fn, WRNVecLoopNode *WRLp) {
#endif // INTEL_CUSTOMIZATION
  PredicatedScalarEvolution PSE(*SE, *Lp);
  VPOVectorizationLegality LVL(Lp, PSE, &Fn);

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

  // Create a VPlanOptReportBuilder object, lifetime is a single loop that we
  // process for vectorization
  VPlanOptReportBuilder VPORBuilder(LORBuilder, LI);

  BasicBlock *Header = Lp->getHeader();
  VPlanVLSAnalysis VLSA(Lp, Header->getContext(), *DL, SE);
  LoopVectorizationPlanner LVP(WRLp, Lp, LI, SE, TLI, TTI, DL, DT, &LVL, &VLSA);

#if INTEL_CUSTOMIZATION
  // Setup the use of new predicator in the planner if user has not disabled
  // the same.
  if (EnableNewVPlanPredicator)
    LVP.setUseNewPredicator();

  if (!LVP.buildInitialVPlans(&Fn.getContext(), DL)) {
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: No VPlans constructed.\n");
    return false;
  }
#else
  LVP.buildInitialVPlans();
#endif //INTEL_CUSTOMIZATION

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LVP.printCostModelAnalysisIfRequested();
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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
    (void)Plan;
    assert(Plan && "Unexpected null VPlan");
    errs() << "Print initial VPlan for VF=" << VF << "\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    Plan->dump(errs());
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
#endif //INTEL_CUSTOMIZATION

  bool ModifiedLoop = false;
  if (!DisableCodeGen) {
    if (VPlanVectCand)
      LLVM_DEBUG(dbgs() << "VD: VPlan Generating code in function: "
                        << Fn.getName() << "\n");

    VPlan *Plan = LVP.getVPlanForVF(VF);
    VPOCodeGen VCodeGen(Lp, Fn.getContext(), PSE, LI, DT, TLI, TTI, VF, 1, &LVL,
                        &VLSA, Plan);
    VCodeGen.initOpenCLScalarSelectSet(volcanoScalarSelect);
    if (VF != 1) {
      // Run VLS analysis before IR for the current loop is modified.
      VCodeGen.getVLS()->getOVLSMemrefs(LVP.getVPlanForVF(VF), VF);

      LVP.executeBestPlan(VCodeGen);

      // Strip the directives once the loop is vectorized. In stress testing,
      // WRLp is null and no directives need deletion.
      if (WRLp)
        VPOUtils::stripDirectives(WRLp);

      CandLoopsVectorized++;
      ModifiedLoop = true;
      addOptReportRemarks<VPOCodeGen>(VPORBuilder, &VCodeGen);
    }
  }

  // Emit opt report remark if a VPlan candidate SIMD loop was not vectorized
  // TODO: Emit reason for bailing out
  if (!ModifiedLoop)
    VPORBuilder.addRemark(Lp, OptReportVerbosity::Medium, 15436, "");

  return ModifiedLoop;
}

#if INTEL_CUSTOMIZATION
template <>
bool VPlanDriver::processLoop<vpo::HLLoop>(vpo::HLLoop *Lp, Function &Fn,
                                           WRNVecLoopNode *WRLp) {
  auto *Self = static_cast<VPlanDriverHIR *>(this);
  return Self->processLoop(Lp, Fn, WRLp);
}
#endif // INTEL_CUSTOMIZATION
} // namespace vpo
} // namespace llvm
// The interface getUniqueExitBlock() asserts that the loop has dedicated
// exits. Check that a loop has dedicated exits before the check for unique
// exit block. This is especially needed when stress testing VPlan builds.
static bool hasDedicadedAndUniqueExits(Loop *Lp) {

  if (!Lp->hasDedicatedExits()) {
    LLVM_DEBUG(dbgs() << "VD: loop form "
                      << "(" << Lp->getName()
                      << ") is not supported: no dedicated exits.\n");
    return false;
  }

  if (!Lp->getUniqueExitBlock()) {
    LLVM_DEBUG(dbgs() << "VD: loop form "
                      << "(" << Lp->getName()
                      << ") is not supported: multiple exit blocks.\n");
    return false;
  }
  return true;
}

// Auxiliary function that checks only loop-specific constraints. Generic loop
// nest constraints are in 'isSupported' function.
static bool isSupportedRec(Loop *Lp) {

  if (!LoopMassagingEnabled && !hasDedicadedAndUniqueExits(Lp))
    return false;

  for (Loop *SubLoop : Lp->getSubLoops()) {
    if (!isSupportedRec(SubLoop))
      return false;
  }

  return true;
}

namespace llvm {
namespace vpo {
// Return true if this loop is supported in VPlan
#if INTEL_CUSTOMIZATION
template <> bool VPlanDriver::isSupported<llvm::Loop>(Loop *Lp) {
#else  // INTEL_CUSTOMIZATION
  bool VPlanDriver::isSupported(Loop *Lp) {
#endif // INTEL_CUSTOMIZATION

  // When running directly from opt there is no guarantee that the loop is in
  // LCSSA form because we no longer apply this transformation from within
  // VPlanDriver. The reasoning behind this is due to the idea that we want to
  // perform all the enabling transformations before VPlanDriver so that the
  // only modifications to the underlying LLVM IR are done as a result of
  // vectorization. When stress testing VPlan construction, allow loops not
  // in LCSSA form.
  if (!VPlanConstrStressTest)
    assert(Lp->isRecursivelyLCSSAForm(*DT, *LI) &&
           "Loop is not in LCSSA form!");

  if (!hasDedicadedAndUniqueExits(Lp))
    return false;

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

#if INTEL_CUSTOMIZATION
template <> bool VPlanDriver::isSupported<vpo::HLLoop>(vpo::HLLoop *Lp) {
  auto *Self = static_cast<VPlanDriverHIR *>(this);
  return Self->isSupported(Lp);
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
template <>
void VPlanDriver::collectAllLoops<llvm::Loop>(SmallVectorImpl<Loop *> &Loops) {
#else  // INTEL_CUSTOMIZATION
void VPlanDriver::collectAllLoops(SmallVectorImpl<Loop *> &Loops) {
#endif // INTEL_CUSTOMIZATION

  std::function<void(Loop *)> collectSubLoops = [&](Loop *Lp) {
    Loops.push_back(Lp);
    for (Loop *InnerLp : Lp->getSubLoops())
      collectSubLoops(InnerLp);
  };

  for (Loop *Lp : *LI)
    collectSubLoops(Lp);
}

#if INTEL_CUSTOMIZATION
template <>
void VPlanDriver::collectAllLoops<vpo::HLLoop>(
    SmallVectorImpl<vpo::HLLoop *> &Loops) {
  auto *Self = static_cast<VPlanDriverHIR *>(this);
  return Self->collectAllLoops(Loops);
}
#endif // INTEL_CUSTOMIZATION
} // namespace vpo
} // namespace llvm

#if INTEL_CUSTOMIZATION
INITIALIZE_PASS_BEGIN(VPlanDriverHIR, "VPlanDriverHIR",
                      "VPlan Vectorization Driver HIR", false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
// INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
// INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
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

  AU.addRequired<WRegionInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();

  // HIR path does not work without setPreservesAll
  AU.setPreservesAll(); // TODO: ?

  //  AU.addRequired<LoopInfoWrapperPass>();
  //  AU.addRequired<ScalarEvolutionWrapperPass>();

  AU.addRequired<HIRFrameworkWrapperPass>();
  AU.addRequired<HIRLoopStatisticsWrapperPass>();
  //  AU.addRequiredTransitive<HIRLoopLocalityWrapperPass>();
  AU.addRequired<HIRDDAnalysisWrapperPass>();
  AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
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
  LORBuilder.setup(Fn.getContext(),
                   getAnalysis<OptReportOptionsPass>().getVerbosity());

  return VPlanDriver::processFunction<loopopt::HLLoop>(Fn);
}

bool VPlanDriverHIR::processLoop(HLLoop *Lp, Function &Fn,
                                 WRNVecLoopNode *WRLp) {
  // TODO: How do we allow stress-testing for HIR path?
  assert(WRLp && "WRLp should be non-null!");

  HLLoop *HLoop = WRLp->getTheLoop<HLLoop>();
  (void)HLoop;
  assert(HLoop && "Expected HIR Loop.");
  assert(HLoop->getParentRegion() && "Expected parent HLRegion.");

  if (WRLp->isOmpSIMDLoop() && !WRLp->isValidHIRSIMDRegion()) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    assert(false && "VPlan: Invalid HIR SIMD region for given loop");
#else
    WithColor::warning() << "Loop was not vectorized. Invalid SIMD region "
                            "detected for given loop\n";
    return false;
#endif
  }

  // Create a VPlanOptReportBuilder object, lifetime is a single loop that we
  // process for vectorization
  VPlanOptReportBuilder VPORBuilder(LORBuilder);

  VPlanVLSAnalysisHIR VLSA(DDA, Fn.getContext(), *DL);

  HIRSafeReductionAnalysis *SafeRedAnalysis =
      &getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR();
  HIRVectorizationLegality HIRVecLegal(SafeRedAnalysis, DDA);
  LoopVectorizationPlannerHIR LVP(WRLp, Lp, TLI, TTI, DL, &HIRVecLegal, DDA,
                                  &VLSA);

  // Send explicit data from WRLoop to the Legality.
  LVP.EnterExplicitData(WRLp, HIRVecLegal);
  // Find any DDRefs in loop pre-header that are aliases to the descriptor
  // variables
  HIRVecLegal.findAliasDDRefs(WRLp->getEntryHLNode(), HLoop);

  // Setup the use of new predicator in the planner if user has not disabled
  // the same.
  if (EnableNewVPlanPredicator)
    LVP.setUseNewPredicator();

  if (!LVP.buildInitialVPlans(&Fn.getContext(), DL)) {
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: No VPlans constructed.\n");
    return false;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LVP.printCostModelAnalysisIfRequested<VPlanCostModelProprietary>();
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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
  assert(Plan && "Unexpected null VPlan");
  Plan->setName(PlanName);

  LLVM_DEBUG(dbgs() << "VD:\n" << *Plan);

  if (VPlanPrintInit) {
    errs() << "Print initial VPlan for VF=" << VF << "\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    Plan->dump(errs());
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }

  bool ModifiedLoop = false;
  if (!DisableCodeGen) {
    HIRSafeReductionAnalysis *SRA;
    SRA = &getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR();
    auto *VPLI = Plan->getVPLoopInfo();
    assert(std::distance(VPLI->begin(), VPLI->end()) == 1 &&
           "Expected single outermost loop!");
    VPLoop *OuterMostVPLoop = *VPLI->begin();
    const VPLoopEntityList *Entities =
        Plan->getLoopEntities(OuterMostVPLoop);
    RegDDRef *PeelArrayRef = nullptr;
    VPlanIdioms::Opcode SearchLoopOpcode =
        VPlanIdioms::isSearchLoop(Plan, VF, true, PeelArrayRef);
    VPOCodeGenHIR VCodeGen(TLI, TTI, SRA, &VLSA, Plan, Fn, Lp, LORBuilder, WRLp,
                           Entities, &HIRVecLegal, SearchLoopOpcode,
                           PeelArrayRef);
    bool LoopIsHandled = (VF != 1 && VCodeGen.loopIsHandled(Lp, VF));

    // Erase intrinsics before and after the loop if we either vectorized the
    // loop or if this loop is an auto vectorization candidate. SIMD Intrinsics
    // are left around for loops that are not vectorized.
    if (LoopIsHandled || WRLp->getIsAutoVec())
      VCodeGen.eraseLoopIntrins();

    if (LoopIsHandled) {
      CandLoopsVectorized++;
      if (LVP.executeBestPlan(&VCodeGen)) {
        ModifiedLoop = true;
        VPlanDriver::addOptReportRemarks<VPOCodeGenHIR, loopopt::HLLoop>(
            VPORBuilder, &VCodeGen);
      }
    }
  }

  // Emit opt report remark if a VPlan candidate loop was not vectorized
  // TODO: Emit reason for not vectorizing too, check
  // VPOCodeGenHIR::loopIsHandled
  if (!ModifiedLoop)
    VPORBuilder.addRemark(Lp, OptReportVerbosity::Medium, 15436, "");

  return ModifiedLoop;
}

bool VPlanDriverHIR::isSupported(HLLoop *Lp) {
  if (HIRLoopStats->getSelfLoopStatistics(Lp).hasSwitches())
    return false;

  return true;
}

void VPlanDriverHIR::collectAllLoops(SmallVectorImpl<HLLoop *> &Loops) {
  HIRF->getHLNodeUtils().gatherAllLoops(Loops);
}

bool VPlanDriverHIR::isVPlanCandidate(Function &Fn, HLLoop *Lp) {
  // This function is only used in the LLVM-IR path to generate VPlan
  // candidates.
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// \brief Overrides FunctionPass's printer pass to return one which prints
  /// HIR instead of LLVM IR.
 FunctionPass *VPlanDriverHIR::createPrinterPass(raw_ostream &OS,
                                  const std::string &Banner) const {
    return createHIRPrinterPass(OS, Banner);
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// IMPORTANT -- keep this function at the end of the file until VPO and
// LoopVectorization legality can be merged.
#undef LoopVectorizationLegality
#include "llvm/Transforms/Vectorize/LoopVectorizationLegality.h"
#endif // INTEL_CUSTOMIZATION

namespace llvm {
namespace vpo {
#if INTEL_CUSTOMIZATION
template <>
bool VPlanDriver::isVPlanCandidate<llvm::Loop>(Function &Fn, Loop *Lp) {
#else // INTEL_CUSTOMIZATION
bool VPlanDriver::isVPlanCandidate(Function &Fn, Loop *Lp) {
#endif
  // Only consider inner loops
  if (!Lp->empty())
    return false;

  PredicatedScalarEvolution PSE(*SE, *Lp);
  LoopVectorizationRequirements Requirements(*ORE);
  LoopVectorizeHints Hints(Lp, true, *ORE);
  std::function<const LoopAccessInfo &(Loop &)> GetLAA =
      [&](Loop &L) -> const LoopAccessInfo & { return LAA->getInfo(&L); };
  LoopVectorizationLegality LVL(Lp, PSE, DT, TTI, TLI, AA, &Fn, &GetLAA, LI, ORE,
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

#if INTEL_CUSTOMIZATION
template <>
bool VPlanDriver::isVPlanCandidate<vpo::HLLoop>(Function &Fn, vpo::HLLoop *Lp) {
  auto *Self = static_cast<VPlanDriverHIR *>(this);
  return Self->isVPlanCandidate(Fn, Lp);
}
#endif // INTEL_CUSTOMIZATION

} // namespace vpo
} // namespace llvm
