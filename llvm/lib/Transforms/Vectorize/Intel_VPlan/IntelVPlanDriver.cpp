//===-- IntelVPlanDriver.cpp ----------------------------------------------===//
//
//   Copyright (C) 2015-2023 Intel Corporation. All rights reserved.
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
#include "IntelVPAlignAssumeCleanup.h"
#include "IntelVPMemRefTransform.h"
#include "IntelVPOCodeGen.h"
#include "IntelVPOLoopAdapters.h"
#include "IntelVPTransformLibraryCalls.h"
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
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/DemandedBits.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define DEBUG_TYPE "vplan-vec"

using namespace llvm;
using namespace llvm::vpo;

using RemarkRecord = OptReportStatsTracker::RemarkRecord;

static cl::opt<bool> DisableCodeGen(
    "disable-vplan-codegen", cl::init(false), cl::Hidden,
    cl::desc(
        "Disable VPO codegen, when true, the pass stops at VPlan creation"));

static cl::opt<bool> ReportLoopNumber(
    "vplan-report-loop-number", cl::init(false), cl::Hidden,
    cl::desc("Print vectorizer's internal loop number in the opt report"));

static cl::opt<bool> EnableOuterLoopHIR(
    "enable-vplan-outer-loop-hir", cl::init(true), cl::Hidden,
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
    "vplan-enable-peeling-hir", cl::init(true), cl::Hidden,
    cl::desc("Enable generation of peel loops to improve "
             "alignment of memory accesses in HIR path"));

static cl::opt<bool> VPlanEnableGeneralPeelingOpt(
    "vplan-enable-general-peeling", cl::init(true), cl::Hidden,
    cl::desc(
        "Enable peeling in general. When true this effectively enables static "
        "peeling, dynamic peeling needs an additional switch "
        "(-vplan-enable-peeling) to be enabled. When false disables any "
        "peeling. Pragma [no]dynamic_align always overrides both switches."));

static cl::opt<bool> VPlanEnableGeneralPeelingHIROpt(
    "vplan-enable-general-peeling-hir", cl::init(true), cl::Hidden,
    cl::desc(
        "Enable peeling in general for HIR path. When true this effectively "
        "enables static peeling, dynamic peeling needs an additional switch "
        "(-vplan-enable-peeling-hir) to be enabled. When false disables any "
        "peeling. Pragma [no]dynamic_align always overrides both switches."));

bool VPlanDriverPass::RunForSycl = false;
bool VPlanDriverPass::RunForO0 = false;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool>
    VPlanPrintInit("vplan-print-after-init", cl::init(false),
                   cl::desc("Print plain dump after initial VPlan generated"));

static cl::opt<bool> VPlanPrintAfterSingleTripCountOpt(
    "vplan-print-after-single-trip-count-opt", cl::init(false),
    cl::desc("Print after backedge branch rewrite for single trip count vector "
             "loop"));

static cl::opt<bool> VPlanPrintAfterFinalCondTransform(
    "vplan-print-after-final-cond-transform", cl::init(false),
    cl::desc("Print after private finalization instructions transformation"));

static cl::opt<bool> PrintHIRBeforeVPlan(
    "print-hir-before-vplan", cl::init(false),
    cl::desc("Print HLLoop which we attempt to vectorize via VPlanDriverHIR"));

static cl::opt<bool, /*ExternalStorage=*/true> VPlanDebugOptReportOpt(
    "vplan-debug-opt-report",
    cl::location(VPlanDriverImpl::EmitDebugOptRemarks), cl::init(false),
    cl::Hidden,
    cl::desc(
        "Enable the output of debug remarks from VPlan in the opt report."));
#else
static constexpr bool VPlanPrintInit = false;
static constexpr bool VPlanPrintAfterSingleTripCountOpt = false;
static constexpr bool VPlanPrintAfterFinalCondTransform = false;
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
bool VPlanDriverImpl::hasDedicatedAndUniqueExits(Loop *Lp,
                                                 WRNVecLoopNode *WRLp) {

  if (!Lp->hasDedicatedExits()) {
    LLVM_DEBUG(dbgs() << "VD: loop form "
                      << "(" << Lp->getName()
                      << ") is not supported: no dedicated exits.\n");
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailGenericBailout,
                     getAuxMsg(AuxRemarkID::NoDedicatedExits));
    return false;
  }

  if (!Lp->getUniqueExitBlock()) {
    LLVM_DEBUG(dbgs() << "VD: loop form "
                      << "(" << Lp->getName()
                      << ") is not supported: multiple exit blocks.\n");
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailBadlyFormedMultiExitLoop,
                     WRLp && WRLp->isOmpSIMDLoop()
                         ? getAuxMsg(AuxRemarkID::SimdLoop)
                         : getAuxMsg(AuxRemarkID::Loop));
    return false;
  }
  return true;
}

// Auxiliary function that checks only loop-specific constraints. Generic loop
// nest constraints are in 'isSupported' function.
bool VPlanDriverImpl::isSupportedRec(Loop *Lp, WRNVecLoopNode *WRLp) {

  if (!LoopMassagingEnabled && !hasDedicatedAndUniqueExits(Lp, WRLp))
    return false;

  for (Loop *SubLoop : Lp->getSubLoops()) {
    if (!isSupportedRec(SubLoop, WRLp))
      return false;
  }

  return true;
}

static bool canProcessMaskedVariant(const VPlanVector &P) {
  // First check that there is nothing between loop latch condition
  // and branch. Masked mode loop creator may insert inconsistent phi-s
  // for such instructions.
  const VPLoop *VLoop = P.getMainLoop(true /* StrictCheck */);
  const VPBasicBlock *LatchBlock = VLoop->getLoopLatch();
  assert(LatchBlock && "expected non-null latch");
  const VPBranchInst *Br = LatchBlock->getTerminator();
  auto *LatchCond = cast<VPInstruction>(Br->getCondition());
  if (Br->getPrevNode() != LatchCond)
    return false;

  for (const VPInstruction &I : vpinstructions(&P))
    switch (I.getOpcode()) {
    default:
      break;
    // Cloning of VPRegion is not implemented yet, hence we can't support masked
    // variants for CFGs containing them.
    case VPInstruction::GeneralMemOptConflict:
      return false;
    case VPInstruction::PrivateFinalUncond:
    case VPInstruction::PrivateFinalUncondMem:
      // Until CG for extract vector by non-const index is implemented.
      if (I.getType()->isVectorTy())
        return false;
    }
  return true;
}

