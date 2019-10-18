//===-- IntelVPlanHCFGBuilderHIR.cpp --------------------------------------===//
//
//   Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"

#define DEBUG_TYPE "VPlanHCFGBuilder"

using namespace llvm;
using namespace vpo;

/// Check if the incoming \p Ref matches the original SIMD descriptor DDRef \p
/// DescrRef
static bool isSIMDDescriptorDDRef(const RegDDRef *DescrRef, const DDRef *Ref) {
  assert(DescrRef->isAddressOf() &&
         "Original SIMD descriptor ref is not address of type.");

  // Since we know descriptor ref is always address of type, set address-of to
  // false for equality check. Reset to true after check.
  const_cast<RegDDRef *>(DescrRef)->setAddressOf(false);
  if (DescrRef->getDDRefUtils().areEqual(DescrRef, Ref)) {
    const_cast<RegDDRef *>(DescrRef)->setAddressOf(true);
    return true;
  }

  const_cast<RegDDRef *>(DescrRef)->setAddressOf(true);

  // Special casing for incoming Ref of the form %s which was actually the Base
  // CE of the memref %s[0]
  auto *DescrRefCE = DescrRef->getBaseCE();
  if (auto *BDDR = dyn_cast<BlobDDRef>(Ref)) {
    auto *RefCE = BDDR->getSingleCanonExpr();
    if (DescrRefCE->getCanonExprUtils().areEqual(DescrRefCE, RefCE))
      return true;
  }

  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void HIRVectorizationLegality::dump(raw_ostream &OS) const {
  OS << "HIRLegality Descriptor Lists\n";
  OS << "\n\nHIRLegality PrivatesList:\n";
  for (auto &Pvt : PrivatesList) {
    Pvt.dump();
    OS << "\n";
  }
  OS << "\n\nHIRLegality LinearList:\n";
  for (auto &Lin : LinearList) {
    Lin.dump();
    OS << "\n";
  }
  OS << "\n\nHIRLegality ReductionList:\n";
  for (auto &Red : ReductionList) {
    Red.dump();
    OS << "\n";
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

template <typename DescrType>
DescrType *HIRVectorizationLegality::findDescr(ArrayRef<DescrType> List,
                                               const DDRef *Ref) const {
  for (auto &Descr : List) {
    DescrType *CurrentDescr = const_cast<DescrType *>(&Descr);
    assert(isa<RegDDRef>(CurrentDescr->Ref) &&
           "The original SIMD descriptor Ref is not a RegDDRef.");
    if (isSIMDDescriptorDDRef(cast<RegDDRef>(CurrentDescr->Ref), Ref))
      return CurrentDescr;

    // Check if Ref matches any aliases of current descriptor's ref
    if (CurrentDescr->Aliases.count(Ref))
      return CurrentDescr;
  }

  return nullptr;
}

// Explicit template instantiations for findDescr.
template HIRVectorizationLegality::RedDescr *
HIRVectorizationLegality::findDescr(
    ArrayRef<HIRVectorizationLegality::RedDescr> List, const DDRef *Ref) const;
template HIRVectorizationLegality::PrivDescr *
HIRVectorizationLegality::findDescr(
    ArrayRef<HIRVectorizationLegality::PrivDescr> List, const DDRef *Ref) const;
template HIRVectorizationLegality::LinearDescr *
HIRVectorizationLegality::findDescr(
    ArrayRef<HIRVectorizationLegality::LinearDescr> List,
    const DDRef *Ref) const;

void HIRVectorizationLegality::recordPotentialSIMDDescrUse(DDRef *Ref) {

  DescrWithAliases *Descr = getLinearRednDescriptors(Ref);

  // If Ref does not correspond to linear/reduction then nothing to do
  if (!Descr)
    return;

  assert(isa<RegDDRef>(Descr->Ref) &&
         "The original SIMD descriptor Ref is not a RegDDRef.");
  if (isSIMDDescriptorDDRef(cast<RegDDRef>(Descr->Ref), Ref)) {
    // Ref refers to the original descriptor
    // TODO: should we assert that InitVPValue is not set already?
    Descr->InitValue = Ref;
  } else {
    // Ref is an alias to the original descriptor
    auto AliasIt = Descr->Aliases.find(Ref);
    assert(AliasIt != Descr->Aliases.end() && "Alias not found.");
    DescrValues *Alias = AliasIt->second.get();
    Alias->InitValue = Ref;
  }
}

void HIRVectorizationLegality::recordPotentialSIMDDescrUpdate(
    HLInst *UpdateInst) {
  RegDDRef *Ref = UpdateInst->getLvalDDRef();

  // Instruction does not write into any LVal, bail out of analysis
  if (!Ref)
    return;

  DescrWithAliases *Descr = getLinearRednDescriptors(Ref);

  // If Ref does not correspond to linear/reduction then nothing to do
  if (!Descr)
    return;

  assert(isa<RegDDRef>(Descr->Ref) &&
         "The original SIMD descriptor Ref is not a RegDDRef.");
  if (isSIMDDescriptorDDRef(cast<RegDDRef>(Descr->Ref), Ref)) {
    // Ref refers to the original descriptor
    Descr->UpdateInstructions.push_back(UpdateInst);
  } else {
    // Ref is an alias to the original descriptor
    auto AliasIt = Descr->Aliases.find(Ref);
    assert(AliasIt != Descr->Aliases.end() && "Alias not found.");
    DescrValues *Alias = AliasIt->second.get();
    Alias->UpdateInstructions.push_back(UpdateInst);
  }
}

void HIRVectorizationLegality::findAliasDDRefs(HLNode *ClauseNode,
                                               HLLoop *HLoop) {

  // Container to collect all nodes that are present before HLoop to process for
  // potential aliases
  SmallVector<HLNode *, 8> PreLoopNodes;

  // Collect nodes between the SIMD clause directive and the HLLoop node
  HLNode *CurNode = ClauseNode;
  while (auto *NextNode = CurNode->getNextNode()) {
    if (NextNode == HLoop)
      break;

    LLVM_DEBUG(dbgs() << "PreHLLoop node: "; NextNode->dump(););
    PreLoopNodes.push_back(NextNode);
    CurNode = NextNode;
  }

  // Collect nodes present in HLLoop's preheader
  for (auto &Pre : make_range(HLoop->pre_begin(), HLoop->pre_end())) {
    LLVM_DEBUG(dbgs() << "Preheader node: "; Pre.dump(););
    PreLoopNodes.push_back(&Pre);
  }

  // Process all pre-loop nodes
  for (HLNode *PLN : PreLoopNodes) {
    // Evaluate Rvals of only HLInsts in the pre-loop nodes
    if (auto *HInst = dyn_cast<HLInst>(PLN)) {
      RegDDRef *RVal = HInst->getRvalDDRef();

      // If there is no RVal ignore node
      if (!RVal)
        continue;

      // Check if RVal is any of explicit SIMD descriptors
      DescrWithAliases *Descr = isPrivate(RVal);
      if (Descr == nullptr)
        Descr = isLinear(RVal);
      if (Descr == nullptr)
        Descr = isReduction(RVal);

      // RVal is not a SIMD descriptor, move to next HLInst
      if (Descr == nullptr)
        continue;

      LLVM_DEBUG(dbgs() << "Found an alias for a SIMD descriptor ref ";
                 RVal->dump(); dbgs() << "\n");
      RegDDRef *LVal = HInst->getLvalDDRef();
      assert(LVal && "HLInst in the preheader does not have an Lval.");
      Descr->Aliases[LVal] = std::make_unique<DescrValues>(LVal);
    }
  }
}

const HIRVectorIdioms *
HIRVectorizationLegality::getVectorIdioms(HLLoop *Loop) const {
  IdiomListTy &IdiomList = VecIdioms[Loop];
  if (!IdiomList) {
    IdiomList.reset(new HIRVectorIdioms());
    HIRVectorIdiomAnalysis Analysis;
    Analysis.gatherIdioms(*IdiomList, DDAnalysis->getGraph(Loop), *SRA, Loop);
  }
  return IdiomList.get();
}

bool HIRVectorizationLegality::isMinMaxIdiomTemp(const DDRef *Ref,
                                                 HLLoop *Loop) const {
  auto *Idioms = getVectorIdioms(Loop);
  for (auto &Inst : make_range(Idioms->begin(), Idioms->end()))
    if ((Inst.second == HIRVectorIdioms::IdiomId::MinOrMax ||
         Inst.second == HIRVectorIdioms::IdiomId::MMFirstLastLoc) &&
        DDRefUtils::areEqual(Inst.first->getLvalDDRef(), Ref))
      return true;

  return false;
}

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

  /// Output TopRegion. Owned during the PlainCFG build process, moved
  // afterwards.
  std::unique_ptr<VPRegionBlock> TopRegion;
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

  VPBasicBlock *getOrCreateVPBB(HLNode *HNode = nullptr);
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
                     SmallDenseMap<VPBasicBlock *, HLLoop *> &H2HLLp,
                     HIRVectorizationLegality *HIRLegality)
      : TheLoop(Lp), Plan(Plan), Header2HLLoop(H2HLLp),
        Decomposer(Plan, Lp, DDG, *HIRLegality) {}

  /// Build a plain CFG for an HLLoop loop nest. Return the TopRegion containing
  /// the plain CFG.
  std::unique_ptr<VPRegionBlock> buildPlainCFG();

  /// Convert incoming loop entities to the VPlan format.
  void
  convertEntityDescriptors(HIRVectorizationLegality *Legal,
                           VPlanHCFGBuilder::VPLoopEntityConverterList &CvtVec);
};

/// Retrieve an existing VPBasicBlock for \p HNode. It there is no existing
/// VPBasicBlock, a new VPBasicBlock is created and mapped to \p HNode. If \p
/// HNode is null, the new VPBasicBlock is not mapped to any HLNode.
VPBasicBlock *PlainCFGBuilderHIR::getOrCreateVPBB(HLNode *HNode) {

  // Auxiliary function that creates an empty VPBasicBlock, set its parent to
  // TopRegion and increases TopRegion's size.
  auto createVPBB = [&]() -> VPBasicBlock * {
    VPBasicBlock *NewVPBB =
        new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
    NewVPBB->setParent(TopRegion.get());
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
    ActiveVPBB = getOrCreateVPBB(HNode);
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

  // Visit loop PH only if the loop is not the outermost loop we are
  // vectorizing. DDGraph doesn't include outermost loop PH and Exit at this
  // point so we push them outside of the region represented in VPlan.
  if (HLp != TheLoop && HLp->hasPreheader()) {
    HLNodeUtils::visitRange<false /*Recursive*/>(
        *this /*visitor*/, HLp->pre_begin(), HLp->pre_end());

    assert(ActiveVPBB == HLN2VPBB[&*HLp->pre_begin()] &&
           "Loop PH generates more than one VPBB?");
  } else
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
    MultiExitLandingPad = getOrCreateVPBB();
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
  Latch->setCondBit(LatchCondBit);

  // - Loop Exits -
  // Force creation of a new VPBB for Exit.
  ActiveVPBB = nullptr;

  // Visit loop Exit only if the loop is not the outermost loop we are
  // vectorizing. DDGraph doesn't include outermost loop PH and Exit at this
  // point so we push them outside of the region represented in VPlan.
  if (HLp != TheLoop && HLp->hasPostexit()) {
    HLNodeUtils::visitRange<false /*Recursive*/>(
        *this /*visitor*/, HLp->post_begin(), HLp->post_end());

    assert(ActiveVPBB == HLN2VPBB[&*HLp->post_begin()] &&
           "Loop Exit generates more than one VPBB?");
  } else
    // There is no Exit in HLLoop. Create dummy VPBB as Exit (see comment for
    //  dummy PH).
    updateActiveVPBB();

  if (HLp->isMultiExit()) {
    // Connect loop's regular exit to multi-exit landing pad and set landing pad
    // as new predecessor for subsequent VPBBs.
    connectVPBBtoPreds(MultiExitLandingPad);
    Predecessors.push_back(MultiExitLandingPad);
    ActiveVPBB = MultiExitLandingPad;
  }

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
  ConditionVPBB->setCondBit(CondBit);

  // - Then branch -
  // Force creation of a new VPBB for Then branch even if the Then branch has no
  // children.
  ActiveVPBB = nullptr;
  updateActiveVPBB();
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
    LabelVPBB = getOrCreateVPBB(Label);
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

void VPlanHCFGBuilderHIR::populateVPLoopMetadata(VPLoopInfo *VPLInfo) {
  for (VPLoop *VPL : VPLInfo->getLoopsInPreorder()) {
    auto *Header = cast<VPBasicBlock>(VPL->getHeader());

    assert(Header2HLLoop.count(Header) &&
           "Missing mapping from loop header to HLLoop!");
    const HLLoop *HLoop = Header2HLLoop[Header];

    using TripCountTy = VPLoop::TripCountTy;
    TripCountTy TripCount;
    if (HLoop->isConstTripLoop(&TripCount)) {
      VPL->setKnownTripCount(TripCount);
      return;
    }

    TripCountInfo TCInfo;
    if (TripCountTy MaxTripCount = HLoop->getMaxTripCountEstimate()) {
      TCInfo.MaxTripCount = MaxTripCount;
      LLVM_DEBUG(dbgs() << "Max trip count is " << MaxTripCount
                        << " set by pragma loop count\n");
    }

    unsigned MinTripCount;
    if (HLoop->getPragmaBasedMinimumTripCount(MinTripCount)) {
      TCInfo.MinTripCount = MinTripCount;
      LLVM_DEBUG(dbgs() << "Min trip count is " << TCInfo.MinTripCount
                        << " set by pragma loop count\n");
    }

    unsigned AvgTripCount;
    if (HLoop->getPragmaBasedAverageTripCount(AvgTripCount)) {
      TCInfo.TripCount = AvgTripCount;
      LLVM_DEBUG(dbgs() << "Average trip count is " << AvgTripCount
                        << " set by pragma loop count\n");
    }
    TCInfo.calculateEstimatedTripCount();
    VPL->setTripCountInfo(TCInfo);
  }
}

std::unique_ptr<VPRegionBlock> PlainCFGBuilderHIR::buildPlainCFG() {
  // Create new TopRegion.
  TopRegion = std::make_unique<VPRegionBlock>(
      VPBlockBase::VPRegionBlockSC, VPlanUtils::createUniqueName("region"));

  // Create a dummy VPBB as TopRegion's Entry.
  assert(!ActiveVPBB && "ActiveVPBB must be null.");
  updateActiveVPBB();
  TopRegion->setEntry(ActiveVPBB);

  // Trigger the visit of the loop nest.
  visit(TheLoop);

  // Create a dummy VPBB as TopRegion's Exit.
  ActiveVPBB = nullptr;
  updateActiveVPBB();

  // At this point, all the VPBasicBlocks have been built and all the
  // VPInstructions have been created for the loop nest. It's time to fix
  // VPInstructions representing a PHI operation.
  Decomposer.fixPhiNodes();

  TopRegion->setExit(ActiveVPBB);
  TopRegion->setSize(TopRegionSize);

  return std::move(TopRegion);
}

VPlanHCFGBuilderHIR::VPlanHCFGBuilderHIR(const WRNVecLoopNode *WRL, HLLoop *Lp,
                                         VPlan *Plan,
                                         HIRVectorizationLegality *Legal,
                                         const DDGraph &DDG)
    : VPlanHCFGBuilder(nullptr, nullptr, nullptr,
                       Lp->getHLNodeUtils().getDataLayout(), WRL, Plan,
                       nullptr),
      TheLoop(Lp), DDG(DDG), HIRLegality(Legal) {
  Verifier = std::make_unique<VPlanVerifierHIR>(Lp);
  assert((!WRLp || WRLp->getTheLoop<HLLoop>() == TheLoop) &&
         "Inconsistent Loop information");
}

class ReductionDescriptorHIR {
  using DataType = loopopt::HLInst;
  using RecurrenceKind = VPReduction::RecurrenceKind;
  using MinMaxRecurrenceKind = VPReduction::MinMaxRecurrenceKind;
  friend class ReductionInputIteratorHIR;
  friend class MinMaxIdiomsInputIteratorHIR;

public:
  ReductionDescriptorHIR() { clear(); }

  const DataType *getHLInst() const { return HLInst; }
  const DataType *getParentInst() const { return ParentInst; }
  RecurrenceKind getKind() const { return RKind; }
  MinMaxRecurrenceKind getMinMaxKind() const { return MK; }
  Type *getRedType() const { return RedType; }
  bool isSigned() const { return IsSigned; }

private:
  void fillReductionKinds(Type *DestType, unsigned OpCode, PredicateTy Pred,
                          bool IsMax) {
    MK = MinMaxRecurrenceKind::MRK_Invalid;
    RedType = DestType;
    IsSigned = false;
    switch (OpCode) {
    case Instruction::FAdd:
    case Instruction::FSub:
      RKind = RecurrenceKind::RK_FloatAdd;
      break;
    case Instruction::Add:
    case Instruction::Sub:
      RKind = RecurrenceKind::RK_IntegerAdd;
      break;
    case Instruction::FMul:
      RKind = RecurrenceKind::RK_FloatMult;
      break;
    case Instruction::Mul:
      RKind = RecurrenceKind::RK_IntegerMult;
      break;
    case Instruction::And:
      RKind = RecurrenceKind::RK_IntegerAnd;
      break;
    case Instruction::Or:
      RKind = RecurrenceKind::RK_IntegerOr;
      break;
    case Instruction::Xor:
      RKind = RecurrenceKind::RK_IntegerXor;
      break;
    case Instruction::Select: {
      if (RedType->isIntegerTy()) {
        RKind = RecurrenceKind::RK_IntegerMinMax;
      } else {
        assert(RedType->isFloatingPointTy() &&
               "Floating point type expected at this point!");
        RKind = RecurrenceKind::RK_FloatMinMax;
      }
      setMinMaxReductionKind(Pred, IsMax);
      break;
    }
    default:
      llvm_unreachable("Unexpected reduction opcode");
      break;
    }
  }

  void setMinMaxReductionKind(PredicateTy Pred, bool IsMax) {
    switch (Pred) {
    case PredicateTy::ICMP_SGE:
    case PredicateTy::ICMP_SGT:
    case PredicateTy::ICMP_SLE:
    case PredicateTy::ICMP_SLT:
      MK = IsMax ? MinMaxRecurrenceKind::MRK_SIntMax
                 : MinMaxRecurrenceKind::MRK_SIntMin;
      IsSigned = true;
      break;
    case PredicateTy::ICMP_UGE:
    case PredicateTy::ICMP_UGT:
    case PredicateTy::ICMP_ULE:
    case PredicateTy::ICMP_ULT:
      MK = IsMax ? MinMaxRecurrenceKind::MRK_UIntMax
                 : MinMaxRecurrenceKind::MRK_UIntMin;
      break;
    default:
      MK = IsMax ? MinMaxRecurrenceKind::MRK_FloatMax
                 : MinMaxRecurrenceKind::MRK_FloatMin;
      break;
    }
  }

  void clear() {
    HLInst = nullptr;
    ParentInst = nullptr;
    RKind = RecurrenceKind::RK_NoRecurrence;
    MK = MinMaxRecurrenceKind::MRK_Invalid;
    RedType = nullptr;
    IsSigned = false;
  }

  const DataType *HLInst;
  const DataType *ParentInst; // Link to parent reduction.
  RecurrenceKind RKind;
  MinMaxRecurrenceKind MK;
  Type *RedType;
  bool IsSigned;
};

/// Class implements input iterator for reductions. The input is done
/// from HIRSafeReductionAnalysis object.
/// The HIRSafeReductionAnalysis contains list of HIRSafeRedInfo which, in turn,
/// contains another list, the list of statements. The first list items contain
/// the common information about redcution type, operation, etc. The second list
/// contains info about concrete statements. We need to iteratate through the
/// all statements so this iterator goes through both lists, first taking the
/// HIRSafeRedInfo and then going through its list of statements. Currently
/// masked safe reductions are not converted since more analysis is needed in
/// VPlan HCFG  to correctly capture loop exit instruction. (JIRA:
/// CMPLRLLVM-9609)
class ReductionInputIteratorHIR {
  using RecurrenceKind = VPReduction::RecurrenceKind;
  using MinMaxRecurrenceKind = VPReduction::MinMaxRecurrenceKind;
public:
  using iterator_category = std::input_iterator_tag;
  using value_type = ReductionDescriptorHIR;
  using const_pointer = const ReductionDescriptorHIR *;
  using const_reference = const ReductionDescriptorHIR &;

  /// Constructor. The \p Begin defines for which point the iterator is created,
  /// either for the beginning of the sequence or for the end.
  ReductionInputIteratorHIR(bool Begin, const SafeRedInfoList &SRCL) {
    ChainCurrent = Begin ? SRCL.begin() : SRCL.end();
    ChainEnd = SRCL.end();
    resetRedIterators();
    fillData();
  }

  inline bool operator==(const ReductionInputIteratorHIR &R) const {
    return ChainCurrent == R.ChainCurrent && ChainEnd == R.ChainEnd &&
           RedCurrent == R.RedCurrent && RedEnd == R.RedEnd;
  }
  inline bool operator!=(const ReductionInputIteratorHIR &R) const {
    return !operator==(R);
  }
  inline const_reference operator*() { return Descriptor; }
  inline const_pointer operator->()  { return &operator*(); }

  ReductionInputIteratorHIR &operator++() {
    advance();
    return *this;
  }

private:
  /// Move the iterator forward.
  void advance() {
    if (RedCurrent != RedEnd)
      RedCurrent++;
    if (RedCurrent == RedEnd) {
      if (ChainCurrent != ChainEnd) {
        ChainCurrent++;
        resetRedIterators();
      }
      else
        llvm_unreachable("Can't advance iterator");
    }
    fillData();
  }

  void resetRedIterators() {
    RedCurrent = RedEnd = nullptr; // invalidate the statements iterator
    while (ChainCurrent != ChainEnd) {
      RedCurrent = ChainCurrent->Chain.begin();
      RedEnd = ChainCurrent->Chain.end();
      if (RedCurrent != RedEnd) {
        // TODO: the only last statement in reduction chain is decomposed
        // as reduction, i.e. has a PHI instruction. Probably, it's ok but
        // need to investigate whether we need other statements as reductions.
        RedCurrent = RedEnd;
        RedCurrent--;
        if (!isMaskedReduction()) {
          auto Opcode = ChainCurrent->OpCode;
          // Predicate type is needed to determine reduction kind for min/max
          // reductions. For other reductions predicate is undefined.
          auto Pred = Opcode == Instruction::Select
                          ? (*RedCurrent)->getPredicate().Kind
                          : PredicateTy::BAD_ICMP_PREDICATE;
          Descriptor.fillReductionKinds(
              (*RedCurrent)->getLvalDDRef()->getDestType(), Opcode, Pred,
              (*RedCurrent)->isMax());
          break;
        } else {
          // invalidate iterators for masked reductions
          RedCurrent = RedEnd = nullptr;
        }
      }
      ChainCurrent++;
    }
  }

  void fillData() {
    if (RedCurrent != RedEnd)
      Descriptor.HLInst = *RedCurrent;
    else
      Descriptor.clear();
  }

  bool isMaskedReduction() {
    if (RedCurrent == RedEnd)
      return false;

    if (isa<HLIf>((*RedCurrent)->getParent()))
      return true;

    return false;
  }

private:
  ReductionDescriptorHIR Descriptor;
  SafeRedInfoList::const_iterator ChainCurrent;
  SafeRedInfoList::const_iterator ChainEnd;
  SafeRedChain::const_iterator RedCurrent;
  SafeRedChain::const_iterator RedEnd;
};

/// Class implements input iterator for minmax+index vector idioms. The input is
/// done from HIRVectorIdioms object. The HIRVectorIdioms contains list of
/// instructions that are recognized as vector minmax idioms linked with other,
/// index, instructions. The iterator iterates over HIRVectorIdioms main list,
/// looking for minmax idioms and then it goes through the list of linked
/// indexes. Both kinds of instructions are imported as reductions (VPReduction
/// and VPReductionIndex).
class MinMaxIdiomsInputIteratorHIR {
  using RecurrenceKind = VPReduction::RecurrenceKind;
  using MinMaxRecurrenceKind = VPReduction::MinMaxRecurrenceKind;

public:
  using iterator_category = std::input_iterator_tag;
  using value_type = ReductionDescriptorHIR;
  using const_pointer = const ReductionDescriptorHIR *;
  using const_reference = const ReductionDescriptorHIR &;

  /// Constructor. The \p Begin defines for which point the iterator is created,
  /// either for the beginning of the sequence or for the end.
  MinMaxIdiomsInputIteratorHIR(bool Begin, const HIRVectorIdioms &IList)
      : IdiomList(IList) {
    MainCurrent = Begin ? IdiomList.begin() : IdiomList.end();
    MainEnd = IdiomList.end();
    resetRedIterators();
    fillData();
  }

  inline bool operator==(const MinMaxIdiomsInputIteratorHIR &R) const {
    return MainCurrent == R.MainCurrent && MainEnd == R.MainEnd &&
           LinkedCurrent == R.LinkedCurrent && LinkedEnd == R.LinkedEnd;
  }
  inline bool operator!=(const MinMaxIdiomsInputIteratorHIR &R) const {
    return !operator==(R);
  }
  inline const_reference operator*() { return Descriptor; }
  inline const_pointer operator->() { return &operator*(); }

  MinMaxIdiomsInputIteratorHIR &operator++() {
    advance();
    return *this;
  }

private:
  /// Move the iterator forward.
  void advance() {
    if (LinkedCurrent != LinkedEnd)
      LinkedCurrent++;
    if (LinkedCurrent == LinkedEnd) {
      if (MainCurrent != MainEnd) {
        advanceMainIter();
        resetRedIterators();
      } else
        llvm_unreachable("Can't advance iterator");
    }
    fillData();
  }

  void resetRedIterators() {
    LinkedCurrent = LinkedEnd = nullptr; // invalidate the statements iterator
    fillTempArray();
    if (MainCurrent != MainEnd) {
      LinkedCurrent = TempVector.begin();
      LinkedEnd = TempVector.end();
      assert(LinkedCurrent != LinkedEnd && "Unexpected empty list");
      MainInst = *LinkedCurrent;
    }
  }

  void fillData() {
    if (LinkedCurrent != LinkedEnd) {
      unsigned Opcode = 0;
      (*LinkedCurrent)->isReductionOp(&Opcode);
      // Only select is expected here.
      assert(Opcode == Instruction::Select && "expected reduction");
      Descriptor.fillReductionKinds(
          (*LinkedCurrent)->getLvalDDRef()->getDestType(), Opcode,
          (*LinkedCurrent)->getPredicate(), (*LinkedCurrent)->isMax());
      Descriptor.HLInst = *LinkedCurrent;
      if (Descriptor.HLInst != MainInst)
        Descriptor.ParentInst = MainInst;
    } else
      Descriptor.clear();
  }

  void advanceMainIter() {
    while (MainCurrent != MainEnd) {
      MainCurrent++;
      if (MainCurrent == MainEnd ||
          MainCurrent->second == HIRVectorIdioms::MinOrMax)
        break;
    }
  }

  void fillTempArray() {
    TempVector.clear();
    if (MainCurrent != MainEnd) {
      TempVector.push_back(MainCurrent->first);
      auto *LinkedList = IdiomList.getLinkedIdioms(MainCurrent->first);
      if (LinkedList)
        for (auto Linked : *LinkedList)
          TempVector.push_back(Linked);
    }
  }

private:
  using TempVectorTy = SmallVector<const ReductionDescriptorHIR::DataType *, 2>;

  ReductionDescriptorHIR Descriptor;
  // Reduction instruction representing the main min/max reduction.
  const ReductionDescriptorHIR::DataType *MainInst;
  const HIRVectorIdioms &IdiomList;

  // Iterators pointing to the current and last min/max+index idiom.
  HIRVectorIdioms::const_iterator MainCurrent;
  HIRVectorIdioms::const_iterator MainEnd;
  // Temporary list to collect the main min/max reduction and all of its
  // corresponding linked index reductions. The list is cleared and repopulated
  // for every main min/max reduction.
  TempVectorTy TempVector;
  // Iterators for the temporary list.
  TempVectorTy::const_iterator LinkedCurrent;
  TempVectorTy::const_iterator LinkedEnd;
};

// Base class for VPLoopEntity conversion functors.
class VPEntityConverterBase {
public:
  using InductionList = VPDecomposerHIR::VPInductionHIRList;
  using LinearList = HIRVectorizationLegality::LinearListTy;
  using ExplicitReductionList = HIRVectorizationLegality::ReductionListTy;
  using PrivatesListTy = HIRVectorizationLegality::PrivatesListTy;
  using RecurrenceKind = VPReduction::RecurrenceKind;
  using MinMaxRecurrenceKind = VPReduction::MinMaxRecurrenceKind;
  using InductionKind = VPInduction::InductionKind;


  VPEntityConverterBase(VPDecomposerHIR &Decomp) : Decomposer(Decomp) {}

protected:
  VPDecomposerHIR &Decomposer;
};

/// Convert the data from auto-recognized induction list.
class InductionListCvt : public VPEntityConverterBase {
public:
  InductionListCvt(VPDecomposerHIR &Decomp) : VPEntityConverterBase(Decomp) {}

  void operator()(InductionDescr &Descriptor,
                  const InductionList::value_type &CurValue) {
    VPDecomposerHIR::VPInductionHIR *ID = CurValue.get();
    Descriptor.setInductionBinOp(ID->getUpdateInstr());
    Descriptor.setBinOpcode(Instruction::BinaryOpsEnd);
    Type *IndTy = Descriptor.getInductionBinOp()->getType();
    if (IndTy->isIntegerTy())
      Descriptor.setKind(VPInduction::InductionKind::IK_IntInduction);
    else if (IndTy->isPointerTy())
      Descriptor.setKind(VPInduction::InductionKind::IK_PtrInduction);
    else if (IndTy->isFloatingPointTy())
      Descriptor.setKind(VPInduction::InductionKind::IK_FpInduction);
    else
      llvm_unreachable("Unsupported induction data type.");
    Descriptor.setStartPhi(nullptr);
    Descriptor.setStart(ID->getStart());
    Descriptor.setStep(ID->getStep());
    Descriptor.setAllocaInst(nullptr);
  }
};

/// Convert data from linears list.
class LinearListCvt : public VPEntityConverterBase {
public:
  LinearListCvt(VPDecomposerHIR &Decomp) : VPEntityConverterBase(Decomp) {}

  void operator()(InductionDescr &Descriptor,
                  const LinearList::value_type &CurrValue) {
    Type *IndTy = CurrValue.Ref->getDestType();
    const HLDDNode *HLNode = CurrValue.Ref->getHLDDNode();
    Descriptor.setStartPhi(
        dyn_cast<VPInstruction>(Decomposer.getVPValueForNode(HLNode)));
    if (IndTy->isIntegerTy())
      Descriptor.setKind(InductionDescriptor::IK_IntInduction);
    else if (IndTy->isPointerTy())
      Descriptor.setKind(InductionDescriptor::IK_PtrInduction);
    else {
      assert(IndTy->isFloatingPointTy() && "unexpected induction type");
      Descriptor.setKind(InductionDescriptor::IK_FpInduction);
    }
    Descriptor.setStart(Descriptor.getStartPhi());
    int64_t Stride = CurrValue.Step->getSingleCanonExpr()->getConstant();
    Constant *Cstep = nullptr;
    if (IndTy->isPointerTy()) {
      Type *PointerElementType = IndTy->getPointerElementType();
      // The pointer stride cannot be determined if the pointer element type is
      // not sized.
      assert(PointerElementType->isSized() &&
             "Can't determine size of pointed-to type");
      const DataLayout &DL = CurrValue.Ref->getDDRefUtils().getDataLayout();
      int64_t Size =
          static_cast<int64_t>(DL.getTypeAllocSize(PointerElementType));
      assert(Size && "Can't determine size of pointed-to type");
      Type *IntTy = DL.getIntPtrType(IndTy);
      Cstep = ConstantInt::get(IntTy, Stride * Size);
    } else
      Cstep = ConstantInt::get(IndTy, Stride);
    Descriptor.setStep(Decomposer.getVPValueForConst(Cstep));
    Descriptor.setInductionBinOp(nullptr);
    Descriptor.setBinOpcode(Instruction::Add);
    Descriptor.setAllocaInst(Descriptor.getStart());
  }
};

/// Convert data from auto-recognized reductions list.
template <class Iterator>
class ReductionListCvt : public VPEntityConverterBase {
  using value_type = typename Iterator::value_type;

public:
  ReductionListCvt(VPDecomposerHIR &Decomp) : VPEntityConverterBase(Decomp) {}

  void operator()(ReductionDescr &Descriptor, const value_type &CurValue) {
    auto Inst = CurValue.getHLInst();
    if (Inst)
      Descriptor.setExit(
          dyn_cast<VPInstruction>(Decomposer.getVPValueForNode(Inst)));
    else
      Descriptor.setExit(nullptr);
    Descriptor.setStartPhi(nullptr);
    Descriptor.setStart(nullptr);
    Descriptor.setKind(CurValue.getKind());
    Descriptor.setMinMaxKind(CurValue.getMinMaxKind());
    Descriptor.setRecType(CurValue.getRedType());
    Descriptor.setSigned(CurValue.isSigned());
    Descriptor.setAllocaInst(nullptr);
    Inst = CurValue.getParentInst();
    if (Inst)
      Descriptor.setLinkPhi(
          dyn_cast<VPInstruction>(Decomposer.getVPValueForNode(Inst)));
    else
      Descriptor.setLinkPhi(nullptr);
  }
};

class ExplicitReductionListCvt : public VPEntityConverterBase {
public:
  ExplicitReductionListCvt(VPDecomposerHIR &Decomp) : VPEntityConverterBase(Decomp) {}

  /// Fill in the data from list of explicit reductions
  void operator()(ReductionDescr &Descriptor,
                  const ExplicitReductionList::value_type &CurrValue) {
    Type *RType = CurrValue.Ref->getDestType();
    assert(isa<PointerType>(RType) &&
           "SIMD reduction descriptor DDRef is not pointer type.");
    // Get pointee type of descriptor ref
    RType = cast<PointerType>(RType)->getElementType();
    // Translate HIRLegality descriptor's UpdateInstructions to corresponding
    // VPInstructions
    for (auto *UpdateInst : CurrValue.UpdateInstructions) {
      VPValue *UpdateVPInst = Decomposer.getVPValueForNode(UpdateInst);
      assert(UpdateVPInst && isa<VPInstruction>(UpdateVPInst) &&
             "Instruction that updates reduction descriptor is not valid.");
      Descriptor.addUpdateVPInst(cast<VPInstruction>(UpdateVPInst));
    }
    // Set start value of descriptor (can be null)
    Descriptor.setStart(
        CurrValue.InitValue
            ? Decomposer.getVPExternalDefForDDRef(CurrValue.InitValue)
            : nullptr);

    if (HIRVectorizationLegality::DescrValues *Alias =
            CurrValue.getValidAlias()) {
      VPValue *AliasInit =
          Decomposer.getVPExternalDefForDDRef(Alias->InitValue);
      SmallVector<VPInstruction *, 4> AliasUpdates;
      for (auto *UpdateInst : Alias->UpdateInstructions)
        AliasUpdates.push_back(
            cast<VPInstruction>(Decomposer.getVPValueForNode(UpdateInst)));
      Descriptor.setAlias(AliasInit, AliasUpdates);
    }

    // NOTE: Exit is not set here, it is identified based on some analysis in
    // Phase 2
    Descriptor.setExit(nullptr);
    // AI will refer to the ExternalDef (start value) of the original descriptor
    // (can be null if its not used inside loop)
    Descriptor.setAllocaInst(Descriptor.getStart());

    Descriptor.setStartPhi(nullptr);
    Descriptor.setKind(CurrValue.Kind);
    Descriptor.setMinMaxKind(CurrValue.MMKind);
    // In the directive, we have the kinds always set as for integers. Need to
    // correct them for fp-data.
    if (RType->isFloatingPointTy()) {
      if (CurrValue.Kind == RecurrenceKind::RK_IntegerAdd)
        Descriptor.setKind( RecurrenceKind::RK_FloatAdd);
      else if (CurrValue.Kind == RecurrenceKind::RK_IntegerMult)
        Descriptor.setKind(RecurrenceKind::RK_FloatMult);
      else if (CurrValue.Kind == RecurrenceKind::RK_IntegerMinMax) {
        Descriptor.setKind(RecurrenceKind::RK_FloatMinMax);
        if (CurrValue.MMKind == MinMaxRecurrenceKind::MRK_UIntMin ||
            CurrValue.MMKind == MinMaxRecurrenceKind::MRK_SIntMin)
          Descriptor.setMinMaxKind(MinMaxRecurrenceKind::MRK_FloatMin);
        else
          Descriptor.setMinMaxKind(MinMaxRecurrenceKind::MRK_FloatMax);
      }
    }
    Descriptor.setRecType(RType);
    Descriptor.setSigned(CurrValue.IsSigned);
    Descriptor.setLinkPhi(nullptr);
  }
};

// Convert data from Privates list
class PrivatesListCvt : public VPEntityConverterBase {
public:
  PrivatesListCvt(VPDecomposerHIR &Decomp, bool IsCond = false,
                  bool IsLast = false)
      : VPEntityConverterBase(Decomp), IsCondPriv(IsCond), IsLastPriv(IsLast) {}

  void operator()(PrivateDescr &Descriptor,
                  const PrivatesListTy::value_type &CurValue) {
    Descriptor.setAllocaInst(nullptr);
    Descriptor.setIsConditional(IsCondPriv);
    Descriptor.setIsLast(IsLastPriv);
    Descriptor.setIsExplicit(true);
    Descriptor.setIsMemOnly(false);
  }

private:
  bool IsCondPriv;
  bool IsLastPriv;
};

class HLLoop2VPLoopMapper {
public:
  HLLoop2VPLoopMapper() = delete;
  explicit HLLoop2VPLoopMapper(
      const VPlan *Plan,
      SmallDenseMap<VPBasicBlock *, HLLoop *> Header2HLLoop) {

    std::function<void(const VPLoop *)> mapLoop2VPLoop =
        [&](const VPLoop *VPL) {
          const HLLoop *L = Header2HLLoop[cast<VPBasicBlock>(VPL->getHeader())];
          assert(L != nullptr && "Can't find Loop");
          LoopMap[L] = VPL;
          for (auto VLoop : *VPL)
            mapLoop2VPLoop(VLoop);
        };

    VPLoop *TopLoop = *(Plan->getVPLoopInfo()->begin());
    mapLoop2VPLoop(TopLoop);
  }

  const VPLoop *operator[](const HLLoop *L) const {
    auto Iter = LoopMap.find(L);
    return Iter == LoopMap.end() ? nullptr : Iter->second;
  }
protected:
  DenseMap<const HLLoop *, const VPLoop *> LoopMap;
};

typedef VPLoopEntitiesConverter<ReductionDescr, HLLoop,
                                HLLoop2VPLoopMapper> ReductionConverter;
typedef VPLoopEntitiesConverter<InductionDescr, HLLoop,
                                HLLoop2VPLoopMapper> InductionConverter;

void PlainCFGBuilderHIR::convertEntityDescriptors(
    HIRVectorizationLegality *Legal,
    VPlanHCFGBuilder::VPLoopEntityConverterList &CvtVec) {

  using InductionList = VPDecomposerHIR::VPInductionHIRList;
  using LinearList = HIRVectorizationLegality::LinearListTy;
  using ExplicitReductionList = HIRVectorizationLegality::ReductionListTy;

  ReductionConverter *RedCvt = new ReductionConverter(Plan);
  InductionConverter *IndCvt = new InductionConverter(Plan);

  for (auto LoopDescr = Header2HLLoop.begin(), End = Header2HLLoop.end();
       LoopDescr != End; ++LoopDescr) {
    HLLoop *HL = LoopDescr->second;
    Legal->getSRA()->computeSafeReductionChains(HL);
    const SafeRedInfoList &SRCL = Legal->getSRA()->getSafeRedInfoList(HL);

    LLVM_DEBUG(
        dbgs() << "Found the following auto-recognized reductions in the loop "
                  "with header ";
        dbgs() << LoopDescr->first->getName() << "\n";
        for (auto &SafeRedInfo : SRCL)
          for (auto &HlInst : SafeRedInfo.Chain) {
            const VPInstruction *Inst =
                cast<VPInstruction>(Decomposer.getVPValueForNode(HlInst));
            Inst->dump();
          }
    );

    const InductionList &IL = Decomposer.getInductions(HL);
    iterator_range<InductionList::const_iterator> InducRange(IL.begin(),
                                                             IL.end());
    InductionListCvt InducListCvt(Decomposer);
    auto InducPair = std::make_pair(InducRange, InducListCvt);

    const LinearList &LL = Legal->getLinears();
#if 1
    // TODO: remove after correction of descriptor translation (see
    iterator_range<LinearList::const_iterator> LinearRange(LL.end(), LL.end());
#else
    iterator_range<LinearList::const_iterator> LinearRange(LL->begin(),
                                                           LL->end());
#endif
    LinearListCvt LinListCvt(Decomposer);
    auto LinearPair = std::make_pair(LinearRange, LinListCvt);

    iterator_range<ReductionInputIteratorHIR> ReducRange(
        ReductionInputIteratorHIR(true, SRCL),
        ReductionInputIteratorHIR(false, SRCL));
    ReductionListCvt<ReductionInputIteratorHIR> RedListCvt(Decomposer);
    auto ReducPair = std::make_pair(ReducRange, RedListCvt);

    const ExplicitReductionList &ERL = Legal->getReductions();

    iterator_range<ExplicitReductionList::const_iterator> ExplRedRange(
        ERL.begin(), ERL.end());

    ExplicitReductionListCvt ExplRedCvt(Decomposer);
    auto ExplRedPair = std::make_pair(ExplRedRange, ExplRedCvt);

    const HIRVectorIdioms *Idioms = Legal->getVectorIdioms(HL);
    iterator_range<MinMaxIdiomsInputIteratorHIR> MinMaxIdiomRange(
        MinMaxIdiomsInputIteratorHIR(true, *Idioms),
        MinMaxIdiomsInputIteratorHIR(false, *Idioms));
    ReductionListCvt<MinMaxIdiomsInputIteratorHIR> RedIdiomCvt(Decomposer);
    auto RedIdiomPair = std::make_pair(MinMaxIdiomRange, RedIdiomCvt);

    RedCvt->createDescrList(HL, ReducPair, ExplRedPair, RedIdiomPair);
    IndCvt->createDescrList(HL, InducPair, LinearPair);
  }
  CvtVec.push_back(std::unique_ptr<VPLoopEntitiesConverterBase>(RedCvt));
  CvtVec.push_back(std::unique_ptr<VPLoopEntitiesConverterBase>(IndCvt));
}

std::unique_ptr<VPRegionBlock>
VPlanHCFGBuilderHIR::buildPlainCFG(VPLoopEntityConverterList &CvtVec) {
  PlainCFGBuilderHIR PCFGBuilder(TheLoop, DDG, Plan, Header2HLLoop,
                                 HIRLegality);
  std::unique_ptr<VPRegionBlock> TopRegion = PCFGBuilder.buildPlainCFG();
  if (LoopEntityImportEnabled)
    PCFGBuilder.convertEntityDescriptors(HIRLegality, CvtVec);
  return TopRegion;
}

void VPlanHCFGBuilderHIR::passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) {
  typedef VPLoopEntitiesConverterTempl<HLLoop2VPLoopMapper> BaseConverter;

  // The HIRLegality lists should be populated by now
  LLVM_DEBUG(HIRLegality->dump());

  HLLoop2VPLoopMapper Mapper(Plan, Header2HLLoop);
  for (auto &Cvt : Cvts) {
    BaseConverter *Converter = dyn_cast<BaseConverter>(Cvt.get());
    Converter->passToVPlan(Plan, Mapper);
  }
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
