//===---- DDGraph.cpp - DD Graph implementation ---------------------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/IR/Metadata.h"

using namespace llvm;
using namespace llvm::loopopt;

DDEdge::DepType DDEdge::getEdgeType() const {
  RegDDRef *SrcRef = dyn_cast<RegDDRef>(Src);
  RegDDRef *SinkRef = dyn_cast<RegDDRef>(Sink);
  bool SrcIsLval = (SrcRef && SrcRef->isLval()) ? true : false;
  bool SinkIsLval = (SinkRef && SinkRef->isLval()) ? true : false;

  if (SrcIsLval) {
    if (SinkIsLval) {
      return DepType::OUTPUT;
    }
    return DepType::FLOW;
  } else {
    if (SinkIsLval) {
      return DepType::ANTI;
    }
    return DepType::INPUT;
  }
}

bool DDEdge::isForwardDep(bool CheckIfPath) const {
  auto SrcTopSortNum = getSrc()->getHLDDNode()->getTopSortNum();
  auto SinkTopSortNum = getSink()->getHLDDNode()->getTopSortNum();

  // Handle the case A[I] = A[I] + B[I]
  // Case 1: The flow edge (from Lval to Rval) is backward.
  // Case 2: The anti edge (from Rval to Lval) is forward.
  if (SrcTopSortNum == SinkTopSortNum) {
    RegDDRef *SrcRef = dyn_cast<RegDDRef>(Src);
    bool SrcIsLval = (SrcRef && SrcRef->isLval());
    return !SrcIsLval;
  }

  if (CheckIfPath) {
    // a workaround for the fact that CFG representation in VPlan isn't lexical.
    // TODO: Remove once CMPLRLLVM-3064 will be implemented
    auto SinkNode = getSink()->getHLDDNode();
    auto SrcNode = getSrc()->getHLDDNode();
    auto *CommonParent =
        HLNodeUtils::getLexicalLowestCommonAncestorParent(SrcNode, SinkNode);
    if (auto *IfParent = dyn_cast<HLIf>(CommonParent)) {
      if (IfParent->isThenChild(SrcNode) != IfParent->isThenChild(SinkNode))
        return false; // assume backward
    }
  }
  return (SrcTopSortNum < SinkTopSortNum);
}

std::string DDEdge::getOptReportStr() const {

  std::string Str;
  raw_string_ostream OS(Str);
  OS << "assumed ";
  OS << getEdgeType();
  OS << " dependence";

  std::string SrcInfo = Src->getNameAndDbgLocForOptRpt();
  std::string SinkInfo = Sink->getNameAndDbgLocForOptRpt();

  if (!SrcInfo.empty() && !SinkInfo.empty())
    OS << " between " << SrcInfo << "and " << SinkInfo;
  return OS.str();
}

void DDEdge::print(raw_ostream &OS) const {
  formatted_raw_ostream FOS(OS);
  FOS << Src->getHLDDNode()->getNumber() << ":";
  FOS << Sink->getHLDDNode()->getNumber() << " ";
  Src->print(FOS);
  FOS << " --> ";
  Sink->print(FOS);
  FOS << " ";
  FOS << getEdgeType();
  FOS << " ";
  assert(DV.size() == DistVector.size());
  DV.print(FOS);
  DistVector.print(FOS);

  if (FirstIterPeelingRemovesDep) {
    FOS << " FirstIterPeelingRemovesDep";
  }

  FOS << " \n";

  // todo
}

unsigned DDGraph::getTotalNumIncomingFlowEdges(const DDRef *Ref) const {
  unsigned Num = 0;
  for (auto &Edge : incoming(Ref)) {
    if (Edge->isFlow()) {
      Num++;
    }
  }

  const RegDDRef *RRef = dyn_cast<RegDDRef>(Ref);
  if (!RRef) {
    return Num;
  }

  for (auto &BRRef : make_range(RRef->blob_begin(), RRef->blob_end())) {
    for (auto &Edge : incoming(BRRef)) {
      assert(Edge->isFlow() &&
             "Incoming edges to blob refs should be flow edges only");
      (void)Edge;
      Num++;
    }
  }
  return Num;
}

bool DDGraph::singleEdgeGoingOut(const DDRef *LRef) {
  unsigned NumEdge = 0;

  for (auto *Edge : outgoing(LRef)) {
    (void)Edge;
    if (NumEdge++ > 1) {
      return false;
    }
  }

  return true;
}

void DDGraph::print(raw_ostream &OS) const {
  DDARefGatherer::MapTy Refs;
  DDARefGatherer::gather(CurNode, Refs);

  for (auto &Pair : Refs) {
    for (DDRef *Ref : Pair.second) {
      for (DDEdge *E : outgoing(Ref)) {
        E->print(OS);
      }
    }
  }
}
