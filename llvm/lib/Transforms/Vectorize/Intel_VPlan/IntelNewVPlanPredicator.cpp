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

static cl::opt<bool>
    PrintAfterLoopCFU("vplan-print-after-loop-cfu", cl::init(false), cl::Hidden,
                      cl::desc("Print VPlan after LoopCFU transformation."));

static cl::opt<bool> PrintAfterLinearization(
    "vplan-print-after-linearization", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan after predication and linearization."));

static cl::opt<bool> DotAfterLinearization(
    "vplan-dot-after-linearization", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan digraph after predication and linearization."));

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
    auto *DA = Plan.getVPlanDA();
    if (DA->isDivergent(*LHS) || DA->isDivergent(*RHS))
      DA->markDivergent(*Or);

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

VPValue *VPlanPredicator::createNot(VPValue *Cond) {
  auto *Not = Builder.createNot(Cond, Cond->getName() + ".not");
  auto *DA = Plan.getVPlanDA();
  if (DA->isDivergent(*Cond))
    DA->markDivergent(*Not);

  return Not;
}

void VPlanPredicator::calculatePredicateTerms(VPBlockBase *CurrBlock) {
  VPRegionBlock *Region = CurrBlock->getParent();
  // Blocks that dominate region exit inherit the predicate from the region.
  if (VPDomTree.dominates(CurrBlock, Region->getExit())) {
    assert(Block2PredicateTerms.count(Region) == 1 &&
           "Region should have been processed already!");
    Block2PredicateTerms[CurrBlock] = {{PredicateTerm(Region)}};
    return;
  }

  if (auto *PredBB = CurrBlock->getSinglePredecessor()) {
    if (PredBB->getSingleSuccessor() == CurrBlock) {
      // Re-use PredBB's block predicate - it is the same and we don't want to
      // emit VPInstructions to re-calculate it in generic way based on the
      // influencing conditions that affect CurrBlock's predicate.
      assert(Block2PredicateTerms.count(PredBB) == 1 &&
             "PredBB should have been processed already!");
      Block2PredicateTerms[CurrBlock] = {{PredicateTerm(PredBB)}};
      return;
    }
  }

  Block2PredicateTerms[CurrBlock] = {};

  for (auto *PredBB : CurrBlock->getPredecessors()) {
    auto *Cond = PredBB->getCondBit();
    assert((Cond || CurrBlock == PredBB->getSuccessors()[0]) &&
           "Single predecessor on false edge?");
    // Cond == nullptr would just mean that PredBB's predicate should be used.
    // Still ok.
    Block2PredicateTerms[CurrBlock].push_back(PredicateTerm(
        PredBB, Cond, CurrBlock != PredBB->getSuccessors()[0] /* Negate */));
  }
}

VPValue *VPlanPredicator::createValueForPredicateTerm(PredicateTerm Term) {
  auto *Block = Term.OriginBlock;
  auto *Val = Term.Condition;
  if (Term.Negate) {
    assert(Val && "Can't negate non-existing condition!");
    Val = createNot(Val);
  }
  VPValue *Predicate = Block->getPredicate();
  if (!Predicate)
    return Val;

  if (!Val)
    return Predicate;

  // TODO: Once we start presrving uniform control flow, there will be no
  // need to create "and" for uniform predicate that is true on all incoming
  // edges.
  auto *DA = Plan.getVPlanDA();
  bool IsDivergent = DA->isDivergent(*Val) || DA->isDivergent(*Predicate);
  Val = Builder.createAnd(Predicate, Val,
                          "vp." + Block->getName() + ".br." + Val->getName());
  if (IsDivergent)
    DA->markDivergent(*Val);
  return Val;
}

static void markPhisAsBlended(VPBlockBase *Block) {
  for (auto &PhiRecipe : Block->getEntryBasicBlock()->getVPPhis()) {
    auto *Phi = cast<VPPHINode>(&PhiRecipe);
    Phi->setBlend(true);
  }
}

void VPlanPredicator::linearizeRegion(
    const ReversePostOrderTraversal<VPBlockBase *> &RegionRPOT) {
  assert(RegionRPOT.begin() != RegionRPOT.end() &&
         "RegionRPOT can't be empty!");

  // Region entry handled during outer region processing.
  auto It = ++RegionRPOT.begin();
  auto End = RegionRPOT.end();

  while (It != End) {
    VPBlockBase *PrevBlock = *(It - 1);
    VPBlockBase *CurrBlock = *It;
    ++It;

    // Skip loop headers and latches to keep intact loop header predecessors and
    // loop latch successors.
    if (VPLI->isLoopHeader(CurrBlock)) {
      continue;
    }

    markPhisAsBlended(CurrBlock);

    if (VPBlockUtils::blockIsLoopLatch(PrevBlock, VPLI)) {
      // Condition above seems to rely on loop-simplified form, assert for it:
      assert(PrevBlock == CurrBlock->getSinglePredecessor() &&
             "Not in loop-simplified form?");

      // Preserve the exiting edge from the loop.
      // TODO: We probably turn LCSSA phis to blend phis by doing that continue
      // here. Not sure if it's bad or not.
      continue;
    }

    PrevBlock->clearSuccessors();
    CurrBlock->clearPredecessors();
    VPBlockUtils::connectBlocks(PrevBlock, CurrBlock);
  }
}

// Predicate and linearize the CFG within Region.
void VPlanPredicator::predicateAndLinearizeRegionRec(VPRegionBlock *Region,
                                                     bool SearchLoopHack) {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(Region->getEntry());
  VPDomTree.recalculate(*Region);

  for (VPBlockBase *Block : RPOT)
    calculatePredicateTerms(Block);

  if (!SearchLoopHack)
    linearizeRegion(RPOT);

  auto *DA = Plan.getVPlanDA();
  ReversePostOrderTraversal<VPBlockBase *> PostLinearizationRPOT(
      Region->getEntry());
  for (VPBlockBase *Block : PostLinearizationRPOT) {
    const auto &PredTerms = Block2PredicateTerms[Block];

    VPBuilder::InsertPointGuard Guard(Builder);
    auto *BB = Block->getEntryBasicBlock();
    auto It = BB->begin();

    if (VPLI->isLoopHeader(Block))
      while (It != BB->end() && isa<VPPHINode>(*It))
        ++It;

    Builder.setInsertPoint(BB, It);

    if (PredTerms.size() == 1 && PredTerms[0].Condition == nullptr) {
      // Re-use predicate of the OriginBlock.
      Block->setPredicate(PredTerms[0].OriginBlock->getPredicate());
      auto *Predicate = Block->getPredicate();
      if (Predicate && Block != Region->getEntry()) {
        // Pred for region entry created when processing the region itself.
        auto *BlockPredicateInst = Builder.createPred(Predicate);
        if (DA->isDivergent(*Predicate))
          DA->markDivergent(*BlockPredicateInst);
      }

      continue;
    }

    std::list<VPValue *> IncomingConditions;
    for (auto Term : PredTerms)
      if (auto *Val = createValueForPredicateTerm(Term))
        IncomingConditions.push_back(Val);

    auto *Predicate = genPredicateTree(IncomingConditions);
    Block->setPredicate(Predicate);
    if (Predicate) {
      auto *BlockPredicateInst = Builder.createPred(Predicate);
      if (DA->isDivergent(*Predicate))
        DA->markDivergent(*BlockPredicateInst);
    }
  }

  // Recurse inside Region
  for (auto *Block : make_range(RPOT.begin(), RPOT.end()))
    if (VPRegionBlock *SubRegion = dyn_cast<VPRegionBlock>(Block))
      predicateAndLinearizeRegionRec(SubRegion, SearchLoopHack);
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

    if (PrintAfterLoopCFU) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      outs() << "After inner loop control flow transformation\n";
      Plan.dump(outs(), true /* print DA info */);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    }
  }
#endif // INTEL_CUSTOMIZATION
  // Predicate the blocks within Region.
  Block2PredicateTerms[Plan.getEntry()] = {};

  predicateAndLinearizeRegionRec(cast<VPRegionBlock>(Plan.getEntry()),
                                 Exits.size() != 1 /* SearchLoopHack */);
#if INTEL_CUSTOMIZATION
  fixupUniformInnerLoops();
  LLVM_DEBUG(dbgs() << "VPlan after predication and linearization\n");
  LLVM_DEBUG(Plan.setName("Predicator: After predication\n"));
  LLVM_DEBUG(Plan.dump());

  if (PrintAfterLinearization) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      outs() << "After predication and linearization\n";
      Plan.dump(outs(), true /* print DA info */);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
  if (DotAfterLinearization) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    outs() << Plan;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
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