// Below function converts
//
// f90-dv-buffer-init i64 %dv.init ptr %alloca.priv
//
// Into
//
// %dv.init = call ptr %alloca.priv ptr %priv ptr @_f90_dope_vector_init2
// i64 %num.el = udiv i64 %dv.init i64 4
// i1 %is.allocated = icmp sgt i64 %dv.init i64 0
// br i1 %is.allocated, BB1, BB2
//
// BB1:
//   ptr %dv.array.buffer = allocate-dv-buffer i32 i64 %num.el, OrigAlign = 4
//   ptr %alloca.addr = getelementptr %"DVType", ptr %alloca.priv i32 0 i32 0
//   store ptr %dv.array.buffer ptr %alloca.addr
//   br BB2
//
// BB2:
// ...

static void preprocessDopeVectorInstructions(VPlanVector *Plan) {
  VPLoopInfo *VPLI = Plan->getVPLoopInfo();
  VPDominatorTree *DT = Plan->getDT();
  VPPostDominatorTree *PDT = Plan->getPDT();
  VPlanDivergenceAnalysisBase *DA = Plan->getVPlanDA();

  const auto &VPInstRange =
      map_range(make_filter_range(vpinstructions(Plan),
                                  [](const VPInstruction &VPInst) {
                                    return VPInst.getOpcode() ==
                                           VPInstruction::F90DVBufferInit;
                                  }),
                [](VPInstruction &VPInst) { return &VPInst; });

  SmallVector<VPInstruction *, 2> DVInitInst(VPInstRange.begin(),
                                             VPInstRange.end());
  const DataLayout *DL = Plan->getDataLayout();
  auto *I32 = Type::getInt32Ty(*Plan->getLLVMContext());
  VPConstant *I32Zero = Plan->getVPConstant(ConstantInt::get(I32, 0));

  for (VPInstruction *VPInst : DVInitInst) {
    auto *VPF90DVInitInst = dyn_cast<F90DVBufferInit>(VPInst);
    VPBuilder Builder;
    VPBasicBlock *VPBB = VPF90DVInitInst->getParent();
    VPBasicBlock *VPBBTrue = VPBlockUtils::splitBlock(
        VPBB, VPF90DVInitInst->getIterator(), VPLI, DT, PDT);
    VPInstruction *NextInst = &*std::next(VPF90DVInitInst->getIterator());
    VPBasicBlock *VPBBFalse = VPBlockUtils::splitBlock(
        VPBBTrue, NextInst->getIterator(), VPLI, DT, PDT);
    Builder.setInsertPoint(VPBB);
    VPValue *DVInit = VPF90DVInitInst->getOperand(0);
    VPValue *PrivateMem = VPF90DVInitInst->getOperand(1);
    Type *DVElementType = VPF90DVInitInst->getF90DVElementType();

    auto *NumElements = Builder.createNaryOp(
        Instruction::UDiv, DVInit->getType(),
        {DVInit,
         Plan->getVPConstant(ConstantInt::get(
             DVInit->getType(), DL->getTypeSizeInBits(DVElementType) / 8))});
    DA->updateDivergence(*NumElements);

    VPConstant *DVTypeZero =
        Plan->getVPConstant(ConstantInt::get(DVInit->getType(), 0));
    auto *IsAllocated = Builder.createCmpInst(CmpInst::ICMP_SGT, DVInit,
                                              DVTypeZero, "is.allocated");
    DA->updateDivergence(*IsAllocated);
    VPBB->setTerminator(VPBBTrue, VPBBFalse, IsAllocated);
    VPBlockUtils::updateDomTrees(VPBBTrue, VPBBFalse, VPBB);

    Builder.setInsertPoint(&*VPBBTrue->begin());
    auto *VPAllocaPriv = cast<VPAllocatePrivate>(PrivateMem);
    auto *DVTypePtr = PointerType::get(DVElementType, 0);
    // TODO: Use <VF x element_type> for alignment instead of current approach
    // using alignment of DV Element
    auto *AllocateBuffer = Builder.create<VPAllocateDVBuffer>(
        ".array.buffer", DVTypePtr, DVElementType,
        DL->getPrefTypeAlign(DVElementType), ArrayRef<VPValue *>{NumElements});
    auto *BaseAddrGEP = Builder.createGEP(
        VPAllocaPriv->getAllocatedType(), DVTypePtr, VPAllocaPriv,
        {I32Zero, I32Zero}, nullptr /* Underlying Instruction */);
    DA->updateDivergence(*BaseAddrGEP);
    Builder.createStore(AllocateBuffer, BaseAddrGEP);

    VPBB->eraseInstruction(VPF90DVInitInst);
  }
}

