//===- HIRLoopFusion.cpp - Implements Loop Fusion transformation ----------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// The files HIRLoopFusion.cpp, HIRLoopFusionGraph.h and HIRLoopFusionGraph.cpp
// contain an implementation of the "Fast Greedy Weighted Fusion" algorithm by
// Ken Kennedy, International Journal of Parallel Programming, Vol. 29, No. 5,
// October 2001.
//
// It can be thought of as proceeding in stages:
//
// 1. Create FuseNode for each loop and for each standalone statement in a
// region of nodes. Create FuseEdges along forward dependencies between every
// FuseNode. Setup initial cost for fuse edges and possibility for the fusion.
// Compute input edges.
//
// 2. Compute initial successor, predecessor, and neighbor sets. This can be
// implemented in O(V+E) time.
//
// 3. Process the vertices in V to compute for each vertex the set pathFrom[v],
// which contains all vertices that can be reached by a path from vertex v, and
// the set badPathFrom[v], a subset of pathFrom[v] that includes the set of
// vertices that can be reached from v by a path that contains a bad vertex.
// This phase can be done in time O(Ed+V) set operations, where Ed is a number
// of directed edges. Each set operation takes O(V) time.
//
// 4. Invert the sets pathFrom and badPathFrom respectively to produce the sets
// pathTo[v] and badPathTo[v] for each vertex v in the graph. The set pathTo[v]
// contains the vertices from which there is a path to v; the set badPathTo[v]
// contains the vertices from which v can be reached via a bad path. Inversion
// can be done in O(V^2) total time by simply iterating over pathFrom(v) for
// each v - adding v to pathTo[w] for every w in pathFrom(v).
//
// 5. Insert each of the edges in E = (Ed and Eu) to a priority queue EdgeHeap
// by weight. Eu is a set of undirected edges. Using the priority queue
// implemented as a heap, this takes O(E*lgE) == O(E*lgV) time
//
// 6. While EdgeHeap is non-empty, select and remove the
// heaviest edge (v, w) from it. If w in badPathFrom[v] then do not fuse -
// repeat step 6. Otherwise, do the following:
//
//   a. Collapse (v, w), and every edge on a directed path between them into a
//   single node.
//
//   b. After each collapse of a vertex into v, adjust the sets pathFrom,
//   badPathFrom, pathTo, and badPathTo to reflect the new graph. That is, the
//   composite node will now be reached from every vertex that reaches a vertex
//   in the composite and it will reach any vertex that is reached by a vertex
//   in the composite.
//
//   c. After each vertex collapse, recompute successor, predecessor and
//   neighbor sets for the composite vertex and recompute weights between the
//   composite vertex and other vertices as appropriate.
//
// The entire algorithm takes O(EV+V^2) time.
//
// TODO:
// 1) Merge sibling loops in a region.
// 2) Align iteration spaces in case of fusion preventing dependency.
// 3) Do not fuse chunks from the previous distribution.
// 4) Use locality information for the cost modeling.
// 5) For compile time, do aggressive fusion (try to fuse loops in a lexical
//    order, give up if loops may not be fused, proceed to the next loop).
// 6) Make liveout update more precise.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopFusion.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRLoopFusionGraph.h"

#define OPT_SWITCH "hir-loop-fusion"
#define OPT_DESC "HIR Loop Fusion"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::fusion;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

STATISTIC(FusedCount, "Number of HIR loops fused");

namespace {

class HIRLoopFusion {
  class LoopVisitor;

  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  HIRLoopStatistics &HLS;

  template <bool PreLoop>
  bool generatePreOrPostLoops(HLNode *AnchorNode,
                              const SmallVectorImpl<unsigned> &Indices,
                              const SmallVectorImpl<int64_t> &Bounds,
                              const SmallVectorImpl<HLLoop *> &Candidates,
                              SmallDenseSet<unsigned> &IndexSet);

  HLLoop *fuseLoops(const SmallVectorImpl<HLLoop *> &Candidates);

  void sortHLNodes(const FuseGraph &FG);

