//===-- DriverLLVM.cpp ----------------------------------------------------===//
//
//   INTEL CONFIDENTIAL
//
//   Copyright (C) 2015 Intel Corporation
//
//   This software and the related documents are Intel copyrighted materials,
//   and your use of them is governed by the express license under which they
//   were provided to you ("License").  Unless the License provides otherwise,
//   you may not use, modify, copy, publish, distribute, disclose or treansmit
//   this software or the related documents without Intel's prior written
//   permission.
//
//   This software and the related documents are provided as is, with no
//   express or implied warranties, other than those that are expressly
//   stated in the License.
//
//===----------------------------------------------------------------------===//
//
/// \file DriverLLVM.cpp
/// This file implements the VPlan vectorizer driver pass for LLVM IR.
///
/// Split from Driver.cpp on 2023-10-26.
//
//===----------------------------------------------------------------------===//

#include "DriverLLVM.h"
#include "../IntelVPAlignAssumeCleanup.h"
#include "../IntelVPMemRefTransform.h"
#include "../IntelVPlanCostModel.h"
#include "../IntelVPlanMaskedModeLoop.h"
#include "../IntelVolcanoOpenCL.h"
#include "../LLVM/CodeGenLLVM.h"
#include "LegalityLLVM.h"
#include "llvm/Analysis/DemandedBits.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Vectorize/IntelVPlanDriverPass.h"

#define DEBUG_TYPE "VPlanDriver"

using namespace llvm;
using namespace llvm::vpo;

// The interface getUniqueExitBlock() asserts that the loop has dedicated
// exits. Check that a loop has dedicated exits before the check for unique
// exit block. This is especially needed when stress testing VPlan builds.
bool DriverLLVMImpl::hasDedicatedAndUniqueExits(Loop *Lp,
                                                WRNVecLoopNode *WRLp) {

  if (!Lp->hasDedicatedExits()) {
    LLVM_DEBUG(dbgs() << "VD: loop form "
                      << "(" << Lp->getName()
                      << ") is not supported: no dedicated exits.\n");
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailGenericBailout,
                     AuxRemarkID::NoDedicatedExits);
    return false;
  }

  if (!Lp->getUniqueExitBlock()) {
    LLVM_DEBUG(dbgs() << "VD: loop form "
                      << "(" << Lp->getName()
                      << ") is not supported: multiple exit blocks.\n");
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailBadlyFormedMultiExitLoop,
                     WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                                   : AuxRemarkID::Loop);
    return false;
  }
  return true;
}

// Auxiliary function that checks only loop-specific constraints. Generic loop
// nest constraints are in 'isSupported' function.
bool DriverLLVMImpl::isSupportedRec(Loop *Lp, WRNVecLoopNode *WRLp) {

  if (!LoopMassagingEnabled && !hasDedicatedAndUniqueExits(Lp, WRLp))
    return false;

  for (Loop *SubLoop : Lp->getSubLoops()) {
    if (!isSupportedRec(SubLoop, WRLp))
      return false;
  }

  return true;
}