static void preprocessPrivateFinalCondInstructions(VPlanVector *Plan) {

  VPLoopInfo *VPLI = Plan->getVPLoopInfo();
  VPDominatorTree *DT = Plan->getDT();
  VPPostDominatorTree *PDT = Plan->getPDT();
  VPlanDivergenceAnalysisBase *DA = Plan->getVPlanDA();

  // For conditional privates it is possible that the condition will be always
  // false and the value will not be calculated during the loop iterations.
  // So we need to skip conditional private finalization instructions in case if
  // there is no valid last index value for it.
  // The transformation looks like below:
  //
  // For in-memory privates:
  //
  //  %lv = private-final-masked-mem i32 %vec_val, i1 %exec_mask
  //  store %lv, %orig_mem
  // ===>
  //  %check_mask = all-zero-check i1 %exec_mask
  //  %br label %mask_zero, label %mask_non_zero
  // mask_non_zero:
  //  %lv = private-final-masked-mem i32 %vec_val, i1 %exec_mask
  //  store %lv, %orig_mem
  // mask_zero:
  //
  // For registerized privates:
  //
  // loop_exit:
  //  ...
  //  %lv = private-final-masked i32 %vec_val, i1 %exec_mask, %orig_val
  // ===>
  // loop_exit:
  //  ...
  //  %check_mask = all-zero-check i1 %exec_mask
  //  %br label %mask_zero, label %mask_non_zero
  // mask_non_zero:
  //  %lv = private-final-masked i32 %vec_val, i1 %exec_mask, %orig_val
  // mask_zero:
  //  %lv2 = phi [%lv, %mask_non_zero], [%orig_val, %loop_exit]
  //
  const auto &VPInstRange = map_range(
      make_filter_range(
          vpinstructions(Plan),
          [](const VPInstruction &VPInst) {
            return VPInst.getOpcode() == VPInstruction::PrivateFinalCond ||
                   VPInst.getOpcode() == VPInstruction::PrivateFinalCondMem ||
                   VPInst.getOpcode() == VPInstruction::PrivateFinalMasked ||
                   VPInst.getOpcode() == VPInstruction::PrivateFinalMaskedMem ||
                   VPInst.getOpcode() ==
                       VPInstruction::PrivateLastValueNonPODMasked ||
                   VPInst.getOpcode() ==
                       VPInstruction::PrivateFinalArrayMasked ||
                   VPInst.getOpcode() ==
                       VPInstruction::PrivateLastValueArrayNonPODMasked;
          }),
      [](VPInstruction &VPInst) { return &VPInst; });

  SmallVector<VPInstruction *, 2> FinalVPInst(VPInstRange.begin(),
                                              VPInstRange.end());
  for (VPInstruction *VPInst : FinalVPInst) {

    VPBuilder Builder;
    VPBasicBlock *VPBB = VPInst->getParent();
    VPBasicBlock *VPBBTrue =
        VPBlockUtils::splitBlock(VPBB, VPInst->getIterator(), VPLI, DT, PDT);
    VPBasicBlock *VPBBFalse;
    VPInstruction *NextInst = &*std::next(VPInst->getIterator());

    switch (VPInst->getOpcode()) {
    case VPInstruction::PrivateFinalCondMem:
    case VPInstruction::PrivateFinalMaskedMem: {
      assert(VPInst->getNumUsers() == 1 &&
             "Expected exactly one user of memory private finalization "
             "instruction");
      assert(*VPInst->user_begin() == NextInst &&
             isa<VPLoadStoreInst>(NextInst) &&
             "Expected a store instruction next after memory private "
             "finalization instruction");
      // PrivateFinalCondMem and PrivateFinalMaskedMem requires additional
      // assertion checks before VPBBFalse can be generated.
      LLVM_FALLTHROUGH;
    }
    case VPInstruction::PrivateLastValueNonPODMasked:
    case VPInstruction::PrivateLastValueArrayNonPODMasked: {
      VPBBFalse = VPBlockUtils::splitBlock(
          VPBBTrue, std::next(NextInst->getIterator()), VPLI, DT, PDT);
      break;
    }
    case VPInstruction::PrivateFinalArrayMasked:
      VPBBFalse = VPBlockUtils::splitBlock(
          VPBBTrue, NextInst->getIterator(), VPLI, DT, PDT);
      break;
    default: {
      VPBBFalse = VPBlockUtils::splitBlock(VPBBTrue, NextInst->getIterator(),
                                           VPLI, DT, PDT);
      Builder.setInsertPoint(&*VPBBFalse->begin());
      VPPHINode *Phi = Builder.createPhiInstruction(VPInst->getType());
      VPInst->replaceAllUsesWith(Phi);
      Phi->addIncoming(VPInst->getOperand(2), VPBB);
      Phi->addIncoming(VPInst, VPBBTrue);
      DA->updateDivergence(*Phi);
      break;
    }
    }

    Builder.setInsertPoint(VPBB);
    VPCmpInst *CmpInst;

    if (VPInst->getOpcode() == VPInstruction::PrivateFinalMasked ||
        VPInst->getOpcode() == VPInstruction::PrivateFinalMaskedMem) {
      CmpInst = cast<VPCmpInst>(VPInst->getOperand(1));
    } else if (VPInst->getOpcode() ==
                   VPInstruction::PrivateLastValueNonPODMasked ||
               VPInst->getOpcode() == VPInstruction::PrivateFinalArrayMasked ||
               VPInst->getOpcode() ==
                   VPInstruction::PrivateLastValueArrayNonPODMasked) {
      CmpInst = cast<VPCmpInst>(VPInst->getOperand(2));
    } else {
      // The index operand of the VPPrivateFinalCond is initialized with -1,
      // so comparing it with -1 we check the lanes where it was re-assigned.
      VPValue *LastIndex = VPInst->getOperand(1);
      CmpInst = Builder.createCmpInst(
          CmpInst::ICMP_NE, LastIndex,
          Plan->getVPConstant(ConstantInt::get(LastIndex->getType(), -1)));
      DA->updateDivergence(*CmpInst);
    }

    VPValue *AllZeroCheck = Builder.createAllZeroCheck(CmpInst);
    DA->updateDivergence(*AllZeroCheck);
    VPBB->setTerminator(VPBBFalse, VPBBTrue, AllZeroCheck);
    VPBlockUtils::updateDomTrees(VPBBTrue, VPBBFalse, VPBB);
  }

  if (!FinalVPInst.empty())
    Plan->invalidateAnalyses({VPAnalysisID::SVA});

  VPLAN_DUMP(VPlanPrintAfterFinalCondTransform,
             "private finalization instructions transformation", Plan);
}