  void runOnNodeRange(HLNode *ParentNode, HLNodeRangeTy Range);

public:
  HIRLoopFusion(HIRFramework &HIRF, HIRDDAnalysis &DDA, HIRLoopStatistics &HLS)
      : HIRF(HIRF), DDA(DDA), HLS(HLS) {}

  bool run();
};
} // namespace

template <typename LiveInIterTy, typename LiveOutIterTy>
static void setLiveRangeInfo(HLLoop *DstLoop, LiveInIterTy LiveInBegin,
                             LiveInIterTy LiveInEnd, LiveOutIterTy LiveOutBegin,
                             LiveOutIterTy LiveOutEnd) {
  for (auto LiveIn : make_range(LiveInBegin, LiveInEnd)) {
    DstLoop->addLiveInTemp(LiveIn);
  }

  for (auto LiveOut : make_range(LiveOutBegin, LiveOutEnd)) {
    DstLoop->addLiveOutTemp(LiveOut);
  }
}

static void copyLiveRangeInfo(HLLoop *DstLoop, const HLLoop *SrcLoop) {
  setLiveRangeInfo(DstLoop, SrcLoop->live_in_begin(), SrcLoop->live_in_end(),
                   SrcLoop->live_out_begin(), SrcLoop->live_out_end());
}

static void moveMergeZtt(HLLoop *DstLoop, HLLoop *SrcLoop) {
  SmallVector<PredicateTuple, 8> ZTTs;
  HIRTransformUtils::cloneOrRemoveZttPredicates(SrcLoop, ZTTs, false);
  HIRTransformUtils::mergeZtt(DstLoop, ZTTs);
}

static void scavengeLoopParts(const SmallVectorImpl<HLLoop *> &Candidates,
                              const SmallDenseSet<unsigned> &IndexSet,
                              SmallVectorImpl<PredicateTuple> &ZTTs,
                              HLContainerTy &Preheader, HLContainerTy &Nodes,
                              HLContainerTy &Postexit,
                              SmallVectorImpl<unsigned> &LiveIns,
                              SmallVectorImpl<unsigned> &LiveOuts) {
  for (unsigned J = 0, E = Candidates.size(); J < E; ++J) {
    if (IndexSet.count(J)) {
      HLLoop *LoopJ = Candidates[J];

      // Handle ZTT
      HIRTransformUtils::cloneOrRemoveZttPredicates(LoopJ, ZTTs, true);

      // Handle preheader
      HLNodeUtils::remove(&Preheader, LoopJ->pre_begin(), LoopJ->pre_end());

      // Handle body
      HLNodeUtils::cloneSequence(&Nodes, LoopJ->getFirstChild(),
                                 LoopJ->getLastChild());

      // Handle postexit
      HLNodeUtils::remove(&Postexit, LoopJ->post_begin(), LoopJ->post_end());

      // Handle live-ins and live-outs
      LiveIns.append(LoopJ->live_in_begin(), LoopJ->live_in_end());
      LiveOuts.append(LoopJ->live_out_begin(), LoopJ->live_out_end());
    }
  }
}

template <bool PreLoop>
bool HIRLoopFusion::generatePreOrPostLoops(
    HLNode *AnchorNode, const SmallVectorImpl<unsigned> &Indices,
    const SmallVectorImpl<int64_t> &Bounds,
    const SmallVectorImpl<HLLoop *> &Candidates,
    SmallDenseSet<unsigned> &IndexSet) {
  HLLoop *FirstLoop = Candidates.front();

  LoopOptReportBuilder &LORBuilder = HIRF.getLORBuilder();
  HLLoop *LastPostLoop = nullptr;

  auto CreateLoop = [&LORBuilder, FirstLoop](RegDDRef *LowerDDRef,
                                             RegDDRef *UpperDDRef) {
    HLLoop *NewLoop = FirstLoop->cloneEmpty();

    // No pragma trip count metadata for a peeled loop
    NewLoop->removeLoopMetadata("llvm.loop.intel.loopcount_maximum");
    NewLoop->removeLoopMetadata("llvm.loop.intel.loopcount_minimum");
    NewLoop->removeLoopMetadata("llvm.loop.intel.loopcount_average");

    LORBuilder(*NewLoop).addRemark(OptReportVerbosity::Low,
                                   "Peeled loop after fusion");
    NewLoop->setLowerDDRef(LowerDDRef);
    NewLoop->setUpperDDRef(UpperDDRef);

    return NewLoop;
  };

  bool HasPeeledLoop = false;
  unsigned LastIndex = Indices.front();
  for (unsigned I = 1, E = Indices.size(); I < E; ++I) {
    unsigned ThisIndex = Indices[I];

    if (Bounds[LastIndex] <= Bounds[ThisIndex] - 1) {
      HLLoop *NewLoop = nullptr;

      RegDDRef *NewLowerDDRef = nullptr;
      RegDDRef *NewUpperDDRef = nullptr;

      if (PreLoop) {
        // Generate DO i = X(LastIndex), X(ThisIndex) - 1
        NewLowerDDRef = Candidates[LastIndex]->getLowerDDRef()->clone();
        NewUpperDDRef = Candidates[ThisIndex]->getLowerDDRef()->clone();

        NewUpperDDRef->getSingleCanonExpr()->addConstant(-1, true);
        NewLoop = CreateLoop(NewLowerDDRef, NewUpperDDRef);

        HLNodeUtils::insertBefore(AnchorNode, NewLoop);
      } else {
        // Generate DO i = X(LastIndex) + 1, X(ThisIndex)
        NewLowerDDRef = Candidates[LastIndex]->getUpperDDRef()->clone();
        NewUpperDDRef = Candidates[ThisIndex]->getUpperDDRef()->clone();

        NewLowerDDRef->getSingleCanonExpr()->addConstant(1, true);
        NewLoop = CreateLoop(NewLowerDDRef, NewUpperDDRef);

        HLNodeUtils::insertAfter(AnchorNode, NewLoop);
        AnchorNode = NewLoop;
        LastPostLoop = NewLoop;
      }

      SmallVector<PredicateTuple, 8> ZTTs;
      HLContainerTy Preheader;
      HLContainerTy Nodes;
      HLContainerTy Postexit;
      SmallVector<unsigned, 16> LiveIns;
      SmallVector<unsigned, 16> LiveOuts;

      // Collect loop parts: bodies, ztts and live-in/out info.
      scavengeLoopParts(Candidates, IndexSet, ZTTs, Preheader, Nodes, Postexit,
                        LiveIns, LiveOuts);

      // Apply collected info to the new loop.
      HIRTransformUtils::mergeZtt(NewLoop, ZTTs);
      HLNodeUtils::insertAsFirstPreheaderNodes(NewLoop, &Preheader);
      HLNodeUtils::insertAsFirstChildren(NewLoop, &Nodes);
      HLNodeUtils::insertAsFirstPostexitNodes(NewLoop, &Postexit);
      setLiveRangeInfo(NewLoop, LiveIns.begin(), LiveIns.end(),
                       LiveOuts.begin(), LiveOuts.end());

      NewLoop->normalize();
      HasPeeledLoop = true;

      // Add loop liveouts for every pre-loop and post-loop except the last one.
      if (PreLoop || I < E - 1) {
        HIRTransformUtils::addCloningInducedLiveouts(NewLoop);
      }
    } else {
      // Empty loop
    }

    LastIndex = ThisIndex;

    if (PreLoop) {
      // Each following preloop will include bodies of the previous preloops.
      IndexSet.insert(ThisIndex);
    } else {
      // For the postloops the algorithm reverses the process, generating
      // postloops that has fewer and fewer of the statements contained in the
      // central fused loop.
      IndexSet.erase(ThisIndex);
    }
  }

  if (LastPostLoop) {
    LORBuilder(*FirstLoop).moveSiblingsTo(*LastPostLoop);
  }

  return HasPeeledLoop;
}

