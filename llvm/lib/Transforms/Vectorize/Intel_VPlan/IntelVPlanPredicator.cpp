//===-- VPlanPredicator.cpp -------------------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
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

static LoopVPlanDumpControl
    LinearizationDumpControl("linearization", "linearization");
static LoopVPlanDumpControl
    PredicatesEmissionDumpControl("predicates-emission", "predicates emission");

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
    if (isa<VPPHINode>(Inst)) {
      auto BB = Inst->getParent();
      if (auto *BlockPredicate = BB->getBlockPredicate())
        // Cond might be a phi/blend that needs active lane extraction. Such
        // extraction uses block's predicate and must happen before its
        // VPBlockPredicate instruction. We insert VPActiveLane right after the
        // predicate calculation (if it happens in that block) and this NOT
        // right before the VPBlockPredicate.
        Builder.setInsertPoint(BlockPredicate);
      else
        Builder.setInsertPointFirstNonPhi(BB);
    }
    else
      Builder.setInsertPoint(Inst->getParent(), ++Inst->getIterator());
  else
    Builder.setInsertPointFirstNonPhi(&Plan.getEntryBlock());

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

static void updateInsertPointForVPActiveLane(VPBuilder &Builder, VPBasicBlock *BB) {
  auto PredicateInst = dyn_cast_or_null<VPInstruction>(BB->getPredicate());
  // Note this special scenario:
  //   bb0
  //     %pred = or %term1, %term2  ; <- Out Predicated
  //     block-predicate %pred
  //     br i1 %cond, %bb1, %bb2
  //
  //   bb1:
  //     br %bb3
  //
  //   bb2:
  //     br %bb3
  //
  //   bb3:
  //     %blend
  //     ; <- We set InsertPoint to here.
  //     %not = not %blend
  //     %block-predicate %pred
  if (PredicateInst && PredicateInst->getParent() == BB)
    Builder.setInsertPoint(PredicateInst->getNextNode());
  else
    Builder.setInsertPointAfterBlends(BB);
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
    assert(Cond && "Unconditional branch can't result in dom frontier!");
    bool InfluenceBBUniformity =
        Block2PredicateTermsAndUniformity[InfluenceBB].second;
    bool CondUniformity = !Plan.getVPlanDA()->isDivergent(*Cond);
    bool Negate = !VPPostDomTree.dominates(CurrBlock,
                                           InfluenceBB->getSuccessor(0));
    LLVM_DEBUG(dbgs() << "  Influencing term: {Block: "
                      << InfluenceBB->getName() << ", Cond: ";
               Cond->printAsOperand(dbgs());
               dbgs() << ", uniformity (BB, Cond): (" << InfluenceBBUniformity
                      << ", " << CondUniformity << "), negate: " << Negate
                      << "}\n");
    Uniform &= InfluenceBBUniformity;
    Uniform &= CondUniformity;
    PredicateTerm Term(InfluenceBB, Cond, Negate);
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

  LLVM_DEBUG(dbgs() << "Propagating PredicateTerm "
                    << Term.OriginBlock->getName() << " "
                    << Term.Condition->getName() << " Negate: " << Term.Negate
                    << "\n");

  VPValue *Val = createDefiningValueForPredicateTerm(Term);
  assert(Val && "Value for PredicateTerm wasn't created!");

  // SSA Phi insertion is equivalent to performing a mem2reg transformation for
  // an alloca with two stores: false at the region entry, Val at the Block
  // (splitting of the Block for AND instructions creation is irrelevant here,
  // as it will be a single-succ/single-pred edge for Block/SplitBlock).
  VPBasicBlock *Block = Term.OriginBlock;

  SmallPtrSet<VPBasicBlock *, 2> DefBlocks = {Block, &Plan.getEntryBlock()};
  SmallPtrSet<VPBasicBlock *, 16> LiveInBlocks;
  SmallVector<VPBasicBlock *, 8> IDFPHIBlocks;
  computeLiveInsForIDF(Term, LiveInBlocks);

  LLVM_DEBUG({
    dbgs() << "LiveInBlocks:\n";
    for (auto *BB : LiveInBlocks)
      dbgs() << BB->getName() << " ";
  });

  VPlanForwardIDFCalculator IDF(*Plan.getDT());
  IDF.setDefiningBlocks(DefBlocks);
  IDF.setLiveInBlocks(LiveInBlocks);
  IDF.calculate(IDFPHIBlocks);

  LLVM_DEBUG(dbgs() << "\nIDFPHIBlocks:\n";
             for (auto *BB : IDFPHIBlocks) { dbgs() << BB->getName() << " "; } dbgs() << "\n");

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
  if (Cond && Plan.getVPlanDA()->isDivergent(*Cond))
    return false;

  bool BlockIsUniform = Block2PredicateTermsAndUniformity[Block].second;
  if (BlockIsUniform)
    return true;

  if (!Cond)
    return false;

  // Try to preserve uniform condition under a top-level mask.
  if (!isa<VPExternalDef>(Cond))
    // Non-trivial in case of all-false outer mask.
    return false;

  VPBasicBlock *PostDom = Plan.getPDT()->getNode(Block)->getIDom()->getBlock();
  for (auto It = std::next(df_begin(Block)), End = df_end(Block);
       It != End;) {
    // No outer edge going into the region.
    if (!Plan.getDT()->dominates(Block, *It))
      return false;

    if (*It == PostDom) {
      It.skipChildren();
      continue;
    }

    // No intermix of subregions between each other.
    if (llvm::none_of(Block->getSuccessors(),
                      [&It, DT = Plan.getDT()](const VPBasicBlock *Succ) {
                        return DT->dominates(Succ, *It);
                      }))
      return false;

    ++It;
  }

  return true;
}

