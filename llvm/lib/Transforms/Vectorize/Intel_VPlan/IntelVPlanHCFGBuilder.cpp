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
#include "IntelVPlanLoopExitCanonicalization.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanSyncDependenceAnalysis.h"
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

bool LoopMassagingEnabled = true;
static cl::opt<bool, true> LoopMassagingEnabledOpt(
    "vplan-enable-loop-massaging", cl::location(LoopMassagingEnabled),
    cl::Hidden,
    cl::desc("Enable loop massaging in VPlan (Multiple to Singular Exit)"));

#if INTEL_CUSTOMIZATION
extern cl::opt<bool> EnableVPValueCodegen;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool> VPlanPrintAfterLoopMassaging(
    "vplan-print-after-loop-massaging", cl::init(false),
    cl::desc("Print plain dump after loop massaging"));

static cl::opt<bool>
    VPlanPrintSimplifyCFG("vplan-print-after-simplify-cfg", cl::init(false),
                          cl::desc("Print plain dump after VPlan simplify "
                                   "plain CFG"));

static cl::opt<bool>
    VPlanPrintHCFG("vplan-print-after-hcfg", cl::init(false),
                   cl::desc("Print plain dump after build VPlan H-CFG."));

static cl::opt<bool>
    VPlanPrintPlainCFG("vplan-print-plain-cfg", cl::init(false),
                       cl::desc("Print plain dump after VPlan buildPlainCFG."));

