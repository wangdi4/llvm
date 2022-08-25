//===-- IntelVPlanHCFGBuilder.cpp -----------------------------------------===//
//
//   Copyright (C) 2017-2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the algorithm that builds the hierarchical CFG in
/// VPlan. Further documentation can be found in document 'VPlan Hierarchical
/// CFG Builder'.
///
//===----------------------------------------------------------------------===//
#include "IntelVPlanHCFGBuilder.h"
#include "IntelLoopVectorizationLegality.h"
#include "IntelVPLoopAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanCFGBuilder.h"
#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVerifier.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "VPlanHCFGBuilder"

using namespace llvm;
using namespace llvm::vpo;

#if INTEL_CUSTOMIZATION
static LoopVPlanDumpControl PlainCFGDumpControl("plain-cfg",
                                                "importing plain CFG");
static cl::opt<bool, true> VPlanPrintLegalityOpt(
    "vplan-print-legality", cl::Hidden, cl::location(VPlanPrintLegality),
    cl::desc("Print SIMD clause data structure details in VPlan legality."));
#endif // INTEL_CUSTOMIZATION

namespace llvm {
namespace vpo {
bool VPlanPrintLegality = false;
} // namespace vpo
} // namespace llvm

VPlanHCFGBuilder::VPlanHCFGBuilder(Loop *Lp, LoopInfo *LI, const DataLayout &DL,
                                   const WRNVecLoopNode *WRL, VPlanVector *Plan,
                                   VPOVectorizationLegality *Legal,
                                   ScalarEvolution *SE,
                                   BlockFrequencyInfo *BFI)
    : TheLoop(Lp), LI(LI), BFI(BFI), WRLp(WRL), Plan(Plan), Legal(Legal),
      SE(SE) {
  // TODO: Turn Verifier pointer into an object when Patch #3 of Patch Series
  // #1 lands into VPO and VPlanHCFGBuilderBase is removed.
  Verifier = std::make_unique<VPlanVerifier>(Lp, DL);

  // FIXME: Uncomment assert once we support on-the-fly updates of the LoopInfo
  // in our VPlan CodeGen. See a comment for the
  // VPlanDriverImpl::runStandardMode<llvm::Loop> in IntelVPlanDriver.cpp.
  // assert((!WRLp || WRLp->getTheLoop<Loop>() == TheLoop) &&
  //        "Inconsistent Loop information");
}

VPlanHCFGBuilder::~VPlanHCFGBuilder() = default;

static TripCountInfo readIRLoopMetadata(MDNode *LoopID) {
  TripCountInfo TCInfo;
  if (!LoopID)
    // Default construct to trigger usage of the default estimated trip count
    // later.
    return TCInfo;

  for (const MDOperand &MDOp : LoopID->operands()) {
    const auto *MD = dyn_cast<MDNode>(MDOp);
    if (!MD)
      continue;
    const auto *S = dyn_cast<MDString>(MD->getOperand(0));
    if (!S)
      continue;

    auto ExtractValue = [S, MD](auto &TCInfoField, StringRef MetadataName,
                                StringRef ReadableString) -> void {
      (void)ReadableString;
      if (S->getString().equals(MetadataName)) {
        TCInfoField =
            mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
        LLVM_DEBUG(dbgs() << ReadableString << " trip count is " << TCInfoField
                          << " set by pragma loop count.\n";);
      }
    };
    ExtractValue(TCInfo.MaxTripCount, "llvm.loop.intel.loopcount_maximum",
                 "Max");
    ExtractValue(TCInfo.MinTripCount, "llvm.loop.intel.loopcount_minimum",
                 "Min");
    ExtractValue(TCInfo.TripCount, "llvm.loop.intel.loopcount_average",
                 "Average");
  }

  return TCInfo;
}

