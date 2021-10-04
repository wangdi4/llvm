//===-- IntelVPlanDriver.cpp ----------------------------------------------===//
//
//   Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Vectorize/IntelVPlanDriver.h"
#include "IntelLoopVectorizationLegality.h"
#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPMemRefTransform.h"
#include "IntelVPOCodeGen.h"
#include "IntelVPOLoopAdapters.h"
#include "IntelVPSOAAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanAllZeroBypass.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanMaskedModeLoop.h"
#include "IntelVPlanScalarEvolution.h"
#include "IntelVPlanVConflictTransformation.h"
#include "IntelVPlanVLSTransform.h"
#include "IntelVolcanoOpenCL.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/DemandedBits.h"  // INTEL
#include "llvm/Analysis/GlobalsModRef.h" // INTEL_CUSTOMIZATION
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/MemorySSA.h" // INTEL_CUSTOMIZATION
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/IR/OptBisect.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Vectorize.h"
#if INTEL_CUSTOMIZATION
#include "VPlanHIR/IntelLoopVectorizationPlannerHIR.h"
#include "VPlanHIR/IntelVPOCodeGenHIR.h"
#include "VPlanHIR/IntelVPlanScalarEvolutionHIR.h"
#include "VPlanHIR/IntelVPlanValueTrackingHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#endif // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "vplan-vec"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt; // INTEL

static cl::opt<bool> DisableCodeGen(
    "disable-vplan-codegen", cl::init(false), cl::Hidden,
    cl::desc(
        "Disable VPO codegen, when true, the pass stops at VPlan creation"));

static cl::opt<bool> EnableOuterLoopHIR(
    "enable-vplan-outer-loop-hir", cl::init(false), cl::Hidden,
    cl::desc("Enable vectorization of outer loops in VPlan HIR path"));

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

static cl::opt<unsigned> VPlanVectCand(
    "vplan-build-vect-candidates", cl::init(0),
    cl::desc(
        "Construct VPlan for vectorization candidates (CG stress testing)"));

static cl::opt<bool>
    VPlanEnablePeelingOpt("vplan-enable-peeling", cl::init(false),
                          cl::desc("Enable generation of peel loops to improve "
                                   "alignment of memory accesses"));

static cl::opt<bool> VPlanEnablePeelingHIROpt(
    "vplan-enable-peeling-hir", cl::init(false), cl::Hidden,
    cl::desc("Enable generation of peel loops to improve "
             "alignment of memory accesses in HIR path"));

namespace llvm {
namespace vpo {
// Flag to indicate if peeling is enabled. Flag is set based on appropriate
// value of command line option for the IR kind being processed.
extern bool VPlanEnablePeeling;
} // namespace vpo
} // namespace llvm

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool>
    VPlanPrintInit("vplan-print-after-init", cl::init(false),
                   cl::desc("Print plain dump after initial VPlan generated"));

static cl::opt<bool> VPlanPrintAfterSingleTripCountOpt(
    "vplan-print-after-single-trip-count-opt", cl::init(false),
    cl::desc("Print after backedge branch rewrite for single trip count vector "
             "loop"));

static cl::opt<bool> PrintHIRBeforeVPlan(
    "print-hir-before-vplan", cl::init(false),
    cl::desc("Print HLLoop which we attempt to vectorize via VPlanDriverHIR"));
#else
static constexpr bool VPlanPrintInit = false;
static constexpr bool VPlanPrintAfterSingleTripCountOpt = false;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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

// This function adds new SzAddMD metadata string to the loop. We use it set
// llvm.loop.vectorize.enable and llvm.loop.isvectorized metadata attributes:
//   llvm.loop.vectorize.enable - is added by the front-end (called without
//   -fiopenmp option) or VPlan Vectorizer if pragma simd is specified the
//   the loop.
//   llvm.loop.isvectorized - is added by vectorizer for vectorized loops.
static void setLoopMD(const Loop *const Lp, const char *const SzAddMD) {
  if (!Lp)
    return;
  LLVMContext &Context = Lp->getHeader()->getContext();
  MDNode *AddMD = MDNode::get(
      Context,
      {MDString::get(Context, SzAddMD),
       ConstantAsMetadata::get(ConstantInt::get(Context, APInt(32, 1)))});
  MDNode *LoopID = Lp->getLoopID();
  MDNode *NewLoopID =
      makePostTransformationMetadata(Context, LoopID, {SzAddMD}, {AddMD});
  Lp->setLoopID(NewLoopID);
}

static void setHLLoopMD(HLLoop *const Lp, const char *const SzAddMD,
                        LLVMContext &Context) {
  if (!Lp)
    return;
  MDNode *AddMD = MDNode::get(
      Context,
      {MDString::get(Context, SzAddMD),
       ConstantAsMetadata::get(ConstantInt::get(Context, APInt(32, 1)))});
  Lp->addLoopMetadata({AddMD});
}

static bool canProcessMaskedVariant(const VPlan &P) {
  for (const VPInstruction &I : vpinstructions(&P))
    switch (I.getOpcode()) {
    default:
      break;
    // We need special processing for those instructions in masked mode,
    // as we need to extract last value not from (VF-1)th lane but from
    // the lane defined by the execution mask.
    case VPInstruction::PrivateFinalUncond:
    case VPInstruction::PrivateFinalUncondMem:
    case VPInstruction::PrivateFinalArray:
    case VPInstruction::PrivateLastValueNonPOD:
      return false;
    }
  return true;
}

