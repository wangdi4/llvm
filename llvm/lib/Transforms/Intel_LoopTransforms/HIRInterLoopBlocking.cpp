// ===- HIRInterLoopBlocking.cpp - Blocking over multiple loopnests -==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//
//===----------------------------------------------------------------------===//
// Profitablity checker for spatial inter-loop blocking.
// Spatial inter-loop blocking is for taking advantage of spatial locality
// across different loop nests.
// For example, in the following loop nests, the spatial locality of array
// A[i][j][k] and B[i][j][k] can be utilized through Loops L1 and L2.
// Conceptually, it is similar to the fusion of loops L1 and L2 and doing
// loop blocking over the fused loop. However, physical fusion of the loop
// bodies doesn't happen. Instead, loops L1 and L2, progress together
// in a tile-by-tile fashion, through guarded loop bounds.
//
// Input:
// L3: for ts = 0 ..  {         //-- Outermost Loop
//   L1: for i = 0, N
//         for j = 0, N
//           for k = 0, N {
//               ..
//               A[i][j][k] = B[i][j][k] + B[i+1][j][k+1];
//               ..
//            }
//
//   L2: for i = 0, N
//         for j = 0, N
//           for k = 0, N {
//               ..
//               B[i][j][k] = A[i][j][k-1] + A[i-1][j][k];
//               ..
//            }
// }
//
// Output:
// L3: for ts = 0 ..  {         //-- Outermost Loop
//   for II = 0, N, S           //-- By-strip loops enclosing both L1 and L2
//     for JJ = 0, N, S
//       for KK = 0, N, S {
//
//        L1: for i = II, min(N, II + S)
//              for j = JJ, min(N, JJ + S)
//                for k = KK, min(N, KK + S) {
//                   ..
//                   A[i][j][k] = B[i][j][k] + B[i+1][j][k+1];
//                   ..
//                 }
//
//        L2: for i = II, min(N, II + S)
//              for j = JJ, min(N, JJ + S)
//                for k = KK, min(N, KK + S) {
//                   ..
//                   B[i][j][k] = A[i][j][k-1] + A[i-1][j][k];
//                   ..
//                }
//       } // end of by-strip KK-loop
// }
//
// The existence of the outermost loop, L3, is not a requirement for
// the spatial locality of A and B across L1 and L2. However, in this pass,
// the existence of the outermost loop is ensured.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRInterLoopBlockingPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRArraySectionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeIterator.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRPrintDiag.h"
#include "HIRStencilPattern.h"

#define OPT_SWITCH "hir-inter-loop-blocking"
#define OPT_DESC "HIR Spatial blocking over multiple loopnests"
#define DEBUG_TYPE OPT_SWITCH
#define LLVM_DEBUG_PROFIT_REPORT(X) DEBUG_WITH_TYPE(OPT_SWITCH "-profit", X)

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden, cl::desc("Disable " OPT_DESC "."));

static cl::opt<std::string>
    FilterFunc(OPT_SWITCH "-filter-func", cl::ReallyHidden,
               cl::desc("Run " OPT_DESC " only on the specified function."));

namespace {

// SmallSet wanted the size be less than 32 with assertion
// SmallSet<unsigned, 64> will incur an assertion.
typedef std::set<unsigned> BasePtrIndexSetTy;

// Base vistor used for Profitablity and Legality checks.
// Collects a sequence of loop nests represented as list, [FirstSpatialLoop,
// LastSpatialLoop]. All in-between loops between First and LastSpatialLoop are
// all included in the list. Only innermost loops are marked. As an innermost
// loop is added into the list, the lowest common ancestor is maintained. When
// list, [FirstSpatialLoop, LastSpatialLoop], forms a valid sequence, it is
// guranteed that All innermost loops through First to LastSpatialLoops will
// share a lowest common ancestor, "PrevLCA". Also, it is guaranteed that
// LastSpatialLoop and a loop after it lexicographically can be cleanly cut
// without any loop distribution.
// This visitor strives to prolong the sequence of spatial loops until a
// function call is found. Exceptions are a couple of pure functions, e.g. exp,
// sin, and I/O call. I/O calls can be removed out of the loop body of the
// outermost loop (or PrevLCA) by cleaning up logic like loop peeling when
// I/O calls happen only at specific early iterations.
class CheckerVisitor : public HLNodeVisitorBase {
public:
  CheckerVisitor(HIRFramework &HIRF, HIRArraySectionAnalysis &HASA,
                 HIRDDAnalysis &DDA, StringRef FuncName)
      : SkipNode(nullptr), IsDone(false), HIRF(HIRF), HASA(HASA), DDA(DDA),
        Func(FuncName), FirstSpatialLoop(nullptr), LastSpatialLoop(nullptr),
        PrevLCA(nullptr), HasIOCall(nullptr) {}

  bool skipRecursion(const HLNode *Node) const { return SkipNode == Node; }
  bool isDone() const { return IsDone; }

  void visit(HLIf *HIf) {
    markVisited(HIf);

    bool HasLoopAncestors = any_of(
        std::next(TraversalAncestors.rbegin()), TraversalAncestors.rend(),
        [](const HLNode *Node) { return isa<HLLoop>(Node); });

    if (!HasLoopAncestors) {
      SkipNode = HIf;
    }
  }

  void visit(HLInst *HInst) {
    markVisited(HInst);

    const HLLoop *ParentLoop = HInst->getParentLoop();

    if (!ParentLoop) {
      return;
    }

    // TODO: consider to remove the check. visit(HLLoop*) is also checking
    //       the same condition. Revisit when the transformation is enabled.
    if (ParentLoop->isInnermost() && !isCleanCut(LastSpatialLoop, ParentLoop)) {
      bailOut();
      return;
    }

    // Stop at the call.
    // Some calls are exceptions.
    if (HInst->isCallInst() && !isAllowedCallInLoopBody(HInst) &&
        !isIOCall(HInst)) {
      stopAndWork(3, ParentLoop);
    }

    if (isIOCall(HInst)) {
      HasIOCall = true;
    }
  }

  void visit(HLNode *Node) { markVisited(Node); }

  void postVisit(HLRegion *Reg) {
    markPostVisited(Reg);
    reset();
  }

  void postVisit(HLLoop *Loop) {
    markPostVisited(Loop);
    if (!Loop->isInnermost() && FirstSpatialLoop != LastSpatialLoop &&
        PrevLCA == Loop) {
      stopAndWork(4, nullptr);
    }
  }

  void postVisit(HLNode *Node) { markPostVisited(Node); }

  bool hasIOCall() const { return HasIOCall; }

public:
  HLNode *SkipNode;
  bool IsDone;

protected:
  // TODO: return val bool is not being used
  virtual bool stopAndWork(int CallSiteLoc,
                           const HLLoop *StopLoop = nullptr) = 0;

  inline void markVisited(HLNode *Node) { TraversalAncestors.push_back(Node); }
  inline void markPostVisited(HLNode *Node) { TraversalAncestors.pop_back(); }

  inline void setDone(bool Val) { IsDone = Val; }

  void init() {
    FirstSpatialLoop = nullptr;
    LastSpatialLoop = nullptr;
    PrevLCA = nullptr;
    HasIOCall = false;
  }

  // Forget about the results so far.
  // And try to find next candidate.
  virtual void reset() = 0;

  // Forget about the results so far.
  // And stop trying to find another candidate. Done with this search
  // without any results.
  void bailOut() {
    setDone(true);
    reset();
  }

  // A couple of pure functions are allowed in loop bodies.
  bool isAllowedCallInLoopBody(const HLInst *HInst) const {

    Intrinsic::ID Id;
    if (HInst->isIntrinCall(Id) &&
        (Id == Intrinsic::sin || Id == Intrinsic::exp)) {
      assert(!HInst->isUnknownAliasingCallInst());
      return true;
    }

    return false;
  }

  inline bool isIOCall(const HLInst *HInst) const {
    // Sometime CalledFunction is null.
    //    %call.i10.i = tail call signext i8 %13(%"class.std::ctype"* nonnull
    //    %9, i8 signext 10)
    return HInst->isCallInst() && HInst->getCallInst()->getCalledFunction() &&
           HInst->getCallInst()->getCalledFunction()->getName() ==
               "for_write_seq_lis";
  }

  // Check if PrevLCA is the lowest common ancestor of
  // PrevLoop and current Loop
  // Otherwise, loop distribution is needed. We don't do
  // loop distribution.
  // Example:
  // L1
  //   L2 -- FirstSpatialLoop
  //   End Loop L2
  //
  //   L3
  //
  //     L4   -- LastSpatialLoop, LCA so far = LCA of L2 and L3 = L1
  //     End Loop L4
  //
  //     L5    -- Current Loop, LCA of L4 and L5 = L3
  //           -- [FirstSpatialLoop L2, LastSpatialLoop L3] requires
  //           -- a cut between L4 and L5 --> distribution of L3!!
  //     End Loop L5
  //
  //   End Loop L3
  //
  // End L1
  bool isCleanCut(const HLLoop *PrevLoop, const HLLoop *Loop) const {
    if (!Loop || !PrevLoop || !PrevLCA)
      return true;

    const HLLoop *CurLCA =
        HLNodeUtils::getLowestCommonAncestorLoop(PrevLoop, Loop);

    if (CurLCA != PrevLCA &&
        CurLCA->getNestingLevel() >= PrevLCA->getNestingLevel()) {
      return false;
    }

    return true;
  }

protected:
  HIRFramework &HIRF;
  HIRArraySectionAnalysis &HASA;
  HIRDDAnalysis &DDA;

  StringRef Func;

  // Candidates of inter loop blocking
  // [FirstSpatialLoop, LastSpatialLoop]
  HLLoop *FirstSpatialLoop;
  HLLoop *LastSpatialLoop;

  // The lowest common ancestor(LCA) of FirstSpatialLoop and previous
  // LastSpatialLoop. Traversal increases the range of
  // [FirstSpatialLoop, LastSpatialLoop] by updating LastSpatialLoop.
  // Over the course, LCA of First and Last spatial loop is maintained
  // by ensuring LCA == prevLCA.
  HLLoop *PrevLCA;
  bool HasIOCall;

  // Ancestors of DFS when the search gets to a node.
  // Used for skipping unrelated if-stmt.
  SmallVector<HLNode *, 16> TraversalAncestors;
};

// Collect a sequence of valid sibling loopnests.
// See if the internal State changes through INIT, FIRSTHALF, and SECONDHALF.
// Uses logic of CheckerVisitor to do the structural check in extending the
// sequence of [FirstSpatialLoop, LastSpatialLoop]. Additionally, whenever a new
// innermost loop is encountered, it examines if the current profitable status
// is broken. The sequence is extended as long as the profitable status is not
// broken. ProfitablityChecker works in a fashion of a state machine. Three
// states are, INIT, FirstHalf, and SecondHalf. A sequence is profitable if the
// state starts with FirstHalf and ends with SecondHalf.
// In FirstHalf, one set of memrefs is being read and the other set of refs is
// written. In SecondHalf, the reverse is true for the same set of refs.
//
// Example: DO Loop
//   DO Loop 2
//     H[i] = E[i] -- E() is read, H() is written in the FirstHalf
//   End Loop
//
//   DO Loop 3
//     E[i] = H[i] -- H() is read, E() is written in the SecondHalf
//   End Loop
// End Loop
//
// FirstSpatialLoop = Loop 2, LastSpatialLoop = Loop 3
// Notice that the pattern above demonstrates the reuse of "read after write"
// data across Loops 2 and 3.
class ProfitabilityChecker : public CheckerVisitor {

protected:
  enum OuterLoopState { INIT, FIRSTHALF, SECONDHALF };

public:
  ProfitabilityChecker(HIRFramework &HIRF, HIRArraySectionAnalysis &HASA,
                       HIRDDAnalysis &DDA, StringRef FuncName)
      : CheckerVisitor(HIRF, HASA, DDA, FuncName), State(INIT),
        IsProfitable(false) {}

  bool isProfitable() {
    HIRF.getHLNodeUtils().visitAll(*this);

    return IsProfitable;
  }

  HLLoop *getOutermostLoop() const { return PrevLCA; }

  void visit(HLLoop *Loop) {
    markVisited(Loop);

    if (!Loop->isInnermost()) {
      // Simply recurse into.
      return;
    }

    if (Loop->getNestingLevel() == 1) {
      reset();
      return;
    }

    if (!isCleanCut(LastSpatialLoop, Loop)) {
      bailOut();
      return;
    }

    if (!analyzeProfitablity(Loop)) {
      stopAndWork(2, Loop);
      SkipNode = Loop;
      return;
    }

    InnermostLoops.push_back(Loop);
    LastSpatialLoop = Loop;
    if (!FirstSpatialLoop) {
      FirstSpatialLoop = Loop;
    } else {

      HLLoop *LCA =
          HLNodeUtils::getLowestCommonAncestorLoop(FirstSpatialLoop, Loop);
      if (!LCA) {
        SkipNode = Loop;
        reset();
        return;
      }

      if (PrevLCA && PrevLCA != LCA) {
        // Fail and no transformation
        // In theory, [FirstSpatialLoop, previous LastSpatialLoop] could
        // be a valid candidate. But, we don't care about that, but just
        // stop here.
        bailOut();
      } else {
        PrevLCA = LCA;
      }
    }

    SkipNode = Loop;
  }

  void visit(HLIf *HIf) { CheckerVisitor::visit(HIf); }
  void visit(HLInst *HInst) { CheckerVisitor::visit(HInst); }
  void visit(HLNode *Node) { CheckerVisitor::visit(Node); }

private:
  // Stop navigation and check/work on current
  // [FirstSpatialLoop, LastSpatialLoop] so far.
  bool stopAndWork(int CallSiteLoc, const HLLoop *StopLoop = nullptr) override {

    if (InnermostLoops.size() <= 1) {
      reset();
      return false;
    }

    if (!isCleanCut(LastSpatialLoop, StopLoop)) {
      bailOut();
      return false;
    }

    IsProfitable = State == SECONDHALF;

    LLVM_DEBUG(dbgs() << CallSiteLoc << " ";
               dbgs() << "Profitable ?: " << Func << ": ";
               dbgs() << IsProfitable << "\n"; for (auto *Loop
                                                    : InnermostLoops) {
                 dbgs() << Loop->getNumber() << " ";
               } dbgs() << "\n");

    setDone(true);
    return true;
  }

  // See if any element of Cur is found in History.
  bool intersects(const BasePtrIndexSetTy &Cur,
                  const BasePtrIndexSetTy &History) const {

    return any_of(Cur, [History](unsigned BaseIndex) {
      return History.count(BaseIndex);
    });
  }

protected:
  // Examine def/use pattern across innermost loops.
  // We are looking for the following pattern:
  // eg.) For an array, A[], being read(written) for a while,
  // then begin written(read) for a while across a sequence
  // of spatial loops.
  bool analyzeProfitablity(const HLLoop *Loop) {

    BasePtrIndexSetTy CurDefSet;
    BasePtrIndexSetTy CurUseSet;

    const ArraySectionAnalysisResult &Res = HASA.getOrCompute(Loop);

    // Collect information for the current innermost loop
    for (auto BaseIndex : Res.knownBaseIndices()) {
      const ArraySectionInfo *Info = Res.get(BaseIndex);
      if (!Info)
        return false;

      if (Info->isDef())
        CurDefSet.insert(BaseIndex);
      else
        CurUseSet.insert(BaseIndex);
    }

    // Convert: Def --> Use, Use --> Def
    bool ConvertFromUseToDef = intersects(CurDefSet, UseHistory);
    bool ConvertFromDefToUse = intersects(CurUseSet, DefHistory);

    // Maintain: Def --> Def, Use --> Use
    bool MaintainedAsDef = intersects(CurDefSet, DefHistory);
    bool MaintainedAsUse = intersects(CurUseSet, UseHistory);
    (void)MaintainedAsDef;
    (void)MaintainedAsUse;

    if (ConvertFromUseToDef != ConvertFromDefToUse) {
      LLVM_DEBUG(dbgs() << "analyzeProfitablity: " << Loop->getNumber()
                        << "\n");
      LLVM_DEBUG(dbgs() << "ConvertFromUseToDef: " << ConvertFromUseToDef
                        << "\n");
      LLVM_DEBUG(dbgs() << "ConvertFromDefToUse: " << ConvertFromDefToUse
                        << "\n");
      LLVM_DEBUG(dbgs() << "MaintainedAsDef: " << MaintainedAsDef << "\n");
      LLVM_DEBUG(dbgs() << "MaintainedAsUse: " << MaintainedAsUse << "\n");

      return false;
    }

    // UseHistory and DefHistory should have no intersection
    assert(!ConvertFromUseToDef || !MaintainedAsDef);

    if (ConvertFromUseToDef) {
      if (State != FIRSTHALF) {
        LLVM_DEBUG(dbgs() << "State: " << State << "\n");
        return false;
      }

      // swap Use and DefHistory
      UseHistory.clear();
      DefHistory.clear();
      State = SECONDHALF;
    } else if (State == INIT) {
      State = FIRSTHALF;
    }

    // Update history
    std::for_each(CurDefSet.begin(), CurDefSet.end(),
                  [&](unsigned Index) { DefHistory.insert(Index); });
    std::for_each(CurUseSet.begin(), CurUseSet.end(),
                  [&](unsigned Index) { UseHistory.insert(Index); });

    return true;
  }

  void reset() {

    CheckerVisitor::init();

    // Not resetting IsChanged. Once changed for a region, it is.
    UseHistory.clear();
    DefHistory.clear();
    InnermostLoops.clear();
    State = INIT;
    IsProfitable = false;
  }

protected:
  OuterLoopState State;
  bool IsProfitable;

  BasePtrIndexSetTy DefHistory; // RW
  BasePtrIndexSetTy UseHistory; // Read-only

  SmallVector<HLLoop *, 32> InnermostLoops;
};

// Main driver for HIRInterLoopBlocking.
bool driver(HIRFramework &HIRF, HIRArraySectionAnalysis &HASA,
            HIRDDAnalysis &DDA, StringRef FuncName) {
  if (DisablePass) {
    return false;
  }

  if (!FilterFunc.empty() && !FuncName.equals(FilterFunc)) {
    return false;
  }

  ProfitabilityChecker PC(HIRF, HASA, DDA, FuncName);

  bool IsProfitable = PC.isProfitable();

  if (!IsProfitable) {
    LLVM_DEBUG(dbgs() << "NOT Profitable\n");
    return false;
  }

  LLVM_DEBUG_PROFIT_REPORT(dbgs() << "Profitable\n");

  assert(PC.getOutermostLoop());

  LLVM_DEBUG(dbgs() << PC.getOutermostLoop()->getNumber() << "\n";);

  return true;
}
} // namespace

PreservedAnalyses
HIRInterLoopBlockingPass::run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM) {
  driver(AM.getResult<HIRFrameworkAnalysis>(F),
         AM.getResult<HIRArraySectionAnalysisPass>(F),
         AM.getResult<HIRDDAnalysisPass>(F), F.getName());
  return PreservedAnalyses::all();
}

class HIRInterLoopBlockingLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRInterLoopBlockingLegacyPass() : HIRTransformPass(ID) {
    initializeHIRInterLoopBlockingLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRArraySectionAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return driver(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                  getAnalysis<HIRArraySectionAnalysisWrapperPass>().getASA(),
                  getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                  F.getName());
  }
};

char HIRInterLoopBlockingLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRInterLoopBlockingLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRArraySectionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRInterLoopBlockingLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRInterLoopBlockingPass() {
  return new HIRInterLoopBlockingLegacyPass();
}