// Update the pragma trip count metadata information for the loop after loop
// fusion. Only preserve the trip count metadata for the fused loop if there
// is no peeled loop and the loops have the same pragma trip count metadata
static void
updatePragmaTripCountInfo(HLLoop *FirstLoop,
                          const SmallVectorImpl<HLLoop *> &Candidates,
                          bool HasPeeledLoop) {

  // If there exists peeled loop, pragma trip count metadata will be removed
  if (HasPeeledLoop) {
    FirstLoop->removePragmaBasedMinimumTripCount();
    FirstLoop->removePragmaBasedMaximumTripCount();
    FirstLoop->removePragmaBasedAverageTripCount();
    return;
  }

  SmallDenseSet<unsigned> MaxTripCountSet;
  SmallDenseSet<unsigned> MinTripCountSet;
  SmallDenseSet<unsigned> AvgTripCountSet;

  unsigned MaxTripCount = 0;
  unsigned MinTripCount = 0;
  unsigned AvgTripCount = 0;

  for (unsigned I = 0, E = Candidates.size(); I < E; ++I) {

    if (Candidates[I]->getPragmaBasedMaximumTripCount(MaxTripCount)) {
      MaxTripCountSet.insert(MaxTripCount);
    }

    if (Candidates[I]->getPragmaBasedMinimumTripCount(MinTripCount)) {
      MinTripCountSet.insert(MinTripCount);
    }

    if (Candidates[I]->getPragmaBasedAverageTripCount(AvgTripCount)) {
      AvgTripCountSet.insert(AvgTripCount);
    }
  }

  // Different pragma trip count metadata in the loops will be removed
  if (MaxTripCountSet.size() > 1) {

    FirstLoop->removePragmaBasedMaximumTripCount();

  } else if (MaxTripCountSet.size() == 1) {

    MaxTripCount = *MaxTripCountSet.begin();
    FirstLoop->setPragmaBasedMaximumTripCount(MaxTripCount);
  }

  if (MinTripCountSet.size() > 1) {

    FirstLoop->removePragmaBasedMinimumTripCount();

  } else if (MinTripCountSet.size() == 1) {

    MinTripCount = *MinTripCountSet.begin();
    FirstLoop->setPragmaBasedMinimumTripCount(MinTripCount);
  }

  if (AvgTripCountSet.size() > 1) {

    FirstLoop->removePragmaBasedAverageTripCount();

  } else if (AvgTripCountSet.size() == 1) {

    AvgTripCount = *AvgTripCountSet.begin();
    FirstLoop->setPragmaBasedAverageTripCount(AvgTripCount);
  }
}

