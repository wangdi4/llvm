//===------ HIROptReportImpl.cpp --------------------------- --*- C++ -*---===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements LoopOptReportTraits specializations for HIR loops and
// regions.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

LoopOptReport LoopOptReportTraits<HLLoop>::getOrCreatePrevOptReport(
    HLLoop &Loop, const LoopOptReportBuilder &Builder) {

  struct PrevLoopFinder : public HLNodeVisitorBase {
    const HLLoop *FoundLoop = nullptr;
    const HLNode *FirstNode;

    PrevLoopFinder(const HLNode *F) : FirstNode(F) {}
    bool isDone() const { return FoundLoop; }
    void visit(const HLLoop *Lp) {
      if (Lp != FirstNode && Lp->getTopSortNum() < FirstNode->getTopSortNum())
        FoundLoop = Lp;
    }
    void visit(const HLNode *Node) {}
    void postVisit(const HLNode *Node) {}
  };

  PrevLoopFinder PLF(&Loop);
  const HLNode *FirstNode;
  const HLNode *LastNode;
  const HLLoop *ParentLoop = Loop.getParentLoop();
  if (ParentLoop) {
    FirstNode = ParentLoop->getFirstChild();
    LastNode = Loop.getHLNodeUtils().getImmediateChildContainingNode(ParentLoop,
                                                                     &Loop);

  } else {
    const HLRegion *ParentRegion = Loop.getParentRegion();
    FirstNode = ParentRegion->getFirstChild();
    LastNode = Loop.getHLNodeUtils().getImmediateChildContainingNode(
        ParentRegion, &Loop);
  }

  HLNodeUtils::visitRange<true, false, false>(PLF, FirstNode, LastNode);
  if (!PLF.FoundLoop)
    return nullptr;

  HLLoop &Lp = const_cast<HLLoop &>(*PLF.FoundLoop);
  return Builder(Lp).getOrCreateOptReport();
}

LoopOptReport LoopOptReportTraits<HLLoop>::getOrCreateParentOptReport(
    HLLoop &Loop, const LoopOptReportBuilder &Builder) {
  if (HLLoop *Dest = Loop.getParentLoop())
    return Builder(*Dest).getOrCreateOptReport();

  if (HLRegion *Dest = Loop.getParentRegion())
    return Builder(*Dest).getOrCreateOptReport();

  llvm_unreachable("Failed to find a parent");
}

void LoopOptReportTraits<HLLoop>::traverseChildLoopsBackward(
    HLLoop &Loop, LoopVisitorTy Func) {
  struct LoopVisitor : public HLNodeVisitorBase {
    using LoopVisitorTy = LoopOptReportTraits<HLLoop>::LoopVisitorTy;
    LoopVisitorTy Func;

    LoopVisitor(LoopVisitorTy Func) : Func(Func) {}
    void postVisit(HLLoop *Lp) { Func(*Lp); }
    void visit(const HLNode *Node) {}
    void postVisit(const HLNode *Node) {}
  };

  if (Loop.hasChildren()) {
    LoopVisitor LV(Func);
    HLNodeUtils::visitRange<true, false, false>(LV, Loop.getFirstChild(),
                                                Loop.getLastChild());
  }
}