void VPlanHCFGBuilder::populateVPLoopMetadata(VPLoopInfo *VPLInfo) {
  for (VPLoop *VPL : VPLInfo->getLoopsInPreorder()) {
    auto *VPLatch = cast_or_null<VPBasicBlock>(VPL->getLoopLatch());
    assert(VPLatch && "No dedicated latch!");
    BasicBlock *Latch = VPLatch->getOriginalBB();
    assert(Latch && "Loop massaging happened before VPLoop's creation?");
    // TODO: IR loop is needed here to query SCEV. Consider dropping it after
    // transitioning SCEV query to loop ID metadata during CFG build.
    Loop *Lp = LI->getLoopFor(Latch);
    assert(Lp &&
           "VPLoopLatch does not correspond to Latch, massaging happened?");
    assert(SE && "SCEV has not been calculated.");

    using TripCountTy = TripCountInfo::TripCountTy;
    // Check if we know the TC exactly.
    if (TripCountTy KnownTC = SE->getSmallConstantTripCount(Lp)) {
      VPL->setKnownTripCount(KnownTC);
      LLVM_DEBUG(dbgs() << "The trip count for loop " << Lp->getName()
                        << " is " << KnownTC << "\n";);
      continue;
    } else {
      LLVM_DEBUG(dbgs() << "Could not estimate trip count for loop "
                        << Lp->getLoopDepth() << " using ScalarEvolution\n";);
    }
    // If not, check if an estimation for max TC can be obtained.
    // First, try reading pragmas.
    TripCountInfo TCInfo = readIRLoopMetadata(VPL->getLoopID());
    if (TripCountTy KnownMaxTC = SE->getSmallConstantMaxTripCount(Lp)) {
      // Then see if the compiler can infer a better estimate.
      TCInfo.MaxTripCount = std::min(TCInfo.MaxTripCount, KnownMaxTC);
      LLVM_DEBUG(dbgs() << "The max trip count for loop " << Lp->getName()
                        << " is estimated to be " << TCInfo.MaxTripCount
                        << "\n";);
    }

    TCInfo.calculateEstimatedTripCount();
    VPL->setTripCountInfo(TCInfo);
  }
}

bool VPlanHCFGBuilder::buildHierarchicalCFG() {

  VPLoopEntityConverterList CvtVec;

  // Build Top Region enclosing the plain CFG
  if (!buildPlainCFG(CvtVec))
    return false;

  Plan->computeDT();
  auto &VPDomTree = *Plan->getDT();

  LLVM_DEBUG(Plan->setName("HCFGBuilder: Plain CFG\n"); dbgs() << *Plan);
  LLVM_DEBUG(Verifier->verifyLoops(Plan, VPDomTree, Plan->getVPLoopInfo()));

  // Compute dom tree for the plain CFG for VPLInfo. We don't need post-dom tree
  // at this point.
  VPDomTree.recalculate(*Plan);
  LLVM_DEBUG(dbgs() << "Dominator Tree After buildPlainCFG\n";
             VPDomTree.print(dbgs()));

  // TODO: If more efficient, we may want to "translate" LoopInfo to VPLoopInfo.
  // Compute VPLInfo and keep it in VPlan
  Plan->setVPLoopInfo(std::make_unique<VPLoopInfo>());
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLInfo->analyze(VPDomTree);
  populateVPLoopMetadata(VPLInfo);

  // LLVM_DEBUG(dbgs() << "Loop Info:\n"; LI->print(dbgs()));
  LLVM_DEBUG(dbgs() << "VPLoop Info After buildPlainCFG:\n";
             VPLInfo->print(dbgs()));

  passEntitiesToVPlan(CvtVec);
  // Remove any duplicate induction PHIs collected during importing
  Plan->getOrCreateLoopEntities(*VPLInfo->begin())
      ->replaceDuplicateInductionPHIs();

  // Compute postdom tree for the plain CFG.
  Plan->computePDT();
  LLVM_DEBUG(dbgs() << "PostDominator Tree After buildPlainCFG:\n";
             Plan->getPDT()->print(dbgs()));

  VPLAN_DUMP(PlainCFGDumpControl, Plan);

  return true;
}

class PrivatesListCvt;

namespace {
// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// VPInstructions.
class PlainCFGBuilder : public VPlanLoopCFGBuilder {
public:
  friend PrivatesListCvt;

