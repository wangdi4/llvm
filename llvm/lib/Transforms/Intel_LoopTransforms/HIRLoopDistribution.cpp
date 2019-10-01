//===----- HIRLoopDistribution.cpp - Distribution of HIR loops  -----------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#define LLVM_DEBUG_DDG(X) DEBUG_WITH_TYPE("hir-loop-distribute-ddg", X)

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::distribute;

cl::opt<bool> DisableDist("disable-hir-loop-distribute",
                          cl::desc("Disable HIR Loop Distribution"), cl::Hidden,
                          cl::init(false));

cl::opt<unsigned> MaxMemResourceToDistribute(
    "hir-loop-distribute-max-mem",
    cl::desc("Number of memory references to be placed into new distributed "
             "loop chunks"),
    cl::Hidden, cl::init(20));

cl::opt<unsigned> ScalarExpansionCost(
    "hir-loop-distribute-scex-cost",
    cl::desc(
        "Number of mem operations in loop when to enable scalar expansion."),
    cl::Hidden, cl::init(20));

enum PragmaReturnCode {
  NotProcessed,
  NoDistribution,
  Success,
  UnsupportedStmts,
  TooComplex,
  TooManyDistributePoints,
  Last
};

const char *OptReportMsg[Last] = {
    "Distribute point pragma not processed",
    "No Distribution as requested by pragma",
    "Distribute point pragma processed",
    "Distribute point pragma not processed: Unsupported constructs in loops",
    "Distribute point pragma not processed: Loop is too complex",
    "Distribute point pragma not processed: Too many Distribute points"};

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
  LoopOptReportBuilder &LORBuilder = HIRF.getLORBuilder();

  for (auto I = Loops.begin(), E = Loops.end(); I != E; ++I) {
    HLLoop *Lp = *I;
    LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION: <" << Lp->getNumber() << ">\n");

    if (!loopIsCandidate(Lp)) {
      LLVM_DEBUG(
          dbgs() << "LOOP DISTRIBUTION: Loop is not candidate with current "
                    "heuristics \n");
      continue;
    }

    LoopHasDistributePoint = false;
    if (Lp->hasDistributePoint()) {
      LoopHasDistributePoint = true;
      unsigned RC = distributeLoopForDirective(Lp);
      if (RC != NotProcessed) {
        LORBuilder(*Lp).addRemark(OptReportVerbosity::Low, OptReportMsg[RC]);
      }
      continue;
    }

    unsigned TotalMemOps = 0;
    bool ForceCycleForLoopIndepDep = true;
    bool CreateControlNodes = false;

    if (DistCostModel == DistHeuristics::BreakMemRec) {
      TotalMemOps = HLR.getSelfLoopResource(Lp).getNumIntMemOps() +
                    HLR.getSelfLoopResource(Lp).getNumFPMemOps();

      if (TotalMemOps >= ScalarExpansionCost) {
        ForceCycleForLoopIndepDep = false;
      }

      LLVM_DEBUG(dbgs() << "[Distribution] Loop has " << TotalMemOps
                        << " memory operations which makes it "
                        << (ForceCycleForLoopIndepDep ? "non-" : "")
                        << "profitable for scalar expansion\n");

      CreateControlNodes = true;
    }

    // Sparse array reduction info is needed to create the DistPPGraph
    // and in findDistPoints while breaking the PiBlock Recurrences.
    SARA.computeSparseArrayReductionChains(Lp);

    std::unique_ptr<PiGraph> PG(new PiGraph(
        Lp, DDA, SARA, ForceCycleForLoopIndepDep, CreateControlNodes));

    if (!PG->isGraphValid()) {
      LLVM_DEBUG(
          dbgs() << "LOOP DISTRIBUTION: Distribution for loop failed due to "
                 << PG->getFailureReason() << "\n");
      continue;
    }

    if (PG->hasControlDependences() && Lp->isStripmineRequired(StripmineSize) &&
        !Lp->canStripmine(StripmineSize)) {
      // Assume stripmine is required
      LLVM_DEBUG(dbgs() << "Stripmine is not possible but assumed to be "
                           "required for loops with control dependencies\n");
      continue;
    }

    LLVM_DEBUG_DDG(dbgs() << "DDG dump:\n");
    LLVM_DEBUG_DDG(DDA.getGraph(Lp).dump());

    LLVM_DEBUG(dbgs() << "\nPiGraph dump:\n");
    LLVM_DEBUG(PG->dump());
    LLVM_DEBUG(dbgs() << "\n");

    // Single piblock graph isn't worth considering
    if (PG->size() < 2) {
      // TODO might still be able to scalar expand though...
      LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION:  too many dependences\n");

      if (DistCostModel != DistHeuristics::BreakMemRec) {
        continue;
      }
    }

    SmallVector<PiBlockList, 8> NewOrdering;
    findDistPoints(Lp, PG, NewOrdering);

    if (NewOrdering.size() > 1 && NewOrdering.size() < MaxDistributedLoop) {
      SmallVector<HLDDNodeList, 8> DistributedLoops;
      processPiBlocksToHLNodes(PG, NewOrdering, DistributedLoops);
      distributeLoop(Lp, DistributedLoops, false, LORBuilder);
      Modified = true;
    } else {
      LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION: "
                        << "Found no valid distribution points"
                        << "\n");
    }
  }

  return Modified;
}

