//===--- HIRLoopDistributionGraph.cpp - Forms Distribution Graph  ---------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// This file forms the Distribution Pre Proc Graph
//===----------------------------------------------------------------------===//

// In order to establish piblock graph, we start with
// the DD graph and reduce it to a smaller graph(DistPPGraph) as a
// preprocessing step. The nodes(DistPPNodes) in this graph are also
// indivisible groups of hlnodes, but this graph is permitted to have cycles.
// There is only a single DistPPEdge between nodes, but it can represent
// multiple DD edges. The number of edges/nodes in this graph is likely to
// be much more tractable than the DDGraph itself for scc detection/analysis.
// This graph is then analyzed for sccs, each of which forms a pi block.
//
// Consider the following loop nest
//          BEGIN REGION { }
//          <29>         + DO i1 = 0, 99998, 1   <DO_LOOP>
//          <30>         |   + DO i2 = 0, 99998, 1   <DO_LOOP>
//          <6>          |   |   %0 = (@B)[0][i1 + 1][i2 + 1];
//          <8>          |   |   %1 = (@C)[0][i1 + 1][i2 + 1];
//          <9>          |   |   %add = %0  +  %1;
//          <11>         |   |   (@A)[0][i1 + 1][i2 + 1] = %add;
//          <14>         |   |   %3 = (@A)[0][i1 + 1][i2];
//          <15>         |   |   %conv18 = %3  *  2.000000e+00;
//          <17>         |   |   (@D)[0][i1 + 1][i2 + 1] = %conv18;
//          <30>         |   + END LOOP
//          <29>         + END LOOP
//          END REGION
// If we are distributing the innermost loop, we have 7 stmts, each of which
// forms its own indivisible DistPPNode. DD edges are analyzed to form
// DistPPEdges. The resulting DistPPGraph is then analyzed
// for SCCs, each of which is a pi block. This results in a graph with two nodes
// PiBlock:
//<6>       %0 = (@B)[0][i1 + 1][i2 + 1];
//<8>       %1 = (@C)[0][i1 + 1][i2 + 1];
//<9>       %add = %0  +  %1;
//<11>      (@A)[0][i1 + 1][i2 + 1] = %add;
// PiBlock:
//<14>      %3 = (@A)[0][i1 + 1][i2];
//<15>      %conv18 = %3  *  2.000000e+00;
//<17>      (@D)[0][i1 + 1][i2 + 1] = %conv18;
// and a Pi Edge from first block to second block representing dd constraints(in
// this case only the flow < to/from A[][][])
//
// However consider the slightly modified version
//<38>         + DO i1 = 0, 99998, 1   <DO_LOOP>
//<39>         |   + DO i2 = 0, 99998, 1   <DO_LOOP>
//<6>          |   |   if (i2 + 1 < 27)
//<6>          |   |   {
//<11>         |   |      %0 = (@B)[0][i1 + 1][i2 + 1];
//<13>         |   |      %1 = (@C)[0][i1 + 1][i2 + 1];
//<14>         |   |      %add = %0  +  %1;
//<16>         |   |      (@A)[0][i1 + 1][i2 + 1] = %add;
//<6>          |   |   }
//<6>          |   |   else
//<6>          |   |   {
//<33>         |   |      %3 = (@A)[0][i1 + 1][i2];
//<34>         |   |      %conv19 = %3  *  2.000000e+00;
//<36>         |   |      (@D)[0][i1 + 1][i2 + 1] = %conv19;
//<6>          |   |   }
//<39>         |   + END LOOP
//<38>         + END LOOP
// The graph for the innermost loop has a single DistPPNode which contains the
// HLIf and indirectly its children.
//
// Graphs for the outermost loop also contain a single node for the inner
// HLLoop. It is assumed that if the innermost level loop was distributable,
// it would have already been done. This forces clients to analyze loops
// innermost to outermost if considering all distribution possibilities

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopDistributionPreProcGraph.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-distribution-pre-proc-graph"

DistPPGraph::DistPPGraph(HLLoop *Loop, HIRDDAnalysis *DDA,
                         HIRLoopStatistics *HLS) {
  const unsigned MaxDDEdges = 256;

  createNodes(Loop);
  if (!isGraphValid()) {
    return;
  }

  DDGraph DDG = DDA->getGraph(Loop);

  DistributionEdgeCreator EdgeCreator(&DDG, this, Loop, HLS);
  Loop->getHLNodeUtils().visitRange(EdgeCreator, Loop->getFirstChild(),
                                    Loop->getLastChild());

  if (EdgeCreator.EdgeCount > MaxDDEdges) {
    setInvalid("Too many DD edges for proper analysis");
  }
}
void DistPPGraph::createNodes(HLLoop *Loop) {
  const unsigned MaxDistPPSize = 128;

  DistributionNodeCreator NodeCreator(this);
  Loop->getHLNodeUtils().visitRange(NodeCreator, Loop->getFirstChild(),
                                    Loop->getLastChild());
  // Bail early before DD for invalid cases
  if (getNodeCount() > MaxDistPPSize) {
    setInvalid("Too many stmts to analyze");
  }

  if (getNodeCount() == 1) {
    setInvalid("Single Node Loop cannot be analyzed");
  }
}
