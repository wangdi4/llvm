//===- HIRLoopDistribution.h - Implements Loop Distribution ---------------===//
//
// Copyright (C) 2016-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file is used for HIR Loop Distribution
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPDISTRIBUTIONIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPDISTRIBUTIONIMPL_H

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopResource.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSparseArrayReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SparseBitVector.h"

#include "HIRLoopDistributionGraph.h"

namespace llvm {
namespace loopopt {

namespace distribute {
const unsigned MaxDistributedLoop = 25;
const unsigned MaxArrayTempsAllowed = 50;
const unsigned SmallTripCount = 16;
const unsigned StripmineSize = 64;
// For stress testing, use small max resource
// const unsigned MaxMemResourceToDistribute = 2;

enum class DistHeuristics : unsigned char {
  NotSpecified = 0, // Default enum for command line option. Will be overridden
                    // by pass constructor argument
  MaximalDist,      // Everytime you can, do it. Testing only
  NestFormation,    // Try to form perfect loop nests
  BreakMemRec,      // Break recurrence among mem refs ie A[i] -> A[i+i]
  BreakScalarRec    // Break recurrence among scalars. Requires scalar expansion
};

typedef SmallVector<HLDDNode *, 12> HLDDNodeList;
typedef SmallVector<PiBlock *, 4> PiBlockList;
typedef DDRefGatherer<DDRef, AllRefs ^ (ConstantRefs | GenericRValRefs |
                                        IsAddressOfRefs)>
    Gatherer;

class ScalarExpansion {
public:
  struct Candidate {
    struct UseCand {
      DDRef *Ref;
      HLNode *FirstNode;
      bool IsTempRedefined;

      // Instruction that should be cloned to be recomputable.
      const HLInst *DepInst;
    };

    bool SafeToRecompute = true;

    SmallDenseMap<HLLoop *, HLNode *> LoopDefInsertNode;
    SmallDenseMap<HLLoop *, HLNode *> LoopUseInsertNode;

    SmallVector<DDRef *, 8> TmpDefs;
    SmallVector<UseCand, 8> TmpUses;

    unsigned getSymbase() const { return TmpDefs.front()->getSymbase(); }

    bool isTempRequired() const {
      return TmpDefs.size() != 1 || !SafeToRecompute;
    }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    LLVM_DUMP_METHOD void dump() {
      TmpDefs.front()->dump();

      dbgs() << " (sb:" << getSymbase();
      dbgs() << ") (";
      for (auto &TmpDef : enumerate(TmpDefs)) {
        dbgs() << TmpDef.value()->getHLDDNode()->getNumber();
        if (TmpDef.index() != TmpDefs.size() - 1) {
          dbgs() << ",";
        }
      }
      dbgs() << ") -> (";
      for (auto &TmpUse : enumerate(TmpUses)) {
        dbgs() << TmpUse.value().Ref->getHLDDNode()->getNumber();
        if (TmpUse.index() != TmpUses.size() - 1) {
          dbgs() << ",";
        }
      }
      dbgs() << ") SafeToRecompute: " << SafeToRecompute;
    }
#endif
  };

private:
  HLLoop *Loop;
  HLNodeUtils &HNU;
  bool HasDistributePoint;
  bool HasBadCandidate;

  SmallVector<Candidate, 8> Candidates;
  SparseBitVector<> ModifiedBases;
  // <Symbase, Loop number>
  using SymbaseLoopSetTy = SmallSet<std::pair<unsigned, unsigned>, 8>;

public:
  ScalarExpansion(HLLoop *Loop, bool HasDistributePoint,
                  ArrayRef<HLDDNodeList> Chunks);

  ArrayRef<Candidate> getCandidates() const { return Candidates; }

  bool isTempRequired() const {
    return std::any_of(Candidates.begin(), Candidates.end(),
                       isTempRequiredPredicate);
  }

  bool isScalarExpansionRequired() const {
    return !Candidates.empty();
  }

  template <bool IsDef> void getInsertNodeForTmpDefsUses();

  unsigned getNumTempsRequired() const {
    return std::count_if(Candidates.begin(), Candidates.end(),
                         isTempRequiredPredicate);
  }

  bool hasBadCandidate() const {return HasBadCandidate;}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump() {
    for (auto &Cand : Candidates) {
      Cand.dump();
      dbgs() << "\n";
    }
  }
#endif

private:
  // Find the instruction that defines \p RVal.
  bool findDepInst(const RegDDRef *RVal, const HLInst *&DepInst);

  // Check if \p TmpDef is safe to recompute in loop with \p ChunkIdx rather
  // than use a temp.
  bool isSafeToRecompute(const RegDDRef *TmpDef, unsigned ChunkIdx,
                         const SymbaseLoopSetTy &SymbaseLoopSet,
                         const SparseBitVector<> &ModifiedSymbases,
                         const HLInst *&DepInst);

  void analyze(ArrayRef<HLDDNodeList> Chunks);

  bool isScalarExpansionCandidate(const DDRef *Ref) const;