void HIRLoopDistribution::processPiBlocksToHLNodes(
    const std::unique_ptr<PiGraph> &PGraph,
    ArrayRef<PiBlockList> GroupsOfPiBlocks,
    SmallVectorImpl<HLDDNodeList> &DistributedLoops) {

  // Maps (Original control statement, PiBlock list) -> Original or cloned HLIf.
  SmallDenseMap<std::pair<HLIf *, const PiBlockList *>, HLIf *> ControlGuards;

  unsigned LoopNum = 0;
  for (auto &PList : GroupsOfPiBlocks) {
    HLDDNodeList &CurLoopHLDDNodeList = DistributedLoops.emplace_back();

    // Combine PiBlocks within single ordering group.
    SmallVector<DistPPNode *, 32> MergedPiBlock;
    for (auto *PiBlock : PList) {
      for (auto *PPNode :
           make_range(PiBlock->dist_node_begin(), PiBlock->dist_node_end())) {
        MergedPiBlock.push_back(PPNode);
      }
    }

    std::sort(MergedPiBlock.begin(), MergedPiBlock.end(),
              [](DistPPNode *A, DistPPNode *B) {
                return A->getNode()->getTopSortNum() <
                       B->getNode()->getTopSortNum();
              });

    for (auto *PPNode : MergedPiBlock) {
      HLNode *Node = PPNode->getNode();

      // Set the existing control node for the PList.
      if (PPNode->isControlNode()) {
        HLIf *ControlNode = cast<HLIf>(Node);
        ControlGuards[{ControlNode, &PList}] = ControlNode;
      }

      auto ControlDep = PGraph->getControlDependence(PPNode);
      if (!ControlDep) {
        CurLoopHLDDNodeList.push_back(cast<HLDDNode>(Node));
        continue;
      }

      HLIf *OrigControlNode = cast<HLIf>(ControlDep->first->getNode());
      HLIf *&ControlNode = ControlGuards[{OrigControlNode, &PList}];

      // Check if control node doesn't exist in the current PiBlock.
      if (!ControlNode) {
        ControlNode = OrigControlNode->cloneEmpty();
        // Use {LoopNum, true} to indicate insert or move HLIf to its final
        // place in HIR.
        DistDirectiveNodeMap[ControlNode] = {LoopNum, true};
        CurLoopHLDDNodeList.push_back(ControlNode);
      }

      if (ControlDep->second) {
        HLNodeUtils::moveAsLastThenChild(ControlNode, Node);
      } else {
        HLNodeUtils::moveAsLastElseChild(ControlNode, Node);
      }
    }

    ++LoopNum;
  }
}