  PlainCFGBuilder(Loop *Lp, LoopInfo *LI, VPlanVector *Plan,
                 BlockFrequencyInfo *BFI)
      : VPlanLoopCFGBuilder(Plan, Lp, LI, BFI) {}

  void
  convertEntityDescriptors(LoopVectorizationLegality *Legal,
                           ScalarEvolution *SE,
                           VPlanHCFGBuilder::VPLoopEntityConverterList &Cvts);

  using VPlanLoopCFGBuilder::getOrCreateVPOperand;
};
} // anonymous namespace

static void assertIsSingleElementAlloca(Value *CurValue) {
  if (!CurValue)
    return;
  if (auto AllocaI = dyn_cast<AllocaInst>(CurValue)) {
    Value *ArrSize = AllocaI->getArraySize();
    assert((isa<ConstantInt>(ArrSize) && cast<ConstantInt>(ArrSize)->isOne()) &&
           "Alloca is unsupported for privatization");
    (void)ArrSize;
  }
}

// Base class for VPLoopEntity conversion functors.
class VPEntityConverterBase {
public:
  using InductionList = VPOVectorizationLegality::InductionList;
  using LinearListTy = VPOVectorizationLegality::LinearListTy;
  using ReductionList = VPOVectorizationLegality::ReductionList;
  using ExplicitReductionList = VPOVectorizationLegality::ExplicitReductionList;
  using InMemoryReductionList = VPOVectorizationLegality::InMemoryReductionList;
  using UDRList = VPOVectorizationLegality::UDRList;
  using PrivDescrTy = VPOVectorizationLegality::PrivDescrTy;
  using PrivDescrNonPODTy = VPOVectorizationLegality::PrivDescrNonPODTy;

  VPEntityConverterBase(PlainCFGBuilder &Bld) : Builder(Bld) {}

protected:
  PlainCFGBuilder &Builder;
};

