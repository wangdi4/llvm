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
#include "IntelVPlanIDF.h"
#else
#include "VPlanPredicator.h"
#include "VPlan.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/MapVector.h"
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

static cl::opt<bool> PreserveUniformCFG(
    "vplan-preserve-uniform-branches", cl::init(true), cl::Hidden,
    cl::desc("Preserve uniform branches during linearization."));

static cl::opt<bool> SortBlendPhisInPredicator(
    "vplan-sort-blend-phis-in-predicator", cl::init(false), cl::Hidden,
    cl::desc("Sort incoming blocks of blend phis in the predicator."));

namespace llvm {
namespace vpo {
cl::opt<bool> DisableLCFUMaskRegion(
    "disable-vplan-cfu-mask-region", cl::init(true), cl::Hidden,
    cl::desc("Disable construction of non-loop mask subregion in LoopCFU"));
}
} // namespace llvm

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

VPValue *VPlanPredicator::getOrCreateNot(VPValue *Cond) {
  auto It = Cond2NotCond.find(Cond);
  if (It != Cond2NotCond.end())
    return It->second;

  auto *Inst = dyn_cast<VPInstruction>(Cond);
  VPBuilder::InsertPointGuard Guard(Builder);
  VPBasicBlock *InsertBB =
      Inst ? Inst->getParent() : Plan.getEntry()->getEntryBasicBlock();

  Builder.setInsertPoint(InsertBB, InsertBB->end());
  auto *Not = Builder.createNot(Cond, Cond->getName() + ".not");

  auto *DA = Plan.getVPlanDA();
  if (DA->isDivergent(*Cond))
    DA->markDivergent(*Not);

  Cond2NotCond[Cond] = Not;
  return Not;
}

static void getPostDomFrontier(VPBlockBase *Block,
                               VPPostDominatorTree &PDT,
                               SmallPtrSetImpl<VPBlockBase *> &Frontier) {
  assert(Frontier.empty() && "Output set isn't empty on the entry!");
  // TODO: LLORG's templated DomFrontier uses DFS numbering. Not sure if that's
  // needed to ensure some particular traversal order.
  SmallVector<VPBlockBase *, 8> PostDominatedBlocks;
  PDT.getDescendants(Block, PostDominatedBlocks);
  // getDescendants includes the node itself into the list too, no need to
  // special case for it.
  for (VPBlockBase *B : PostDominatedBlocks)
    for (VPBlockBase *Pred : B->getPredecessors())
      if (!PDT.dominates(Block, Pred))
        Frontier.insert(Pred);
}

void VPlanPredicator::calculatePredicateTerms(VPBlockBase *CurrBlock) {
  LLVM_DEBUG(dbgs() << "Calculating predicate terms for "
                    << CurrBlock->getName() << ":\n");
  VPRegionBlock *Region = CurrBlock->getParent();
  // Blocks that dominate region exit inherit the predicate from the region.
  if (VPDomTree.dominates(CurrBlock, Region->getExit())) {
    assert(Block2PredicateTermsAndUniformity.count(Region) == 1 &&
           "Region should have been processed already!");
    PredicateTerm Term(Region);
    Block2PredicateTermsAndUniformity[CurrBlock] = {
        {Term}, Block2PredicateTermsAndUniformity[Region].second};
    PredicateTerm2UseBlocks[Term].push_back(CurrBlock);
    LLVM_DEBUG(dbgs() << " Re-using region's predicate, {Block: "
                      << Region->getName() << ", Uniformity: "
                      << Block2PredicateTermsAndUniformity[Region].second
                      << "}\n");
    return;
  }

  if (auto *PredBB = CurrBlock->getSinglePredecessor()) {
    if (PredBB->getSingleSuccessor() == CurrBlock) {
      // Re-use PredBB's block predicate - it is the same and we don't want to
      // emit VPInstructions to re-calculate it in generic way based on the
      // influencing conditions (post-dom frontier) that affect CurrBlock's
      // predicate.
      assert(Block2PredicateTermsAndUniformity.count(PredBB) == 1 &&
             "PredBB should have been processed already!");
      PredicateTerm Term(PredBB);
      Block2PredicateTermsAndUniformity[CurrBlock] = {
          {Term}, Block2PredicateTermsAndUniformity[PredBB].second};
      PredicateTerm2UseBlocks[Term].push_back(CurrBlock);
      LLVM_DEBUG(dbgs() << " Re-using previous block predicate, {Block: "
                        << PredBB->getName() << ", Uniformity: "
                        << Block2PredicateTermsAndUniformity[PredBB].second
                        << "}\n");
      return;
    }
  }

  Block2PredicateTermsAndUniformity[CurrBlock] = {};

  SmallPtrSet<VPBlockBase *, 12> Frontier;
  getPostDomFrontier(CurrBlock, VPPostDomTree, Frontier);

  // Conditional branches that are "interesting" for computing the predicate for
  // the current block are exactly the ones forming the post dominance frontier.
  // Branches that we post-dominate converge before reaching the current block,
  // and branches that aren't in neither post dominance frontier, nor in the set
  // of the one that CurrBlock dominates, affect CurrBlock indirectly only (via
  // the block predicate of the blocks forming the frontier).
  //
  //        P----------+
  //        |          |
  //        |          |
  //     +- O          |
  //     |  |          |   E      - CurrBlock
  //     |  A ----     |   {A, O} - its post-dom frontier
  //     |  |     |    |
  //     |  B     |    |
  //     \ / \    |    |
  //      C   D  /     |
  //       \ /  /      |
  //        E  /       |
  //        | /        |
  //        F          |
  //
  // Current block will have uniform predicate only if all the blocks in its
  // post-dom frontier have uniform predicates and uniform condition bits.
  bool Uniform = true;
  for (auto *InfluenceBB : Frontier) {
    // Ignore latches of loops that CurrBlock belongs to. Needed for the
    // flattened CFG. Note that latches post-dominate blocks in the loop and the
    // same can't be true for any other block in the CurrBlock's frontier - no
    // need to perform VPLoopInfo-based checks. Here is the case when this check
    // is needed:
    //
    //         |
    //       Header<-----------------------+
    //         |                           |
    //     SESE region                     |
    //         |                           |
    //       CurrBlock (dominates Header)  |
    //       /   \                         |
    //        ...                          |
    //       \ | /                         |
    //       Latch-------------------------+
    //         |
    //
    if (VPPostDomTree.dominates(InfluenceBB, CurrBlock))
      continue;

    auto *Cond = InfluenceBB->getCondBit();
    LLVM_DEBUG(dbgs() << "  Influencing term: {Block: "
                      << InfluenceBB->getName() << ", Cond: ";
               if (Cond) Cond->printAsOperand(dbgs()); else dbgs() << "nullptr";
               dbgs() << ", uniformity: "
                      << Block2PredicateTermsAndUniformity[InfluenceBB].second
                      << "}\n");
    Uniform &= Block2PredicateTermsAndUniformity[InfluenceBB].second;
    Uniform &= !Plan.getVPlanDA()->isDivergent(*Cond);
    assert((Cond || CurrBlock == InfluenceBB->getSuccessors()[0]) &&
           "Single predecessor on false edge?");
    // Cond == nullptr would just mean that PredBB's predicate should be used.
    // Still ok.
    PredicateTerm Term(
        InfluenceBB, Cond,
        !VPPostDomTree.dominates(CurrBlock,
                                 InfluenceBB->getSuccessors()[0]) /* Negate */);
    Block2PredicateTermsAndUniformity[CurrBlock].first.push_back(Term);
    PredicateTerm2UseBlocks[Term].push_back(CurrBlock);
  }
  Block2PredicateTermsAndUniformity[CurrBlock].second = Uniform;
}