template <>
bool VPlanDriverImpl::processLoop<llvm::Loop>(Loop *Lp, Function &Fn,
                                              WRNVecLoopNode *WRLp) {
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
  VPOVectorizationLegality LVL(Lp, PSE, &Fn, &Fn.getContext());
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

  VPLAN_DUMP(VPlanPrintInit,
             "initial VPlan for VF=" + std::to_string(VF), Plan);

  VPAlignAssumeCleanup Cleanup(*Plan);
  Cleanup.transform();

  unsigned UF = LVP.getBestUF();

  // Do the preparation for CG: create auxiliary loops and merge them into one
  // piece of CFG.
  if (VF > 1) {
    LVP.createMergerVPlans(VPAF);

    const auto *PeelingVariant = Plan->getPreferredPeeling(VF);

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
          addOptReportRemarksForVecPeel(PlanDescr, PeelingVariant);
        else if (isa<VPlanScalar>(Plan))
          addOptReportRemarksForScalPeel(PlanDescr, PeelingVariant);
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

  VPOCodeGen VCodeGen(Lp, Fn.getContext(), PSE, LI, DT, TLI, TTI, VF, UF, &LVL,
                      &VLSA, Plan, ORBuilder, isOmpSIMDLoop, FatalErrorHandler);
  VCodeGen.initOpenCLScalarSelectSet(volcanoScalarSelect);

  // Run VLS analysis before IR for the current loop is modified.
  VCodeGen.getVLS()->getOVLSMemrefs(Plan, VF);
  applyVLSTransform(*Plan, VLSA, VF);

#ifndef NDEBUG
  // Run verifier before code gen
  // TODO: DA checks are temporarily disabled here since there are some
  // existing issues with DA not assigning shapes to all instructions
  // Once those are patched, the SkipDA flag should be removed.
  VPlanVerifier::verify(Plan, Lp, VPlanVerifier::SkipDA);
#endif
  LVP.executeBestPlan(VCodeGen);

  // Strip the directives once the loop is vectorized. In stress testing,
  // WRLp is null and no directives need deletion.
  if (WRLp)
    VPOUtils::stripDirectives(WRLp);

  CandLoopsVectorized++;

  // TODO: Move this to VPOCodeGen::finalizeLoop.
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
template <class Loop>
bool VPlanDriverImpl::bailout(VPlanOptReportBuilder &VPORBuilder, Loop *Lp,
                              WRNVecLoopNode *WRLp,
                              VPlanBailoutRemark RemarkData) {

  OptRemarkID ID =
      static_cast<OptRemarkID>(RemarkData.BailoutRemark.getRemarkID());

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
  return false;
}

template <>
bool VPlanDriverImpl::bailout<vpo::HLLoop>(VPlanOptReportBuilder &VPORB,
                                           vpo::HLLoop *Lp,
                                           WRNVecLoopNode *WRLp,
                                           VPlanBailoutRemark RemarkData) {
  auto *Self = static_cast<VPlanDriverHIRImpl *>(this);
  return Self->bailout(VPORB, Lp, WRLp, RemarkData);
}

template <>
bool VPlanDriverImpl::processLoop<vpo::HLLoop>(vpo::HLLoop *Lp, Function &Fn,
                                               WRNVecLoopNode *WRLp) {
  auto *Self = static_cast<VPlanDriverHIRImpl *>(this);
  return Self->processLoop(Lp, Fn, WRLp);
}

// Return true if this loop is supported in VPlan
template <>
bool VPlanDriverImpl::isSupported<llvm::Loop>(Loop *Lp, WRNVecLoopNode *WRLp) {
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
    setBailoutRemark(
        OptReportVerbosity::Medium, OptRemarkID::VecFailComplexControlFlow,
        WRLp && WRLp->isOmpSIMDLoop() ? getAuxMsg(AuxRemarkID::SimdLoop)
                                      : getAuxMsg(AuxRemarkID::Loop));
    return false;
  }

  for (BasicBlock *BB : Lp->blocks()) {
    // We don't support switch statements inside loops.
    if (!isa<BranchInst>(BB->getTerminator())) {
      LLVM_DEBUG(dbgs() << "VD: loop nest contains a switch statement.\n");
      setBailoutRemark(
          OptReportVerbosity::Medium, OptRemarkID::VecFailSwitchPresent,
          WRLp && WRLp->isOmpSIMDLoop() ? getAuxMsg(AuxRemarkID::SimdLoop)
                                        : getAuxMsg(AuxRemarkID::Loop));
      return false;
    }
  }

  LLVM_DEBUG(dbgs() << "VD: loop nest "
                    << "(" << Lp->getName() << ") is supported.\n");

  return true;
}

template <>
bool VPlanDriverImpl::isSupported<vpo::HLLoop>(vpo::HLLoop *Lp,
                                               WRNVecLoopNode *WRLp) {
  auto *Self = static_cast<VPlanDriverHIRImpl *>(this);
  return Self->isSupported(Lp, WRLp);
}

/// Standard Mode: standard path for (TODO: automatic and) explicit
/// vectorization.
/// Explicit vectorization: it uses WRegion analysis to collect and vectorize
/// all the WRNVecLoopNode's.
template <typename Loop>
bool VPlanDriverImpl::runStandardMode(Function &Fn) {

  SmallVector<std::pair<Loop *, WRNVecLoopNode *>, 8> LoopsToVectorize;
  auto AddLoopToVectorize = [&](WRegionNode *WRNode,
                                auto &&AddLoopToVectorize) {
    if (WRNVecLoopNode *WRLp = dyn_cast<WRNVecLoopNode>(WRNode)) {
      Loop *Lp = WRLp->getTheLoop<Loop>();
      clearBailoutRemark();
      //      simplifyLoop(Lp, DT, LI, SE, AC, false /* PreserveLCSSA */);
      //      formLCSSARecursively(*Lp, *DT, LI, SE);

      if (!Lp) {
        LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop was optimized out.\n");
        // No bail-out remark, since Lp is nullptr.
        return;
      }

      if (NestedSimdStrategy == NestedSimdStrategies::Innermost &&
          WRLp->hasChildren())
        for (auto *WRN :
             make_range(WRLp->wrn_child_begin(), WRLp->wrn_child_end()))
          AddLoopToVectorize(WRN, AddLoopToVectorize);

      if (!VPlanForceBuild && !isSupported(Lp, WRLp)) {
        LLVM_DEBUG(dbgs() << "Bailing out: Loop is not supported!\n");
        assert(BR.BailoutRemark &&
               "isSupported() failed to provide bailout data!\n");
        VPlanOptReportBuilder VPORB(ORBuilder, LI);
        (void)bailout(VPORB, Lp, WRLp, BR);
        return;
      }

      if (NestedSimdStrategy != NestedSimdStrategies::Innermost ||
          !WRLp->hasChildren())
        LoopsToVectorize.emplace_back(Lp, WRLp);
    }
  };

  LLVM_DEBUG(dbgs() << "VD: Standard Vectorization mode\n");

  isEmitKernelOptRemarks = true;

  IRKind IR = IRKind::LLVMIR;
  if (std::is_same<Loop, HLLoop>::value)
    IR = IRKind::HIR;
  WR->buildWRGraph(IR);
  WRContainerImpl *WRGraph = WR->getWRGraph();

  LLVM_DEBUG(dbgs() << "WD: WRGraph #nodes= " << WRGraph->size() << "\n");
  for (auto WRNode : *WRGraph)
    AddLoopToVectorize(WRNode, AddLoopToVectorize);

  bool ModifiedFunc = false;
  for (const auto &It : LoopsToVectorize) {
    LLVM_DEBUG(dbgs() << "VD: Starting VPlan for \n");
    LLVM_DEBUG(It.second->dump());

    ModifiedFunc |= processLoop(It.first, Fn, It.second);
  }

  return ModifiedFunc;
}

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

  SmallVector<std::pair<BasicBlock *, WRNVecLoopNode *>, 8> LoopsToVectorize;
  auto AddLoopToVectorize = [&](WRegionNode *WRNode,
                                auto &&AddLoopToVectorize) {
    if (WRNVecLoopNode *WRLp = dyn_cast<WRNVecLoopNode>(WRNode)) {
      Loop *Lp = WRLp->getTheLoop<Loop>();
      clearBailoutRemark();

      if (!Lp) {
        LLVM_DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop was optimized out.\n");
        // No bail-out remark since Lp is nullptr.
        return;
      }

      if ((NestedSimdStrategy == NestedSimdStrategies::Innermost ||
           NestedSimdStrategy == NestedSimdStrategies::FromInside) &&
          WRLp->hasChildren())
        for (auto *WRN :
             make_range(WRLp->wrn_child_begin(), WRLp->wrn_child_end()))
          AddLoopToVectorize(WRN, AddLoopToVectorize);

      if (!VPlanForceBuild && !isSupported(Lp, WRLp)) {
        LLVM_DEBUG(dbgs() << "Bailing out: Loop is not supported!\n");
        assert(BR.BailoutRemark &&
               "isSupported() failed to provide bailout data!\n");
        VPlanOptReportBuilder VPORB(ORBuilder, LI);
        (void)bailout(VPORB, Lp, WRLp, BR);
        return;
      }

      if (NestedSimdStrategy != NestedSimdStrategies::Innermost ||
          !WRLp->hasChildren())
        LoopsToVectorize.emplace_back(Lp->getHeader(), WRLp);
    }
  };

  LLVM_DEBUG(dbgs() << "VD: Standard Vectorization mode\n");

  isEmitKernelOptRemarks = true;

  IRKind IR = IRKind::LLVMIR;
  WR->buildWRGraph(IR);
  WRContainerImpl *WRGraph = WR->getWRGraph();

  LLVM_DEBUG(dbgs() << "WD: WRGraph #nodes= " << WRGraph->size() << "\n");
  for (auto WRNode : *WRGraph)
    AddLoopToVectorize(WRNode, AddLoopToVectorize);

  bool ModifiedFunc = false;
  for (const auto &It : LoopsToVectorize) {
    Loop *Lp = LI->getLoopFor(It.first);
    LLVM_DEBUG(dbgs() << "VD: Starting VPlan for \n");
    LLVM_DEBUG(It.second->dump());

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
      ModifiedFunc |= formLCSSA(*Lp, *DT, LI, SE);

    ModifiedFunc |= processLoop(Lp, Fn, It.second);
  }

  return ModifiedFunc;
}