// Conversion functor for auto-recognized reductions
class ReductionListCvt : public VPEntityConverterBase {
public:
  ReductionListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(ReductionDescr &Descriptor,
                  const ReductionList::value_type &CurValue) {
    Descriptor.clear();
    const RecurrenceDescriptor &RD = CurValue.second;
    Descriptor.setStartPhi(
        dyn_cast<VPInstruction>(Builder.getOrCreateVPOperand(CurValue.first)));
    Descriptor.setStart(
        Builder.getOrCreateVPOperand(RD.getRecurrenceStartValue()));
    Descriptor.setExit(dyn_cast<VPInstruction>(
        Builder.getOrCreateVPOperand(RD.getLoopExitInstr())));
    Descriptor.setKind(RD.getRecurrenceKind());
    Descriptor.setRecType(RD.getRecurrenceType());
    Descriptor.setSigned(RD.isSigned());
    Descriptor.setAllocaInst(nullptr);
    Descriptor.setLinkPhi(nullptr);
  }
};
// Conversion functor for explicit reductions
class ExplicitReductionListCvt : public VPEntityConverterBase {
public:
  ExplicitReductionListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(ReductionDescr &Descriptor,
                  const ExplicitReductionList::value_type &CurValue) {
    Descriptor.clear();
    const RecurrenceDescriptor &RD = CurValue.second.RD;
    Descriptor.setStartPhi(
        dyn_cast<VPInstruction>(Builder.getOrCreateVPOperand(CurValue.first)));
    Descriptor.setStart(
        Builder.getOrCreateVPOperand(RD.getRecurrenceStartValue()));
    Descriptor.addUpdateVPInst(dyn_cast<VPInstruction>(
        Builder.getOrCreateVPOperand(RD.getLoopExitInstr())));
    // Exit is not set here, it is determined based on some analyses in Phase 2
    Descriptor.setExit(nullptr);
    Descriptor.setKind(RD.getRecurrenceKind());
    Descriptor.setRecType(RD.getRecurrenceType());
    Descriptor.setSigned(RD.isSigned());
    assertIsSingleElementAlloca(CurValue.second.RedVarPtr);
    Descriptor.setAllocaInst(
        Builder.getOrCreateVPOperand(CurValue.second.RedVarPtr));
    Descriptor.setLinkPhi(nullptr);
    Descriptor.setInscanReductionKind(CurValue.second.InscanRedKind);
  }
};
// Conversion functor for in-memory reductions
class InMemoryReductionListCvt : public VPEntityConverterBase {
public:
  InMemoryReductionListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(ReductionDescr &Descriptor,
                  const InMemoryReductionList::value_type &CurValue) {
    Descriptor.clear();
    auto *RednUpdate = cast<VPInstruction>(
        Builder.getOrCreateVPOperand(CurValue.second.UpdateInst));
    assertIsSingleElementAlloca(CurValue.first);
    VPValue *OrigAlloca = Builder.getOrCreateVPOperand(CurValue.first);
    Descriptor.setStartPhi(nullptr);
    Descriptor.setStart(OrigAlloca);
    Descriptor.addUpdateVPInst(RednUpdate);
    Descriptor.setExit(nullptr);
    Descriptor.setKind(CurValue.second.Kind);
    // According to discussion with paropt team, we can have either alloca
    // or cast<>(alloca) or addrspace_cast<>(alloca) in the reduction clause for
    // non-arrays.
    auto *AI = cast<AllocaInst>(CurValue.first->stripPointerCasts());
    Descriptor.setRecType(AI->getAllocatedType());
    Descriptor.setSigned(false);
    Descriptor.setAllocaInst(OrigAlloca); // Keep original value from clause.
    Descriptor.setLinkPhi(nullptr);
    Descriptor.setInscanReductionKind(CurValue.second.InscanRedKind);
  }
};
// Conversion functor for user-defined reductions. Implementation mimics
// in-memory reduction converter along with capturing
// initialization/finalization functions.
class UDRListCvt : public VPEntityConverterBase {
public:
  UDRListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(ReductionDescr &Descriptor,
                  const UDRList::value_type &CurValue) {
    Descriptor.clear();
    assertIsSingleElementAlloca(CurValue->getRef());
    VPValue *StartVal = Builder.getOrCreateVPOperand(CurValue->getRef());
    Descriptor.setStart(StartVal);
    Descriptor.setKind(CurValue->getKind());
    auto *AI = cast<AllocaInst>(CurValue->getRef()->stripPointerCasts());
    Descriptor.setRecType(AI->getAllocatedType());
    Descriptor.setSigned(false);
    Descriptor.setAllocaInst(StartVal);
    Descriptor.setCombiner(CurValue->getCombiner());
    Descriptor.setInitializer(CurValue->getInitializer());
    Descriptor.setCtor(CurValue->getCtor());
    Descriptor.setDtor(CurValue->getDtor());
  }
};

// Conversion functor for auto-recognized inductions
class InductionListCvt : public VPEntityConverterBase {
public:
  InductionListCvt(PlainCFGBuilder &Bld, VPlanVector *Plan, ScalarEvolution *SE)
      : VPEntityConverterBase(Bld), Plan(Plan), SE(SE) {}

