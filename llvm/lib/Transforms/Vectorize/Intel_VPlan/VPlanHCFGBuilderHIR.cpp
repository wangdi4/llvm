//===-- VPlanHCFGBuilderHIR.cpp -------------------------------------------===//
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

#include "VPlanHCFGBuilderHIR.h"
#include "../Intel_VPlanBuilderHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"

#define DEBUG_TYPE "VPlanHCFGBuilder"

using namespace llvm;
using namespace vpo;

// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// OneByOneRecipe's and ConditionBitRecipe's. Return VPRegionBlock that
// encloses all the VPBasicBlock's of the plain CFG.
class PlainCFGBuilderHIR : public HLNodeVisitorBase {
  friend HLNodeVisitor<PlainCFGBuilderHIR, false /*Recursive*/>;

private:
  /// Outermost loop of the input loop nest.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest. It is
  /// used to navigate from sink blobs to their respective source blobs.
  const DDGraph &DDG;

  IntelVPlanUtils &PlanUtils;

  /// Map between loop header VPBasicBlock's and their respective HLLoop's. It
  /// is populated in this phase to keep the information necessary to create
  /// VPLoopRegionHIR's later in the H-CFG construction process.
  SmallDenseMap<VPBasicBlock *, HLLoop *> &Header2HLLoop;

  /// VPInstruction builder for HIR.
  VPBuilderHIR VPHIRBuilder;

  /// Output TopRegion.
  VPRegionBlock *TopRegion = nullptr;
  /// Number of VPBasicBlocks in TopRegion.
  unsigned TopRegionSize = 0;

  /// Hold the set of dangling predecessors to be connected to the next active
  /// VPBasicBlock.
  std::deque<VPBasicBlock *> Predecessors;

  /// Hold the VPBasicBlock that is being populated with instructions. Null
  /// value indicates that a new active VPBasicBlock has to be created.
  VPBasicBlock *ActiveVPBB = nullptr;

  /// Map between HLNode's that open a VPBasicBlock and such VPBasicBlock's.
  DenseMap<HLNode *, VPBasicBlock *> HLN2VPBB;

  /// Map between HLInst's and their respective VPValue's representing their
  /// definition.
  DenseMap<HLDDNode *, VPValue *> HLDef2VPValue;

  VPBasicBlock *createOrGetVPBB(HLNode *HNode = nullptr);
  void connectVPBBtoPreds(VPBasicBlock *VPBB);
  void updateActiveVPBB(HLNode *HNode = nullptr, bool IsPredecessor = true);

  // VPInstruction methods
  VPValue *createNoOperandVPInst(HLDDNode *DDNode = nullptr);
  VPValue *createOrGetVPDefFrom(const DDEdge *Edge);
  VPValue *createOrGetVPOperand(RegDDRef *HIROp);
  void buildVPOpsForDDNode(HLDDNode *HInst,
                           SmallVectorImpl<VPValue *> &VPValueOps);
  void createOrFixVPInstr(HLDDNode *DDNode);

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
  PlainCFGBuilderHIR(HLLoop *Lp, const DDGraph &DDG, IntelVPlanUtils &Utils,
                     SmallDenseMap<VPBasicBlock *, HLLoop *> &H2HLLp)
      : TheLoop(Lp), DDG(DDG), PlanUtils(Utils), Header2HLLoop(H2HLLp) {}

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
    VPBasicBlock *NewVPBB = PlanUtils.createBasicBlock();
    PlanUtils.setBlockParent(NewVPBB, TopRegion);
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
      DEBUG(dbgs() << "Creating VPBasicBlock for " << HNode->getHLNodeID()
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
    PlanUtils.appendBlockSuccessor(Pred, VPBB);
    PlanUtils.appendBlockPredecessor(VPBB, Pred);
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
    VPHIRBuilder.setInsertPoint(ActiveVPBB);

    if (IsPredecessor)
      Predecessors.push_back(ActiveVPBB);
  }
}

// Return the Constant representation of a constant RegDDRef.
static Constant *getConstantFromHIR(RegDDRef *RDDRef) {
  assert(RDDRef->getSingleCanonExpr() &&
         "Constant CanonExpr that is not Single CanonExpr?");
  CanonExpr *CExpr = RDDRef->getSingleCanonExpr();

  if (CExpr->isIntConstant()) {
    int64_t CECoeff = CExpr->getConstant();
    Type *CETy = CExpr->getDestType();

    // Null value for pointer types needs special treatment
    if (CECoeff == 0 && CETy->isPointerTy()) {
      return Constant::getNullValue(CETy);
    }
    return ConstantInt::getSigned(CETy, CECoeff);
  }

  ConstantFP *FPConst;
  if (CExpr->isFPConstant(&FPConst))
    return FPConst;

  if (CExpr->isNull())
    return ConstantPointerNull::get(cast<PointerType>(CExpr->getDestType()));

  llvm_unreachable("Unsupported HIR Constant.");
}

// Return the VPInstruction opcode for a given HLDDNode.
static unsigned getOpcodeFromHIR(HLDDNode *DDNode) {
  if (auto *HInst = dyn_cast<HLInst>(DDNode)) {
    assert(HInst->getLLVMInstruction() &&
           "Missing LLVM Instruction for HLInst.");
    return HInst->getLLVMInstruction()->getOpcode();
  }

  if (auto *HIf = dyn_cast<HLIf>(DDNode)) {
    assert(HIf->getNumPredicates() && "HLIf with no predicate?");
    Type *PredType = (*HIf->ddref_begin())->getDestType();

    // HIR only generates multiple predicates for integers.
    if (HIf->getNumPredicates() > 1 || PredType->isIntOrIntVectorTy()) {
      return Instruction::ICmp;
    }

    assert(PredType->isFPOrFPVectorTy() && "Expected a floating point type");
    return Instruction::FCmp;
  }

  llvm_unreachable("Missing opcode for HLInst.");
}

// Create a VPInstruction with no operands and do not insert it in any
// VPBasicBlock.
VPValue *PlainCFGBuilderHIR::createNoOperandVPInst(HLDDNode *DDNode) {
  VPBuilder::InsertPointGuard Guard(VPHIRBuilder);
  VPHIRBuilder.clearInsertionPoint();
  unsigned Opcode = DDNode ? getOpcodeFromHIR(DDNode) : 0 /*No operand*/;
  VPValue *NewVPVal =
      VPHIRBuilder.createNaryOp(Opcode, {} /*No operands*/, DDNode);
  return NewVPVal;
}

// Returns the VPValue that defines Edge's sink.
VPValue *PlainCFGBuilderHIR::createOrGetVPDefFrom(const DDEdge *Edge) {
  // Get the HLDDNode causing the definition.
  HLDDNode *DefNode = Edge->getSrc()->getHLDDNode();
  auto VPValIt = HLDef2VPValue.find(DefNode);

  // Return the VPValue associated to the HLDDNode definition if it has been
  // visited previously.
  if (VPValIt != HLDef2VPValue.end())
    return VPValIt->second;

  // HLDDNode definition hasn't been visited yet. Create VPInstruction without
  // operands and do not insert it in the VPBasicBlock. This VPInstruciton will
  // be fixed and inserted when the HLDDNode definition is processed in
  // createVPInstructionsForRange.
  VPValue *NewVPInst = createNoOperandVPInst(DefNode);
  HLDef2VPValue[DefNode] = NewVPInst;
  return NewVPInst;
}

// Helper function that is used by 'buildVPOpsForDDNode' to create a new VPValue
// or retrieve an existing one for an HLDDNode's operand (HIROp). This function
// must only be used to create/retrieve VPValues for *HLDDNode's operands*
// and not to create regular VPInstruction's. For the latter, you should look at
// 'createOrFixVPInstr'.
VPValue *PlainCFGBuilderHIR::createOrGetVPOperand(RegDDRef *HIROp) {
  if (HIROp->isConstant()) {
    return PlanUtils.getVPlan()->getVPConstant(getConstantFromHIR(HIROp));
  }

  if (HIROp->isUnitaryBlob()) {
    // Operand represents a single temporal that doesn't need decomposition.
    // Conversions or single temporals with constant additive != 1 will not hit
    // here.
    auto OpInEdges = DDG.incoming(HIROp);

    // If operand has incoming DD edges, we need to retrieve (or create) the
    // VPValues associated to the DD sources (definition). If there are
    // multiple definitions, in addition, we introduce a semi-phi operation that
    // "blends" all the VPValue definitions.
    if (OpInEdges.begin() != OpInEdges.end()) {
      // Single definition.
      if (std::next(OpInEdges.begin()) == OpInEdges.end())
        return createOrGetVPDefFrom(*OpInEdges.begin());

      // Multiple definitions.
      SmallVector<VPValue *, 4> OpVPDefs;
      for (const DDEdge *Edge : OpInEdges) {
        OpVPDefs.push_back(createOrGetVPDefFrom(Edge));
      }

      return VPHIRBuilder.createSemiPhiOp(OpVPDefs);
    }

    // Operand has no incoming DD edges. This means that HIROp is a use whose
    // definition is outside VPlan.
    VPValue *NewVPVal = createNoOperandVPInst(0 /*No opcode*/);
    PlanUtils.getVPlan()->addExternalDef(cast<VPInstruction>(NewVPVal));
    return NewVPVal;
  }

  // Operand is a complex RegDDRef that needs decomposition. As it may contain
  // different temps (uses), we cannot introduce them into the VPValue U-D
  // chain until decomposition happens. For that reason, we use a VPValue to
  // mark that operand needs to be fixed later.
  // FIXME: This memory is not being freed. It will be fixed when introducing
  // decomposition.
  return new VPValue();
}

// Return a sequence of VPValues (VPValueOps) that represents DDNode's operands
// in VPlan. In addition to the RegDDRef to VPValue translation, operands are
// sorted in the way VPlan expects them. Some operands, such as the LHS operand
// in some HIR instructions, are ignored because they are not explicitly
// represented as an operand in VPlan.
void PlainCFGBuilderHIR::buildVPOpsForDDNode(
    HLDDNode *DDNode, SmallVectorImpl<VPValue *> &VPValueOps) {

  auto *HInst = dyn_cast<HLInst>(DDNode);
  bool IsStore =
      HInst && HInst->getLLVMInstruction()->getOpcode() == Instruction::Store;

  // Collect operands necessary to build a VPInstruction out of an HLInst and
  // translate them into VPValue's. We skip LHS operands for most instructions.
  for (RegDDRef *HIROp :
       make_range(DDNode->op_ddref_begin(), DDNode->op_ddref_end())) {
    if (HIROp->isLval() && !IsStore)
      continue;

    VPValueOps.push_back(createOrGetVPOperand(HIROp));
  }

  // Fix discrepancies in the order of operands between HLInst and
  // VPInstruction:
  //     - Store: dest = store src -> store src dest
  if (IsStore)
    std::iter_swap(VPValueOps.begin(), std::next(VPValueOps.begin()));
}

// Main helper function that creates a VPInstruction for DDNode and inserts it
// in the active VPBasicBlock and the HLDef2VPValue map. If a VPInstruction was
// created before for this DDNode, the VPInstruction is retrieved, its operands
// are created and it's inserted in the active VPBasicBlock.
void PlainCFGBuilderHIR::createOrFixVPInstr(HLDDNode *DDNode) {

  DEBUG(dbgs() << "Creating or fixing:"; DDNode->dump(); dbgs() << "\n");

  // Translate HIR operands into VPValue operands. This needs to happen before
  // creating the VPInstruction because it may introduce new VPInstructions for
  // operands (e.g., semi-phis).
  SmallVector<VPValue *, 4> VPValueOps;
  buildVPOpsForDDNode(DDNode, VPValueOps);

  auto VPValIt = HLDef2VPValue.find(DDNode);
  VPInstruction *NewVPInst;

  if (VPValIt != HLDef2VPValue.end()) {
    // DDNode is a definition with a user that has been previously visited. We
    // have to set its operands properly and insert it into the
    // VPBasicBlock/Recipe.
    NewVPInst = cast<VPInstruction>(VPValIt->second);
    VPHIRBuilder.insert(NewVPInst);
  } else {
    // Create new VPInstruction.
    // NOTE: We set operands later to factorize code in 'if' and 'else'
    // branches.
    NewVPInst = cast<VPInstruction>(VPHIRBuilder.createNaryOp(
        getOpcodeFromHIR(DDNode), {} /*No operands*/, DDNode));

    HLDef2VPValue[DDNode] = NewVPInst;
  }

  // Set VPInstruction's operands.
  for (VPValue *Operand : VPValueOps) {
    NewVPInst->addOperand(Operand);
  }
}