namespace {
// The ordering algorithm was taken from the region vectorizer at
// https://github.com/cdl-saarland/rv/blob/master/src/transform/Linearizer.cpp
//
// See also https://compilers.cs.uni-saarland.de/papers/moll_parlin_pldi18.pdf,
// Figure 3 in particular.
class LinearizationBlockOrdering {
  VPDominatorTree *DT;
  SmallVector<VPBasicBlock *, 32> Blocks;
  DenseMap<const VPBasicBlock *, int> Indices;

  using rpot_iterator =
      ReversePostOrderTraversal<VPBasicBlock *>::const_rpo_iterator;

  void processBlockRec(VPBasicBlock *DomBB, rpot_iterator RPOTIt,
                       rpot_iterator RPOTEnd) {
    int Id = Blocks.size();
    Blocks.push_back(DomBB);
    assert(Indices.count(DomBB) == 0 && "Block must not be processed yet!");
    Indices[DomBB] = Id;

    auto *DomBBNode = DT->getNode(DomBB);
    for (auto It = RPOTIt; It != RPOTEnd; ++It) {
      auto *BB = *It;
      auto *BBNode = DT->getNode(BB);
      assert(BBNode && "Expected non-null BBNode for a basic block.");
      auto *BBIDom = BBNode->getIDom();
      if (BBIDom != DomBBNode)
        continue;

      // No special loop processing because of our loop exits canonicalization.
      processBlockRec(BB, It, RPOTEnd);
    }
  }

public:
  LinearizationBlockOrdering(
      VPlanVector &Plan, const ReversePostOrderTraversal<VPBasicBlock *> &RPOT)
      : DT(Plan.getDT()) {
    Blocks.reserve(Plan.size());
    processBlockRec(*RPOT.begin(), RPOT.begin(), RPOT.end());
    assert(Blocks.size() == Plan.size() && "Unprocessed block(s)!");
  }

  using iterator = decltype(Blocks.begin());
  iterator begin() { return Blocks.begin(); }
  iterator end() { return Blocks.end(); }

  int getIndex(const VPBasicBlock *BB) { return Indices[BB]; }
};
} // namespace