#if INTEL_CUSTOMIZATION
template <>
bool VPlanDriverImpl::processLoop<llvm::Loop>(Loop *Lp, Function &Fn,
                                              WRNVecLoopNode *WRLp) {
#else
bool VPlanDriverImpl::processLoop(Loop *Lp, Function &Fn,
                                  WRNVecLoopNode *WRLp) {
#endif // INTEL_CUSTOMIZATION
  // Enable peeling for LLVM-IR path from command line switch
  VPlanEnablePeeling = VPlanEnablePeelingOpt;

  // TODO: Not sure if that's the correct long-term solution. If ScalarEvolution
  // can consume on-the-fly updates to the LoopInfo analysis, then we might be
  // able to use a single ScalarEvolution object. If not, then creating new
  // SE/PSE for each loop processed would be the correct solution in long-term
  // too.
  //
  // Not a question right now because we recalculate LoopInfo from scratch and
  // ScalarEvolution definitely can't work with that - all the SE's internal
  // maps with Loop's as keys would be stale.
  ScalarEvolution SE(Fn, *TLI, *AC, *DT, *LI);
  PredicatedScalarEvolution PSE(SE, *Lp);
  VPOVectorizationLegality LVL(Lp, PSE, &Fn);

  // If region has SIMD directive mark then we will reuse community metadata on
  // Loop so that WarnMissedTransforms pass will detect if this loop is not
  // vectorized later
  const bool isOmpSIMDLoop = WRLp && WRLp->isOmpSIMDLoop();
  if (isOmpSIMDLoop)
    setLoopMD(Lp, "llvm.loop.vectorize.enable");

  // Send explicit data from WRLoop to the Legality.
  // The decision about possible loop vectorization is based
  // on this data.
  LoopVectorizationPlanner::EnterExplicitData(WRLp, LVL);

  // The function canVectorize() collects information about induction
  // and reduction variables. It also verifies that the loop vectorization
  // is fully supported.
  CallInst *RegionEntry =
      (WRLp == nullptr) ? nullptr : cast<CallInst>(WRLp->getEntryDirective());
  bool CanVectorize = LVL.canVectorize(*DT, RegionEntry);
  if (!CanVectorize) {
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: Cannot prove legality.\n");

    // Only bail out if we are generating code, we want to continue if
    // we are only stress testing VPlan builds below.
    if (!VPlanConstrStressTest && !DisableCodeGen)
      return false;
  }

  BasicBlock *Header = Lp->getHeader();
  VPlanVLSAnalysis VLSA(Lp, Header->getContext(), *DL, TTI);
  LoopVectorizationPlanner LVP(WRLp, Lp, LI, TLI, TTI, DL, DT, &LVL, &VLSA);
  std::string VPlanName = "";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  VPlanName = std::string(Fn.getName()) + ":" + std::string(Lp->getName());
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  LVP.readLoopMetadata();
#if INTEL_CUSTOMIZATION
  if (!LVP.buildInitialVPlans(&Fn.getContext(), DL, VPlanName, &SE,
                              CanVectorize || DisableCodeGen)) {
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: No VPlans constructed.\n");
    return false;
  }

  VPAnalysesFactory VPAF(SE, Lp, DT, AC, DL);
  populateVPlanAnalyses(LVP, VPAF);

#else
  LVP.buildInitialVPlans();
#endif // INTEL_CUSTOMIZATION

  if (EnableMaskedVariant) {
    DenseMap<VPlanVector *, std::shared_ptr<VPlanMasked>> OrigClonedVPlans;
    for (auto &Pair : LVP.getAllVPlans()) {
      std::shared_ptr<VPlanVector> Plan = Pair.second.MainPlan;
      VPLoop *VLoop = Plan->getMainLoop(true);
      // Masked variant is not generated for loops without normalized induction.
      if (!VLoop->hasNormalizedInduction())
        continue;
      if (Pair.second.MaskedModeLoop)
        // Already have it.
        continue;

      auto It = OrigClonedVPlans.find(Plan.get());
      if (It != OrigClonedVPlans.end()) {
        // We have already cloned that main loop, add the same clone for
        // this VF.
        LVP.appendVPlanPair(
            Pair.first, LoopVectorizationPlanner::VPlanPair{Plan, It->second});
        continue;
      }
      // Check if we can process VPlan in masked mode. E.g. the code for some
      // entities processing is not implemented yet.
      if (!canProcessMaskedVariant(*Plan))
        continue;

      // Check whether we have a known TC and it's a power of two and is
      // less than maximum VF. In such cases the masked mode loop will be
      // most likely not used. (Peeling will be unprofitable and remainder
      // can be created unmasked.) This will allow us saving compile time
      // e.g. during function vectorization.
      // TODO: implement target check, i.e. whether target has instructions
      // to implement masked operations. If there are no such insructions we
      // don't enable masked variants. Though, that probably better to leave
      // for cost modeling.
      uint64_t TripCount = VLoop->getTripCountInfo().TripCount;
      if (!VLoop->getTripCountInfo().IsEstimated) {
        auto Max = *(std::max_element(LVP.getVectorFactors().begin(),
                                      LVP.getVectorFactors().end()));
        if (isPowerOf2_64(TripCount) && TripCount <= Max)
          continue;
      }

      MaskedModeLoopCreator MML(cast<VPlanNonMasked>(Plan.get()), VPAF);
      std::shared_ptr<VPlanMasked> MaskedPlan = MML.createMaskedModeLoop();
      OrigClonedVPlans[Plan.get()] = MaskedPlan;
      LVP.appendVPlanPair(
          Pair.first, LoopVectorizationPlanner::VPlanPair{Plan, MaskedPlan});
    }
  }

  // VPlan Predicator
  LVP.predicate();

  // VPlan construction stress test ends here.
  if (VPlanConstrStressTest)
    return false;

  assert((WRLp || VPlanVectCand) && "WRLp can be null in stress testing only!");

  unsigned VF;
  VPlanVector *Plan;
  std::tie(VF, Plan) = LVP.selectBestPlan();
  assert(Plan && "Unexpected null VPlan");

  LLVM_DEBUG(std::string PlanName; raw_string_ostream RSO(PlanName);
             RSO << "VD: Initial VPlan for VF=" << VF; RSO.flush();
             Plan->setName(PlanName); dbgs() << *Plan);

  VPLAN_DUMP(VPlanPrintInit,
             "initial VPlan for VF=" + std::to_string(VF), Plan);

  unsigned UF = LVP.getLoopUnrollFactor();
  // If EnableCFGMerge is disabled, run AZB and unroll at this point in the
  // pipeline.
  if (!EnableNewCFGMerge) {
    LVP.insertAllZeroBypasses(Plan, VF);
    LVP.unroll(*Plan);
  }

  // Workaround for kernel vectorization. Kernel vectorization is done through
  // loop creation inside vec-clone) followed by loop vectorization. That
  // leaves a loop CFG that can't be optimized away (even though it will be
  // 1-iteration loop) without scheduling indvars-simplify or unroller later in
  // the pipeline. We don't want to spend compile-time on these passes so do
  // some special casing here for a vector loop that will result in exactly one
  // iteration.
  //
  // For ahead of time calculation a better approach is to perform small-trip
  // count loop-unroll which won't be limited to exactly one iteration. For
  // kernel vectorization the long-term plan would be to import the region
  // itself into VPlan without artificial loop being created at all.
  if (auto *TripCount = dyn_cast<SCEVConstant>(PSE.getBackedgeTakenCount()))
    if ((VF * UF - 1) == TripCount->getAPInt()) {
      VPLoop *Lp = (*Plan->getVPLoopInfo()->begin());
      VPBasicBlock *Latch = Lp->getLoopLatch();
      assert(Latch && "Latch should not be a null pointer.");
      VPBasicBlock *Header = Lp->getHeader();
      bool BackedgeOnTrue = Latch->getSuccessor(0) == Header;
      auto &Context = Fn.getContext();
      auto *Cond = BackedgeOnTrue ? ConstantInt::getFalse(Context)
                                  : ConstantInt::getTrue(Context);
      auto *VPCond = Plan->getVPConstant(Cond);
      Latch->setCondBit(VPCond);
      VPLAN_DUMP(VPlanPrintAfterSingleTripCountOpt,
                 "single iteration optimization", Plan);
    }

  // Do the preparation for CG: create auxiliary loops and merge them into one
  // piece of CFG.
  if (VF > 1) {
    LVP.createMergerVPlans(VPAF);

    // Note, the loop is executed only when new cfg merger is enabled.
    for (const CfgMergerPlanDescr &PlanDescr : LVP.mergerVPlans()) {
      auto LpKind = PlanDescr.getLoopType();
      VPlan *Plan = PlanDescr.getVPlan();

      if (isa<VPlanVector>(Plan))
        // All-zero bypass is added after best plan selection because cost model
        // tuning is not yet implemented and we don't want to prevent
        // vectorization.
        LVP.insertAllZeroBypasses(cast<VPlanVector>(Plan), PlanDescr.getVF());

      // For unroller, we only want to pass the main-vector, i.e., the unmasked
      // vector loop.
      if (LpKind == CfgMergerPlanDescr::LoopType::LTMain)
        if (auto *NonMaskedVPlan = dyn_cast<VPlanNonMasked>(Plan))
          LVP.unroll(*NonMaskedVPlan);

      // Transform SOA-GEPs.
      // Do this transformation only for Masked and Non-masked, i.e.,
      // vector-loops.
      if (isa<VPlanVector>(Plan))
        if (EnableSOAAnalysis) {
          VPMemRefTransform VPMemRefTrans(*cast<VPlanVector>(Plan));
          VPMemRefTrans.transformSOAGEPs(PlanDescr.getVF());
        }
    }
    LVP.emitPeelRemainderVPLoops(VF, UF);
  }

  if (DisableCodeGen)
    return false;

  if (VF == 1) {
    // Emit opt report remark if a VPlan candidate SIMD loop was not vectorized.
    // TODO: Emit reason for bailing out.
    VPlanOptReportBuilder(ORBuilder, LI)
        .addRemark(Lp, OptReportVerbosity::Medium, 15436, "");
    return false;
  }

  LLVM_DEBUG(dbgs() << "VD: VPlan Generating code in function: " << Fn.getName()
                    << "\n");

  VPOCodeGen VCodeGen(Lp, Fn.getContext(), PSE, LI, DT, TLI, VF, UF, &LVL,
                      &VLSA, Plan, isOmpSIMDLoop, FatalErrorHandler);
  VCodeGen.initOpenCLScalarSelectSet(volcanoScalarSelect);

  // Run VLS analysis before IR for the current loop is modified.
  VCodeGen.getVLS()->getOVLSMemrefs(Plan, VF);
  applyVLSTransform(*Plan, VLSA, VF);

  // Transform SOA-GEPs.
  if (!EnableNewCFGMerge && EnableSOAAnalysis) {
    VPMemRefTransform VPMemRefTrans(*cast<VPlanVector>(Plan));
    VPMemRefTrans.transformSOAGEPs(VF);
  }

  LVP.executeBestPlan(VCodeGen);

  // Strip the directives once the loop is vectorized. In stress testing,
  // WRLp is null and no directives need deletion.
  if (WRLp)
    VPOUtils::stripDirectives(WRLp);

  CandLoopsVectorized++;
  VPlanOptReportBuilder VPORBuilder(ORBuilder, LI);
  addOptReportRemarks<VPOCodeGen>(WRLp, VPORBuilder, &VCodeGen);

  // Mark source and vector and scalar loops with isvectorized directive so that
  // WarnMissedTransforms pass will not complain that vector and scalar loops
  // are not vectorized
  if (isOmpSIMDLoop) {
    setLoopMD(VCodeGen.getMainLoop(), "llvm.loop.isvectorized");
    setLoopMD(VCodeGen.getOrigLoop(), "llvm.loop.isvectorized");
  }

  // Emit kernel optimization remarks.
  if (isEmitKernelOptRemarks) {
    // TODO: Collect remarks about Gather/Scatter counts during CG itself using
    // VPlanOptReportBuilder framework.
    unsigned GatherCount = 0;
    unsigned ScatterCount = 0;
    for (auto Block : VCodeGen.getMainLoop()->getBlocks())
      if (auto BB = dyn_cast<BasicBlock>(Block))
        for (auto &Inst : *BB)
          if (auto IntrinInst = dyn_cast<IntrinsicInst>(&Inst)) {
            Intrinsic::ID ID = IntrinInst->getIntrinsicID();
            if (ID == Intrinsic::masked_gather)
              GatherCount++;
            if (ID == Intrinsic::masked_scatter)
              ScatterCount++;
          }

    OptimizationRemark R("VPlan Vectorization", "Vectorized", &Fn);
    if (VectorVariant::isVectorVariant(Fn.getName()))
      R << ore::NV("Remark",
                   "Kernel was " + Twine(VF).str() + "-way vectorized");
    else
      R << ore::NV("Remark", "Loop was " + Twine(VF).str() + "-way vectorized");
    if (GatherCount > 0)
      R << ore::NV("Remark", Twine(GatherCount).str() + " gathers");
    if (ScatterCount > 0)
      R << ore::NV("Remark", Twine(ScatterCount).str() + " scatters");

    OptimizationRemarkEmitter &ORE = WR->getORE();
    ORE.emit(R);
  }

  return true;
}

#if INTEL_CUSTOMIZATION
template <>
bool VPlanDriverImpl::processLoop<vpo::HLLoop>(vpo::HLLoop *Lp, Function &Fn,
                                               WRNVecLoopNode *WRLp) {
  auto *Self = static_cast<VPlanDriverHIRImpl *>(this);
  return Self->processLoop(Lp, Fn, WRLp);
}
#endif // INTEL_CUSTOMIZATION

// Return true if this loop is supported in VPlan
template <> bool VPlanDriverImpl::isSupported<llvm::Loop>(Loop *Lp) {
  // TODO: Ensure this is true for the new pass manager. Currently, vplan-driver
  // isn't added to the pass manager at all. Once it's done there would be three
  // options probably:
  //   1) "Conventional" LCSSA pass in the pass manager builder before VPlan
  //   2) Explicit LCSSA run inside the VPlanPass itself
  //   3) Analogue of FunctionToLoopPassAdaptor that will ensure required passes
  //      are run before VPlan
  assert(Lp->isRecursivelyLCSSAForm(*DT, *LI) && "Loop is not in LCSSA form!");

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
template <> bool VPlanDriverImpl::isSupported<vpo::HLLoop>(vpo::HLLoop *Lp) {
  auto *Self = static_cast<VPlanDriverHIRImpl *>(this);
  return Self->isSupported(Lp);
}
#endif // INTEL_CUSTOMIZATION

/// Standard Mode: standard path for (TODO: automatic and) explicit
/// vectorization.
/// Explicit vectorization: it uses WRegion analysis to collect and vectorize
/// all the WRNVecLoopNode's.
#if INTEL_CUSTOMIZATION
template <typename Loop>
#endif // INTEL_CUSTOMIZATION
bool VPlanDriverImpl::runStandardMode(Function &Fn) {

  LLVM_DEBUG(dbgs() << "VD: Stardard Vectorization mode\n");

  isEmitKernelOptRemarks = true;

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

#if INTEL_CUSTOMIZATION
// We recalculate LoopInfo after each loop processed (see processLoop) so can't
// use Loop* from WRNVecLoopNode as in the default implementation. Make an
// explicit specialization to workaround that fact.
//
// Note that We have an issue with WRNVecLoopNode storing the stale Loop after
// our LoopInfo recompute. It doesn't seem to cause any issue for now but might
// be a source for some hidden bugs. Anyway, proper on-the-fly LoopInfo update
// is a long-term solution, the below is simply "good enough" workaround for
// now.
template <>
bool VPlanDriverImpl::runStandardMode<llvm::Loop>(Function &Fn) {

  LLVM_DEBUG(dbgs() << "VD: Stardard Vectorization mode\n");

  isEmitKernelOptRemarks = true;

  IRKind IR = IRKind::LLVMIR;
  WR->buildWRGraph(IR);
  WRContainerImpl *WRGraph = WR->getWRGraph();

  LLVM_DEBUG(dbgs() << "WD: WRGraph #nodes= " << WRGraph->size() << "\n");
  SmallVector<std::pair<BasicBlock *, WRNVecLoopNode *>, 8> LoopsToVectorize;
  for (auto WRNode : *WRGraph) {
    if (WRNVecLoopNode *WRLp = dyn_cast<WRNVecLoopNode>(WRNode)) {
      Loop *Lp = WRLp->getTheLoop<Loop>();

      if (!Lp) {
        LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop was optimized out.\n");
        continue;
      }

      if (!VPlanForceBuild && !isSupported(Lp)) {
        LLVM_DEBUG(dbgs() << "Bailing out: Loop is not supported!\n");
        continue;
      }

      LoopsToVectorize.emplace_back(Lp->getHeader(), WRLp);
    }
  }

  bool ModifiedFunc = false;
  for (auto It : LoopsToVectorize) {
    Loop *Lp = LI->getLoopFor(It.first);
    LLVM_DEBUG(dbgs() << "VD: Starting VPlan for \n");
    LLVM_DEBUG(It.second->dump());

    ModifiedFunc |= processLoop(Lp, Fn, It.second);
  }

  return ModifiedFunc;
}
#endif // INTEL_CUSTOMIZATION

/// Construction Stress Testing Mode: builds the H-CFG for any loop in the
/// function.
#if INTEL_CUSTOMIZATION
/// TODO: WIP for HIR.
template <typename Loop>
#endif //INTEL_CUSTOMIZATION
bool VPlanDriverImpl::runConstructStressTestMode(Function &Fn) {

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
bool VPlanDriverImpl::runCGStressTestMode(Function &Fn) {

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

#if INTEL_CUSTOMIZATION
// Common LLVM-IR/HIR high-level implementation to process a function. It gets
// LLVM-IR-HIR common analyses and choose an execution mode.
template <typename Loop>
#endif // INTEL_CUSTOMIZATION
bool VPlanDriverImpl::processFunction(Function &Fn) {

  // We cannot rely on compiler driver not invoking vectorizer for
  // non-vector targets. Ensure vectorizer won't cause any issues for
  // such targets.
  if (!TTI->getNumberOfRegisters(TTI->getRegisterClassForType(true)))
    return false;

  DL = &Fn.getParent()->getDataLayout();
  isEmitKernelOptRemarks = false;

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

/// Function to add vectorization related remarks for loops created by given
/// codegen object \p VCodeGen
#if INTEL_CUSTOMIZATION
// TODO: Change VPOCodeGenType. This cannot be used in the open sourcing patches
template <class VPOCodeGenType, typename Loop>
#else
template <class VPOCodeGenType>
#endif //INTEL_CUSTOMIZATION
void VPlanDriverImpl::addOptReportRemarks(WRNVecLoopNode *WRLp,
                                          VPlanOptReportBuilder &VPORBuilder,
                                          VPOCodeGenType *VCodeGen) {
  if ((!WRLp || (WRLp->isOmpSIMDLoop() && WRLp->getSafelen() == 0 &&
      WRLp->getSimdlen() == 0)) && (TTI->getRegisterBitWidth
      (TargetTransformInfo::RGK_FixedWidthVector) <= 256) &&
      TTI->isAdvancedOptEnabled(
      TTI::AdvancedOptLevel::AO_TargetHasIntelAVX512))
    // 15569 remark is "Compiler has chosen to target XMM/YMM vector."
    // "Try using -mprefer-vector-width=512 to override."
    VPORBuilder.addRemark(VCodeGen->getMainLoop(),
                          OptReportVerbosity::High, 15569);
  // The new vectorized loop is stored in MainLoop
  Loop *MainLoop = VCodeGen->getMainLoop();
  // Adds remark LOOP WAS VECTORIZED
  VPORBuilder.addRemark(MainLoop, OptReportVerbosity::Low, 15300);
  // Add remark about VF
  VPORBuilder.addRemark(MainLoop, OptReportVerbosity::Low, 15305,
                        Twine(VCodeGen->getVF()).str());
  // Add remark about UF
  if (VCodeGen->getUF() > 1)
    VPORBuilder.addRemark(MainLoop, OptReportVerbosity::Low, 15399,
                          Twine(VCodeGen->getUF()).str());
  VCodeGen->getOptReportStatsTracker().emitRemarks(VPORBuilder, MainLoop);

  // If remainder loop was generated for MainLoop, report that it is currently
  // not vectorized
  if (VCodeGen->getNeedRemainderLoop()) {
    Loop *RemLoop = VCodeGen->getRemainderLoop();
    VPORBuilder.addRemark(RemLoop, OptReportVerbosity::Medium, 15441, "");
  }
}

void VPlanDriverImpl::populateVPlanAnalyses(LoopVectorizationPlanner &LVP,
                                            VPAnalysesFactoryBase &VPAF) {
  for (auto &Pair : LVP.getAllVPlans()) {
    auto &Plan = *Pair.second.MainPlan;
    if (!Plan.getVPSE())
      Plan.setVPSE(VPAF.createVPSE());
    if (!Plan.getVPVT())
      Plan.setVPVT(VPAF.createVPVT(Plan.getVPSE()));
  }
}

// Definitions of addRemark functions in VPlanOptReportBuilder
template <typename... Args>
void VPlanOptReportBuilder::addRemark(HLLoop *Lp,
                                      OptReportVerbosity::Level Verbosity,
                                      unsigned MsgID, Args &&...args) {
  ORBuilder(*Lp).addRemark(Verbosity, MsgID, std::forward<Args>(args)...);
}

template <typename... Args>
void VPlanOptReportBuilder::addRemark(Loop *Lp,
                                      OptReportVerbosity::Level Verbosity,
                                      unsigned MsgID, Args &&...args) {
  // For LLVM-IR Loop, LORB needs a valid LoopInfo object
  assert(LI && "LoopInfo for opt-report builder is null.");
  ORBuilder(*Lp, *LI).addRemark(Verbosity, MsgID, std::forward<Args>(args)...);
}

INITIALIZE_PASS_BEGIN(VPlanDriver, "vplan-vec", "VPlan Vectorizer",
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
INITIALIZE_PASS_DEPENDENCY(LCSSAWrapperPass)
INITIALIZE_PASS_END(VPlanDriver, "vplan-vec", "VPlan Vectorizer",
                    false, false)

char VPlanDriver::ID = 0;

#if INTEL_CUSTOMIZATION
VPlanDriver::VPlanDriver(FatalErrorHandlerTy FatalErrorHandler)
    : FunctionPass(ID), FatalErrorHandler(FatalErrorHandler) {
  initializeVPlanDriverPass(*PassRegistry::getPassRegistry());
}
#endif // INTEL_CUSTOMIZATION

Pass *llvm::createVPlanDriverPass(FatalErrorHandlerTy FatalErrorHandler) {
  return new VPlanDriver(FatalErrorHandler);
}

void VPlanDriver::getAnalysisUsage(AnalysisUsage &AU) const {

#if INTEL_CUSTOMIZATION
  // TODO (CMPLRS-44750): We do not preserve LoopInfo as we remove loops, create
  // new loops. Same holds for Scalar Evolution which needs to be computed for
  // newly created loops.

  // TODO (CMPLRS-44750): Preserve analyses.
#endif // INTEL_CUSTOMIZATION
  AU.addRequiredID(LCSSAID);
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

static std::string getDescription(const Function &F) {
  return "function (" + F.getName().str() + ")";
}

bool VPlanDriver::skipFunction(const Function &F) const {
  OptPassGate &Gate = F.getContext().getOptPassGate();
  if (Gate.isEnabled() && !Gate.shouldRunPass(this, getDescription(F)))
    return true;

  bool IsOmpSimdKernel = (F.getMetadata("omp_simd_kernel") != nullptr);
  if (F.hasOptNone() && !IsOmpSimdKernel &&
      VPOAnalysisUtils::skipFunctionForOpenmp(const_cast<Function &>(F))) {
    LLVM_DEBUG(dbgs() << "Skipping pass '" << getPassName() << "' on function "
                      << F.getName() << "\n");
    return true;
  }
  return false;
}

bool VPlanDriver::runOnFunction(Function &Fn) {
  if (skipFunction(Fn))
    return false;

  auto SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  auto DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  // TODO: LI shouldn't be necessary as we are building VPLoopInfo. Maybe only
  // for debug/stress testing.
  auto LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  auto AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(Fn);

  auto AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  auto DB = &getAnalysis<DemandedBitsWrapperPass>().getDemandedBits();
  auto ORE = &getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();
  auto *LAA = &getAnalysis<LoopAccessLegacyAnalysis>();
  auto GetLAA = [&](Loop &L) -> const LoopAccessInfo & {
    return LAA->getInfo(&L);
  };
  auto Verbosity = getAnalysis<OptReportOptionsPass>().getVerbosity();
  auto TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(Fn);
  auto TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(Fn);
  auto WR = &getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();

  return Impl.runImpl(Fn, LI, SE, DT, AC, AA, DB, GetLAA, ORE, Verbosity, WR,
                      TTI, TLI, nullptr, nullptr, FatalErrorHandler);
}

PreservedAnalyses VPlanDriverPass::run(Function &F,
                                       FunctionAnalysisManager &AM) {

  auto SE = &AM.getResult<ScalarEvolutionAnalysis>(F);
  auto DT = &AM.getResult<DominatorTreeAnalysis>(F);
  // TODO: LI shouldn't be necessary as we are building VPLoopInfo. Maybe only
  // for debug/stress testing.
  auto LI = &AM.getResult<LoopAnalysis>(F);
  auto AC = &AM.getResult<AssumptionAnalysis>(F);

  auto AA = &AM.getResult<AAManager>(F);
  auto DB = &AM.getResult<DemandedBitsAnalysis>(F);
  auto ORE = &AM.getResult<OptimizationRemarkEmitterAnalysis>(F);
  auto Verbosity = AM.getResult<OptReportOptionsAnalysis>(F).getVerbosity();
  auto TTI = &AM.getResult<TargetIRAnalysis>(F);
  auto TLI = &AM.getResult<TargetLibraryAnalysis>(F);
  auto WR = &AM.getResult<WRegionInfoAnalysis>(F);
  auto BFI = &AM.getResult<BlockFrequencyAnalysis>(F);
  auto &LAM = AM.getResult<LoopAnalysisManagerFunctionProxy>(F).getManager();
  auto GetLAA = [&](Loop &L) -> const LoopAccessInfo & {
    LoopStandardAnalysisResults AR = {*AA,  *AC,  *DT, *LI,    *SE,
                                      *TLI, *TTI, BFI, nullptr /* MemorySSA */};
    return LAM.getResult<LoopAccessAnalysis>(L, AR);
  };

  if (!Impl.runImpl(F, LI, SE, DT, AC, AA, DB, GetLAA, ORE, Verbosity, WR, TTI,
                    TLI, nullptr, nullptr, nullptr))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses::none();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  return PA;
}

bool VPlanDriverImpl::runImpl(
    Function &Fn, LoopInfo *LI, ScalarEvolution *SE, DominatorTree *DT,
    AssumptionCache *AC, AliasAnalysis *AA, DemandedBits *DB,
    std::function<const LoopAccessInfo &(Loop &)> GetLAA,
    OptimizationRemarkEmitter *ORE, OptReportVerbosity::Level Verbosity,
    WRegionInfo *WR, TargetTransformInfo *TTI, TargetLibraryInfo *TLI,
    BlockFrequencyInfo *BFI, ProfileSummaryInfo *PSI,
    FatalErrorHandlerTy FatalErrorHandler) {

#if INTEL_CUSTOMIZATION
  LLVM_DEBUG(dbgs() << "VPlan LLVM-IR Driver for Function: " << Fn.getName()
                    << "\n");
#endif // INTEL_CUSTOMIZATION

  this->SE = SE;
  this->DT = DT;
  this->LI = LI;
  this->AC = AC;
  this->AA = AA;
  this->DB = DB;
  this->GetLAA = &GetLAA;
  this->ORE = ORE;
  this->TTI = TTI;
  this->TLI = TLI;
  this->WR = WR;
  this->FatalErrorHandler = FatalErrorHandler;

  ORBuilder.setup(Fn.getContext(), Verbosity);
  bool ModifiedFunc = processFunction(Fn);

  return ModifiedFunc;
}

namespace llvm {
namespace vpo {

#if INTEL_CUSTOMIZATION
template <>
void VPlanDriverImpl::collectAllLoops<llvm::Loop>(
    SmallVectorImpl<Loop *> &Loops) {
#else  // INTEL_CUSTOMIZATION
void VPlanDriverImpl::collectAllLoops(SmallVectorImpl<Loop *> &Loops) {
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
void VPlanDriverImpl::collectAllLoops<vpo::HLLoop>(
    SmallVectorImpl<vpo::HLLoop *> &Loops) {
  auto *Self = static_cast<VPlanDriverHIRImpl *>(this);
  return Self->collectAllLoops(Loops);
}
#endif // INTEL_CUSTOMIZATION
} // namespace vpo
} // namespace llvm

#if INTEL_CUSTOMIZATION
INITIALIZE_PASS_BEGIN(VPlanDriverHIR, "hir-vplan-vec",
                      "VPlan HIR Vectorizer", false, false)
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
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(VPlanDriverHIR, "hir-vplan-vec",
                    "VPlan HIR Vectorizer", false, false)

char VPlanDriverHIR::ID = 0;

VPlanDriverHIR::VPlanDriverHIR(bool LightWeightMode) :
  FunctionPass(ID), Impl(LightWeightMode) {
  initializeVPlanDriverHIRPass(*PassRegistry::getPassRegistry());
}

Pass *llvm::createVPlanDriverHIRPass(bool LightWeightMode) {
  return new VPlanDriverHIR(LightWeightMode);
}

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
  AU.addRequired<AssumptionCacheTracker>();
  AU.addRequired<DominatorTreeWrapperPass>();
}

bool VPlanDriverHIR::runOnFunction(Function &Fn) {
  if (skipFunction(Fn))
    return false;

  auto HIRF = &getAnalysis<HIRFrameworkWrapperPass>().getHIR();
  auto HIRLoopStats = &getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS();
  auto DDA = &getAnalysis<HIRDDAnalysisWrapperPass>().getDDA();
  auto Verbosity = getAnalysis<OptReportOptionsPass>().getVerbosity();
  auto SafeRedAnalysis =
      &getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR();
  auto TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(Fn);
  auto TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(Fn);
  auto WR = &getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
  auto AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(Fn);
  auto DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  return Impl.runImpl(Fn, HIRF, HIRLoopStats, DDA, SafeRedAnalysis, Verbosity,
                      WR, TTI, TLI, AC, DT, nullptr);
}

PreservedAnalyses VPlanDriverHIRPass::runImpl(Function &F,
                                              FunctionAnalysisManager &AM,
                                              loopopt::HIRFramework &HIRF) {
  auto HIRLoopStats = &AM.getResult<HIRLoopStatisticsAnalysis>(F);
  auto DDA = &AM.getResult<HIRDDAnalysisPass>(F);
  auto Verbosity = AM.getResult<OptReportOptionsAnalysis>(F).getVerbosity();
  auto SafeRedAnalysis = &AM.getResult<HIRSafeReductionAnalysisPass>(F);
  auto TTI = &AM.getResult<TargetIRAnalysis>(F);
  auto TLI = &AM.getResult<TargetLibraryAnalysis>(F);
  auto WR = &AM.getResult<WRegionInfoAnalysis>(F);
  auto AC = &AM.getResult<AssumptionAnalysis>(F);
  auto DT = &AM.getResult<DominatorTreeAnalysis>(F);

  Impl.runImpl(F, &HIRF, HIRLoopStats, DDA, SafeRedAnalysis, Verbosity, WR, TTI,
               TLI, AC, DT, nullptr);
  return PreservedAnalyses::all();
}

bool VPlanDriverHIRImpl::runImpl(
    Function &Fn, loopopt::HIRFramework *HIRF,
    loopopt::HIRLoopStatistics *HIRLoopStats, loopopt::HIRDDAnalysis *DDA,
    loopopt::HIRSafeReductionAnalysis *SafeRedAnalysis,
    OptReportVerbosity::Level Verbosity, WRegionInfo *WR,
    TargetTransformInfo *TTI, TargetLibraryInfo *TLI, AssumptionCache *AC,
    DominatorTree *DT, FatalErrorHandlerTy FatalErrorHandler) {
  LLVM_DEBUG(dbgs() << "VPlan HIR Driver for Function: " << Fn.getName()
                    << "\n");
  this->HIRF = HIRF;
  this->HIRLoopStats = HIRLoopStats;
  this->DDA = DDA;
  this->SafeRedAnalysis = SafeRedAnalysis;
  this->TTI = TTI;
  this->TLI = TLI;
  this->WR = WR;
  this->setAC(AC);
  this->setDT(DT);

  ORBuilder.setup(Fn.getContext(), Verbosity);
  return VPlanDriverImpl::processFunction<loopopt::HLLoop>(Fn);
}

bool VPlanDriverHIRImpl::processLoop(HLLoop *Lp, Function &Fn,
                                     WRNVecLoopNode *WRLp) {
  // Enable peeling for HIR path from command line switch
  VPlanEnablePeeling = VPlanEnablePeelingHIROpt;

  // TODO: How do we allow stress-testing for HIR path?
  assert(WRLp && "WRLp should be non-null!");

  HLLoop *HLoop = WRLp->getTheLoop<HLLoop>();
  (void)HLoop;
  assert(HLoop && "Expected HIR Loop.");
  assert(HLoop->getParentRegion() && "Expected parent HLRegion.");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintHIRBeforeVPlan) {
    dbgs() << "Candidate HLLoop before VPlan (" << Fn.getName() << "):\n";
    HLoop->dump();
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  const bool isOmpSIMDLoop = WRLp->isOmpSIMDLoop();
  if (isOmpSIMDLoop && !WRLp->isValidHIRSIMDRegion()) {
//#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
//    assert(false && "VPlan: Invalid HIR SIMD region for given loop");
//#else
    WithColor::warning() << "Loop was not vectorized. Invalid SIMD region "
                            "detected for given loop\n";
    return false;
//#endif
  }

  // If region has SIMD directive mark then mark source loop with SIMD directive
  // so that WarnMissedTransforms pass will detect that this loop is not
  // vectorized later
  if (isOmpSIMDLoop)
    setHLLoopMD(Lp, "llvm.loop.vectorize.enable", Fn.getContext());

  // Create a VPlanOptReportBuilder object, lifetime is a single loop that we
  // process for vectorization
  VPlanOptReportBuilder VPORBuilder(ORBuilder);

  VPlanVLSAnalysisHIR VLSA(DDA, Fn.getContext(), *DL, TTI);

  HIRVectorizationLegality HIRVecLegal(TTI, SafeRedAnalysis, DDA);
  LoopVectorizationPlannerHIR LVP(WRLp, Lp, TLI, TTI, DL, &HIRVecLegal, DDA,
                                  &VLSA, LightWeightMode);

  // Send explicit data from WRLoop to the Legality.
  LVP.EnterExplicitData(WRLp, HIRVecLegal);
  if (HIRVecLegal.hasF90DopeVectorPrivate()) {
    LLVM_DEBUG(dbgs() << "F90 dope vector privates are not supported\n");
    return false;
  }
  // Find any DDRefs in loop pre-header that are aliases to the descriptor
  // variables
  HIRVecLegal.findAliasDDRefs(WRLp->getEntryHLNode(), HLoop);
  std::string VPlanName = "";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Lp->getNumber() is not used here because it returns HLNode number, not
  // just HLLoop number, thus it may be unstable to be captured in lit tests.
  VPlanName = std::string(Fn.getName()) + ":HIR";
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  LVP.readLoopMetadata();
  if (!LVP.buildInitialVPlans(&Fn.getContext(), DL, VPlanName)) {
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: No VPlans constructed.\n");
    // Erase intrinsics before and after the loop if this loop is an auto
    // vectorization candidate.
    if (WRLp->getIsAutoVec())
      eraseLoopIntrins(Lp, WRLp);
    return false;
  }

  VPAnalysesFactoryHIR VPAF(Lp, getDT(), getAC(), DL);
  populateVPlanAnalyses(LVP, VPAF);

  // VPlan construction stress test ends here.
  // TODO: Move after predication.
  if (VPlanConstrStressTest)
    return false;

  // VPlan Predicator
  LVP.predicate();

  // It is enough to update the VPlan for VF=1 since all the different vector
  // factors share the same VPlan.
  assert(std::equal(std::next(LVP.getAllVPlans().begin()),
                    LVP.getAllVPlans().end(), LVP.getAllVPlans().begin(),
                    [](const auto &P1, const auto &P2) {
                      return P1.second.MainPlan.get() ==
                             P2.second.MainPlan.get();
                    }) &&
         "All the VPlans with different vector factor are expected to be the "
         "same.");
  if (!processVConflictIdiom(*LVP.getAllVPlans().begin()->second.MainPlan.get(),
                             Fn)) {
    LLVM_DEBUG(dbgs() << "VConflict idiom is not supported.\n");
    return false;
  }

  // TODO: don't force vectorization if getIsAutoVec() is set to true.
  unsigned VF;
  VPlanVector *Plan;
  std::tie(VF, Plan) = LVP.selectBestPlan();
  assert(Plan && "Unexpected null VPlan");

  // Set the final name for this initial VPlan.
  std::string PlanName;
  raw_string_ostream RSO(PlanName);
  RSO << "Initial VPlan for VF=" << VF;
  RSO.flush();
  Plan->setName(PlanName);

  LLVM_DEBUG(dbgs() << "VD:\n" << *Plan);

  VPLAN_DUMP(VPlanPrintInit,
             "initial VPlan for VF=" + std::to_string(VF), Plan);

  // If new CFG merger is not enabled, run AZB at this point in pipeline.
  if (!EnableNewCFGMerge || !EnableNewCFGMergeHIR) {
    LVP.insertAllZeroBypasses(Plan, VF);
  }

  unsigned UF = LVP.getLoopUnrollFactor();

  // Start preparations to generate auxiliary loops.
  if (VF > 1) {
    LVP.createMergerVPlans(VPAF);

    // Run some VPlan-to-VPlan transforms for each new auxiliary loop created by
    // CFGMerger.
    for (const CfgMergerPlanDescr &PlanDescr : LVP.mergerVPlans()) {
      VPlan *Plan = PlanDescr.getVPlan();

      if (isa<VPlanVector>(Plan)) {
        // All-zero bypass is added after best plan selection because cost model
        // tuning is not yet implemented and we don't want to prevent
        // vectorization.
        LVP.insertAllZeroBypasses(cast<VPlanVector>(Plan), PlanDescr.getVF());
      }

      // TODO: Unroller and SOA transform missing here.
    }

    LVP.emitPeelRemainderVPLoops(VF, UF);
  }

  bool ModifiedLoop = false;
  if (!DisableCodeGen) {
    VPLoop *OuterMostVPLoop = Plan->getMainLoop(true);
    const VPLoopEntityList *Entities =
        Plan->getLoopEntities(OuterMostVPLoop);
    RegDDRef *PeelArrayRef = nullptr;
    VPlanIdioms::Opcode SearchLoopOpcode =
        VPlanIdioms::isSearchLoop(Plan, VF, true, PeelArrayRef);
    VPOCodeGenHIR VCodeGen(TLI, TTI, SafeRedAnalysis, &VLSA, Plan, Fn, Lp,
                           ORBuilder, Entities, &HIRVecLegal, SearchLoopOpcode,
                           PeelArrayRef, isOmpSIMDLoop);
    bool LoopIsHandled = (VF != 1 && VCodeGen.loopIsHandled(Lp, VF));

    // Erase intrinsics before and after the loop if we either vectorized the
    // loop or if this loop is an auto vectorization candidate. SIMD Intrinsics
    // are left around for loops that are not vectorized.
    if (LoopIsHandled || WRLp->getIsAutoVec())
      eraseLoopIntrins(Lp, WRLp);

    if (LoopIsHandled) {
      CandLoopsVectorized++;
      // TODO: Is this placement of unroller correct? It is being run for the
      // final VPlan before CG. Potentially problematic for merged CFG.
      LVP.unroll(*cast<VPlanNonMasked>(Plan));
      if (LVP.executeBestPlan(&VCodeGen, UF)) {
        ModifiedLoop = true;
        VPlanDriverImpl::addOptReportRemarks<VPOCodeGenHIR, loopopt::HLLoop>(
            WRLp, VPORBuilder, &VCodeGen);
        // Mark source and vectorized loops with isvectorized directive so that
        // WarnMissedTransforms pass will not complain that this loop is not
        // vectorized. We also tag the main vector loop based on
        // simd/auto-vectorization scenarios. This tag will be reflected in
        // downstream HIR dumps.
        if (isOmpSIMDLoop) {
          setHLLoopMD(Lp, "llvm.loop.isvectorized", Fn.getContext());
          VCodeGen.getMainLoop()->setVecTag(HLLoop::VecTagTy::SIMD);
        } else {
          VCodeGen.getMainLoop()->setVecTag(HLLoop::VecTagTy::AUTOVEC);
        }
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

bool VPlanDriverHIRImpl::isSupported(HLLoop *Lp) {
  if (HIRLoopStats->getTotalLoopStatistics(Lp).hasSwitches())
    return false;

  // Bail out for outer loops if not enabled
  if (!EnableOuterLoopHIR && !Lp->isInnermost())
    return false;

  // Unsupported HLLoop types for vectorization
  if (!((Lp->isDo() || Lp->isDoMultiExit()) && Lp->isNormalized()))
    return false;

  return true;
}

void VPlanDriverHIRImpl::collectAllLoops(SmallVectorImpl<HLLoop *> &Loops) {
  HIRF->getHLNodeUtils().gatherAllLoops(Loops);
}

bool VPlanDriverHIRImpl::isVPlanCandidate(Function &Fn, HLLoop *Lp) {
  // This function is only used in the LLVM-IR path to generate VPlan
  // candidates.
  return false;
}

void VPlanDriverHIRImpl::eraseLoopIntrins(HLLoop *Lp,
                                          WRNVecLoopNode *WVecNode) {
  auto IsValidDirectiveNode = [](HLNode *Node, SmallSet<int, 2> &DirectiveIDs) {
    HLInst *HInst = dyn_cast<HLInst>(Node);
    if (!HInst)
      return false;

    // Entry/exit nodes are expected to be intrinsic calls.
    auto *IntrinInst = HInst->getIntrinCall();
    if (!IntrinInst)
      return false;

    // Entry/exit nodes are expected to be calls to region entry/exit
    // directives.
    int DirID = vpo::VPOAnalysisUtils::getRegionDirectiveID(IntrinInst);
    if (!DirectiveIDs.count(DirID))
      return false;

    // All checks passed.
    return true;
  };

  // List of begin/end directives IDs that must be removed.
  SmallSet<int, 2> BeginOrEndDirIDs;

  // 1. Remove entry directive node.
  auto BeginNode = WVecNode->getEntryHLNode();
  assert(BeginNode && "Unexpected null entry node in WRNVecLoopNode");
  BeginOrEndDirIDs.insert(DIR_OMP_SIMD);
  BeginOrEndDirIDs.insert(DIR_VPO_AUTO_VEC);
  assert(IsValidDirectiveNode(BeginNode, BeginOrEndDirIDs) &&
         "Unexpected entry node for WRNVecLoopNode");
  HLNodeUtils::remove(BeginNode);

  // 2. Remove exit directive node.
  BeginOrEndDirIDs.clear();
  BeginOrEndDirIDs.insert(DIR_OMP_END_SIMD);
  BeginOrEndDirIDs.insert(DIR_VPO_END_AUTO_VEC);
  auto ExitNode = WVecNode->getExitHLNode();
  assert(ExitNode && "Unexpected null exit node in WRNVecLoopNode");
  assert(IsValidDirectiveNode(ExitNode, BeginOrEndDirIDs) &&
         "Unexpected exit node for WRNVecLoopNode");
  HLNodeUtils::remove(ExitNode);

  (void)IsValidDirectiveNode;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Overrides FunctionPass's printer pass to return one which prints
/// HIR instead of LLVM IR.
FunctionPass *
VPlanDriverHIR::createPrinterPass(raw_ostream &OS,
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
bool VPlanDriverImpl::isVPlanCandidate<llvm::Loop>(Function &Fn, Loop *Lp) {
#else // INTEL_CUSTOMIZATION
bool VPlanDriverImpl::isVPlanCandidate(Function &Fn, Loop *Lp) {
#endif
  // Only consider inner loops
  if (!Lp->isInnermost())
    return false;

  PredicatedScalarEvolution PSE(*SE, *Lp);
  LoopVectorizationRequirements Requirements;
  LoopVectorizeHints Hints(Lp, true, *ORE);
  LoopVectorizationLegality LVL(Lp, PSE, DT, TTI, TLI, AA, &Fn, GetLAA, LI, ORE,
                                &Requirements, &Hints, DB, AC, BFI, PSI);

  if (!LVL.canVectorize(false /* EnableVPlanNativePath */))
    return false;

  // No induction - bail out for now
  if (!LVL.getPrimaryInduction())
    return false;

  // Bail out if any runtime checks are needed
  auto LAI = &(*GetLAA)(*Lp);
  if (LAI->getNumRuntimePointerChecks())
    return false;

  return true;
}

#if INTEL_CUSTOMIZATION
template <>
bool VPlanDriverImpl::isVPlanCandidate<vpo::HLLoop>(Function &Fn,
                                                    vpo::HLLoop *Lp) {
  auto *Self = static_cast<VPlanDriverHIRImpl *>(this);
  return Self->isVPlanCandidate(Fn, Lp);
}
#endif // INTEL_CUSTOMIZATION

} // namespace vpo
} // namespace llvm