/// Construction Stress Testing Mode: builds the H-CFG for any loop in the
/// function.
/// TODO: WIP for HIR.
template <typename Loop>
bool VPlanDriverImpl::runConstructStressTestMode(Function &Fn) {

  LLVM_DEBUG(dbgs() << "VD: VPlan Construction Stress Test mode\n");

  SmallVector<Loop *, 8> Worklist;
  collectAllLoops(Worklist);

  bool ModifiedFunc = false;
  for (Loop *Lp : Worklist) {
    HIRLoopAdapter<Loop> LpAdapter(Lp);
    clearBailoutRemark();
    // Process only innermost loops if VPlanStressOnlyInnermost is enabled
    if (!VPlanStressOnlyInnermost || LpAdapter.isInnermost()) {
      // simplifyLoop(Lp, DT, LI, SE, AC, false /* PreserveLCSSA */);
      // formLCSSARecursively(*Lp, *DT, LI, SE);
      if (VPlanForceBuild || isSupported(Lp, nullptr /*No WRLp*/))
        ModifiedFunc |= processLoop(Lp, Fn, nullptr /*No WRegion*/);
    }
  }

  return ModifiedFunc;
}

/// CG Stress Testing Mode: generates vector code for the first VPlanVectCand
/// number of loops marked as vectorizable using LoopVectorize analysis. When
/// debugging vector CG issues, we can do a binary search to find out the
/// problem loop by setting VPlanVectCand appropriately.
template <typename Loop>
bool VPlanDriverImpl::runCGStressTestMode(Function &Fn) {

  LLVM_DEBUG(dbgs() << "VD: VPlan CG Stress Test mode\n");

  SmallVector<Loop *, 8> Worklist;
  collectAllLoops(Worklist);

  int ModifiedFunc = false;
  for (Loop *Lp : Worklist) {
    clearBailoutRemark();
    if (CandLoopsVectorized < VPlanVectCand && isVPlanCandidate(Fn, Lp)) {
      ModifiedFunc |= processLoop(Lp, Fn, nullptr /* No WRegion */);
    }
  }

  return ModifiedFunc;
}

// Common LLVM-IR/HIR high-level implementation to process a function. It gets
// LLVM-IR-HIR common analyses and choose an execution mode.
template <typename Loop>
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
  if (VPlanVectCand)
    ModifiedFunc = runCGStressTestMode<Loop>(Fn);
  else if (!VPlanConstrStressTest)
    ModifiedFunc = runStandardMode<Loop>(Fn);
  else {
    ModifiedFunc = runConstructStressTestMode<Loop>(Fn);
    assert(!ModifiedFunc &&
           "VPlan Construction stress testing can't modify Function!");
  }
  return ModifiedFunc;
}

/// Function to add vectorization related remarks for loops created by given
/// codegen object \p VCodeGen
// TODO: Change VPOCodeGenType. This cannot be used in the open sourcing patches
template <class VPOCodeGenType, typename Loop>
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
    VPORBuilder.addRemark(VCodeGen->getMainLoop(), OptReportVerbosity::High,
                          OptRemarkID::VectorizerShortVector);
  // The new vectorized loop is stored in MainLoop
  Loop *MainLoop = VCodeGen->getMainLoop();
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
    Loop *RemLoop = VCodeGen->getRemainderLoop();
    VPORBuilder.addRemark(RemLoop, OptReportVerbosity::Medium,
                          OptRemarkID::UnvectorizedRemainderLoop, "");
  }
}