bool DriverLLVMImpl::processLoop(Loop *Lp, Function &Fn, WRNVecLoopNode *WRLp) {
  // Enable peeling for LLVM-IR path from command line switch
  VPlanEnablePeeling = VPlanEnablePeelingOpt;
  VPlanEnableGeneralPeeling = VPlanEnableGeneralPeelingOpt;

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
  LegalityLLVM LVL(Lp, PSE, &Fn, &Fn.getContext());
  VPlanOptReportBuilder VPORBuilder(ORBuilder, LI);

  // If region has SIMD directive mark then we will reuse community metadata on
  // Loop so that WarnMissedTransforms pass will detect if this loop is not
  // vectorized later
  const bool isOmpSIMDLoop = WRLp && WRLp->isOmpSIMDLoop();
  if (isOmpSIMDLoop)
    setLoopMD(Lp, "llvm.loop.vectorize.enable");

  // The function canVectorize() collects information about induction
  // and reduction variables. It also verifies that the loop vectorization
  // is fully supported.
  bool CanVectorize = LVL.canVectorize(*DT, WRLp);
  if (!CanVectorize) {
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: Cannot prove legality.\n");

    // Only bail out if we are generating code, we want to continue if
    // we are only stress testing VPlan builds below.
    if (!VPlanConstrStressTest && !DisableCodeGen) {
      auto &LVLBR = LVL.getBailoutRemark();
      assert(LVLBR.BailoutRemark && "Legality didn't set bailout data!");
      return bailout(VPORBuilder, Lp, WRLp, LVLBR);
    }
  }

  BasicBlock *Header = Lp->getHeader();
  VPlanVLSAnalysis VLSA(Lp, Header->getContext(), *DL, TTI);
  LoopVectorizationPlanner LVP(WRLp, Lp, LI, TLI, TTI, DL, DT, &LVL, &VLSA,
                               &Fn.getContext(), BFI);
  std::string VPlanName = "";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  VPlanName = std::string(Fn.getName()) + ":" + std::string(Lp->getName());
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  LVP.readLoopMetadata();
  VPAnalysesFactory VPAF(SE, Lp, DT, DL);
  if (!LVP.buildInitialVPlans(Header->getModule(), VPlanName, *AC, VPAF, &SE,
                              CanVectorize || DisableCodeGen)) {
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: No VPlans constructed.\n");
    auto &LVPBR = LVP.getBailoutRemark();
    assert(LVPBR.BailoutRemark &&
           "buildInitialVPlans did not set bailout data!");
    return bailout(VPORBuilder, Lp, WRLp, LVPBR);
  }

  populateVPlanAnalyses(LVP, VPAF);

  LVP.runPeepholeBeforePredicator();

  if (EnableMaskedVariant)
    generateMaskedModeVPlans(&LVP, &VPAF);

  // VPlan Predicator
  LVP.predicate();

#ifndef NDEBUG
  // Run verifier after predicator
  LVP.verifyAllVPlans(Lp);
#endif

  // VPlan construction stress test ends here.
  if (VPlanConstrStressTest) {
    setBailoutRemark(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Stress testing only."));
    return bailout(VPORBuilder, Lp, WRLp, BR);
  }

  assert((WRLp || VPlanVectCand) && "WRLp can be null in stress testing only!");

  // Transform masked integer div/rem before CM.
  LVP.blendWithSafeValue();

  LVP.disableNegOneStrideOptInMaskedModeVPlans();

  unsigned VF;
  VPlanVector *Plan;
  std::tie(VF, Plan) = LVP.selectBestPlan();
  assert(Plan && "Unexpected null VPlan");

  LLVM_DEBUG(std::string PlanName; raw_string_ostream RSO(PlanName);
             RSO << "VD: Initial VPlan for VF=" << VF; RSO.flush();
             Plan->setName(PlanName); dbgs() << *Plan);

  VPLAN_DUMP(VPlanPrintInit, "initial VPlan for VF=" + std::to_string(VF),
             Plan);

  VPAlignAssumeCleanup Cleanup(*Plan);
  Cleanup.transform();

  unsigned UF = LVP.getBestUF();

  // Do the preparation for CG: create auxiliary loops and merge them into one
  // piece of CFG.
  if (VF > 1) {
    LVP.createMergerVPlans(VPAF);

    const auto *PreferredPeeling = Plan->getPreferredPeeling(VF);
    // TODO: remove this hack after getGuaranteedPeeling is fixed.
    const auto *GuaranteedPeeling =
        LVP.peelWasSelected() ||
                (PreferredPeeling &&
                 (dyn_cast<VPlanNoPeelingAligned>(PreferredPeeling) ||
                  dyn_cast<VPlanNoPeelingUnaligned>(PreferredPeeling)))
            ? Plan->getGuaranteedPeeling(VF)
            : &VPlanNoPeeling::LoopObject;

    // Note, the loop is executed only when new cfg merger is enabled.
    for (const CfgMergerPlanDescr &PlanDescr : LVP.mergerVPlans()) {
      auto LpKind = PlanDescr.getLoopType();
      VPlan *Plan = PlanDescr.getVPlan();

      if (isa<VPlanVector>(Plan)) {
        // All-zero bypass is added after best plan selection because cost model
        // tuning is not yet implemented and we don't want to prevent
        // vectorization.
        LVP.insertAllZeroBypasses(cast<VPlanVector>(Plan), PlanDescr.getVF());

        // For non-peel loops, if we have a guaranteed peel, we should propagate
        // the alignment from that peel. Do so now before CG.
        if (LpKind != CfgMergerPlanDescr::LoopType::LTPeel)
          VPlanAlignmentAnalysis::propagateAlignment(cast<VPlanVector>(Plan),
                                                     TTI, PlanDescr.getVF(),
                                                     GuaranteedPeeling);
      }

      // For unroller, we only want to pass the main-vector, i.e., the unmasked
      // vector loop.
      if (LpKind == CfgMergerPlanDescr::LoopType::LTMain)
        if (auto *NonMaskedVPlan = dyn_cast<VPlanNonMasked>(Plan)) {
          LVP.unroll(*NonMaskedVPlan);
          // Workaround for kernel vectorization. Kernel vectorization is done
          // through loop creation inside vec-clone) followed by loop
          // vectorization. That leaves a loop CFG that can't be optimized away
          // (even though it will be 1-iteration loop) without scheduling
          // indvars-simplify or unroller later in the pipeline. We don't want
          // to spend compile-time on these passes so do some special casing
          // here for a vector loop that will result in exactly one iteration.
          //
          // For ahead of time calculation a better approach is to perform
          // small-trip count loop-unroll which won't be limited to exactly one
          // iteration. For kernel vectorization the long-term plan would be to
          // import the region itself into VPlan without artificial loop being
          // created at all.
          if (auto *TripCount =
                  dyn_cast<SCEVConstant>(PSE.getBackedgeTakenCount()))
            if ((VF * UF - 1) == TripCount->getAPInt()) {
              VPLoop *Lp = (*NonMaskedVPlan->getVPLoopInfo()->begin());
              VPBasicBlock *Latch = Lp->getLoopLatch();
              assert(Latch && "Latch should not be a null pointer.");
              VPBasicBlock *Header = Lp->getHeader();
              bool BackedgeOnTrue = Latch->getSuccessor(0) == Header;
              auto &Context = Fn.getContext();
              auto *Cond = BackedgeOnTrue ? ConstantInt::getFalse(Context)
                                          : ConstantInt::getTrue(Context);
              auto *VPCond = NonMaskedVPlan->getVPConstant(Cond);
              Latch->setCondBit(VPCond);
              VPLAN_DUMP(VPlanPrintAfterSingleTripCountOpt,
                         "single iteration optimization", NonMaskedVPlan);
            }
        }

      // Transform SOA-GEPs and library calls.
      // Do this transformation only for Masked and Non-masked, i.e.,
      // vector-loops.
      if (auto *VPlan = dyn_cast<VPlanVector>(Plan)) {
        if (EnableSOAAnalysis) {
          VPMemRefTransform VPMemRefTrans(*VPlan);
          VPMemRefTrans.transformSOAGEPs(PlanDescr.getVF());
        }
      }

      // Capture opt-report remarks for main VPLoop.
      if (PlanDescr.getLoopType() == CfgMergerPlanDescr::LoopType::LTMain)
        addOptReportRemarksForMainPlan(WRLp, PlanDescr);

      // Capture opt-report remarks for remainder loops.
      if (LpKind == CfgMergerPlanDescr::LoopType::LTRemainder) {
        if (isa<VPlanVector>(Plan))
          addOptReportRemarksForVecRemainder(PlanDescr);
        else if (isa<VPlanScalar>(Plan))
          addOptReportRemarksForScalRemainder(PlanDescr);
      }

      // Capture opt-report remarks for peel loops.
      if (LpKind == CfgMergerPlanDescr::LoopType::LTPeel) {
        if (isa<VPlanVector>(Plan))
          addOptReportRemarksForVecPeel(PlanDescr, PreferredPeeling);
        else if (isa<VPlanScalar>(Plan))
          addOptReportRemarksForScalPeel(PlanDescr, PreferredPeeling);
      }
    }
    LVP.emitPeelRemainderVPLoops(VF, UF);
  }

  preprocessDopeVectorInstructions(Plan);
  preprocessPrivateFinalCondInstructions(Plan);

  if (DisableCodeGen) {
    setBailoutRemark(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Code generation is disabled."));
    return bailout(VPORBuilder, Lp, WRLp, BR);
  }

  if (VF == 1 || !LVP.canLowerVPlan(*Plan, VF)) {
    auto &LVPBR = LVP.getBailoutRemark();
    assert(LVPBR.BailoutRemark && "canLowerVPlan did not set bailout data!");
    return bailout(VPORBuilder, Lp, WRLp, LVPBR);
  }

  LLVM_DEBUG(dbgs() << "VD: VPlan Generating code in function: " << Fn.getName()
                    << "\n");

  CodeGenLLVM VCodeGen(Lp, Fn.getContext(), PSE, LI, DT, TLI, TTI, VF, UF, &LVL,
                       &VLSA, Plan, ORBuilder, isOmpSIMDLoop, VecErrorHandler);
  VCodeGen.initOpenCLScalarSelectSet(volcanoScalarSelect);

  // Run VLS analysis before IR for the current loop is modified.
  VCodeGen.getVLS()->getOVLSMemrefs(Plan, VF);
  applyVLSTransform(*Plan, VLSA, VF);

#ifndef NDEBUG
  // Run verifier before code gen
  VPlanVerifier::verify(Plan, Lp);
#endif
  LVP.executeBestPlan(VCodeGen);

  // Strip the directives once the loop is vectorized. In stress testing,
  // WRLp is null and no directives need deletion.
  if (WRLp)
    VPOUtils::stripDirectives(WRLp);

  incrementCandLoopsVectorized();

  // TODO: Move this to CodeGenLLVM::finalizeLoop.
  VCodeGen.lowerVPlanOptReportRemarks();

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
    for (const auto &Block : VCodeGen.getMainLoop()->getBlocks())
      if (const auto &BB = dyn_cast<BasicBlock>(Block))
        for (auto &Inst : *BB)
          if (auto IntrinInst = dyn_cast<IntrinsicInst>(&Inst)) {
            Intrinsic::ID ID = IntrinInst->getIntrinsicID();
            if (ID == Intrinsic::masked_gather)
              GatherCount++;
            if (ID == Intrinsic::masked_scatter)
              ScatterCount++;
          }

    OptimizationRemark R("VPlan Vectorization", "Vectorized", &Fn);
    if (VFInfo::isVectorVariant(Fn.getName()))
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

// Add an opt-report remark indicating why we can't vectorize the loop.
// Returns false as a convenience to facilitate "return bailout(...);" usage.
// Bail-out reasons with messages of more interest to compiler maintainers
// than to users should be marked with verbosity High and never emitted in
// release compilers.  For these, we first emit a more generic Medium message.
bool DriverLLVMImpl::bailout(VPlanOptReportBuilder &VPORBuilder, Loop *Lp,
                             WRNVecLoopNode *WRLp,
                             VPlanBailoutRemark RemarkData) {

  OptRemarkID ID = RemarkData.BailoutRemark.getRemarkID();

  if (RemarkData.BailoutLevel == OptReportVerbosity::High &&
      ID == OptRemarkID::VecFailGenericBailout) {
    VPORBuilder.addRemark(Lp, OptReportVerbosity::Medium, ID, std::string());
#ifndef NDEBUG
    VPORBuilder.addRemark(Lp, RemarkData.BailoutLevel,
                          RemarkData.BailoutRemark);
#endif
  } else {
    VPORBuilder.addRemark(Lp, RemarkData.BailoutLevel,
                          RemarkData.BailoutRemark);
  }

  // Execute error handler
  if (VecErrorHandler)
    VecErrorHandler(Lp->getHeader()->getParent(), VecErrorKind::Bailout);

  return false;
}

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

// Return true if this loop is supported in VPlan
bool DriverLLVMImpl::isSupported(Loop *Lp, WRNVecLoopNode *WRLp) {
  // TODO: Ensure this is true for the new pass manager. Currently, vplan-driver
  // isn't added to the pass manager at all. Once it's done there would be three
  // options probably:
  //   1) "Conventional" LCSSA pass in the pass manager builder before VPlan
  //   2) Explicit LCSSA run inside the VPlanPass itself
  //   3) Analogue of FunctionToLoopPassAdaptor that will ensure required passes
  //      are run before VPlan
  assert(Lp->isRecursivelyLCSSAForm(*DT, *LI) && "Loop is not in LCSSA form!");

  if (!hasDedicatedAndUniqueExits(Lp, WRLp)) {
    assert(BR.BailoutRemark &&
           "hasDedicatedAndUniqueExits did not set bailout data!");
    return false;
  }

  // Check for loop specific constraints
  if (!isSupportedRec(Lp, WRLp)) {
    LLVM_DEBUG(dbgs() << "VD: loop nest "
                      << "(" << Lp->getName() << ") is not supported.\n");
    assert(BR.BailoutRemark && "isSupportedRec() did not set bailout data!");
    return false;
  }

  // Check generic loop nest constraints
  if (isIrreducibleCFG(Lp, LI)) {
    LLVM_DEBUG(dbgs() << "VD: loop nest "
                      << "(" << Lp->getName()
                      << ") is not supported: irreducible CFG.\n");
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailComplexControlFlow,
                     WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                                   : AuxRemarkID::Loop);
    return false;
  }

  for (BasicBlock *BB : Lp->blocks()) {
    // We don't support switch statements inside loops.
    if (!isa<BranchInst>(BB->getTerminator())) {
      LLVM_DEBUG(dbgs() << "VD: loop nest contains a switch statement.\n");
      setBailoutRemark(OptReportVerbosity::Medium,
                       OptRemarkID::VecFailSwitchPresent,
                       WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                                     : AuxRemarkID::Loop);
      return false;
    }
  }

  LLVM_DEBUG(dbgs() << "VD: loop nest "
                    << "(" << Lp->getName() << ") is supported.\n");

  return true;
}

llvm::Loop *DriverLLVMImpl::adjustLoopIfNeeded(llvm::Loop *Lp,
                                               BasicBlock *Header) {
  // We recalculate LoopInfo after each loop processed (see processLoop) so
  // can't use Loop* from WRNVecLoopNode as in the HIR path.
  //
  // Note that We have an issue with WRNVecLoopNode storing the stale Loop
  // after our LoopInfo recompute. It doesn't seem to cause any issue for
  // now but might be a source for some hidden bugs. Anyway, proper
  // on-the-fly LoopInfo update is a long-term solution, the below is simply
  // "good enough" workaround for now.
  assert(Header != nullptr);
  return LI->getLoopFor(Header);
}

bool DriverLLVMImpl::formLCSSAIfNeeded(Loop *Lp) {
  // Inner loop vectorization might cause LCSSA form breakage. For example:
  //
  //   vector.body:
  //     %vec.phi3 = phi <4 x i32> ..., [ %0, %vector.body ]
  //     ...
  //     %wide.load = load <4 x i32>, ptr %scalar.gep, align 4
  //     %0 = add <4 x i32> %vec.phi3, %wide.load
  //     ...
  //     br i1 %3, label %VPlannedBB4, label %vector.body
  //
  //   VPlannedBB4:
  //     %4 = call i32 @llvm.vector.reduce.add.v4i32(<4 x i32> %0)
  //
  // formLCSSA call changes instruction form in the latter block; after that
  // we can do vectorization of the outer loop.
  //
  //   VPlannedBB4:
  //     %.lcssa = phi <4 x i32> [ %0, %vector.body ]
  //     %4 = call i32 @llvm.vector.reduce.add.v4i32(<4 x i32> %.lcssa)
  //
  if (NestedSimdStrategy == NestedSimdStrategies::FromInside)
    return formLCSSA(*Lp, *DT, LI, SE);
  return false;
}

VPlanDriverLLVMPass::VPlanDriverLLVMPass() { Impl = new DriverLLVMImpl(); }

VPlanDriverLLVMPass::VPlanDriverLLVMPass(const VPlanDriverLLVMPass &P) noexcept
    : PassInfoMixin<VPlanDriverLLVMPass>(P) {
  Impl = new DriverLLVMImpl();
}

VPlanDriverLLVMPass::~VPlanDriverLLVMPass() { delete Impl; }

PreservedAnalyses VPlanDriverLLVMPass::run(Function &F,
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
  LoopAccessInfoManager *LAIs = &AM.getResult<LoopAccessAnalysis>(F);

  if (!Impl->runImpl(F, LI, SE, DT, AC, AA, DB, LAIs, ORE, Verbosity, WR, TTI,
                     TLI, BFI, nullptr, VecErrorHandler))
    return PreservedAnalyses::all();

  // If the function was modified, verify using the LLVM verifier
  assert(!verifyFunction(F, &dbgs()) &&
         "Function IR after VPlan failed verification");

  auto PA = PreservedAnalyses::none();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  return PA;
}

bool DriverLLVMImpl::runImpl(
    Function &Fn, LoopInfo *LI, ScalarEvolution *SE, DominatorTree *DT,
    AssumptionCache *AC, AliasAnalysis *AA, DemandedBits *DB,
    LoopAccessInfoManager *LAIs, OptimizationRemarkEmitter *ORE,
    OptReportVerbosity::Level Verbosity, WRegionInfo *WR,
    TargetTransformInfo *TTI, TargetLibraryInfo *TLI, BlockFrequencyInfo *BFI,
    ProfileSummaryInfo *PSI, VecErrorHandlerTy VecErrHandler) {

  LLVM_DEBUG(dbgs() << "VPlan LLVM-IR Driver for Function: " << Fn.getName()
                    << "\n");

  this->SE = SE;
  this->DT = DT;
  this->LI = LI;
  this->AC = AC;
  this->AA = AA;
  this->DB = DB;
  this->LAIs = LAIs;
  this->ORE = ORE;
  this->TTI = TTI;
  this->TLI = TLI;
  this->BFI = BFI;
  this->WR = WR;
  this->VecErrorHandler = VecErrHandler;

#ifndef NDEBUG
  // Debug error handler.
  if (!VecErrorHandler && DebugErrHandler) {
    auto HandlerForDebug = [](llvm::Function *F, VecErrorKind K) -> void {
      switch (K) {
      case VecErrorKind::Bailout:
        dbgs() << "Bailout signaled on " << F->getName() << "\n";
        break;
      case VecErrorKind::Fatal:
        dbgs() << "Fatal error signaled on " << F->getName() << "\n";
        break;
      }
    };
    VecErrorHandler = HandlerForDebug;
  }
#endif

  ORBuilder.setup(Fn.getContext(), Verbosity);
  bool ModifiedFunc = processFunction<llvm::Loop>(Fn);

  return ModifiedFunc;
}

void DriverLLVMImpl::collectAllLoops(SmallVectorImpl<Loop *> &Loops) {

  std::function<void(Loop *)> collectSubLoops = [&](Loop *Lp) {
    Loops.push_back(Lp);
    for (Loop *InnerLp : Lp->getSubLoops())
      collectSubLoops(InnerLp);
  };

  for (Loop *Lp : *LI)
    collectSubLoops(Lp);
}

// IMPORTANT -- keep this function at the end of the file until VPO and
// LoopVectorization legality can be merged.
#undef LoopVectorizationLegality
#include "llvm/Transforms/Vectorize/LoopVectorizationLegality.h"

namespace llvm {
namespace vpo {
bool DriverLLVMImpl::isVPlanCandidate(Function &Fn, Loop *Lp) {
  // Only consider inner loops
  if (!Lp->isInnermost())
    return false;

  if (!LAIs)
    return false;

  PredicatedScalarEvolution PSE(*SE, *Lp);
  LoopVectorizationRequirements Requirements;
  LoopVectorizeHints Hints(Lp, true, *ORE);
  LoopVectorizationLegality LVL(Lp, PSE, DT, TTI, TLI, &Fn, *LAIs, LI, ORE,
                                &Requirements, &Hints, DB, AC, BFI, PSI);

  if (!LVL.canVectorize(false /* EnableVPlanNativePath */))
    return false;

  // No induction - bail out for now
  if (!LVL.getPrimaryInduction())
    return false;

  // Bail out if any runtime checks are needed
  auto LAI = &LAIs->getInfo(*Lp);
  if (LAI->getNumRuntimePointerChecks())
    return false;

  return true;
}

} // namespace vpo
} // namespace llvm