  void operator()(InductionDescr &Descriptor,
                  const InductionList::value_type &CurValue) {
    Descriptor.clear();
    const InductionDescriptor &ID = CurValue.second;
    Descriptor.setStartPhi(
        dyn_cast<VPInstruction>(Builder.getOrCreateVPOperand(CurValue.first)));
    Descriptor.setKind(ID.getKind());
    Descriptor.setStart(Builder.getOrCreateVPOperand(ID.getStartValue()));
    const SCEV *Step = ID.getStep();
    Value *V = nullptr;
    if (auto UndefStep = dyn_cast<SCEVUnknown>(Step))
      V = UndefStep->getValue();
    else if (auto ConstStep = dyn_cast<SCEVConstant>(Step))
      V = ConstStep->getValue();
    if (V)
      Descriptor.setStep(Builder.getOrCreateVPOperand(V));
    else {
      // Step of induction is variable, populate it later via VPlan
      Descriptor.setStep(nullptr);
      // Variable step; auto-detected case
      // Holds auto-detected SCEV returned by IVDescriptor which is later used
      // by insertInductionVPInstructions to create VPInstructions Eg: (8 *
      // %step)
      if (Descriptor.getKind() ==
          llvm::InductionDescriptorData::IK_PtrInduction) {
        Descriptor.setStepSCEV(Step);
        Descriptor.setStepType(Step->getType());
      }
    }
    if (ID.getInductionBinOp()) {
      Descriptor.setInductionOp(dyn_cast<VPInstruction>(
          Builder.getOrCreateVPOperand(ID.getInductionBinOp())));
      Descriptor.setIndOpcode(Instruction::BinaryOpsEnd);
    } else {
      assert(Descriptor.getStartPhi() &&
             "Induction descriptor does not have starting PHI.");
      Type *IndTy = Descriptor.getStartPhi()->getType();
      assert((IndTy->isIntegerTy() || IndTy->isPointerTy()) &&
             "unexpected induction type");
      Descriptor.setInductionOp(nullptr);
      Descriptor.setKindAndOpcodeFromTy(IndTy);
    }

    // Compute lower/upper range for IV.
    VPValue *StartVal = nullptr;
    VPValue *EndVal = nullptr;
    Value *PhiOp0 = CurValue.first->getOperand(0);
    Value *PhiOp1 = CurValue.first->getOperand(1);
    if (SE->isSCEVable(PhiOp0->getType()) &&
        SE->isSCEVable(PhiOp1->getType())) {
      const SCEV *Start = SE->getSCEV(PhiOp0);
      const SCEV *UpdateExpr = SE->getSCEV(PhiOp1);
      if (!isa<SCEVAddRecExpr>(UpdateExpr))
        std::swap(Start, UpdateExpr);
      // Update for induction should be SCEVAddRecExpr since importing is
      // based from LLVM's InductionDescriptor, but check just in case. No
      // harm in just not setting the lower/upper range of iv.
      if (const SCEVAddRecExpr *AddRecExpr =
          dyn_cast<SCEVAddRecExpr>(UpdateExpr)) {
        const Loop *L = AddRecExpr->getLoop();
        uint64_t TripCount = SE->getSmallConstantMaxTripCount(L);
        Type *ConstantTy = CurValue.first->getType();
        int64_t LowerVal;
        if (const SCEVConstant *Lower = dyn_cast<SCEVConstant>(Start)) {
          LowerVal = Lower->getValue()->getSExtValue();
          StartVal =
              Plan->getVPConstant(ConstantInt::get(ConstantTy, LowerVal));
        }
        if (StartVal && TripCount && isa<SCEVConstant>(Step)) {
          int64_t StrideVal =
              cast<SCEVConstant>(Step)->getValue()->getSExtValue();
          int64_t UpperVal = LowerVal + StrideVal * TripCount;
          EndVal = Plan->getVPConstant(ConstantInt::get(ConstantTy, UpperVal));
        }
      }
    }
    Descriptor.setStartVal(StartVal);
    Descriptor.setEndVal(EndVal);
    Descriptor.setAllocaInst(nullptr);
  }

private:
  VPlanVector *Plan;
  ScalarEvolution *SE;
};

