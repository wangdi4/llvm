//===-- IntelVPlanHCFGBuilderHIR.cpp --------------------------------------===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file extends VPlanCFGBuilderBase with support to build a hierarchical
/// CFG from HIR.
///
/// The algorithm consist of a Visitor that traverses HLNode's (lexical links)
/// in topological order and builds a plain CFG out of them. It returns a region
/// (TopRegion) containing the plain CFG.
///
/// It is inspired by AVR-based VPOCFG algorithm and uses a non-recursive
/// visitor to explicitly handle visits of "compound" HLNode's (HLIfs, HLLoop,
/// HLSwitch) and trigger the creation-closure of VPBasicBlocks.
///
/// Creation/closure of VPBasicBlock's is triggered by:
///   1) HLLoop Pre-header
///   *) HLoop Header
///   *) End of HLLoop body
///   *) HLoop Exit (Postexit)
///   *) If-then branch
///   *) If-else branch
///   *) End of HLIf
///   *) HLLabel
///   *) HLGoto
///
/// The algorithm keeps an active VPBasicBlock (ActiveVPBB) that is populated
/// with "instructions". When one of the previous conditions is met, a new
/// active VPBasicBlock is created and connected to its predecessors. A list of
/// VPBasicBlock (Predecessors) holds the predecessors to be connected to the
/// new active VPBasicBlock when it is created HLGoto needs special treatment
/// since its VPBasicBlock is not reachable from an HLLabel. For that reason, a
/// VPBasicBlock ending with an HLGoto is connected to its successor when HLGoto
/// is visited.
///
/// TODO's:
///   - Outer loops.
///   - Expose ZTT for inner loops.
///   - HLSwitch
///   - Loops with multiple exits.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanHCFGBuilderHIR.h"
#include "IntelVPLoopRegionHIR.h"
#include "IntelVPlanBuilderHIR.h"
#include "IntelVPlanDecomposerHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"

#define DEBUG_TYPE "VPlanHCFGBuilder"

using namespace llvm;
using namespace vpo;

// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// VPInstructions. Return VPRegionBlock that encloses all the VPBasicBlock's of
// the plain CFG.
class PlainCFGBuilderHIR : public HLNodeVisitorBase {
  friend HLNodeVisitor<PlainCFGBuilderHIR, false /*Recursive*/>;

private:
  /// Outermost loop of the input loop nest.
  HLLoop *TheLoop;

  VPlan *Plan;

  /// Map between loop header VPBasicBlock's and their respective HLLoop's. It
  /// is populated in this phase to keep the information necessary to create
  /// VPLoopRegionHIR's later in the H-CFG construction process.
  SmallDenseMap<VPBasicBlock *, HLLoop *> &Header2HLLoop;

  /// Output TopRegion.
  VPRegionBlock *TopRegion = nullptr;
  /// Number of VPBasicBlocks in TopRegion.
  unsigned TopRegionSize = 0;

  /// Hold the set of dangling predecessors to be connected to the next active
  /// VPBasicBlock.
  std::deque<VPBasicBlock *> Predecessors;

  /// Hold a pointer to the current HLLoop being processed.
  HLLoop *CurrentHLp = nullptr;

  /// Hold the VPBasicBlock that is being populated with instructions. Null
  /// value indicates that a new active VPBasicBlock has to be created.
  VPBasicBlock *ActiveVPBB = nullptr;

  /// Hold the VPBasicBlock that will be used as a landing pad for loops with
  /// multiple exits. If the loop is a single-exit loop, no landing pad
  /// VPBasicBlock is created.
  VPBasicBlock *MultiExitLandingPad = nullptr;

  /// Map between HLNode's that open a VPBasicBlock and such VPBasicBlock's.
  DenseMap<HLNode *, VPBasicBlock *> HLN2VPBB;

  /// Utility to create VPInstructions out of a HLNode.
  VPDecomposerHIR Decomposer;

  VPBasicBlock *createOrGetVPBB(HLNode *HNode = nullptr);
  void connectVPBBtoPreds(VPBasicBlock *VPBB);
  void updateActiveVPBB(HLNode *HNode = nullptr, bool IsPredecessor = true);

  // Visitor methods
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  void visit(HLLoop *HLp);
  void visit(HLIf *HIf);
  void visit(HLSwitch *HSw) {
    llvm_unreachable("Switches are not supported yet.");
  };
  void visit(HLInst *HInst);
  void visit(HLGoto *HGoto);
  void visit(HLLabel *HLabel);

public:
  PlainCFGBuilderHIR(HLLoop *Lp, const DDGraph &DDG, VPlan *Plan,
                     SmallDenseMap<VPBasicBlock *, HLLoop *> &H2HLLp)
      : TheLoop(Lp), Plan(Plan), Header2HLLoop(H2HLLp),
        Decomposer(Plan, Lp, DDG) {}

  /// Build a plain CFG for an HLLoop loop nest. Return the TopRegion containing
  /// the plain CFG.
  VPRegionBlock *buildPlainCFG();
};

/// Retrieve an existing VPBasicBlock for \p HNode. It there is no existing
/// VPBasicBlock, a new VPBasicBlock is created and mapped to \p HNode. If \p
/// HNode is null, the new VPBasicBlock is not mapped to any HLNode.
VPBasicBlock *PlainCFGBuilderHIR::createOrGetVPBB(HLNode *HNode) {

  // Auxiliary function that creates an empty VPBasicBlock, set its parent to
  // TopRegion and increases TopRegion's size.
  auto createVPBB = [&]() -> VPBasicBlock * {
    VPBasicBlock *NewVPBB =
        new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
    NewVPBB->setParent(TopRegion);
    ++TopRegionSize;

    return NewVPBB;
  };

  if (!HNode) {
    // No HLNode associated to this VPBB.
    return createVPBB();
  } else {
    // Try to retrieve existing VPBB for this HLNode. Otherwise, create a new
    // VPBB and add it to the map.
    auto BlockIt = HLN2VPBB.find(HNode);

    if (BlockIt == HLN2VPBB.end()) {
      // New VPBB
      // TODO: Print something more useful.
      LLVM_DEBUG(dbgs() << "Creating VPBasicBlock for " << HNode->getNumber()
                        << "\n");
      VPBasicBlock *VPBB = createVPBB();
      HLN2VPBB[HNode] = VPBB;
      // NewVPBB->setOriginalBB(BB);
      return VPBB;
    } else {
      // Retrieve existing VPBB
      return BlockIt->second;
    }
  }
}

/// Connect \p VPBB to all the predecessors in Predecessors and clear
/// Predecessors.
void PlainCFGBuilderHIR::connectVPBBtoPreds(VPBasicBlock *VPBB) {

  for (VPBasicBlock *Pred : Predecessors) {
    Pred->appendSuccessor(VPBB);
    VPBB->appendPredecessor(Pred);
  }

  Predecessors.clear();
}

// Update active VPBasicBlock only when this is null. It creates a new active
// VPBasicBlock, connect it to existing predecessors, set it as new insertion
// point in VPHIRBUilder and, if \p ISPredecessor is true, add it as predecessor
// of the (future) subsequent active VPBasicBlock's.
void PlainCFGBuilderHIR::updateActiveVPBB(HLNode *HNode, bool IsPredecessor) {
  if (!ActiveVPBB) {
    ActiveVPBB = createOrGetVPBB(HNode);
    connectVPBBtoPreds(ActiveVPBB);

    if (IsPredecessor)
      Predecessors.push_back(ActiveVPBB);
  }
}