void VPlanDriverImpl::addOptReportRemarksForMainPlan(
    WRNVecLoopNode *WRLp, const CfgMergerPlanDescr &MainPlanDescr) {
  assert(MainPlanDescr.getLoopType() == CfgMergerPlanDescr::LoopType::LTMain &&
         "Only main loop plan descriptors expected here.");
  auto *VPLpInfo = cast<VPlanVector>(MainPlanDescr.getVPlan())->getVPLoopInfo();
  auto *MainVPLoop = *VPLpInfo->begin();
  OptReportStatsTracker &OptRptStats =
      MainPlanDescr.getVPlan()->getOptRptStatsForLoop(MainVPLoop);
  LLVMContext &C = ORBuilder.getContext();

  // TODO: This remark should be added by CM, not this late after
  // CFGMerger.
  if ((!WRLp || (WRLp->isOmpSIMDLoop() && WRLp->getSafelen() == 0 &&
                 WRLp->getSimdlen() == 0)) &&
      (TTI->getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector) <=
       256) &&
      TTI->isAdvancedOptEnabled(
          TTI::AdvancedOptLevel::AO_TargetHasIntelAVX512)) {
    // 15569 remark is "Compiler has chosen to target XMM/YMM vector."
    // "Try using -mprefer-vector-width=512 to override."
    OptRptStats.GeneralRemarks.emplace_back(
        C, OptRemarkID::VectorizerShortVector, OptReportVerbosity::High);
  }

  if (WRLp && WRLp->isOmpSIMDLoop())
    // Adds remark SIMD LOOP WAS VECTORIZED
    OptRptStats.GeneralRemarks.emplace_back(C, OptRemarkID::SimdLoopVectorized,
                                            OptReportVerbosity::Low);
  else
    // Adds remark LOOP WAS VECTORIZED
    OptRptStats.GeneralRemarks.emplace_back(C, OptRemarkID::LoopVectorized,
                                            OptReportVerbosity::Low);

  // Next print the loop number.
  if (ReportLoopNumber)
    OptRptStats.GeneralRemarks.emplace_back(
        C, OptRemarkID::VectorizerLoopNumber, OptReportVerbosity::Low,
        Twine(LoopVectorizationPlanner::getVPlanOrderNumber()).str());

  // Add remark about VF
  OptRptStats.GeneralRemarks.emplace_back(C, OptRemarkID::VectorizationFactor,
                                          OptReportVerbosity::Low,
                                          Twine(MainPlanDescr.getVF()).str());

  if (MainPlanDescr.getUF() > 1)
    OptRptStats.GeneralRemarks.emplace_back(
        C, OptRemarkID::VectorizerUnrollFactor, OptReportVerbosity::Low,
        Twine(MainPlanDescr.getUF()).str());
}

void VPlanDriverImpl::addOptReportRemarksForVecRemainder(
    const CfgMergerPlanDescr &PlanDescr) {
  assert(PlanDescr.getLoopType() == CfgMergerPlanDescr::LoopType::LTRemainder &&
         "Only remainder loop plan descriptors expected here.");
  auto *VPLI = cast<VPlanVector>(PlanDescr.getVPlan())->getVPLoopInfo();
  auto *OuterLp = *VPLI->begin();
  OptReportStatsTracker &OptRptStats =
      PlanDescr.getVPlan()->getOptRptStatsForLoop(OuterLp);

  LLVMContext &C = ORBuilder.getContext();
  OptRptStats.OriginRemarks.emplace_back(C,
                                         OptRemarkID::VectorizerRemainderLoop);

  if (PlanDescr.isNonMaskedVecRemainder())
    OptRptStats.GeneralRemarks.emplace_back(
        C, OptRemarkID::VectorizedRemainderLoopUnmasked,
        OptReportVerbosity::Low);
  else
    OptRptStats.GeneralRemarks.emplace_back(
        C, OptRemarkID::VectorizedRemainderLoopMasked, OptReportVerbosity::Low);

  OptRptStats.GeneralRemarks.emplace_back(C, OptRemarkID::VectorizationFactor,
                                          OptReportVerbosity::Low,
                                          Twine(PlanDescr.getVF()).str());
}

void VPlanDriverImpl::addOptReportRemarksForScalRemainder(
    const CfgMergerPlanDescr &PlanDescr) {
  assert(PlanDescr.getLoopType() == CfgMergerPlanDescr::LoopType::LTRemainder &&
         "Only remainder loop plan descriptors expected here.");
  auto *ScalarLpI =
      cast<VPlanScalar>(PlanDescr.getVPlan())->getScalarLoopInst();
  // TODO: Any other remarks for scalar peel/remainder loops? Should we report
  // that they were not vectorized?
  ScalarLpI->addOriginRemark(RemarkRecord{
      ORBuilder.getContext(), OptRemarkID::VectorizerRemainderLoop});
}

static std::optional<RemarkRecord>
getPeeledMemrefRemark(LLVMContext &C, const VPlanPeelingVariant *V) {
  if (!VPlanDriverImpl::EmitDebugOptRemarks)
    return {};

  if (!isa<VPlanDynamicPeeling>(V))
    return {};

  const VPLoadStoreInst *Memref = cast<VPlanDynamicPeeling>(V)->memref();
  assert(Memref && "dynamic peel with no memref?");

  // Try to get a suitable name/location to associate with the memref. First try
  // looking at the underlying HIR -- if this exists, the info is usually
  // accurate.
  std::string NameAndDbgLoc;
  if (const RegDDRef *Ref = Memref->getHIRMemoryRef())
    NameAndDbgLoc = Ref->getNameAndDbgLocForOptRpt();

  // If we have no HIR, try just reporting a valid debug location, preferably
  // the location of the pointer we are loading from/storing to.
  if (NameAndDbgLoc.empty()) {
    DebugLoc Loc = Memref->getDebugLocation();
    if (const auto *PtrI = dyn_cast<VPInstruction>(Memref->getPointerOperand()))
      if (PtrI->getDebugLocation())
        Loc = PtrI->getDebugLocation();
    if (Loc)
      NameAndDbgLoc = (Twine("(") + Twine(Loc->getLine()) + ":" +
                       Twine(Loc->getColumn()) + ") ")
                          .str();
  }

  return VPlanDriverImpl::getDebugRemark<RemarkRecord>(
      C, "peeled by memref ", NameAndDbgLoc, "(",
      Memref->getOpcode() == Instruction::Load ? "load" : "store", ")");
}

void VPlanDriverImpl::addOptReportRemarksForVecPeel(
    const CfgMergerPlanDescr &PlanDescr, const VPlanPeelingVariant *Variant) {
  assert(PlanDescr.getLoopType() == CfgMergerPlanDescr::LoopType::LTPeel &&
         "Only peel loop plan descriptors expected here.");
  assert(Variant && "No peeling variant with peel loop?");
  auto *VPLI = cast<VPlanVector>(PlanDescr.getVPlan())->getVPLoopInfo();
  auto *OuterLp = *VPLI->begin();
  OptReportStatsTracker &OptRptStats =
      PlanDescr.getVPlan()->getOptRptStatsForLoop(OuterLp);

  LLVMContext &C = ORBuilder.getContext();
  OptRptStats.OriginRemarks.emplace_back(C, OptRemarkID::VectorizerPeelLoop);

  OptRptStats.GeneralRemarks.emplace_back(C, OptRemarkID::VectorizedPeelLoop,
                                          OptReportVerbosity::Low);

  OptRptStats.GeneralRemarks.emplace_back(C, OptRemarkID::VectorizationFactor,
                                          OptReportVerbosity::Low,
                                          Twine(PlanDescr.getVF()).str());

  OptRptStats.GeneralRemarks.emplace_back(
      C,
      isa<VPlanStaticPeeling>(Variant) ? OptRemarkID::VectorizerStaticPeeling
                                       : OptRemarkID::VectorizerDynamicPeeling,
      OptReportVerbosity::High);

  OptRptStats.GeneralRemarks.emplace_back(
      C, OptRemarkID::VectorizerEstimatedPeelIters, OptReportVerbosity::High,
      std::to_string(Variant->maxPeelCount()));

  if (const auto PeeledMemrefRemark = getPeeledMemrefRemark(C, Variant))
    OptRptStats.GeneralRemarks.push_back(*std::move(PeeledMemrefRemark));
}