HLLoop *HIRLoopFusion::fuseLoops(const SmallVectorImpl<HLLoop *> &Candidates) {
  // The crucial problem of the fuseLoops mechanics that we must address
  // candidate loop mismatches in the upper and lower bounds. The basic idea is
  // to sort the lower bounds and upper bounds into a non-decreasing sequences
  // IndexLower and IndexUpper. Then output three groups of loops:
  // 1. A sequence of loops with lower bounds L(1), L(2), ..., L(n-1) and upper
  //    bounds L(2)-1, L(3)-1, L(n)-1, such that body of the output loop with
  //    upper bound L(k-1) has the bodies of the input loops with lower bounds
  //    L(1), L(2), ..., L(k-1).
  // 2. The central fused loop
  // 3. A sequence of loops with lower bounds H(1)+1, H(2)+1, ..., H(n-1) and
  //    upper bounds H(2), H(3), H(n), such that body of the output loop with
  //    upper bound Hk has the bodies of the input loops with upper bounds
  //    H(k), H(k+1), ..., H(n).
  //
  //    L1:    | -1  ------------------------  998 |
  //    L2:      | 1  -----------------------  998 |
  //    L3:      | 1  -------------------------  1000 |
  //
  FusedCount += Candidates.size();

  SmallVector<unsigned, 8> IndexLower(Candidates.size());
  SmallVector<unsigned, 8> IndexUpper(Candidates.size());
  SmallVector<int64_t, 8> LowerConst(Candidates.size());
  SmallVector<int64_t, 8> UpperConst(Candidates.size());

  for (unsigned I = 0, E = Candidates.size(); I < E; ++I) {
    IndexLower[I] = I;
    IndexUpper[I] = I;

    LowerConst[I] = Candidates[I]->getLowerCanonExpr()->getConstant();
    UpperConst[I] = Candidates[I]->getUpperCanonExpr()->getConstant();
  }

  std::sort(IndexLower.begin(), IndexLower.end(),
            [LowerConst](unsigned Idx1, unsigned Idx2) -> bool {
              return LowerConst[Idx1] < LowerConst[Idx2];
            });

  std::sort(IndexUpper.begin(), IndexUpper.end(),
            [UpperConst](unsigned Idx1, unsigned Idx2) -> bool {
              return UpperConst[Idx1] < UpperConst[Idx2];
            });

  unsigned LastIndex = IndexLower.back();
  unsigned ThisIndex = IndexUpper.front();

  HLLoop *FirstLoop = Candidates.front();
  HLNode *Marker = FirstLoop->getHLNodeUtils().getOrCreateMarkerNode();

  HLNodeUtils::replace(FirstLoop, Marker);

  // IndexSet indicates which loop bodies should be included into the generated
  // loop.
  SmallDenseSet<unsigned> IndexSet;
  IndexSet.insert(IndexLower.front());
  bool HasPreLoop = generatePreOrPostLoops<true>(Marker, IndexLower, LowerConst,
                                                 Candidates, IndexSet);
  IndexSet.erase(IndexUpper.front());
  bool HasPostLoop = generatePreOrPostLoops<false>(
      Marker, IndexUpper, UpperConst, Candidates, IndexSet);

  updatePragmaTripCountInfo(FirstLoop, Candidates, (HasPreLoop || HasPostLoop));

  for (auto LoopI = std::next(Candidates.begin()), E = Candidates.end();
       LoopI != E; ++LoopI) {
    HLNodeUtils::remove(*LoopI);

    moveMergeZtt(FirstLoop, *LoopI);

    HLNodeUtils::moveAsLastPreheaderNodes(FirstLoop, (*LoopI)->pre_begin(),
                                          (*LoopI)->pre_end());
    HLNodeUtils::moveAsLastChildren(FirstLoop, (*LoopI)->child_begin(),
                                    (*LoopI)->child_end());
    HLNodeUtils::moveAsLastPostexitNodes(FirstLoop, (*LoopI)->post_begin(),
                                         (*LoopI)->post_end());

    copyLiveRangeInfo(FirstLoop, *LoopI);
  }

  // Place common loop.
  FirstLoop->setLowerDDRef(Candidates[LastIndex]->removeLowerDDRef());
  FirstLoop->setUpperDDRef(Candidates[ThisIndex]->removeUpperDDRef());

  HLNodeUtils::replace(Marker, FirstLoop);
  FirstLoop->normalize();

  // Add possible new liveouts because of post loop.
  if (HasPostLoop) {
    HIRTransformUtils::addCloningInducedLiveouts(FirstLoop);
  }

  return FirstLoop;
}

class HIRLoopFusion::LoopVisitor : public HLNodeVisitorBase {
  HIRLoopFusion &LF;
  unsigned LoopCount;
  HLLoop *FirstLoop;
  HLLoop *LastLoop;
  HLNode *SkipNode;

private:
  void visitChildContainer(HLNode *ParentNode, HLNodeRangeTy Range) {
    LF.runOnNodeRange(ParentNode, Range);
  }

public:
  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}

  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }

  void visit(HLLoop *Loop) {
    if (Loop->isInnermost()) {
      // Do not recurse into innermost loops.
      SkipNode = Loop;
    }

    if (!isGoodLoop(Loop)) {
      // Do not take into account loops that may not be fused anyways.
      return;
    }

    LoopCount++;

    if (!FirstLoop) {
      FirstLoop = Loop;
    }

    LastLoop = Loop;
  }

  void visit(HLIf *If) {
    visitChildContainer(If, make_range(If->then_begin(), If->then_end()));
    visitChildContainer(If, make_range(If->else_begin(), If->else_end()));
  }

  void visit(HLSwitch *Switch) {
    for (unsigned Case = 1, E = Switch->getNumCases(); Case < E; ++Case) {
      visitChildContainer(Switch, make_range(Switch->case_child_begin(Case),
                                             Switch->case_child_end(Case)));
    }
    visitChildContainer(Switch, make_range(Switch->default_case_child_begin(),
                                           Switch->default_case_child_end()));
  }

  LoopVisitor(HIRLoopFusion &LF)
      : LF(LF), LoopCount(0), FirstLoop(nullptr), LastLoop(nullptr),
        SkipNode(nullptr) {}

  unsigned getLoopCount() const { return LoopCount; }

  HLLoop *getSingleLoop() const {
    assert(getLoopCount() < 2 && "There are more than one loop");
    return LastLoop;
  }

  HLNodeRangeTy getLoopRange() const {
    return make_range(FirstLoop->getIterator(),
                      std::next(LastLoop->getIterator()));
  }
};

