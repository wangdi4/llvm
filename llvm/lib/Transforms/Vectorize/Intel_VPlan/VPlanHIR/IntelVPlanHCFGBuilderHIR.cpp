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
/// in topological order and builds a plain CFG out of them.
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
///   - HLSwitch
///   - Loops with multiple exits.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanHCFGBuilderHIR.h"
#include "IntelVPlanBuilderHIR.h"
#include "IntelVPlanDecomposerHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Pass.h"

#define DEBUG_TYPE "VPlanHCFGBuilder"

using namespace llvm;
using namespace vpo;

static LoopVPlanDumpControl VPlanHIRDecomposerControl("hir-decomposer",
                                                      "VPlanHIRDecomposer");

/// Check if the incoming \p Ref matches the original SIMD descriptor DDRef \p
/// DescrRef
static bool isSIMDDescriptorDDRef(const RegDDRef *DescrRef, const DDRef *Ref) {
  assert(DescrRef->isAddressOf() &&
         "Original SIMD descriptor ref is not address of type.");

  auto *RegRef = dyn_cast<RegDDRef>(Ref);
  if (RegRef) {
    if (!RegRef->isMemRef())
      return false;

    // Since we know descriptor ref is always address of type, call dedicated
    // compare.
    if (DDRefUtils::areEqualWithoutAddressOf(DescrRef, RegRef))
      return true;
  } else {
    // Special casing for incoming Ref of the form %s which was actually the
    // Base CE of the memref %s[0]
    auto *DescrRefCE = DescrRef->getBaseCE();
    if (auto *BDDR = dyn_cast<BlobDDRef>(Ref)) {
      auto *RefCE = BDDR->getSingleCanonExpr();
      if (CanonExprUtils::areEqual(DescrRefCE, RefCE))
        return true;
    }
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
  OS << "\n\nHIRLegality PrivatesNonPODList:\n";
  for (auto &NPPvt : PrivatesNonPODList) {
    NPPvt.dump();
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
  OS << "\n\nHIRLegality UDRList:\n";
  for (auto &UDR : UDRList) {
    UDR.dump();
    OS << "\n";
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

template <typename DescrType>
DescrType *HIRVectorizationLegality::findDescr(ArrayRef<DescrType> List,
                                               const DDRef *Ref) const {
  for (auto &Descr : List) {
    // TODO: try to avoid returning the non-const ptr.
    DescrType *CurrentDescr = const_cast<DescrType *>(&Descr);
    assert(isa<RegDDRef>(CurrentDescr->getRef()) &&
           "The original SIMD descriptor Ref is not a RegDDRef.");
    if (isSIMDDescriptorDDRef(cast<RegDDRef>(CurrentDescr->getRef()), Ref))
      return CurrentDescr;

    // Check if Ref matches any aliases of current descriptor's ref
    if (CurrentDescr->findAlias(Ref))
      return CurrentDescr;
  }

  return nullptr;
}

// Explicit template instantiations for findDescr.
template HIRVectorizationLegality::RedDescrTy *
HIRVectorizationLegality::findDescr(
    ArrayRef<HIRVectorizationLegality::RedDescrTy> List,
    const DDRef *Ref) const;
template HIRVectorizationLegality::PrivDescrTy *
HIRVectorizationLegality::findDescr(
    ArrayRef<HIRVectorizationLegality::PrivDescrTy> List,
    const DDRef *Ref) const;
template HIRVectorizationLegality::PrivDescrNonPODTy *
HIRVectorizationLegality::findDescr(
    ArrayRef<HIRVectorizationLegality::PrivDescrNonPODTy> List,
    const DDRef *Ref) const;
template HIRVectorizationLegality::LinearDescr *
HIRVectorizationLegality::findDescr(
    ArrayRef<HIRVectorizationLegality::LinearDescr> List,
    const DDRef *Ref) const;

void HIRVectorizationLegality::recordPotentialSIMDDescrUse(DDRef *Ref) {

  DescrWithInitValueTy *Descr = getLinearRednDescriptors(Ref);

  // If Ref does not correspond to SIMD descriptor then nothing to do
  if (!Descr)
    return;

  assert(isa<RegDDRef>(Descr->getRef()) &&
         "The original SIMD descriptor Ref is not a RegDDRef.");
  if (isSIMDDescriptorDDRef(cast<RegDDRef>(Descr->getRef()), Ref)) {
    // Ref refers to the original descriptor
    // TODO: should we assert that InitVPValue is not set already?
    Descr->setInitValue(Ref);
  } else {
    // Ref is an alias to the original descriptor
    auto AliasIt = Descr->findAlias(Ref);
    assert(AliasIt && "Alias not found.");
    auto *Alias = cast<DescrWithInitValueTy>(AliasIt);
    Alias->setInitValue(Ref);
  }
}

void HIRVectorizationLegality::recordPotentialSIMDDescrUpdate(
    HLInst *UpdateInst) {
  RegDDRef *Ref = UpdateInst->getLvalDDRef();

  // Instruction does not write into any LVal, bail out of analysis
  if (!Ref)
    return;

  // Check, whether Ref is POD or non-POD Private
  DescrWithAliasesTy *Descr = getPrivateDescr(Ref);
  if (!Descr)
    Descr = getPrivateDescrNonPOD(Ref);
  // If Ref is not private check if it is linear reduction
  if (!Descr)
    Descr = getLinearRednDescriptors(Ref);

  // If Ref does not correspond to SIMD descriptor then nothing to do
  if (!Descr)
    return;

  assert(isa<RegDDRef>(Descr->getRef()) &&
         "The original SIMD descriptor Ref is not a RegDDRef.");
  if (isSIMDDescriptorDDRef(cast<RegDDRef>(Descr->getRef()), Ref)) {
    // Ref refers to the original descriptor
    Descr->addUpdateInstruction(UpdateInst);
  } else {
    // Ref is an alias to the original descriptor
    auto Alias = Descr->findAlias(Ref);
    assert(Alias && "Alias not found.");
    Alias->addUpdateInstruction(UpdateInst);
  }
}

bool HIRVectorizationLegality::bailout(BailoutReason Code) {
    DEBUG_WITH_TYPE("HIRLegality", dbgs() << getBailoutReasonStr(Code));
    return false;
}

bool HIRVectorizationLegality::canVectorize(const WRNVecLoopNode *WRLp) {
  // Send explicit data from WRLoop to the Legality.
  return EnterExplicitData(WRLp);
}

void HIRVectorizationLegality::findAliasDDRefs(HLNode *BeginNode,
                                               HLNode *EndNode, HLLoop *HLoop) {

  // Containers to collect all nodes that are present before/after HLoop to
  // process for potential aliases.
  SetVector<HLNode *> PreLoopNodes;
  SetVector<HLNode *> PostLoopNodes;

  // Collect nodes between the begin-SIMD clause directive and the HLLoop node.
  HLNode *CurNode = BeginNode;
  while (auto *NextNode = CurNode->getNextNode()) {
    if (NextNode == HLoop)
      break;
    PreLoopNodes.insert(NextNode);
    CurNode = NextNode;
  }
  // Collect nodes present in HLLoop's preheader.
  auto PreRange = map_range(HLoop->preheaderNodes(), [](HLNode &N) { return &N; });
  PreLoopNodes.insert(PreRange.begin(), PreRange.end());

  // Collect nodes present in HLLoop's postexit.
  // In some cases the EndNode can reside in the loop post exit. Thus we might
  // miss it going by the nodes after the loop. See the
  // hir_simd_directives_preheader_postexit.ll for an example of such EndNode
  // placement.
  bool EndFound = false;
  for (HLNode &Node: HLoop->postExitNodes()) {
    if (&Node == EndNode) {
      EndFound = true;
      break;
    }
    PostLoopNodes.insert(&Node);
  }
  // Collect nodes between the HLLoop node and the end-SIMD clause directive.
  if (!EndFound) {
    CurNode = HLoop->getNextNode();
    while (CurNode && CurNode != EndNode) {
      PostLoopNodes.insert(CurNode);
      CurNode = CurNode->getNextNode();
    }
    assert(CurNode && "can't find region end");
  }
  auto getDescr = [this](RegDDRef *Ref) {
    // Check if Ref is any of explicit SIMD descriptors.
    DescrWithAliasesTy *Descr = getPrivateDescr(Ref);
    if (!Descr)
      Descr = getPrivateDescrNonPOD(Ref);
    if (!Descr)
      Descr = getLinearRednDescriptors(Ref);
    return Descr;
  };
  auto addAlias = [](DescrWithAliasesTy *Descr, DDRef *Val) {
    if (isa<DescrWithInitValueTy>(Descr))
      Descr->addAlias(Val, std::make_unique<DescrWithInitValueTy>(Val));
    else
      Descr->addAlias(Val, std::make_unique<DescrWithAliasesTy>(Val));
  };
  // Process all pre-loop nodes.
  for (HLNode *PLN : PreLoopNodes) {
    LLVM_DEBUG(dbgs() << "PreHLLoop node: "; PLN->dump(););
    // Evaluate Rvals of only HLInsts in the pre-loop nodes.
    auto *HInst = dyn_cast<HLInst>(PLN);
    // TODO: Check whether we really can have HLoop, HLIf, or other-non-HInst
    // things here (between the simd-region begin statement and the loop). If
    // so we need a special processing for them. Same for the postloop nodes.
    if (!HInst)
      continue;
    RegDDRef *RVal = HInst->getRvalDDRef();
    if (!RVal)
      continue;

    DescrWithAliasesTy *Descr = getDescr(RVal);
    // RVal is not a SIMD descriptor, move to next HLInst.
    if (!Descr)
      continue;

    LLVM_DEBUG(dbgs() << "Found an alias for a SIMD descriptor ref ";
               RVal->dump(); dbgs() << "...."; HInst->dump(); dbgs() << "\n");
    RegDDRef *LVal = HInst->getLvalDDRef();
    assert(LVal && "HLInst in the preheader does not have an Lval.");
    addAlias(Descr, LVal);
  }
  // Process all post-loop nodes.
  for (HLNode *PLN : PostLoopNodes) {
    LLVM_DEBUG(dbgs() << "PostHLLoop node: "; PLN->dump(););
    // Evaluate LVals of only HLInsts in the pre-loop nodes.
    auto *HInst = dyn_cast<HLInst>(PLN);
    if (!HInst)
      continue;
    RegDDRef *LVal = HInst->getLvalDDRef();
    if (!LVal)
      continue;

    DescrWithAliasesTy *Descr = getDescr(LVal);
    // LVal is not a SIMD descriptor, move to next HLInst.
    if (!Descr)
      continue;

    LLVM_DEBUG(dbgs() << "Found an alias for a SIMD descriptor ref ";
               LVal->dump(); dbgs() << "...."; HInst->dump(); dbgs() << "\n");
    RegDDRef *RVal = HInst->getRvalDDRef();
    assert(RVal && "HLInst in the postexit does not have an Rval.");
    // TODO: extend to non-terminals. That might be useful for inductions.
    // Currently, we can bailout on non-recognized phi for such entities.
    if (RVal->isTerminalRef())
      addAlias(Descr, RVal);
  }
  LLVM_DEBUG(dbgs() << "HIR legality after collecting aliases\n"; dump(););
}

const HIRVectorIdioms *
HIRVectorizationLegality::getVectorIdioms(HLLoop *Loop) const {
  IdiomListTy &IdiomList = VecIdioms[Loop];
  if (!IdiomList) {
    IdiomList.reset(new HIRVectorIdioms());
    HIRVectorIdiomAnalysis Analysis;
    Analysis.gatherIdioms(TTI, *IdiomList, DDAnalysis->getGraph(Loop), *SRA,
                          Loop);
  }
  return IdiomList.get();
}

bool HIRVectorizationLegality::isMinMaxIdiomTemp(const DDRef *Ref,
                                                 HLLoop *Loop) const {
  auto *Idioms = getVectorIdioms(Loop);
  for (auto &IdiomDescr : make_range(Idioms->begin(), Idioms->end()))
    if ((IdiomDescr.second == HIRVectorIdioms::IdiomId::MinOrMax ||
         IdiomDescr.second == HIRVectorIdioms::IdiomId::MMFirstLastIdx ||
         IdiomDescr.second == HIRVectorIdioms::IdiomId::MMFirstLastVal) &&
        DDRefUtils::areEqual(
            static_cast<const HLInst *>(IdiomDescr.first)->getLvalDDRef(), Ref))
      return true;

  return false;
}

// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// VPInstructions.
class PlainCFGBuilderHIR : public HLNodeVisitorBase {
  friend HLNodeVisitor<PlainCFGBuilderHIR, false /*Recursive*/>;

private:
  /// Outermost loop of the input loop nest.
  HLLoop *TheLoop;

  VPlanVector *Plan;

  /// Map between loop header VPBasicBlock's and their respective HLLoop's. It
  /// is populated in this phase to keep the information necessary to create
  /// entity descriptors.
  SmallDenseMap<VPBasicBlock *, HLLoop *> &Header2HLLoop;

  HIRVectorizationLegality *Legal;

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

  /// Hold condition bits for the blocks with multiple exits.
  DenseMap<VPBasicBlock *, VPValue *> CondBits;

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

  // Collects all VConflict load and store instructions.
  bool collectVConflictLoadAndStoreInsns();

  /// Collects the instructions of VConflict pattern and replaces them with
  /// VPGeneralMemOptConflict instruction. Returns true if it emits VPConflict
  /// instruction.
  bool collectVConflictPatternInsnsAndEmitVPConflict();

  // Keep VConflict store and load instructions and conflict index.
  SmallVector<std::tuple<VPInstruction *, VPInstruction *, VPValue *>, 2>
      VConflictStoreLoadIndexInsns;

public:
  PlainCFGBuilderHIR(HLLoop *Lp, const DDGraph &DDG, VPlanVector *Plan,
                     SmallDenseMap<VPBasicBlock *, HLLoop *> &H2HLLp,
                     HIRVectorizationLegality *HIRLegality)
      : TheLoop(Lp), Plan(Plan), Header2HLLoop(H2HLLp), Legal(HIRLegality),
        Decomposer(Plan, Lp, DDG, *HIRLegality) {}

  /// Build a plain CFG for an HLLoop loop nest.
  bool buildPlainCFG();

  /// Convert incoming loop entities to the VPlan format.
  void
  convertEntityDescriptors(VPlanHCFGBuilder::VPLoopEntityConverterList &CvtVec);
};

/// Retrieve an existing VPBasicBlock for \p HNode. It there is no existing
/// VPBasicBlock, a new VPBasicBlock is created and mapped to \p HNode. If \p
/// HNode is null, the new VPBasicBlock is not mapped to any HLNode.
VPBasicBlock *PlainCFGBuilderHIR::getOrCreateVPBB(HLNode *HNode) {

  // Auxiliary function that creates an empty VPBasicBlock, set its parent to
  // Plan and increases Plan's size.
  auto createVPBB = [&]() -> VPBasicBlock * {
    VPBasicBlock *NewVPBB =
        new VPBasicBlock(VPlanUtils::createUniqueName("BB"), Plan);
    NewVPBB->setTerminator();
    Plan->insertAtBack(NewVPBB);
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
    VPBasicBlock *Succ = Pred->getSingleSuccessor();
    if (Succ)
      Pred->setTerminator(Succ, VPBB, CondBits[Pred]);
    else
      Pred->setTerminator(VPBB);
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

  // Make the zero trip test(Ztt) explicit for inner loops.
  bool EmitZtt = HLp != TheLoop && HLp->hasZtt();
  VPBasicBlock *ZttStartBlock = nullptr;
  if (EmitZtt) {
    // Force creation of a new VPBB for ztt check
    ActiveVPBB = nullptr;
    updateActiveVPBB();
    ZttStartBlock = ActiveVPBB;

    // Generate the compare instructions for the loop Ztt check and
    // store the final compare as the condition bit to be used to
    // bypass the loop.
    CondBits[ZttStartBlock] = Decomposer.createLoopZtt(HLp, ZttStartBlock);
  }

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
  } else {
    // There is no PH in HLLoop. Create dummy VPBB as PH. We could introduce
    // this dummy VPBB in simplifyPlainCFG, but according to the design for
    // LLVM-IR, we expect to have a loop with a PH as input. It's then better to
    // introduce the dummy PH here.
    updateActiveVPBB();
    ActiveVPBB->getTerminator()->setDebugLocation(HLp->getDebugLoc());
  }

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

  // Map loop header VPBasicBlock with HLLoop.
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
  Latch->setTerminator(Header);
  CondBits[Latch] = LatchCondBit;

  Latch->getTerminator()->setDebugLocation(HLp->getBranchDebugLoc());

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

  if (EmitZtt) {
    // Force creation of new block for Ztt check to jump to bypassing the loop.
    ActiveVPBB = nullptr;
    updateActiveVPBB();

    assert(ZttStartBlock && "Unexpected null Ztt start block");
    // ZttStartBlock jumps to either the preheader or the new block created
    // here using the condition bit that we set up earlier.
    ZttStartBlock->setTerminator(Preheader, ActiveVPBB,
                                 CondBits[ZttStartBlock]);
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

  unsigned VecLoopLevel = TheLoop->getNestingLevel();
  bool IsInvariantCondition =
      all_of(HIf->op_ddrefs(), [VecLoopLevel](const RegDDRef *Ref) {
        // Not invariant.
        if (!Ref->isStructurallyInvariantAtLevel(VecLoopLevel))
          return false;

        // Not hoistable.
        if (Ref->isMemRef())
          return false;

        for (const CanonExpr *CE : Ref->canons()) {
          for (auto BlobIt : CE->blobs()) {
            const BlobTy Blob = CE->getBlobUtils().getBlob(BlobIt.Index);
            // Not hoistable either.
            if (BlobUtils::mayContainUDivByZero(Blob))
              return false;
          }
        }

        return true;
      });
  VPValue *CondBit;
  if (IsInvariantCondition)
    CondBit = Plan->getVPExternalDefForIfCond(HIf);
  else
    CondBit = Decomposer.createVPInstructionsForNode(HIf, ActiveVPBB);
  CondBits[ConditionVPBB] = CondBit;

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
  if (HGoto->isExternal() || !HLNodeUtils::contains(TheLoop, Label)) {
    // Exiting goto in multi-exit loop. Use multi-exit landing pad as successor
    // of the goto VPBB. This should be done only when target label is not
    // inside the loop nest being decomposed.
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
  Decomposer.createVPBranchInstruction(ActiveVPBB, LabelVPBB, HGoto);

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
    auto *Header = VPL->getHeader();

    assert(Header2HLLoop.count(Header) &&
           "Missing mapping from loop header to HLLoop!");
    const HLLoop *HLoop = Header2HLLoop[Header];

    using TripCountTy = VPLoop::TripCountTy;
    TripCountTy TripCount;
    if (HLoop->isConstTripLoop(&TripCount)) {
      VPL->setKnownTripCount(TripCount);
      continue;
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

// Dumps bail-out messages when the legality checks of VConflict idiom fail.
static bool reportMatchFail(StringRef Reason) {
  LLVM_DEBUG(dbgs() << Reason << '\n');
  return false;
}

// Creates VPConflict idiom and its region. First, we form the region by
// collecting the uses of VConflictLoad until we reach VConflictStore. All these
// instructions will be removed from the original VPlan and they will be placed
// inside VConflict region. For the following example,
//
// for (int i=0; i<N; i++){
//   index = B[i];
//   A[index] = A[index] + C[0];
// }
//
// the code before VConflict generation is:
//
// i32* %vp.subscript.B = subscript inbounds i32* %B i64 %induction
// i32 %vp.load.B = load i32* %vp.subscript.B
// i64 %vp.conflict.index = sext i32 %vp.load.B to i64
// i32* %vp.subscript.A = subscript inbounds i32* %A i64 %vp.conflict.index
// i32 %vp.load.A = load i32* %vp.subscript.A
// i32 %vp.load.C = load i32* %C
// i32 %data = add i32 %vp.load.A i32 %vp.load.C
// i64 %vp.load.B.sext = sext i32 %vp.load.B to i64
// i32* %vp.subscript.A = subscript inbounds i32* %A i64 %vp.load.b.sext
// store i32 %data i32* %vp.subscript.A
//
// the code after VConflict generation is:
//
// i32* %vp.subscript.B = subscript inbounds i32* %B i64 %induction
// i32 %vp.load.B = load i32* %vp.subscript.B
// i64 %vp.conflict.index = sext i32 %vp.load.B to i64
// i32* %vp.subscript.A = subscript inbounds i32* %A i64 %vp.conflict.index
// i32 %vp.load.A = load i32* %vp.subscript.A
// i32 %vp.load.C = load i32* %C
// i64 %vp.load.B.sext = sext i32 %vp.load.B to i64
// i32* %vp.subscript.A = subscript inbounds i32* %A i64 %vp.load.b.sext
// i32 %vp.general.mem.opt.conflict = vp-general-mem-opt-conflict i64
// %vp.vconflict.index void %vp.conflict.region i32 %vp.load.A i32 %vp.load.C ->
// VConflictRegion (i32 %vp.live.in0 i32 %vp.live.in1 ) {
//   value : none
//   mask : none
//   live-in : i32 %vp.live.in0
//   live-in : i32 %vp.live.in1
//   Region:
//   VConflictBB
//   i32 %data = add i32 %vp.live.in0 i32 %vp.live.in1
//   live-out : i32 %data = add i32 %vp.live.in0 i32 %vp.live.in1
// }
// store i32 %vp.general.mem.opt.conflict i32* %vp.subscript.A
//
// The first three operands of VPGeneralMemOptConflict are always the
// conflicting index, conflict region and the load for array A. The rest are the
// live-ins. The load for array A is also a live-in. There is an implicit
// mapping of the operands [2:] of VPGeneralMemOptConflict and the live-ins of
// the region. The live-ins of the region are the renamed operands [2:] of
// VPGeneralMemOptConflict. For example, %vp.load.A and %vp.load.C operands of
// VPGeneralMemOptConflict correspond to %vp.live.in0 and %vp.line.in1
// respectively.
//
bool PlainCFGBuilderHIR::collectVConflictPatternInsnsAndEmitVPConflict() {
  for (auto &T : VConflictStoreLoadIndexInsns) {
    VPInstruction *VConflictStore;
    VPInstruction *VConflictLoad;
    VPValue *VConflictIndex;
    std::tie(VConflictStore, VConflictLoad, VConflictIndex) = T;
    assert(VConflictLoad && "VConflict load is expected.");
    assert(VConflictIndex && "Conflict index is expected.");
    VConflictIndex->setName("vconflict.index");

    // Collect the instructions of VConflict region by checking the uses of
    // VConflictLoad. In the simplest case, one value of the data of store
    // instruction is the load instruction and the other one is the value:
    //
    //   %Val = add i32 %VConflictLoad, 100
    //   i64 %sext = sext i32 %VPStoreLoadIndex to i64
    //   i32* %subscript = subscript inbounds i32* %A i64 %sext
    //   store i32 %Val i32* %subscript
    //
    // In other cases (as it is shown below), we have to find the chain of the
    // instructions that lead to the data of store:
    //
    //   i32 %add = add i32 %VConflictLoad i32 %Val1
    //   i32 %mul = mul i32 %add i32 %Val2
    //   i32 %Val = add i32 %mul i32 %Val3
    //   i64 %sext = sext i32 %VPStoreLoadIndex to i64
    //   i32* %subscript = subscript inbounds i32* %A i64 %sext
    //   store i32 %Val i32* %subscript
    //
    // Here, we collect all the uses of VConflictLoad and the uses of its uses
    // until we reach the definition of the data of VConflictStore.
    // TODO: Remove redundant instructions from VConflict pattern.

    auto *ParentVPBB = VConflictLoad->getParent();
    if (ParentVPBB != VConflictStore->getParent())
      return reportMatchFail(
          "VConflict load and store are in different basic blocks.");

    // First, we collect all the instructions between VConflictLoad and
    // VConflcitStore.
    assert(
        std::distance(VConflictLoad->getIterator(), ParentVPBB->end()) >
            std::distance(VConflictStore->getIterator(), ParentVPBB->end()) &&
        "VConflict idiom load is expected to appear before the store.");
    auto InstRange = map_range(
        make_range(VConflictLoad->getIterator(), VConflictStore->getIterator()),
        [](VPInstruction &I) { return &I; });
    SmallPtrSet<VPInstruction *, 2> InsnsBetweenLoadStore(InstRange.begin(),
                                                          InstRange.end());

    // Next, we start from VConflictLoad uses and we collect all the uses until
    // we reach VConflictStore.
    SmallVector<VPInstruction *, 2> RegionInsns;
    using df_iter = df_iterator<VPUser *>;
    for (auto It = std::next(df_iter::begin(VConflictLoad)),
              End = df_iter::end(VConflictLoad);
         It != End;) {
      if (*It == VConflictStore) {
        It.skipChildren();
        continue;
      }
      if (!isa<VPInstruction>(*It) ||
          !InsnsBetweenLoadStore.count(cast<VPInstruction>(*It)))
        return reportMatchFail(
            "VConflict load's use-chain escapes the region.");
      RegionInsns.push_back(cast<VPInstruction>(*It));
      It++;
    }

    // Check if any of the RegionInsns has uses outside of VConflict pattern.
    // Such uses need a special processing which is not implemented yet. Hence,
    // we need to bail-out.
    for (auto *I : RegionInsns) {
      if ((any_of(I->users(), [&](VPUser *U) {
            return !is_contained(RegionInsns, U) && (U != VConflictStore);
          })))
        return reportMatchFail("VConflict region should not have instructions "
                               "with uses outside of the region.");
      if (I->mayHaveSideEffects())
        return reportMatchFail(
            "VConflict region should not have instructions with side effects.");
    }

    // Collect the live-ins of VConflict region. We check if any operands of the
    // VConflct region's instructions have definition outside of the region.
    SetVector<VPValue *> RgnLiveIns;
    RgnLiveIns.insert(VConflictLoad);
    for (auto *VPInst : RegionInsns) {
      for (auto *Op : VPInst->operands()) {
        if (isa<VPConstant>(Op) || isa<VPExternalDef>(Op))
          continue;
        if (is_contained(RegionInsns, Op))
          continue;
        RgnLiveIns.insert(Op);
      }
    }

    if (RgnLiveIns.contains(VConflictStore->getOperand(0)))
      return reportMatchFail("VConflict store operand cannot be a live-in.");

    // Create new region and fill it with instructions, live-ins and live-outs.
    auto Region =
        std::make_unique<VPRegion>(Plan->getLLVMContext(), "conflict.region");

    // The optimized version of Histogram does not have control-flow. Therefore,
    // the region consists of one basic block.
    VPBasicBlock *VConflictBB = Region->addBB("VConflictBB");
    SmallVector<VPBasicBlock *, 2> VConflictBBs(1, VConflictBB);
    // Fill Region with the RegionInsns of VConflict pattern.
    for (auto It = RegionInsns.rbegin(), End = RegionInsns.rend(); It != End;
         It++) {
      VPInstruction *I = *It;
      VPBasicBlock *ParentBB = I->getParent();
      ParentBB->removeInstruction(I);
      VConflictBB->addInstructionAfter(I, nullptr);
    }
    // Replace the live-in operands of the region with new values. The
    // instructions that do not belong to VConflict region should not have
    // uses inside the region because this breaks the algorithms e.g.
    // divergence analysis, scalar vector analysis. The RenamedLiveIns are owned
    // by the region.
    int LInIt = 0;
    SmallVector<std::unique_ptr<VPValue>, 2> RenamedLiveIns;
    for (auto *LIn : RgnLiveIns) {
      auto NewValue = std::make_unique<VPValue>(LIn->getType());
      NewValue->setName("live.in" + std::to_string(LInIt));
      LIn->replaceAllUsesWithInRegion(NewValue.get(), VConflictBBs);
      RenamedLiveIns.push_back(std::move(NewValue));
      LInIt++;
    }
    // Generate new live-out. Its operand is the original live-out.
    SmallVector<std::unique_ptr<VPRegionLiveOut>, 2> RgnLiveOuts;
    VPValue *LiveOutUse = VConflictStore->getOperand(0);
    auto NewLiveOut = std::make_unique<VPRegionLiveOut>(LiveOutUse);
    RgnLiveOuts.push_back(std::move(NewLiveOut));
    Region->addRgnLiveInsOuts(std::move(RenamedLiveIns),
                              std::move(RgnLiveOuts));

    // Create VPGeneralMemOptConflict instruction.
    VPBuilder VPBldr;
    VPBldr.setInsertPoint(VConflictStore);
    auto *Conflict = VPBldr.create<VPGeneralMemOptConflict>(
        "vp.general.mem.opt.conflict", VConflictStore->getOperand(0)->getType(),
        VConflictIndex, std::move(Region), RgnLiveIns.getArrayRef());
    // Update VPConflictStore's first operand with VPGeneralMemOptConflict.
    VConflictStore->setOperand(0, Conflict);
  }
  return true;
}

bool PlainCFGBuilderHIR::collectVConflictLoadAndStoreInsns() {
  const HIRVectorIdioms *Idioms = Legal->getVectorIdioms(TheLoop);
  for (auto &Idiom : *Idioms)
    if (Idiom.second == HIRVectorIdioms::IdiomId::VConflictLikeStore) {
      const HLInst *HIRStore = Idiom.first;
      VPInstruction *VConflictStore =
          cast<VPInstruction>(Decomposer.getVPValueForNode(HIRStore));
      VPInstruction *VConflictLoad =
          Decomposer.getVPLoadConflict(Idioms->getVConflictLoad(HIRStore));
      VPValue *VConflictIndex =
          Decomposer.getVPConflictIndex(Idioms->getVConflictLoad(HIRStore));
      VConflictStoreLoadIndexInsns.push_back(
          std::make_tuple(VConflictStore, VConflictLoad, VConflictIndex));
    }
  return !VConflictStoreLoadIndexInsns.empty();
}

bool PlainCFGBuilderHIR::buildPlainCFG() {
  // Create a dummy VPBB as Plan's Entry.
  assert(!ActiveVPBB && "ActiveVPBB must be null.");
  updateActiveVPBB();

  // Trigger the visit of the loop nest.
  visit(TheLoop);

  // Create a dummy VPBB as Plan's Exit.
  ActiveVPBB = nullptr;
  updateActiveVPBB();

  // Create empty PHIs for live-out temps in Plan's exit block.
  Decomposer.createExitPhisForExternalUses(ActiveVPBB);

  // At this point, all the VPBasicBlocks have been built and all the
  // VPInstructions have been created for the loop nest. It's time to fix
  // VPInstructions representing a PHI operation.
  Decomposer.fixPhiNodes();

  // Initial plain CFG is ready at this point. Do post-processing to fix
  // VPExternalUses that have multiple operands i.e. multiple live-out
  // VPInstructions for single temp/symbase.
  Decomposer.fixExternalUses();

  // If the loop has load and store instructions which are marked as
  // VConflictLoad and VConflictStore respectively, then we emit VConflict
  // idiom.
  if (collectVConflictLoadAndStoreInsns())
    if (!collectVConflictPatternInsnsAndEmitVPConflict()) {
      LLVM_DEBUG(dbgs() << "The current VConflict idiom is not supported.\n");
      return false;
    }

  VPLAN_DUMP(VPlanHIRDecomposerControl, Plan);

  return true;
}

VPlanHCFGBuilderHIR::VPlanHCFGBuilderHIR(const WRNVecLoopNode *WRL, HLLoop *Lp,
                                         VPlanVector *Plan,
                                         HIRVectorizationLegality *Legal,
                                         const DDGraph &DDG)
    : VPlanHCFGBuilder(nullptr, nullptr, Lp->getHLNodeUtils().getDataLayout(),
                       WRL, Plan, nullptr),
      TheLoop(Lp), DDG(DDG), HIRLegality(Legal) {
  Verifier = std::make_unique<VPlanVerifierHIR>(Lp);
  assert((!WRLp || WRLp->getTheLoop<HLLoop>() == TheLoop) &&
         "Inconsistent Loop information");
}

class ReductionDescriptorHIR {
  using DataType = loopopt::HLInst;
  friend class ReductionInputIteratorHIR;
  friend class MinMaxIdiomsInputIteratorHIR;

public:
  ReductionDescriptorHIR() { clear(); }

  const DataType *getHLInst() const { return HLInst; }
  SafeRedChain getRedChain() const { return RedChain; }
  const DataType *getParentInst() const { return ParentInst; }
  RecurKind getKind() const { return RKind; }
  Type *getRedType() const { return RedType; }
  bool isSigned() const { return IsSigned; }
  HIRVectorIdioms::IdiomId getIdiomKind() const {return IdiomKind;}

private:
  void fillReductionKinds(Type *DestType, unsigned OpCode, PredicateTy Pred,
                          bool IsMax, HIRVectorIdioms::IdiomId IdKind) {
    RedType = DestType;
    IsSigned = false;
    IdiomKind = IdKind;
    switch (OpCode) {
    case Instruction::FAdd:
    case Instruction::FSub:
      RKind = RecurKind::FAdd;
      break;
    case Instruction::Add:
    case Instruction::Sub:
      RKind = RecurKind::Add;
      break;
    case Instruction::FMul:
      RKind = RecurKind::FMul;
      break;
    case Instruction::Mul:
      RKind = RecurKind::Mul;
      break;
    case Instruction::And:
      RKind = RecurKind::And;
      break;
    case Instruction::Or:
      RKind = RecurKind::Or;
      break;
    case Instruction::Xor:
      RKind = RecurKind::Xor;
      break;
    case Instruction::Select:
      setMinMaxReductionKind(Pred, IsMax);
      break;
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
      RKind = IsMax ? RecurKind::SMax : RecurKind::SMin;
      IsSigned = true;
      break;
    case PredicateTy::ICMP_UGE:
    case PredicateTy::ICMP_UGT:
    case PredicateTy::ICMP_ULE:
    case PredicateTy::ICMP_ULT:
      RKind = IsMax ? RecurKind::UMax : RecurKind::UMin;
      break;
    default:
      assert(CmpInst::isFPPredicate(Pred) && "expected FP predicate");
      RKind = IsMax ? RecurKind::FMax : RecurKind::FMin;
      break;
    }
  }

  void clear() {
    HLInst = nullptr;
    ParentInst = nullptr;
    RKind = RecurKind::None;
    RedType = nullptr;
    IsSigned = false;
    IdiomKind = HIRVectorIdioms::NoIdiom;
  }

  const DataType *HLInst;
  SafeRedChain RedChain;
  const DataType *ParentInst; // Link to parent reduction.
  RecurKind RKind;
  Type *RedType;
  bool IsSigned;
  HIRVectorIdioms::IdiomId IdiomKind = HIRVectorIdioms::NoIdiom;
};

/// Class implements input iterator for reductions. The input is done
/// from HIRSafeReductionAnalysis object.
/// The HIRSafeReductionAnalysis contains list of HIRSafeRedInfo which, in turn,
/// contains another list, the list of statements. The first list items contain
/// the common information about redcution type, operation, etc. The second list
/// contains info about concrete statements. We need to iteratate through the
/// all statements so this iterator goes through both lists, first taking the
/// HIRSafeRedInfo and then going through its list of statements.
class ReductionInputIteratorHIR {
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
        auto Opcode = ChainCurrent->OpCode;
        // Predicate type is needed to determine reduction kind for min/max
        // reductions. For other reductions predicate is undefined.
        auto Pred = PredicateTy::BAD_ICMP_PREDICATE;
        if (Opcode == Instruction::Select) {
          Pred = isa<SelectInst>((*RedCurrent)->getLLVMInstruction())
                     ? (*RedCurrent)->getPredicate().Kind
                     : PredicateTy::FIRST_FCMP_PREDICATE;
        }

        Descriptor.fillReductionKinds(
            (*RedCurrent)->getLvalDDRef()->getDestType(), Opcode, Pred,
            (*RedCurrent)->isMax(), HIRVectorIdioms::NoIdiom);
        Descriptor.RedChain = ChainCurrent->Chain;
        break;
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
    // Skip idioms in IdiomList until Min/Max idiom is found.
    while (MainCurrent != MainEnd &&
           MainCurrent->second != HIRVectorIdioms::MinOrMax)
      ++MainCurrent;
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
      MainInst = LinkedCurrent->first;
    }
  }

  void fillData() {
    if (LinkedCurrent != LinkedEnd) {
      const loopopt::HLInst *InstPtr = LinkedCurrent->first;
      // Only select is expected here.
      assert(isa<SelectInst>(InstPtr->getLLVMInstruction()) &&
             "expected select instruction");
      Descriptor.fillReductionKinds(InstPtr->getLvalDDRef()->getDestType(),
                                    Instruction::Select, InstPtr->getPredicate(),
                                    InstPtr->isMax(), LinkedCurrent->second);
      Descriptor.HLInst = InstPtr;
      if (Descriptor.HLInst != MainInst)
        Descriptor.ParentInst = MainInst;
      else
        Descriptor.ParentInst = nullptr;
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
      TempVector.push_back({MainCurrent->first, HIRVectorIdioms::MinOrMax});
      auto *LinkedList = IdiomList.getLinkedIdioms(MainCurrent->first);
      if (LinkedList)
        for (auto Linked : *LinkedList) {
          HIRVectorIdioms::IdiomId Id = IdiomList.isIdiom(Linked);
          TempVector.push_back({Linked, Id});
        }
    }
  }

private:
  using IdiomItem = std::pair<const ReductionDescriptorHIR::DataType *,
                              HIRVectorIdioms::IdiomId>;
  using TempVectorTy = SmallVector<IdiomItem, 2>;

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
  using UDRList = HIRVectorizationLegality::UDRListTy;
  using PrivatesListTy = HIRVectorizationLegality::PrivatesListTy;
  using PrivatesNonPODListTy = HIRVectorizationLegality::PrivatesNonPODListTy;
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
    Descriptor.setInductionOp(ID->getUpdateInstr());
    Descriptor.setIndOpcode(Instruction::BinaryOpsEnd);
    Type *IndTy = Descriptor.getInductionOp()->getType();
    VPInduction::InductionKind Kind;
    std::tie(std::ignore, Kind) = Descriptor.getKindAndOpcodeFromTy(IndTy);
    Descriptor.setKind(Kind);
    Descriptor.setStartPhi(nullptr);
    Descriptor.setStart(ID->getStart());
    Descriptor.setStep(ID->getStep());
    Descriptor.setStartVal(ID->getStartVal());
    Descriptor.setEndVal(ID->getEndVal());
    Descriptor.setAllocaInst(nullptr);
  }
};

/// Convert data from linears list.
class LinearListCvt : public VPEntityConverterBase {
public:
  LinearListCvt(VPDecomposerHIR &Decomp) : VPEntityConverterBase(Decomp) {}

  void operator()(InductionDescr &Descriptor,
                  const LinearList::value_type &CurrValue) {
    // TODO: for opaque pointers we may need to pull type information down
    // through the legality checker.
    Type *IndTy = CurrValue.getRef()->getDestType();
    const HLDDNode *HLNode = CurrValue.getRef()->getHLDDNode();
    Descriptor.setStartPhi(
        dyn_cast<VPInstruction>(Decomposer.getVPValueForNode(HLNode)));
    Descriptor.setKindAndOpcodeFromTy(IndTy);
    Descriptor.setStart(Descriptor.getStartPhi());
    int64_t Stride = CurrValue.Step->getSingleCanonExpr()->getConstant();
    Constant *Cstep = nullptr;
    if (IndTy->isPointerTy()) {
      Type *PointerElementType = IndTy->getPointerElementType();
      // The pointer stride cannot be determined if the pointer element type is
      // not sized.
      assert(PointerElementType->isSized() &&
             "Can't determine size of pointed-to type");
      const DataLayout &DL =
          CurrValue.getRef()->getDDRefUtils().getDataLayout();
      int64_t Size =
          static_cast<int64_t>(DL.getTypeAllocSize(PointerElementType));
      assert(Size && "Can't determine size of pointed-to type");
      Type *IntTy = DL.getIntPtrType(IndTy);
      Cstep = ConstantInt::get(IntTy, Stride * Size);
    } else
      Cstep = ConstantInt::get(IndTy, Stride);
    Descriptor.setStep(Decomposer.getVPValueForConst(Cstep));
    Descriptor.setInductionOp(nullptr);
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
    if (Inst) {
      Descriptor.setExit(
          dyn_cast<VPInstruction>(Decomposer.getVPValueForNode(Inst)));
      for (auto *ChainInst : CurValue.getRedChain()) {
        if (ChainInst == Inst)
          continue;
        LLVM_DEBUG(dbgs() << "ChainInst: "; ChainInst->dump());
        Descriptor.addLinkedVPValue(Decomposer.getVPValueForNode(ChainInst));
      }
    } else {
      Descriptor.setExit(nullptr);
    }
    Descriptor.setStartPhi(nullptr);
    Descriptor.setStart(nullptr);
    Descriptor.setKind(CurValue.getKind());
    Descriptor.setRecType(CurValue.getRedType());
    Descriptor.setSigned(CurValue.isSigned());
    Descriptor.setAllocaInst(nullptr);
    Inst = CurValue.getParentInst();
    if (Inst)
      Descriptor.setLinkPhi(
          dyn_cast<VPInstruction>(Decomposer.getVPValueForNode(Inst)));
    else
      Descriptor.setLinkPhi(nullptr);
    Descriptor.setIsLinearIndex(CurValue.getIdiomKind() ==
                                HIRVectorIdioms::MMFirstLastIdx);
  }
};

class ExplicitReductionListCvt : public VPEntityConverterBase {
public:
  ExplicitReductionListCvt(VPDecomposerHIR &Decomp) : VPEntityConverterBase(Decomp) {}

  /// Fill in the data from list of explicit reductions
  void operator()(ReductionDescr &Descriptor,
                  const ExplicitReductionList::value_type &CurrValue) {
    // Get pointee type of descriptor ref
    Type *RType = cast<RegDDRef>(CurrValue.getRef())->getBasePtrElementType();
    // Translate HIRLegality descriptor's UpdateInstructions to corresponding
    // VPInstructions
    for (auto *UpdateInst : CurrValue.getUpdateInstructions()) {
      auto *VPInst =
          dyn_cast<VPInstruction>(Decomposer.getVPValueForNode(UpdateInst));
      assert(VPInst && "Instruction updating reduction descriptor is invalid.");
      Descriptor.addUpdateVPInst(VPInst);
    }
    // Set start value of descriptor (can be null)
    Descriptor.setStart(
        CurrValue.getInitValue()
            ? Decomposer.getVPExternalDefForDDRef(CurrValue.getInitValue())
            : nullptr);

    if (HIRVectorizationLegality::DescrValueTy *Alias =
            CurrValue.getValidAlias()) {
      auto *RedAlias =
          cast<HIRVectorizationLegality::DescrWithInitValueTy>(Alias);
      VPValue *AliasInit =
          Decomposer.getVPExternalDefForDDRef(RedAlias->getInitValue());
      SmallVector<VPInstruction *, 4> AliasUpdates;
      for (auto *UpdateInst : Alias->getUpdateInstructions())
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
    Descriptor.setKind(CurrValue.getKind());
    // In the directive, we have the kinds always set as for integers. Need to
    // correct them for fp-data.
    if (RType->isFloatingPointTy()) {
      if (CurrValue.getKind() == RecurKind::Add)
        Descriptor.setKind(RecurKind::FAdd);
      else if (CurrValue.getKind() == RecurKind::Mul)
        Descriptor.setKind(RecurKind::FMul);
      else if (CurrValue.getKind() == RecurKind::UMin ||
               CurrValue.getKind() == RecurKind::SMin)
        Descriptor.setKind(RecurKind::FMin);
      else if (CurrValue.getKind() == RecurKind::UMax ||
               CurrValue.getKind() == RecurKind::SMax)
        Descriptor.setKind(RecurKind::FMax);
    }
    Descriptor.setRecType(RType);
    Descriptor.setSigned(CurrValue.isSigned());
    Descriptor.setLinkPhi(nullptr);
    Descriptor.setIsLinearIndex(false);
  }

  /// Fill in the data from list of user defined reductions
  void operator()(ReductionDescr &Descriptor,
                  const UDRList::value_type &CurrValue) {
    Descriptor.clear();
    // Use explicit reductions data filling operator above to populate
    // preliminary fields in descriptor.
    auto *BaseTyVal =
        static_cast<const ExplicitReductionList::value_type *>(&CurrValue);
    ExplicitReductionListCvt{Decomposer}(Descriptor, *BaseTyVal);
    // Capture functions needed for initialization/finalization.
    Descriptor.setCombiner(CurrValue.getCombiner());
    Descriptor.setInitializer(CurrValue.getInitializer());
    Descriptor.setCtor(CurrValue.getCtor());
    Descriptor.setDtor(CurrValue.getDtor());
  }
};

// Convert data from Privates list
class PrivatesListCvt : public VPEntityConverterBase {
public:
  PrivatesListCvt(VPDecomposerHIR &Decomp) : VPEntityConverterBase(Decomp) {}

  void operator()(PrivateDescr &Descriptor,
                  const PrivatesListTy::value_type &CurValue) {
    auto *DescrRef = cast<RegDDRef>(CurValue.getRef());
    DDRef *BasePtrRef = DescrRef->getBlobDDRef(DescrRef->getBasePtrBlobIndex());
    Descriptor.setAllocaInst(
        Decomposer.getVPExternalDefForSIMDDescr(BasePtrRef));
    Descriptor.setAllocatedType(CurValue.getType());
    Descriptor.setIsConditional(CurValue.isCond());
    Descriptor.setIsLast(CurValue.isLast());
    Descriptor.setIsExplicit(true);
    Descriptor.setIsMemOnly(false);
    Descriptor.setIsF90(CurValue.isF90());
    if (HIRVectorizationLegality::DescrValueTy *Alias =
            CurValue.getValidAlias()) {
      SmallVector<VPInstruction *, 4> AliasUpdates;
      for (auto *UpdateInst : Alias->getUpdateInstructions())
        AliasUpdates.push_back(
            cast<VPInstruction>(Decomposer.getVPValueForNode(UpdateInst)));
      Descriptor.setAlias(nullptr /*AliasInit*/, AliasUpdates);
    }
    for (auto UpdateInst: CurValue.getUpdateInstructions())
      Descriptor.addUpdateVPInst(
          cast<VPInstruction>(Decomposer.getVPValueForNode(UpdateInst)));
  }

  void operator()(PrivateDescr &Descriptor,
                  const PrivatesNonPODListTy::value_type &CurValue) {
    auto *DescrRef = cast<RegDDRef>(CurValue.getRef());
    DDRef *BasePtrRef = DescrRef->getBlobDDRef(DescrRef->getBasePtrBlobIndex());
    Descriptor.setAllocaInst(
        Decomposer.getVPExternalDefForSIMDDescr(BasePtrRef));
    Descriptor.setAllocatedType(CurValue.getType());
    Descriptor.setIsConditional(CurValue.isCond());
    Descriptor.setIsLast(CurValue.isLast());
    Descriptor.setCtor(CurValue.getCtor());
    Descriptor.setDtor(CurValue.getDtor());
    Descriptor.setCopyAssign(CurValue.getCopyAssign());
    Descriptor.setIsExplicit(true);
    Descriptor.setIsMemOnly(false);
    Descriptor.setIsF90(CurValue.isF90());
  }
};

class CompressExpandIdiomListCvt : public VPEntityConverterBase {
  const HIRVectorIdioms *VecIdioms;

public:
  CompressExpandIdiomListCvt(VPDecomposerHIR &Decomp,
                             const HIRVectorIdioms *VecIdioms)
      : VPEntityConverterBase(Decomp), VecIdioms(VecIdioms) {}

  void operator()(CompressExpandIdiomDescr &Desc, const HIRVecIdiom &Idiom) {

    std::function<void(const HIRVecIdiom &Idiom)> AddIdiom =
        [&](const HIRVecIdiom &Idiom) {
          switch (VecIdioms->isIdiom(Idiom)) {
          case HIRVectorIdioms::CEIndexIncFirst:
          case HIRVectorIdioms::CEIndexIncNext: {
            int64_t Stride;
            bool IsIncrement = HIRVectorIdioms::isIncrementInst(
                Idiom.get<const HLInst *>(), Stride);
            assert(IsIncrement && "Increment instruction expected.");
            (void)IsIncrement;
            Desc.addIncrement(
                cast<VPInstruction>(Decomposer.getVPValueForCEIdiom(Idiom)),
                Stride);
            break;
          }
          case HIRVectorIdioms::CEStore:
            Desc.addStore(
                cast<VPLoadStoreInst>(Decomposer.getVPValueForCEIdiom(Idiom)));
            break;
          case HIRVectorIdioms::CELoad:
            Desc.addLoad(
                cast<VPLoadStoreInst>(Decomposer.getVPValueForCEIdiom(Idiom)));
            break;
          case HIRVectorIdioms::CELdStIndex:
            Desc.addIndex(
                cast<VPInstruction>(Decomposer.getVPValueForCEIdiom(Idiom)));
            break;
          default:
            llvm_unreachable("Unexpected CE idiom id.");
          }

          const auto *LinkedIdioms = VecIdioms->getLinkedIdioms(Idiom);
          if (LinkedIdioms)
            for (const auto &LinkedIdiom : *LinkedIdioms)
              AddIdiom(LinkedIdiom);
        };

    AddIdiom(Idiom);
  }
};

class HLLoop2VPLoopMapper {
public:
  HLLoop2VPLoopMapper() = delete;
  explicit HLLoop2VPLoopMapper(
      const VPlanVector *Plan,
      SmallDenseMap<VPBasicBlock *, HLLoop *> Header2HLLoop) {

    std::function<void(const VPLoop *)> mapLoop2VPLoop =
        [&](const VPLoop *VPL) {
          const HLLoop *L = Header2HLLoop[VPL->getHeader()];
          assert(L != nullptr && "Can't find Loop");
          LoopMap[L] = VPL;
          // Capture opt-report remarks that are present for current loop in
          // incoming HIR.
          const_cast<VPLoop *>(VPL)->setOptReport(L->getOptReport());
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

template <typename DescrType>
using Converter =
    VPLoopEntitiesConverter<DescrType, HLLoop, HLLoop2VPLoopMapper>;

void PlainCFGBuilderHIR::convertEntityDescriptors(
    VPlanHCFGBuilder::VPLoopEntityConverterList &CvtVec) {
  auto RedCvt = std::make_unique<Converter<ReductionDescr>>(Plan);
  auto IndCvt = std::make_unique<Converter<InductionDescr>>(Plan);
  auto PrivCvt = std::make_unique<Converter<PrivateDescr>>(Plan);
  auto PrivNonPODCvt = std::make_unique<Converter<PrivateDescr>>(Plan);
  auto CEIdiomCvt = std::make_unique<Converter<CompressExpandIdiomDescr>>(Plan);

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

    auto Bind = [](auto &&Range, auto &&Converter) {
      return std::make_pair(std::ref(Range), std::move(Converter));
    };

    const HIRVectorIdioms *Idioms = Legal->getVectorIdioms(HL);

    // clang-format off
    RedCvt->createDescrList(HL,
      Bind(make_range(ReductionInputIteratorHIR(true, SRCL),
                      ReductionInputIteratorHIR(false, SRCL)),
           ReductionListCvt<ReductionInputIteratorHIR>{Decomposer}),
      Bind(Legal->getReductions(), ExplicitReductionListCvt{Decomposer}),
      Bind(make_range(MinMaxIdiomsInputIteratorHIR(true, *Idioms),
                      MinMaxIdiomsInputIteratorHIR(false, *Idioms)),
           ReductionListCvt<MinMaxIdiomsInputIteratorHIR>{Decomposer}),
      Bind(Legal->getUDRs(), ExplicitReductionListCvt{Decomposer}));

    IndCvt->createDescrList(HL,
      Bind(Decomposer.getInductions(HL), InductionListCvt{Decomposer}),
      // TODO: ArrayRef-based empty slice here serves as a stub because
      // LinearListCvt is not working correctly. Fix it when the converter
      // is fixed.
      Bind(makeArrayRef(Legal->getLinears()).take_front(0),
           LinearListCvt{Decomposer}));

    PrivCvt->createDescrList(HL,
      Bind(Legal->getPrivates(), PrivatesListCvt{Decomposer}));

    PrivNonPODCvt->createDescrList(HL,
      Bind(Legal->getNonPODPrivates(), PrivatesListCvt{Decomposer}));
    // clang-format on

    const HIRVectorIdioms *VecIdioms = Legal->getVectorIdioms(HL);
    CEIdiomCvt->createDescrList(
        HL, Bind(map_range(
                     VecIdioms->getIdiomsById(HIRVectorIdioms::CEIndexIncFirst),
                     [](const auto &Pair) { return Pair.first; }),
                 CompressExpandIdiomListCvt(Decomposer, VecIdioms)));
  }
  CvtVec.emplace_back(std::move(RedCvt));
  CvtVec.emplace_back(std::move(IndCvt));
  CvtVec.emplace_back(std::move(PrivCvt));
  CvtVec.emplace_back(std::move(PrivNonPODCvt));
  CvtVec.emplace_back(std::move(CEIdiomCvt));
}

bool VPlanHCFGBuilderHIR::buildPlainCFG(VPLoopEntityConverterList &CvtVec) {
  PlainCFGBuilderHIR PCFGBuilder(TheLoop, DDG, Plan, Header2HLLoop,
                                 HIRLegality);

  if (!PCFGBuilder.buildPlainCFG())
    return false;

  PCFGBuilder.convertEntityDescriptors(CvtVec);

  return true;
}

void VPlanHCFGBuilderHIR::passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) {
  typedef VPLoopEntitiesConverterTempl<HLLoop2VPLoopMapper> BaseConverter;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (VPlanPrintLegality) {
    // The HIRLegality lists should be populated by now
    HIRLegality->dump(dbgs());
  }
#endif

  HLLoop2VPLoopMapper Mapper(Plan, Header2HLLoop);
  for (auto &Cvt : Cvts) {
    BaseConverter *Converter = dyn_cast<BaseConverter>(Cvt.get());
    Converter->passToVPlan(Plan, Mapper);
  }
}
