//===--- HIRLoopDistributionGraph.cpph - Forms Distribution Graph  --------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Provides a DAG of piblocks(PiGraph) which LoopDistribution can analyze for
// distribution points or reductions. A pi block is formally defined as one or
// more hlnodes in a loop which must remain in same loop. A group of hlnodes
// which form an SCC in DDgraph would be a piblock[cannot break cycle without
// violating dd constraints] as would an hlif and its children[cannot split
// it without violation of control flow constraints]. The edges, PiEdges,
// describe the DD constraints amongs pi blocks.
//
// PiGraph is formed from DistPPGraph, which is a condensed form of DD graph.
// See implementation for more details
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopDistributionGraph.h"

using namespace llvm;
using namespace llvm::loopopt;

void PiBlock::setPiBlockType(const std::vector<DistPPNode *> &SCCNodes) {
  int StmtCount = 0;
  int LoopCount = 0;
  for (DistPPNode *Node : SCCNodes) {
    if (isa<HLLoop>(Node->HNode)) {
      LoopCount++;
    } else {
      StmtCount++;
    }
  }

  if (StmtCount == 0) {
    if (LoopCount == 0) {
      llvm_unreachable("Malformed pi block in loop distribution");
    } else if (LoopCount == 1) {
      BlockType = PiBlockType::SingleLoop;
    } else {
      BlockType = PiBlockType::MultipleLoop;
    }
  } else if (StmtCount == 1) {
    if (LoopCount == 0) {
      BlockType = PiBlockType::SingleStmt;
    } else {
      BlockType = PiBlockType::StmtAndLoop;
    }
  } else if (StmtCount > 1) {
    if (LoopCount == 0) {
      BlockType = PiBlockType::MultipleStmt;
    } else {
      BlockType = PiBlockType::StmtAndLoop;
    }
  }
}

void llvm::loopopt::PiGraph::createNodes() {

  for (auto I = all_scc_begin(PPGraph), E = all_scc_end(PPGraph); I != E; ++I) {
    addPiBlock(*I);
  }

  // scc_iterator uses tarjans algorithm, which emits scc's in
  // reverse top sort order. Reverse piblock list to restore
  // top sort order
  std::reverse(PiBlocks.begin(), PiBlocks.end());
}