void PlainCFGBuilderHIR::visit(HLLoop *HLp) {
  assert((HLp->isDo() || HLp->isDoMultiExit()) && HLp->isNormalized() &&
         "Unsupported HLLoop type.");
  // Set HLp as current loop before we visit its children.
  HLLoop *PrevCurrentHLp = CurrentHLp;
  CurrentHLp = HLp;

  // TODO: Print something more useful.
  LLVM_DEBUG(dbgs() << "Visiting HLLoop: " << HLp->getNumber() << "\n");

  // - ZTT for inner loops -
  // TODO: isInnerMost(), ztt_pred_begin/end

  // - Loop PH -
  // Force creation of a new VPBB for PH.
  ActiveVPBB = nullptr;

  // TODO: DDGraph doesn't include HLLoop PH and Exit at this point. As a
  // workaround, we push them outside of the region represented in VPlan and
  // create an empty VPBasicBlock for them.
  // if (HLp->hasPreheader()) {
  //  HLNodeUtils::visitRange<false /*Recursive*/>(
  //      *this /*visitor*/, HLp->pre_begin(), HLp->pre_end());

  //  assert(ActiveVPBB == HLN2VPBB[&*HLp->pre_begin()] &&
  //         "Loop PH generates more than one VPBB?");
  //} else
  // There is no PH in HLLoop. Create dummy VPBB as PH. We could introduce
  // this dummy VPBB in simplifyPlainCFG, but according to the design for
  // LLVM-IR, we expect to have a loop with a PH as input. It's then better to
  // introduce the dummy PH here.
  updateActiveVPBB();

  VPBasicBlock *Preheader = ActiveVPBB;

  // - Loop Body -
  if (HLp->isMultiExit()) {
    // FIXME: In outer loop vectorization scenarios, more than one loop can be a
    // multi-exit loop. We need to use a stack to store the landing pad of each
    // multi-exit loop in the loop nest.
    assert(!MultiExitLandingPad && "Only one multi-exit loops is supported!");
    // Create a new landing pad for all the multiple exits.
    MultiExitLandingPad = createOrGetVPBB();
  }

  // Force creation of a new VPBB for loop H.
  ActiveVPBB = nullptr;
  updateActiveVPBB();
  VPBasicBlock *Header = ActiveVPBB;
  assert(Header && "Expected VPBasicBlock for loop header.");

  // Map loop header VPBasicBlock with HLLoop for later loop region detection.
  Header2HLLoop[Header] = HLp;

  // Materialize the Loop IV and IV Start.
  Decomposer.createLoopIVAndIVStart(HLp, Preheader);

  // Visit loop body
  HLNodeUtils::visitRange<false /*Recursive*/>(
      *this /*visitor*/, HLp->child_begin(), HLp->child_end());

  // Loop latch: an HLoop will always have a single latch that will also be an
  // exiting block. Keep track of it. If there is no active VPBB, we have to
  // create a new one.
  updateActiveVPBB();
  VPBasicBlock *Latch = ActiveVPBB;

  // Materialize IV Next and bottom test in the loop latch. Connect Latch to
  // Header and set Latch condition bit.
  VPValue *LatchCondBit =
      Decomposer.createLoopIVNextAndBottomTest(HLp, Preheader, Latch);
  VPBlockUtils::connectBlocks(Latch, Header);
  Latch->setCondBit(LatchCondBit, Plan);

  // - Loop Exits -
  // Force creation of a new VPBB for Exit.
  ActiveVPBB = nullptr;

  // TODO: DDGraph doesn't include HLLoop PH and Exit at this point. As a
  // workaround, we push them outside of the region represented in VPlan and
  // create an empty VPBasicBlock for them.
  // if (HLp->hasPostexit()) {
  //  HLNodeUtils::visitRange<false /*Recursive*/>(
  //      *this /*visitor*/, HLp->post_begin(), HLp->post_end());

  //  assert(ActiveVPBB == HLN2VPBB[&*HLp->post_begin()] &&
  //         "Loop Exit generates more than one VPBB?");
  //} else
  // There is no Exit in HLLoop. Create dummy VPBB as Exit (see comment for
  // dummy PH).
  updateActiveVPBB();

  if (HLp->isMultiExit()) {
    // Connect loop's regular exit to multi-exit landing pad and set landing pad
    // as new predecessor for subsequent VPBBs.
    connectVPBBtoPreds(MultiExitLandingPad);
    Predecessors.push_back(MultiExitLandingPad);
    ActiveVPBB = MultiExitLandingPad;
  }

  // At this point, all the VPBasicBlocks have been built and all the
  // VPInstructions have been created for this loop. It's time to fix
  // VPInstructions representing a semi-phi operation.
  Decomposer.fixPhiNodes();

  // Restore previous current HLLoop.
  CurrentHLp = PrevCurrentHLp;
}

