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
#include "IntelVPlanPredicator.h"
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
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "VPlanPredicator"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool> PreserveUniformCFG(
    "vplan-preserve-uniform-branches", cl::init(true), cl::Hidden,
    cl::desc("Preserve uniform branches during linearization."));

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
    Plan.getVPlanDA()->updateDivergence(*Or);

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
  VPBasicBlock *InsertBB = Inst ? Inst->getParent() : Plan.getEntryBlock();

  Builder.setInsertPoint(InsertBB, InsertBB->end());
  auto *Not = Builder.createNot(Cond, Cond->getName() + ".not");

  Plan.getVPlanDA()->updateDivergence(*Not);

  Cond2NotCond[Cond] = Not;
  return Not;
}

static void getPostDomFrontier(VPBasicBlock *Block, VPPostDominatorTree &PDT,
                               SmallPtrSetImpl<VPBasicBlock *> &Frontier) {
  assert(Frontier.empty() && "Output set isn't empty on the entry!");
  // TODO: LLORG's templated DomFrontier uses DFS numbering. Not sure if that's
  // needed to ensure some particular traversal order.
  SmallVector<VPBasicBlock *, 8> PostDominatedBlocks;
  PDT.getDescendants(Block, PostDominatedBlocks);
  // getDescendants includes the node itself into the list too, no need to
  // special case for it.
  for (VPBasicBlock *B : PostDominatedBlocks)
    for (VPBasicBlock *Pred : B->getPredecessors())
      if (!PDT.dominates(Block, Pred))
        Frontier.insert(Pred);
}

void VPlanPredicator::calculatePredicateTerms(VPBasicBlock *CurrBlock) {
  LLVM_DEBUG(dbgs() << "Calculating predicate terms for "
                    << CurrBlock->getName() << ":\n");
  // Special case for predicate re-use in case of full re-convergence, i.e.
  // diamond pattern.
  //
  //        \   /
  //         BB0 (D)
  //         / \
  //        /   \
  //       BB2  BB1
  //        \   /
  //         BB3  (Re-use BB0 predicate)
  //
  // Note, that if BB3 would have another incoming edge we won't do the same
  // re-use and will fallback to the post-dominator frontier based algorithm.
  //
  // This special case covers an important need of the region bypassing
  // algorithm that works post-linearization and needs to determine the region
  // boundaries.
  //
  // Happens to handle the single-successor single-predecessor edge as well:
  //
  //        BB0 (Predicate)
  //         |
  //        BB1 (re-use same predicate)
  //
  VPDomTreeNode *CurrBlockDT = VPDomTree.getNode(CurrBlock);
  assert(CurrBlockDT && "Expected node in dom tree!");
  if (auto *IDomNode = CurrBlockDT->getIDom())
    if (VPPostDomTree.dominates(CurrBlock, IDomNode->getBlock())) {
      auto *BB = IDomNode->getBlock();
      PredicateTerm Term(BB);
      Block2PredicateTermsAndUniformity[CurrBlock] = {
          {BB}, Block2PredicateTermsAndUniformity[BB].second};
      PredicateTerm2UseBlocks[Term].push_back(CurrBlock);

      LLVM_DEBUG(dbgs() << " Re-using idom's predicate, {Block: "
                        << BB->getName() << ", Uniformity: "
                        << Block2PredicateTermsAndUniformity[BB].second
                        << "}\n");
      return;
    }

  Block2PredicateTermsAndUniformity[CurrBlock] = {};

  SmallPtrSet<VPBasicBlock *, 12> Frontier;
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

  VPBuilder::InsertPointGuard Guard(Builder);
  VPBasicBlock *PredicateInstsBB = nullptr;
  // TODO: Don't do splitting once we start preserving uniform control flow
  // and the Block is uniform.
  if (SplitBlocks.count(Block))
    PredicateInstsBB = Block->getSingleSuccessor();
  else {
    PredicateInstsBB =
        VPBlockUtils::splitBlockEnd(Block, VPLI, &VPDomTree, nullptr);
    SplitBlocks.insert(Block);
  }
  Builder.setInsertPoint(PredicateInstsBB);
  // TODO: Once we start presrving uniform control flow, there will be no
  // need to create "and" for uniform predicate that is true on all incoming
  // edges.
  Val = Builder.createAnd(Predicate, Val,
                          VPValue::getVPNamePrefix() + Block->getName() +
                              ".br." + Val->getName());
  Plan.getVPlanDA()->updateDivergence(*Val);
  return Val;
}