  static bool isTempRequiredPredicate(const Candidate &C) {
    return C.isTempRequired();
  }
};

class HIRLoopDistribution {
  typedef unsigned LoopNum;
  typedef bool InsertOrMove;

public:
  HIRLoopDistribution(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                      HIRSafeReductionAnalysis &SRA,
                      HIRSparseArrayReductionAnalysis &SARA,
                      HIRLoopResource &HLR, HIRLoopLocality &HLL,
                      DistHeuristics DistCostModel)
      : HIRF(HIRF), DDA(DDA), SRA(SRA), SARA(SARA), HNU(HIRF.getHLNodeUtils()),
        HLR(HLR), HLL(HLL), DistCostModel(DistCostModel) {}

  bool run();

private:
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  HIRSafeReductionAnalysis &SRA;
  HIRSparseArrayReductionAnalysis &SARA;
  HLNodeUtils &HNU;
  HIRLoopResource &HLR;
  HIRLoopLocality &HLL;

  DistHeuristics DistCostModel;
  HLLoop *NewLoops[MaxDistributedLoop];
  SmallSet<unsigned, 12> TempArraySB;
  SmallDenseMap<const HLDDNode *, std::pair<LoopNum, InsertOrMove>, 16>
      DistDirectiveNodeMap;

  SmallDenseMap<const HLDDNode *, std::pair<LoopNum, LoopNum>, 16> IfNodeMap;

private:
  void findDistPoints(const HLLoop *L, std::unique_ptr<PiGraph> const &PGraph,
                      SmallVectorImpl<PiBlockList> &DistPoints) const;

  // Returns true if this edge contains dd edge with (<) at loop level
  // Such an edge would be eliminated by distributing the src sink piblocks
  // into separate loops
  bool piEdgeIsMemRecurrence(const HLLoop *Lp, const PiGraphEdge &PiEdge) const;

  // Loop may be discarded prior to any analysis by some heuristics.
  // For example, the costmodel may consider only innermost loops, no need
  // to do potentially expensive analysis on others
  bool loopIsCandidate(HLLoop *L) const;

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

  void
  splitSpatialLocalityGroups(const HLLoop *L,
                             std::unique_ptr<PiGraph> const &PiGraph,
                             SmallVectorImpl<PiBlockList> &DistPoints) const;

  // Uses ordered list of PiBlockLists to form distributed version of Loop.
  // Each PiBlockList will form a new loop(with same bounds as Loop) containing
  // each piblock's hlnodes.

  void distributeLoop(HLLoop *L,
                      SmallVectorImpl<HLDDNodeList> &DistributedLoops,
                      const ScalarExpansion &SCEX, OptReportBuilder &ORBuilder,
                      bool ForDirective);

  // Create TEMP[i] = temp and insert
  RegDDRef *createTempArrayStore(HLLoop *Lp, RegDDRef *TempRef,
                                 unsigned OrigLoopLevel);

  // Insert an assignment TEMP[i] = temp after DDNode
  void insertTempArrayStore(HLLoop *Lp, RegDDRef *TempRef,
                            RegDDRef *TmpArrayRef, HLDDNode *TempRefDDNode);

  // Create an assignment  temp = TEMP[i]
  // TempRefined if true means temp is refined in the sink loop
  // Insertion of assignment needs special handling
  void createTempArrayLoad(RegDDRef *TempRef, RegDDRef *TempArrayRef,
                           HLNode *Node, bool TempRefined);

  // After scalar expansion, scalar temps is need to be replaced with Array Temp
  void replaceWithArrayTemp(unsigned OrigLoopLevel,
                            ArrayRef<ScalarExpansion::Candidate> Candidates);

  // After calling Stripmining util, temp iv coeffs need to fixed
  // as single IV:  TEMP[i2], while other indexes have i1, i2
  void fixTempArrayCoeff(HLLoop *Loop);

  // Process pragma for Loop directive Distribute Point

  unsigned distributeLoopForDirective(HLLoop *Lp);

  // Collect HNodes that match LoopNum
  void
  collectHNodesForDirective(HLLoop *Lp,
                            SmallVectorImpl<HLDDNodeList> &DistributedLoops,
                            HLDDNodeList &CurLoopHLDDNodeList);
  // If nodes can be split up into different loops.
  HLDDNode *processPragmaForIf(HLDDNode *IfParent, HLDDNode *CurrentIf,
                               HLDDNodeList &CurLoopHLDDNodeList,
                               unsigned LoopNum);
  // Moving children of If to new loops
  void moveIfChildren(HLContainerTy::iterator Begin,
                      HLContainerTy::iterator End, HLIf *NewHLIf,
                      HLDDNode *TopIfHNode, HLDDNodeList &CurLoopHLDDNodeList,
                      unsigned TopIfLoopNum, bool IsThenChild);

  void
  processPiBlocksToHLNodes(const std::unique_ptr<PiGraph> &PGraph,
                           ArrayRef<PiBlockList> GroupsOfPiBlocks,
                           SmallVectorImpl<HLDDNodeList> &DistributedLoops);

  void invalidateLoop(HLLoop *Loop) const;
};

class HIRLoopDistributionLegacyPass : public HIRTransformPass {
  DistHeuristics DistCostModel;

public:
  HIRLoopDistributionLegacyPass(char &ID, DistHeuristics DistCostModel)
      : HIRTransformPass(ID), DistCostModel(DistCostModel) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F) override;
};

} // namespace distribute
} // namespace loopopt
} // namespace llvm
#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPDISTRIBUTIONIMPL_H
