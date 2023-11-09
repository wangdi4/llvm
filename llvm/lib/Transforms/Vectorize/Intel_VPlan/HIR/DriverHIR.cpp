//===-- DriverHIR.cpp -----------------------------------------------------===//
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
/// \file DriverHIR.cpp
/// This file implements the VPlan vectorizer driver pass for HIR.
///
/// Split from Driver.cpp on 2023-10-26.
//
//===----------------------------------------------------------------------===//

#include "DriverHIR.h"
#include "../IntelVPAlignAssumeCleanup.h"
#include "../IntelVPlanMaskedModeLoop.h"
#include "../IntelVPlanVConflictTransformation.h"
#include "IntelLoopVectorizationPlannerHIR.h"
#include "IntelVPOCodeGenHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Transforms/Vectorize/IntelVPlanDriverPass.h"

#define DEBUG_TYPE "vplan-vec"

using namespace llvm;
using namespace llvm::vpo;

/// Function to add vectorization related remarks for loops created by given
/// codegen object \p VCodeGen
// TODO: Change VPOCodeGenType. This cannot be used in the open sourcing patches
void DriverHIRImpl::addOptReportRemarks(WRNVecLoopNode *WRLp,
                                        VPlanOptReportBuilder &VPORBuilder,
                                        VPOCodeGenHIR *VCodeGen) {
  if ((!WRLp || (WRLp->isOmpSIMDLoop() && WRLp->getSafelen() == 0 &&
                 WRLp->getSimdlen() == 0)) &&
      (TTI->getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector) <=
       256) &&
      TTI->isAdvancedOptEnabled(TTI::AdvancedOptLevel::AO_TargetHasIntelAVX512))
    // 15569 remark is "Compiler has chosen to target XMM/YMM vector."
    // "Try using -mprefer-vector-width=512 to override."
    VPORBuilder.addRemark(VCodeGen->getMainLoop(), OptReportVerbosity::High,
                          OptRemarkID::VectorizerShortVector);
  // The new vectorized loop is stored in MainLoop
  HLLoop *MainLoop = VCodeGen->getMainLoop();
  if (WRLp && WRLp->isOmpSIMDLoop())
    // Adds remark SIMD LOOP WAS VECTORIZED
    VPORBuilder.addRemark(MainLoop, OptReportVerbosity::Low,
                          OptRemarkID::SimdLoopVectorized);
  else
    // Adds remark LOOP WAS VECTORIZED
    VPORBuilder.addRemark(MainLoop, OptReportVerbosity::Low,
                          OptRemarkID::LoopVectorized);

  // Next print the loop number.
  if (ReportLoopNumber)
    VPORBuilder.addRemark(
        MainLoop, OptReportVerbosity::Low, OptRemarkID::VectorizerLoopNumber,
        Twine(LoopVectorizationPlanner::getVPlanOrderNumber()).str());

  // Add remark about VF
  VPORBuilder.addRemark(MainLoop, OptReportVerbosity::Low,
                        OptRemarkID::VectorizationFactor,
                        Twine(VCodeGen->getVF()).str());
  // Add remark about UF
  if (VCodeGen->getUF() > 1)
    VPORBuilder.addRemark(MainLoop, OptReportVerbosity::Low,
                          OptRemarkID::VectorizerUnrollFactor,
                          Twine(VCodeGen->getUF()).str());

  // VPLoop corresponding to MainLoop should be the outer most loop in CFG.
  auto *Plan = cast<VPlanVector>(VCodeGen->getPlan());
  auto *OuterMostVLp = *(Plan->getVPLoopInfo()->begin());
  VCodeGen->getPlan()
      ->getOptRptStatsForLoop(OuterMostVLp)
      .emitRemarks(VPORBuilder, MainLoop);

  // If remainder loop was generated for MainLoop, report that it is currently
  // not vectorized
  if (VCodeGen->getNeedRemainderLoop()) {
    HLLoop *RemLoop = VCodeGen->getRemainderLoop();
    VPORBuilder.addRemark(RemLoop, OptReportVerbosity::Medium,
                          OptRemarkID::UnvectorizedRemainderLoop, "");
  }
}