void PlainCFGBuilderHIR::visit(HLLoop *HLp) {

  // TODO: Print something more useful.
  DEBUG(dbgs() << "Visiting HLLoop: " << HLp->getHLNodeID() << "\n");

  // - ZTT for inner loops -
  // TODO: isInnerMost(), ztt_pred_begin/end

  // - Loop PH -
  // Force creation of a new VPBB for PH.
  ActiveVPBB = nullptr;

  if (HLp->hasPreheader()) {
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
  }

  // - Loop H -
  // Force creation of a new VPBB for H.
  ActiveVPBB = nullptr;
  HLNodeUtils::visitRange<false /*Recursive*/>(
      *this /*visitor*/, HLp->child_begin(), HLp->child_end());

  // Map loop header VPBasicBlock with HLLoop for later loop region detection.
  VPBasicBlock *Header = HLN2VPBB[&*HLp->child_begin()];
  assert(Header && "Expected VPBasicBlock for loop header.");
  Header2HLLoop[Header] = HLp;

  // An HLoop will always have a single latch that will be also an exiting
  // block. Keep track of it. If there is no active VPBB, we have to create a
  // new one.
  // TODO: Materialize exit condition.
  updateActiveVPBB();
  // Connect Latch to Header and add ConditionBitRecipe.
  // TODO: Workaround. Setting a fake ConditionBitRecipe.
  PlanUtils.connectBlocks(ActiveVPBB /*Latch*/, Header);
  PlanUtils.setBlockCondBitVPVal(ActiveVPBB /*Latch*/, new VPValue());

  // - Loop Exits -
  // Force creation of a new VPBB for Exit.
  ActiveVPBB = nullptr;

  if (HLp->hasPostexit()) {
    HLNodeUtils::visitRange<false /*Recursive*/>(
        *this /*visitor*/, HLp->post_begin(), HLp->post_end());

    assert(ActiveVPBB == HLN2VPBB[&*HLp->post_begin()] &&
           "Loop Exit generates more than one VPBB?");
  } else {
    // There is no Exit in HLLoop. Create dummy VPBB as Exit (see comment for
    // dummy PH).
    updateActiveVPBB();
  }
}

void PlainCFGBuilderHIR::visit(HLIf *HIf) {

  // - Condition -
  // We do not create a new active  VPBasicBlock for HLIf predicates
  // (condition). We reuse the previous one (if possible).
  // TODO: Predicates in HLIf are not HLInst's but CmpInst! We have to process
  // them separately and manually, creating VPInstructions for them and
  // combining them with AND operations.
  updateActiveVPBB(HIf);

  // Create (single, not decomposed) VPInstruction for HLIf's predicate.
  createOrFixVPInstr(HIf);

  VPBasicBlock *ConditionVPBB = ActiveVPBB;
  // assert("HLIf condition generates more than one VPBB?");
  // TODO: Workaround. Setting a fake ConditionBitRecipe.
  PlanUtils.setBlockCondBitVPVal(ActiveVPBB, nullptr);

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

  // Create VPInstruction for HInst
  createOrFixVPInstr(HInst);
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

  // Create (or get) a new VPBB for HLLabel and connect to HLGoto's VPBB.
  HLLabel *Label = HGoto->getTargetLabel();
  VPBasicBlock *LabelVPBB = createOrGetVPBB(Label);
  PlanUtils.connectBlocks(ActiveVPBB, LabelVPBB);

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
  TopRegion = PlanUtils.createRegion(false /*isReplicator*/);

  // Create a dummy VPBB as TopRegion's Entry.
  assert(!ActiveVPBB && "ActiveVPBB must be null.");
  updateActiveVPBB();
  PlanUtils.setRegionEntry(TopRegion, ActiveVPBB);

  // Trigger the visit of the loop nest.
  visit(TheLoop);

  // Create a dummy VPBB as TopRegion's Exit.
  ActiveVPBB = nullptr;
  updateActiveVPBB();
  PlanUtils.setRegionExit(TopRegion, ActiveVPBB);

  PlanUtils.setRegionSize(TopRegion, TopRegionSize);

  return TopRegion;
}

VPRegionBlock *VPlanHCFGBuilderHIR::buildPlainCFG() {
  PlainCFGBuilderHIR PCFGBuilder(TheLoop, DDG, PlanUtils, Header2HLLoop);
  VPRegionBlock *TopRegion = PCFGBuilder.buildPlainCFG();
  return TopRegion;
}

VPLoopRegion *VPlanHCFGBuilderHIR::createLoopRegion(VPLoop *VPLp) {
  assert(isa<VPBasicBlock>(VPLp->getHeader()) &&
         "Expected VPBasicBlock as Loop header.");
  HLLoop *HLLp = Header2HLLoop[cast<VPBasicBlock>(VPLp->getHeader())];
  assert(HLLp && "Expected HLLoop");
  return VPlanHCFGBuilderBase::PlanUtils.createLoopRegionHIR(VPLp, HLLp);
}

