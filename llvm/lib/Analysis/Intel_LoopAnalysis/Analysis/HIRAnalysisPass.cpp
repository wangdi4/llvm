//===----- HIRAnalysisPass.cpp - implements base HIR analysis pass --------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the base HIR analysis pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-analysis-pass"

struct HIRAnalysisPass::PrintVisitor final : public HLNodeVisitorBase {
  HIRAnalysisPass &HAP;
  formatted_raw_ostream FOS;

  PrintVisitor(HIRAnalysisPass &HAP, raw_ostream &OS) : HAP(HAP), FOS(OS) {}

  void visit(const HLRegion *Reg) {
    Reg->printHeader(FOS, 0, false, false);

    // Call derived class's print() after printing the header.
    HAP.print(FOS, Reg);
  }

  void postVisit(const HLRegion *Reg) { Reg->printFooter(FOS, 0); }

  void visit(const HLLoop *Lp) {

    Lp->printHeader(FOS, Lp->getNestingLevel(), false);

    // Call derived class's print() after printing the header.
    HAP.print(FOS, Lp);
  }

  void postVisit(const HLLoop *Lp) {
    Lp->printFooter(FOS, Lp->getNestingLevel());
  }

  void visit(const HLNode *Node) {}

  void postVisit(const HLNode *Node) {}
};

void HIRAnalysisPass::print(raw_ostream &OS, const Module *) const {

  // Remove constness as HIR analyses are on-demand and have to compute results
  // for printing.
  HIRAnalysisPass &HAP = *const_cast<HIRAnalysisPass *>(this);

  PrintVisitor PV(HAP, OS);
  auto HIRF = getAnalysisIfAvailable<HIRFramework>();
  assert(HIRF && "HIRFramework not available!");

  HIRF->getHLNodeUtils().visitAll(PV);
}