VPValue *
VPlanPredicator::createDefiningValueForPredicateTerm(PredicateTerm Term) {
  assert(PredicateTerm2LiveInMap.find(Term) == PredicateTerm2LiveInMap.end() &&
         "Defining value has been created already!");

  assert(PredicateTerm2UseBlocks.count(Term) == 1 &&
         "No use blocks recorded for the PredicateTerm!");
  auto *Block = Term.OriginBlock;
  auto *Val = Term.Condition;

  if (Term.Negate) {
    assert(Val && "Can negate non-existing condition!");
    Val = getOrCreateNot(Val);
  }

  VPValue *Predicate = Block2Predicate[Block];
  if (!Predicate)
    return Val;

  if (!Val)
    return Predicate;

  auto *DA = Plan.getVPlanDA();
  VPBuilder::InsertPointGuard Guard(Builder);
  VPBasicBlock *PredicateInstsBB = nullptr;
  // TODO: Don't do splitting once we start preserving uniform control flow
  // and the Block is uniform.
  if (SplitBlocks.count(Block))
    PredicateInstsBB =
        cast<VPBasicBlock>(Block->getExitBasicBlock()->getSingleSuccessor());
  else {
    PredicateInstsBB = VPBlockUtils::splitExitBlock(Block, VPLI, VPDomTree);
    SplitBlocks.insert(Block);
  }
  Builder.setInsertPoint(PredicateInstsBB);
  // TODO: Once we start presrving uniform control flow, there will be no
  // need to create "and" for uniform predicate that is true on all incoming
  // edges.
  bool IsDivergent = DA->isDivergent(*Val) || DA->isDivergent(*Predicate);
  Val = Builder.createAnd(Predicate, Val,
                          "vp." + Block->getName() + ".br." + Val->getName());
  if (IsDivergent)
    DA->markDivergent(*Val);

  return Val;
}

VPValue *
VPlanPredicator::getOrCreateValueForPredicateTerm(PredicateTerm Term,
                                                  VPBlockBase *AtBlock) {
  auto It = PredicateTerm2LiveInMap.find(Term);
  if (It != PredicateTerm2LiveInMap.end()) {
    // Already calculated the IDF and inserted needed phis.
    assert(It->second.count(AtBlock) &&
           "Missing live-in information for PredicateTerm!");
    return It->second[AtBlock];
  }

  VPValue *Val = createDefiningValueForPredicateTerm(Term);
  assert(Val && "Value for PredicateTerm wasn't created!");

  // SSA Phi insertion is equivalent to performing a mem2reg transformation for
  // an alloca with two stores: false at the region entry, Val at the Block
  // (splitting of the Block for AND instructions creation is irrelevant here,
  // as it will be a single-succ/single-pred edge for Block/SplitBlock).
  VPBlockBase *Block = Term.OriginBlock;

  SmallPtrSet<VPBlockBase *, 2> DefBlocks = {Block,
                                             AtBlock->getParent()->getEntry()};
  SmallPtrSet<VPBlockBase *, 16> LiveInBlocks;
  SmallVector<VPBlockBase *, 8> IDFPHIBlocks;
  computeLiveInsForIDF(Term, LiveInBlocks);

  VPlanForwardIDFCalculator IDF(VPDomTree);
  IDF.setDefiningBlocks(DefBlocks);
  IDF.setLiveInBlocks(LiveInBlocks);
  IDF.calculate(IDFPHIBlocks);

  DenseMap<VPBlockBase *, VPValue *> &LiveValueMap =
      PredicateTerm2LiveInMap[Term];
  assert(LiveValueMap.begin() == LiveValueMap.end() &&
         "Live ins already collected?");
  LiveValueMap[Block] = Val;

  using EdgeTy = std::pair<VPBlockBase * /* Curr */, VPBlockBase * /* Pred */>;
  SmallVector<EdgeTy, 16> Worklist;
  DenseSet<EdgeTy> Visited;
  Worklist.emplace_back(Block, Block);

  while (!Worklist.empty()) {
    VPBlockBase *BB, *PredBB;
    std::tie(BB, PredBB) = Worklist.back();
    Worklist.pop_back();

    // Add successors to worklist now, so that the code can do early-continue
    // without duplicating this insertion code.
    for (auto *Succ : BB->getSuccessors()) {
      // Don't try to go outside the sub-graph contained in LiveInBlocks - we
      // don't know where to insert phis outside it (and we don't need to).
      if (LiveInBlocks.count(Succ) &&
          Visited.insert(std::make_pair(Succ, BB)).second)
        Worklist.emplace_back(Succ, BB);
    }

    auto *LiveIn = LiveValueMap[PredBB];
    if (!is_contained(IDFPHIBlocks, BB)) {
      LiveValueMap[BB] = LiveIn;
      continue;
    }
    assert(BB != Block &&
           "Why does the PredicateTerm.OriginBlock need an SSA phi?");
    VPPHINode *Phi;
    auto ExistingLiveInIt = LiveValueMap.find(BB);
    if (ExistingLiveInIt != LiveValueMap.end()) {
      // Already visited, there is an existing phi.
      Phi = cast<VPPHINode>(ExistingLiveInIt->second);
    } else {
      // First visit, create the phi, set all incoming values to false as
      // default.
      Phi = new VPPHINode(LiveIn->getType());
      Phi->setName(Val->getName() + ".phi." + BB->getName());
      BB->getEntryBasicBlock()->addRecipeAfter(Phi, nullptr /*be the first*/);
      for (auto *BBPred : BB->getPredecessors()) {
        Phi->addIncoming(
            Plan.getVPConstant(ConstantInt::getFalse(*Plan.getLLVMContext())),
            BBPred->getExitBasicBlock());
      }
      LiveValueMap[BB] = Phi;
    }
    Phi->setIncomingValue(Phi->getBlockIndex(PredBB->getExitBasicBlock()),
                          LiveIn);

    auto *DA = Plan.getVPlanDA();
    // TODO: Should it be an assert instead?
    if (DA->isDivergent(*LiveIn))
      DA->markDivergent(*Phi);
  }

  assert(LiveValueMap.count(AtBlock) == 1 && "Live for AtBlock not computed!");
  return LiveValueMap[AtBlock];
}