void PlainCFGBuilderHIR::visit(HLIf *HIf) {

  // - Condition -
  // We do not create a new active  VPBasicBlock for HLIf predicates
  // (condition). We reuse the previous one (if possible).
  updateActiveVPBB(HIf);
  VPBasicBlock *ConditionVPBB = ActiveVPBB;

  // Create (single, not decomposed) VPInstruction for HLIf's predicate and set
  // it as condition bit of the active VPBasicBlock.
  // TODO: Remove "not decomposed" when decomposing HLIfs.
  VPInstruction *CondBit =
      Decomposer.createVPInstructionsForNode(HIf, ActiveVPBB);
  ConditionVPBB->setCondBit(CondBit, Plan);

  // - Then branch -
  // Force creation of a new VPBB for Then branch.
  ActiveVPBB = nullptr;
  HLNodeUtils::visitRange<false /*Recursive*/>(
      *this /*visitor*/, HIf->then_begin(), HIf->then_end());

  // - Else branch -
  if (HIf->hasElseChildren()) {
    // Hold predecessors from Then branch to be used after HLIf visit and before
    // visiting else branch.
    SmallVector<VPBasicBlock *, 2> ThenOutputPreds(Predecessors.begin(),
                                                   Predecessors.end());
    // Clear Predecessors before Else branch visit (we don't want to connect
    // Then branch VPBasicBlock's with Else branch VPBasicBlock's) and add HLIf
    // condition as new predecessor for Else branch.
    Predecessors.clear();
    Predecessors.push_back(ConditionVPBB);

    // Force creation of a new VPBB for Else branch.
    ActiveVPBB = nullptr;
    HLNodeUtils::visitRange<false /*Recursive*/>(
        *this /*visitor*/, HIf->else_begin(), HIf->else_end());

    // Prepend predecessors generated by Then branch to those in Predecessors
    // from Else branch.
    // to be used after HLIf visit.
    Predecessors.insert(Predecessors.begin(), ThenOutputPreds.begin(),
                        ThenOutputPreds.end());
  } else {
    // No Else branch

    // Add ConditionVPBB to Predecessors for HLIf successor. Predecessors
    // contains predecessors from Then branch.
    // TODO: In this order? back or front?
    Predecessors.push_back(ConditionVPBB);
  }

  // Force the creation of a new VPBB for the next HLNode.
  ActiveVPBB = nullptr;
}

void PlainCFGBuilderHIR::visit(HLInst *HInst) {
  // Create new VPBasicBlock if there isn't a reusable one.
  updateActiveVPBB(HInst);

  // Create VPInstructions for HInst.
  Decomposer.createVPInstructionsForNode(HInst, ActiveVPBB);
}

