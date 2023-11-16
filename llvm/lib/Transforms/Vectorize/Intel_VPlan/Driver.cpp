//===-- Driver.cpp --------------------------------------------------------===//
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
/// \file Driver.cpp
/// This file implements the VPlan vectorizer driver pass.
//
//===----------------------------------------------------------------------===//

#include "HIR/DriverHIR.h"
#include "HIR/IntelLoopVectorizationPlannerHIR.h"
#include "IntelVPOLoopAdapters.h"
#include "IntelVPlanAllZeroBypass.h"
#include "IntelVPlanMaskedModeLoop.h"
#include "LLVM/DriverLLVM.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Transforms/Vectorize/IntelVPlanDriverPass.h"

#define DEBUG_TYPE "VPlanDriver"

using namespace llvm;
using namespace llvm::vpo;

using RemarkRecord = OptReportStatsTracker::RemarkRecord;

cl::opt<bool, true> DisableCodeGenOpt(
    "disable-vplan-codegen", cl::location(DisableCodeGen), cl::Hidden,
    cl::desc(
        "Disable VPO codegen, when true, the pass stops at VPlan creation"));

cl::opt<bool, true> ReportLoopNumberOpt(
    "vplan-report-loop-number", cl::location(ReportLoopNumber), cl::Hidden,
    cl::desc("Print vectorizer's internal loop number in the opt report"));

cl::opt<bool, true> EnableOuterLoopHIROpt(
    "enable-vplan-outer-loop-hir", cl::location(EnableOuterLoopHIR), cl::Hidden,
    cl::desc("Enable vectorization of outer loops in VPlan HIR path"));

// TODO: Unify with LoopVectorize's vplan-build-stress-test?
cl::opt<bool, true> VPlanConstrStressTestOpt(
    "vpo-vplan-build-stress-test", cl::location(VPlanConstrStressTest),
    cl::desc("Construct VPlan for every loop (stress testing)"));

static cl::opt<bool> VPlanStressOnlyInnermost(
    "vplan-build-stress-only-innermost", cl::init(false),
    cl::desc("When stress testing is enable, construct VPlan only for "
             "innermost loops"));

static cl::opt<bool>
    VPlanForceBuild("vplan-force-build", cl::init(false),
                    cl::desc("Construct VPlan even if loop is not supported "
                             "(only for development)"));

cl::opt<unsigned, true> VPlanVectCandOpt(
    "vplan-build-vect-candidates", cl::location(VPlanVectCand),
    cl::desc(
        "Construct VPlan for vectorization candidates (CG stress testing)"));

cl::opt<bool, true> VPlanEnablePeelingOptn(
    "vplan-enable-peeling", cl::location(VPlanEnablePeelingOpt),
    cl::desc("Enable generation of peel loops to improve "
             "alignment of memory accesses"));

cl::opt<bool, true> VPlanEnablePeelingHIROptn(
    "vplan-enable-peeling-hir", cl::location(VPlanEnablePeelingHIROpt),
    cl::Hidden,
    cl::desc("Enable generation of peel loops to improve "
             "alignment of memory accesses in HIR path"));

cl::opt<bool, true> VPlanEnableGeneralPeelingOptn(
    "vplan-enable-general-peeling", cl::location(VPlanEnableGeneralPeelingOpt),
    cl::Hidden,
    cl::desc(
        "Enable peeling in general. When true this effectively enables static "
        "peeling, dynamic peeling needs an additional switch "
        "(-vplan-enable-peeling) to be enabled. When false disables any "
        "peeling. Pragma [no]dynamic_align always overrides both switches."));

cl::opt<bool, true> VPlanEnableGeneralPeelingHIROptn(
    "vplan-enable-general-peeling-hir",
    cl::location(VPlanEnableGeneralPeelingHIROpt), cl::Hidden,
    cl::desc(
        "Enable peeling in general for HIR path. When true this effectively "
        "enables static peeling, dynamic peeling needs an additional switch "
        "(-vplan-enable-peeling-hir) to be enabled. When false disables any "
        "peeling. Pragma [no]dynamic_align always overrides both switches."));