static cl::opt<bool> VPlanDotPlainCFG(
    "vplan-dot-plain-cfg", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan digraph after VPlan buildPlainCFG."));

static cl::opt<bool> VPlanDotLoopMassaging(
    "vplan-dot-loop-massaging", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan digraph after loop massaging."));

static cl::opt<bool> DumpAfterVPEntityInstructions(
    "vplan-print-after-vpentity-instrs", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan after insertion of VPEntity instructions."));
#else
static constexpr bool VPlanPrintSimplifyCFG = false;
static constexpr bool VPlanPrintHCFG = false;
static constexpr bool VPlanPrintPlainCFG = false;
static constexpr bool VPlanDotPlainCFG = false;
static constexpr bool VPlanDotLoopMassaging = false;
static constexpr bool DumpAfterVPEntityInstructions = false;
static constexpr bool VPlanPrintAfterLoopMassaging = false;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
#endif // INTEL_CUSTOMIZATION

VPlanHCFGBuilder::VPlanHCFGBuilder(Loop *Lp, LoopInfo *LI, const DataLayout &DL,
                                   const WRNVecLoopNode *WRL, VPlan *Plan,
                                   VPOVectorizationLegality *Legal)
    : TheLoop(Lp), LI(LI), WRLp(WRL), Plan(Plan), Legal(Legal) {
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

// Split loops' preheader block that are not in canonical form
void VPlanHCFGBuilder::splitLoopsPreheader(VPLoop *VPL) {

  // TODO: So far, I haven't found a test case that hits one of these asserts.
  // The code commented out below should cover the second one.

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  // Temporal assert to detect loop header with more than one loop external
  // predecessor
  unsigned NumExternalPreds = 0;
  for (const VPBasicBlock *Pred : VPL->getHeader()->getPredecessors()) {
    if (!VPL->contains(Pred))
      ++NumExternalPreds;
  }
  assert((NumExternalPreds == 1) &&
         "Loop header's external predecessor is not 1");

  // Temporal assert to detect loop preheader with multiple successors
  assert((VPL->getLoopPreheader()->getNumSuccessors() == 1) &&
         "Loop preheader with multiple successors are not supported");

  // If PH has multiple successors, create new PH such that PH->NewPH->H
  // if (VPL->getLoopPreheader()->getNumSuccessors() > 1) {

  //  VPBasicBlock *OldPreheader = VPL->getLoopPreheader();
  //  VPBasicBlock *Header = VPL->getHeader();
  //  assert((DomTree.getNode(Header)->getIDom()->getBlock() == OldPreheader) &&
  //         "Header IDom is not Preheader");

  //  // Create new preheader
  //  VPBasicBlock *NewPreheader = PlanUtils.createBasicBlock();
  //  PlanUtils.insertBlockAfter(NewPreheader, OldPreheader);

  //  // Add new preheader to VPLoopInfo
  //  if (VPLoop *PHLoop = VPLInfo->getLoopFor(OldPreheader)) {
  //    PHLoop->addBasicBlockToLoop(NewPreheader, *VPLInfo);
  //  }

  //  // Update dom/postdom information

  //  // Old preheader is idom of new preheader
  //  VPDomTreeNode *NewPHDomNode =
  //      DomTree.addNewBlock(NewPreheader, OldPreheader /*IDom*/);

  //  // New preheader is idom of header
  //  VPDomTreeNode *DTHeader = DomTree.getNode(Header);
  //  assert(DTHeader && "Expected DomTreeNode for loop header");
  //  DomTree.changeImmediateDominator(DTHeader, NewPHDomNode);

  //  // Header is ipostdom of new preheader
  //  //VPDomTreeNode *NewPHPostDomNode =
  //  PostDomTree.addNewBlock(NewPreheader, Header /*IDom*/);

  //  // New preheader is not ipostdom of any block
  //
  //  // This is not true: New preheader is ipostdom of old preheader
  //  //VPDomTreeNode *PDTPreheader = PostDomTree.getNode(OldPreheader);
  //  //assert(PDTPreheader && "Expected DomTreeNode for loop preheader");
  //  //PostDomTree.changeImmediateDominator(PDTPreheader, NewPHPostDomNode);
  //}

  VPBasicBlock *PH = VPL->getLoopPreheader();
  assert(PH && "Expected loop preheader");
  assert((PH->getNumSuccessors() == 1) &&
         "Expected preheader with single successor");

  // Split loop PH if:
  //    - there is no WRLp (auto-vectorization). We need an empty loop PH.
  //    - has multiple predecessors (it's a potential exit of another region).
  //    - is loop H of another loop.
  if (!WRLp || !PH->getSinglePredecessor() || VPLInfo->isLoopHeader(PH)) {
    VPBlockUtils::splitBlockEnd(PH, VPLInfo, Plan->getDT(), Plan->getPDT());
  }

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsPreheader(VPSL);
  }
}

#if INTEL_CUSTOMIZATION
// Return the nearest common post dominator of all the VPBasicBlocks in \p
// InputVPBlocks.
static VPBasicBlock *getNearestCommonPostDom(
    const VPPostDominatorTree &VPPostDomTree,
    const SmallVectorImpl<VPBasicBlock *> &InputVPBlocks) {
  assert(InputVPBlocks.size() > 0 && "Expected at least one input block!");
  VPBasicBlock *NearestDom = *InputVPBlocks.begin();

  if (InputVPBlocks.size() == 1)
    return NearestDom;

  for (auto *InputVPB : InputVPBlocks) {
    NearestDom = VPPostDomTree.findNearestCommonDominator(InputVPB, NearestDom);
    assert(NearestDom && "Nearest post dominator can't be null!");
  }

  return NearestDom;
}
#endif // INTEL_CUSTOMIZATION

// Split loops' exit block that are not in canonical form
void VPlanHCFGBuilder::splitLoopsExit(VPLoop *VPL) {

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

#if INTEL_CUSTOMIZATION
  SmallVector<VPBasicBlock *, 4> LoopExits;
  VPL->getUniqueExitBlocks(LoopExits);
  // If loop has single exit, the actual block to be potentially split is the
  // single exit. If loop has multiple exits, the actual block to be potentially
  // split is the common landing pad (nearest post dom) of all the exits.
  // TODO: If the CFG after the loop exits gets more complicated, we can get
  // the common post-dom block of all the exits.
  VPBasicBlock *Exit = getNearestCommonPostDom(*Plan->getPDT(), LoopExits);
#else
  VPBasicBlock *Exit = VPL->getUniqueExitBlock();
  assert(Exit && "Only single-exit loops expected");
#endif // INTEL_CUSTOMIZATION

  // Split loop exit with multiple successors or that is preheader of another
  // loop
  VPBasicBlock *PotentialH = Exit->getSingleSuccessor();
  if (!PotentialH ||
      (VPLInfo->isLoopHeader(PotentialH) &&
       VPLInfo->getLoopFor(PotentialH)->getLoopPreheader() == Exit))
    VPBlockUtils::splitBlockEnd(Exit, VPLInfo, Plan->getDT(), Plan->getPDT());

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsExit(VPSL);
  }
}

// Main function that canonicalizes the plain CFG and applyies transformations
// that enable the detection of more regions during the hierarchical CFG
// construction.
void VPlanHCFGBuilder::simplifyPlainCFG() {
  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  assert((VPLInfo->size() == 1) && "Expected only 1 top-level loop");
  VPLoop *TopLoop = *VPLInfo->begin();
  auto &VPDomTree = *Plan->getDT();
  (void)VPDomTree;

  splitLoopsPreheader(TopLoop);

  LLVM_DEBUG(dbgs() << "Dominator Tree Before mergeLoopExits\n";
             VPDomTree.print(dbgs()));

  if (LoopMassagingEnabled) {
    // TODO: Bail-out loop massaging for uniform inner loops.
    for (auto *VPL : post_order(TopLoop)) {
      if (VPL == TopLoop) {
        // TODO: Uncomment after search loops are supported without hacks.
        // assert(VPL->getLoopLatch() == VPL->getExitingBlock() &&
        //        "Top level loop is expected to be in canonical form!");
        continue;
      }
      singleExitWhileLoopCanonicalization(VPL);
      mergeLoopExits(VPL);
      LLVM_DEBUG(Verifier->verifyLoops(Plan, VPDomTree, Plan->getVPLoopInfo()));
    }
    VPLAN_DUMP(VPlanPrintAfterLoopMassaging, "loop massaging", Plan);
    VPLAN_DOT(VPlanDotLoopMassaging, Plan);
    LLVM_DEBUG(dbgs() << "Dominator Tree After mergeLoopExits\n";
               VPDomTree.print(dbgs()));
  }

  splitLoopsExit(TopLoop);
}

static TripCountInfo readIRLoopMetadata(Loop *Lp) {
  TripCountInfo TCInfo;
  MDNode *LoopID = Lp->getLoopID();
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
      if (S->getString().equals(MetadataName))
        TCInfoField =
            mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
      LLVM_DEBUG(dbgs() << ReadableString << " trip count is " << TCInfoField
                        << " set by pragma loop count.\n";);
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
    Loop *Lp = LI->getLoopFor(Latch);
    assert(Lp &&
           "VPLoopLatch does not correspond to Latch, massaging happened?");
    TripCountInfo TCInfo = readIRLoopMetadata(Lp);
    TCInfo.calculateEstimatedTripCount();
    VPL->setTripCountInfo(TCInfo);
  }
}

void VPlanHCFGBuilder::buildHierarchicalCFG() {

  VPLoopEntityConverterList CvtVec;

  // Build Top Region enclosing the plain CFG
  buildPlainCFG(CvtVec);
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

  VPLAN_DUMP(VPlanPrintPlainCFG, "importing plain CFG", Plan);

  // FIXME: Split Move everything after initial CFG construction into separate
  // transformation "passes" and schedule them in the planner/driver instead. We
  // want to lower LoopEntites early in the pipeline so have to call them in
  // this file for the time being awaiting VPlan pipeline refactoring.
  VPLoop *MainLoop = *(Plan->getVPLoopInfo()->begin());
  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(MainLoop);
  VPBuilder VPIRBuilder;
  LE->insertVPInstructions(VPIRBuilder);

  VPLAN_DUMP(DumpAfterVPEntityInstructions, "insertion VPEntities instructions",
             Plan);

  emitVecSpecifics();

  // Prepare/simplify CFG for hierarchical CFG construction
  simplifyPlainCFG();

  VPLAN_DUMP(VPlanPrintSimplifyCFG, "simplify plain CFG", Plan);
  VPLAN_DOT(VPlanDotPlainCFG, Plan);

  LLVM_DEBUG(Plan->setName("HCFGBuilder: After simplifyPlainCFG\n");
             dbgs() << *Plan);
  LLVM_DEBUG(dbgs() << "Dominator Tree After simplifyPlainCFG\n";
             VPDomTree.print(dbgs()));
  LLVM_DEBUG(dbgs() << "PostDominator Tree After simplifyPlainCFG:\n";
             Plan->getPDT()->print(dbgs()));
  LLVM_DEBUG(dbgs() << "VPLoop Info After simplifyPlainCFG:\n";
             VPLInfo->print(dbgs()));

#if INTEL_CUSTOMIZATION
  LLVM_DEBUG(Verifier->verifyLoops(Plan, VPDomTree, Plan->getVPLoopInfo()));
#else
  LLVM_DEBUG(Verifier->verifyLoops(VPDomTree, Plan->getVPLoopInfo()));
#endif

  LLVM_DEBUG(Plan->setName("HCFGBuilder: After building HCFG\n");
             dbgs() << *Plan;);

  VPLAN_DUMP(VPlanPrintHCFG, "building H-CFG", Plan);
}

class PrivatesListCvt;

namespace {
// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// VPInstructions.
class PlainCFGBuilder : public VPlanLoopCFGBuilder {
public:
  friend PrivatesListCvt;

  PlainCFGBuilder(Loop *Lp, LoopInfo *LI, VPlan *Plan)
      : VPlanLoopCFGBuilder(Plan, Lp, LI) {}

  void
  convertEntityDescriptors(LoopVectorizationLegality *Legal,
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
  using PrivatesListTy = VPOVectorizationLegality::PrivatesListTy;

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
    Descriptor.setMinMaxKind(RD.getMinMaxRecurrenceKind());
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
    const RecurrenceDescriptor &RD = CurValue.second.first;
    Descriptor.setStartPhi(
        dyn_cast<VPInstruction>(Builder.getOrCreateVPOperand(CurValue.first)));
    Descriptor.setStart(
        Builder.getOrCreateVPOperand(RD.getRecurrenceStartValue()));
    Descriptor.addUpdateVPInst(dyn_cast<VPInstruction>(
        Builder.getOrCreateVPOperand(RD.getLoopExitInstr())));
    // Exit is not set here, it is determined based on some analyses in Phase 2
    Descriptor.setExit(nullptr);
    Descriptor.setKind(RD.getRecurrenceKind());
    Descriptor.setMinMaxKind(RD.getMinMaxRecurrenceKind());
    Descriptor.setRecType(RD.getRecurrenceType());
    Descriptor.setSigned(RD.isSigned());
    assertIsSingleElementAlloca(CurValue.second.second);
    Descriptor.setAllocaInst(
        Builder.getOrCreateVPOperand(CurValue.second.second));
    Descriptor.setLinkPhi(nullptr);
  }
};
// Conversion functor for in-memory reductions
class InMemoryReductionListCvt : public VPEntityConverterBase {
public:
  InMemoryReductionListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(ReductionDescr &Descriptor,
                  const InMemoryReductionList::value_type &CurValue) {
    Descriptor.clear();
    assertIsSingleElementAlloca(CurValue.first);
    VPValue *AllocaInst = Builder.getOrCreateVPOperand(CurValue.first);
    Descriptor.setStartPhi(nullptr);
    Descriptor.setStart(AllocaInst);
    Descriptor.setExit(nullptr);
    Descriptor.setKind(CurValue.second.first);
    Descriptor.setMinMaxKind(CurValue.second.second);
    Descriptor.setRecType(nullptr);
    Descriptor.setSigned(false);
    Descriptor.setAllocaInst(AllocaInst);
    Descriptor.setLinkPhi(nullptr);
  }
};

// Conversion functor for auto-recognized inductions
class InductionListCvt : public VPEntityConverterBase {
public:
  InductionListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

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
    }
    if (ID.getInductionBinOp()) {
      Descriptor.setInductionBinOp(dyn_cast<VPInstruction>(
          Builder.getOrCreateVPOperand(ID.getInductionBinOp())));
      Descriptor.setBinOpcode(Instruction::BinaryOpsEnd);
    } else {
      assert(Descriptor.getStartPhi() &&
             "Induction descriptor does not have starting PHI.");
      Type *IndTy = Descriptor.getStartPhi()->getType();
      assert((IndTy->isIntegerTy() || IndTy->isPointerTy()) &&
             "unexpected induction type");
      Descriptor.setInductionBinOp(nullptr);
      Descriptor.setKindAndOpcodeFromTy(IndTy);
    }
    Descriptor.setAllocaInst(nullptr);
  }
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

    Type *IndTy = CurValue.first->getType();
    assert(IndTy->isPointerTy() &&
           "expected pointer type for explicit induction");
    IndTy = IndTy->getPointerElementType();
    Type *StepTy = IndTy;
    Descriptor.setKindAndOpcodeFromTy(IndTy);
    if (IndTy->isPointerTy()) {
      assert(isa<Instruction>(CurValue.first) &&
             "Linear descriptor is not an instruction.");
      const DataLayout &DL =
          cast<Instruction>(CurValue.first)->getModule()->getDataLayout();
      StepTy = DL.getIntPtrType(IndTy);
    }
    Value *Cstep = ConstantInt::get(StepTy, CurValue.second);
    Descriptor.setStep(Builder.getOrCreateVPOperand(Cstep));

    Descriptor.setInductionBinOp(nullptr);
    assertIsSingleElementAlloca(CurValue.first);
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
      return Builder.contains(Inst) || (isTrivialPointerAliasingInst(Inst) &&
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
  void collectAliases(PrivateDescr &Descriptor, Value *Alloca) {
    SetVector<Value *> WorkList;

    // Start with the Alloca Inst.
    WorkList.insert(Alloca);

    while (!WorkList.empty()) {
      Value *Head = WorkList.back();
      WorkList.pop_back();
      for (auto *Use : Head->users()) {
        if (isa<IntrinsicInst>(Use) &&
            VPOAnalysisUtils::isOpenMPDirective(cast<IntrinsicInst>(Use)))
          continue;

        // Check that the use of this alias is within the loop-region and it is
        // an alias-able instruction to begin with.
        // Rather than the more generic 'aliasing', we are more concerned here
        // with finding if the pointer here is based on another pointer.
        // LLVM Aliasing instructions -
        // https://llvm.org/docs/LangRef.html#pointer-aliasing-rules
        Instruction *Inst = cast<Instruction>(Use);

        if ((isTrivialPointerAliasingInst(Inst) || isa<PtrToIntInst>(Inst)) &&
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
            Descriptor.addAlias(NewVPOperand, VPInst);
          }
        }
      }
    }
  }

public:
  PrivatesListCvt(PlainCFGBuilder &Bld, bool IsCond = false,
                  bool IsLast = false)
      : VPEntityConverterBase(Bld), IsCondPriv(IsCond), IsLastPriv(IsLast) {}

  void operator()(PrivateDescr &Descriptor,
                  const PrivatesListTy::value_type &CurValue) {

    Descriptor.clear();
    assertIsSingleElementAlloca(CurValue);
    auto *VPAllocaVal = Builder.getOrCreateVPOperand(CurValue);

    // Collect the out-of-loop aliases corresponding to this AllocaVal.
    // TODO: This is a temporary solution. Aliases to the private descriptor
    // should be collected earlier with new descriptor representation in
    // VPOLegality.
    collectAliases(Descriptor, CurValue);

    Descriptor.setAllocaInst(VPAllocaVal);
    Descriptor.setIsConditional(IsCondPriv);
    Descriptor.setIsLast(IsLastPriv);
    Descriptor.setIsExplicit(true);
    Descriptor.setIsMemOnly(true);
  }

private:
  bool IsCondPriv;
  bool IsLastPriv;
};

class Loop2VPLoopMapper {
public:
  Loop2VPLoopMapper() = delete;
  explicit Loop2VPLoopMapper(const Loop *TheLoop, const VPlan *Plan) {
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
    VPlanHCFGBuilder::VPLoopEntityConverterList &Cvts) {

  using InductionList = VPOVectorizationLegality::InductionList;
  using LinearListTy = VPOVectorizationLegality::LinearListTy;
  using ReductionList = VPOVectorizationLegality::ReductionList;
  using ExplicitReductionList = VPOVectorizationLegality::ExplicitReductionList;
  using InMemoryReductionList = VPOVectorizationLegality::InMemoryReductionList;
  using PrivatesListTy = VPOVectorizationLegality::PrivatesListTy;

  ReductionConverter *RedCvt = new ReductionConverter(Plan);
  InductionConverter *IndCvt = new InductionConverter(Plan);
  PrivatesConverter *PrivCvt = new PrivatesConverter(Plan);

  // TODO: create legality and import descriptors for all inner loops too.

  const InductionList *IL = Legal->getInductionVars();
  iterator_range<InductionList::const_iterator> InducRange(IL->begin(), IL->end());
  InductionListCvt InducListCvt(*this);

  const LinearListTy *LL = Legal->getLinears();
  iterator_range<LinearListTy::const_iterator> LinearRange(LL->begin(), LL->end());
  LinearListCvt LinListCvt(*this);

  const ReductionList *RL = Legal->getReductionVars();
  iterator_range<ReductionList::const_iterator> ReducRange(RL->begin(), RL->end());
  ReductionListCvt RedListCvt(*this);

  const ExplicitReductionList *ERL = Legal->getExplicitReductionVars();
  iterator_range<ExplicitReductionList::const_iterator> ExplicitReductionRange(
      ERL->begin(), ERL->end());
  ExplicitReductionListCvt ExpRLCvt(*this);

  const InMemoryReductionList *IMRL = Legal->getInMemoryReductionVars();
  iterator_range<InMemoryReductionList::const_iterator> InMemoryReductionRange(
      IMRL->begin(), IMRL->end());
  InMemoryReductionListCvt IMRLCvt(*this);
  auto ReducPair = std::make_pair(ReducRange, RedListCvt);
  auto ExplicitRedPair = std::make_pair(ExplicitReductionRange, ExpRLCvt);
  auto InMemoryRedPair = std::make_pair(InMemoryReductionRange, IMRLCvt);

  // TODO: VPOLegality stores Privates, LastPrivates and CondPrivates in
  // different lists. This is different from the way HIRLegality store this
  // information. Till we have a unified way of storing the information and
  // accessing it, we will have to do with the following hack where we we go
  // through the Privates, which is a superset, and check membership of elements
  // within ConPrivates and LastPrivates. This helps us separate out paivates
  // based on types. This code will be simplified when we have the correct
  // implementation for Privates in VPOLegality.
  const PrivatesListTy &PrivatesList = Legal->getPrivates();
  const PrivatesListTy &CondPrivatesList = Legal->getCondPrivates();
  const PrivatesListTy &LastPrivatesList = Legal->getLastPrivates();

  PrivatesListTy NewPrivatesList;
  PrivatesListTy NewCondPrivatesList;
  PrivatesListTy NewLastPrivatesList;

  for (auto Val : PrivatesList) {
    if (CondPrivatesList.count(Val))
      NewCondPrivatesList.insert(Val);
    else if (LastPrivatesList.count(Val))
      NewLastPrivatesList.insert(Val);
    else
      NewPrivatesList.insert(Val);
  }

  iterator_range<PrivatesListTy::const_iterator> PrivatesRange(
      NewPrivatesList.begin(), NewPrivatesList.end());
  iterator_range<PrivatesListTy::const_iterator> CondPrivatesRange(
      NewCondPrivatesList.begin(), NewCondPrivatesList.end());
  iterator_range<PrivatesListTy::const_iterator> LastPrivatesRange(
      NewLastPrivatesList.begin(), NewLastPrivatesList.end());

  PrivatesListCvt PrivListCvt(*this);
  PrivatesListCvt CondPrivListCvt(*this, true /*IsCond*/, false /*IsLast*/);
  PrivatesListCvt LastPrivListCvt(*this, false /*IsCond*/, true /*IsLast*/);

  RedCvt->createDescrList(TheLoop, ReducPair, ExplicitRedPair, InMemoryRedPair);

  auto InducPair = std::make_pair(InducRange, InducListCvt);
  auto LinearPair = std::make_pair(LinearRange, LinListCvt);

  auto PrivatesPair = std::make_pair(PrivatesRange, PrivListCvt);
  auto CondPrivatesPair = std::make_pair(CondPrivatesRange, CondPrivListCvt);
  auto LastPrivatesPair = std::make_pair(LastPrivatesRange, LastPrivListCvt);

  IndCvt->createDescrList(TheLoop, InducPair, LinearPair);

  PrivCvt->createDescrList(TheLoop, PrivatesPair, CondPrivatesPair,
                           LastPrivatesPair);

  Cvts.push_back(std::unique_ptr<VPLoopEntitiesConverterBase>(RedCvt));
  Cvts.push_back(std::unique_ptr<VPLoopEntitiesConverterBase>(IndCvt));
  Cvts.push_back(std::unique_ptr<VPLoopEntitiesConverterBase>(PrivCvt));
}

void VPlanHCFGBuilder::buildPlainCFG(VPLoopEntityConverterList &Cvts) {
  PlainCFGBuilder PCFGBuilder(TheLoop, LI, Plan);
  PCFGBuilder.buildCFG();
  // Converting loop enities.
  PCFGBuilder.convertEntityDescriptors(Legal, Cvts);
}

void VPlanHCFGBuilder::passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) {
  typedef VPLoopEntitiesConverterTempl<Loop2VPLoopMapper> BaseConverter;

  Loop2VPLoopMapper Mapper(TheLoop, Plan);
  for (auto &Cvt : Cvts) {
    BaseConverter *Converter = dyn_cast<BaseConverter>(Cvt.get());
    Converter->passToVPlan(Plan, Mapper);
  }
}

void VPlanHCFGBuilder::emitVecSpecifics() {
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLoop *CandidateLoop = *VPLInfo->begin();

  auto *PreHeader = CandidateLoop->getLoopPreheader();
  assert(PreHeader && "Single pre-header is expected!");

  Type *VectorLoopIVType = Legal->getWidestInductionType();
  if (!VectorLoopIVType) {
    // Ugly workaround for tests forcing VPlan build when we can't actually do
    // that. Shouldn't happen outside stress/forced pipeline.
    VectorLoopIVType = Type::getInt64Ty(*Plan->getLLVMContext());
  }
  auto *VPOne = Plan->getVPConstant(ConstantInt::get(VectorLoopIVType, 1));

  VPBuilder Builder;
  Builder.setInsertPoint(PreHeader);
  auto *VF = Builder.createInductionInitStep(VPOne, Instruction::Add, "VF");

  auto *OrigTC = Builder.createOrigTripCountCalculation(TheLoop, CandidateLoop,
                                                        VectorLoopIVType);
  auto *TC = Builder.createVectorTripCountCalculation(OrigTC);

  emitVectorLoopIV(TC, VF);
}

void VPlanHCFGBuilder::emitVectorLoopIV(VPValue *TripCount, VPValue *VF) {
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLoop *CandidateLoop = *VPLInfo->begin();

  auto *PreHeader = CandidateLoop->getLoopPreheader();
  auto *Header = CandidateLoop->getHeader();
  auto *Latch = CandidateLoop->getLoopLatch();
  assert(PreHeader && "Single pre-header is expected!");
  assert(Latch && "Single loop latch is expected!");

  Type *VectorLoopIVType = TripCount->getType();
  auto *VPZero =
      Plan->getVPConstant(ConstantInt::getNullValue(VectorLoopIVType));

  VPBuilder Builder;
  Builder.setInsertPoint(Header, Header->begin());
  auto *IV = Builder.createPhiInstruction(VectorLoopIVType, "vector.loop.iv");
  IV->addIncoming(VPZero, PreHeader);
  Builder.setInsertPoint(Latch);
  auto *IVUpdate = Builder.createAdd(IV, VF, "vector.loop.iv.next");
  IV->addIncoming(IVUpdate, Latch);
  auto *ExitCond = Builder.createCmpInst(
      Latch->getSuccessor(0) == Header ? CmpInst::ICMP_NE : CmpInst::ICMP_EQ,
      IVUpdate, TripCount, "vector.loop.exitcond");

  VPValue *OrigExitCond = Latch->getCondBit();
  Latch->setCondBit(ExitCond);

  // FIXME: Without explicit terminators, CondBit isn't a proper user.
  if (any_of(*Plan, [OrigExitCond](const VPBasicBlock &BB) {
        return BB.getCondBit() == OrigExitCond;
      }))
    return;

  // If original exit condition had single use, remove it - we calculate exit
  // condition differently now.
  // FIXME: "_or_null" here is due to broken stess pipeline that must really
  // stop right after CFG is imported, before *any* transformation is tried on
  // it.
  if (auto *Inst = dyn_cast_or_null<VPInstruction>(OrigExitCond))
    if (Inst->getNumUsers() == 0)
      Latch->eraseInstruction(Inst);
}
