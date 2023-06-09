#if INTEL_FEATURE_SW_ADVANCED
// ===- HIRNonPerfectNestLoopBlocking.cpp - Blocking non-perfect loopnests -==//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//
//===----------------------------------------------------------------------===//
//
// This pass has the similarity to HIRInterLoopBlocking pass.
// 1. This pass aims to do the loop tiling over non-perfect loopnest.
// 2. This pass potentially can apply loop tiling over multiple sibling
//    loopnests.
//
// Main differences are
// 1. This pass finds a single enclosing loop containing all sibling loop nests.
//    Upon transformation, around that single eclosing loop tiling's floor loops
//    (i.e. byStripLoops) will be placed.
// 2. Extending 1, this pass's profiltability model is similar to KandR
// algorithm
//    of the regular loop blocking pass. The single enclosing loop is the loop,
//    where its iduction variable is missing in memrefs.
// 3. This pass is enabled for C inputs.
//
// Example.
// From
//
// L1: Do i1 = LB1, UB1, 1
//    ....
//    L2: Do i2 = LB2, UB2, 1
//
//    L3: Do i2 = LB2, UB2, 1
//
// -->
// To
//
// S is stripmine size - M1 is ByStrip loop of i2-loops (L2, L3)
// M1: Do i1 = LB2, UB2, S
//   L1: Do i2 =  LB1, UB1, 1
//      ....
//      L2: Do i3 = i1, min((i1 + S - 1), UB), 1
//
//      L3: Do i3 = i1, min((i1 + S - 1), UB), 1
//
// Notice L2, and L3 could be in a control flow preventing them
// to be merged. (E.g. if - else)
//
// Current limitations
// - It works only 2-level loop nests.

#include "llvm/Transforms/Intel_LoopTransforms/HIRNonPerfectNestLoopBlockingPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRArraySectionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeIterator.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRInterLoopBlocking.h"

#define OPT_SWITCH "hir-non-perfect-nest-loop-blocking"
#define OPT_DESC "HIR non-perfect-nest loop blocking"

#define DEBUG_TYPE OPT_SWITCH
#define LLVM_DEBUG_PROFIT_REPORT(X) DEBUG_WITH_TYPE(OPT_SWITCH "-profit", X)

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::interloopblocking;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden, cl::desc("Disable " OPT_DESC "."));

static cl::opt<std::string>
    FilterFunc(OPT_SWITCH "-filter-func", cl::ReallyHidden,
               cl::desc("Run " OPT_DESC " only on the specified function."));

static cl::opt<unsigned> MinProfitableConstTC(
    OPT_SWITCH "-min-const-tc", cl::ReallyHidden, cl::init(400),
    cl::desc("Minimum constant trip counts for " OPT_DESC "."));

namespace {

// Given OutermostLoop, this class collects all of its inner loops
// if the following conditions hold.
// 1. The loopnest depth of OutermostLoop to its innermost loops is 2.
// 2. OutermostLoop shouldn't be unknown and innermost loops should be DO loops.
// 3. Per other HL structure, more constraints are shown in visit functions.
class InnerDoLoopCollector : public HLNodeVisitorBase {
public:
  InnerDoLoopCollector(SmallVectorImpl<HLLoop *> &AllInnermostLoops,
                       HLLoop *OutermostLoop)
      : AllInnermostLoops(AllInnermostLoops), OutermostLoop(OutermostLoop),
        OutermostLevel(OutermostLoop->getNestingLevel()), BailedOut(false) {}

  bool collect() {
    HLNodeUtils::visitRange(*this, OutermostLoop->child_begin(),
                            OutermostLoop->child_end());

    return isSuccessful();
  }

  void visit(HLLoop *Loop);
  void visit(HLInst *);
  void visit(HLGoto *);
  void visit(HLSwitch *);
  void visit(HLNode *){};
  void postVisit(HLNode *Node){};