void PlainCFGBuilderHIR::visit(HLGoto *HGoto) {

  // If there is an ActiveVPBB we have to remove it from Predecessors. HLGoto's
  // VPBB and HLLabel's VPBB are connected explicitly in this visit function
  // because they "break" the expected topological order traversal and,
  // therefore, need special treatment.
  if (ActiveVPBB) {
    // If this assert is raised, we would have to remove ActiveVPBB using
    // find/erase (more expensive).
    assert(Predecessors.back() == ActiveVPBB &&
           "Expected ActiveVPBB at the end of Predecessors.");
    Predecessors.pop_back();
  }

  // Create new VPBasicBlock if there isn't a reusable one. If a new ActiveVPBB
  // is created, do not add it to Predecessors (see previous comment).
  updateActiveVPBB(HGoto, false /*IsPredecessor*/);

  HLLabel *Label = HGoto->getTargetLabel();
  VPBasicBlock *LabelVPBB;
  if (HGoto->isExternal() || !HLNodeUtils::contains(CurrentHLp, Label)) {
    // Exiting goto in multi-exit loop. Use multi-exit landing pad as successor
    // of the goto VPBB.
    // TODO: When dealing with multi-loop H-CFGs, landing pad needs to properly
    // dispatch exiting gotos when labels have representation in VPlan. That
    // massaging should happen as a separate simplification step. Currently, all
    // the exiting gotos would go to the landing pad.
    assert(CurrentHLp->isDoMultiExit() && "Expected multi-exit loop!");
    assert(MultiExitLandingPad && "Expected landing pad for multi-exit loop!");

    Decomposer.createVPInstructionsForNode(HGoto, ActiveVPBB);
    LabelVPBB = MultiExitLandingPad;
  } else {
    assert(Label && "Label can't be null!");
    // Goto inside the loop. Create (or get) a new VPBB for HLLabel
    LabelVPBB = createOrGetVPBB(Label);
  }

  // Connect to HLGoto's VPBB to HLLabel's VPBB.
  VPBlockUtils::connectBlocks(ActiveVPBB, LabelVPBB);

  // Force the creation of a new VPBasicBlock for the next HLNode.
  ActiveVPBB = nullptr;
}

void PlainCFGBuilderHIR::visit(HLLabel *HLabel) {
  // Force the creation of a new VPBasicBlock for an HLLabel.
  ActiveVPBB = nullptr;
  updateActiveVPBB(HLabel);
}

VPRegionBlock *PlainCFGBuilderHIR::buildPlainCFG() {
  // Create new TopRegion.
  TopRegion = new VPRegionBlock(VPBlockBase::VPRegionBlockSC,
                                VPlanUtils::createUniqueName("region"));

  // Create a dummy VPBB as TopRegion's Entry.
  assert(!ActiveVPBB && "ActiveVPBB must be null.");
  updateActiveVPBB();
  TopRegion->setEntry(ActiveVPBB);

  // Trigger the visit of the loop nest.
  visit(TheLoop);

  // Create a dummy VPBB as TopRegion's Exit.
  ActiveVPBB = nullptr;
  updateActiveVPBB();
  TopRegion->setExit(ActiveVPBB);

  TopRegion->setSize(TopRegionSize);

  return TopRegion;
}

VPRegionBlock *VPlanHCFGBuilderHIR::buildPlainCFG() {
  PlainCFGBuilderHIR PCFGBuilder(TheLoop, DDG, Plan, Header2HLLoop);
  VPRegionBlock *TopRegion = PCFGBuilder.buildPlainCFG();
  return TopRegion;
}

VPLoopRegion *VPlanHCFGBuilderHIR::createLoopRegion(VPLoop *VPLp) {
  assert(isa<VPBasicBlock>(VPLp->getHeader()) &&
         "Expected VPBasicBlock as Loop header.");
  HLLoop *HLLp = Header2HLLoop[cast<VPBasicBlock>(VPLp->getHeader())];
  assert(HLLp && "Expected HLLoop");
  VPLoopRegion *Loop =
      new VPLoopRegionHIR(VPlanUtils::createUniqueName("loop"), VPLp, HLLp);
  Loop->setReplicator(false /*IsReplicator*/);
  return Loop;
}