VPValue *
VPlanPredicator::getOrCreateValueForPredicateTerm(PredicateTerm Term,
                                                  VPBasicBlock *AtBlock) {
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
  VPBasicBlock *Block = Term.OriginBlock;

  SmallPtrSet<VPBasicBlock *, 2> DefBlocks = {Block, Plan.getEntryBlock()};
  SmallPtrSet<VPBasicBlock *, 16> LiveInBlocks;
  SmallVector<VPBasicBlock *, 8> IDFPHIBlocks;
  computeLiveInsForIDF(Term, LiveInBlocks);

  VPlanForwardIDFCalculator IDF(VPDomTree);
  IDF.setDefiningBlocks(DefBlocks);
  IDF.setLiveInBlocks(LiveInBlocks);
  IDF.calculate(IDFPHIBlocks);

  DenseMap<VPBasicBlock *, VPValue *> &LiveValueMap =
      PredicateTerm2LiveInMap[Term];
  assert(LiveValueMap.begin() == LiveValueMap.end() &&
         "Live ins already collected?");
  LiveValueMap[Block] = Val;

  using EdgeTy = std::pair<VPBasicBlock * /* Curr */, VPBasicBlock * /* Pred */>;
  SmallVector<EdgeTy, 16> Worklist;
  DenseSet<EdgeTy> Visited;
  Worklist.emplace_back(Block, Block);

  while (!Worklist.empty()) {
    VPBasicBlock *BB, *PredBB;
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
      BB->addInstructionAfter(Phi, nullptr /*be the first*/);
      for (auto *BBPred : BB->getPredecessors()) {
        Phi->addIncoming(
            Plan.getVPConstant(ConstantInt::getFalse(*Plan.getLLVMContext())),
            BBPred);
      }
      LiveValueMap[BB] = Phi;
    }
    Phi->setIncomingValue(Phi->getBlockIndex(PredBB), LiveIn);

    // TODO: Should it be an assert instead?
    Plan.getVPlanDA()->updateDivergence(*Phi);
  }

  assert(LiveValueMap.count(AtBlock) == 1 && "Live for AtBlock not computed!");
  return LiveValueMap[AtBlock];
}

// TODO: This is a temporary hack to make sure the predicator uses the blend
// as an operand in block-predicate instructions rather than the old phi.
// Once the predicator is re-designed, this code should be removed.
void VPlanPredicator::replacePhiPredicateTermWithBlend(VPPHINode *Phi,
                                                       VPBlendInst *Blend) {
  for (auto &It : Block2PredicateTermsAndUniformity) {
    for (auto &PredTerm : It.second.first) {
      VPValue *Cond = PredTerm.Condition;
      if (Cond && Cond == Phi)
        PredTerm.Condition = Blend;
    }
  }

  for (auto &It : PredicateTerm2UseBlocks) {
    auto &PredTerm = It.first;
    VPValue *Cond = PredTerm.Condition;
    if (Cond && Cond == Phi) {
      PredicateTerm NewTerm(PredTerm.OriginBlock, Blend, PredTerm.Negate);
      for (auto *BB : It.second)
        PredicateTerm2UseBlocks[NewTerm].push_back(BB);
    }
  }
}

