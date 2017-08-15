//===- HIRLoopDistribution.h - Implements Loop Distribution ---------------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
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
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPDISTRIBUTION_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPDISTRIBUTION_H

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopDistributionGraph.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {
namespace loopopt {

enum class DistHeuristics : unsigned char {
  NotSpecified = 0, // Default enum for command line option. Will be overridden
                    // by pass constructor argument
  MaximalDist,      // Everytime you can, do it. Testing only
  NestFormation,    // Try to form perfect loop nests
  BreakMemRec,      // Break recurrence among mem refs ie A[i] -> A[i+i]
  BreakScalarRec,   // Break recurrence among scalars. Requires scalar expansion
};

class HIRLoopDistribution : public HIRTransformPass {
  typedef SmallVector<PiBlock *, 4> PiBlockList;

public:
  HIRLoopDistribution(char &ID, DistHeuristics DistCostModel)
      : HIRTransformPass(ID), DistCostModel(DistCostModel) {}
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
    AU.addRequiredTransitive<HIRLoopStatistics>();
    AU.addRequiredTransitive<HIRDDAnalysis>();
  }
  int OptReportLevel = 1;

private:
  Function *F;
  HIRDDAnalysis *DDA;
  DistHeuristics DistCostModel;

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
}
#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLOOPDISTRIBUTION_H
