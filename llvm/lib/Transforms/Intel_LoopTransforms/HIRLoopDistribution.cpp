//===----- HIRLoopDistribution.cpp - Distribution of HIR loops  -----------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Implements HIR Loop Distribution. Works on Loops from innermost to outermost,
// distributing according to specified heuristics. Two important models are
// distribution with intent to form perfect loop nests(enables more
// optimizations such as mem set recognition, interchange) and
// distribution to break recurrences(enables vectorization)
//===----------------------------------------------------------------------===//
//

#include "llvm/ADT/SCCIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"

#include "HIRLoopDistribution.h"

#define DEBUG_TYPE "hir-loop-distribute"

using namespace llvm;
using namespace llvm::loopopt;

cl::opt<bool> DisableDist("disable-hir-loop-distribute",
                          cl::desc("Disable HIR Loop Distribution"), cl::Hidden,
                          cl::init(false));

bool HIRLoopDistribution::runOnFunction(Function &F) {

  this->F = &F;

  if (DisableDist || skipFunction(F)) {
    if (OptReportLevel >= 3) {
      dbgs() << "LOOP DISTRIBUTION: Transform disabled \n";
    }
    return false;
  }

  auto HIRF = &getAnalysis<HIRFramework>();
  DDA = &getAnalysis<HIRDDAnalysis>();
  auto HLS = &getAnalysis<HIRLoopStatistics>();

  SmallVector<HLLoop *, 64> Loops;

  if (DistCostModel == DistHeuristics::BreakMemRec) {
    HIRF->getHLNodeUtils().gatherInnermostLoops(Loops);
  } else {
    HIRF->getHLNodeUtils().gatherAllLoops(Loops);
    // Work from innermost to outermost
    std::sort(Loops.begin(), Loops.end(), [](HLLoop *A, HLLoop *B) -> bool {
      return A->getNestingLevel() > B->getNestingLevel();
    });
  }

  bool Modified = false;
  for (auto I = Loops.begin(), E = Loops.end(); I != E; ++I) {
    HLLoop *Lp = *I;
    if (!loopIsCandidate(Lp)) {
      if (OptReportLevel >= 3) {
        dbgs() << "LOOP DISTRIBUTION: Loop is not candidate with current "
                  "heuristics \n";
      }
      continue;
    }

    std::unique_ptr<PiGraph> PG(new PiGraph(Lp, DDA, HLS));

    if (!PG->isGraphValid()) {
      if (OptReportLevel >= 3) {
        dbgs() << "LOOP DISTRIBUTION: Distribution for loop failed due to "
               << PG->getFailureReason() << "\n";
      }
      continue;
    }

    // Single piblock graph isn't worth considering
    if (PG->size() < 2) {
      if (OptReportLevel >= 3) {
        // TODO might still be able to scalar expand though...
        dbgs() << "LOOP DISTRIBUTION: Skipped loop because of too many "
                  "dependences\n";
      }
      continue;
    }

    SmallVector<PiBlockList, 8> NewOrdering;
    findDistPoints(Lp, PG, NewOrdering);
    if (NewOrdering.size() > 1) {
      distributeLoop(Lp, NewOrdering);
      Modified = true;
    } else {
      if (OptReportLevel >= 3) {
        dbgs() << "LOOP DISTRIBUTION: "
               << "Found no valid distribution points"
               << "\n";
      }
    }
  }

  return Modified;
}

bool HIRLoopDistribution::piEdgeIsRecurrence(const HLLoop *Lp,
                                             const PiGraphEdge &Edge) const {
  for (auto DDEdgeIt = Edge.getDDEdges().begin(), End = Edge.getDDEdges().end();
       DDEdgeIt != End; ++DDEdgeIt) {
    if ((*DDEdgeIt)->getDVAtLevel(Lp->getNestingLevel()) & DVKind::LT) {
      return true;
    }
  }
  return false;
}

void HIRLoopDistribution::distributeLoop(
    HLLoop *Loop, SmallVectorImpl<PiBlockList> &DistPoints) {

  assert(DistPoints.size() > 1 && "Invalid loop distribution");

  if (OptReportLevel >= 3) {
    dbgs() << "LOOP DISTRIBUTION : " << DistPoints.size()
           << " way distributed\n";
  }
  unsigned LastLoopNum = DistPoints.size();
  unsigned Num = 0;
  bool CopyPreHeader = true;
  auto &HNU = Loop->getHLNodeUtils();

  for (PiBlockList &PList : DistPoints) {
    // Each PiBlockList forms a new loop
    // Clone Empty Loop, but copy preheader for 1st loop and
    // postexit for last loop
    // TODO: Determine which loop needs  preheader/postexit

    HLLoop *NewLoop = Loop->cloneEmptyLoop();
    if (CopyPreHeader) {
      HNU.moveAsFirstPreheaderNodes(NewLoop, Loop->pre_begin(),
                                    Loop->pre_end());
      CopyPreHeader = false;
    }
    if (++Num == LastLoopNum) {
      HNU.moveAsFirstPostexitNodes(NewLoop, Loop->post_begin(),
                                   Loop->post_end());
    }

    HNU.insertBefore(Loop, NewLoop);
    // Each piblock is comprised of multiple HLNodes
    for (PiBlock *PiBlk : PList) {
      for (auto NodeI = PiBlk->nodes_begin(), E = PiBlk->nodes_end();
           NodeI != E; ++NodeI) {
        HNU.moveAsLastChild(NewLoop, *NodeI);
      }
    }
  }

  Loop->getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
      Loop);
  HIRInvalidationUtils::invalidateBody(Loop);

  // The loop is now empty, all its children moved into new loops
  assert(!Loop->hasChildren() &&
         "Loop Distribution failed to account for all Loop Children");
  HNU.remove(Loop);
}

