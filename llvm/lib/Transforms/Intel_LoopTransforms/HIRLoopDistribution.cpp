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
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDTests.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopDistributionGraph.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"

#define DEBUG_TYPE "hir-loop-distribute"

using namespace llvm;
using namespace llvm::loopopt;

cl::opt<bool> DisableDist("disable-hir-loop-distribute",
                          cl::desc("Disable HIR Loop Distribution"), cl::Hidden,
                          cl::init(false));

namespace {

enum class DistHeuristics : unsigned char {
  NotSpecified = 0, // Default enum for command line option. Will be overridden
                    // by pass constructor argument
  MaximalDist,      // Everytime you can, do it. Testing only
  NestFormation,    // Try to form perfect loop nests
  BreakMemRec,      // Break recurrence among mem refs ie A[i] -> A[i+i]
  BreakScalarRec,   // Break recurrence among scalars. Requires scalar expansion
};

cl::opt<DistHeuristics> CmdLineHeuristics(
    "hir-loop-distribute-heuristics", cl::Hidden,
    cl::desc("HIR Loop Distribution Heuristics"),
    cl::values(
        clEnumValN(DistHeuristics::NotSpecified, "none",
                   "No specified heuristics, uses pass defaults"),
        clEnumValN(DistHeuristics::MaximalDist, "maximal",
                   "Distribute at every legal opportunity(not implemented)"),
        clEnumValN(DistHeuristics::NestFormation, "nest",
                   "Distribute with intent to form perfect loop nests"),
        clEnumValN(DistHeuristics::BreakMemRec, "mem-rec",
                   "Distribute to break recurrences among memory references"),
        clEnumValN(
            DistHeuristics::BreakScalarRec, "scalar-rec",
            "Distribute to break recurrences among scalars(not implemented)"),
        clEnumValEnd),
    cl::init(DistHeuristics::NotSpecified));

class HIRLoopDistribution : public HIRTransformPass {
  typedef SmallVector<PiBlock *, 4> PiBlockList;

public:
  static char ID;

  HIRLoopDistribution(bool FormPerfectLoopNests = true) : HIRTransformPass(ID) {
    initializeHIRLoopDistributionPass(*PassRegistry::getPassRegistry());
    if (FormPerfectLoopNests) {
      DistCostModel = DistHeuristics::NestFormation;
    } else {
      DistCostModel = DistHeuristics::BreakMemRec;
    }
  }

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
    AU.addRequiredTransitive<HIRDDAnalysis>();
  }

private:
  Function *F;
  HIRDDAnalysis *DDA;
  int OptReportLevel = 1;

  // Heuristics set used for this pass
  DistHeuristics DistCostModel = DistHeuristics::BreakMemRec;

  void findDistPoints(const HLLoop *L, std::unique_ptr<PiGraph> const &PGraph,
                      SmallVectorImpl<PiBlockList> &DistPoints) const;

  // Returns true if this edge contains dd edge with (<) at loop level
  // Such an edge would be eliminated by distributing the src sink piblocks
  // into separate loops
  bool piEdgeIsRecurrence(const HLLoop *Lp, const PiGraphEdge &Edge) const;

  // Loop may be discarded prior to any analysis by some heuristics.
  // For example, the costmodel may consider only innermost loops, no need
  // to do potentially expensive analysis on others
  bool loopIsCandidate(const HLLoop *L) const;

  // Breaks up pi graph into loops(loop is formed by a list of piblocks)
  // according to appropriate "cost model".  Very primitive and missing
  // important considerations such as trip count, predicted vectorizability
  void breakPiBlockRecurrences(const HLLoop *L,
                               std::unique_ptr<PiGraph> const &PiGraph,
                               SmallVectorImpl<PiBlockList> &DistPoints) const;

  // Breaks up pigraph with intent to form perfect loop nests, even at cost
  // of skipping creation of potentially vectorizable loops
  void formPerfectLoopNests(std::unique_ptr<PiGraph> const &PGraph,
                            SmallVectorImpl<PiBlockList> &DistPoints) const;

  // Uses ordered list of PiBlockLists to form distributed version of Loop.
  // Each PiBlockList will form a new loop(with same bounds as Loop) containing
  // each piblock's hlnodes.
  void distributeLoop(HLLoop *L, SmallVectorImpl<PiBlockList> &DistPoints);
};
}

char HIRLoopDistribution::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopDistribution, "hir-loop-distribute",
                      "HIR Loop Distribution", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_END(HIRLoopDistribution, "hir-loop-distribute",
                    "HIR Loop Distribution", false, false)

FunctionPass *llvm::createHIRLoopDistributionPass(bool FormPerfectNests) {
  return new HIRLoopDistribution(FormPerfectNests);
}

bool HIRLoopDistribution::piEdgeIsRecurrence(const HLLoop *Lp,
                                             const PiGraphEdge &Edge) const {
  for (auto DDEdgeIt = Edge.getDDEdges().begin(), End = Edge.getDDEdges().end();
       DDEdgeIt != End; ++DDEdgeIt) {
    if ((*DDEdgeIt)->getDVAtLevel(Lp->getNestingLevel()) & DV::LT) {
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

  for (PiBlockList &PList : DistPoints) {
    // Each PiBlockList forms a new loop
    HLLoop *NewLoop = Loop->cloneEmptyLoop();
    HLNodeUtils::insertBefore(Loop, NewLoop);
    // Each piblock is comprised of multiple hlnodes
    for (PiBlock *PiBlk : PList) {
      for (auto NodeI = PiBlk->nodes_begin(), E = PiBlk->nodes_end();
           NodeI != E; ++NodeI) {
        HLNodeUtils::moveAsLastChild(NewLoop, *NodeI);
      }
    }
  }

  Loop->getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Loop);
  HIRInvalidationUtils::invalidateBody(Loop);

  // The loop is now empty, all its children moved into new loops
  assert(!Loop->hasChildren() &&
         "Loop Distribution failed to account for all Loop Children");
  HLNodeUtils::erase(Loop);
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
  if (!Lp->hasChildren()) {
    return false;
  }
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
  if (DistCostModel == DistHeuristics::BreakMemRec && !Lp->isInnermost()) {
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
    // Why ruin perfection
    if (HLNodeUtils::isPerfectLoopNest(Lp, &InnermostLoop)) {
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
      if (piEdgeIsRecurrence(Lp, *EdgeIt)) {
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

bool HIRLoopDistribution::runOnFunction(Function &F) {

  this->F = &F;

  if (DisableDist) {
    if (OptReportLevel >= 3) {
      dbgs() << "LOOP DISTRIBUTION: Transform disabled \n";
    }
    return false;
  }

  if (CmdLineHeuristics != DistHeuristics::NotSpecified) {
    DistCostModel = CmdLineHeuristics;
  }

  DDA = &getAnalysis<HIRDDAnalysis>();
  SmallVector<HLLoop *, 64> Loops;
  HLNodeUtils::gatherAllLoops(Loops);

  // Work from innermost to outermost
  std::sort(Loops.begin(), Loops.end(), [](HLLoop *A, HLLoop *B) -> bool {
    return A->getNestingLevel() > B->getNestingLevel();
  });

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

    std::unique_ptr<PiGraph> PG(new PiGraph(Lp, DDA));
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