void VPlanDriverImpl::addOptReportRemarksForScalPeel(
    const CfgMergerPlanDescr &PlanDescr, const VPlanPeelingVariant *Variant) {
  assert(PlanDescr.getLoopType() == CfgMergerPlanDescr::LoopType::LTPeel &&
         "Only peel loop plan descriptors expected here.");
  assert(Variant && "No peeling variant with peel loop?");
  auto *ScalarLpI =
      cast<VPlanScalar>(PlanDescr.getVPlan())->getScalarLoopInst();
  LLVMContext &C = ORBuilder.getContext();
  ScalarLpI->addOriginRemark(RemarkRecord{C, OptRemarkID::VectorizerPeelLoop});

  ScalarLpI->addGeneralRemark(RemarkRecord{
      C,
      isa<VPlanStaticPeeling>(Variant) ? OptRemarkID::VectorizerStaticPeeling
                                       : OptRemarkID::VectorizerDynamicPeeling,
      OptReportVerbosity::High});

  ScalarLpI->addGeneralRemark(RemarkRecord{
      C, OptRemarkID::VectorizerEstimatedPeelIters, OptReportVerbosity::High,
      std::to_string(Variant->maxPeelCount())});

  if (const auto PeeledMemrefRemark = getPeeledMemrefRemark(C, Variant))
    ScalarLpI->addGeneralRemark(*std::move(PeeledMemrefRemark));
}

void VPlanDriverImpl::populateVPlanAnalyses(LoopVectorizationPlanner &LVP,
                                            VPAnalysesFactoryBase &VPAF) {
  for (auto &Pair : LVP.getAllVPlans())
    VPAF.populateVPlanAnalyses(*Pair.second.MainPlan);
}

void VPlanDriverImpl::generateMaskedModeVPlans(LoopVectorizationPlanner *LVP,
                                               VPAnalysesFactoryBase *VPAF) {
  DenseMap<VPlanVector *, std::shared_ptr<VPlanMasked>> OrigClonedVPlans;
  for (auto &Pair : LVP->getAllVPlans()) {
    std::shared_ptr<VPlanVector> Plan = Pair.second.MainPlan;
    VPLoop *VLoop = Plan->getMainLoop(true);
    // Masked variant is not generated for loops without normalized induction.
    if (!VLoop->hasNormalizedInduction()) {
      LLVM_DEBUG(dbgs() << "skipping masked_mode: non-normalized "
                        << Plan->getName() << "\n";);
      continue;
    }
    if (Pair.second.MaskedModeLoop)
      // Already have it.
      continue;
    // Masked mode is not supported for early-exit loops.
    if (Plan->isEarlyExitLoop())
      continue;

    auto It = OrigClonedVPlans.find(Plan.get());
    if (It != OrigClonedVPlans.end()) {
      // We have already cloned that main loop, add the same clone for
      // this VF.
      LVP->appendVPlanPair(
          Pair.first, LoopVectorizationPlanner::VPlanPair{Plan, It->second});
      continue;
    }
    // Check if we can process VPlan in masked mode. E.g. the code for some
    // entities processing is not implemented yet.
    if (!canProcessMaskedVariant(*Plan)) {
      LLVM_DEBUG(dbgs() << "skipping masked_mode: can't process "
                        << Plan->getName() << "\n";);
      continue;
    }

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
      auto Max = *(std::max_element(LVP->getVectorFactors().begin(),
                                    LVP->getVectorFactors().end()));
      auto Min = *(std::min_element(LVP->getVectorFactors().begin(),
                                    LVP->getVectorFactors().end()));
      if (isPowerOf2_64(TripCount) && TripCount <= Max && TripCount > Min) {
        LLVM_DEBUG(dbgs() << "skipping masked_mode: trip count "
                          << Plan->getName() << "\n";);
        continue;
      }
    }

    MaskedModeLoopCreator MML(cast<VPlanNonMasked>(Plan.get()), *VPAF);
    std::shared_ptr<VPlanMasked> MaskedPlan = MML.createMaskedModeLoop();
    OrigClonedVPlans[Plan.get()] = MaskedPlan;
    LVP->appendVPlanPair(Pair.first,
                         LoopVectorizationPlanner::VPlanPair{Plan, MaskedPlan});
  }
}

static std::string getDescription(const Function &F) {
  return "function (" + F.getName().str() + ")";
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
  LoopAccessInfoManager *LAIs = &AM.getResult<LoopAccessAnalysis>(F);

  if (!Impl.runImpl(F, LI, SE, DT, AC, AA, DB, LAIs, ORE, Verbosity, WR, TTI,
                    TLI, BFI, nullptr, nullptr))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses::none();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  return PA;
}

bool VPlanDriverImpl::runImpl(
    Function &Fn, LoopInfo *LI, ScalarEvolution *SE, DominatorTree *DT,
    AssumptionCache *AC, AliasAnalysis *AA, DemandedBits *DB,
    LoopAccessInfoManager *LAIs,
    OptimizationRemarkEmitter *ORE, OptReportVerbosity::Level Verbosity,
    WRegionInfo *WR, TargetTransformInfo *TTI, TargetLibraryInfo *TLI,
    BlockFrequencyInfo *BFI, ProfileSummaryInfo *PSI,
    FatalErrorHandlerTy FatalErrorHandler) {

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
  this->FatalErrorHandler = FatalErrorHandler;

  ORBuilder.setup(Fn.getContext(), Verbosity);
  bool ModifiedFunc = processFunction(Fn);

  return ModifiedFunc;
}