void VPlanPredicator::linearizeRegion() {
  LinearizationBlockOrdering BlockOrdering(Plan, RPOT);

  // Init list of edges we're going to preserve. The check might/will be CFG
  // based and we don't want to run it in the middle of the processing.
  DenseMap<VPBasicBlock * /* Dst */, SmallPtrSet<VPBasicBlock * /* Src */, 4>>
      EdgesToPreserve;

  for (VPBasicBlock *Block : BlockOrdering)
    if (shouldPreserveOutgoingEdges(Block))
      for (VPBasicBlock *Succ : Block->getSuccessors())
        EdgesToPreserve[Succ].insert(Block);

  // Keep track of the edges that were removed during linearization process.
  // Once we meet any divergent condition that is going to be linearized we keep
  // a single outgoing edge (to the CurrBlock, see below) and remove another
  // one. Keep track of the removed ones to properly process another successor
  // later on.
  //
  // Use std::map to avoid invalidation of references to the values stored in
  // the map when insertions happen.
  std::map<VPBasicBlock * /* Dst */, SmallVector<VPBasicBlock * /* Src */, 4>>
      RemovedDivergentEdgesMap;

  int CurrBlockIndex = 0;
  // VPlan entry block is assumed to be unmasked.
  for (VPBasicBlock *CurrBlock : drop_begin(BlockOrdering)) {
    // We've peeled 0-th iteration, so incrementing in the beginning of the loop
    // is correct.
    ++CurrBlockIndex;

    LLVM_DEBUG(dbgs() << "Processing incoming edges to " << CurrBlock->getName()
                      << "\n");

    // Process incoming edges to the CurrBlock. Once this iterations finishes,
    // CurrBlock's incoming edges are properly set. Also create new basic blocks
    // if CurrBlock is a point of re-convergence of several divergent conditions
    // (or even of a single one if uniform incoming edges are present). Blocks
    // that would need post-processing for blends creation are marked as such as
    // well.
    SmallPtrSetImpl<VPBasicBlock *> &PredsToPreserve =
        EdgesToPreserve[CurrBlock];
    SmallVector<VPBasicBlock *, 4> RemainingDivergentEdges;
    SmallVectorImpl<VPBasicBlock *> &RemovedDivergentEdges =
        RemovedDivergentEdgesMap[CurrBlock];

    for (auto *Pred : CurrBlock->getPredecessors()) {
      if (PredsToPreserve.count(Pred))
        continue;

      RemainingDivergentEdges.push_back(Pred);
    }

    if (RemainingDivergentEdges.size() + RemovedDivergentEdges.size() == 0) {
      // FIXME: CG to create a separate BB if there are PHIs here instead.
      // For now, just mark phis as blend to avoid phis in the middle of the
      // generated BB.
      if (PredsToPreserve.size() == 1 &&
          CurrBlock->getSinglePredecessor()->getSingleSuccessor()) {
        for (auto &Phi : CurrBlock->getVPPhis()) {
          PhisToBlendProcess[CurrBlock].push_back(&Phi);
        }
      }

      // No more fixups needed, al predecessors are uniform edges that we didn't
      // touch.
      LLVM_DEBUG(dbgs() << "No fixup needed\n");
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
          Src->setTerminator(TargetToKeep);
        };

    LLVM_DEBUG(dbgs() << "Remaining divergent edges:");
    for (auto *Pred : RemainingDivergentEdges) {
      LLVM_DEBUG(dbgs() << " " << Pred->getName());
      // The edge is in the linearized subgraph and is processed first. Keep it,
      // but remove other successors of the pred to perform linearization.
      DropDivergentEdgesFromAndLinkWith(Pred, CurrBlock);
    }

    LLVM_DEBUG(dbgs() << "\nAlready dropped divergent edges:");
    for (auto *Pred : RemovedDivergentEdges) {
      LLVM_DEBUG(dbgs() << " " << Pred->getName());
      // Ensure that all preds where edges were dropped can reach this block on
      // all pathes from them. Add new edges if necessary.
      auto It = df_begin(Pred);
      auto End = df_end(Pred);
      while (It != End) {
        if (any_of(It->getSuccessors(),
                   [&BlockOrdering, CurrBlockIndex](const VPBasicBlock *Succ) {
                     return BlockOrdering.getIndex(Succ) < CurrBlockIndex;
                   })) {
          // Not a leaf in the subgraph processed so far.
          ++It;
          continue;
        }

        if (BlockOrdering.getIndex(*It) >= CurrBlockIndex) {
          // Block is outside the subgraph we're currently working on.
          It.skipChildren();
          continue;
        }

        DropDivergentEdgesFromAndLinkWith(*It, CurrBlock);
        It.skipChildren();
      }
    }

    LLVM_DEBUG(dbgs() << "\n");

    // All incoming edges to CurrBlock are correct now.
    for (auto &Phi : CurrBlock->getVPPhis()) {
      PhisToBlendProcess[CurrBlock].push_back(&Phi);
    }
  }
}

namespace {
class PhiToBlendUpdater {
  VPDominatorTree &VPDomTree;
  VPPostDominatorTree &VPPostDomTree;
  VPLoopInfo *VPLI;

  /// Block where the phis require blend processing as identified by the CFG
  /// linearization pass.
  VPBasicBlock *Block;

  /// Original Phis that need to be transformed into blends/ssa-phis. Cleared
  /// early in the transformation process because CFG updates would invalidate
  /// the information carried in them.
  SmallVector<VPPHINode *, 4> Phis;

  /// Incoming blocks of the original Phis before transformation starts.
  SmallPtrSet<VPBasicBlock *, 4> DefBlocks;
  /// Blocks where real VPPHINodes need to be introduced to maintain SSA form.
  SmallVector<VPBasicBlock *, 8> IDFPHIBlocks;

  /// Move incoming values/blocks information into the map. That is needed for
  /// several reasons:
  ///   1) Blends have to be created by splitting edges. Such splitting might
  ///      invalidated the phis.
  ///   2) We will re-use the phi in case IDF identified it as merge point.
  ///      Easier when the phi is "empty".
  ///   3) If not re-used, it will be deleted anyway.
  SmallVector<DenseMap<VPBasicBlock * /* OrigIncomingBB */,
                       VPValue * /* OrigIncomingVal */>,
              4>
      OrigValsMaps;

  /// Store information about created merge PHIs in blocks identified by the
  /// IDF.
  SmallVector<
      DenseMap<VPBasicBlock * /* MergeBB */, VPPHINode * /* MergePhi */>, 4>
      MergePhiMaps;

public:
  PhiToBlendUpdater(VPBasicBlock *Block, ArrayRef<VPPHINode *> Phis)
      : VPDomTree(*(*cast<VPlanVector>(Block->getParent())).getDT()),
        VPPostDomTree(*(*cast<VPlanVector>(Block->getParent())).getPDT()),
        VPLI(cast<VPlanVector>(Block->getParent())->getVPLoopInfo()),
        Block(Block), Phis(Phis.begin(), Phis.end()) {}

  void processSingleIncomingValuePhis() {
    assert(Phis[0]->getNumIncomingValues() == 1 &&
           "Only the simple case is processed here!");
    // If the following assert would fail, we'd be transforming LCSSA phi into a
    // blend which doesn't look desirable right now. Still, the linearization
    // logic is such that we never put such blocks into the list of blocks that
    // require blend processing.
    assert(
        !VPBlockUtils::blockIsLoopLatch(Block->getSinglePredecessor(), VPLI) &&
        "Loop exits in loop-simplified forms don't require blend processing!");
    for (VPPHINode *Phi : Phis) {
      Phi->replaceAllUsesWith(Phi->getOperand(0), false /* InvalidateIR */);
      Phi->getParent()->eraseInstruction(Phi);
    }
  }