  bool isDone() { return isBailedOut(); }

private:
  void setBailedOut() { BailedOut = true; }
  bool isBailedOut() const { return BailedOut; }
  bool isSuccessful() const { return !isBailedOut(); }

  static bool isProfitableTripCount(const HLLoop *Loop);

private:
  SmallVectorImpl<HLLoop *> &AllInnermostLoops;

  HLLoop *OutermostLoop;
  unsigned OutermostLevel;
  bool BailedOut;
};

} // namespace

void InnerDoLoopCollector::visit(HLLoop *Loop) {
  // Inner loops shouldn't be deeper level than 2.
  if (Loop->getNestingLevel() > 2) {
    setBailedOut();
    return;
  }

  // TODO: consider avoiding RuntimeDD'ed innermost loop.
  //       Blender had many candidates passed current legality checks
  //       with runtimeDDed innermost loops.

  if (!Loop->isDo()) {
    setBailedOut();
    return;
  }

  if (Loop->hasLiveOutTemps()) {
    setBailedOut();
    return;
  }

  const auto *UBCE = Loop->getUpperCanonExpr();
  if (!UBCE->isInvariantAtLevel(OutermostLevel)) {
    LLVM_DEBUG(dbgs() << "Inner loops bound is not invariant at outer level.");
    setBailedOut();
    return;
  }

  // This more of profitability check.
  if (!isProfitableTripCount(Loop)) {
    LLVM_DEBUG(dbgs() << "Inner loops trip count is too small for blocking.");
    setBailedOut();
    return;
  }

  AllInnermostLoops.push_back(Loop);
}

bool InnerDoLoopCollector::isProfitableTripCount(const HLLoop *Loop) {

  uint64_t TripCount;
  if (Loop->isConstTripLoop(&TripCount)) {
    if (TripCount < MinProfitableConstTC)
      return false;
  } else {
    // Unknown sizes are regared as profitable.
    unsigned MaxTCEst = Loop->getMaxTripCountEstimate();
    if (MaxTCEst && MaxTCEst < MinProfitableConstTC)
      return false;
  }

  return true;
}

void InnerDoLoopCollector::visit(HLGoto *HGoto) {
  const HLLoop *ParLoop = HGoto->getParentLoop();
  assert(ParLoop && "Parent loop should exists!\n");

  if (ParLoop != OutermostLoop) {
    LLVM_DEBUG(dbgs() << "Inner loops have Goto is not for unreachable.");
    setBailedOut();
    return;
  }

  // Check if its target is unreachable
  if (!HGoto->getTargetBBlock() ||
      !isa<UnreachableInst>(HGoto->getTargetBBlock()->getTerminator())) {
    LLVM_DEBUG(dbgs() << "Goto is not for unreachable.");
    setBailedOut();
    return;
  }
}

void InnerDoLoopCollector::visit(HLInst *HInst) {
  if (HInst->isCallInst()) {
    // We avoid any call what so ever.
    setBailedOut();
    return;
  }

  // Consider Preheaders of innermost
  const HLLoop *ParLoop = HInst->getLexicalParentLoop();
  assert(ParLoop && "Inst should have a parent loop!\n");
  if (ParLoop->isInnermost())
    return;

  // No memrefs except in the innermost loop
  // Only innermost loops are allowed to have memrefs.
  for (const auto *OperRef : HInst->op_ddrefs())
    if (OperRef->isMemRef()) {
      setBailedOut();
      return;
    }
}

void InnerDoLoopCollector::visit(HLSwitch *HSwitch) {
  const HLLoop *ParLoop = HSwitch->getParentLoop();
  assert(ParLoop && "Switch should have parent loop!\n");

  if (ParLoop->isInnermost()) {
    setBailedOut();
    return;
  }
}

namespace {

class CandidateVisitor : public HLNodeVisitorBase {
public:
  CandidateVisitor(HIRDDAnalysis &DDA) : DDA(DDA), SkipNode(nullptr) {}