static void
turnPhisToBlends(VPBlockBase *Block,
                 DenseMap<const VPBlockBase *, int> &BlockIndexInRPOT) {
  for (VPPHINode &Phi : Block->getEntryBasicBlock()->getVPPhis()) {
    Phi.setBlend(true);
    if (SortBlendPhisInPredicator)
      Phi.sortIncomingBlocksForBlend(&BlockIndexInRPOT);
  }
}

bool VPlanPredicator::shouldPreserveUniformBranches() const {
  if (Plan.isFullLinearizationForced())
    return false;

  return PreserveUniformCFG;
}

bool VPlanPredicator::shouldPreserveOutgoingEdges(VPBlockBase *Block) {
  if (VPBlockUtils::blockIsLoopLatch(Block, VPLI)) {
    // Preserve the exiting edge from the loop.

    assert(Block->getNumSuccessors() == 2 &&
           "While and/or multi-exit loops aren't expected!");
    assert(Block->getSuccessors()[0]->getNumPredecessors() +
                   Block->getSuccessors()[1]->getNumPredecessors() ==
               3 &&
           "Not in loop-simplified form?");

    return true;
  }

  if (VPLI->isLoopHeader(Block->getSingleSuccessor())) {
    return true;
  }

  if (!shouldPreserveUniformBranches())
    return false;

  assert(!VPLI->isLoopHeader(Block->getSingleHierarchicalSuccessor()) &&
         "No loop region formed?");
  assert(none_of(Block->getSuccessors(),
                 [this](const VPBlockBase *Block) {
                   return VPLI->isLoopHeader(Block);
                 }) &&
         "No dedicated pre-header?");

  auto *Cond = Block->getCondBit();
  bool BlockIsUniform = Block2PredicateTermsAndUniformity[Block].second;
  return BlockIsUniform && (!Cond || !Plan.getVPlanDA()->isDivergent(*Cond));
}