// Conversion functor for explcit linears
class LinearListCvt : public VPEntityConverterBase {
public:
  LinearListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(InductionDescr &Descriptor,
                  const LinearListTy::value_type &CurValue) {
    Descriptor.clear();
    Descriptor.setStartPhi(nullptr);
    Descriptor.setStart(Builder.getOrCreateVPOperand(CurValue.first));

    Value *V = CurValue.first;
    assert(V->getType()->isPointerTy() &&
           "expected pointer type for explicit induction");
    Type *IndTy;
    Type *IndPointeeTy;
    Value *Step;
    std::tie(IndTy, IndPointeeTy, Step) = CurValue.second;
    Descriptor.setKindAndOpcodeFromTy(IndTy);

    Type *StepTy = IndTy;
    Descriptor.setStepType(StepTy);
    if (isa<ConstantInt>(Step)) {
      int StepInt = cast<ConstantInt>(Step)->getSExtValue();
      if (IndTy->isPointerTy()) {
        const DataLayout &DL =
            cast<Instruction>(V)->getModule()->getDataLayout();
        StepTy = DL.getIntPtrType(IndTy);
        if (IndTy->isOpaquePointerTy())
          StepInt = DL.getTypeAllocSize(IndPointeeTy).getFixedSize() * StepInt;
      }
      Descriptor.setStep(
          Builder.getOrCreateVPOperand(ConstantInt::get(StepTy, StepInt)));
    } else { // Variable step; explicit case
      if (IndTy->isPointerTy()) {
        // Storing information in converter, which is then used by
        // insertInductionVPInstructions to generate VPInstructions
        const DataLayout &DL =
            cast<Instruction>(V)->getModule()->getDataLayout();
        Descriptor.setStepType(DL.getIntPtrType(IndTy));
        if (IndTy->isOpaquePointerTy())
          Descriptor.setStepMultiplier(
              DL.getTypeAllocSize(IndPointeeTy).getFixedSize());
      }
      Descriptor.setStep(Builder.getOrCreateVPOperand(Step));
    }

    Descriptor.setInductionOp(nullptr);
    assertIsSingleElementAlloca(V);
    // Initialize the AllocaInst of the descriptor with the induction start
    // value. Explicit inductions always have a valid memory allocation.
    Descriptor.setAllocaInst(Descriptor.getStart());
    Descriptor.setIsExplicitInduction(true);
  }
};

// Convert data from Privates list
class PrivatesListCvt : public VPEntityConverterBase {

  bool AliasesWithinLoopImpl(Instruction *Inst,
                             SmallPtrSetImpl<Value *> &Visited) {
    // Here we use \p Visited to avoid infinite loop on reference-cycles. E.g.,
    //    %0 = phi i1 [ %1, ... ], ...
    //    %1 = phi i1 [ %0, ... ], ...
    if (!Visited.insert(Inst).second)
      return false;

    return llvm::any_of(Inst->users(), [&](Value *User) {
      Instruction *Inst = cast<Instruction>(User);
      return Builder.contains(Inst) ||
             ((isTrivialPointerAliasingInst(Inst) || isa<SelectInst>(Inst)) &&
               AliasesWithinLoopImpl(Inst, Visited));
    });
  }

  // Helper to recursively evaluate if there is any user of an alias \p Inst or
  // any user of nested aliases *based on* this alias is inside the loop-region.
  bool AliasesWithinLoop(Instruction *Inst) {
    SmallPtrSet<Value *, 8> Visited;
    return AliasesWithinLoopImpl(Inst, Visited);
  }