  void
  computeLiveInsForBlendsIDF(const SmallPtrSetImpl<VPBasicBlock *> &DefBlocks,
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

  /// \p BlendOps is filled with the operands in reverse order.
  ///
  /// It is similar to llvm::SSAUpdaterBulk::computeValueAt but we don't stop at
  /// the point of a found value and go up till the immediate dominators to get
  /// all the values that need to be blended. The iteration stops only
  /// when we arrive at the unpredicated block.
  void getBlendArgs(int Idx, VPBasicBlock *AtBB,
                    SmallVectorImpl<VPValue *> &BlendOps) {
    auto ExitCond = [](VPBasicBlock *AtBB){
      return !AtBB->getPredicate() &&
        // FIXME: This is ugly, hopefully we will redesign the whole way
        // uniform instructions with divergent operands are processed...
        none_of(*AtBB, [](const VPInstruction &Inst) {
                return isa<VPActiveLane>(Inst);
        });
    };

    // Manual tail call elimination.
    while (true) {
      assert(ExitCond(&*(AtBB->getParent()->begin())) &&
             "Plan.begin() does not satisfy exit condition.");

      auto IsUndef = [](const VPValue *V) {
        return isa<VPConstant>(V) &&
               isa<UndefValue>(cast<VPConstant>(V)->getConstant());
      };
      if (OrigValsMaps[Idx].count(AtBB)) {
        VPValue *Val = OrigValsMaps[Idx][AtBB];
        if (!IsUndef(Val)) {
          VPValue *Pred = AtBB->getPredicate();
          BlendOps.push_back(Pred);
          BlendOps.push_back(Val);
        }
      }
      // The phi corresponds to the values blended earlier in CFG than the def
      // from the block itself.
      if (MergePhiMaps[Idx].count(AtBB)) {
        VPValue *Val = MergePhiMaps[Idx][AtBB];
        BlendOps.push_back(nullptr /* no predicate/true */);
        BlendOps.push_back(Val);
        return;
      }

      if (ExitCond(AtBB))
        return;

      assert(VPDomTree.getNode(AtBB) && "Invalid Dominator Tree.");
      auto *IDom = VPDomTree.getNode(AtBB)->getIDom();
      assert(IDom && "Block does not have immediate dominator.");
      AtBB = IDom->getBlock(); // Repeat the same with updated AtBB.
    }
  }

  /// For the original phi number \p Idx, create the required blends (if at all)
  /// on the edge \p From -> \p To. In many cases blends coming into the merge
  /// phis have to be introduced on the edge from the predicated block to merge
  /// phi (which is basically the place where two divergent paths based on
  /// different conditions have to merge into the single linearized control
  /// flow). Such blends must be computed unpredicated and we need to introduce
  /// an unpredicated basic block at the place of that edge. \p Builder
  /// parameter serves that purpose. If its insertPointBlock is defined we use
  /// that insertion point, otherwise we do the split and update the \p Builder.
  VPValue *blendOverEdge(int Idx, VPBasicBlock *From, VPBasicBlock *To,
                         VPBuilder &Builder) {
    VPlan *Plan = From->getParent();

    SmallVector<VPValue *, 8> BlendOps;
    getBlendArgs(Idx, From, BlendOps);
    assert(BlendOps.size() % 2 == 0 &&
           "Number of blend operands must be even!");
    int NumBlendVals = BlendOps.size() / 2;

    if (NumBlendVals == 0) {
      // Before linearization | After
      //     entry (U)        |   entry
      //    /    \            |    |  |
      //   /      \           |    | bb1
      // bb3<-bb2<-bb1        |    |  |
      //    \     /           |    | bb2
      //     \   /            |    | /
      //      bb4             |    bb3 ;  merge-phi = phi [undef, %def1]
      //                      |     |
      //                      |    bb4 ; blend merge-phi, true, %def3, bb3.pred
      VPValue *Undef =
          Plan->getVPConstant(UndefValue::get(Phis[Idx]->getType()));
      return Undef;
    }

    if (NumBlendVals == 1) {
      // See the %def1 in the merge-phi in the example above (for the
      // NumBlendVals==0).
      return BlendOps[1];
    }

    if (!Builder.getInsertBlock()) {
      VPBasicBlock *BlendBB = VPBlockUtils::splitEdge(
          From, To, VPlanUtils::createUniqueName("blend.bb"), VPLI, &VPDomTree,
          &VPPostDomTree);
      Builder.setInsertPoint(BlendBB);
    }

    VPBlendInst *Blend = Builder.create<VPBlendInst>(
        Phis[Idx]->getName() + ".blend." + From->getName(),
        Phis[Idx]->getType());
    Plan->getVPlanDA()->markDivergent(*Blend);

    for (int ValNumber = 0; ValNumber < NumBlendVals; ++ValNumber) {
      VPValue *Predicate = BlendOps[NumBlendVals * 2 - ValNumber * 2 - 2];
      VPValue *Val = BlendOps[NumBlendVals * 2 - ValNumber * 2 - 1];
      Blend->addIncoming(Val, Predicate, Plan);
    }
    return Blend;
  }

  void run() {
    if (Phis.empty())
      return;

    if (Phis[0]->getNumIncomingValues() == 1) {
      // LLVM IR CG merges (reuses) several VPBasicBlocks so we can't leave a
      // phi even in case of
      //
      // BB0:
      //   br BB1
      // BB1: ; preds: BB0
      //   %val = phi[ %something, BB0 ]
      //
      // as that might result in a phi in the middle of the llvm::BasicBlock
      // after VPlan CG.
      processSingleIncomingValuePhis();
      return;
    }

    auto &SomePhi = *Phis[0];
    for (auto *PredicateBlock : SomePhi.blocks())
      DefBlocks.insert(PredicateBlock);

    SmallPtrSet<VPBasicBlock *, 4> LiveInBlocks;
    computeLiveInsForBlendsIDF(DefBlocks, Block, LiveInBlocks);

    VPlanForwardIDFCalculator IDF(VPDomTree);
    IDF.setDefiningBlocks(DefBlocks);
    IDF.setLiveInBlocks(LiveInBlocks);
    IDF.calculate(IDFPHIBlocks);

    int Size = Phis.size();

    // Fill in OrigValsMaps and clear original phis.
    for (int Idx = 0; Idx < Size; ++Idx) {
      OrigValsMaps.emplace_back(); // Create an entry.
      VPPHINode *OrigPhi = Phis[Idx];
      for (VPBasicBlock *BB : OrigPhi->blocks()) {
        OrigValsMaps[Idx][BB] = OrigPhi->getIncomingValue(BB);
      }
      OrigPhi->clear();
    }

    for (int Idx = 0; Idx < Size; ++Idx)
      MergePhiMaps.emplace_back(); // Create an entry.

    // Create all the merge phis.
    for (VPBasicBlock *BBForMerge : IDFPHIBlocks) {
      VPBuilder PhiBuilder;
      PhiBuilder.setInsertPoint(BBForMerge, BBForMerge->begin());
      for (int Idx = 0; Idx < Size; ++Idx) {
        VPPHINode *OrigPhi = Phis[Idx];
        VPPHINode *MergePhi =
            BBForMerge == Block
                ? OrigPhi
                : PhiBuilder.createPhiInstruction(OrigPhi->getType(),
                                                  OrigPhi->getName() + ".phi." +
                                                      BBForMerge->getName());
        MergePhiMaps[Idx][BBForMerge] = MergePhi;
      }
    }

    // Fill in merge phis' operands.
    for (VPBasicBlock *BBForMerge : IDFPHIBlocks) {
      SmallVector<VPBasicBlock *, 8> Preds(BBForMerge->getPredecessors());
      for (VPBasicBlock *Pred : Preds) {
        VPBuilder Builder;
        for (int Idx = 0; Idx < Size; ++Idx) {
          VPValue *BlendedVal = blendOverEdge(Idx, Pred, BBForMerge, Builder);
          VPBasicBlock *BlendBB = Builder.getInsertBlock();
          MergePhiMaps[Idx][BBForMerge]->addIncoming(BlendedVal,
                                                     BlendBB ? BlendBB : Pred);
        }
      }
    }

    auto *VecVPlan = cast<VPlanVector>(Block->getParent());
    auto *DA = VecVPlan->getVPlanDA();
    if (is_contained(IDFPHIBlocks, Block)) {
      if (VecVPlan->areActiveLaneInstructionsDisabled())
        return;

      VPActiveLane *ActiveLane = nullptr;
      VPBuilder Builder;
      updateInsertPointForVPActiveLane(Builder, Block);

      for (auto *Phi : Phis) {
        if (DA->isDivergent(*Phi))
          continue;

        if (!ActiveLane) {
          auto *Mask = Block->getPredicate();
          if (!Mask)
            Mask = Block->getParent()->getVPConstant(
                ConstantInt::getTrue(*Block->getParent()->getLLVMContext()));
          ActiveLane = Builder.create<VPActiveLane>("active.lane", Mask);
          DA->markUniform(*ActiveLane);
        }

        auto *UniformVal = Builder.create<VPActiveLaneExtract>(
            Phi->getName() + ".active", Phi, ActiveLane);
        DA->markUniform(*UniformVal);
        DA->markDivergent(*Phi);
        Phi->replaceUsesWithIf(
            UniformVal, [UniformVal](VPUser *U) { return U != UniformVal; });
      }

      return;
    }

    SmallVector<VPValue *, 4> UniformBlends;
    // Final blend.
    VPBasicBlock *Pred = Block->getSinglePredecessor();
    assert(Pred && "Expected single predecessor!");
    VPBuilder Builder;
    Builder.setInsertPoint(Block, Block->begin());
    for (int Idx = 0; Idx < Size; ++Idx) {
      VPValue *BlendedVal = blendOverEdge(Idx, Pred, Block, Builder);
      // TODO: HIR Mixed CG has issues propagating invalidate through the
      // use-chain. Blends/phis are gonna be lowered to using the same temp,
      // so invalidation might not actually be needed.
      Phis[Idx]->replaceAllUsesWith(BlendedVal, false /* InvalidateIR */);

      // Now see if the original phi was uniform and we'd need to extract active
      // value from a potentially divergent def (might be uniform only on the
      // edge leading to the original phi).
      if (DA->isDivergent(*Phis[Idx]))
        continue; // Not a uniform phi, no scalar extract.

      if (!isa<VPBlendInst>(BlendedVal) && !DA->isDivergent(*BlendedVal))
        // Original phi was like [ %uniform, %bb0 ], [ undef, %bb1 ]
        continue;

      // In all other cases we need to extract the uniform value from some
      // active lane.
      UniformBlends.push_back(BlendedVal);
    }
    for (int Idx = 0; Idx < Size; ++Idx)
      Phis[Idx]->getParent()->eraseInstruction(Phis[Idx]);

    if (UniformBlends.empty())
      return;

    if (VecVPlan->areActiveLaneInstructionsDisabled())
      return;

    updateInsertPointForVPActiveLane(Builder, Block);
    VPValue *Mask = Block->getPredicate();
    if (!Mask)
      Mask = Block->getParent()->getVPConstant(
          ConstantInt::getTrue(*Block->getParent()->getLLVMContext()));
    VPActiveLane *ActiveLane =
        Builder.create<VPActiveLane>(Mask->getName() + ".active", Mask);
    DA->markUniform(*ActiveLane);

    for (VPValue *Blend : UniformBlends) {
      VPValue *UniformVal = Builder.create<VPActiveLaneExtract>(
          Blend->getName() + ".active", Blend, ActiveLane);
      DA->markUniform(*UniformVal);
      Blend->replaceUsesWithIf(
          UniformVal, [UniformVal](VPUser *U) { return U != UniformVal; });
    }
  }
};
} // namespace

void VPlanPredicator::transformPhisToBlends() {
  for (VPBasicBlock *Block : RPOT) {
    PhiToBlendUpdater Updater(Block, PhisToBlendProcess[Block]);
    Updater.run();
  }
}

void VPlanPredicator::fixupUniformInnerLoops() {
  VPlanDivergenceAnalysisBase *DA = Plan.getVPlanDA();
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
    DA->updateDivergence(*NewAllZeroCheck);
    VPValue *NewCondBit;
    if (!BackEdgeIsFalseSucc) {
      NewAllZeroCheck = Builder.createNot(NewAllZeroCheck);
      DA->updateDivergence(*NewAllZeroCheck);
      NewCondBit = Builder.createAnd(NewAllZeroCheck, CondBit);
    } else
      NewCondBit = Builder.createOr(NewAllZeroCheck, CondBit);

    DA->updateDivergence(*NewCondBit);
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
        DA->updateDivergence(*BlockPredicateInst);
      }

