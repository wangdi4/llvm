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

#include "IntelVPlanPredicator.h"
#include "IntelVPlan.h"
#include "IntelVPlanIDF.h"
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

static cl::opt<bool> DotBeforeBlends(
    "vplan-dot-before-blends", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan digraph before blends are processed."));

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
VPValue *VPlanPredicator::genPredicateTree(std::list<VPValue *> &Worklist,
                                           VPBuilder &Builder) {
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

  VPBuilder Builder;
  if (auto *Inst = dyn_cast<VPInstruction>(Cond))
    if (isa<VPPHINode>(Inst))
      Builder.setInsertPointFirstNonPhi(Inst->getParent());
    else
      Builder.setInsertPoint(Inst->getParent(), ++Inst->getIterator());
  else
    Builder.setInsertPointFirstNonPhi(Plan.getEntryBlock());

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
  VPDomTreeNode *CurrBlockDT = Plan.getDT()->getNode(CurrBlock);
  assert(CurrBlockDT && "Expected node in dom tree!");
  if (auto *IDomNode = CurrBlockDT->getIDom())
    if (Plan.getPDT()->dominates(CurrBlock, IDomNode->getBlock())) {
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

  VPPostDominatorTree &VPPostDomTree = *Plan.getPDT();
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
    assert((Cond || CurrBlock == InfluenceBB->getSuccessor(0)) &&
           "Single predecessor on false edge?");
    // Cond == nullptr would just mean that PredBB's predicate should be used.
    // Still ok.
    PredicateTerm Term(
        InfluenceBB, Cond,
        !VPPostDomTree.dominates(CurrBlock,
                                 InfluenceBB->getSuccessor(0)) /* Negate */);
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

  Val = VPBuilder().setInsertPoint(Block).createAnd(
      Predicate, Val, Block->getName() + ".br." + Val->getName());
  Plan.getVPlanDA()->updateDivergence(*Val);
  if (!BlocksToSplit.count(Block))
    BlocksToSplit[Block] = cast<VPInstruction>(Val);

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

  VPlanForwardIDFCalculator IDF(*Plan.getDT());
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
      // default because that's the value coming into the block from the region
      // entry. Any edge carrying "true" value will appear as the result of this
      // worklist processing.
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
    assert(Block->getSuccessor(0)->getNumPredecessors() +
                   Block->getSuccessor(1)->getNumPredecessors() ==
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

void VPlanPredicator::linearizeRegion() {
  DenseMap<const VPBasicBlock *, int> BlockIndexInRPOT;
  int CurrBlockRPOTIndex = 0;
  for (auto *Block : RPOT)
    BlockIndexInRPOT[Block] = CurrBlockRPOTIndex++;

  // Keep track of the edges that were removed during linearization process.
  // Once we meet any divergent condition that is going to be linearized we keep
  // a single outgoing edge (to the CurrBlock, see below) and remove another
  // one. Keep track of the removed ones to properly process another successor
  // later on.
  DenseMap<VPBasicBlock * /* Dst */, SmallVector<VPBasicBlock * /* Src */, 4>>
      RemovedDivergentEdgesMap;

  // VPlan entry block is assumed to be unmasked.
  auto It = ++RPOT.begin();
  auto End = RPOT.end();

  CurrBlockRPOTIndex = 0;
  for (VPBasicBlock *CurrBlock : make_range(It, End)) {
    // We've peeled 0-th iteration, so incrementing in the beginning of the loop
    // is correct.
    ++CurrBlockRPOTIndex;

   // Process incoming edges to the CurrBlock. Once this iterations finishes,
   // CurrBlock's incoming edges are properly set. Also create new basic blocks
   // if CurrBlock is a point of re-convergence of several divergent conditions
   // (or even of a single one if uniform incoming edges are present). Blocks
   // that would need post-processing for blends creation are marked as such as
   // well.
    SmallVector<VPBasicBlock *, 4> UniformEdges;
    SmallVector<VPBasicBlock *, 4> RemainingDivergentEdges;
    SmallVectorImpl<VPBasicBlock *> &RemovedDivergentEdges =
        RemovedDivergentEdgesMap[CurrBlock];

    for (auto *Pred : CurrBlock->getPredecessors()) {
      if (shouldPreserveOutgoingEdges(Pred)) {
        UniformEdges.push_back(Pred);
        continue;
      }
      RemainingDivergentEdges.push_back(Pred);
    }

    if (RemainingDivergentEdges.size() + RemovedDivergentEdges.size() == 0) {
      // FIXME: CG to create a separate BB if there are PHIs here instead.
      // For now, just mark phis as blend to avoid phis in the middle of the
      // generated BB.
      if (UniformEdges.size() == 1 &&
          CurrBlock->getSinglePredecessor()->getSingleSuccessor())
        BlocksToBlendProcess.insert(CurrBlock);

      // No more fixups needed, al predecessors are uniform edges that we didn't
      // touch.
      continue;
    }

    auto DropDivergentEdgesFromAndLinkWith =
        [&RemovedDivergentEdgesMap](VPBasicBlock *Src,
                                    VPBasicBlock *TargetToKeep) {
          for (VPBasicBlock *Succ : Src->getSuccessors()) {
            if (Succ == TargetToKeep)
              continue;
            RemovedDivergentEdgesMap[Succ].push_back(Src);
          }
          // TODO: This code should turn to just dropping the terminator and
          // creating a new one.
          Src->clearSuccessors();
          Src->appendSuccessor(TargetToKeep);
        };

    for (auto *Pred : RemainingDivergentEdges) {
      // The edge is in the linearized subgraph and is processed first. Keep it,
      // but remove other successors of the pred to perform linearization.
      assert(!shouldPreserveOutgoingEdges(Pred) &&
             "Trying to remove an edge that should be preserved!");
      DropDivergentEdgesFromAndLinkWith(Pred, CurrBlock);
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
        //
        // Note: we are in the process of iterating over
        // RemovedDivergentEdgesMap[CurrBlock]. Since CurrBlock is passed at the
        // destination to keep no invalidation happens.
        DropDivergentEdgesFromAndLinkWith(LastProcessed, CurrBlock);
      }
    }

    // All incoming edges to CurrBlock are correct now.
    assert(none_of(CurrBlock->getPredecessors(),
                   [CurrBlock, this](VPBasicBlock *PredBlock) -> bool {
                     return shouldPreserveOutgoingEdges(PredBlock) &&
                            !is_contained(PredBlock->getSuccessors(),
                                          CurrBlock);
                   }) &&
           "Uniform edge has been removed!");

    BlocksToBlendProcess.insert(CurrBlock);
  }
}

void VPlanPredicator::transformPhisToBlends(VPBasicBlock *Block) {
  if (Block->getVPPhis().empty())
    return;

  SmallPtrSet<VPBasicBlock *, 4> DefBlocks;
  auto &SomePhi = cast<VPPHINode>(*Block->begin());
  for (auto *PredicateBlock : SomePhi.blocks())
    DefBlocks.insert(PredicateBlock);

  VPDominatorTree &VPDomTree = *Plan.getDT();
  VPPostDominatorTree &VPPostDomTree = *Plan.getPDT();

  SmallPtrSet<VPBasicBlock *, 4> LiveInBlocks;
  computeLiveInsForBlendsIDF(DefBlocks, Block, LiveInBlocks);

  SmallVector<VPBasicBlock *, 8> IDFPHIBlocks;
  VPlanForwardIDFCalculator IDF(VPDomTree);
  IDF.setDefiningBlocks(DefBlocks);
  IDF.setLiveInBlocks(LiveInBlocks);
  IDF.calculate(IDFPHIBlocks);

  // The block itself might need blend processing as well, even if the real phi
  // isn't needed in it. Consider this:
  //
  //     BB0 (D)
  //    /   \
  //  BB3   BB4
  //   \    /
  //     BB5
  //
  // after linearization it's BB0->BB4->BB3->BB5 and BB5 won't be part of IDF
  // blocks. Yet we still need to transform the phis in it into blends, even
  // though no real phis will be created on the path to it.
  if (!is_contained(IDFPHIBlocks, Block))
    IDFPHIBlocks.push_back(Block);

  LLVM_DEBUG(dbgs() << "Processing blends for " << Block->getName()
                    << ":\nDefBlocks: ";
             for (auto *B
                  : DefBlocks) dbgs()
             << B->getName() << " ";
             dbgs() << "\nLiveInBlocks: "; for (auto *B
                                                : LiveInBlocks) dbgs()
                                           << B->getName() << " ";
             dbgs() << "\nIDFPHIBlocks: "; for (auto *B
                                                : IDFPHIBlocks) dbgs()
                                           << B->getName() << " ";
             dbgs() << "\n";);

  // Due to limitation of CG we need to turn phis with single incoming
  // values into blends, even if it's not required by the blend processing
  // itself. The reason for that is CG merging multiple blocks into a single
  // one which might place a real phi into the middle of the block.
  bool ForceFinalPhiToBlendTransform = Block->getSinglePredecessor();

  // Now start creating merge phis where IDF tells us to. The code below will
  // also create blends as needed. A copy from an existing test in
  // vplan_predicator.ll to make mental tracking easier:
  //            entry
  //              |
  //             BB0 (U)-----+
  //           /              \
  //        BB1 (D)            BB2 (D)
  //        /  \              /  \
  //       /    \            /   BB3 (D)
  //       +     \          /  /    \
  //       |      BB4 (D)- / -/---->BB5
  //       |        \     /  /     |
  //       |         \   +  /      |
  //       |          \  | /       |
  //       |           \ v/        |
  //       +---------> BB6 <-------+
  //   After linearization (approximate, might be different in actual implementation)
  //   should be like this:
  //             entry
  //               |
  //              BB0
  //             /  \
  //           BB1  BB2
  //            |    |
  //           BB4  BB3
  //            |    |
  //      Blend_1_4  Blend_2_3
  //             \   /
  //              BB5 MergePhi = [ Blend_1_4, BB4 ], [ Blend_2_3, BB3]
  //               |
  //              BB6 BlendForOrigPhi (MergePhi, BB5Def)
  for (VPBasicBlock *BBForMerge : IDFPHIBlocks) {
    VPBuilder PhiBuilder;
    PhiBuilder.setInsertPoint(BBForMerge, BBForMerge->begin());

    // Make a copy to avoid any potential iterator invalidation.
    SmallVector<VPPHINode*, 4> Phis;
    for (VPPHINode &Phi : Block->getVPPhis())
      Phis.push_back(&Phi);

    DenseMap<VPPHINode *, VPPHINode *> OrigPhiToMergeMap;
    for (VPPHINode *OrigPhi : Phis) {
      VPPHINode *MergePhi =
          BBForMerge == Block
              ? OrigPhi
              : PhiBuilder.createPhiInstruction(OrigPhi->getType(),
                                                OrigPhi->getName() + ".phi." +
                                                    BBForMerge->getName());
      OrigPhiToMergeMap[OrigPhi] = MergePhi;
    }

    SmallVector<VPBasicBlock *, 8> Preds(BBForMerge->getPredecessors());
    for (VPBasicBlock *PredBB : Preds) {
      // For each incoming edge, see what incoming values of the original phi
      // have to be blended over that edge.
      SmallVector<VPBasicBlock *, 4> BBToBlend;
      llvm::copy_if(SomePhi.blocks(), std::back_inserter(BBToBlend),
                    [&](const VPBasicBlock *IncomingBlock) -> bool {
                      return VPPostDomTree.dominates(
                          PredBB, cast<VPBasicBlock>(IncomingBlock));
                    });

      assert(BBToBlend.size() > 0 && "No values for blend!");
      if (BBToBlend.size() == 1 &&
          !(BBForMerge == Block && ForceFinalPhiToBlendTransform))
        continue;

      // VPBlendInst requires its operands to be in sorted order.
      sort(BBToBlend, [&](const VPBasicBlock *LHS, const VPBasicBlock *RHS) {
        return !VPPostDomTree.dominates(LHS, RHS);
      });

      // Create the blends outside the actual CFG first. We need to do that to
      // keep CFG intact as any modifictations might invalidate the information
      // carried in the PHIs' incoming blocks.
      MapVector<VPBlendInst *, VPPHINode *> BlendsMap;
      for (VPPHINode *OrigPhi : Phis) {
        auto *Blend = new VPBlendInst(OrigPhi->getType());
        Blend->setName(OrigPhi->getName() + ".blend." + PredBB->getName());
        // TODO: This is needed only because of HIR Mixed CG limitations.
        Blend->copyUnderlyingFrom(*OrigPhi);

        for (auto *IncomingBB : BBToBlend) {
          Blend->addIncoming(OrigPhi->getIncomingValue(IncomingBB),
                             IncomingBB->getPredicate(), &Plan);
          OrigPhi->removeIncomingValue(IncomingBB);
        }

        VPPHINode *MergePhi = OrigPhiToMergeMap[OrigPhi];
        BlendsMap[Blend] = MergePhi;
      }

      // Now determine actual insertion place for the blends created earlier and
      // insert them.
      VPBuilder BlendBuilder;
      VPBasicBlock *BlendBB;
      if (BBForMerge == Block && ForceFinalPhiToBlendTransform) {
        BlendBB = Block;
        // The phis will be removed during special processing for
        // ForceFinalPhiToBlendTransform at the end of this routine.
        BlendBuilder.setInsertPointAfterBlends(Block);
      } else {
        // This is the operation we wanted to delay as the split can rewrite
        // dest block's phis.
        BlendBB = VPBlockUtils::splitEdge(
            PredBB, BBForMerge, VPlanUtils::createUniqueName("blend.bb"), VPLI,
            &VPDomTree, &VPPostDomTree);
        BlendBuilder.setInsertPoint(BlendBB);
      }
      for (auto It : BlendsMap) {
        BlendBuilder.insert(It.first);
        It.second->addIncoming(It.first, BlendBB);
      }

    }

    // We've already removed the incoming values from original phis that are
    // handled via the MergePhi's created above. Now insert these MergePhi's
    // back to the orig phis so that later merge points could find them as well.
    for (VPPHINode *OrigPhi : Phis) {
      VPPHINode *MergePhi = OrigPhiToMergeMap[OrigPhi];
      if (MergePhi == OrigPhi)
        continue;

      OrigPhi->addIncoming(MergePhi, BBForMerge);
    }
  }

  if (!ForceFinalPhiToBlendTransform)
    return;

  // LLVM IR CodeGen can't handle phis with single incoming block due to basic
  // blocks merge. We've already created corresponding blends for them, now just
  // wire those into the users and remove the phis.
  SmallVector<VPPHINode *, 4> PhisToRemove;
  for (VPPHINode &OrigPhi : Block->getVPPhis()) {
    assert(OrigPhi.getNumIncomingValues() == 1 &&
           "Forcedly processed phi are expected to have exactly 1 predecessor "
           "at this point!");
    VPValue *Op = OrigPhi.getIncomingValue(0u);
    // TODO: HIR Mixed CG has issues propagating invalidate through the
    // use-chain. Blends/phis are gonna be lowered to using the same temp, so
    // invalidation might not actually be needed.
    OrigPhi.replaceAllUsesWith(Op, false /* InvalidateIR */);

    // No explicit terminators and weird getPredicate() interface...
    for (VPBasicBlock &BB : *Block->getParent()) {
      if (BB.getCondBit() == &OrigPhi)
        BB.setCondBit(OrigPhi.getIncomingValue(0u));
    }

    PhisToRemove.push_back(&OrigPhi);
  }

  for (VPPHINode *Phi : PhisToRemove)
    Block->eraseInstruction(Phi);
}

void VPlanPredicator::transformPhisToBlends() {
  for (VPBasicBlock *Block : RPOT) {
    if (BlocksToBlendProcess.count(Block) == 0)
      continue;

    transformPhisToBlends(Block);
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

    bool BackEdgeIsFalseSucc = Latch->getSuccessor(1) == Header;
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

void VPlanPredicator::computeLiveInsForBlendsIDF(
    const SmallPtrSetImpl<VPBasicBlock *> &DefBlocks,
    const VPBasicBlock *OrigPhiBlock,
    SmallPtrSetImpl<VPBasicBlock *> &LiveInBlocks) {
  SmallVector<VPBasicBlock *, 16> Worklist;
  for (auto *DefBlock : DefBlocks)
    llvm::copy_if(DefBlock->getSuccessors(), std::back_inserter(Worklist),
                  [=](const VPBasicBlock *Succ) -> bool {
                    return !VPBlockUtils::isBackEdge(DefBlock, Succ, VPLI);
                  });
  while (!Worklist.empty()) {
    VPBasicBlock *VPBB = Worklist.pop_back_val();

    if (!LiveInBlocks.insert(VPBB).second)
      // Already processed.
      continue;

    if (VPBB == OrigPhiBlock)
      // Original phi post-dominated all defs, so we don't care about what
      // happens later in the successors chain.
      continue;

    llvm::copy_if(VPBB->getSuccessors(), std::back_inserter(Worklist),
                  [=](const VPBasicBlock *Succ) -> bool {
                    return !VPBlockUtils::isBackEdge(VPBB, Succ, VPLI);
                  });
  }
}

void VPlanPredicator::emitPredicates() {
  auto *DA = Plan.getVPlanDA();
  for (VPBasicBlock *Block : RPOT) {
    const auto &PredTerms = Block2PredicateTermsAndUniformity[Block].first;
    bool BlockIsUniform = Block2PredicateTermsAndUniformity[Block].second;
    if (BlockIsUniform && !Plan.isFullLinearizationForced())
      // Block itself is uniform, any predicate terms affecting it are lowered
      // as actual CFG - no need to perform predicate calculations.
      continue;

    if (PredTerms.size() == 1 && PredTerms[0].Condition == nullptr) {
      // Re-use predicate of the OriginBlock.
      assert((Block == PredTerms[0].OriginBlock ||
              Plan.getDT()->dominates(PredTerms[0].OriginBlock, Block)) &&
             "Broken dominance!");
      auto *Predicate = PredTerms[0].OriginBlock->getPredicate();
      Block2Predicate[Block] = Predicate;

      if (Predicate &&
          (!shouldPreserveUniformBranches() || DA->isDivergent(*Predicate))) {
        auto *BlockPredicateInst =
            VPBuilder().setInsertPointAfterBlends(Block).createPred(Predicate);
        Block->setBlockPredicate(BlockPredicateInst);
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

    VPBuilder Builder;
    Builder.setInsertPointAfterBlends(Block);

    auto *Predicate = genPredicateTree(IncomingConditions, Builder);
    Block2Predicate[Block] = Predicate;
    if (Predicate &&
        (!shouldPreserveUniformBranches() || DA->isDivergent(*Predicate))) {
      auto *BlockPredicateInst = Builder.createPred(Predicate);
      Block->setBlockPredicate(BlockPredicateInst);
      Plan.getVPlanDA()->updateDivergence(*BlockPredicateInst);
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

  // Calculate predicates for the blocks in the Plan, but don't lower them into
  // explicit VPInstructions.
  Block2PredicateTermsAndUniformity[Plan.getEntryBlock()] = {{}, true};
  for (VPBasicBlock *Block : RPOT)
    calculatePredicateTerms(Block);

  // Now use collected information to perform the linearization of the CFG.
  // Only the edges are fixed, all the phis aren't touched yet and are
  // inconsistent. We separate the stages because of several reasons:
  //   - Phi-to-blend processing needs actual values for predicates. We use IDF
  //     algorithm to introduce extra phis required to maintain SSA form and
  //     that is easier after the CFG linearization performed as a separate
  //     step.
  //   - The correct phi placement during phi-to-blend processing requires IDF
  //     algorithm as well and is another reason to separate the steps.
  // It does *NOT* update condBits as they're used later for predicates
  // creation. We probably need to fix this for the explicit VPBranchInsts.
  if (!SearchLoopHack)
    linearizeRegion();

  Plan.computeDT();
  Plan.computePDT();
  {
    // Name scope to ensure stale RPOT after std::swap below won't be misused.
    ReversePostOrderTraversal<VPBasicBlock *> PostLinearizationRPOT(
        Plan.getEntryBlock());
    std::swap(RPOT, PostLinearizationRPOT);
  }

  // Note that block splitting to make ANDs created for predicates isn't done
  // during emission.
  emitPredicates();

  // Fix the cond bits.
  for (auto *Block : RPOT)
    if (Block->getNumSuccessors() == 1)
      Block->setCondBit(nullptr);

  VPLAN_DOT(DotBeforeBlends, Plan);

  transformPhisToBlends();

  // Now do the block splitting to move ANDs out of block-predicates influence.
  for (auto It : BlocksToSplit)
    (void)VPBlockUtils::splitBlock(It.first, It.second->getIterator(), VPLI,
                                   Plan.getDT(), Plan.getPDT());

  fixupUniformInnerLoops();

  // Recompute invalidated analyses.
  Plan.computeDT();
  Plan.computePDT();

  LLVM_DEBUG(dbgs() << "VPlan after predication and linearization\n");
  LLVM_DEBUG(Plan.setName("Predicator: After predication\n"));
  LLVM_DEBUG(Plan.dump());
}

VPlanPredicator::VPlanPredicator(VPlan &Plan)
    : Plan(Plan), VPLI(Plan.getVPLoopInfo()), RPOT(Plan.getEntryBlock()) {}