  // This method collects aliases that lie outside the loop-region. We are not
  // concerned with aliases within the loop as they would be acquired
  // when required (e.g., escape analysis).
  void collectMemoryAliases(PrivateDescr &Descriptor, Value *Alloca) {
    SetVector<Value *> WorkList;
    SmallPtrSet<const User *, 4> Visited;

    // Start with the Alloca Inst.
    WorkList.insert(Alloca);

    while (!WorkList.empty()) {
      Value *Head = WorkList.back();
      WorkList.pop_back();
      for (auto *Use : Head->users()) {
        if (Visited.contains(Use) ||
            (isa<IntrinsicInst>(Use) &&
              VPOAnalysisUtils::isOpenMPDirective(cast<IntrinsicInst>(Use))))
          continue;

        // Check that the use of this alias is within the loop-region and it is
        // an alias-able instruction to begin with.
        // Rather than the more generic 'aliasing', we are more concerned here
        // with finding if the pointer here is based on another pointer.
        // LLVM Aliasing instructions -
        // https://llvm.org/docs/LangRef.html#pointer-aliasing-rules
        Visited.insert(Use);
        Instruction *Inst = cast<Instruction>(Use);

        if ((isTrivialPointerAliasingInst(Inst) ||
             isa<PtrToIntInst>(Inst) || isa<SelectInst>(Inst)) &&
            AliasesWithinLoop(Inst)) {
          auto *NewVPOperand = Builder.getOrCreateVPOperand(Inst);
          assert((isa<VPExternalDef>(NewVPOperand) ||
                  isa<VPInstruction>(NewVPOperand)) &&
                 "Expecting a VPExternalDef or a VPInstruction.");
          if (isa<VPExternalDef>(NewVPOperand)) {
            // Reset the insert-point. We do not want the instructions to be
            // currently put into any existing basic block.
            Builder.resetInsertPoint();
            WorkList.insert(Inst);
            VPInstruction *VPInst = Builder.createVPInstruction(Inst);
            assert(VPInst && "Expect a valid VPInst to be created.");
            Descriptor.addAlias(NewVPOperand, VPInst, Inst);
          }
        }
      }
    }
  }

public:
  PrivatesListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(PrivateDescr &Descriptor, const PrivDescrTy *CurValue) {
    Descriptor.clear();
    assertIsSingleElementAlloca(CurValue->getRef());
    auto *RefVal = CurValue->getRef();
    auto *VPAllocaVal = Builder.getOrCreateVPOperand(RefVal);

    // Collect the out-of-loop aliases corresponding to this AllocaVal.
    // TODO: This is a temporary solution. Aliases to the private descriptor
    // should be collected earlier with new descriptor representation in
    // VPOLegality.
    collectMemoryAliases(Descriptor, CurValue->getRef());

    Descriptor.setAllocaInst(VPAllocaVal);
    Descriptor.setIsConditional(CurValue->isCond());
    Descriptor.setIsLast(CurValue->isLast());
    Descriptor.setIsExplicit(true);
    Descriptor.setIsMemOnly(true);
    Descriptor.setAllocatedType(CurValue->getType());
    if (CurValue->isNonPOD()) {
      auto *NonPODCurValue = cast<PrivDescrNonPODTy>(CurValue);
      Descriptor.setCtor(NonPODCurValue->getCtor());
      Descriptor.setDtor(NonPODCurValue->getDtor());
      Descriptor.setCopyAssign(NonPODCurValue->getCopyAssign());
      Descriptor.setIsF90NonPod(NonPODCurValue->isF90NonPod());
    }
    SmallVector<VPInstruction *, 4> AliasUpdates;
    for (auto *Alias : CurValue->aliases()) {
      auto *VAliasRef =
          Builder.getOrCreateVPOperand(const_cast<Value *>(Alias->getRef()));
      if (auto *VI = dyn_cast<VPInstruction>(VAliasRef))
        AliasUpdates.push_back(VI);
      for (const Instruction *UpdateInst : Alias->getUpdateInstructions()) {
        LLVM_DEBUG(dbgs() << "Adding update inst:"; UpdateInst->print(dbgs()));
        AliasUpdates.push_back(cast<VPInstruction>(Builder.getOrCreateVPOperand(
            const_cast<Instruction *>(UpdateInst))));
      }
    }
    // TODO: consider combininig collectMemoryAliases with value-aliases
    // gathering.
    Descriptor.setAlias(nullptr /*AliasInit*/, AliasUpdates);
    for (auto UpdateInst : CurValue->getUpdateInstructions())
      Descriptor.addUpdateVPInst(cast<VPInstruction>(
          Builder.getOrCreateVPOperand(const_cast<Instruction *>(UpdateInst))));
  }
};