static const FuseNode &getEffectiveLexicalFirstNode(const FuseGraph &FG) {
  const FuseNode *FirstFuseNode = nullptr;
  unsigned MinTopsortNum = std::numeric_limits<unsigned>::max();

  for (auto &FuseNode : FG.getFuseNodes()) {
    if (FuseNode.isRemoved()) {
      continue;
    }

    auto TopSortNum = FuseNode.getTopSortNumber();
    if (TopSortNum < MinTopsortNum) {
      MinTopsortNum = TopSortNum;
      FirstFuseNode = &FuseNode;
    }
  }

  return *FirstFuseNode;
}

void HIRLoopFusion::sortHLNodes(const FuseGraph &FG) {
  SmallVector<const FuseNode *, 8> Nodes;
  FG.topologicalSort(Nodes);

  HLNode *PtrNode = getEffectiveLexicalFirstNode(FG).getHLNode();

  bool NeedNextPtr = false;
  for (const FuseNode *Node : Nodes) {
    if (NeedNextPtr) {
      PtrNode = &*std::next(PtrNode->getIterator());
      NeedNextPtr = false;
    }

    HLNode *SortedNode = Node->getHLNode();

    if (PtrNode == SortedNode) {
      NeedNextPtr = true;
    } else {
      HLNodeUtils::moveBefore(PtrNode, SortedNode);
    }
  }
}