namespace llvm {
namespace vpo {

template <>
void VPlanDriverImpl::collectAllLoops<llvm::Loop>(
    SmallVectorImpl<Loop *> &Loops) {

  std::function<void(Loop *)> collectSubLoops = [&](Loop *Lp) {
    Loops.push_back(Lp);
    for (Loop *InnerLp : Lp->getSubLoops())
      collectSubLoops(InnerLp);
  };

  for (Loop *Lp : *LI)
    collectSubLoops(Lp);
}

template <>
void VPlanDriverImpl::collectAllLoops<vpo::HLLoop>(
    SmallVectorImpl<vpo::HLLoop *> &Loops) {
  auto *Self = static_cast<VPlanDriverHIRImpl *>(this);
  return Self->collectAllLoops(Loops);
}
} // namespace vpo
} // namespace llvm

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

  ModifiedHIR = Impl.runImpl(F, &HIRF, HIRLoopStats, DDA, SafeRedAnalysis,
                             Verbosity, WR, TTI, TLI, AC, DT, nullptr);
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
                       getAuxMsg(AuxRemarkID::MultipleMultiExitLoops));
      return bailout(VPORBuilder, Lp, WRLp, BR);
    }
  }

  // If region has SIMD directive mark then mark source loop with SIMD directive
  // so that WarnMissedTransforms pass will detect that this loop is not
  // vectorized later
  if (IsOmpSIMD)
    setHLLoopMD(Lp, "llvm.loop.vectorize.enable");

  VPlanVLSAnalysisHIR VLSA(DDA, Fn.getContext(), *DL, TTI, Lp);

  HIRVectorizationLegality HIRVecLegal(TTI, SafeRedAnalysis, DDA,
                                       &Fn.getContext());
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

  VPLAN_DUMP(VPlanPrintInit,
             "initial VPlan for VF=" + std::to_string(VF), Plan);

  VPAlignAssumeCleanup Cleanup(*Plan);
  Cleanup.transform();

  bool TreeConflictsLowered = false;

  unsigned UF = LVP.getBestUF();

  // Tracker to collect info about loops emitted by CFGMerger.
  MergedCFGInfo MCFGI;

  // Start preparations to generate auxiliary loops.
  if (VF > 1) {
    LVP.createMergerVPlans(VPAF);

    const auto *PeelingVariant = Plan->getPreferredPeeling(VF);

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
        TreeConflictsLowered =
            lowerTreeConflictsToDoublePermuteTreeReduction(VectorPlan,
                                                           VectorPlanVF, Fn);
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
          addOptReportRemarksForVecPeel(PlanDescr, PeelingVariant);
        else if (isa<VPlanScalar>(Plan))
          addOptReportRemarksForScalPeel(PlanDescr, PeelingVariant);
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
  CandLoopsVectorized++;

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
      VPlanDriverImpl::addOptReportRemarks<VPOCodeGenHIR, loopopt::HLLoop>(
          WRLp, VPORBuilder, &VCodeGen);
    // Mark loops with "vectorize.enable" metadata as "isvectorized" so that
    // WarnMissedTransforms pass will not complain that this loop is not
    // vectorized. We also tag the main vector loop based on
    // simd/auto-vectorization scenarios. This tag will be reflected in
    // downstream HIR dumps.
    if (IsOmpSIMD) {
      setHLLoopMD(VCodeGen.getMainLoop(), "llvm.loop.isvectorized");
      VCodeGen.setIsVecMDForHLLoops();
    }
  }

  return ModifiedLoop;
}

// Given a remark R, generate an otherwise identical remark that prepends
// "HIR: " to the first argument string (if present).
OptRemark VPlanDriverHIRImpl::prependHIR(OptRemark R) {

  // The first two operands are the unsigned RemarkID and the format
  // string associated with the RemarkID.  The third is the string we
  // want to update.
  assert(R.getNumOperands() >= 2);
  if (R.getNumOperands() == 2)
    return R;

  const MDString *MDMsg = cast_or_null<MDString>(R.getOperand(2));
  assert(MDMsg && "Expected a string!");
  std::string NewMsg = "HIR: " + std::string(MDMsg->getString());
  MDString *NewMDMsg = MDString::get(ORBuilder.getContext(), NewMsg);
  MDTuple *Tuple = R.get();
  // Operand number for raw tuple is adjusted by one for the
  // intel.optreport.remark tag at operand 0.
  Tuple->replaceOperandWith(3, NewMDMsg);
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
bool VPlanDriverHIRImpl::bailout(VPlanOptReportBuilder &VPORBuilder, HLLoop *Lp,
                                 WRNVecLoopNode *WRLp,
                                 VPlanBailoutRemark RemarkData) {

  OptRemarkID ID =
      static_cast<OptRemarkID>(RemarkData.BailoutRemark.getRemarkID());

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

bool VPlanDriverHIRImpl::isSupported(HLLoop *Lp, WRNVecLoopNode *WRLp) {
  if (HIRLoopStats->getTotalStatistics(Lp).hasSwitches()) {
    setBailoutRemark(
        OptReportVerbosity::Medium, OptRemarkID::VecFailSwitchPresent,
        WRLp && WRLp->isOmpSIMDLoop() ? getAuxMsg(AuxRemarkID::SimdLoop)
                                      : getAuxMsg(AuxRemarkID::Loop));
    return false;
  }

  // Bail out for outer loops if not enabled
  if (!EnableOuterLoopHIR && !Lp->isInnermost()) {
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailGenericBailout,
                     getAuxMsg(AuxRemarkID::OuterLoopVecUnsupported));
    return false;
  }

  // Unsupported HLLoop types for vectorization
  if (!(Lp->isDo() || Lp->isDoMultiExit())) {
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailUnknownInductionVariable,
                     WRLp && WRLp->isOmpSIMDLoop()
                         ? getAuxMsg(AuxRemarkID::SimdLoop)
                         : getAuxMsg(AuxRemarkID::Loop));
    return false;
  }
  if (!Lp->isNormalized()) {
    setBailoutRemark(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailGenericBailout,
                     getAuxMsg(AuxRemarkID::OuterLoopVecUnsupported));
    return false;
  }

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

// IMPORTANT -- keep this function at the end of the file until VPO and
// LoopVectorization legality can be merged.
#undef LoopVectorizationLegality
#include "llvm/Transforms/Vectorize/LoopVectorizationLegality.h"

namespace llvm {
namespace vpo {
template <>
bool VPlanDriverImpl::isVPlanCandidate<llvm::Loop>(Function &Fn, Loop *Lp) {
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

template <>
bool VPlanDriverImpl::isVPlanCandidate<vpo::HLLoop>(Function &Fn,
                                                    vpo::HLLoop *Lp) {
  auto *Self = static_cast<VPlanDriverHIRImpl *>(this);
  return Self->isVPlanCandidate(Fn, Lp);
}

} // namespace vpo
} // namespace llvm