bool HIRLoopDistribution::piEdgeIsRecurrence(const HLLoop *Lp,
                                             const PiGraphEdge &PiEdge) const {
  for (auto &Edge :
       make_range(PiEdge.getDDEdges().begin(), PiEdge.getDDEdges().end())) {
    // TODO: Use Edge->preventsVectorization(Lp->getNestingLevel())
    // Will require to adjust some LIT test.
    if (!Edge->getSrc()->isTerminalRef() &&
        Edge->getDVAtLevel(Lp->getNestingLevel()) & DVKind::LT) {
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

RegDDRef *HIRLoopDistribution::createTempArrayStore(HLLoop *Lp,
                                                    DDRef *TempRef) {

  // Generates  TEMP[i] = tx
  //  tx may be from assignments of this form:
  //  tx = ty   ;  tx = 1000
  //  Make it a self-blob to avoid IR validation error

  HLDDNode *TempRefDDNode = TempRef->getHLDDNode();

  RegDDRef *TempRegRef = dyn_cast<RegDDRef>(TempRef);
  if (!TempRegRef) {
    TempRegRef = HNU.getDDRefUtils().createScalarRegDDRef(
        TempRef->getSymbase(), TempRef->getSingleCanonExpr());
  }

  auto ArrTy = ArrayType::get(TempRef->getDestType(), StripmineSize);

  AllocaBlobIdx = HNU.createAlloca(ArrTy, RegionNode, ".TempArray");

  RegDDRef *TmpArrayRef = HNU.getDDRefUtils().createMemRef(AllocaBlobIdx);

  auto IVType = Lp->getIVType();
  CanonExpr *FirstCE = TempRef->getCanonExprUtils().createCanonExpr(IVType);
  FirstCE->addIV(LoopLevel, 0, 1);

  //  Create constant of 0
  CanonExpr *SecondCE = TempRef->getCanonExprUtils().createCanonExpr(IVType);
  TmpArrayRef->addDimension(SecondCE);
  TmpArrayRef->addDimension(FirstCE);

  TmpArrayRef =
      insertTempArrayStore(Lp, TempRegRef, TmpArrayRef, TempRefDDNode);

  return TmpArrayRef;
}

RegDDRef *HIRLoopDistribution::insertTempArrayStore(HLLoop *Lp,
                                                    RegDDRef *TempRef,
                                                    RegDDRef *TmpArrayRef,
                                                    HLDDNode *TempRefDDNode) {

  RegDDRef *RVal = TempRef->clone();
  HLInst *StoreInst = HNU.createStore(RVal, ".TempSt", TmpArrayRef);
  HLNodeUtils::insertAfter(TempRefDDNode, StoreInst);

  RVal->makeConsistent(TempRef);

  updateLiveInAllocaTemp(Lp, TmpArrayRef->getBasePtrSymbase());
  TempArraySB.push_back(TmpArrayRef->getSymbase());
  return TmpArrayRef;
}

void HIRLoopDistribution::createTempArrayLoad(RegDDRef *TempRef,
                                              RegDDRef *TmpArrayRef,
                                              HLDDNode *Node,
                                              bool TempRedefined) {

  // tx = TEMP[i]
  HLLoop *Lp = Node->getParentLoop();

  const std::string TempName = "scextmp";
  HLInst *LoadInst =
      HNU.createLoad(TmpArrayRef->clone(), TempName, TempRef->clone());

  auto TmpNode = Node;
  HLNode *IfParent;

  if (TempRedefined) {
    HLLoop *Lp = Node->getParentLoop();
    IfParent = cast<HLDDNode>(Lp->getFirstChild());
  } else {
    // if stmt is inside an if, insertion should be done before If
    // because we do insert once per loop
    do {
      IfParent = TmpNode;
      TmpNode = dyn_cast<HLIf>(TmpNode->getParent());
    } while (TmpNode);
  }

  HLNodeUtils::insertBefore(IfParent, LoadInst);
  updateLiveInAllocaTemp(Lp, TmpArrayRef->getBasePtrSymbase());
  TempArraySB.push_back(TmpArrayRef->getSymbase());
}

bool HIRLoopDistribution::isScalarExpansionCandidate(const DDRef *Ref) const {

  // We need to scalar expand scalar global vars besides temps.
  // When Memref cleanup is suppessed for distribute point pragma,
  // Scalar global vars are not turning into temp.
  // Global vars should be scalar expanded also, to avoid loss of
  // functionality

  const RegDDRef *RegRef = dyn_cast<RegDDRef>(Ref);
  bool IsMemRef;

  if (!RegRef) {
    return true;
  }

  if ((IsMemRef = RegRef->isMemRef()) && LoopHasDistributePoint &&
      RegRef->getNumDimensions() == 1) {
    auto BaseCE = RegRef->getBaseCE();
    return !BaseCE->isNonLinear() && RegRef->getDimensionIndex(1)->isZero();
  }

  return !IsMemRef;
}

void HIRLoopDistribution::replaceWithArrayTemp(Gatherer::VectorTy *Refs) {

  // Map for temp symbase and its Array Temp
  SmallDenseMap<unsigned, RegDDRef *, 16> ArrayTempMap;
  // <Symbase, Loop number>
  SmallVector<std::pair<unsigned, unsigned>, 8> InsertLoadVector;

  for (unsigned I = 0; I < LastLoopNum - 1; ++I) {
    for (DDRef *SrcRef : Refs[I]) {
      RegDDRef *TmpArrayRef = nullptr;
      if (SrcRef->isRval()) {
        continue;
      }
      if (!isScalarExpansionCandidate(SrcRef)) {
        continue;
      }
      const HLInst *Inst = cast<HLInst>(SrcRef->getHLDDNode());
      // No need to check for safe reduction because reduction temps
      // Will not be distributed into two different loops
      if (Inst->isInPreheaderOrPostexit()) {
        continue;
      }

      RegDDRef *SrcRegRef = cast<RegDDRef>(SrcRef);
      HLLoop *Lp = SrcRef->getParentLoop();

      for (unsigned J = I + 1; J < LastLoopNum; ++J) {
        bool TempRedefined = false;
        for (DDRef *SinkRef : Refs[J]) {

          // For Pragma, we support global scalar (*tx) to be scalar
          // expanded. In case *tx, *ty are mapped to same Symbase,
          // they will be treated as different

          if (SrcRef->getSymbase() != SinkRef->getSymbase() ||
              (SrcRegRef->isMemRef() &&
               (!DDRefUtils::areEqual(SrcRef, SinkRef)))) {
            continue;
          }

          if (!isScalarExpansionCandidate(SinkRef)) {
            TempRedefined = true;
            continue;
          }

          // Create TEMP[i] = tx and insert
          unsigned SB = SrcRef->getSymbase();

          if (!TmpArrayRef) {
            auto Iter = ArrayTempMap.find(SB);
            // If ArrayTemp is created, just clone and insert
            if (Iter != ArrayTempMap.end()) {
              TmpArrayRef = Iter->second;
              TmpArrayRef =
                  insertTempArrayStore(Lp, SrcRegRef, TmpArrayRef->clone(),
                                       SrcRegRef->getHLDDNode());

            } else {
              TmpArrayRef = createTempArrayStore(Lp, SrcRef);
              ArrayTempMap[SB] = TmpArrayRef;
            }
          }

          // Create tx = TEMP[i] and insert.  Cannot do direct replacement
          //  because Copy Inst does not allow Memref
          //  CrateTempArrayLoad will find the right place to insert before
          //  the if stmt or at loop entry.
          //  Loading is needed once per loop
          if (std::find(InsertLoadVector.begin(), InsertLoadVector.end(),
                        std::make_pair(SB, J)) == InsertLoadVector.end()) {
            assert(TmpArrayRef && "Temp Store missing");
            RegDDRef *SinkTempRef = dyn_cast<RegDDRef>(SinkRef);
            if (!SinkTempRef) {
              SinkTempRef = HNU.getDDRefUtils().createScalarRegDDRef(
                  SinkRef->getSymbase(), SinkRef->getSingleCanonExpr());
            }
            createTempArrayLoad(SinkTempRef, TmpArrayRef,
                                SinkRef->getHLDDNode(), TempRedefined);
            InsertLoadVector.emplace_back(SB, J);
          }
        }
      }
    }
  }
}

bool HIRLoopDistribution::arrayTempExceeded(unsigned LastLoopNum,
                                            unsigned &NumArrayTemps,
                                            Gatherer::VectorTy *Refs) {

  NumArrayTemps = 0;
  if (DistCostModel != DistHeuristics::BreakMemRec) {
    return false;
  }

  for (unsigned I = 0; I < LastLoopNum - 1; ++I) {
    for (const DDRef *Ref : Refs[I]) {

      if (Ref->isRval()) {
        continue;
      }
      if (!isScalarExpansionCandidate(Ref)) {
        continue;
      }
      const HLInst *Inst = cast<HLInst>(Ref->getHLDDNode());
      if (Inst->isInPreheaderOrPostexit()) {
        continue;
      }
      // Check any usage in another loop
      bool Done = false;
      for (unsigned J = I + 1; J < LastLoopNum && !Done; ++J) {
        for (const DDRef *SinkRef : Refs[J]) {
          if (Ref->isLval() && !isScalarExpansionCandidate(SinkRef)) {
            continue;
          }
          if (Ref->getSymbase() == SinkRef->getSymbase()) {
            if (++NumArrayTemps >= MaxArrayTempsAllowed) {
              LLVM_DEBUG(dbgs() << "Loop Dist bail out because #of Array temps "
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

void HIRLoopDistribution::distributeLoop(
    HLLoop *Loop, SmallVectorImpl<HLDDNodeList> &DistributedLoops,
    bool ForDirective, LoopOptReportBuilder &LORBuilder) {
  assert(DistributedLoops.size() < MaxDistributedLoop &&
         "Number of distributed chunks exceed threshold. Expected the caller "
         "to check before calling this function.");

  invalidateLoop(Loop);

  LastLoopNum = DistributedLoops.size();
  assert(LastLoopNum > 1 && "Invalid loop distribution");
  LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION : " << LastLoopNum
                    << " way distributed\n");

  TempArraySB.clear();
  bool CopyPreHeader = true;
  HLLoop *LoopNode;

  RegionNode = Loop->getParentRegion();
  LoopLevel = Loop->getNestingLevel();

  // Gather DDRefs into an array per loop
  Gatherer::VectorTy Refs[MaxDistributedLoop];
  unsigned I = 0;
  for (HLDDNodeList &HLNodeList : DistributedLoops) {
    for (HLDDNode *Node : HLNodeList) {
      Gatherer::gather(Node, Refs[I]);
    }
    I++;
  }

  // Find number of Scalar Temps.
  // Large number of extra memory refs will affect performance
  // TODO: Do not proceed if threshold exceeds
  arrayTempExceeded(LastLoopNum, NumArrayTemps, Refs);

  bool StripmineRequired = Loop->isStripmineRequired(StripmineSize);
  if (NumArrayTemps && StripmineRequired &&
      !Loop->canStripmine(StripmineSize)) {
    // It's fine to bail out for loops without control flow dependencies.
    // Loops with such dependencies do preliminary checks.
    return;
  }

  unsigned Num = 0;
  I = 0;
  for (HLDDNodeList &HLNodeList : DistributedLoops) {
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
      if (ForDirective) {
        LORBuilder(*LoopNode).addRemark(OptReportVerbosity::Low,
                                        OptReportMsg[Success]);
      }
      LORBuilder(*LoopNode).addRemark(OptReportVerbosity::Low,
                                      "Loop distributed (%d way)", LastLoopNum);
    }

    if (++Num == LastLoopNum) {
      HLNodeUtils::moveAsFirstPostexitNodes(LoopNode, Loop->post_begin(),
                                            Loop->post_end());
    }
    for (HLDDNode *Node : HLNodeList) {
      if (DistDirectiveNodeMap[Node].second) {
        HLNodeUtils::insertAsLastChild(LoopNode, Node);
      } else {
        HLNodeUtils::moveAsLastChild(LoopNode, Node);
      }
    }

    LORBuilder(*LoopNode).addOrigin("Distributed chunk %d", Num);
  }

  // The loop is now empty, all its children are moved to new loops
  // except for user pragma, where some of the IF needs to be cloned

  assert((!Loop->hasChildren() || Loop->hasDistributePoint()) &&
         "Loop Distribution failed to account for all Loop Children");

  for (unsigned I = 0; I < LastLoopNum; ++I) {
    // Distributed flag is used by Loop Fusion to skip loops that are
    // distributed.  Need to set for Memory related distribution only.
    // Distributed loops for enabling perfect loop nest, can still be fused
    // after interchange is done
    if (DistCostModel == DistHeuristics::BreakMemRec) {
      NewLoops[I]->setDistributedForMemRec();
    }
    HLNodeUtils::insertBefore(Loop, NewLoops[I]);
    HLNodeUtils::removeEmptyNodes(NewLoops[I], false);
  }
  if (NumArrayTemps) {
    replaceWithArrayTemp(Refs);

    // For constant trip count <= StripmineSize, no stripmine is done
    if (StripmineRequired) {
      HIRTransformUtils::stripmine(NewLoops[0], NewLoops[LastLoopNum - 1],
                                   StripmineSize);
      // Fix TempArray index if stripmine is peformed: 64 * i1 + i2 => i2
      fixTempArrayCoeff(NewLoops[0]->getParentLoop());
    }
  }

  HLNodeUtils::remove(Loop);

  // Distribution for perfect loopnest is not profitable by itself, it is only
  // used for enabling other transformations so we should not mark region as
  // modified.
  if (DistCostModel != DistHeuristics::NestFormation) {
    RegionNode->setGenCode();
  }
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
            dyn_cast<HLLoop>((*(Blk->dist_node_begin()))->getNode());
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
            dyn_cast<HLLoop>((*(Blk->dist_node_begin()))->getNode());
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

  if (Lp->hasUnrollEnablingPragma() || Lp->hasVectorizeEnablingPragma() ||
      !Lp->isDo()) {
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
  // Walk through topsorted nodes of Pigraph, keeping in mind the fact that each
  // of those nodes can legally form its own loop if the loops(distributed
  // chunks) are ordered in same topsort order of nodes.
  // Add the node to current loop. Look for outgoing edges that indicate
  // recurrences. If none, continue on. Otherwise terminate the current loop,
  // start a new one. Src and sink of recurrence will be in different loops,
  // breaking the recurrence.
  unsigned NumRefCounter = 0;
  SmallVector<unsigned, 12> MemRefSBVector;

  bool UserCallSeen = false;
  bool PrevUserCallSeen = false;

  SRA.computeSafeReductionChains(Lp);
  bool HasSparseArrayReductions = SARA.getNumSparseArrayReductionChains(Lp) > 0;
  PiBlockList SparseReductionBlocks;

  auto CommitCurrentBlockList = [&]() {
    DistPoints.push_back(CurLoopPiBlkList);
    CurLoopPiBlkList.clear();
    NumRefCounter = 0;
  };

  // Get number of loads/stores, needed to decide if threshold is exceeded.
  // Arrays with same SB, in general, have locality, and do not need to be
  // added twice.  Will need some fine tuning later

  for (auto N = PGraph->node_begin(), E = PGraph->node_end(); N != E; ++N) {
    PrevUserCallSeen = UserCallSeen;
    UserCallSeen = false;

    PiBlock *SrcBlk = *N;

    // If this current block has sparse array reduction instructions,
    // we need to break the recurrence before this block so that sparse array
    // reductions can be distributed into another separate loop.
    if (HasSparseArrayReductions && !CurLoopPiBlkList.empty() &&
        containsSparseArrayReductions(SrcBlk, SARA)) {

      // If there is no outgoing dependencies from sparse array reduction block,
      // we can combine such blocks at the end.
      bool NoOutgoingDeps = llvm::empty(PGraph->outgoing(SrcBlk));
      if (NoOutgoingDeps) {
        SparseReductionBlocks.push_back(SrcBlk);
        continue;
      }
    }

    for (auto NodeI = SrcBlk->nodes_begin(), E = SrcBlk->nodes_end();
         NodeI != E; ++NodeI) {
      HLDDNode *Node = cast<HLDDNode>(*NodeI);

      // Split blocks with user calls.
      if (HLInst *Inst = dyn_cast<HLInst>(Node)) {
        if (Inst->isCallInst() && !Inst->getIntrinCall()) {
          UserCallSeen = true;
        }
      }

      for (auto RefIt = Node->ddref_begin(), E = Node->ddref_end(); RefIt != E;
           ++RefIt) {
        const RegDDRef *Ref = *RefIt;
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

    if (!CurLoopPiBlkList.empty() && PrevUserCallSeen && !UserCallSeen) {
      CommitCurrentBlockList();
    }

    CurLoopPiBlkList.push_back(SrcBlk);

    if (NumRefCounter >= MaxMemResourceToDistribute) {
      CommitCurrentBlockList();
      continue;
    }

    for (auto *Edge : PGraph->outgoing(SrcBlk)) {
      // TODO this is overly aggressive for at least two reasons.
      // Case1: 3 block graph with 2 edges,
      // 1->3, 2->3. This would create 3 loops, but 1 and 2 could have been
      // combined. OTOH, if piblock 1 would form a non vectorizable loop and
      // 2 doesnt, we would want them separate.
      // Case2: 2 block graph, with single recurrence edge between the two
      // Once split, the two loops are non vectorizable. If legality was
      // the only concern, we can iterate over all dd edges indirectly by
      // looking at distppedges whos src/sink are in piblock
      if (piEdgeIsRecurrence(Lp, *Edge)) {
        CommitCurrentBlockList();
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

  if (!SparseReductionBlocks.empty()) {
    DistPoints.push_back(SparseReductionBlocks);
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

unsigned HIRLoopDistribution::distributeLoopForDirective(HLLoop *Lp) {

  //  Process user pragma Loop Directive for distribution
  //  - No data dependency checking is needed
  //  - Multiple distribution points are allowed
  //  - User can put the pragma inside nested If statements, which is not
  //    handled in automatic  distribution
  //  IfNodeMap stores lowest and highest Loop Num

  if (DistCostModel != DistHeuristics::BreakMemRec || !Lp->isInnermost()) {
    return NotProcessed;
  }

  if (!Lp->canStripmine(StripmineSize)) {
    return TooComplex;
  }

  HLDDNode *Node = cast<HLDDNode>(Lp->getFirstChild());
  // Distribute point placed right before first stmt implies
  // no distribution
  if (Node && Node->isDistributePoint()) {
    Node->setDistributePoint(false);
    return NoDistribution;
  }

  // - Mark each HLNode with the Loop Num
  // - For if stmt - identify lowest and highest loop Num
  // - For each Node in HLNNode, walk top down
  //   - if loopNum != current new LoopNum
  //      push HLDDNodeList in DistributedLoops
  //   - if not IF stmt or current loopnum == lowest and highest loopnum
  //         -- save Node in HLDDNodeList because it can be moved as a nest
  //         -- continue
  //   (If stmt from there on)
  //   - Clone empty IF stmt
  //   - Walk all stmts in IF nest,
  //     - if loopNum in stmt matches current loop
  //         attach HLNode to new IF depending on T/F path

  DistDirectiveNodeMap.clear();
  IfNodeMap.clear();

  HLDDNodeList CurLoopHLDDNodeList;
  HLDDNode *TopIfNode = nullptr;
  unsigned TopIfNodeLoopNum = 0;
  unsigned DistLoopNum = 1;
  PragmaReturnCode UnsupportedRC = Success;

  ForEach<HLNode>::visitRange(
      Lp->child_begin(), Lp->child_end(),
      [this, &DistLoopNum, &TopIfNode, &TopIfNodeLoopNum,
       &UnsupportedRC](HLNode *Node) {
        HLDDNode *HNode = dyn_cast<HLDDNode>(Node);

        if (!HNode || isa<HLSwitch>(HNode)) {
          UnsupportedRC = UnsupportedStmts;
          return;
        }

        if (HNode->isDistributePoint()) {
          HNode->setDistributePoint(false);
          DistLoopNum++;
          if (DistLoopNum >= MaxDistributedLoop) {
            UnsupportedRC = TooManyDistributePoints;
            return;
          }
        }
        // 2nd argument indicates insert/move (for cloned/existing)
        DistDirectiveNodeMap[HNode] = std::make_pair(DistLoopNum, false);
        if (!(isa<HLIf>(HNode->getParent()))) {
          if (TopIfNode) {
            IfNodeMap[TopIfNode] =
                std::make_pair(TopIfNodeLoopNum, DistLoopNum);
          }
          if (isa<HLIf>(HNode)) {
            TopIfNode = HNode;
            TopIfNodeLoopNum = DistLoopNum;
          } else {
            TopIfNode = nullptr;
          }
        }
      });

  if (UnsupportedRC != Success) {
    return UnsupportedRC;
  }
  if (TopIfNode) {
    // TopIfNode is live out
    IfNodeMap[TopIfNode] = std::make_pair(TopIfNodeLoopNum, DistLoopNum);
  }

  SmallVector<HLDDNodeList, 8> DistributedLoops;
  collectHNodesForDirective(Lp, DistributedLoops, CurLoopHLDDNodeList);
  distributeLoop(Lp, DistributedLoops, true, HIRF.getLORBuilder());
  return Success;
}

void HIRLoopDistribution::collectHNodesForDirective(
    HLLoop *Lp, SmallVectorImpl<HLDDNodeList> &DistributedLoops,
    HLDDNodeList &CurLoopHLDDNodeList) {

  unsigned DistLoopNum = 1;

  for (auto NodeIt = Lp->child_begin(), E = Lp->child_end(); NodeIt != E;
       ++NodeIt) {
    HLDDNode *HNode = cast<HLDDNode>(&*NodeIt);
    if (DistLoopNum != DistDirectiveNodeMap[HNode].first) {
      DistributedLoops.push_back(CurLoopHLDDNodeList);
      CurLoopHLDDNodeList.clear();
      DistLoopNum++;
    }

    if ((!isa<HLIf>(HNode) &&
         DistLoopNum == DistDirectiveNodeMap[HNode].first) ||
        (isa<HLIf>(HNode) && DistLoopNum == IfNodeMap[HNode].first &&
         DistLoopNum == IfNodeMap[HNode].second)) {
      CurLoopHLDDNodeList.push_back(HNode);
      continue;
    }

    if (isa<HLIf>(HNode)) {
      for (unsigned LoopNum = IfNodeMap[HNode].first;
           LoopNum <= IfNodeMap[HNode].second; ++LoopNum) {
        HLDDNode *NewIf =
            processPragmaForIf(HNode, HNode, CurLoopHLDDNodeList, LoopNum);
        if (NewIf) {
          CurLoopHLDDNodeList.push_back(NewIf);
        }
        if (LoopNum != IfNodeMap[HNode].second) {
          DistributedLoops.push_back(CurLoopHLDDNodeList);
          CurLoopHLDDNodeList.clear();
        }
        DistLoopNum = LoopNum;
      }
    }
  }

  if (CurLoopHLDDNodeList.size()) {
    DistributedLoops.push_back(CurLoopHLDDNodeList);
  }
}

void HIRLoopDistribution::moveIfChildren(HLContainerTy::iterator Begin,
                                         HLContainerTy::iterator End,
                                         HLIf *NewHLIf, HLDDNode *TopIfHNode,
                                         HLDDNodeList &CurLoopHLDDNodeList,
                                         unsigned TopIfLoopNum,
                                         bool IsThenChild) {

  unsigned NodeLoopNum = 0;
  HLDDNode *NewHLIfChild = nullptr;

  for (auto Iter = Begin; Iter != End;) {

    HLDDNode *Node = cast<HLDDNode>(&*Iter);

    NodeLoopNum = DistDirectiveNodeMap[Node].first;
    // Iter needs to be bumped up here because Node is changed
    ++Iter;
    if (isa<HLIf>(Node)) {
      NewHLIfChild = processPragmaForIf(TopIfHNode, Node, CurLoopHLDDNodeList,
                                        TopIfLoopNum);
      if (NewHLIfChild) {
        IsThenChild ? HLNodeUtils::insertAsLastThenChild(NewHLIf, NewHLIfChild)
                    : HLNodeUtils::insertAsLastElseChild(NewHLIf, NewHLIfChild);
      }
    } else if (NodeLoopNum == TopIfLoopNum) {
      IsThenChild ? HLNodeUtils::moveAsLastThenChild(NewHLIf, Node)
                  : HLNodeUtils::moveAsLastElseChild(NewHLIf, Node);
    }
  }
}

HLDDNode *HIRLoopDistribution::processPragmaForIf(
    HLDDNode *TopIfHNode, HLDDNode *CurrentIfHNode,
    HLDDNodeList &CurLoopHLDDNodeList, unsigned TopIfLoopNum) {

  HLNode *Node = CurrentIfHNode;
  HLIf *IfParent = cast<HLIf>(Node);
  HLIf *NewHLIf = IfParent->cloneEmpty();

  if (TopIfHNode == CurrentIfHNode) {
    DistDirectiveNodeMap[NewHLIf] = std::make_pair(TopIfLoopNum, true);
  }

  moveIfChildren(IfParent->then_begin(), IfParent->then_end(), NewHLIf,
                 TopIfHNode, CurLoopHLDDNodeList, TopIfLoopNum, true);
  moveIfChildren(IfParent->else_begin(), IfParent->else_end(), NewHLIf,
                 TopIfHNode, CurLoopHLDDNodeList, TopIfLoopNum, false);

  // For nested If statements, pragma can be inserted anywhere and
  // ends up with empty If.

  if (!NewHLIf->hasThenChildren() && !NewHLIf->hasElseChildren()) {
    NewHLIf = nullptr;
  }
  return NewHLIf;
}

void HIRLoopDistribution::invalidateLoop(loopopt::HLLoop *Loop) const {
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
      Loop);
  HIRInvalidationUtils::invalidateBody(Loop);
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