VPlanDriverHIRPass::VPlanDriverHIRPass(bool LightWeightMode,
                                       bool WillRunLLVMIRVPlan) {
  Impl = new DriverHIRImpl(LightWeightMode, WillRunLLVMIRVPlan);
}

VPlanDriverHIRPass::VPlanDriverHIRPass(const VPlanDriverHIRPass &P) noexcept
    : loopopt::HIRPassInfoMixin<VPlanDriverHIRPass>(P) {
  Impl = new DriverHIRImpl(P.Impl->lightWeightMode(),
                           P.Impl->willRunLLVMIRVPlan());
}

VPlanDriverHIRPass::~VPlanDriverHIRPass() { delete Impl; }

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

  ModifiedHIR = Impl->runImpl(F, &HIRF, HIRLoopStats, DDA, SafeRedAnalysis,
                              Verbosity, WR, TTI, TLI, AC, DT);
  return PreservedAnalyses::all();
}

bool DriverHIRImpl::runImpl(Function &Fn, loopopt::HIRFramework *HIRF,
                            loopopt::HIRLoopStatistics *HIRLoopStats,
                            loopopt::HIRDDAnalysis *DDA,
                            loopopt::HIRSafeReductionAnalysis *SafeRedAnalysis,
                            OptReportVerbosity::Level Verbosity,
                            WRegionInfo *WR, TargetTransformInfo *TTI,
                            TargetLibraryInfo *TLI, AssumptionCache *AC,
                            DominatorTree *DT) {
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
  return DriverImpl::processFunction<loopopt::HLLoop>(Fn);
}

