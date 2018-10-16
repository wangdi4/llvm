//===----- HIRLoopDistribution.cpp - Distribution of HIR loops  -----------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSparseArrayReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRLoopDistributionImpl.h"

#define DEBUG_TYPE "hir-loop-distribute"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::distribute;

cl::opt<bool> DisableDist("disable-hir-loop-distribute",
                          cl::desc("Disable HIR Loop Distribution"), cl::Hidden,
                          cl::init(false));

bool HIRLoopDistribution::run() {
  if (DisableDist) {
    LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION: Transform disabled\n");
    return false;
  }

  SmallVector<HLLoop *, 64> Loops;

  if (DistCostModel == DistHeuristics::BreakMemRec) {
    HIRF.getHLNodeUtils().gatherInnermostLoops(Loops);
  } else {
    HIRF.getHLNodeUtils().gatherAllLoops(Loops);
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
    unsigned TotalMemOps = 0;
    bool ForceCycleForLoopIndepDep = true;

    if (DistCostModel == DistHeuristics::BreakMemRec) {
      TotalMemOps = HLR.getSelfLoopResource(Lp).getNumIntMemOps() +
                    HLR.getSelfLoopResource(Lp).getNumFPMemOps();
      if (TotalMemOps >= MaxMemResourceToDistribute) {
        ForceCycleForLoopIndepDep = false;
      }
    }

    // Sparse array reduction info is needed to create the DistPPGraph
    // and in findDistPoints while breaking the PiBlock Recurrences.
    SARA.computeSparseArrayReductionChains(Lp);

    std::unique_ptr<PiGraph> PG(
        new PiGraph(Lp, DDA, SARA, ForceCycleForLoopIndepDep));

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
        dbgs() << "LOOP DISTRIBUTION:  too many dependences\n";
      }

      if (DistCostModel != DistHeuristics::BreakMemRec) {
        continue;
      }
    }

    SmallVector<PiBlockList, 8> NewOrdering;
    findDistPoints(Lp, PG, NewOrdering);

    if (NewOrdering.size() > 1) {
      Modified = distributeLoop(Lp, NewOrdering, HIRF.getLORBuilder());
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

static void updateLiveInAllocaTemp(HLLoop *Loop, unsigned SB) {

  HLLoop *Lp = Loop;
  while (Lp) {
    Lp->addLiveInTemp(SB);
    Lp = Lp->getParentLoop();
  }
}

RegDDRef *HIRLoopDistribution::createTempArrayStore(RegDDRef *TempRef) {

  // TEMP[i] = tx
  HLDDNode *HLNode = TempRef->getHLDDNode();
  HLLoop *Lp = HLNode->getParentLoop();

  auto ArrTy = ArrayType::get(TempRef->getDestType(), StripmineSize);

  AllocaBlobIdx = HNU.createAlloca(ArrTy, RegionNode, ".TempArray");

  RegDDRef *TmpArrayRef = HNU.getDDRefUtils().createMemRef(AllocaBlobIdx);

  auto IVType = Lp->getIVType();
  CanonExpr *FirstCE = TempRef->getCanonExprUtils().createCanonExpr(IVType);
  FirstCE->addIV(LoopLevel, 0, 1);
  //  Create constant of 0
  CanonExpr *SecondCE = TempRef->getCanonExprUtils().createCanonExpr(IVType);
  TmpArrayRef->addDimension(FirstCE);
  TmpArrayRef->addDimension(SecondCE);
  HLInst *StoreInst = HNU.createStore(TempRef->clone(), ".TempSt", TmpArrayRef);

  HLNodeUtils::insertAfter(HLNode, StoreInst);

  updateLiveInAllocaTemp(Lp, TmpArrayRef->getBasePtrSymbase());
  TempArraySB.push_back(TmpArrayRef->getSymbase());

  return TmpArrayRef;
}

void HIRLoopDistribution::createTempArrayLoad(RegDDRef *TempRef,
                                              RegDDRef *TmpArrayRef,
                                              HLDDNode *Node) {

  //  tx = TEMP[i]
  HLLoop *Lp = Node->getParentLoop();

  const std::string TempName = "scextmp";
  HLInst *LoadInst =
      HNU.createLoad(TmpArrayRef->clone(), TempName, TempRef->clone());

  // if stmt is inside an if, insertion should be done before If
  // because we do insert once per loop

  auto TmpNode = Node;
  HLNode *IfParent;

  do {
    IfParent = TmpNode;
    TmpNode = dyn_cast<HLIf>(TmpNode->getParent());
  } while (TmpNode);

  HLNodeUtils::insertBefore(IfParent, LoadInst);
  updateLiveInAllocaTemp(Lp, TmpArrayRef->getBasePtrSymbase());
  TempArraySB.push_back(TmpArrayRef->getSymbase());
}

void HIRLoopDistribution::replaceWithArrayTemp(
    TerminalRefGatherer::VectorTy *Refs) {

  RegDDRef *TmpArrayRef = nullptr;

  for (unsigned I = 0; I < LastLoopNum - 1; ++I) {
    // refs can only be terminal or blobs here
    for (DDRef *Ref : Refs[I]) {
      if (Ref->isRval()) {
        continue;
      }

      bool StoreInserted = false;
      RegDDRef *TempRef = cast<RegDDRef>(Ref);
      const HLInst *Inst = dyn_cast<HLInst>(TempRef->getHLDDNode());

      // No need to check for safe reduction because reduction temps
      // will not be distributed into two different loops
      if (Inst && Inst->isInPreheaderOrPostexit()) {
        continue;
      }
      for (unsigned J = I + 1; J < LastLoopNum; ++J) {
        bool LoadInserted = false;
        for (auto It = Refs[J].begin(); It != Refs[J].end();) {
          // Refs here could be blob or temp
          DDRef *SinkRef = *It;
          if (SinkRef->isLval() ||
              TempRef->getSymbase() != SinkRef->getSymbase()) {
            ++It;
            continue;
          }
          // Create TEMP[i] = tx and insert
          if (!StoreInserted) {
            TmpArrayRef = createTempArrayStore(TempRef);
            StoreInserted = true;
          }
          //  Create tx = TEMP[i] and insert.  Cannot do direct replacement
          //  because Copy Inst does not allow Memref
          if (!LoadInserted) {
            createTempArrayLoad(TempRef, TmpArrayRef, SinkRef->getHLDDNode());
            LoadInserted = true;
          }

          It = Refs[J].erase(It);
        }
      }
    }
  }
}

bool HIRLoopDistribution::arrayTempExceeded(
    unsigned LastLoopNum, unsigned &NumArrayTemps,
    TerminalRefGatherer::VectorTy *Refs) {

  NumArrayTemps = 0;
  if (DistCostModel != DistHeuristics::BreakMemRec) {
    return false;
  }

  for (unsigned I = 0; I < LastLoopNum - 1; ++I) {
    for (const DDRef *Ref : Refs[I]) {
      if (Ref->isRval()) {
        continue;
      }

      const HLInst *Inst = dyn_cast<HLInst>(Ref->getHLDDNode());
      if (Inst &&
          (Inst->isInPreheaderOrPostexit() || SRA.isSafeReduction(Inst))) {
        continue;
      }
      // Check any usage in another loop
      bool Done = false;
      for (unsigned J = I + 1; J < LastLoopNum && !Done; ++J) {
        for (const DDRef *SinkRef : Refs[J]) {

          if (SinkRef->isLval()) {
            continue;
          }

          if (Ref->getSymbase() == SinkRef->getSymbase()) {
            if (++NumArrayTemps >= MaxArrayTempsAllowed) {
              LLVM_DEBUG(dbgs()
                         << "Loop Dist  bail out because #of Array temps "
                            "exceeded");
              return true;
            }
            // Add to NumArrayTemp once per temp
            Done = true;
            break;
          }
        }
      }
    }
  }

  return false;
}

/// distributeLoop handles distribution to enable perfect loop nests and
/// breaking of
///   memref recurrences. In addition, loops with a lot of memory references
///   will be distributed

/// Distribution with Scalar Expansion:
/// t1 =  ..  ;  = t1;  enables more expressions to be distributed into
/// different loop nests. Temps need to be changed to small arrays by
/// stripmining.
///  - main focus is to split up loops that have too many memrefs because
///    HW prefetcher will give up and Strength reduction code cannot not handle
///    well.
///  - Ideally, maximal distribution should be done and let fusion fuses it
///  back.
///    But product compiler cannot afford the long compile time.
///  - We can replace the temp by temp array before distribution.
///    Temp array is then cleaned up later if Dead store can be done.
///    It's fine for compiler that has expressions trees.
///    But LLVM has a lot of temps. We need to use a different approach.
/// -  The trick is to avoid the backedge in Dist graph for scalar temps that
/// have DV (=)
/// -  Replaces them later with Array temp when it is needed after distribution.
/// -  There is an advantage of LLVM with many temps. We don't need to do node
/// splitting.
/// -  But if we relax too much for not creating the backedge for temps,
///     some of the live range of the temp can become larger.
/// -  We can add extra dist edge for lexical links. That may also create cases
/// that hinder distribution
///     because there could be other back edges, which is not unknown until we
///     build the pi-graph.
/// -  In summary, without using maximal distribution, the solution cannot be
/// perfect.

bool HIRLoopDistribution::distributeLoop(
    HLLoop *Loop, SmallVectorImpl<PiBlockList> &DistPoints,
    LoopOptReportBuilder &LORBuilder) {

  assert(DistPoints.size() > 1 && "Invalid loop distribution");

  if (OptReportLevel >= 3) {
    dbgs() << "LOOP DISTRIBUTION : " << DistPoints.size()
           << " way distributed\n";
  }

  TempArraySB.clear();
  bool CopyPreHeader = true;
  HLLoop *LoopNode;
  LastLoopNum = DistPoints.size();
  if (LastLoopNum >= MaxDistributedLoop) {
    return false;
  }

  RegionNode = Loop->getParentRegion();
  LoopLevel = Loop->getNestingLevel();

  // Gather DDRefs into an array per loop
  TerminalRefGatherer::VectorTy Refs[MaxDistributedLoop];

  unsigned I = 0;
  for (PiBlockList &PList : DistPoints) {
    // Each PiBlockList forms a new loop
    // Each piblock is comprised of multiple HLNodes
    for (PiBlock *PiBlk : PList) {
      for (auto NodeI = PiBlk->nodes_begin(), E = PiBlk->nodes_end();
           NodeI != E; ++NodeI) {
        TerminalRefGatherer::gather(*NodeI, Refs[I]);
      }
    }
    I++;
  }
  // Find number of Scalar Temps.
  // Large number of extra memory refs will affect performance
  // Do not proceed if threshold exceeded

  if (arrayTempExceeded(LastLoopNum, NumArrayTemps, Refs)) {
    return false;
  }

  bool NotRequired = true;
  if (NumArrayTemps && !(Loop->canStripmine(StripmineSize, NotRequired))) {
    return false;
  }

  unsigned Num = 0;
  I = 0;
  for (PiBlockList &PList : DistPoints) {
    // Each PiBlockList forms a new loop
    // Clone Empty Loop. Copy preheader for 1st loop and
    // postexit for last loop
    // TODO: Determine which loop needs preheader/postexit

    LoopNode = Loop->cloneEmpty();
    NewLoops[I++] = LoopNode;

    if (CopyPreHeader) {
      HLNodeUtils::moveAsFirstPreheaderNodes(LoopNode, Loop->pre_begin(),
                                             Loop->pre_end());
      CopyPreHeader = false;
      LORBuilder(*LoopNode).addRemark(OptReportVerbosity::Low,
                                      "Loop distributed (%d way)", LastLoopNum);
    }
    if (++Num == LastLoopNum) {
      HLNodeUtils::moveAsFirstPostexitNodes(LoopNode, Loop->post_begin(),
                                            Loop->post_end());
    }
    // Each piblock is comprised of multiple HLNodes
    for (PiBlock *PiBlk : PList) {
      for (auto NodeI = PiBlk->nodes_begin(), E = PiBlk->nodes_end();
           NodeI != E; ++NodeI) {
        HLNodeUtils::moveAsLastChild(LoopNode, *NodeI);
      }
    }
    LORBuilder(*LoopNode).addOrigin("Distributed chunk %d", Num);
  }

  // The loop is now empty, all its children moved into new loops
  assert(!Loop->hasChildren() &&
         "Loop Distribution failed to account for all Loop Children");

  for (unsigned I = 0; I < LastLoopNum; ++I) {
    // Distributed flag is used by Loop Fusion to skip loops that are
    // distributed Need to set for Memory related distribution only. Distributed
    // loops for enabling perfect loop nest, can still be fused after
    // interchange is done
    if (DistCostModel == DistHeuristics::BreakMemRec) {
      NewLoops[I]->setDistributedForMemRec();
    }
    HLNodeUtils::insertBefore(Loop, NewLoops[I]);
  }

  if (NumArrayTemps) {
    replaceWithArrayTemp(Refs);
    // For constant trip count <= StripmineSize, no stripmine is done
    if (!NotRequired) {
      HIRTransformUtils::stripmine(NewLoops[0], NewLoops[LastLoopNum - 1],
                                   StripmineSize);
      // Fix TempArray index if stripmine is peformed: 64 * i1 + i2 => i2
      fixTempArrayCoeff(NewLoops[0]->getParentLoop());
    }
  }

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
      Loop);
  HIRInvalidationUtils::invalidateBody(Loop);

  RegionNode->setGenCode();
  HLNodeUtils::remove(Loop);
  return true;
}

void HIRLoopDistribution::fixTempArrayCoeff(HLLoop *Loop) {

  // After stripemine, change coeff from  of TempArray
  //  from  i1 * 64 + i2  to   i2
  unsigned Level = Loop->getNestingLevel();

  ForEach<HLDDNode>::visitRange(
      Loop->child_begin(), Loop->child_end(), [this, Level](HLDDNode *Node) {
        for (RegDDRef *Ref :
             llvm::make_range(Node->ddref_begin(), Node->ddref_end())) {
          if (std::find(TempArraySB.begin(), TempArraySB.end(),
                        Ref->getSymbase()) == TempArraySB.end()) {
            continue;
          }

          for (CanonExpr *CE :
               llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
            CE->removeIV(Level);
          }
        }
      });
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

  if (Lp->hasUnrollEnablingPragma() || Lp->hasVectorizeEnablingPragma()) {
    return false;
  }

  if (DistCostModel == DistHeuristics::NestFormation && Lp->isInnermost()) {
    return false;
  }

  uint64_t TripCount;
  // Skip  some constant trip counts loops:  small, looks like copy stmt
  if (Lp->isConstTripLoop(&TripCount)) {
    if (TripCount < 5 || HLR.getTotalLoopResource(Lp).getNumFPOps() == 0) {
      return false;
    }
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
      if (!NonUnitStride && HLNodeUtils::hasNonUnitStrideRefs(Loop)) {
        NonUnitStride = true;
      }
    }
    if (!NonUnitStride) {
      return false;
    }
  }

  return true;
}

// Right now we are checking whether this PiBlock contains any sparse
// array reduction instructions. Later we may want to modify to match more
// patterns like in 435.gromacs
bool containsSparseArrayReductions(PiBlock *SrcBlk,
                                   HIRSparseArrayReductionAnalysis &SARA) {
  for (auto NodeI = SrcBlk->nodes_begin(), E = SrcBlk->nodes_end(); NodeI != E;
       ++NodeI) {
    HLDDNode *Node = cast<HLDDNode>(*NodeI);
    HLInst *Inst = dyn_cast<HLInst>(Node);
    if (Inst && SARA.isSparseArrayReduction(Inst)) {
      return true;
    }
  }
  return false;
}

void HIRLoopDistribution::breakPiBlockRecurrences(
    const HLLoop *Lp, std::unique_ptr<PiGraph> const &PGraph,
    SmallVectorImpl<PiBlockList> &DistPoints) const {

  PiBlockList CurLoopPiBlkList;
  // Walk through topsorted nodes of Pigraph, keeping in mind the fact that
  // each of those nodes can legally form its own loop if the
  // loops(distributed chunks) are ordered in same topsort order of nodes. Add
  // the node to current loop. Look for outgoing edges that indicate
  // recurrences. If none, continue on. Otherwise terminate the current loop,
  // start a new one. Src and sink of recurrence will be in different loops,
  // breaking the recurrence.

  unsigned NumRefCounter = 0;
  SmallVector<unsigned, 12> MemRefSBVector;

  SRA.computeSafeReductionChains(Lp);
  unsigned HasSparseArrayReductions =
      SARA.getNumSparseArrayReductionChains(Lp) > 0;

  // Get number of loads/stores, needed to decide if threashold is exceeded.
  // Arrays with same SB, in general, have locality, and do not need to be
  // added twice.  Will need some fine tuning later

  for (auto N = PGraph->node_begin(), E = PGraph->node_end(); N != E; ++N) {
    PiBlock *SrcBlk = *N;

    // If this current block has sparse array reduction instructions,
    // we need to break the recurrence before this block so that sparse array
    // reductions can be distributed into another separate loop.
    if (HasSparseArrayReductions && !CurLoopPiBlkList.empty() &&
        containsSparseArrayReductions(SrcBlk, SARA)) {
      DistPoints.push_back(CurLoopPiBlkList);
      CurLoopPiBlkList.clear();
      NumRefCounter = 0;
    }

    for (auto NodeI = SrcBlk->nodes_begin(), E = SrcBlk->nodes_end();
         NodeI != E; ++NodeI) {
      HLDDNode *Node = cast<HLDDNode>(*NodeI);

      for (auto RefIt = Node->ddref_begin(), E = Node->ddref_end(); RefIt != E;
           ++RefIt) {
        RegDDRef *Ref = *RefIt;
        if (Ref->isMemRef()) {
          unsigned SB = Ref->getSymbase();
          if (std::find(MemRefSBVector.begin(), MemRefSBVector.end(), SB) !=
              MemRefSBVector.end()) {
            continue;
          }
          MemRefSBVector.push_back(SB);
          if (Ref->isRval()) {
            NumRefCounter++;
          } else {
            NumRefCounter += 2;
          }
        }
      }
    }

    CurLoopPiBlkList.push_back(SrcBlk);
    if (NumRefCounter >= MaxMemResourceToDistribute) {
      DistPoints.push_back(CurLoopPiBlkList);
      CurLoopPiBlkList.clear();
      NumRefCounter = 0;
      continue;
    }

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

  LLVM_DEBUG(dbgs() << "Loop Dist proposes " << DistPoints.size()
                    << " Loops\n");
}

void HIRLoopDistributionLegacyPass::getAnalysisUsage(
    llvm::AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  // Loop Statistics is not used by this pass directly but it used by
  // HLNodeUtils::dominates() utility. This is a workaround to keep the pass
  // manager from freeing it.
  AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  AU.addRequiredTransitive<HIRLoopResourceWrapperPass>();
  AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
  AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
  AU.addRequiredTransitive<HIRSparseArrayReductionAnalysisWrapperPass>();
}

bool HIRLoopDistributionLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  return HIRLoopDistribution(
             getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
             getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
             getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR(),
             getAnalysis<HIRSparseArrayReductionAnalysisWrapperPass>()
                 .getHSAR(),
             getAnalysis<HIRLoopResourceWrapperPass>().getHLR(), DistCostModel)
      .run();
}
