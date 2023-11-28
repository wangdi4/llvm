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
// - Only a simple profitability check.

#include "llvm/Transforms/Intel_LoopTransforms/HIRNonPerfectNestLoopBlockingPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeIterator.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

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

static cl::opt<int>
    DefaultStripmineSize(OPT_SWITCH "-stripmine-size", cl::init(2048),
                         cl::ReallyHidden,
                         cl::desc("Preset stripmine size for " OPT_DESC));

static cl::opt<bool> DisableTransform("disable-rewrite-" OPT_SWITCH,
                                      cl::init(false), cl::Hidden,
                                      cl::desc("Only check " OPT_DESC "."));

static cl::opt<std::string>
    FilterFunc(OPT_SWITCH "-filter-func", cl::ReallyHidden,
               cl::desc("Run " OPT_DESC " only on the specified function."));

static cl::opt<unsigned> MinProfitableConstTC(
    OPT_SWITCH "-min-const-tc", cl::ReallyHidden, cl::init(400),
    cl::desc("Minimum constant trip counts for " OPT_DESC "."));

static bool
doTransForNonDimMatchingLoops(const LoopToDimInfoTy &InnermostLoopToDimInfos,
                              const LoopToConstRefTy &InnermostLoopToRepRef,
                              HLLoop *OutermostLoop, HIRDDAnalysis &DDA) {

  if (DisableTransform) {
    LLVM_DEBUG(dbgs() << "Transformation is disabled.\n");

    return false;
  }

  // It is known that OutermostLoop is a NonDimMatching Loop
  HLNode *NodeOutsideByStrip = OutermostLoop->getParent();

  StringRef FuncName = (OutermostLoop->getHLNodeUtils())
                           .getHIRFramework()
                           .getFunction()
                           .getName();

  // Dummy input as Shift amount is not needed.
  InnermostLoopToShiftTy InnermostLoopToShiftVec(0);
  unsigned Size = InnermostLoopToDimInfos.begin()->second.size();

  // Currently, just default stripmine size.
  SmallVector<unsigned, 4> PreSetStripmineSizes(Size, DefaultStripmineSize);

  return Transformer(PreSetStripmineSizes, InnermostLoopToDimInfos,
                     InnermostLoopToRepRef, InnermostLoopToShiftVec,
                     NodeOutsideByStrip, DDA, FuncName)
      .rewrite(false /* ClongDVLoads */, false /* AlignLoops */);
}

namespace {
// This checker only works on the following structure.
//
// Loop L1<---- LCA loop of all its innermost loops
//   Loopnest L2_1
//   ...
//   Loopnest L2_2
//   ...
//   Loopnest L2_k
// End Loop L1
//
// Following conditions should be met.
// 1. Depths of all inner loopnest L2_1 through L2_k should be the same.
// 2. Loop L1 should be the LCA of Loopness L2_1 through L2_k.
//    Thus, L1 is the LCA of all innermost loops.
// There can be if-stmt or switch stmt within L1 loop and some codes not in
// loopnests L2_1 through L2_k.
//
// It assumes after loop tiling, all generated ByStrip loops will be
// placed outside LCALoop.
//
class LegalityChecker : public HLNodeVisitorBase {
public:
  typedef DDRefGatherer<DDRef, AllRefs ^ (ConstantRefs | GenericRValRefs |
                                          IsAddressOfRefs)>
      RefGatherer;

public:
  LegalityChecker(HIRDDAnalysis &DDA,
                  const SmallVectorImpl<HLLoop *> &InnermostLoops,
                  HLLoop *LCALoop)
      : BailedOut(false), SkipNode(nullptr), DDA(DDA),
        InnermostLoops(InnermostLoops), LCALoop(LCALoop),
        OutermostLevel(LCALoop->getNestingLevel()),
        InnermostLevel(InnermostLoops.front()->getNestingLevel()) {}

  // The main entry point for verifying legality.
  // If successful, result is returned on the two output parameters.
  // These output parameters are used later as input to the class Transformer's
  // object.
  bool run(LoopToDimInfoTy &InnermostLoopToDimInfo,
           LoopToConstRefTy &InnermostLoopToRepRef);

  // Visit function for loop is only to skip the innermost loop.
  // Thus, using visit functions codes outside of the innermost loops
  // are visited.
  void visit(HLLoop *);
  void visit(HLNode *) {}
  void visit(HLDDNode *);

  void postVisit(HLNode *) {}

  bool isDone() const { return isNotOK(); }
  bool skipRecursion(HLNode *Node) const { return SkipNode == Node; }

private:
  bool isNotOK() const { return BailedOut; }
  void setNotOK(const DDRef *Ref = nullptr) {
    BailedOut = true;

    LLVM_DEBUG(if (Ref) {
      dbgs() << "Stopping Ref: ";
      Ref->dump(1);
      dbgs() << "\n";
    });
  }

  // Returns true if all Lp in Loops has the same bounds.
  bool static haveSameLoopBounds(ArrayRef<HLLoop *> Loops);
  bool haveSameLoopBounds() const;

  // Returns true if there is no backward flow edges.
  // Some flow forward temp dependences can also preventing loop blocking.
  // For example, a sibling loops temp definition is used in the next
  // sibling loops' body. This edge is forward flow.
  // We check additionally all loops doesn't have live-outs, which will
  // catch such loop-blocking preventing edges.
  template <typename T> bool hasBackwardTempDeps(ArrayRef<T *> Refs) const;

  bool hasMemRefDepsPreventingLoopBlocking(ArrayRef<DDRef *> Refs,
                                           DimInfoVecTy &DimInfoVec) const;

  // visitor logic is used for this function.
  bool isValidOutsideInnermostLoops() {
    HLNodeUtils::visitRange(*this, LCALoop->child_begin(),
                            LCALoop->child_end());

    // No early-exit means it is OK.
    return !isNotOK();
  }

  // Verify if the given InnermostLoop is legal. If so computes its dimension
  // info to DimInfo and returns a representative memref.
  RegDDRef *isValidInnermostLoop(HLLoop *InnermostLoop, DimInfoVecTy &DimInfo);

  // Helper function for isValidInnermostLoop
  // Computes DimInfoVec using interloopblocking::InnermostLoopAnalyzer is used.
  // Retures true if DimInfoVec is valid.
  bool hasValidDims(const RegDDRef *Ref, DimInfoVecTy &DimInfoVec) const;

  // Notice RepRef* is not actively used later on.
  // and almost dummy information.
  // However, current setup of transformation requires that info for contructing
  // an object of class Transformer.
  // TODO: clean up HIRInterLoopBlocking's class Transformer so that an empty
  //       InnermostToRefRep can be passed when no-alignment or no-extra
  //       guardings are needed.
  RegDDRef *getRepMemRef(ArrayRef<DDRef *> Refs);

public:
  bool BailedOut;
  HLNode *SkipNode;

private:
  HIRDDAnalysis &DDA;
  const SmallVectorImpl<HLLoop *> &InnermostLoops;
  HLLoop *LCALoop;
  unsigned OutermostLevel;
  unsigned InnermostLevel;
};

} // namespace

bool LegalityChecker::run(LoopToDimInfoTy &InnermostLoopToDimInfo,
                          LoopToConstRefTy &InnermostLoopToRepRef) {

  // Check the equality of LB/UB of innermost loops
  // If equal, use DDG directly, otherwise, skip it.
  // All the innermost loops are already have the same depth.
  if (!haveSameLoopBounds()) {
    LLVM_DEBUG(dbgs() << "Loop bounds of sibling loops are different.\n");
    return false;
  }

  // Check LCALoop outside of innermost loops
  if (!isValidOutsideInnermostLoops()) {
    LLVM_DEBUG(dbgs() << "Not Passed legality check in outside inner loop.\n");
    return false;
  }

  // Check each innermost loop.
  // InnermostLoops have memrefs where DimInfo can be obtained.
  for (auto *Lp : InnermostLoops) {

    DimInfoVecTy DimInfoVec;

    RegDDRef *RepRef = isValidInnermostLoop(Lp, DimInfoVec);
    if (!RepRef) {
      LLVM_DEBUG(dbgs() << "Not Passed legality check in a innermost loop.\n");
      return false;
    }

    InnermostLoopToDimInfo.emplace_back(Lp, DimInfoVec);
    InnermostLoopToRepRef.emplace(Lp, RepRef);
  }

  return true;
} // end of run()

void LegalityChecker::visit(HLLoop *Loop) {

  // Visitor visits Preheader/Postexit without Loop.
  if (Loop->isInnermost()) {
    // Innermost loops are considered separately.
    SkipNode = Loop;
    return;
  }
}

void LegalityChecker::visit(HLDDNode *DDNode) {

  // For non-innermost loops only
  // Thus, only temp refs are considered.
  // By this time, non-existence of memrefs outside of innermost
  // loops are verified.
  if (hasBackwardTempDeps(
          ArrayRef<RegDDRef *>(DDNode->ddref_begin(), DDNode->ddref_end()))) {
    setNotOK();
  }
}

RegDDRef *LegalityChecker::isValidInnermostLoop(HLLoop *InnermostLoop,
                                                DimInfoVecTy &DimInfoVec) {

  RefGatherer::VectorTy Refs;

  RefGatherer::gatherRange(InnermostLoop->child_begin(),
                           InnermostLoop->child_end(), Refs);

  if (hasBackwardTempDeps<DDRef>(Refs)) {
    setNotOK();
    return nullptr;
  }

  if (hasMemRefDepsPreventingLoopBlocking(Refs, DimInfoVec)) {
    setNotOK();
    return nullptr;
  }

  RegDDRef *RepRef = getRepMemRef(Refs);
  LLVM_DEBUG(dbgs() << "RepMemRef: " << (RepRef ? "non-null" : "null") << "\n");

  return RepRef;
}

RegDDRef *LegalityChecker::getRepMemRef(ArrayRef<DDRef *> Refs) {

  DDGraph DDG = DDA.getGraph(LCALoop);

  for (auto *Ref : Refs) {

    RegDDRef *RegRef = dyn_cast<RegDDRef>(Ref);

    if (!RegRef)
      continue;

    if (!RegRef->isMemRef())
      continue;

    // Presence of outgoing edge
    if (!DDG.getNumOutgoingEdges(RegRef))
      continue;

    return RegRef;
  }

  // When there is no memref with an outgoing edge,
  // nullptr can be returned.
  return nullptr;
}

template <typename T>
bool LegalityChecker::hasBackwardTempDeps(ArrayRef<T *> Refs) const {

  DDGraph DDG = DDA.getGraph(LCALoop);

  for (auto *Ref : Refs) {

    // TempRefs
    if (!Ref->isTerminalRef() || !Ref->isLval())
      continue;

    // Outgoing edges
    for (auto *Edge : DDG.outgoing(Ref)) {
      LLVM_DEBUG(dbgs() << "CHECK Edge: \n");
      LLVM_DEBUG(Edge->dump());
      if (!Edge->isFlow())
        continue;

      // flow edge
      if (Edge->isBackwardDep()) {
        LLVM_DEBUG(dbgs() << "Edge is Flow and Backward\n");
        LLVM_DEBUG(Edge->dump());
        return true;
      }
    }
  }

  return false;
}

bool LegalityChecker::hasMemRefDepsPreventingLoopBlocking(
    ArrayRef<DDRef *> Refs, DimInfoVecTy &DimInfoVec) const {

  DDGraph DDG = DDA.getGraph(LCALoop);

  // MemRefs
  for (auto *Ref : Refs) {

    const RegDDRef *RegRef = dyn_cast<RegDDRef>(Ref);

    if (!RegRef)
      continue;

    if (!RegRef->isMemRef())
      continue;

    // Outgoing edges
    for (auto *Edge : DDG.outgoing(RegRef)) {
      const RegDDRef *DestRef = cast<RegDDRef>(Edge->getSink());

      // This is only structural check.
      if (!DDRefUtils::areEqual(cast<RegDDRef>(Edge->getSrc()), DestRef))
        return true;

      // For the outermost level - stride can be zero
      // Exclude edges like A[0]:A[0], A[%t]:A[%t]
      // This check may not catch MIV such as A[i1 + i2].
      // Other than that it can give more opportunity.
      // Example: A[i1][i1], A[i2][i1] and so on.
      // Current transformation logic may have limitation with such refs.
      // Check for all inner-loop levels.
      for (unsigned Level = OutermostLevel + 1; Level <= InnermostLevel;
           Level++) {
        int64_t Stride = 0;
        bool ConstStride = DestRef->getConstStrideAtLevel(Level, &Stride);
        if (!ConstStride || Stride == 0)
          return true;
      }
    }

    // Check for the source RegRef itself.
    for (unsigned Level = OutermostLevel + 1; Level <= InnermostLevel;
         Level++) {
      int64_t Stride = 0;
      bool ConstStride = RegRef->getConstStrideAtLevel(Level, &Stride);
      if (!ConstStride || Stride == 0)
        return true;
    }

    DimInfoVec.clear();
    if (!hasValidDims(RegRef, DimInfoVec)) {
      return true;
    }
  }

  return false;
}

bool LegalityChecker::hasValidDims(const RegDDRef *Ref,
                                   DimInfoVecTy &DimInfoVec) const {

  // interloopblocking::InnermostLoopAnalyzer::collectDimInfo()
  // returns false if Ref has an OutermostLevel's IV.
  // This is not necessary and can be extended eventually.
  bool Collected = InnermostLoopAnalyzer::collectDimInfo(
      Ref, LCALoop->getNestingLevel(), InnermostLevel, DimInfoVec);
  if (!Collected) {
    LLVM_DEBUG(dbgs() << "Invalid Dimension info.\n");
    return false;
  }

  assert(!DimInfoVec.empty() && "DimInfo should have been obtained.");

  // Following conditions are verified.
  // 1. At least one dim with IV - all const, all blob are not valid.
  //    This is guaranteed by getConstStiredAtLevel & non zero stride.

  // 2. No Dim has more than one IV
  //    Checked by collectDimInfo()

  // 3. No IVs appear more than one dimension
  //    - this can be relaxed

  // 4. IVs are in increasing order of dimension (A[i1][i2][i3])
  //    - this can be relaxed

  // For simplicity, avoid Ref with any Konst or Blob dim.
  for (auto &DimInfo : DimInfoVec)
    if (DimInfo == DimInfoTy::KONST || DimInfo == DimInfoTy::BLOB) {
      LLVM_DEBUG(dbgs() << "Invalid Dimension info with blob or const dim.\n");
      return false;
    }

  return true;
}

bool LegalityChecker::haveSameLoopBounds(ArrayRef<HLLoop *> Loops) {

  return std::all_of(Loops.begin(), Loops.end(), [Loops](const HLLoop *Lp) {
    return CanonExprUtils::areEqual(Loops[0]->getUpperCanonExpr(),
                                    Lp->getUpperCanonExpr()) &&
           CanonExprUtils::areEqual(Loops[0]->getLowerCanonExpr(),
                                    Lp->getLowerCanonExpr());
  });
}

bool LegalityChecker::haveSameLoopBounds() const {

  // Check the loopbounds of innermostLoops
  if (!haveSameLoopBounds(InnermostLoops))
    return false;

  const HLLoop *FirstParLoop = InnermostLoops.front()->getParentLoop();
  if (FirstParLoop == LCALoop) {
    return true;
  }

  // Check parents at lower levels deeper than LCALoop's level
  unsigned Size = InnermostLoops.size();
  SmallVector<HLLoop *, 16> ParLoops(Size, nullptr);
  for (unsigned I = 0; I < Size; I++)
    ParLoops[I] = InnermostLoops[I]->getParentLoop();

  while (ParLoops.front() && ParLoops.front() != LCALoop) {
    if (!haveSameLoopBounds(ParLoops))
      return false;

    for (unsigned I = 0; I < Size; I++)
      ParLoops[I] = ParLoops[I]->getParentLoop();
  }

  return true;
}

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
    LLVM_DEBUG(dbgs() << "Inner loops have liveouts.\n");
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
  if (Loop->hasLiveOutTemps()) {
    LLVM_DEBUG(dbgs() << "Not a candidate: ");
    LLVM_DEBUG(dbgs() << "LCALoop has live outs.\n");
    return;
  }

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

  // Check legality
  LoopToDimInfoTy InnermostLoopToDimInfo;
  LoopToConstRefTy InnermostLoopToRepRef;
  bool IsLegal = LegalityChecker(DDA, AllInnermostLoops, Loop)
                     .run(InnermostLoopToDimInfo, InnermostLoopToRepRef);

  const auto &Func = Loop->getHLNodeUtils().getHIRFramework().getFunction();
  (void)Func;

  if (!IsLegal) {
    LLVM_DEBUG(dbgs() << "Not a legal candidate.\n");
    LLVM_DEBUG(dbgs() << "Function: ";
               if (Func.hasName()) dbgs() << Func.getName() << "\n");
    return;
  }

  LLVM_DEBUG_PROFIT_REPORT(dbgs() << "Candidate loopnest in function ");
  LLVM_DEBUG_PROFIT_REPORT(if (Func.hasName()) dbgs()
                               << Func.getName() << ":\n";);
  LLVM_DEBUG_PROFIT_REPORT(Loop->dump());

  // Rewrite.
  bool Result = doTransForNonDimMatchingLoops(InnermostLoopToDimInfo,
                                              InnermostLoopToRepRef, Loop, DDA);

  LLVM_DEBUG(dbgs() << "Transformation was " << (Result ? "done" : "not done")
                    << "\n");
  (void)Result;
  if (Result) {
    LLVM_DEBUG_PROFIT_REPORT(
        dbgs() << "In Function - ";
        if (Func.hasName()) dbgs() << Func.getName() << ", ";
        dbgs() << "NonPerfectNestLoopBlocking was done:\n");
    LLVM_DEBUG_PROFIT_REPORT(Loop->getParentRegion()->dump());
  }
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