void VPlanPredicator::linearizeRegion(
    const ReversePostOrderTraversal<VPBlockBase *> &RegionRPOT) {
  assert(RegionRPOT.begin() != RegionRPOT.end() &&
         "RegionRPOT can't be empty!");

  DenseMap<const VPBlockBase *, int> BlockIndexInRPOT;
  int CurrBlockRPOTIndex = 0;
  for (auto *Block : RegionRPOT)
    BlockIndexInRPOT[Block] = CurrBlockRPOTIndex++;

  // Region entry handled during outer region processing.
  auto It = ++RegionRPOT.begin();
  auto End = RegionRPOT.end();

  CurrBlockRPOTIndex = 0;
  for (VPBlockBase *CurrBlock : make_range(It, End)) {
    // We've peeled 0-th iteration, so incrementing in the beginning of the loop
    // is correct.
    ++CurrBlockRPOTIndex;

   // Process incoming edges to the CurrBlock. Once this iterations finishes,
   // CurrBlock's incoming edges are properly set (*). Also create new basic
   // blocks if CurrBlock is a point of re-convergence of several divergent
   // conditions (or even of a single one if uniform incoming edges are
   // present). Phi-nodes are marked as blended too, if needed.
   //
   // (*) Another important thing is that region CFG isn't consistent after
   // the modifications above are done. We exploit the fact that edges are
   // recorded in both successor and predecessor and perform only partial
   // updates. Successors' representation of edges between already processed
   // blocks is maintained correctly on each step. Predecessors are finalized
   // much later because we need to know original edges in the un-processed
   // graph during transformation.

    SmallVector<VPBlockBase *, 4> UniformEdges;
    SmallVector<VPBlockBase *, 4> RemainingDivergentEdges;
    SmallVector<VPBlockBase *, 4> RemovedDivergentEdges;
    for (auto *Pred : CurrBlock->getPredecessors()) {
      if (shouldPreserveOutgoingEdges(Pred)) {
        UniformEdges.push_back(Pred);
        continue;
      }
      if (is_contained(Pred->getSuccessors(), CurrBlock))
        RemainingDivergentEdges.push_back(Pred);
      else
        RemovedDivergentEdges.push_back(Pred);
    }

    if (RemainingDivergentEdges.size() + RemovedDivergentEdges.size() == 0) {
      // FIXME: CG to create a separate BB if there are PHIs here instead.
      // For now, just mark phis as blend to avoid phis in the middle of the
      // generated BB.
      if (UniformEdges.size() == 1)
        turnPhisToBlends(CurrBlock, BlockIndexInRPOT);

      // No more fixups needed, al predecessors are uniform edges that we didn't
      // touch.
      continue;
    }

    // Consider this:
    //         BB0 (U)
    //       /     \
    //     BB1 (D)  |
    //    /   \     |
    //   /     \    |
    //  BB3   BB2   |
    //   \    /    /
    //    BB4<----+
    //
    // Before linearization, PHIs in BB4 have 3 incoming blocks (BB3, BB2, BB).
    // After the transformation, there will be only two edges (BB3 and BB2 will
    // be linearized). As such, selecting between BB2 and BB3 should happen via
    // blending, and the resulting phi will only select between that blend and
    // the value coming from BB0. For that, we need to introduce a separate BB
    // to put the blend phi in, e.g.:
    //
    //       BB0 (U)
    //     /        \
    //    BB1        |
    //     |         |
    //    BB2        |
    //     |         |
    //    BB3        |
    //     |         |
    //   BlendBB     |
    //     |        /
    //    BB4<-----+
    //
    // Things get more complicated if we have several linearized sub-graphs
    // coming into this block:
    //
    //         BB0 (U)
    //         /       \
    //        /        BB5 (U)
    //       /        /   \
    //      /        /     BB6 (D)
    //     BB1 (D)  /     / \
    //    /   \    +     /   \
    //   /     \   |   BB7   BB8
    //  BB3   BB2  |   /    /
    //   \      \  |  /    /
    //    +------->BB4<---+
    //
    // After linearization we will have this:
    //
    //       BB0
    //      /  \
    //    BB1   BB5
    //     |     | \
    //    BB2    |  BB6
    //     |     |   |
    //    BB3    |  BB7
    //     |     |   |
    //     |     |  BB8
    //   Blend   |   |
    //       \   | Blend2
    //        \  | /
    //          BB4
    //
    // Note, that two different BlendBBs are needed. Basically, for they should
    // be created for each incoming divergent edge remainig after linearization.
    // However, don't be confused with
    // RemainingDivergentEdges/RemovedDivergentEdges above. For some of the
    // removed edges we will create a new edge. As such, blending BBs insertion
    // happens after all the incoming edges to CurrBB are determined.
    //
    // Iteration order matters! This is used to fill in incoming values to phis.
    // Any order is valid there, but generating random order would make unit
    // testing flaky + such varying isn'g good for the compiler as it might
    // affect later optimizations too.
    MapVector<VPBlockBase *, SmallVector<VPBlockBase *, 4>> EdgeToBlendBBs;

    for (auto *Pred : RemainingDivergentEdges) {
      // The edge is in the linearized subgraph and is processed first. Keep it,
      // but remove other successors of the pred to perform linearization.
      assert(!shouldPreserveOutgoingEdges(Pred) &&
             "Trying to remove an edge that should be preserved!");
      Pred->getSuccessors().clear();
      Pred->appendSuccessor(CurrBlock);

      // FIXME: Strange VPPHINode's incoming basic blocks interface.
      EdgeToBlendBBs[Pred].push_back(Pred->getExitBasicBlock());
    }

    if (RemainingDivergentEdges.size() == 1 &&
        UniformEdges.size() + RemovedDivergentEdges.size() == 0) {
      // E.g. for isa<VPRegionBlock>(CurrBlock). Shouldn't and even can't do any
      // further processing.
      turnPhisToBlends(CurrBlock, BlockIndexInRPOT);
      continue;
    }

    for (auto *Pred : RemovedDivergentEdges) {
      // Check if Pred is in the same linearized sub-graph that the CurrBlock
      // is. In other words, do we reach any of the remaining edges when going
      // through Pred's single successors chain?

      VPBlockBase *LastProcessed = Pred;
      VPBlockBase *PredSucc = Pred->getSingleHierarchicalSuccessor();
      // Don't go into the blocks that haven't been processed before this one
      // , including itself.
      while (PredSucc && BlockIndexInRPOT[PredSucc] < CurrBlockRPOTIndex) {
        LastProcessed = PredSucc;
        assert(VPBlockUtils::countSuccessorsNoBE(PredSucc, VPLI) <= 1 &&
               "Broken linearized chain!");
        auto *SavedPtr = PredSucc;
        PredSucc = nullptr;
        for (auto *Succ : SavedPtr->getHierarchicalSuccessors())
          if (!VPBlockUtils::isBackEdge(SavedPtr, Succ, VPLI)) {
            PredSucc = Succ;
            break;
          }
      }

      if (is_contained(LastProcessed->getSuccessors(), CurrBlock)) {
        // Nothing to do.
        //
        // Indeed, the LastProcessed-CurrBlock edge is one of the following:
        //   - Uniform edge (e.g. exiting edge of an inner loop). No successors
        //     fixup is needed.
        //   - Remaining divergent edge. Successors were fixed up in a loop
        //     processing such kind of edges.
        //   - New edge connecting this block to a linearized chain created on
        //     one of the previous iterations of this loop. Successors were
        //     fixed up during edge creation (else part of this condition).
      } else {
        // Pred was processed as part of some other linearization chain. Need to
        // merge it with the current one.
        LastProcessed->getSuccessors().clear();
        VPBlockUtils::connectBlocks(LastProcessed, CurrBlock);
      }

      // FIXME: Strange VPPHINode's incoming basic blocks interface.
      EdgeToBlendBBs[LastProcessed].push_back(Pred->getExitBasicBlock());
    }

    if (UniformEdges.size() + EdgeToBlendBBs.size() == 1) {
      turnPhisToBlends(CurrBlock, BlockIndexInRPOT);
      continue;
    }

    // All incoming edges to CurrBlock are correct now.
    assert(none_of(CurrBlock->getPredecessors(),
                   [CurrBlock, this](VPBlockBase *PredBlock) -> bool {
                     return shouldPreserveOutgoingEdges(PredBlock) &&
                            !is_contained(PredBlock->getSuccessors(),
                                          CurrBlock);
                   }) &&
           "Uniform edge has been removed!");

    // Now, create BlendBBs and blending phis inside them.

    auto VPPhisIteratorRange = cast<VPBasicBlock>(CurrBlock)->getVPPhis();
    if (VPPhisIteratorRange.begin() == VPPhisIteratorRange.end())
      // CurrBlock doesn't have any phis, no extra processing needed.
      continue;

    for (auto &It : EdgeToBlendBBs) {
      auto *IncomingBlock = It.first;
      IncomingBlock->getSuccessors().clear();
      auto BlendBB = new VPBasicBlock(VPlanUtils::createUniqueName("blend.bb"));
      BlendBB->setParent(CurrBlock->getParent());
      VPBlockUtils::connectBlocks(IncomingBlock, BlendBB);
      VPBlockUtils::connectBlocks(BlendBB, CurrBlock);
      CurrBlock->removePredecessor(IncomingBlock);
      // Re-use IncomingBlock's position for blend phi sorting purpose.
      BlockIndexInRPOT[BlendBB] = BlockIndexInRPOT[IncomingBlock];
      for (VPPHINode &Phi : VPPhisIteratorRange) {
        auto BlendPhi = new VPPHINode(Phi.getType());
        BlendPhi->setBlend(true);
        BlendBB->addRecipe(BlendPhi);
        Plan.getVPlanDA()->markDivergent(*BlendPhi);
        int NumIncoming = Phi.getNumIncomingValues();
        // Ugly loop to protect against iterator invalidation due to removal
        // of incoming values.
        for (int IdxIt = 0; IdxIt < NumIncoming; ++IdxIt) {
          int Idx = NumIncoming - 1 - IdxIt;
          VPValue *PhiIncVal = Phi.getIncomingValue(Idx);
          auto *PhiIncBB = cast<VPBasicBlock>(Phi.getIncomingBlock(Idx));
          if (!is_contained(It.second, PhiIncBB))
            continue;
          Phi.removeIncomingValue(PhiIncBB);
          BlendPhi->addIncoming(PhiIncVal, PhiIncBB);
        }
        Phi.addIncoming(BlendPhi, BlendBB);
        if (SortBlendPhisInPredicator)
          BlendPhi->sortIncomingBlocksForBlend(&BlockIndexInRPOT);
      }
    }
  }

  // Do remaining edges fixups. Don't remove cond bits yet as they're still
  // needed to generate predicates (see shouldPreserveOutgoingEdges).
  for (auto *Block : RegionRPOT) {
    SmallVector<VPBlockBase *, 4> Preds(Block->getPredecessors().begin(),
                                        Block->getPredecessors().end());
    for (auto *PredBB : Preds) {
      if (!is_contained(PredBB->getHierarchicalSuccessors(), Block)) {
        Block->removePredecessor(PredBB);
      }
    }
  }
}