bool DriverHIRImpl::processLoop(HLLoop *Lp, Function &Fn,
                                WRNVecLoopNode *WRLp) {
  // Enable peeling for HIR path from command line switch
  VPlanEnablePeeling = VPlanEnablePeelingHIROpt && VPlanEnablePeelingOpt;
  VPlanEnableGeneralPeeling = VPlanEnableGeneralPeelingHIROpt;

  bool IsOmpSIMD = WRLp && WRLp->isOmpSIMDLoop();

  // TODO: How do we allow stress-testing for HIR path?
  assert(WRLp && "WRLp should be non-null!");

  HLLoop *HLoop = WRLp->getTheLoop<HLLoop>();
  (void)HLoop;
  assert(HLoop && "Expected HIR Loop.");
  assert(HLoop->getParentRegion() && "Expected parent HLRegion.");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PrintHIRBeforeVPlan && llvm::isFunctionInPrintList(Fn.getName())) {
    dbgs() << "Candidate HLLoop before VPlan (" << Fn.getName()
           << "), VPlan#=" << LoopVectorizationPlanner::getVPlanOrderNumber()
           << "\n";
    HLoop->dump();
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Create a VPlanOptReportBuilder object, lifetime is a single loop that we
  // process for vectorization.
  VPlanOptReportBuilder VPORBuilder(ORBuilder);

  if (IsOmpSIMD && !WRLp->isValidHIRSIMDRegion()) {
    WithColor::warning() << "Loop was not vectorized. Invalid SIMD region "
                            "detected for given loop\n";
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailBadlyFormedSimdLoop);
    return bailout(VPORBuilder, Lp, WRLp, BR);
  }

  if (!Lp->isInnermost()) {
    SmallVector<HLLoop *, 8> InnerLoops;
    HIRF->getHLNodeUtils().gatherAllLoops(Lp, InnerLoops);
    unsigned NumMultiExitLps =
        llvm::count_if(InnerLoops, [](HLLoop *L) { return L->isMultiExit(); });
    if (Lp->isMultiExit())
      NumMultiExitLps++;
    if (NumMultiExitLps > 1) {
      LLVM_DEBUG(dbgs() << "VD: Not vectorizing: Cannot support multiple "
                           "multi-exit loops.\n");
      setBailoutRemark(OptReportVerbosity::High,
                       OptRemarkID::VecFailGenericBailout,
                       AuxRemarkID::MultipleMultiExitLoops);
      return bailout(VPORBuilder, Lp, WRLp, BR);
    }
  }

  // If region has SIMD directive mark then mark source loop with SIMD directive
  // so that WarnMissedTransforms pass will detect that this loop is not
  // vectorized later
  if (IsOmpSIMD)
    setHLLoopMD(Lp, "llvm.loop.vectorize.enable");

  VPlanVLSAnalysisHIR VLSA(DDA, Fn.getContext(), *DL, TTI, Lp);

  LegalityHIR HIRVecLegal(TTI, SafeRedAnalysis, DDA, &Fn.getContext());
  LoopVectorizationPlannerHIR LVP(WRLp, Lp, TLI, TTI, DL, getDT(), &HIRVecLegal,
                                  DDA, &VLSA, LightWeightMode,
                                  &Fn.getContext());

  // Send explicit data from WRLoop to the Legality and check whether we can
  // handle it.
  bool CanVectorize = HIRVecLegal.canVectorize(WRLp);

  if (!CanVectorize) {
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: Cannot prove legality.\n");
    auto &HVLBR = HIRVecLegal.getBailoutRemark();
    assert(HVLBR.BailoutRemark && "HIR legality didn't set bailout data!");
    return bailout(VPORBuilder, Lp, WRLp, HVLBR);
  }
  // Find any DDRefs in loop pre-header/post-exit that are aliases to the
  // descriptor variables
  HIRVecLegal.findAliasDDRefs(WRLp->getEntryHLNode(), WRLp->getExitHLNode(),
                              HLoop);
  std::string VPlanName = "";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Lp->getNumber() is not used here because it returns HLNode number, not
  // just HLLoop number, thus it may be unstable to be captured in lit tests.
  VPlanName = std::string(Fn.getName()) + ":HIR";
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  LVP.readLoopMetadata();
  if (IsOmpSIMD && (LVP.isDynAlignEnabled() || LVP.isVecRemainderEnforced()) &&
      !VPlanEnableGeneralPeeling) {
    // If peeling and/or remainder vectorization are enforced,
    // bailout relying on the vplan-vec after loop opt if either
    // cfg merger or general peeling is disabled.
    LLVM_DEBUG(dbgs() << "Delegating peel and remainder vectorization to post "
                         "loopopt vplan-vec\n");
#ifndef NDEBUG
    setBailoutRemark(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     "Delegating peel and remainder vectorization to "
                     "post-loopopt vplan-vec");
    return bailout(VPORBuilder, Lp, WRLp, BR);
#else
    return false;
#endif // NDEBUG
  }
  VPAnalysesFactoryHIR VPAF(Lp, getDT(), DL);
  HLNodeUtils &HNU = Lp->getHLNodeUtils();
  if (!LVP.buildInitialVPlans(&HNU.getModule(), VPlanName, *getAC(), VPAF)) {
    LLVM_DEBUG(dbgs() << "VD: Not vectorizing: No VPlans constructed.\n");
    // Erase intrinsics before and after the loop if this loop is an auto
    // vectorization candidate.
    if (WRLp->getIsAutoVec())
      eraseLoopIntrins(Lp, WRLp);
    auto &LVPBR = LVP.getBailoutRemark();
    assert(LVPBR.BailoutRemark &&
           "buildInitialVPlans did not set bailout data!");
    return bailout(VPORBuilder, Lp, WRLp, LVPBR);
  }

  populateVPlanAnalyses(LVP, VPAF);

  // VPlan construction stress test ends here.
  // TODO: Move after predication.
  if (VPlanConstrStressTest) {
    setBailoutRemark(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Stress testing only."));
    return bailout(VPORBuilder, Lp, WRLp, BR);
  }

  LVP.runPeepholeBeforePredicator();

  if (EnableMaskedVariantHIR && EnableMaskedVariant)
    generateMaskedModeVPlans(&LVP, &VPAF);

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
    // PassManager will invoke LLVM-IR based VPlan when HIR auto-vec fails.
    // TODO: remove call to erase the loop intrinsics after VPlan supports
    // conflict idioms for LLVM-IR side of things.
    if (WRLp->getIsAutoVec())
      eraseLoopIntrins(Lp, WRLp);
    LLVM_DEBUG(dbgs() << "VConflict idiom is not supported.\n");
    setBailoutRemark(OptReportVerbosity::High,
                     OptRemarkID::VecFailGenericBailout,
                     INTERNAL("Unsupported VConflict idiom."));
    return bailout(VPORBuilder, Lp, WRLp, BR);
  }

  // Transform masked integer div/rem before CM.
  LVP.blendWithSafeValue();

  LVP.disableNegOneStrideOptInMaskedModeVPlans();

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

  VPLAN_DUMP(VPlanPrintInit, "initial VPlan for VF=" + std::to_string(VF),
             Plan);

  VPAlignAssumeCleanup Cleanup(*Plan);
  Cleanup.transform();

  bool TreeConflictsLowered = false;

  unsigned UF = LVP.getBestUF();

  // Tracker to collect info about loops emitted by CFGMerger.
  MergedCFGInfo MCFGI;

  // Start preparations to generate auxiliary loops.
  if (VF > 1) {
    LVP.createMergerVPlans(VPAF);

    const auto *PreferredPeeling = Plan->getPreferredPeeling(VF);
    // TODO: remove this hack after getGuaranteedPeeling is fixed in #17045
    const auto *GuaranteedPeeling =
        PreferredPeeling &&
                (dyn_cast<VPlanNoPeelingAligned>(PreferredPeeling) ||
                 dyn_cast<VPlanNoPeelingUnaligned>(PreferredPeeling))
            ? Plan->getGuaranteedPeeling(VF)
            : nullptr;

    // Run some VPlan-to-VPlan transforms for each new auxiliary loop created by
    // CFGMerger.
    for (const CfgMergerPlanDescr &PlanDescr : LVP.mergerVPlans()) {
      // Update tracker based on loop type.
      if (PlanDescr.getLoopType() == CfgMergerPlanDescr::LoopType::LTPeel)
        MCFGI.setPeelLoopEmitted(true);
      else if (PlanDescr.getLoopType() ==
               CfgMergerPlanDescr::LoopType::LTRemainder)
        MCFGI.setRemainderLoopEmitted(true);

      auto LpKind = PlanDescr.getLoopType();
      VPlan *Plan = PlanDescr.getVPlan();

      if (auto *VectorPlan = dyn_cast<VPlanVector>(Plan)) {
        unsigned VectorPlanVF = PlanDescr.getVF();
        // All-zero bypass is added after best plan selection because cost model
        // tuning is not yet implemented and we don't want to prevent
        // vectorization.
        LVP.insertAllZeroBypasses(VectorPlan, VectorPlanVF);

        // We need to lower tree conflict here where VF is available for each
        // VPlan. Otherwise, we can end up generating illegal permute
        // instructions.
        TreeConflictsLowered = lowerTreeConflictsToDoublePermuteTreeReduction(
            VectorPlan, VectorPlanVF, Fn);

        // For non-peel loops, if we have a guaranteed peel, we should propagate
        // the alignment from that peel. Do so now before CG.
        if (LpKind != CfgMergerPlanDescr::LoopType::LTPeel)
          VPlanAlignmentAnalysis::propagateAlignment(
              VectorPlan, TTI, PlanDescr.getVF(), GuaranteedPeeling);
      }

      // TODO: SOA transform missing here.

      if (LpKind == CfgMergerPlanDescr::LoopType::LTMain) {
        // unroll is run on main loop only
        if (auto *NonMaskedVPlan = dyn_cast<VPlanNonMasked>(Plan))
          LVP.unroll(*NonMaskedVPlan);
        // Capture opt-report remarks for main VPLoop.
        addOptReportRemarksForMainPlan(WRLp, PlanDescr);
      }

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

  bool ModifiedLoop = false;
  RegDDRef *PeelArrayRef = nullptr;
  VPlanIdioms::Opcode SearchLoopOpcode =
      VPlanIdioms::isSearchLoop(Plan, true, PeelArrayRef);
  VPOCodeGenHIR VCodeGen(TLI, TTI, SafeRedAnalysis, &VLSA, Plan, Fn, Lp,
                         ORBuilder, &HIRVecLegal, SearchLoopOpcode,
                         PeelArrayRef, IsOmpSIMD, MCFGI, LVP.getLoopDescrs());

  if (!VCodeGen.loopIsHandled(Lp, VF)) {
    // We erase intrinsics before and after the loop if we either vectorize
    // the loop or if this loop is an auto-vectorization candidates.  SIMD
    // intrinsics are left around for loops that are not vectorized.
    if (WRLp->getIsAutoVec())
      eraseLoopIntrins(Lp, WRLp);
    auto &CGBR = VCodeGen.getBailoutRemark();
    assert(CGBR.BailoutRemark && "loopIsHandled did not set bailout data!");
    return bailout(VPORBuilder, Lp, WRLp, CGBR);
  }

  if (!LVP.canLowerVPlan(*Plan, VF) || VF == 1) {
    // Likewise.
    if (WRLp->getIsAutoVec())
      eraseLoopIntrins(Lp, WRLp);
    auto &LVPBR = LVP.getBailoutRemark();
    assert(LVPBR.BailoutRemark && "Planner did not set bailout data!");
    return bailout(VPORBuilder, Lp, WRLp, LVPBR);
  }

  VCodeGen.setTreeConflictsLowered(TreeConflictsLowered);

  // We're vectorizing, so remove loop intrinsics.
  eraseLoopIntrins(Lp, WRLp);
  incrementCandLoopsVectorized();

  if (NestedSimdStrategy == NestedSimdStrategies::Innermost)
    for (WRegionNode *Node = WRLp->getParent(); Node; Node = Node->getParent())
      if (WRNVecLoopNode *LoopNode = dyn_cast<WRNVecLoopNode>(Node))
        eraseLoopIntrins(LoopNode->getTheLoop<HLLoop>(), LoopNode);

  if (NestedSimdStrategy == NestedSimdStrategies::Outermost) {
    auto EraseIntrins = [&](WRNVecLoopNode *N, auto &&EraseIntrins) -> void {
      for (auto *Child : make_range(N->wrn_child_begin(), N->wrn_child_end()))
        if (WRNVecLoopNode *LoopNode = dyn_cast<WRNVecLoopNode>(Child)) {
          eraseLoopIntrins(LoopNode->getTheLoop<HLLoop>(), LoopNode);
          EraseIntrins(LoopNode, EraseIntrins);
        }
    };
    EraseIntrins(WRLp, EraseIntrins);
  }

  // When CFG merger is not enabled run unroller here.
  if (LVP.mergerVPlans().empty())
    LVP.unroll(*cast<VPlanNonMasked>(Plan));
  if (LVP.executeBestPlan(&VCodeGen, UF)) {
    ModifiedLoop = true;
    // Use HLLoop based opt-report generation for non-merged CFG-based CG.
    // TODO: Drop this when merged CFG-based CG is used by default.
    if (LVP.mergerVPlans().empty())
      addOptReportRemarks(WRLp, VPORBuilder, &VCodeGen);
    // Mark loops with "vectorize.enable" metadata as "isvectorized" so that
    // WarnMissedTransforms pass will not complain that this loop is not
    // vectorized. We also tag the main vector loop based on
    // simd/auto-vectorization scenarios. This tag will be reflected in
    // downstream HIR dumps.
    if (IsOmpSIMD) {
      setHLLoopMD(VCodeGen.getMainLoop(), "llvm.loop.isvectorized");
      VCodeGen.setIsVecMDForHLLoops();
    }

    // If the vectorization strategy supposes outer loop vectorization we need
    // to normalize all the inner loops before doing that.
    if (NestedSimdStrategy == NestedSimdStrategies::FromInside) {
      struct Normalizer : HLNodeVisitorBase {
        void visit(const HLNode *) {}
        void postVisit(const HLNode *) {}
        void visit(HLLoop *Loop) { Loop->normalize(); }
      } Visitor;
      HLLoop *ParentLoop = VCodeGen.getMainLoop()->getParentLoop();
      if (ParentLoop && ParentLoop->isSIMD())
        HLNodeUtils::visit(Visitor, ParentLoop);
    }
  }

  return ModifiedLoop;
}

// Given a remark R, generate an otherwise identical remark that prepends
// "HIR: " to the first argument string (if present).
OptRemark DriverHIRImpl::prependHIR(OptRemark R) {

  // The first operand is the unsigned RemarkID.  The second is the
  // string we want to update.
  assert(R.getNumOperands() >= 1);
  if (R.getNumOperands() == 1)
    return R;

  const MDString *MDMsg = cast_or_null<MDString>(R.getOperand(1));
  assert(MDMsg && "Expected a string!");
  std::string NewMsg = "HIR: " + std::string(MDMsg->getString());
  MDString *NewMDMsg = MDString::get(ORBuilder.getContext(), NewMsg);
  MDTuple *Tuple = R.get();
  // Operand number for raw tuple is adjusted by one for the
  // intel.optreport.remark tag at operand 0.
  Tuple->replaceOperandWith(2, NewMDMsg);
  return OptRemark(Tuple);
}

// Add an opt-report remark indicating why we can't vectorize the loop.
// Returns false as a convenience to faciliate "return bailout(...);" usage.
//  - Bail-out reasons with messages of more interest to compiler maintainers
//    than to users should be marked with verbosity High.  For these, we first
//    emit a more generic Medium message.
//  - If we have an OMP SIMD loop and we bail out, we might later vectorize
//    along the LLVM-IR path.  To avoid confusion and double reporting, report
//    only for internal compilers when this can occur.
bool DriverHIRImpl::bailout(VPlanOptReportBuilder &VPORBuilder, HLLoop *Lp,
                            WRNVecLoopNode *WRLp,
                            VPlanBailoutRemark RemarkData) {

  OptRemarkID ID = RemarkData.BailoutRemark.getRemarkID();

  if (WRLp && WRLp->isOmpSIMDLoop() && WillRunLLVMIRVPlan) {
#if !INTEL_PRODUCT_RELEASE
    VPORBuilder.addRemark(Lp, RemarkData.BailoutLevel,
                          prependHIR(RemarkData.BailoutRemark));
#endif // !INTEL_PRODUCT_RELEASE
  } else if (RemarkData.BailoutLevel == OptReportVerbosity::High &&
             ID == OptRemarkID::VecFailGenericBailout) {
    VPORBuilder.addRemark(Lp, OptReportVerbosity::Medium, ID, std::string());
#if !INTEL_PRODUCT_RELEASE
    VPORBuilder.addRemark(Lp, RemarkData.BailoutLevel,
                          RemarkData.BailoutRemark);
#endif // !INTEL_PRODUCT_RELEASE
  } else {
    VPORBuilder.addRemark(Lp, RemarkData.BailoutLevel,
                          RemarkData.BailoutRemark);
  }

  return false;
}

bool DriverHIRImpl::isSupported(HLLoop *Lp, WRNVecLoopNode *WRLp) {
  if (HIRLoopStats->getTotalStatistics(Lp).hasSwitches()) {
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailSwitchPresent,
                     WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                                   : AuxRemarkID::Loop);
    return false;
  }

  // Bail out for outer loops if not enabled
  if (!EnableOuterLoopHIR && !Lp->isInnermost()) {
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailGenericBailout,
                     AuxRemarkID::OuterLoopVecUnsupported);
    return false;
  }

  // Unsupported HLLoop types for vectorization
  if (!(Lp->isDo() || Lp->isDoMultiExit())) {
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailUnknownInductionVariable,
                     WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                                   : AuxRemarkID::Loop,
                     " 5.0");
    return false;
  }
  if (!Lp->isNormalized()) {
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailGenericBailout,
                     AuxRemarkID::OuterLoopVecUnsupported);
    return false;
  }

  return true;
}

void DriverHIRImpl::collectAllLoops(SmallVectorImpl<HLLoop *> &Loops) {
  HIRF->getHLNodeUtils().gatherAllLoops(Loops);
}

bool DriverHIRImpl::isVPlanCandidate(Function &Fn, HLLoop *Lp) {
  // This function is only used in the LLVM-IR path to generate VPlan
  // candidates.
  return false;
}

void DriverHIRImpl::eraseLoopIntrins(HLLoop *Lp, WRNVecLoopNode *WVecNode) {
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