// Form perfect loop candidates by grouping stmt only piblocks
void HIRLoopDistribution::formPerfectLoopNests(
    std::unique_ptr<PiGraph> const &PGraph,
    SmallVectorImpl<PiBlockList> &DistPoints) const {

  // All piblocks that are only stmts and are roots in DAG can form their
  // own distributed loop
  PiBlockList StmtRootBlocks;

  PiBlockList CurLoopPiBlkList;
  const HLLoop *InnermostLoop;

  for (auto N = PGraph->node_begin(), E = PGraph->node_end(); N != E; ++N) {
    PiBlock *Blk = *N;
    PiBlock::PiBlockType BlockType = Blk->getBlockType();
    if (PGraph->incoming_edges_begin(Blk) == PGraph->incoming_edges_end(Blk)) {
      // If blk has no incoming edges it must be the root of a component of
      // top sorted graph
      if (BlockType == PiBlock::PiBlockType::SingleStmt ||
          BlockType == PiBlock::PiBlockType::MultipleStmt) {
        // stmt only root blocks form their own perfect loop
        StmtRootBlocks.push_back(Blk);
      } else if (BlockType == PiBlock::PiBlockType::SingleLoop) {
        HLLoop *SingleLoop =
            dyn_cast<HLLoop>((*(Blk->dist_node_begin()))->HNode);
        assert(SingleLoop && "SingleLoop piblock did not contain a loop");
        // perfect subloops are distributed into their own loop
        if (SingleLoop->isInnermost() ||
            HLNodeUtils::isPerfectLoopNest(SingleLoop, &InnermostLoop)) {
          DistPoints.push_back(PiBlockList(1, Blk));
        } else {
          CurLoopPiBlkList.push_back(Blk);
        }
      } else {
        // piblocks of mixed loop/stmts cannot form their own perfect loop
        // add them to the current loop
        CurLoopPiBlkList.push_back(Blk);
      }
    } else {
      if (BlockType == PiBlock::PiBlockType::SingleLoop) {
        HLLoop *SingleLoop =
            dyn_cast<HLLoop>((*(Blk->dist_node_begin()))->HNode);
        assert(SingleLoop && "SingleLoop piblock did not contain a loop");
        if (SingleLoop->isInnermost() ||
            HLNodeUtils::isPerfectLoopNest(SingleLoop, &InnermostLoop)) {
          // terminate our current loop and append it to loop list
          if (!CurLoopPiBlkList.empty()) {
            DistPoints.push_back(CurLoopPiBlkList);
            CurLoopPiBlkList.clear();
          }
          // and make the perfect loop its own loop nest, appended to loop
          // list in order to maintain program order
          PiBlockList PerfectLoop;
          PerfectLoop.push_back(Blk);
          DistPoints.push_back(PerfectLoop);
        } else {
          CurLoopPiBlkList.push_back(Blk);
        }
      } else {
        CurLoopPiBlkList.push_back(Blk);
      }
    }
  }

  // Terminate current loop if we haven't already
  if (!CurLoopPiBlkList.empty()) {
    DistPoints.push_back(CurLoopPiBlkList);
    CurLoopPiBlkList.clear();
  }

  // The loop represented by this list must come before all others,
  if (!StmtRootBlocks.empty()) {
    DistPoints.insert(DistPoints.begin(), StmtRootBlocks);
  }
}