void VPlanPredicator::computeLiveInsForIDF(
    PredicateTerm Term, SmallPtrSetImpl<VPBlockBase *> &LiveInBlocks) {
  auto &UseBlocks = PredicateTerm2UseBlocks[Term];
  SmallVector<VPBlockBase *, 16> Worklist(UseBlocks.begin(), UseBlocks.end());

  while (!Worklist.empty()) {
    VPBlockBase *VPBB = Worklist.pop_back_val();

    // Blocks on the path from region entry to the def block aren't interesting,
    // they don't depend on the condition in Term.OriginBlock and won't be
    // affecting the blocks that depend (these should see the def from
    // Term.OriginBlock).
    if (VPBB == Term.OriginBlock)
      continue;

    if (!LiveInBlocks.insert(VPBB).second)
      // Already processed.
      continue;

    // Add predecessors of VPBB unless it is a latch coming through back edge.
    for (VPBlockBase *Pred : VPBB->getPredecessors()) {
      if (VPBlockUtils::isBackEdge(Pred, VPBB, VPLI))
        continue;

      Worklist.push_back(Pred);
    }
  }
}

// Predicate and linearize the CFG within Region.
void VPlanPredicator::predicateAndLinearizeRegionRec(VPRegionBlock *Region,
                                                     bool SearchLoopHack) {
  ReversePostOrderTraversal<VPBlockBase *> RPOT(Region->getEntry());
  VPDomTree.recalculate(*Region);
  VPPostDomTree.recalculate(*Region);

  for (VPBlockBase *Block : RPOT)
    calculatePredicateTerms(Block);

  if (!SearchLoopHack)
    linearizeRegion(RPOT);

  auto *DA = Plan.getVPlanDA();

  // Get updated DomTree for the proper IDF calculation to insert needed phis
  // for predicates propagation.
  VPDomTree.recalculate(*Region);
  ReversePostOrderTraversal<VPBlockBase *> PostLinearizationRPOT(
      Region->getEntry());
  for (VPBlockBase *Block : PostLinearizationRPOT) {
    const auto &PredTerms = Block2PredicateTermsAndUniformity[Block].first;

    if (PredTerms.size() == 1 && PredTerms[0].Condition == nullptr) {
      // Re-use predicate of the OriginBlock.
      assert((Block->getParent() == PredTerms[0].OriginBlock ||
              VPDomTree.dominates(PredTerms[0].OriginBlock, Block)) &&
             "Broken dominance!");
      auto *Predicate = PredTerms[0].OriginBlock->getPredicate();
      Block2Predicate[Block] = Predicate;

      if (Block == Region->getEntry())
        // Block-predicate instruction for region entry was created when
        // processing the region itself.
        continue;

      if (Predicate &&
          (!shouldPreserveUniformBranches() || DA->isDivergent(*Predicate))) {
        VPBuilder::InsertPointGuard Guard(Builder);
        Builder.setInsertPointFirstNonPhi(Block->getEntryBasicBlock());
        Block->setPredicate(Predicate);
        auto *BlockPredicateInst = Builder.createPred(Predicate);
        if (DA->isDivergent(*Predicate))
          DA->markDivergent(*BlockPredicateInst);
      }

      continue;
    }

    // Either 2+ incoming edges or a single edge under condition. Create generic
    // OR sequence for all incoming predicates.
    assert(Block != Region->getEntry() && "Not a SESE region?");

    std::list<VPValue *> IncomingConditions;
    for (auto Term : PredTerms)
      if (auto *Val = getOrCreateValueForPredicateTerm(Term, Block))
        IncomingConditions.push_back(Val);

    VPBuilder::InsertPointGuard Guard(Builder);
    Builder.setInsertPointFirstNonPhi(Block->getEntryBasicBlock());

    auto *Predicate = genPredicateTree(IncomingConditions);
    Block2Predicate[Block] = Predicate;
    if (Predicate &&
        (!shouldPreserveUniformBranches() || DA->isDivergent(*Predicate))) {
      Block->setPredicate(Predicate);
      auto *BlockPredicateInst = Builder.createPred(Predicate);
      if (DA->isDivergent(*Predicate))
        DA->markDivergent(*BlockPredicateInst);
    }
  }

  // Finally, fix the cond bits.
  for (auto *Block : RPOT)
    if (Block->getNumSuccessors() == 1)
      Block->setCondBit(nullptr);

  // Recurse inside Region
  for (auto *Block : make_range(RPOT.begin(), RPOT.end()))
    if (VPRegionBlock *SubRegion = dyn_cast<VPRegionBlock>(Block))
      predicateAndLinearizeRegionRec(SubRegion, SearchLoopHack);
}

#if INTEL_CUSTOMIZATION
#include "IntelVPlanLoopCFU.h"
#endif // INTEL_CUSTOMIZATION

// Entry point. The driver function for the predicator.
void VPlanPredicator::predicate(void) {
#if INTEL_CUSTOMIZATION
  assert(VPLI->size() == 1 && "more than 1 loop?");
  VPLoop *VPL = *VPLI->begin();
  SmallVector<VPBlockBase *, 4> Exits;
  VPL->getExitBlocks(Exits);

  // Transform inner loop control to become uniform.
  if (VPlanLoopCFU) {
    LLVM_DEBUG(dbgs() << "Before inner loop control flow transformation\n");
    LLVM_DEBUG(Plan.dump());
    handleInnerLoopBackedges(VPL);
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
  Block2PredicateTermsAndUniformity[Plan.getEntry()] = {{}, true};

  predicateAndLinearizeRegionRec(cast<VPRegionBlock>(Plan.getEntry()),
                                 Exits.size() != 1 /* SearchLoopHack */);
#if INTEL_CUSTOMIZATION
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