      continue;
    }

    // Either 2+ incoming edges or a single edge under condition. Create generic
    // OR sequence for all incoming predicates.
    std::list<VPValue *> IncomingConditions;
    for (const auto &Term : PredTerms)
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
      DA->updateDivergence(*BlockPredicateInst);
    }
  }
}

// Entry point. The driver function for the predicator.
void VPlanPredicator::predicate() {
  bool SearchLoopHack = false;
  if (VPLI->size() == 1 && !(*VPLI->begin())->getExitBlock())
    // For function vectorization loop exits canonicalization have already
    // handled it.
    SearchLoopHack = true;

  // Calculate predicates for the blocks in the Plan, but don't lower them into
  // explicit VPInstructions.
  Block2PredicateTermsAndUniformity[&Plan.getEntryBlock()] = {{}, true};
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
  if (!SearchLoopHack)
    linearizeRegion();

  VPLAN_DUMP(LinearizationDumpControl, Plan);

  Plan.computeDT();
  Plan.computePDT();
  {
    // Name scope to ensure stale RPOT after std::swap below won't be misused.
    ReversePostOrderTraversal<VPBasicBlock *> PostLinearizationRPOT(
        &Plan.getEntryBlock());
    std::swap(RPOT, PostLinearizationRPOT);
  }

  // Note that block splitting to make ANDs created for predicates isn't done
  // during emission.
  emitPredicates();

  VPLAN_DUMP(PredicatesEmissionDumpControl, Plan);

  transformPhisToBlends();

  // Now do the block splitting to move ANDs out of block-predicates influence.
  for (const auto &It : BlocksToSplit)
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

VPlanPredicator::VPlanPredicator(VPlanVector &Plan)
    : Plan(Plan), VPLI(Plan.getVPLoopInfo()), RPOT(&Plan.getEntryBlock()) {}
