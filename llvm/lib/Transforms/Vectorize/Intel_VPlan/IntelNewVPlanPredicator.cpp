//===-- VPlanPredicator.cpp -------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the VPlanPredicator class which contains the public
/// interfaces to predicate and linearize the VPlan region.
///
//===----------------------------------------------------------------------===//

#if INTEL_CUSTOMIZATION
#include "IntelNewVPlanPredicator.h"
#include "IntelVPlan.h"
#else
#include "VPlanPredicator.h"
#include "VPlan.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "VPlanPredicator"

using namespace llvm;
#if INTEL_CUSTOMIZATION
using namespace llvm::vpo;
extern cl::opt<bool> VPlanLoopCFU;
#define VPlanPredicator NewVPlanPredicator
#endif // INTEL_CUSTOMIZATION

// Generate VPInstructions at the beginning of CurrBB that calculate the
// predicate being propagated from PredBB to CurrBB depending on the edge type
// between them. For example if:
//  i.  PredBB is controlled by predicate %BP, and
//  ii. The edge PredBB->CurrBB is the false edge, controlled by the condition
//  bit value %CBV then this function will generate the following two
//  VPInstructions at the start of CurrBB:
//   %IntermediateVal = not %CBV
//   %FinalVal        = and %BP %IntermediateVal
// It returns %FinalVal.
VPValue *VPlanPredicator::getOrCreateNotPredicate(VPBasicBlock *PredBB,
#if INTEL_CUSTOMIZATION
                                                  VPBlockBase *CurrBB) {
#else
                                                  VPBasicBlock *CurrBB) {
#endif // INTEL_CUSTOMIZATION
  VPValue *CBV = PredBB->getCondBit();

  // Set the intermediate value - this is either 'CBV', or 'not CBV'
  // depending on the edge type.
  EdgeType ET = getEdgeTypeBetween(PredBB, CurrBB);
  VPValue *IntermediateVal = nullptr;
  switch (ET) {
  case EdgeType::TRUE_EDGE:
    // CurrBB is the true successor of PredBB - nothing to do here.
    IntermediateVal = CBV;
    break;

  case EdgeType::FALSE_EDGE:
    // CurrBB is the False successor of PredBB - compute not of CBV.
    IntermediateVal = Builder.createNot(CBV);
    break;
  }

  // Now AND intermediate value with PredBB's block predicate if it has one.
  VPValue *BP = PredBB->getPredicate();
  if (BP)
    return Builder.createAnd(BP, IntermediateVal);
  else
    return IntermediateVal;
}

// Generate a tree of ORs for all IncomingPredicates in  WorkList.
// Note: This function destroys the original Worklist.
//
// P1 P2 P3 P4 P5
//  \ /   \ /  /
//  OR1   OR2 /
//    \    | /
//     \   +/-+
//      \  /  |
//       OR3  |
//         \  |
//          OR4 <- Returns this
//           |
//
// The algorithm uses a worklist of predicates as its main data structure.
// We pop a pair of values from the front (e.g. P1 and P2), generate an OR
// (in this example OR1), and push it back. In this example the worklist
// contains {P3, P4, P5, OR1}.
// The process iterates until we have only one element in the Worklist (OR4).
// The last element is the root predicate which is returned.
VPValue *VPlanPredicator::genPredicateTree(std::list<VPValue *> &Worklist) {
  if (Worklist.empty())
    return nullptr;

  // The worklist initially contains all the leaf nodes. Initialize the tree
  // using them.
  while (Worklist.size() >= 2) {
    // Pop a pair of values from the front.
    VPValue *LHS = Worklist.front();
    Worklist.pop_front();
    VPValue *RHS = Worklist.front();
    Worklist.pop_front();

    // Create an OR of these values.
    VPValue *Or = Builder.createOr(LHS, RHS);

    // Push OR to the back of the worklist.
    Worklist.push_back(Or);
  }

#if !INTEL_CUSTOMIZATION // This assertion is not needed
  assert(Worklist.size() == 1 && "Expected 1 item in worklist");
#endif // INTEL_CUSTOMIZATION

  // The root is the last node in the worklist.
  VPValue *Root = Worklist.front();

  // This root needs to replace the existing block predicate. This is done in
  // the caller function.
  return Root;
}

// Return whether the edge FromBlock -> ToBlock is a TRUE_EDGE or FALSE_EDGE
VPlanPredicator::EdgeType
VPlanPredicator::getEdgeTypeBetween(VPBlockBase *FromBlock,
                                    VPBlockBase *ToBlock) {
  unsigned Count = 0;
  for (VPBlockBase *SuccBlock : FromBlock->getSuccessors()) {
    if (SuccBlock == ToBlock) {
      assert(Count < 2 && "Switch not supported currently");
      return (Count == 0) ? EdgeType::TRUE_EDGE : EdgeType::FALSE_EDGE;
    }
    Count++;
  }

  llvm_unreachable("Broken getEdgeTypeBetween");
}