class Loop2VPLoopMapper {
public:
  Loop2VPLoopMapper() = delete;
  explicit Loop2VPLoopMapper(const Loop *TheLoop, const VPlanVector *Plan) {
    DenseMap<const BasicBlock *, const Loop *> Head2Loop;
    // First fill in the header->loop map
    std::function<void(const Loop *)> getLoopHeaders = [&](const Loop *L) {
      Head2Loop[L->getHeader()] = L;
      for (auto Loop : *L)
        getLoopHeaders(Loop);
    };
    getLoopHeaders(TheLoop);
    // Next fill in the Loop->VPLoop map
    std::function<void(const VPLoop *)> mapLoop2VPLoop =
        [&](const VPLoop *VPL) {
          VPBasicBlock *BB = VPL->getHeader();
          const Loop *L = Head2Loop[BB->getOriginalBB()];
          assert(L != nullptr && "Can't find Loop");
          LoopMap[L] = VPL;
          // Capture opt-report remarks that are present for current loop in
          // incoming IR.
          const_cast<VPLoop *>(VPL)->setOptReport(
              OptReport::findOptReportInLoopID(L->getLoopID()));
          for (auto VLoop : *VPL)
            mapLoop2VPLoop(VLoop);
        };
    const VPLoop *TopLoop = *(Plan->getVPLoopInfo()->begin());
    mapLoop2VPLoop(TopLoop);
  }

  const VPLoop *operator[](const Loop *L) const {
    auto Iter = LoopMap.find(L);
    return Iter == LoopMap.end() ? nullptr : Iter->second;
  }

protected:
  DenseMap<const Loop *, const VPLoop *> LoopMap;
};

// Specialization of reductions and inductions converters.
using ReductionConverter = VPLoopEntitiesConverter<ReductionDescr, Loop, Loop2VPLoopMapper>;
using InductionConverter = VPLoopEntitiesConverter<InductionDescr, Loop, Loop2VPLoopMapper>;
using PrivatesConverter  = VPLoopEntitiesConverter<PrivateDescr, Loop, Loop2VPLoopMapper>;

/// Convert incoming loop entities to the VPlan format.
void PlainCFGBuilder::convertEntityDescriptors(
    VPOVectorizationLegality *Legal,
    ScalarEvolution *SE,
    VPlanHCFGBuilder::VPLoopEntityConverterList &Cvts) {
  auto RedCvt = std::make_unique<ReductionConverter>(Plan);
  auto IndCvt = std::make_unique<InductionConverter>(Plan);
  auto PrivCvt = std::make_unique<PrivatesConverter>(Plan);

  auto Bind = [](auto &&Range, auto &&Converter) {
    return std::make_pair(std::ref(Range), std::move(Converter));
  };

  // clang-format off
  RedCvt->createDescrList(TheLoop,
      Bind(*Legal->getReductionVars(),          ReductionListCvt{*this}),
      Bind(*Legal->getExplicitReductionVars(),  ExplicitReductionListCvt{*this}),
      Bind(*Legal->getInMemoryReductionVars(),  InMemoryReductionListCvt{*this}),
      Bind(*Legal->getUDRVars(),                UDRListCvt{*this}));


  IndCvt->createDescrList(TheLoop,
      Bind(*Legal->getInductionVars(),          InductionListCvt{*this, Plan, SE}),
      Bind(*Legal->getLinears(),                LinearListCvt{*this}));

  PrivCvt->createDescrList(TheLoop,
      Bind(Legal->privates(),                   PrivatesListCvt{*this}));

  // clang-format on

  Cvts.push_back(std::move(RedCvt));
  Cvts.push_back(std::move(IndCvt));
  Cvts.push_back(std::move(PrivCvt));
}

bool VPlanHCFGBuilder::buildPlainCFG(VPLoopEntityConverterList &Cvts) {
  PlainCFGBuilder PCFGBuilder(TheLoop, LI, Plan, BFI);
  PCFGBuilder.buildCFG();
  // Converting loop enities.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (VPlanPrintLegality) {
    Legal->dump(dbgs());
  }
#endif

  PCFGBuilder.convertEntityDescriptors(Legal, SE, Cvts);
  return true;
}

void VPlanHCFGBuilder::passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) {
  using BaseConverter = VPLoopEntitiesConverterTempl<Loop2VPLoopMapper>;

  Loop2VPLoopMapper Mapper(TheLoop, Plan);
  for (auto &Cvt : Cvts) {
    BaseConverter *Converter = dyn_cast<BaseConverter>(Cvt.get());
    Converter->passToVPlan(Plan, Mapper);
  }
}
