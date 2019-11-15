//===--------- HIRLoopStatistics.cpp - Computes loop statisticss ----------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/InitializePasses.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-statistics"

static cl::opt<bool> PrintTotalStatistics(
    "hir-print-total-statistics", cl::init(false), cl::Hidden,
    cl::desc("Prints total loop statistics instead of self loop statistics"));

FunctionPass *llvm::createHIRLoopStatisticsWrapperPass() {
  return new HIRLoopStatisticsWrapperPass();
}

AnalysisKey HIRLoopStatisticsAnalysis::Key;
HIRLoopStatistics HIRLoopStatisticsAnalysis::run(Function &F,
                                                 FunctionAnalysisManager &AM) {
  return HIRLoopStatistics(AM.getResult<HIRFrameworkAnalysis>(F));
}

char HIRLoopStatisticsWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopStatisticsWrapperPass, "hir-loop-statistics",
                      "Loop Statistics Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRLoopStatisticsWrapperPass, "hir-loop-statistics",
                    "Loop Statistics Analysis", false, true)

void HIRLoopStatisticsWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRFrameworkWrapperPass>();
}

bool HIRLoopStatisticsWrapperPass::runOnFunction(Function &F) {
  HLS.reset(
      new HIRLoopStatistics(getAnalysis<HIRFrameworkWrapperPass>().getHIR()));
  return false;
}

void HIRLoopStatisticsWrapperPass::releaseMemory() { HLS.reset(); }

struct LoopStatistics::LoopStatisticsVisitor final : public HLNodeVisitorBase {
  HIRLoopStatistics &HLS;
  const HLLoop *Lp;
  LoopStatistics *SelfStats;
  LoopStatistics *ChildrenStats;

  LoopStatisticsVisitor(HIRLoopStatistics &HLS, const HLLoop *Lp,
                        LoopStatistics *SelfStats,
                        LoopStatistics *ChildrenStats)
      : HLS(HLS), Lp(Lp), SelfStats(SelfStats), ChildrenStats(ChildrenStats) {
    assert((SelfStats || ChildrenStats) &&
           "At least one of self/children stats should be present!");
  }

  // Main function to compute loop statistics.
  void compute() {
    // Do not directly recurse inside children loops. Children statistics is
    // recursively computed for children loops by the visitor using
    // getTotalLoopStatistics().
    Lp->getHLNodeUtils().visitRange<true, false>(*this, Lp->child_begin(),
                                                 Lp->child_end());
  }

  void visit(const HLIf *If) {
    if (SelfStats) {
      SelfStats->NumIfs++;
    }
  }

  void visit(const HLSwitch *Switch) {
    if (SelfStats) {
      SelfStats->NumSwitches++;
    }
  }

  void visit(const HLGoto *Goto) {
    if (SelfStats && !Goto->isUnknownLoopBackEdge()) {
      SelfStats->NumForwardGotos++;
    }
  }

  void visit(const HLLabel *Label) {
    if (SelfStats && !Label->isUnknownLoopHeaderLabel()) {
      SelfStats->NumLabels++;
    }
  }

  void visit(const HLInst *HInst) {
    if (!SelfStats) {
      return;
    }

    auto *Call = HInst->getCallInst();

    if (Call) {
      if (isa<IntrinsicInst>(Call)) {
        SelfStats->NumIntrinsics++;
      } else {
        SelfStats->NumUserCalls++;

        if (!Call->getCalledFunction()) {
          SelfStats->NumIndirectCalls++;
        }
      }

      SelfStats->HasCallsWithUnsafeSideEffects |=
          HLInst::hasUnsafeSideEffects(Call);

      SelfStats->HasCallsWithNoDuplicate |= Call->cannotDuplicate();

      SelfStats->HasCallsWithUnknownAliasing |=
          HLInst::hasUnknownAliasing(Call);
    }
  }

  void visit(const HLLoop *Lp) {
    if (ChildrenStats) {
      *ChildrenStats += HLS.getTotalLoopStatistics(Lp);
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

  Lp->indent(OS, Depth);
  OS << "Number of ifs: " << NumIfs << "\n";

  Lp->indent(OS, Depth);
  OS << "Number of switches: " << NumSwitches << "\n";

  Lp->indent(OS, Depth);
  OS << "Number of forward gotos: " << NumForwardGotos << "\n";

  Lp->indent(OS, Depth);
  OS << "Number of forward goto target labels: " << NumLabels << "\n";

  Lp->indent(OS, Depth);
  OS << "Number of user calls: " << NumUserCalls << "\n";

  Lp->indent(OS, Depth);
  OS << "Number of indirect calls: " << NumIndirectCalls << "\n";

  Lp->indent(OS, Depth);
  OS << "Number of intrinsics: " << NumIntrinsics << "\n";
}

const LoopStatistics &
HIRLoopStatistics::getSelfLoopStatistics(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  auto LSIt = SelfStatisticsMap.find(Loop);

  // Return cached statistics, if present.
  if (LSIt != SelfStatisticsMap.end()) {
    return LSIt->second;
  }

  LoopStatistics SelfStats;
  LoopStatistics::LoopStatisticsVisitor LSV(*this, Loop, &SelfStats, nullptr);

  LSV.compute();

  auto SelfPair = SelfStatisticsMap.insert(std::make_pair(Loop, SelfStats));

  return SelfPair.first->second;
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

  // Check if self statistics also needs to be computed. If so, we compute both
  // together to avoid traversing the loop body twice.
  bool HasSelfStats = SelfStatisticsMap.count(Loop);

  LoopStatistics SelfStats, TotalStats;
  LoopStatistics::LoopStatisticsVisitor LSV(
      *this, Loop, HasSelfStats ? nullptr : &SelfStats, &TotalStats);

  LSV.compute();

  // We need to retrieve the self stats of the loop again as previous DenseMap
  // entry might have been invalidated by the traversal (by creating new entries
  // in the map). insert() doesn't override existing entry so it is okay to
  // invoke it using empty SelfStats.
  auto SelfPair = SelfStatisticsMap.insert(std::make_pair(Loop, SelfStats));

  TotalStats += SelfPair.first->second;

  auto TotalPair = TotalStatisticsMap.insert(std::make_pair(Loop, TotalStats));

  return TotalPair.first->second;
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