void HIRLoopFusion::runOnNodeRange(HLNode *ParentNode, HLNodeRangeTy Range) {
  if (Range.begin() == Range.end()) {
    // Range is empty
    return;
  }

  assert(Range.begin()->getParent() == ParentNode &&
         "Nodes in Range should be children of ParentNode");

  // Run on non-loop nodes
  LoopVisitor LV(*this);
  HLNodeUtils::visitRange<false>(LV, Range.begin(), Range.end());

  if (LV.getLoopCount() < 2) {
    if (HLLoop *Loop = LV.getSingleLoop()) {
      // Run on the single loop.
      runOnNodeRange(Loop, make_range(Loop->child_begin(), Loop->child_end()));
    }

    return;
  }

  LLVM_DEBUG(dbgs() << "runOnNodeRange(<" << ParentNode->getNumber() << ">, <"
                    << LV.getLoopRange().begin()->getNumber() << ">, <"
                    << std::prev(LV.getLoopRange().end())->getNumber()
                    << ">);\n");
  LLVM_DEBUG(dbgs() << "Loop count: " << LV.getLoopCount() << "\n");

  // Shrink range to [FirstLoop, LastLoop] by getting LV.getLoopRange().
  FuseGraph FG = FuseGraph::create(DDA, HLS, ParentNode, LV.getLoopRange());

  LLVM_DEBUG(dbgs() << "\nFinal Fusion Graph dump:\n");
  LLVM_DEBUG(FG.dump());

  LLVM_DEBUG(dbgs() << "\nFinal Fusion Nodes:\n");

  HLLoop *LastLoopFused = nullptr;
  for (const FuseNode &FNode : FG.getFuseNodes()) {
    if (FNode.isBadNode()) {
      continue;
    }

    LLVM_DEBUG(FNode.dump());

    bool LoopsFused = false;
    HLLoop *NextLoop;

    LoopOptReportBuilder &LORBuilder = HIRF.getLORBuilder();

    if (FNode.loops().size() > 1) {
      bool IsReportOn = LORBuilder.isLoopOptReportOn();
      SmallString<32> FuseNums;
      raw_svector_ostream VOS(FuseNums);

      // Traverse in forward order to combine the loop line numbers in the
      // correct loop order
      for (auto LoopI = std::next(FNode.loops().begin()),
                E = FNode.loops().end();
           LoopI != E; ++LoopI) {

        // Need to invalidate all loops except first one
        HIRInvalidationUtils::invalidateBody(*LoopI);

        if (IsReportOn && (*LoopI)->getDebugLoc()) {
          unsigned LineNum = (*LoopI)->getDebugLoc().getLine();
          VOS << LineNum;
          if (LoopI != std::prev(E)) {
            VOS << ",";
          }
        }
      }

      // Traverse in reverse order in order to correctly preserve the lost loop
      // reports
      for (auto LoopI = FNode.loops().rbegin(),
                E = std::prev(FNode.loops().rend());
           LoopI != E; ++LoopI) {

        LORBuilder(**LoopI).addRemark(OptReportVerbosity::Low,
                                      "Loop lost in Fusion");
        LORBuilder(**LoopI).preserveLostLoopOptReport();
      }

      // Align Loops
      // Fuse Loops
      NextLoop = fuseLoops(FNode.loops());

      LLVM_DEBUG(dbgs() << "While " OPT_DESC ":\n");
      LLVM_DEBUG(NextLoop->getParentRegion()->dump());

      LoopsFused = true;
      SmallString<32> FuseLoopNums;
      raw_svector_ostream VOSLN(FuseLoopNums);

      if (!FuseNums.empty()) {
        VOSLN << "with (";
        FuseLoopNums.append(FuseNums);
        VOSLN << ")";
      }

      LORBuilder(*NextLoop).addRemark(OptReportVerbosity::Low,
                                      "Loops have been fused %s", FuseLoopNums);
      LastLoopFused = NextLoop;
    } else {
      NextLoop = FNode.pilotLoop();
    }

    // It's fine to run fusion on inner loops without invalidating the parent
    // loop. Fusion is using LoopStatistics and DDA:
    // 1) Fusion of the outer loops does not invalidate statistics of inner
    //    loops.
    // 2) DDG is used to recover initial relations between references. After
    //    fusion the direction vectors will be invalid, but each pair will be
    //    refined anyways.
    runOnNodeRange(NextLoop,
                   make_range(NextLoop->child_begin(), NextLoop->child_end()));

    if (LoopsFused) {
      HIRInvalidationUtils::invalidateBody(NextLoop);
    }
  }

  if (LastLoopFused) {
    sortHLNodes(FG);

    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(LastLoopFused);
    LastLoopFused->getParentRegion()->setGenCode();
  }
}

bool HIRLoopFusion::run() {
  if (DisablePass) {
    return false;
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : "
                    << HIRF.getFunction().getName() << "\n");

  ForEach<HLRegion>::visitRange(
      HIRF.hir_begin(), HIRF.hir_end(), [this](HLRegion *Reg) {
        runOnNodeRange(Reg, make_range(Reg->child_begin(), Reg->child_end()));
      });

  return false;
}

PreservedAnalyses HIRLoopFusionPass::run(llvm::Function &F,
                                         llvm::FunctionAnalysisManager &AM) {
  HIRLoopFusion(AM.getResult<HIRFrameworkAnalysis>(F),
                AM.getResult<HIRDDAnalysisPass>(F),
                AM.getResult<HIRLoopStatisticsAnalysis>(F))
      .run();

  return PreservedAnalyses::all();
}

class HIRLoopFusionLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLoopFusionLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLoopFusionLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRLoopFusion(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                         getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                         getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS())
        .run();
  }
};

char HIRLoopFusionLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopFusionLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRLoopFusionLegacyPass, OPT_SWITCH, OPT_DESC, false, false)

FunctionPass *llvm::createHIRLoopFusionPass() {
  return new HIRLoopFusionLegacyPass();
}