  void run(HLRegion *Reg) {
    HLNodeUtils::visitRange(*this, Reg->child_begin(), Reg->child_end());
  }

  void visit(HLLoop *Loop);
  void visit(HLNode *Node){};
  void postVisit(HLNode *Node){};

  bool skipRecursion(const HLNode *Node) const { return SkipNode == Node; }

public:
  HIRDDAnalysis &DDA;

  HLNode *SkipNode;
};

} // namespace

void CandidateVisitor::visit(HLLoop *Loop) {

  // This visitor doesn't traverse outerloop further.
  // Inner loop visitor runs separately.
  SkipNode = Loop;

  // Can be done with gatherOutermostLoops().
  // However, no need to maintain the vector of outermost loops
  unsigned Level = Loop->getNestingLevel();
  if (Level > 1) {
    llvm_unreachable("Top Level should only visit only outermost loops");
  }

  // This collector is strictly looking for 2-level nested loop,
  // where the outermost loop's level is 1.
  // This is 1-level loopnest. No opportunity for loop tiling.
  if (Loop->isInnermost())
    return;
  
  // TODO: This is not a general assumption. Could be removed eventually.
  HLNode *ParentNode = Loop->getParent();
  if (!isa<HLRegion>(ParentNode))
    return;

  // We don't consider outermost loop with liveout.
  // This is a conservative check.
  if (Loop->hasLiveOutTemps())
    return;

  // Do nothing for Unknown Loops
  // MultiExit loops are possbile in case all the goto's target is unreachable.
  if (Loop->isUnknown()) {
    LLVM_DEBUG(dbgs() << "Not a candidate: ");
    LLVM_DEBUG(dbgs() << "LCALoop is a UnKnown-loop.\n");
    LLVM_DEBUG(
        dbgs()
        << "Function: "
        << (Loop->getHLNodeUtils()).getHIRFramework().getFunction().getName()
        << "\n");
    return;
  }

  // Level 1 loop
  SmallVector<HLLoop *, 8> AllInnermostLoops;
  InnerDoLoopCollector ValidInnerloopCollector(AllInnermostLoops, Loop);
  if (!ValidInnerloopCollector.collect()) {
    LLVM_DEBUG(dbgs() << "No candidate was found in this region.\n");
    return;
  }
  assert(!AllInnermostLoops.empty() &&
         "There should be at least one innermost loop");

  // TODO: Add legality (profitablity) checker.
  //       Add rewrite logic.

  LLVM_DEBUG_PROFIT_REPORT(dbgs() << "Candidate loopnest in function ");
  LLVM_DEBUG_PROFIT_REPORT(
      const auto &Func = Loop->getHLNodeUtils().getHIRFramework().getFunction();
      if (Func.hasName()) dbgs() << Func.getName() << ": \n";);
  LLVM_DEBUG_PROFIT_REPORT(Loop->dump());
}

void driver(HIRFramework &HIRF, HIRDDAnalysis &DDA, TargetTransformInfo &TTI,
            HIRLoopStatistics &HLS, const Function &F) {

  if (!TTI.isLibIRCAllowed())
    return;

  if (!FilterFunc.empty() && !F.getName().equals(FilterFunc))
    return;

  for (auto &Region : HIRF.regions()) {
    CandidateVisitor Visitor(DDA);
    Visitor.run(cast<HLRegion>(&Region));
  }
}

PreservedAnalyses HIRNonPerfectNestLoopBlockingPass::runImpl(
    Function &F, FunctionAnalysisManager &AM, HIRFramework &HIRF) {

  if (DisablePass)
    return PreservedAnalyses::all();

  driver(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
         AM.getResult<TargetIRAnalysis>(F),
         AM.getResult<HIRLoopStatisticsAnalysis>(F), F);
  return PreservedAnalyses::all();
}
#endif // INTEL_FEATURE_SW_ADVANCED
