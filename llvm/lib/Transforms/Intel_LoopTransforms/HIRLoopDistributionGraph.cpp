//===--- HIRLoopDistributionGraph.cpph - Forms Distribution Graph  --------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "HIRLoopDistributionGraph.h"

#define LLVM_DEBUG_DDG(X) DEBUG_WITH_TYPE("hir-loop-distribute-ddg", X)

using namespace llvm;
using namespace llvm::loopopt;

void PiBlock::setPiBlockType(ArrayRef<DistPPNode *> SCCNodes) {
  int StmtCount = 0;
  int LoopCount = 0;
  for (DistPPNode *Node : SCCNodes) {
    if (isa<HLLoop>(Node->getNode())) {
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

void PiGraph::createNodes() {
  for (auto I = all_scc_begin(PPGraph), E = all_scc_end(PPGraph); I != E; ++I) {
    addPiBlock(*I);
  }

  // scc_iterator uses tarjans algorithm, which emits scc's in
  // reverse top sort order. Reverse piblock list to restore
  // top sort order
  std::reverse(PiBlocks.begin(), PiBlocks.end());
}

void PiGraph::createEdges() {
  for (auto PiBlkIt = PiBlocks.begin(), PiEndIt = PiBlocks.end();
       PiBlkIt != PiEndIt; ++PiBlkIt) {
    PiBlock *SrcBlk = *PiBlkIt;
    for (auto NodeIt = SrcBlk->dist_node_begin(),
              EndIt = SrcBlk->dist_node_end();
         NodeIt != EndIt; ++NodeIt) {
      // Maps a sink piblock to a list of ddedges
      DenseMap<PiBlock *, SmallVector<const DDEdge *, 16>> CurEdges;

      // Go through all outgoing dist edges and add their dd edges
      // to sink pi block's list in CurEdges
      for (auto EdgeIt = PPGraph->outgoing_edges_begin(*NodeIt),
                EndEdgeIt = PPGraph->outgoing_edges_end(*NodeIt);
           EdgeIt != EndEdgeIt; ++EdgeIt) {
        PiBlock *SinkPiBlk = DistPPNodeToPiBlock[(*EdgeIt)->getSink()];
        assert(SinkPiBlk && "Invalid dist edge added");
        if (SrcBlk == SinkPiBlk) {
          // No cycles, not even self cycles
          continue;
        }
        CurEdges[SinkPiBlk].append((*EdgeIt)->DDEdges.begin(),
                                   (*EdgeIt)->DDEdges.end());
      }

      // Edges in graph cannot be modified once added.
      // Once all edge lists for a given src piblock are fully created, create
      // PiGraphEdges out of them
      for (auto PiEdgeIt = CurEdges.begin(), PiEdgeEnd = CurEdges.end();
           PiEdgeIt != PiEdgeEnd; ++PiEdgeIt) {
        addEdge(PiGraphEdge(SrcBlk, PiEdgeIt->first, PiEdgeIt->second));
      }
    }
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

LLVM_DUMP_METHOD
void PiGraph::dump() const {
  dbgs() << "\n<start> Proposed order\n";
  for (auto *Block : PiBlocks) {
    dbgs() << "\nPiBlock: \n";
    Block->dump();

    LLVM_DEBUG_DDG(
    dbgs() << "\nInternal PP Edges: \n";
    for (auto *Node :
        make_range(Block->dist_node_begin(), Block->dist_node_end())) {
      for (auto *Edge : PPGraph->outgoing(Node)) {
        Edge->dump();
      }
      dbgs() << "-\n";
    }

    dbgs() << "\nExternal Pi Edges: \n";
    for (auto *Edge : outgoing(Block)) {
      Edge->dump();
    }
    dbgs() << "\n";
    );
  }
  dbgs() << "<end>\n";
}

LLVM_DUMP_METHOD
void PiBlock::dump() const {
  for (auto *PPNode : DistPPNodes) {
    PPNode->dump();
  }
}

LLVM_DUMP_METHOD
void PiGraphEdge::dump() const {
  for (auto *DDEdge : DDEdges) {
    DDEdge->dump();
  }
}

#endif