// Generate all predicates needed for CurrBlock by going through its immediate
// predecessor blocks.
void VPlanPredicator::createOrPropagatePredicates(VPBlockBase *CurrBlock,
                                                  VPRegionBlock *Region) {
#if !INTEL_CUSTOMIZATION
  // Blocks that dominate region exit inherit the predicate from the region.
  // Return after setting the predicate.
  if (VPDomTree.dominates(CurrBlock, Region->getExit())) {
    VPValue *RegionBP = Region->getPredicate();
    CurrBlock->setPredicate(RegionBP);
    return;
  }
#endif // INTEL_CUSTOMIZATION

  // Collect all incoming predicates in a worklist.
  std::list<VPValue *> IncomingPredicates;

#if INTEL_CUSTOMIZATION
  // PHIs in non loop header blocks are currently converted to selects during
  // vector code generation - record the decision here where we have VPLoopInfo.
  // In future, this decision to blend needs to be revisited using DA
  // information.
  VPBasicBlock *CurrBB = CurrBlock->getEntryBasicBlock();
  bool BlendPhi = !VPLI->isLoopHeader(CurrBB);
  VPInstruction *FirstNonPHIInst = nullptr;

  for (auto I = CurrBB->begin(), E = CurrBB->end(); I != E; ++I) {
    VPRecipeBase *Ingredient = &*I;
    VPPHINode *VPPhi = dyn_cast<VPPHINode>(Ingredient);
    if (VPPhi)
      VPPhi->setBlend(BlendPhi);
    else {
      FirstNonPHIInst = dyn_cast<VPInstruction>(Ingredient);
      break;
    }
  }

  // TODO - the PHIs need to be the first instructions in a block. For now,
  // we are generating the predication related instructions at the start
  // of the block if we are going to blend the PHIs into selects as the
  // incoming predicates are used to generate the selects. This needs to
  // be changed to explicitly convert such PHIs to selects.
  if (BlendPhi || !FirstNonPHIInst)
    Builder.setInsertPoint(CurrBB, CurrBB->begin());
  else
    Builder.setInsertPoint(FirstNonPHIInst);
#else
  // Set the builder's insertion point to the top of the current BB
  VPBasicBlock *CurrBB = cast<VPBasicBlock>(CurrBlock->getEntryBasicBlock());
  Builder.setInsertPoint(CurrBB, CurrBB->begin());
#endif // INTEL_CUSTOMIZATION

  // For each predecessor, generate the VPInstructions required for
  // computing 'BP AND (not) CBV" at the top of CurrBB.
  // Collect the outcome of this calculation for all predecessors
  // into IncomingPredicates.
  for (VPBlockBase *PredBlock : CurrBlock->getPredecessors()) {
    // Skip back-edges.
    if (VPBlockUtils::isBackEdge(PredBlock, CurrBlock, VPLI))
      continue;

    VPValue *IncomingPredicate = nullptr;
    unsigned NumPredSuccsNoBE =
        VPBlockUtils::countSuccessorsNoBE(PredBlock, VPLI);

    // If there is an unconditional branch to the currBB, then we don't create
    // edge predicates. We use the predecessor's block predicate instead.
    if (NumPredSuccsNoBE == 1)
      IncomingPredicate = PredBlock->getPredicate();
    else if (NumPredSuccsNoBE == 2) {
      // Emit recipes into CurrBlock if required
      assert(isa<VPBasicBlock>(PredBlock) && "Only BBs have multiple exits");
      IncomingPredicate =
#if INTEL_CUSTOMIZATION
          getOrCreateNotPredicate(cast<VPBasicBlock>(PredBlock), CurrBlock);
#else
          getOrCreateNotPredicate(cast<VPBasicBlock>(PredBlock), CurrBB);
#endif // INTEL_CUSTOMIZATION
    } else
      llvm_unreachable("FIXME: switch statement ?");

    if (IncomingPredicate)
      IncomingPredicates.push_back(IncomingPredicate);
#if INTEL_CUSTOMIZATION
    // Push the incoming predicate into current basic block's incoming masks so
    // that we can use it to blend phis to selects.
    CurrBB->addMaskBlockPair(IncomingPredicate, PredBlock->getExitBasicBlock());
#endif // INTEL_CUSTOMIZATION
  }

#if INTEL_CUSTOMIZATION
  // Blocks that dominate region exit inherit the predicate from the region.
  // Return after setting the predicate.
  if (VPDomTree.dominates(CurrBlock, Region->getExit())) {
    VPValue *RegionBP = Region->getPredicate();
    CurrBlock->setPredicate(RegionBP);

    // Generate the VPInstruction which marks the block predicate value.
    if (RegionBP) {
      VPBasicBlock *InsertBB;
      if (auto *Rgn = dyn_cast<VPRegionBlock>(CurrBlock))
        InsertBB = Rgn->getEntryBasicBlock();
      else
        InsertBB = cast<VPBasicBlock>(CurrBlock);

      if (InsertBB != Region->getEntryBasicBlock()) {
        Builder.createPred(RegionBP);
      }
    }

    return;
  }
#endif // INTEL_CUSTOMIZATION
  // Logically OR all incoming predicates by building the Predicate Tree.
  VPValue *Predicate = genPredicateTree(IncomingPredicates);
  assert(Predicate && "No predicate generated from Predicate Tree."); // INTEL

  // Now update the block's predicate with the new one.
  CurrBlock->setPredicate(Predicate);
#if INTEL_CUSTOMIZATION
  // Generate the VPInstruction which marks the block predicate value.
  Builder.createPred(Predicate);
#endif // INTEL_CUSTOMIZATION
}

// Generate all predicates needed for Region.
void VPlanPredicator::predicateRegionRec(VPRegionBlock *Region) {
  VPBasicBlock *EntryBlock = cast<VPBasicBlock>(Region->getEntry());
  ReversePostOrderTraversal<VPBlockBase *> RPOT(EntryBlock);

#if INTEL_CUSTOMIZATION
  // FIXME: Predicator is currently computing the dominator information for the
  // region. Once we start storing dominator information in a VPRegionBlock,
  // we can avoid this recalculation.
  VPDomTree.recalculate(*Region);
#endif // INTEL_CUSTOMIZATION
  // Generate edge predicates and append them to the block predicate. RPO is
  // necessary since the predecessor blocks' block predicate needs to be set
  // before the current block's block predicate can be computed.
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end())) {
#if !INTEL_CUSTOMIZATION
    // TODO: Handle nested regions once we start generating the same.
    assert(!isa<VPRegionBlock>(Block) && "Nested region not expected");
#endif // INTEL_CUSTOMIZATION
    createOrPropagatePredicates(Block, Region);
  }
#if INTEL_CUSTOMIZATION
  // Predicate subregions
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end()))
    if (auto SubRegion = dyn_cast<VPRegionBlock>(Block))
      predicateRegionRec(SubRegion);
#endif // INTEL_CUSTOMIZATION
}

// Linearize the CFG within Region.
// TODO: Predication and linearization need RPOT for every region.
// This traversal is expensive. Since predication is not adding new
// blocks, we should be able to compute RPOT once in predication and
// reuse it here. This becomes even more important once we have nested
// regions.
void VPlanPredicator::linearizeRegionRec(VPRegionBlock *Region) {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(Region->getEntry());
  VPBlockBase *PrevBlock = nullptr;

  for (VPBlockBase *CurrBlock : make_range(RPOT.begin(), RPOT.end())) {
#if !INTEL_CUSTOMIZATION
    // TODO: Handle nested regions once we start generating the same.
    assert(!isa<VPRegionBlock>(CurrBlock) && "Nested region not expected");
#endif // INTEL_CUSTOMIZATION
    // Linearize control flow by adding an unconditional edge between PrevBlock
    // and CurrBlock skipping loop headers and latches to keep intact loop
    // header predecessors and loop latch successors.
    if (PrevBlock && !VPLI->isLoopHeader(CurrBlock) &&
        !VPBlockUtils::blockIsLoopLatch(PrevBlock, VPLI)) {

      LLVM_DEBUG(dbgs() << "Linearizing: " << PrevBlock->getName() << "->"
                        << CurrBlock->getName() << "\n");

      PrevBlock->clearSuccessors();
      CurrBlock->clearPredecessors();
      VPBlockUtils::connectBlocks(PrevBlock, CurrBlock);
    }

    PrevBlock = CurrBlock;
  }
#if INTEL_CUSTOMIZATION
  // 2. Recurse inside Region
  for (auto *Block : make_range(RPOT.begin(), RPOT.end()))
    if (VPRegionBlock *SubRegion = dyn_cast<VPRegionBlock>(Block))
      linearizeRegionRec(SubRegion);
#endif // INTEL_CUSTOMIZATION
}