bool VPlanDriverLLVMPass::RunForSycl = false;
bool VPlanDriverLLVMPass::RunForO0 = false;
VecErrorHandlerTy VPlanDriverLLVMPass::VecErrorHandler = nullptr;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
cl::opt<bool, true> VPlanPrintInitOpt(
    "vplan-print-after-init", cl::location(VPlanPrintInit),
    cl::desc("Print plain dump after initial VPlan generated"));

cl::opt<bool, true> VPlanPrintAfterSingleTripCountOptn(
    "vplan-print-after-single-trip-count-opt",
    cl::location(VPlanPrintAfterSingleTripCountOpt),
    cl::desc("Print after backedge branch rewrite for single trip count vector "
             "loop"));

static cl::opt<bool> VPlanPrintAfterFinalCondTransform(
    "vplan-print-after-final-cond-transform", cl::init(false),
    cl::desc("Print after private finalization instructions transformation"));

cl::opt<bool, true> PrintHIRBeforeVPlanOpt(
    "print-hir-before-vplan", cl::location(PrintHIRBeforeVPlan),
    cl::desc("Print HLLoop which we attempt to vectorize via DriverHIR"));

static cl::opt<bool, /*ExternalStorage=*/true> VPlanDebugOptReportOpt(
    "vplan-debug-opt-report", cl::location(EmitDebugOptRemarks),
    cl::init(false), cl::Hidden,
    cl::desc(
        "Enable the output of debug remarks from VPlan in the opt report."));

bool llvm::vpo::VPlanPrintInit = false;
bool llvm::vpo::VPlanPrintAfterSingleTripCountOpt = false;
bool llvm::vpo::PrintHIRBeforeVPlan = false;
#else
namespace llvm {
namespace vpo {
bool VPlanPrintInit = false;
bool VPlanPrintAfterSingleTripCountOpt = false;
} // namespace vpo
} // namespace llvm
static constexpr bool VPlanPrintAfterFinalCondTransform = false;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

#ifndef NDEBUG
cl::opt<bool, true>
    DebugErrHandlerOpt("vplan-debug-error-handler",
                       cl::location(DebugErrHandler),
                       cl::desc("Enable error handler debugging"));

bool llvm::vpo::DebugErrHandler = false;
#endif // NDEBUG

namespace llvm {
namespace vpo {
bool DisableCodeGen = false;
bool ReportLoopNumber = false;
bool EnableOuterLoopHIR = true;
bool VPlanConstrStressTest = false;
unsigned VPlanVectCand = 0;
bool VPlanEnablePeelingOpt = false;
bool VPlanEnablePeelingHIROpt = true;
bool VPlanEnableGeneralPeelingOpt = true;
bool VPlanEnableGeneralPeelingHIROpt = true;
bool EmitDebugOptRemarks = false;
} // namespace vpo
} // namespace llvm

STATISTIC(CandLoopsVectorized, "Number of candidate loops vectorized");

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

void DriverImpl::preprocessDopeVectorInstructions(VPlanVector *Plan) {
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
    auto *Store = Builder.createStore(AllocateBuffer, VPAllocaPriv);
    DA->markDivergent(*AllocateBuffer);
    DA->markDivergent(*Store);

    VPBB->eraseInstruction(VPF90DVInitInst);
  }
}

void DriverImpl::preprocessPrivateFinalCondInstructions(VPlanVector *Plan) {

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
    VPBasicBlock *VPBBMaskNonZero =
        VPBlockUtils::splitBlock(VPBB, VPInst->getIterator(), VPLI, DT, PDT);
    VPBasicBlock *VPBBMaskZero;
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
      VPBBMaskZero = VPBlockUtils::splitBlock(
          VPBBMaskNonZero, std::next(NextInst->getIterator()), VPLI, DT, PDT);
      break;
    }
    case VPInstruction::PrivateFinalArrayMasked:
      VPBBMaskZero = VPBlockUtils::splitBlock(
          VPBBMaskNonZero, NextInst->getIterator(), VPLI, DT, PDT);
      break;
    default: {
      VPBBMaskZero = VPBlockUtils::splitBlock(
          VPBBMaskNonZero, NextInst->getIterator(), VPLI, DT, PDT);
      Builder.setInsertPoint(&*VPBBMaskZero->begin());
      VPPHINode *Phi = Builder.createPhiInstruction(VPInst->getType());
      VPInst->replaceAllUsesWith(Phi);
      Phi->addIncoming(VPInst->getOperand(2), VPBB);
      Phi->addIncoming(VPInst, VPBBMaskNonZero);
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
    VPBB->setTerminator(VPBBMaskZero, VPBBMaskNonZero, AllZeroCheck);
    VPBlockUtils::updateDomTrees(VPBBMaskNonZero, VPBBMaskZero, VPBB);
  }

  if (!FinalVPInst.empty())
    Plan->invalidateAnalyses({VPAnalysisID::SVA});

  VPLAN_DUMP(VPlanPrintAfterFinalCondTransform,
             "private finalization instructions transformation", Plan);
}

template <>
bool DriverImpl::processLoop<vpo::HLLoop>(vpo::HLLoop *Lp, Function &Fn,
                                          WRNVecLoopNode *WRLp) {
  auto *Self = static_cast<DriverHIRImpl *>(this);
  return Self->processLoop(Lp, Fn, WRLp);
}

template <>
bool DriverImpl::processLoop<llvm::Loop>(Loop *Lp, Function &Fn,
                                         WRNVecLoopNode *WRLp) {
  auto *Self = static_cast<DriverLLVMImpl *>(this);
  return Self->processLoop(Lp, Fn, WRLp);
}

template <>
bool DriverImpl::bailout<vpo::HLLoop>(VPlanOptReportBuilder &VPORB,
                                      vpo::HLLoop *Lp, WRNVecLoopNode *WRLp,
                                      VPlanBailoutRemark RemarkData) {
  auto *Self = static_cast<DriverHIRImpl *>(this);
  return Self->bailout(VPORB, Lp, WRLp, RemarkData);
}

template <>
bool DriverImpl::bailout<llvm::Loop>(VPlanOptReportBuilder &VPORB,
                                     llvm::Loop *Lp, WRNVecLoopNode *WRLp,
                                     VPlanBailoutRemark RemarkData) {
  auto *Self = static_cast<DriverLLVMImpl *>(this);
  return Self->bailout(VPORB, Lp, WRLp, RemarkData);
}

template <>
bool DriverImpl::isSupported<vpo::HLLoop>(vpo::HLLoop *Lp,
                                          WRNVecLoopNode *WRLp) {
  auto *Self = static_cast<DriverHIRImpl *>(this);
  return Self->isSupported(Lp, WRLp);
}

template <>
bool DriverImpl::isSupported<llvm::Loop>(Loop *Lp, WRNVecLoopNode *WRLp) {
  auto *Self = static_cast<DriverLLVMImpl *>(this);
  return Self->isSupported(Lp, WRLp);
}

template <>
vpo::HLLoop *DriverImpl::adjustLoopIfNeeded<vpo::HLLoop>(vpo::HLLoop *Lp,
                                                         BasicBlock *Header) {
  // No adjustment for HIR.
  return Lp;
}

template <>
llvm::Loop *DriverImpl::adjustLoopIfNeeded<llvm::Loop>(llvm::Loop *Lp,
                                                       BasicBlock *Header) {
  auto *Self = static_cast<DriverLLVMImpl *>(this);
  return Self->adjustLoopIfNeeded(Lp, Header);
}

template <> bool DriverImpl::formLCSSAIfNeeded<vpo::HLLoop>(vpo::HLLoop *Lp) {
  // No recalculation needed for HIR.
  return false;
}

template <> bool DriverImpl::formLCSSAIfNeeded<llvm::Loop>(llvm::Loop *Lp) {
  auto *Self = static_cast<DriverLLVMImpl *>(this);
  return Self->formLCSSAIfNeeded(Lp);
}

template <>
void DriverImpl::collectAllLoops<vpo::HLLoop>(
    SmallVectorImpl<vpo::HLLoop *> &Loops) {
  auto *Self = static_cast<DriverHIRImpl *>(this);
  return Self->collectAllLoops(Loops);
}

template <>
void DriverImpl::collectAllLoops<Loop>(SmallVectorImpl<Loop *> &Loops) {
  auto *Self = static_cast<DriverLLVMImpl *>(this);
  return Self->collectAllLoops(Loops);
}

template <>
bool DriverImpl::isVPlanCandidate<vpo::HLLoop>(Function &Fn, vpo::HLLoop *Lp) {
  auto *Self = static_cast<DriverHIRImpl *>(this);
  return Self->isVPlanCandidate(Fn, Lp);
}

template <>
bool DriverImpl::isVPlanCandidate<llvm::Loop>(Function &Fn, Loop *Lp) {
  auto *Self = static_cast<DriverLLVMImpl *>(this);
  return Self->isVPlanCandidate(Fn, Lp);
}

/// Standard Mode: standard path for automatic and explicit vectorization.
/// Explicit vectorization: it uses WRegion analysis to collect and vectorize
/// all the WRNVecLoopNode's.
template <typename Loop> bool DriverImpl::runStandardMode(Function &Fn) {
  const bool ShouldVectorizeChildren =
      NestedSimdStrategy == NestedSimdStrategies::Innermost ||
      NestedSimdStrategy == NestedSimdStrategies::FromInside;
  const bool ShouldVectorizeWithChildren =
      NestedSimdStrategy != NestedSimdStrategies::Innermost;

  // Describes a candidate loop to vectorize, along with its WRN node and
  // (optional) loop header. The loop header is used only in the LLVM-IR path.
  struct LoopToVectorize {
    Loop *L = nullptr;
    WRNVecLoopNode *WRLp = nullptr;
    BasicBlock *Header = nullptr;
  };

  SmallVector<LoopToVectorize, 8> LoopsToVectorize;
  auto AddLoopToVectorize = [&LoopsToVectorize](Loop *Lp,
                                                WRNVecLoopNode *WRLp) {
    // This should really be separated into LLVM and HIR logic, but doing
    // so is more mess than it's worth.  Don't copy this coding style.
    // runStandardMode needs rework anyway, so we may clean it up then.
    if constexpr (std::is_same_v<Loop, llvm::Loop>) {
      LoopsToVectorize.push_back(LoopToVectorize{Lp, WRLp, Lp->getHeader()});
    } else {
      LoopsToVectorize.push_back(LoopToVectorize{Lp, WRLp});
    }
  };
  auto AddLoopsToVectorize = [&](WRegionNode *WRNode,
                                 auto &&AddLoopsToVectorize) {
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

      auto Children =
          make_range(WRLp->wrn_child_begin(), WRLp->wrn_child_end());
      if (ShouldVectorizeChildren && WRLp->hasChildren())
        for (auto *WRN : Children)
          AddLoopsToVectorize(WRN, AddLoopsToVectorize);

      if (!VPlanForceBuild && !isSupported(Lp, WRLp)) {
        LLVM_DEBUG(dbgs() << "Bailing out: Loop is not supported!\n");
        assert(BR.BailoutRemark &&
               "isSupported() failed to provide bailout data!\n");
        VPlanOptReportBuilder VPORB(ORBuilder);
        (void)bailout(VPORB, Lp, WRLp, BR);
        return;
      }

      if (!WRLp->hasChildren() ||
          ShouldVectorizeWithChildren &&
              // Note: we don't vectorize outer loop if there is a
              // reduction/private/linear clause in the nested loop since it
              // will be treated as uniform.
              // TODO: remove this bailout and switch nested vectorization
              // strategy to innermost-to-outermost by default in a long-term.
              llvm::all_of(Children, [](WRegionNode *N) {
                return !isa<WRNVecLoopNode>(N) || N->getRed().empty() &&
                                                      N->getPriv().empty() &&
                                                      N->getLinear().empty();
              }))
        AddLoopToVectorize(Lp, WRLp);
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
    AddLoopsToVectorize(WRNode, AddLoopsToVectorize);

  bool ModifiedFunc = false;
  for (const LoopToVectorize &Cand : LoopsToVectorize) {
    Loop *L = Cand.L;
    L = adjustLoopIfNeeded(L, Cand.Header);
    assert(L != nullptr);
    assert(Cand.WRLp != nullptr);
    LLVM_DEBUG(dbgs() << "VD: Starting VPlan for \n");
    LLVM_DEBUG(L->dump());
    ModifiedFunc |= formLCSSAIfNeeded(L);
    ModifiedFunc |= processLoop(L, Fn, Cand.WRLp);
  }

  return ModifiedFunc;
}

/// Construction Stress Testing Mode: builds the H-CFG for any loop in the
/// function.
/// TODO: WIP for HIR.
template <typename Loop>
bool DriverImpl::runConstructStressTestMode(Function &Fn) {

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

void DriverImpl::incrementCandLoopsVectorized() { CandLoopsVectorized++; }

/// CG Stress Testing Mode: generates vector code for the first VPlanVectCand
/// number of loops marked as vectorizable using LoopVectorize analysis. When
/// debugging vector CG issues, we can do a binary search to find out the
/// problem loop by setting VPlanVectCand appropriately.
template <typename Loop> bool DriverImpl::runCGStressTestMode(Function &Fn) {

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
template <typename Loop> bool DriverImpl::processFunction(Function &Fn) {

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

template bool DriverImpl::processFunction<llvm::Loop>(Function &Fn);
template bool DriverImpl::processFunction<vpo::HLLoop>(Function &Fn);

void DriverImpl::addOptReportRemarksForMainPlan(
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

void DriverImpl::addOptReportRemarksForVecRemainder(
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

void DriverImpl::addOptReportRemarksForScalRemainder(
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
  if (!EmitDebugOptRemarks)
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

  return DriverImpl::getDebugRemark<RemarkRecord>(
      C, "peeled by memref ", NameAndDbgLoc, "(",
      Memref->getOpcode() == Instruction::Load ? "load" : "store", ")");
}

void DriverImpl::addOptReportRemarksForVecPeel(
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
      isa<VPlanStaticPeeling>(Variant) || isa<VPlanNoPeeling>(Variant) ||
              isa<VPlanNoPeelingAligned>(Variant) ||
              isa<VPlanNoPeelingUnaligned>(Variant)
          ? OptRemarkID::VectorizerStaticPeeling
          : OptRemarkID::VectorizerDynamicPeeling,
      OptReportVerbosity::High);

  OptRptStats.GeneralRemarks.emplace_back(
      C, OptRemarkID::VectorizerEstimatedPeelIters, OptReportVerbosity::High,
      std::to_string(Variant->maxPeelCount()));

  if (const auto PeeledMemrefRemark = getPeeledMemrefRemark(C, Variant))
    OptRptStats.GeneralRemarks.push_back(*std::move(PeeledMemrefRemark));
}

void DriverImpl::addOptReportRemarksForScalPeel(
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
      isa<VPlanStaticPeeling>(Variant) || isa<VPlanNoPeeling>(Variant) ||
              isa<VPlanNoPeelingAligned>(Variant) ||
              isa<VPlanNoPeelingUnaligned>(Variant)
          ? OptRemarkID::VectorizerStaticPeeling
          : OptRemarkID::VectorizerDynamicPeeling,
      OptReportVerbosity::High});

  ScalarLpI->addGeneralRemark(RemarkRecord{
      C, OptRemarkID::VectorizerEstimatedPeelIters, OptReportVerbosity::High,
      std::to_string(Variant->maxPeelCount())});

  if (const auto PeeledMemrefRemark = getPeeledMemrefRemark(C, Variant))
    ScalarLpI->addGeneralRemark(*std::move(PeeledMemrefRemark));
}

void DriverImpl::populateVPlanAnalyses(LoopVectorizationPlanner &LVP,
                                       VPAnalysesFactoryBase &VPAF) {
  for (auto &Pair : LVP.getAllVPlans())
    VPAF.populateVPlanAnalyses(*Pair.second.MainPlan);
}

void DriverImpl::generateMaskedModeVPlans(LoopVectorizationPlanner *LVP,
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

    // Masked mode is not needed in vector functions.
    if (Plan->isVecFuncVariant())
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