void VPlanPredicator::turnPhisToBlends(
    VPBasicBlock *Block,
    DenseMap<const VPBasicBlock *, int> &BlockIndexInRPOT) {

  VPlanDivergenceAnalysis *DA = Plan.getVPlanDA();
  SmallVector<VPPHINode *, 2> PhisToRemove;
  VPBasicBlock *VPBB = cast<VPBasicBlock>(Block);

  for (VPPHINode &Phi : Block->getVPPhis()) {
    // Generate a new blend instruction using the existing phi incoming values
    // and blocks. The block-predicate instructions are not yet available, and
    // they will be added to the blend at the end of predication.
    VPBlendInst *Blend = new VPBlendInst(Phi.getType());
    // Preserve instruction name for debugging and lit testing.
    Blend->setName(Phi.getName());
    Blend->copyUnderlyingFrom(Phi);

    for (unsigned i = 0; i < Phi.getNumIncomingValues(); i++) {
      VPBasicBlock *IncomingBlock = Phi.getIncomingBlock(i);
      addBlendTuple(Blend, Phi.getIncomingValue(i), IncomingBlock,
                    BlockIndexInRPOT[IncomingBlock]);
    }
    VPBB->addInstruction(Blend, &Phi);

    // Don't invalidate users of the phi because the blend is a functionally
    // equivalent instruction.
    Phi.replaceAllUsesWith(Blend, false /*Invalidate IR*/);

    // Update maps used when generating values for block-predicate instructions.
    replacePhiPredicateTermWithBlend(&Phi, Blend);

    // HCFGBuilder inserts a phi as the CondBit of the new loop latch block
    // during mergeLoopExits(). This phi can be replaced with a blend here,
    // so the CondBit must be replaced explicitly since CondBits do not show
    // up in Def/Use chains. TODO: once CondBits are replaced by proper
    // terminator instructions, this code must be removed.
    VPBasicBlock *PhiParent = Phi.getParent();
    if (PhiParent->getCondBit() == &Phi)
      PhiParent->setCondBit(Blend);

    // Remove instructions later so as to not invalidate Phi iterator.
    PhisToRemove.push_back(&Phi);

    if (DA->isDivergent(Phi))
      DA->markDivergent(*Blend);
    else
      DA->markUniform(*Blend);
  }

  for (auto *Phi : PhisToRemove)
    VPBB->eraseInstruction(Phi);
}

bool VPlanPredicator::shouldPreserveUniformBranches() const {
  if (Plan.isFullLinearizationForced())
    return false;

  return PreserveUniformCFG;
}

bool VPlanPredicator::shouldPreserveOutgoingEdges(VPBasicBlock *Block) {
  if (VPBlockUtils::blockIsLoopLatch(Block, VPLI)) {
    // Preserve the exiting edge from the loop.

    assert(Block->getNumSuccessors() == 2 &&
           "While and/or multi-exit loops aren't expected!");
    assert(Block->getSuccessors()[0]->getNumPredecessors() +
                   Block->getSuccessors()[1]->getNumPredecessors() ==
               3 &&
           "Not in loop-simplified form?");
    assert(!Plan.getVPlanDA()->isDivergent(*Block->getCondBit()) &&
           "Backedge has divergent condition!");
    // TODO: Curreently we handle "uniform" loops under divergent toptest in
    // VPlanPredicator::fixupUniformInnerLoops. The proper solution is going to
    // be an all-zero-based SESE region bypass. In case if it will be running
    // before the predicator, the following assert would be needed.
    //
    // assert(
    //     (LoopItselfIsUnmasked || isAllZeroCheckBased(BackEdgeCondtion)) &&
    //     "Even uniform loops under a divergent top test need special care!");

    return true;
  }

  if (VPLI->isLoopHeader(Block->getSingleSuccessor())) {
    return true;
  }

  if (!shouldPreserveUniformBranches())
    return false;

  assert(!VPLI->isLoopHeader(Block->getSingleSuccessor()) &&
         "No loop region formed?");
  assert(none_of(Block->getSuccessors(),
                 [this](const VPBasicBlock *Block) {
                   return VPLI->isLoopHeader(Block);
                 }) &&
         "No dedicated pre-header?");

  auto *Cond = Block->getCondBit();
  bool BlockIsUniform = Block2PredicateTermsAndUniformity[Block].second;
  return BlockIsUniform && (!Cond || !Plan.getVPlanDA()->isDivergent(*Cond));
}

void VPlanPredicator::linearizeRegion(
    const ReversePostOrderTraversal<VPBasicBlock *> &RegionRPOT) {
  assert(RegionRPOT.begin() != RegionRPOT.end() &&
         "RegionRPOT can't be empty!");

  DenseMap<const VPBasicBlock *, int> BlockIndexInRPOT;
  int CurrBlockRPOTIndex = 0;
  for (auto *Block : RegionRPOT)
    BlockIndexInRPOT[Block] = CurrBlockRPOTIndex++;

  // Region entry handled during outer region processing.
  auto It = ++RegionRPOT.begin();
  auto End = RegionRPOT.end();

  CurrBlockRPOTIndex = 0;
  for (VPBasicBlock *CurrBlock : make_range(It, End)) {
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

    SmallVector<VPBasicBlock *, 4> UniformEdges;
    SmallVector<VPBasicBlock *, 4> RemainingDivergentEdges;
    SmallVector<VPBasicBlock *, 4> RemovedDivergentEdges;
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
    MapVector<VPBasicBlock *, SmallVector<VPBasicBlock *, 4>> EdgeToBlendBBs;

    for (auto *Pred : RemainingDivergentEdges) {
      // The edge is in the linearized subgraph and is processed first. Keep it,
      // but remove other successors of the pred to perform linearization.
      assert(!shouldPreserveOutgoingEdges(Pred) &&
             "Trying to remove an edge that should be preserved!");
      Pred->getSuccessors().clear();
      Pred->appendSuccessor(CurrBlock);

      EdgeToBlendBBs[Pred].push_back(Pred);
    }

    if (RemainingDivergentEdges.size() == 1 &&
        UniformEdges.size() + RemovedDivergentEdges.size() == 0) {
      turnPhisToBlends(CurrBlock, BlockIndexInRPOT);
      continue;
    }

    for (auto *Pred : RemovedDivergentEdges) {
      // Check if Pred is in the same linearized sub-graph that the CurrBlock
      // is. In other words, do we reach any of the remaining edges when going
      // through Pred's single successors chain?

      VPBasicBlock *LastProcessed = Pred;
      VPBasicBlock *PredSucc = Pred->getSingleSuccessor();
      // Don't go into the blocks that haven't been processed before this one
      // , including itself.
      while (PredSucc && BlockIndexInRPOT[PredSucc] < CurrBlockRPOTIndex) {
        LastProcessed = PredSucc;
        auto EdgeFormsLinearizedChain =
            [this, &BlockIndexInRPOT, CurrBlockRPOTIndex](
                const VPBasicBlock *From, const VPBasicBlock *To) {
              return !VPBlockUtils::isBackEdge(From, To, VPLI) &&
                     BlockIndexInRPOT[To] < CurrBlockRPOTIndex;
            };
        assert(count_if(PredSucc->getSuccessors(),
                        [EdgeFormsLinearizedChain,
                         PredSucc](const VPBasicBlock *Succ) {
                          return EdgeFormsLinearizedChain(PredSucc, Succ);
                        }) <= 1 &&
               "Broken linearized chain!");
        auto *SavedPtr = PredSucc;
        PredSucc = nullptr;
        for (auto *Succ : SavedPtr->getSuccessors())
          if (EdgeFormsLinearizedChain(SavedPtr, Succ)) {
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

      EdgeToBlendBBs[LastProcessed].push_back(Pred);
    }

    if (UniformEdges.size() + EdgeToBlendBBs.size() == 1) {
      turnPhisToBlends(CurrBlock, BlockIndexInRPOT);
      continue;
    }

    // All incoming edges to CurrBlock are correct now.
    assert(none_of(CurrBlock->getPredecessors(),
                   [CurrBlock, this](VPBasicBlock *PredBlock) -> bool {
                     return shouldPreserveOutgoingEdges(PredBlock) &&
                            !is_contained(PredBlock->getSuccessors(),
                                          CurrBlock);
                   }) &&
           "Uniform edge has been removed!");

    // Now, create BlendBBs and blending phis inside them.

    auto VPPhisIteratorRange = CurrBlock->getVPPhis();
    if (VPPhisIteratorRange.begin() == VPPhisIteratorRange.end())
      // CurrBlock doesn't have any phis, no extra processing needed.
      continue;

    for (auto &It : EdgeToBlendBBs) {
      auto *IncomingBlock = It.first;
      IncomingBlock->getSuccessors().clear();
      auto BlendBB = new VPBasicBlock(VPlanUtils::createUniqueName("blend.bb"));
      auto *VLoop = VPLI->getLoopFor(CurrBlock);
      assert(VLoop && "VLoop is expected");
      VLoop->addBasicBlockToLoop(BlendBB, *VPLI);
      BlendBB->moveBefore(CurrBlock);
      VPBlockUtils::connectBlocks(IncomingBlock, BlendBB);
      VPBlockUtils::connectBlocks(BlendBB, CurrBlock);
      CurrBlock->removePredecessor(IncomingBlock);
      // Re-use IncomingBlock's position for blend phi sorting purpose.
      BlockIndexInRPOT[BlendBB] = BlockIndexInRPOT[IncomingBlock];
      for (VPPHINode &Phi : VPPhisIteratorRange) {
        auto Blend = new VPBlendInst(Phi.getType());
        BlendBB->addInstruction(Blend);
        Plan.getVPlanDA()->markDivergent(*Blend);
        int NumIncoming = Phi.getNumIncomingValues();
        // Ugly loop to protect against iterator invalidation due to removal
        // of incoming values.
        for (int IdxIt = 0; IdxIt < NumIncoming; ++IdxIt) {
          int Idx = NumIncoming - 1 - IdxIt;
          VPValue *PhiIncVal = Phi.getIncomingValue(Idx);
          auto *PhiIncBB = Phi.getIncomingBlock(Idx);
          if (!is_contained(It.second, PhiIncBB))
            continue;
          Phi.removeIncomingValue(PhiIncBB);
          addBlendTuple(Blend, PhiIncVal, PhiIncBB, BlockIndexInRPOT[PhiIncBB]);
        }
        Phi.addIncoming(Blend, BlendBB);
      }
    }
  }

  // Do remaining edges fixups. Don't remove cond bits yet as they're still
  // needed to generate predicates (see shouldPreserveOutgoingEdges).
  for (auto *Block : RegionRPOT) {
    SmallVector<VPBasicBlock *, 4> Preds(Block->getPredecessors().begin(),
                                         Block->getPredecessors().end());
    for (auto *PredBB : Preds) {
      if (!is_contained(PredBB->getSuccessors(), Block)) {
        Block->removePredecessor(PredBB);
      }
    }
  }
}

void VPlanPredicator::fixupUniformInnerLoops() {
  for (auto *Loop : VPLI->getLoopsInPreorder()) {
    VPBasicBlock *Header = Loop->getHeader();
    auto *Predicate = Header->getPredicate();
    if (!Predicate)
      // Not on a divergent path.
      continue;

    VPBasicBlock *Latch = Loop->getLoopLatch();
    auto *CondBit = Latch->getCondBit();
    auto *CondBitInst = dyn_cast<VPInstruction>(CondBit);
    if (CondBitInst && CondBitInst->getOpcode() == VPInstruction::AllZeroCheck)
      // Already processed by LoopCFU.
      continue;

    if (CondBitInst && CondBitInst->getOpcode() == VPInstruction::Not) {
      // Already processed by LoopCFU.
      continue;
    }

    LLVM_DEBUG(dbgs() << "Fixing up uniform loop with header "
                      << Header->getName() << "\n");

    bool BackEdgeIsFalseSucc = Latch->getSuccessors()[1] == Header;
    VPBuilder Builder;
    Builder.setInsertPoint(Latch);
    auto *NewAllZeroCheck = Builder.createAllZeroCheck(Predicate);
    Plan.getVPlanDA()->updateDivergence(*NewAllZeroCheck);
    VPValue *NewCondBit;
    if (!BackEdgeIsFalseSucc) {
      NewAllZeroCheck = Builder.createNot(NewAllZeroCheck);
      Plan.getVPlanDA()->updateDivergence(*NewAllZeroCheck);
      NewCondBit = Builder.createAnd(NewAllZeroCheck, CondBit);
    } else
      NewCondBit = Builder.createOr(NewAllZeroCheck, CondBit);

    Plan.getVPlanDA()->updateDivergence(*NewCondBit);
    Latch->setCondBit(NewCondBit);
  }
}

void VPlanPredicator::computeLiveInsForIDF(
    PredicateTerm Term, SmallPtrSetImpl<VPBasicBlock *> &LiveInBlocks) {
  auto &UseBlocks = PredicateTerm2UseBlocks[Term];
  SmallVector<VPBasicBlock *, 16> Worklist(UseBlocks.begin(), UseBlocks.end());

  while (!Worklist.empty()) {
    VPBasicBlock *VPBB = Worklist.pop_back_val();

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
    for (VPBasicBlock *Pred : VPBB->getPredecessors()) {
      if (VPBlockUtils::isBackEdge(Pred, VPBB, VPLI))
        continue;

      Worklist.push_back(Pred);
    }
  }
}

/// Sort the incoming blocks of the blend according to their execution order
/// in the linearized CFG. Required to be performed prior to code generation
/// for the blends. The sorting is done on the blend -> { value, block } map
/// kept in the predicator because at the time of blend creation, the block-
/// predicates for the blend are not yet generated.
///
/// \p BlockIndexInRPOTOrNull is a parameter with the mapping of the blocks in
/// \p this blend's parent region to that blocks' RPOT numbers. If
/// not provided, it will be calculated inside the method.
//
// TODO: As an optimization, the sorting can be done once per block, but that
// should be done at the caller side complicating the code.
//
// After linearization, the blends are completed based on the sorted mapping
// and the HCFG coming to codegen might be something like this:
//
//   bb0:
//     %def0 =
//   bb1:
//     predicate %cond0
//     %def1 =
//   bb2:
//     predicate %cond1    ; %cond1 = %cond0 && %something
//     %def2 =
//   bb3:
//     %blend_phi = phi [ %def1, %bb1 ], [ %def0, %bb0 ], [ %def 2, %bb2 ]
//
// We need to generate
//
//  %sel = select %cond0, %def1, %def0
//  %blend = select %cond1 %def2, %sel
//
// Note, that the order of processing needs to be [ %def0, %def1, %def2 ]
// for such CFG.
//
// FIXME: Once we get rid of hierarchical CFG, we would be able to use
// dominance as the comparator.
void VPlanPredicator::sortIncomingBlocksForBlend(
    BlendTupleVectorTy &UnsortedIncomingBlocks,
    BlendTupleVectorTy &SortedIncomingBlocks) {

  for (unsigned Idx = 0; Idx < UnsortedIncomingBlocks.size(); ++Idx) {
    VPValue *IncomingVal = UnsortedIncomingBlocks[Idx].getIncomingValue();
    VPBasicBlock *IncomingBlock =
        UnsortedIncomingBlocks[Idx].getIncomingBlock();
    int RPOTIdx = UnsortedIncomingBlocks[Idx].getIncomingBlockRPOTIdx();
    BlendTuple Curr(IncomingVal, IncomingBlock, RPOTIdx);

    SortedIncomingBlocks.insert(
        upper_bound(SortedIncomingBlocks, Curr,
                    [&](const BlendTuple &Lhs, const BlendTuple &Rhs) {
                      return (Lhs.getIncomingBlockRPOTIdx() <
                             Rhs.getIncomingBlockRPOTIdx());
                    }),
        Curr);
  }
}

// Predicate and linearize the CFG within Region.
void VPlanPredicator::predicateAndLinearizeRegionRec(bool SearchLoopHack) {
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(Plan.getEntryBlock());
  VPDomTree.recalculate(Plan);
  VPPostDomTree.recalculate(Plan);

  for (VPBasicBlock *Block : RPOT)
    calculatePredicateTerms(Block);

  if (!SearchLoopHack)
    linearizeRegion(RPOT);

  // Get updated DomTree for the proper IDF calculation to insert needed phis
  // for predicates propagation.
  VPDomTree.recalculate(Plan);
  ReversePostOrderTraversal<VPBasicBlock *> PostLinearizationRPOT(
      Plan.getEntryBlock());

  auto *DA = Plan.getVPlanDA();
  for (VPBasicBlock *Block : PostLinearizationRPOT) {
    const auto &PredTerms = Block2PredicateTermsAndUniformity[Block].first;

    if (PredTerms.size() == 1 && PredTerms[0].Condition == nullptr) {
      // Re-use predicate of the OriginBlock.
      assert((Block == PredTerms[0].OriginBlock ||
              VPDomTree.dominates(PredTerms[0].OriginBlock, Block)) &&
             "Broken dominance!");
      auto *Predicate = PredTerms[0].OriginBlock->getPredicate();
      Block2Predicate[Block] = Predicate;

      if (Predicate &&
          (!shouldPreserveUniformBranches() || DA->isDivergent(*Predicate))) {
        VPBuilder::InsertPointGuard Guard(Builder);
        Builder.setInsertPointAfterBlends(Block);
        Block->setPredicate(Predicate);
        auto *BlockPredicateInst = Builder.createPred(Predicate);
        Plan.getVPlanDA()->updateDivergence(*BlockPredicateInst);
      }

      continue;
    }

    // Either 2+ incoming edges or a single edge under condition. Create generic
    // OR sequence for all incoming predicates.
    std::list<VPValue *> IncomingConditions;
    for (auto Term : PredTerms)
      if (auto *Val = getOrCreateValueForPredicateTerm(Term, Block))
        IncomingConditions.push_back(Val);

    VPBuilder::InsertPointGuard Guard(Builder);
    Builder.setInsertPointAfterBlends(Block);

    auto *Predicate = genPredicateTree(IncomingConditions);
    Block2Predicate[Block] = Predicate;
    if (Predicate &&
        (!shouldPreserveUniformBranches() || DA->isDivergent(*Predicate))) {
      Block->setPredicate(Predicate);
      auto *BlockPredicateInst = Builder.createPred(Predicate);
      Plan.getVPlanDA()->updateDivergence(*BlockPredicateInst);
    }
  }

  // Fix the cond bits.
  for (auto *Block : RPOT)
    if (Block->getNumSuccessors() == 1)
      Block->setCondBit(nullptr);

  // At the time of blend instruction creation, the block-predicate instructions
  // are not yet available. This code completes the blend instructions by adding
  // operands for both the incoming value and block-predicate.
  for (auto Blend2BlendTupleMap : Blend2BlendTupleVectorMap) {
    VPBlendInst *Blend = Blend2BlendTupleMap.first;
    BlendTupleVectorTy BlendTupleVector = Blend2BlendTupleMap.second;
    BlendTupleVectorTy SortedBlendTupleVector;
    sortIncomingBlocksForBlend(BlendTupleVector, SortedBlendTupleVector);
    for (auto BlendTuple : SortedBlendTupleVector) {
      VPValue *BlockPred = BlendTuple.getIncomingBlock()->getPredicate();
      // BlockPred can be nullptr for the first block in the sorted list of
      // incoming blocks for the blend. Since we cannot add a nullptr as an
      // operand, just set to true. Codegen will skip this anyway and generate
      // the first select using the block-predicate of the next incoming block.
      if (!BlockPred) {
        Type *Ty1 = Type::getInt1Ty(*Plan.getLLVMContext());
        BlockPred = Plan.getVPConstant(ConstantInt::get(Ty1, 1));
      }
      Blend->addIncoming(BlendTuple.getIncomingValue(), BlockPred);
    }
  }
}

// Entry point. The driver function for the predicator.
void VPlanPredicator::predicate(void) {
  bool SearchLoopHack = false;
  if (VPLI->size() != 0) {
    assert(VPLI->size() == 1 && "more than 1 loop?");
    VPLoop *VPL = *VPLI->begin();
    SmallVector<VPBasicBlock *, 4> Exits;
    VPL->getExitBlocks(Exits);

    if (Exits.size() != 1 )
      SearchLoopHack = true;
  }

  // Predicate the blocks within Region.
  Block2PredicateTermsAndUniformity[Plan.getEntryBlock()] = {{}, true};

  predicateAndLinearizeRegionRec(SearchLoopHack);
  fixupUniformInnerLoops();
#if INTEL_CUSTOMIZATION
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
}
#if INTEL_CUSTOMIZATION
#undef VPlanPredicator
#endif // INTEL_CUSTOMIZATION