bool HIRLoopDistribution::loopIsCandidate(const HLLoop *Lp) const {
  // TODO This will miss some opportunities
  // Ex. L has 6 PiBlocks with the first 5 having an edge to 6, which is
  // comprised only of a loop, making L not the innermost. If the first 5
  // piblocks are themselves comprised only of a single stmt
  // then we would want to distribute them into their own single vectorizable
  // loop.
  // However, considering other loops may create more loops unnecessarily
  // do i
  //  do j
  //    PiBlock 1
  //    PiBlock 2
  // Single edge 1->2 indicating recurrence
  // Distributing j forms two vectorizable loops. There will still be an
  // edge between the two new loops when considering I. Distributing
  // again won't enable vectorization, but create more loop overhead.

  if (DistCostModel == DistHeuristics::NestFormation && Lp->isInnermost()) {
    return false;
  }

  if (DistCostModel == DistHeuristics::NestFormation) {
    // Skipping innermost may create fewer perfect nests, but its
    // not necessarily bad.
    // Example:
    // do i
    //  do j
    //    S1 (PiBlock 1)
    //    do k
    //      S2-S4 (PiBlock 2)
    //      S5-S8 (PiBlock 3)
    // Single Edge 1->3
    // Distributing j loop will consider k loop a single node. We might split
    // that entire node into its own loop nest. Second distribution on i loop
    // will result in two perfect nests.
    // If we consider innermost a candidate we might end up with 3 perfect
    // nests if blocks 2 and 3 are distributed once for K loop and again at
    // j loop

    const HLLoop *InnermostLoop;
    auto &HNU = Lp->getHLNodeUtils();

    // Why ruin perfection
    // Should we run distribution in perfect loopnest mode on innermost loops?
    if (!Lp->isInnermost() &&
        HLNodeUtils::isPerfectLoopNest(Lp, &InnermostLoop)) {
      return false;
    }

    // For compile time consideration, throttle for
    // more than 3 innermost loops or nesting level > 3
    // Forming Perfect Loop Nest is primarily to enable interchange

    SmallVector<HLLoop *, 12> InnermostLPVector;

    HNU.gatherInnermostLoops(InnermostLPVector, const_cast<HLLoop *>(Lp));
    if (InnermostLPVector.size() > 2) {
      return false;
    }
    bool NonUnitStride = false;
    for (auto &Loop : InnermostLPVector) {
      if ((Loop->getNestingLevel() - Lp->getNestingLevel()) > 2) {
        return false;
      }
      if (!NonUnitStride && HNU.hasNonUnitStrideRefs(Loop)) {
        NonUnitStride = true;
      }
    }
    if (!NonUnitStride) {
      return false;
    }
  }

  return true;
}

void HIRLoopDistribution::breakPiBlockRecurrences(
    const HLLoop *Lp, std::unique_ptr<PiGraph> const &PGraph,
    SmallVectorImpl<PiBlockList> &DistPoints) const {

  PiBlockList CurLoopPiBlkList;
  // Walk through topsorted nodes of Pigraph, keeping in mind the fact that each
  // of those nodes can legally form its own loop if the loops(distributed
  // chunks) are ordered in same top sort order of nodes.
  // Add the node to current loop. Look for outgoing edges that indicate
  // recurrences. If none, continue on. Otherwise terminate the current loop,
  // start a new one. Src and sink of recurrence will be in different loops,
  // breaking the recurrence.
  for (auto N = PGraph->node_begin(), E = PGraph->node_end(); N != E; ++N) {
    PiBlock *SrcBlk = *N;
    CurLoopPiBlkList.push_back(SrcBlk);
    for (auto EdgeIt = PGraph->outgoing_edges_begin(SrcBlk),
              EndEdgeIt = PGraph->outgoing_edges_end(SrcBlk);
         EdgeIt != EndEdgeIt; ++EdgeIt) {
      // TODO this is overly aggressive for at least two reasons.
      // Case1: 3 block graph with 2 edges,
      // 1->3, 2->3. This would create 3 loops, but 1 and 2 could have been
      // combined. OTOH, if piblock 1 would form a non vectorizable loop and
      // 2 doesnt, we would want them separate.
      // Case2: 2 block graph, with single recurrence edge between the two
      // Once split, the two loops are non vectorizable. If legality was
      // the only concern, we can iterate over all dd edges indirectly by
      // looking at distppedges whos src/sink are in piblock
      if (piEdgeIsRecurrence(Lp, *(*EdgeIt))) {
        DistPoints.push_back(CurLoopPiBlkList);
        CurLoopPiBlkList.clear();
        break;
      }
      // TODO if sink blk is known to be non vectorizable and src blk(s) is
      // vectorizble, we would want to split as well even if edge is not a
      // recurrence
    }
  }
  if (!CurLoopPiBlkList.empty()) {
    DistPoints.push_back(CurLoopPiBlkList);
  }
}

void HIRLoopDistribution::findDistPoints(
    const HLLoop *Lp, std::unique_ptr<PiGraph> const &PGraph,
    SmallVectorImpl<PiBlockList> &DistPoints) const {

  if (DistCostModel == DistHeuristics::BreakMemRec) {
    breakPiBlockRecurrences(Lp, PGraph, DistPoints);
  } else if (DistCostModel == DistHeuristics::NestFormation) {
    formPerfectLoopNests(PGraph, DistPoints);
  }

  DEBUG(dbgs() << "Loop Dist proposes " << DistPoints.size() << " Loops\n");
}
