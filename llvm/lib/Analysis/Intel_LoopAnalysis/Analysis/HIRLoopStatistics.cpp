//===--------- HIRLoopStatistics.cpp - Computes loop statisticss ----------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

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
  return HIRLoopStatistics(AM.getResult<HIRFrameworkAnalysis>(F),
                           AM.getResult<TargetLibraryAnalysis>(F));
}

char HIRLoopStatisticsWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopStatisticsWrapperPass, "hir-loop-statistics",
                      "Loop Statistics Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(HIRLoopStatisticsWrapperPass, "hir-loop-statistics",
                    "Loop Statistics Analysis", false, true)

void HIRLoopStatisticsWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRFrameworkWrapperPass>();
  AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
}

bool HIRLoopStatisticsWrapperPass::runOnFunction(Function &F) {
  HLS.reset(new HIRLoopStatistics(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F)));
  return false;
}

void HIRLoopStatisticsWrapperPass::releaseMemory() { HLS.reset(); }

struct LoopStatistics::LoopOrRegionStatisticsVisitor final
    : public HLNodeVisitorBase {
  HIRLoopStatistics &HLS;
  const HLNode *Node;
  LoopStatistics *SelfStats;
  LoopStatistics *ChildrenStats;

  LoopOrRegionStatisticsVisitor(HIRLoopStatistics &HLS, const HLNode *Node,
                                LoopStatistics *SelfStats,
                                LoopStatistics *ChildrenStats)
      : HLS(HLS), Node(Node), SelfStats(SelfStats),
        ChildrenStats(ChildrenStats) {
    assert((SelfStats || ChildrenStats) &&
           "At least one of self/children stats should be present!");
  }

  // Main function to compute loop/region statistics.
  void compute() {
    // Do not directly recurse inside children loops. Children statistics is
    // recursively computed for children loops by the visitor using
    // getTotalStatistics().

    if (auto *Region = dyn_cast<HLRegion>(Node)) {
      HLNodeUtils::visitRange<true, false>(*this, Region->child_begin(),
                                           Region->child_end());
    } else {
      auto *Lp = dyn_cast<HLLoop>(Node);
      assert(Lp && "Expected Region or Loop!");
      HLNodeUtils::visitRange<true, false>(*this, Lp->child_begin(),
                                           Lp->child_end());
    }
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
      SelfStats->ForwardGotos.push_back(Goto);
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

    bool IsSIMDDir = false;
    auto *Call = HInst->getCallInst();

    if (Call) {
      if (auto *Intr = dyn_cast<IntrinsicInst>(Call)) {
        SelfStats->NumIntrinsics++;

        auto IntrinsicId = Intr->getIntrinsicID();
        if (isTriviallyVectorizable(IntrinsicId)) {
          SelfStats->NumProfitableVectorizableCalls++;
        }

        if (HInst->isSIMDDirective() || HInst->isSIMDEndDirective()) {
          IsSIMDDir = true;
        }

      } else {
        SelfStats->NumUserCalls++;

        if (auto *Func = Call->getCalledFunction()) {
          if (HLS.TLI.isFunctionVectorizable(Func->getName())) {
            SelfStats->NumProfitableVectorizableCalls++;
          }
        } else if (Call->isIndirectCall()) {
          SelfStats->NumIndirectCalls++;
        }
      }

      bool HasUnsafeSideEffects = HLInst::hasUnsafeSideEffects(Call);

      if (!IsSIMDDir && HasUnsafeSideEffects) {
        SelfStats->HasNonSIMDCallsWithUnsafeSideEffects = true;
      }

      SelfStats->HasCallsWithUnsafeSideEffects |= HasUnsafeSideEffects;

      SelfStats->HasCallsWithNoDuplicate |= Call->cannotDuplicate();
      SelfStats->HasConvergentCalls |= Call->isConvergent();

      SelfStats->HasCallsWithUnknownAliasing |=
          HLInst::hasUnknownAliasing(Call);
    }
  }

  void visit(const HLLoop *Lp) {
    if (ChildrenStats) {
      *ChildrenStats += HLS.getTotalStatistics(Lp);
    }
  }

  void visit(const HLNode *Node) {
    llvm_unreachable("Unexpected HLNode type encountered!");
  }

  void postVisit(const HLNode *Node) {}
};

void LoopStatistics::print(formatted_raw_ostream &OS,
                           const HLNode *Node) const {

  unsigned Depth = 0;
  // Indent at one level more than the loop nesting level.
  if (auto *Lp = dyn_cast<HLLoop>(Node)) {
    Depth += Lp->getNestingLevel() + 1;
  }

  Node->indent(OS, Depth);
  OS << "Number of ifs: " << NumIfs << "\n";

  Node->indent(OS, Depth);
  OS << "Number of switches: " << NumSwitches << "\n";

  Node->indent(OS, Depth);
  OS << "Number of forward gotos: " << getNumForwardGotos() << "\n";

  Node->indent(OS, Depth);
  OS << "Number of forward goto target labels: " << getNumLabels() << "\n";

  Node->indent(OS, Depth);
  OS << "Number of user calls: " << NumUserCalls << "\n";

  Node->indent(OS, Depth);
  OS << "Number of indirect calls: " << NumIndirectCalls << "\n";

  Node->indent(OS, Depth);
  OS << "Number of intrinsics: " << NumIntrinsics << "\n";

  Node->indent(OS, Depth);
  OS << "Number of profitable vectorizable calls: "
     << NumProfitableVectorizableCalls << "\n";

  Node->indent(OS, Depth);
  OS << "Has unsafe calls: "
     << (HasCallsWithUnsafeSideEffects ? "yes\n" : "no\n");

  Node->indent(OS, Depth);
  OS << "Has non-SIMD unsafe calls: "
     << (HasNonSIMDCallsWithUnsafeSideEffects ? "yes\n" : "no\n");

  Node->indent(OS, Depth);
  OS << "Has noduplicate calls: "
     << (HasCallsWithNoDuplicate ? "yes\n" : "no\n");

  Node->indent(OS, Depth);
  OS << "Has convergent calls: " << (HasConvergentCalls ? "yes\n" : "no\n");

  Node->indent(OS, Depth);
  OS << "Has unknown aliasing calls: "
     << (HasCallsWithUnknownAliasing ? "yes\n" : "no\n");
}

void LoopStatistics::sortGotos() {
  assert(getNumForwardGotos() > 1 && "More than 1 gotos expected!");

  std::sort(ForwardGotos.begin(), ForwardGotos.end(),
            [](const HLGoto *A, const HLGoto *B) -> bool {
              return A->getTopSortNum() < B->getTopSortNum();
            });
}

const LoopStatistics &
HIRLoopStatistics::getSelfStatisticsImpl(const HLNode *Node) {
  assert(isa<HLLoop>(Node) ||
         isa<HLRegion>(Node) && " Invalid Node for Statistics.");

  auto LSIt = SelfStatisticsMap.find(Node);

  // Return cached statistics, if present.
  if (LSIt != SelfStatisticsMap.end()) {
    return LSIt->second;
  }

  LoopStatistics SelfStats;
  LoopStatistics::LoopOrRegionStatisticsVisitor LSV(*this, Node, &SelfStats,
                                                    nullptr);

  LSV.compute();

  auto SelfPair = SelfStatisticsMap.insert(std::make_pair(Node, SelfStats));

  return SelfPair.first->second;
}

const LoopStatistics &
HIRLoopStatistics::getTotalStatisticsImpl(const HLNode *Node) {
  assert(isa<HLLoop>(Node) ||
         isa<HLRegion>(Node) && " Invalid Node for Statistics.");

  auto *Lp = dyn_cast<HLLoop>(Node);

  // Self and total Node statistics for innermost loops are the same.
  if (Lp && Lp->isInnermost()) {
    return getSelfStatisticsImpl(Node);
  }

  auto LSIt = TotalStatisticsMap.find(Node);

  // Return cached statistics, if present.
  if (LSIt != TotalStatisticsMap.end()) {
    return LSIt->second;
  }

  // Check if self statistics also needs to be computed. If so, we compute both
  // together to avoid traversing the Node body twice.
  bool HasSelfStats = SelfStatisticsMap.count(Node);

  LoopStatistics SelfStats, TotalStats;
  LoopStatistics::LoopOrRegionStatisticsVisitor LSV(
      *this, Node, HasSelfStats ? nullptr : &SelfStats, &TotalStats);

  LSV.compute();

  // We need to retrieve the self stats of the Node again as previous DenseMap
  // entry might have been invalidated by the traversal (by creating new entries
  // in the map). insert() doesn't override existing entry so it is okay to
  // invoke it using empty SelfStats.
  auto SelfPair = SelfStatisticsMap.insert(std::make_pair(Node, SelfStats));

  auto &UpdatedSelfStats = SelfPair.first->second;

  // If both self and children stats (currently in TotalStats) have gotos, they
  // will not be arranged in lexical order so we need sorting.
  bool SortGotos =
      TotalStats.hasForwardGotos() && UpdatedSelfStats.hasForwardGotos();
  TotalStats += UpdatedSelfStats;

  if (SortGotos) {
    TotalStats.sortGotos();
  }

  auto TotalPair = TotalStatisticsMap.insert(std::make_pair(Node, TotalStats));

  return TotalPair.first->second;
}

void HIRLoopStatistics::print(formatted_raw_ostream &OS, const HLLoop *Loop) {
  const LoopStatistics &LS = PrintTotalStatistics ? getTotalStatisticsImpl(Loop)
                                                  : getSelfStatisticsImpl(Loop);
  LS.print(OS, Loop);
}

void HIRLoopStatistics::print(formatted_raw_ostream &OS,
                              const HLRegion *Region) {
  const LoopStatistics &LS = PrintTotalStatistics
                                 ? getTotalStatisticsImpl(Region)
                                 : getSelfStatisticsImpl(Region);
  LS.print(OS, Region);
}

void HIRLoopStatistics::markLoopBodyModified(const HLLoop *Loop) {
  // Remove current loop's self statistics from the cache.
  SelfStatisticsMap.erase(Loop);

  // Remove current and parent loops/region total statistics from the cache.
  HLRegion *Reg = Loop->getParentRegion();
  TotalStatisticsMap.erase(Reg);

  while (Loop) {
    TotalStatisticsMap.erase(Loop);
    Loop = Loop->getParentLoop();
  }
}

void HIRLoopStatistics::markNonLoopRegionModified(const HLRegion *Region) {
  assert(Region && "Node parameter is null.");
  SelfStatisticsMap.erase(Region);
}