#if INTEL_CUSTOMIZATION
#include "IntelVPlanLoopCFU.h"
void VPlanPredicator::fixupUniformInnerLoops(void) {
  // Uniform sub Loop regions need bottom test to be fixed to take into account
  // the predicate of the loop pre-header. We fix the bottom test to take care
  // of the case where the loop should never be entered to begin with. The fix
  // below will enter the inner loop once but the generated code should still be
  // functionally correct since all the loop blocks are appropriately masked.
  // Another way to handle the same would be to add an allzero by pass around
  // such subloops that are under a predicate.
  for (auto *LoopRegion : FixupLoopRegions) {
    auto *Loop = LoopRegion->getVPLoop();
    auto *LoopPH = cast<VPBasicBlock>(Loop->getLoopPreheader());
    auto *LoopPHBP = LoopPH->getPredicate();

    // Nothing to do if this is not an inner loop or if the loop is not under
    // a predicate.
    if (!Loop->getParentLoop() || !LoopPHBP)
      continue;

    auto *LoopLatch = cast<VPBasicBlock>(Loop->getLoopLatch());
    auto *LoopHeader = cast<VPBasicBlock>(Loop->getHeader());
    bool BackEdgeIsFalseSucc = LoopLatch->getSuccessors()[1] == LoopHeader;
    VPValue *LatchCond = LoopLatch->getCondBit();
    VPBuilder::InsertPointGuard Guard(Builder);
    Builder.setInsertPoint(LoopLatch);

    // Check if the loop should not be entered for all lanes
    auto *NewAllZeroCheck = Builder.createAllZeroCheck(LoopPHBP);

    // If the loop back edge is the false successor of the loop latch,
    // we exit the loop if either the all zero check is true or the
    // latch condition bit is true. Otherwise, we take the back edge
    // if the latch condition is true and the all zero check is false.
    VPValue *NewCondBit;
    if (!BackEdgeIsFalseSucc) {
      NewAllZeroCheck = Builder.createNot(NewAllZeroCheck);
      NewCondBit = Builder.createAnd(NewAllZeroCheck, LatchCond);
    } else {
      NewCondBit = Builder.createOr(NewAllZeroCheck, LatchCond);
    }
    LoopLatch->setCondBit(NewCondBit);
  }
}
#endif // INTEL_CUSTOMIZATION
// Entry point. The driver function for the predicator.
void VPlanPredicator::predicate(void) {
#if INTEL_CUSTOMIZATION
  assert(VPLI->size() == 1 && "more than 1 loop?");
  VPBlockBase *PH = (*VPLI->begin())->getLoopPreheader();
  assert(PH && "Unexpected null pre-header!");
  VPLoopRegion *EntryLoopR = cast<VPLoopRegion>(PH->getParent());
  const VPLoop *VPL = EntryLoopR->getVPLoop();
  SmallVector<VPBlockBase *, 4> Exits;
  VPL->getExitBlocks(Exits);

  // Transform inner loop control to become uniform.
  if (VPlanLoopCFU) {
    LLVM_DEBUG(dbgs() << "Before inner loop control flow transformation\n");
    LLVM_DEBUG(Plan.dump());
    handleInnerLoopBackedges(EntryLoopR);
    LLVM_DEBUG(dbgs() << "After inner loop control flow transformation\n");
    LLVM_DEBUG(Plan.dump());
  }
#endif // INTEL_CUSTOMIZATION
  // Predicate the blocks within Region.
  predicateRegionRec(cast<VPRegionBlock>(Plan.getEntry()));

  // Linearlize the blocks with Region.
  if (Exits.size() == 1) // INTEL - search loops need linearization suppressed
    linearizeRegionRec(cast<VPRegionBlock>(Plan.getEntry()));
#if INTEL_CUSTOMIZATION
  fixupUniformInnerLoops();
  LLVM_DEBUG(dbgs() << "VPlan after predication and linearization\n");
  LLVM_DEBUG(Plan.setName("Predicator: After predication\n"));
  LLVM_DEBUG(Plan.dump());
#endif // INTEL_CUSTOMIZATION
}

VPlanPredicator::VPlanPredicator(VPlan &Plan)
#if INTEL_CUSTOMIZATION
    : Plan(Plan), VPLI(Plan.getVPLoopInfo()) {
#else
    : Plan(Plan), VPLI(&(Plan.getVPLoopInfo())) {
#endif // INTEL_CUSTOMIZATION
  // FIXME: Predicator is currently computing the dominator information for the
  // top region. Once we start storing dominator information in a VPRegionBlock,
  // we can avoid this recalculation.
#if INTEL_CUSTOMIZATION
  // VPDomTree.recalculate(*(cast<VPRegionBlock>(Plan.getEntry())));
#else
  VPDomTree.recalculate(*(cast<VPRegionBlock>(Plan.getEntry())));
#endif // INTEL_CUSTOMIZATION
}
#if INTEL_CUSTOMIZATION
#undef VPlanPredicator
#endif // INTEL_CUSTOMIZATION
