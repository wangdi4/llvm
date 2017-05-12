//===--------- HIRLoopStatistics.cpp - Computes loop statisticss ----------===//
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
// This file implements the loop statistics analysis pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopStatistics.h"

#include "llvm/IR/IntrinsicInst.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-statistics"

static cl::opt<bool> PrintTotalStatistics(
    "hir-print-total-statistics", cl::init(false), cl::Hidden,
    cl::desc("Prints total loop statistics instead of self loop statistics"));

FunctionPass *llvm::createHIRLoopStatisticsPass() {
  return new HIRLoopStatistics();
}

char HIRLoopStatistics::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopStatistics, "hir-loop-statistics",
                      "Loop Statistics Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRLoopStatistics, "hir-loop-statistics",
                    "Loop Statistics Analysis", false, true)

void HIRLoopStatistics::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRFramework>();
}

bool HIRLoopStatistics::runOnFunction(Function &F) {
  // This is an on-demand analysis, so we don't perform any analysis here.
  return false;
}

void HIRLoopStatistics::releaseMemory() {
  SelfStatisticsMap.clear();
  TotalStatisticsMap.clear();
}

struct LoopStatistics::LoopStatisticsVisitor final : public HLNodeVisitorBase {
  HIRLoopStatistics &HLS;
  const HLLoop *Lp;
  LoopStatistics &SelfLS;
  LoopStatistics *TotalLS;

  LoopStatisticsVisitor(HIRLoopStatistics &HLS, const HLLoop *Lp,
                        LoopStatistics &SelfLS, LoopStatistics *TotalLS)
      : HLS(HLS), Lp(Lp), SelfLS(SelfLS), TotalLS(TotalLS) {}

  // Main function to compute loop statistics.
  void compute() {
    // Do not directly recurse inside children loops. Total statistics is
    // recursively computed for children loops by the visitor using
    // getTotalLoopStatistics().
    Lp->getHLNodeUtils().visitRange<true, false>(*this, Lp->child_begin(),
                                                 Lp->child_end());

    // Add self reource to total resource and classify it.
    if (TotalLS) {
      *TotalLS += SelfLS;
    }
  }

  void visit(const HLIf *If) { SelfLS.NumIfs++; }
  void visit(const HLSwitch *Switch) { SelfLS.NumSwitches++; }
  void visit(const HLGoto *Goto) { SelfLS.NumGotos++; }
  void visit(const HLLabel *Label) { SelfLS.NumLabels++; }

  void visit(const HLInst *HInst) {
    auto Inst = HInst->getLLVMInstruction();

    auto Call = dyn_cast<CallInst>(Inst);

    if (Call) {
      if (isa<IntrinsicInst>(Call)) {
        SelfLS.NumIntrinsics++;
      } else {
        SelfLS.NumUserCalls++;
      }

      SelfLS.HasUnsafeSideEffects =
          !Call->onlyReadsMemory() && !Call->onlyAccessesArgMemory();
    }
  }

  void visit(const HLLoop *Lp) {
    if (TotalLS) {
      *TotalLS += HLS.getTotalLoopStatistics(Lp);
    }
  }

  void visit(const HLNode *Node) {
    llvm_unreachable("Unexpected HLNode type encountered!");
  }
  void postVisit(const HLNode *Node) {}
};

void LoopStatistics::print(formatted_raw_ostream &OS, const HLLoop *Lp) const {

  // Indent at one level more than the loop nesting level.
  unsigned Depth = Lp->getNestingLevel() + 1;

  if (NumIfs) {
    Lp->indent(OS, Depth);
    OS << "Number of ifs: " << NumIfs << "\n";
  }

  if (NumSwitches) {
    Lp->indent(OS, Depth);
    OS << "Number of switches: " << NumSwitches << "\n";
  }

  if (NumGotos) {
    Lp->indent(OS, Depth);
    OS << "Number of gotos: " << NumGotos << "\n";
  }

  if (NumLabels) {
    Lp->indent(OS, Depth);
    OS << "Number of labels: " << NumLabels << "\n";
  }

  if (NumUserCalls) {
    Lp->indent(OS, Depth);
    OS << "Number of user calls: " << NumUserCalls << "\n";
  }

  if (NumIntrinsics) {
    Lp->indent(OS, Depth);
    OS << "Number of intrinsics: " << NumIntrinsics << "\n";
  }
}

const LoopStatistics &
HIRLoopStatistics::computeLoopStatistics(const HLLoop *Loop, bool SelfOnly) {

  // These will be set below using the cache and SelfOnly paramter.
  LoopStatistics *TotalLS = nullptr;

  // Get or Insert self statistics.
  auto SelfPair =
      SelfStatisticsMap.insert(std::make_pair(Loop, LoopStatistics()));
  LoopStatistics &SelfLS = SelfPair.first->second;

  if (!SelfOnly) {
    // Set TotalLS to indicate that total statistics need to be computed.
    auto TotalPair =
        TotalStatisticsMap.insert(std::make_pair(Loop, LoopStatistics()));
    TotalLS = &TotalPair.first->second;
  }

  LoopStatistics::LoopStatisticsVisitor LSV(*this, Loop, SelfLS, TotalLS);

  LSV.compute();

  return SelfOnly ? SelfLS : *TotalLS;
}

const LoopStatistics &
HIRLoopStatistics::getSelfLoopStatistics(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  auto LSIt = SelfStatisticsMap.find(Loop);

  // Return cached statistics, if present.
  if (LSIt != SelfStatisticsMap.end()) {
    return LSIt->second;
  }

  // Compute and return a new reource.
  return computeLoopStatistics(Loop, true);
}

const LoopStatistics &
HIRLoopStatistics::getTotalLoopStatistics(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  // Self and total loop statistics for innermost loops are the same.
  if (Loop->isInnermost()) {
    return getSelfLoopStatistics(Loop);
  }

  auto LSIt = TotalStatisticsMap.find(Loop);

  // Return cached statistics, if present.
  if (LSIt != TotalStatisticsMap.end()) {
    return LSIt->second;
  }

  // Compute and return a new reource.
  return computeLoopStatistics(Loop, false);
}

void HIRLoopStatistics::print(formatted_raw_ostream &OS, const HLLoop *Lp) {
  const LoopStatistics &LS = PrintTotalStatistics ? getTotalLoopStatistics(Lp)
                                                  : getSelfLoopStatistics(Lp);
  LS.print(OS, Lp);
}

void HIRLoopStatistics::markLoopBodyModified(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  // Remove current loop's self statistics from the cache.
  SelfStatisticsMap.erase(Loop);

  // Remove current and parent loops total statistics from the cache.
  while (Loop) {
    TotalStatisticsMap.erase(Loop);
    Loop = Loop->getParentLoop();
  }
}
