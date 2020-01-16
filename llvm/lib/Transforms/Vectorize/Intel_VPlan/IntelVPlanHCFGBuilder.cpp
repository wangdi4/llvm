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

static cl::opt<bool> VPlanPrintAfterLoopMassaging(
    "vplan-print-after-loop-massaging", cl::init(false),
    cl::desc("Print plain dump after loop massaging"));

#if INTEL_CUSTOMIZATION

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

extern cl::opt<bool> EnableVPValueCodegen;
#endif

VPlanHCFGBuilder::VPlanHCFGBuilder(Loop *Lp, LoopInfo *LI, ScalarEvolution *SE,
                                   const DataLayout &DL,
                                   const WRNVecLoopNode *WRL, VPlan *Plan,
                                   VPOVectorizationLegality *Legal)
    : TheLoop(Lp), LI(LI), SE(SE), WRLp(WRL), Plan(Plan), Legal(Legal) {
  // TODO: Turn Verifier pointer into an object when Patch #3 of Patch Series
  // #1 lands into VPO and VPlanHCFGBuilderBase is removed.
  Verifier = std::make_unique<VPlanVerifier>(Lp, LI, DL);
  assert((!WRLp || WRLp->getTheLoop<Loop>() == TheLoop) &&
         "Inconsistent Loop information");
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
      singleExitWhileLoopCanonicalization(VPL);
      mergeLoopExits(VPL);
      LLVM_DEBUG(Verifier->verifyLoops(Plan, VPDomTree, Plan->getVPLoopInfo()));
    }
#if INTEL_CUSTOMIZATION
    if (VPlanPrintAfterLoopMassaging) {
      errs() << "Print after loop massaging:\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      Plan->dump();
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    }

    if (VPlanDotLoopMassaging) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      outs() << *Plan;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    }
#endif /* INTEL_CUSTOMIZATION */

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

#if INTEL_CUSTOMIZATION
  if (VPlanPrintPlainCFG) {
    errs() << "Print after buildPlainCFG\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    Plan->dump();
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
#endif /* INTEL_CUSTOMIZATION */

  // Prepare/simplify CFG for hierarchical CFG construction
  simplifyPlainCFG();

#if INTEL_CUSTOMIZATION
  if (VPlanPrintSimplifyCFG) {
    errs() << "Print after simplify plain CFG\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    Plan->dump();
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }

  if (VPlanDotPlainCFG) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    outs() << *Plan;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
#endif

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

#if INTEL_CUSTOMIZATION
  if (VPlanPrintHCFG) {
    errs() << "Print after building H-CFG:\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    Plan->dump();
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
  LLVM_DEBUG(Plan->dumpLivenessInfo(dbgs()));
#endif
}

class PrivatesListCvt;

namespace {
// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// VPInstructions.
class PlainCFGBuilder {

  /// Outermost loop of the input loop nest.
  Loop *TheLoop = nullptr;

  LoopInfo *LI = nullptr;

  VPlan *Plan = nullptr;

  // Builder of the VPlan instruction-level representation.
  VPBuilder VPIRBuilder;

  // NOTE: The following maps are intentionally destroyed after the plain CFG
  // construction because subsequent VPlan-to-VPlan transformation may
  // invalidate them.
  // Map incoming BasicBlocks to their newly-created VPBasicBlocks.
  DenseMap<BasicBlock *, VPBasicBlock *> BB2VPBB;
  // Map incoming Value definitions to their newly-created VPValues.
  DenseMap<Value *, VPValue *> IRDef2VPValue;
  /// Map the branches to the condition VPInstruction they are controlled by
  /// (Possibly at a different VPBB).
  DenseMap<Value *, VPValue *> BranchCondMap;

  // Hold phi node's that need to be fixed once the plain CFG has been built.
  SmallVector<PHINode *, 8> PhisToFix;

  // Auxiliary functions
  void setVPBBPredsFromBB(VPBasicBlock *VPBB, BasicBlock *BB);
  void fixPhiNodes();
  VPBasicBlock *getOrCreateVPBB(BasicBlock *BB);
  bool isExternalDef(Value *Val) const;
  // Check whether Val has uses outside the vectorized loop and create
  // VPExternalUse-s for NewVPInst accordingly.
  void addExternalUses(Value *Val, VPValue *NewVPInst);

  void createVPInstructionsForVPBB(VPBasicBlock *VPBB, BasicBlock *BB);

  // Create a VPInstruction based on the input IR instruction.
  VPInstruction *createVPInstruction(Instruction *Inst);

  // Check if the given IR instruction is present in the loop-body.
  bool loopContains(Instruction *Inst) {
    assert(Inst && "Expect a non-null instruction passed into this function.");
    return TheLoop->contains(Inst);
  }

  // Reset the insertion point.
  void resetInsertPoint() { VPIRBuilder.clearInsertionPoint(); }

public:
  friend PrivatesListCvt;

  PlainCFGBuilder(Loop *Lp, LoopInfo *LI, VPlan *Plan)
      : TheLoop(Lp), LI(LI), Plan(Plan) {}

  void buildPlainCFG();

  void
  convertEntityDescriptors(LoopVectorizationLegality *Legal,
                           VPlanHCFGBuilder::VPLoopEntityConverterList &Cvts);
  VPValue *getOrCreateVPOperand(Value *IRVal);
};
} // anonymous namespace

// Set predecessors of \p VPBB in the same order as they are in LLVM \p BB.
void PlainCFGBuilder::setVPBBPredsFromBB(VPBasicBlock *VPBB, BasicBlock *BB) {
  SmallVector<VPBasicBlock *, 8> VPBBPreds;
  // Collect VPBB predecessors.
  for (BasicBlock *Pred : predecessors(BB))
    VPBBPreds.push_back(getOrCreateVPBB(Pred));

  VPBB->setPredecessors(VPBBPreds);
}

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
    Descriptor.setStartPhi(dyn_cast<VPInstruction>(
        Builder.getOrCreateVPOperand(CurValue.first)));
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
      (void)IndTy;
      assert((IndTy->isIntegerTy() || IndTy->isPointerTy()) &&
             "unexpected induction type");
      Descriptor.setInductionBinOp(nullptr);
      Descriptor.setBinOpcode(Instruction::Add);
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
    if (IndTy->isIntegerTy())
      Descriptor.setKind(InductionDescriptor::IK_IntInduction);
    else if (IndTy->isPointerTy()) {
      Descriptor.setKind(InductionDescriptor::IK_PtrInduction);
      assert(isa<Instruction>(CurValue.first) &&
             "Linear descriptor is not an instruction.");
      const DataLayout &DL =
          cast<Instruction>(CurValue.first)->getModule()->getDataLayout();
      StepTy = DL.getIntPtrType(IndTy);
    } else {
      assert(IndTy->isFloatingPointTy() && "unexpected induction type");
      Descriptor.setKind(InductionDescriptor::IK_FpInduction);
    }
    Value *Cstep = ConstantInt::get(StepTy, CurValue.second);
    Descriptor.setStep(Builder.getOrCreateVPOperand(Cstep));

    Descriptor.setInductionBinOp(nullptr);
    Descriptor.setBinOpcode(Instruction::Add);
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
      return Builder.loopContains(Inst) ||
            (isTrivialPointerAliasingInst(Inst) &&
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

// Set operands to VPInstructions representing phi nodes from the input IR.
// VPlan Phi nodes were created without operands in a previous step of the H-CFG
// construction because those operands might not have been created in VPlan at
// that time despite the RPO traversal. This function expects all the
// instructions to have a representation in VPlan so operands of VPlan phis can
// be properly set.
void PlainCFGBuilder::fixPhiNodes() {
  for (auto *Phi : PhisToFix) {
    assert(IRDef2VPValue.count(Phi) && "Missing VPInstruction for PHINode.");
    VPValue *VPVal = IRDef2VPValue[Phi];
#if INTEL_CUSTOMIZATION
    assert(isa<VPPHINode>(VPVal) && "Expected VPPHINode for phi node.");
    auto *VPPhi = cast<VPPHINode>(VPVal);
    assert(VPPhi->getNumOperands() == 0 &&
           "Expected VPInstruction with no operands.");

    for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I)
      VPPhi->addIncoming(getOrCreateVPOperand(Phi->getIncomingValue(I)),
                         getOrCreateVPBB(Phi->getIncomingBlock(I)));
#else
    assert(isa<VPInstruction>(VPVal) && "Expected VPInstruction for phi node.");
    auto *VPPhi = cast<VPInstruction>(VPVal);
    for (Value *Op : Phi->operands())
      VPPhi->addOperand(getOrCreateVPOperand(Op));
#endif // INTEL_CUSTOMIZATION
  }
}

// Create a new empty VPBasicBlock for an incoming BasicBlock or retrieve an
// existing one if it was already created.
VPBasicBlock *PlainCFGBuilder::getOrCreateVPBB(BasicBlock *BB) {

  VPBasicBlock *VPBB;
  auto BlockIt = BB2VPBB.find(BB);

  if (BlockIt == BB2VPBB.end()) {
    // New VPBB
    LLVM_DEBUG(dbgs() << "Creating VPBasicBlock for " << BB->getName() << "\n");
    VPBB = new VPBasicBlock(VPlanUtils::createUniqueName("BB"), Plan);
    BB2VPBB[BB] = VPBB;
    VPBB->setOriginalBB(BB);
    Plan->insertAtBack(VPBB);
  } else {
    // Retrieve existing VPBB
    VPBB = BlockIt->second;
  }

  return VPBB;
}

// Return true if \p Val is considered an external definition in the context of
// the plain CFG construction.
//
// An external definition is either:
#if INTEL_CUSTOMIZATION
// 1. A Value that is neither a Constant nor an Instruction.
#else
// 1. A Value that is not an Instruction. This will be refined in the future.
#endif
// 2. An Instruction that is outside of the CFG snippet represented in VPlan.
// However, since we don't represent loop Instructions in loop PH/Exit as
// VPInstructions during plain CFG construction, those are also considered
// external definitions in this particular context.
bool PlainCFGBuilder::isExternalDef(Value *Val) const {
#if INTEL_CUSTOMIZATION
  assert(!isa<Constant>(Val) &&
         "Constants should have been processed separately.");
  assert(!isa<MetadataAsValue>(Val) &&
         "MetadataAsValue should have been processed separately.");
#endif
  // All the Values that are not Instructions are considered external
  // definitions for now.
  Instruction *Inst = dyn_cast<Instruction>(Val);
  if (!Inst)
    return true;

  // Check whether Instruction definition is within the loop nest.
  return !TheLoop->contains(Inst);
}

// Check whether any use of Val is outside
void PlainCFGBuilder::addExternalUses(Value *Val, VPValue *NewVPInst) {
  for (User *U : Val->users())
    if (auto Inst = dyn_cast<Instruction>(U))
      if (!TheLoop->contains(Inst)) {
        // LLVM IR loop must be in LCSSA form.
        VPExternalUse *User = Plan->getVPExternalUse(cast<PHINode>(Inst));
        User->addOperandWithUnderlyingValue(NewVPInst, Val);
      }
}

// Create a new VPValue or retrieve an existing one for the Instruction's
// operand \p IRVal. This function must only be used to create/retrieve VPValues
// for *Instruction's operands* and not to create regular VPInstruction's. For
// the latter, please, look at 'createVPInstructionsForVPBB'.
VPValue *PlainCFGBuilder::getOrCreateVPOperand(Value *IRVal) {
#if INTEL_CUSTOMIZATION
  // Constant operand
  if (Constant *IRConst = dyn_cast<Constant>(IRVal))
    return Plan->getVPConstant(IRConst);

  if (MetadataAsValue *MDAsValue = dyn_cast<MetadataAsValue>(IRVal))
    return Plan->getVPMetadataAsValue(MDAsValue);
#endif

  auto VPValIt = IRDef2VPValue.find(IRVal);
  if (VPValIt != IRDef2VPValue.end())
    // Operand has an associated VPInstruction or VPValue that was previously
    // created.
    return VPValIt->second;

#if INTEL_CUSTOMIZATION
  // Operand is not Constant or MetadataAsValue and doesn't have a previously
  // created VPInstruction/VPValue. This means that operand is:
#else
  // Operand doesn't have a previously created VPInstruction/VPValue. This
  // means that operand is:
#endif
  //   A) a definition external to VPlan,
  //   B) any other Value without specific representation in VPlan.
  // For now, we use VPValue to represent A and B and classify both as external
  // definitions. We may introduce specific VPValue subclasses for them in the
  // future.
  assert(isExternalDef(IRVal) && "Expected external definition as operand.");
  // A and B: Create VPValue and add it to the pool of external definitions and
  // to the Value->VPValue map.
  VPExternalDef *ExtDef = Plan->getVPExternalDef(IRVal);
  IRDef2VPValue[IRVal] = ExtDef;
  return ExtDef;
}

VPInstruction *PlainCFGBuilder::createVPInstruction(Instruction *Inst) {

  if (auto *Br = dyn_cast<BranchInst>(Inst)) {
    // Branch instruction is not explicitly represented in VPlan but we need
    // to represent its condition bit when it's conditional.
    if (Br->isConditional())
      getOrCreateVPOperand(Br->getCondition());

    // Skip the rest of the Instruction processing for Branch instructions.
    return nullptr;
  }

  VPInstruction *NewVPInst{nullptr};
  if (auto *Phi = dyn_cast<PHINode>(Inst)) {
    // Phi node's operands may have not been visited at this point. We create
    // an empty VPInstruction that we will fix once the whole plain CFG has
    // been built.
#if INTEL_CUSTOMIZATION
    NewVPInst = cast<VPInstruction>(VPIRBuilder.createPhiInstruction(Inst));
#else
    NewVPInst = cast<VPInstruction>(
        VPIRBuilder.createNaryOp(Inst->getOpcode(), {} /*No operands*/, Inst));
#endif // INTEL_CUSTOMIZATION
    PhisToFix.push_back(Phi);
    return NewVPInst;
  } else {
    // Translate LLVM-IR operands into VPValue operands and set them in the
    // new VPInstruction.
    SmallVector<VPValue *, 4> VPOperands;
    for (Value *Op : Inst->operands())
      VPOperands.push_back(getOrCreateVPOperand(Op));

#if INTEL_CUSTOMIZATION
    if (CmpInst *CI = dyn_cast<CmpInst>(Inst)) {
      assert(VPOperands.size() == 2 && "Expected 2 operands in CmpInst.");
      NewVPInst = VPIRBuilder.createCmpInst(VPOperands[0], VPOperands[1], CI);
    } else if (auto *GEP = dyn_cast<GetElementPtrInst>(Inst)) {
      // Build VPGEPInstruction to represent GEP instructions
      SmallVector<VPValue *, 3> IdxList(VPOperands.begin() + 1,
                                        VPOperands.end());
      if (GEP->isInBounds())
        NewVPInst = VPIRBuilder.createInBoundsGEP(VPOperands[0], IdxList, Inst);
      else
        NewVPInst = VPIRBuilder.createGEP(VPOperands[0], IdxList, Inst);
    } else
#endif
      // Build VPInstruction for any arbitraty Instruction without specific
      // representation in VPlan.
      NewVPInst = cast<VPInstruction>(VPIRBuilder.createNaryOp(
          Inst->getOpcode(), Inst->getType(), VPOperands, Inst));
  }
  return NewVPInst;
}

// Create new VPInstructions in a VPBasicBlock, given its BasicBlock
// counterpart. This function must be invoked in RPO so that the operands of a
// VPInstruction in \p BB have been visited before. VPInstructions representing
// Phi nodes are created without operands to honor the RPO traversal. They will
// be fixed later by 'fixPhiNodes'.
void PlainCFGBuilder::createVPInstructionsForVPBB(VPBasicBlock *VPBB,
                                                  BasicBlock *BB) {
  VPIRBuilder.setInsertPoint(VPBB);
  for (Instruction &InstRef : *BB) {
    // There shouldn't be any VPValue for Inst at this point. Otherwise, we
    // visited Inst when we shouldn't, breaking the RPO traversal order.
    assert(!IRDef2VPValue.count(&InstRef) &&
           "Instruction shouldn't have been visited.");
    VPInstruction *NewVPInst = createVPInstruction(&InstRef);
    // createVPInstruction can return nullptr in case of a BranchInst.
    if (NewVPInst) {
      if (TheLoop->contains(&InstRef))
        addExternalUses(&InstRef, NewVPInst);

      IRDef2VPValue[&InstRef] = NewVPInst;
    }
  }
}

void PlainCFGBuilder::buildPlainCFG() {
  // 1. Scan the body of the loop in a topological order to visit each basic
  // block after having visited its predecessor basic blocks.Create a VPBB for
  // each BB and link it to its successor and predecessor VPBBs. Note that
  // predecessors must be set in the same order as they are in the incomming IR.
  // Otherwise, there might be problems with existing phi nodes and algorithms
  // based on predecessors traversal.

  // Create loop PH. PH needs to be explicitly processed since it's not taken
  // into account by LoopBlocksDFS below. Since the loop PH may contain any
  // Instruction, related or not to the loop nest, we do not create
  // VPInstructions for them. Those Instructions used within the loop nest will
  // be modeled as external definitions.
  BasicBlock *PreheaderBB = TheLoop->getLoopPreheader();
  assert((PreheaderBB->getTerminator()->getNumSuccessors() == 1) &&
         "Unexpected loop preheader");
  VPBasicBlock *PreheaderVPBB = getOrCreateVPBB(PreheaderBB);
  // Create empty VPBB for Loop H so that we can link PH->H. H's VPInstructions
  // will be created during RPO traversal.
  VPBasicBlock *HeaderVPBB = getOrCreateVPBB(TheLoop->getHeader());
  // Preheader's predecessors will be set during the loop RPO traversal below.
  PreheaderVPBB->setOneSuccessor(HeaderVPBB);

  LoopBlocksRPO RPO(TheLoop);
  RPO.perform(LI);

  for (BasicBlock *BB : RPO) {
    // Create or retrieve the VPBasicBlock for this BB and create its
    // VPInstructions.
    VPBasicBlock *VPBB = getOrCreateVPBB(BB);
    createVPInstructionsForVPBB(VPBB, BB);

    // Set VPBB successors. We create empty VPBBs for successors if they don't
    // exist already. Instructions will be created when the successor is visited
    // during the RPO traversal.
    Instruction *TI = BB->getTerminator();
    assert(TI && "Terminator expected");
    unsigned NumSuccs = TI->getNumSuccessors();

    if (NumSuccs == 1) {
      VPBasicBlock *SuccVPBB = getOrCreateVPBB(TI->getSuccessor(0));
      assert(SuccVPBB && "VPBB Successor not found");
      VPBB->setOneSuccessor(SuccVPBB);
      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));
    } else if (NumSuccs == 2) {
      VPBasicBlock *SuccVPBB0 = getOrCreateVPBB(TI->getSuccessor(0));
      assert(SuccVPBB0 && "Successor 0 not found");
      VPBasicBlock *SuccVPBB1 = getOrCreateVPBB(TI->getSuccessor(1));
      assert(SuccVPBB1 && "Successor 1 not found");

      // Set VPBB's condition bit.
      assert(isa<BranchInst>(TI) && "Unsupported terminator!");
      auto *Br = cast<BranchInst>(TI);
      Value *BrCond = Br->getCondition();
#if INTEL_CUSTOMIZATION
      VPValue *VPCondBit;
      if (Constant *ConstBrCond = dyn_cast<Constant>(BrCond))
        // Create new VPConstant for constant branch condition.
        VPCondBit = Plan->getVPConstant(ConstBrCond);
      else {
        // Look up the branch condition to get the corresponding VPValue
        // representing the condition bit in VPlan (which may be in another
        // VPBB).
        assert(IRDef2VPValue.count(BrCond) &&
               "Missing condition bit in IRDef2VPValue!");
        VPCondBit = IRDef2VPValue[BrCond];
      }
#else
      // Look up the branch condition to get the corresponding VPValue
      // representing the condition bit in VPlan (which may be in another VPBB).
      assert(IRDef2VPValue.count(BrCond) &&
             "Missing condition bit in IRDef2VPValue!");
      VPValue *VPCondBit = IRDef2VPValue[BrCond];
#endif
      VPBB->setTwoSuccessors(VPCondBit, SuccVPBB0, SuccVPBB1);

      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));
      VPBB->setFBlock(TI->getSuccessor(1));

    } else {
      llvm_unreachable("Number of successors not supported");
    }

    // Set VPBB predecessors in the same order as they are in the incoming BB.
    setVPBBPredsFromBB(VPBB, BB);
  }

  // 2. Process outermost loop exit. We created an empty VPBB for the loop
  // exit BBs during the RPO traversal of the loop nest but their predecessors
  // have to be properly set. Since a loop exit may contain any Instruction,
  // related or not to the loop nest, we do not create VPInstructions for them.
  SmallVector<BasicBlock *, 2> LoopExits;
  TheLoop->getUniqueExitBlocks(LoopExits);
  for (BasicBlock *BB : LoopExits) {
    VPBasicBlock *VPBB = BB2VPBB[BB];
    // Loop exit was already set as successor of the loop exiting BB.
    // We only set its predecessor VPBB now.
    setVPBBPredsFromBB(VPBB, BB);
  }

  // 3. The whole CFG has been built at this point so all the input Values must
  // have a VPlan couterpart. Fix VPlan phi nodes by adding their corresponding
  // VPlan operands.
  fixPhiNodes();

  // 4. Set the EntryBlock and the ExitBlock of SESE region.
  // Create EntryBlock.
  VPBasicBlock *PlanEntryBB =
      new VPBasicBlock(VPlanUtils::createUniqueName("BB"), Plan);
  Plan->insertAtFront(PlanEntryBB);
  VPBlockUtils::connectBlocks(PlanEntryBB, PreheaderVPBB);

  // Create ExitBlock.
  VPBasicBlock *NewPlanExitBB =
      new VPBasicBlock(VPlanUtils::createUniqueName("BB"), Plan);
  Plan->insertAtBack(NewPlanExitBB);

  // Update CFG for ExitBlock.
  if (LoopExits.size() == 1) {
    VPBasicBlock *LoopExitVPBB = BB2VPBB[LoopExits.front()];
    VPBlockUtils::connectBlocks(LoopExitVPBB, NewPlanExitBB);
  } else {
    // If there are multiple exits in the outermost loop, we need another dummy
    // block as landing pad for all of them.
    assert(LoopExits.size() > 1 && "Wrong number of exit blocks");

    VPBasicBlock *LandingPad =
        new VPBasicBlock(VPlanUtils::createUniqueName("BB"), Plan);
    LandingPad->insertBefore(NewPlanExitBB);

    // Connect multiple exits to landing pad
    for (auto ExitBB : make_range(LoopExits.begin(), LoopExits.end())) {
      VPBasicBlock *ExitVPBB = BB2VPBB[ExitBB];
      VPBlockUtils::connectBlocks(ExitVPBB, LandingPad);
    }

    // Connect landing pad to ExitBlock.
    VPBlockUtils::connectBlocks(LandingPad, NewPlanExitBB);
  }

  return;
}

void VPlanHCFGBuilder::buildPlainCFG(VPLoopEntityConverterList &Cvts) {
  PlainCFGBuilder PCFGBuilder(TheLoop, LI, Plan);
  PCFGBuilder.buildPlainCFG();
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
