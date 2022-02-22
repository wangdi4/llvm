#if INTEL_FEATURE_SW_ADVANCED
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

#include "llvm/ADT/SparseBitVector.h"
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
#include <deque>
#include <unordered_set>

#define OPT_SWITCH "hir-inter-loop-blocking"
#define OPT_DESC "HIR Spatial blocking over multiple loopnests"
#define DEBUG_TYPE OPT_SWITCH
#define LLVM_DEBUG_PROFIT_REPORT(X) DEBUG_WITH_TYPE(OPT_SWITCH "-profit", X)
#define LLVM_DEBUG_DD_EDGES(X) DEBUG_WITH_TYPE(OPT_SWITCH "-dd", X)

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden, cl::desc("Disable " OPT_DESC "."));

static cl::opt<int>
    DefaultStripmineSize(OPT_SWITCH "-stripmine-size", cl::init(2),
                         cl::ReallyHidden,
                         cl::desc("Preset stripmine size for " OPT_DESC));

static cl::opt<std::string>
    FilterFunc(OPT_SWITCH "-filter-func", cl::ReallyHidden,
               cl::desc("Run " OPT_DESC " only on the specified function."));

static cl::opt<std::string> RewriteFilterFunc(
    OPT_SWITCH "-rewrite-filter-func", cl::ReallyHidden,
    cl::desc("If given, only to this function, " OPT_DESC " is applied."));

static cl::opt<bool> DisableTransform("disable-rewrite-" OPT_SWITCH,
                                      cl::init(false), cl::Hidden,
                                      cl::desc("Only check " OPT_DESC "."));

static cl::opt<bool>
    CloneDVLoads(OPT_SWITCH "-clone-loads", cl::init(true), cl::ReallyHidden,
                 cl::desc("Clone loads of DVs at the top as needed"));

static cl::opt<bool> ForceTestDriver(OPT_SWITCH "-force-test", cl::init(false),
                                     cl::ReallyHidden,
                                     cl::desc("Run test driver for lit-tests"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool> PrintInfo(OPT_SWITCH "-print-info", cl::init(false),
                               cl::Hidden,
                               cl::desc("Print info about " OPT_DESC "."));

static cl::opt<unsigned>
    PrintDiagLevel(OPT_SWITCH "-diag-level", cl::init(0), cl::ReallyHidden,
                   cl::desc("Print Diag why " OPT_DESC " did not happen."));

static cl::opt<std::string> PrintDiagFunc(
    OPT_SWITCH "-print-diag-func", cl::ReallyHidden,
    cl::desc("Print Diag why " OPT_DESC " did not happen for the function."));
#endif

namespace {
void printMarker(StringRef Marker, ArrayRef<const HLNode *> Nodes,
                 bool DumpNode = false, bool Detail = false) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  printMarker(PrintInfo, Marker, Nodes, DumpNode, Detail);
#endif
}

void printMarker(StringRef Marker, ArrayRef<const RegDDRef *> Refs,
                 bool Detail = false, bool PrintDim = false) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  printMarker(PrintInfo, Marker, Refs, Detail, PrintDim);
#endif
}

// Works for CanonExpr and BlobDDRef
template <typename T>
void printMarker(StringRef Marker, std::initializer_list<T *> Vs,
                 bool Detail = false) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  printMarker(PrintInfo, Marker, Vs, Detail);
#endif
}

void printDiag(StringRef Msg, StringRef FuncName, const HLLoop *Loop = nullptr,
               StringRef Header = "Fail: ", unsigned DiagLevel = 1) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  printDiag(PrintDiagFunc, PrintDiagLevel, Msg, FuncName, Loop, Header,
            DiagLevel);
#endif
}
} // namespace

namespace {

// SmallSet wanted the size be less than 32 with assertion
// SmallSet<unsigned, 64> will incur an assertion.
typedef DenseSet<unsigned> BasePtrIndexSetTy;

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
        PrevLCA(nullptr), HasIOCall(false) {}

  bool skipRecursion(const HLNode *Node) const { return SkipNode == Node; }
  bool isDone() const { return IsDone; }

  void visit(HLIf *HIf);
  void visit(HLInst *HInst);

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

  virtual ~CheckerVisitor(){};

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
  bool isAllowedCallInLoopBody(const HLInst *HInst) const;

  bool isIOCall(const HLInst *HInst) const;

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
  bool isCleanCut(const HLLoop *PrevLoop, const HLLoop *Loop) const;

  // Returns whether a structural check is passed.
  // Depending on conditions, different subsequent actions
  // can occur. (e.g. reset/bailout/skipnode)
  // TODO: Some of the conditions incurs reset(), while
  // others bailout() or nothing.
  // See if breaking this function further can help.
  bool checkStructure(HLLoop *Loop);

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

void CheckerVisitor::visit(HLIf *HIf) {
  markVisited(HIf);

  bool HasLoopAncestors =
      any_of(std::next(TraversalAncestors.rbegin()), TraversalAncestors.rend(),
             [](const HLNode *Node) { return isa<HLLoop>(Node); });

  if (!HasLoopAncestors) {
    SkipNode = HIf;
  }
}

void CheckerVisitor::visit(HLInst *HInst) {
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

bool CheckerVisitor::isAllowedCallInLoopBody(const HLInst *HInst) const {

  Intrinsic::ID Id;
  if (HInst->isIntrinCall(Id) &&
      (Id == Intrinsic::sin || Id == Intrinsic::exp ||
       Id == Intrinsic::experimental_noalias_scope_decl)) {
    assert(!HInst->isUnknownAliasingCallInst());
    return true;
  }

  return false;
}

bool CheckerVisitor::isIOCall(const HLInst *HInst) const {
  // Sometime CalledFunction is null.
  //    %call.i10.i = tail call signext i8 %13(%"class.std::ctype"* nonnull
  //    %9, i8 signext 10)
  return HInst->isCallInst() && HInst->getCallInst()->getCalledFunction() &&
         HInst->getCallInst()->getCalledFunction()->getName() ==
             "for_write_seq_lis";
}

bool CheckerVisitor::isCleanCut(const HLLoop *PrevLoop,
                                const HLLoop *Loop) const {
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

bool CheckerVisitor::checkStructure(HLLoop *Loop) {

  // If already blocked or
  // has any loop blocking related pragma, skip this loop and its
  // children loops.
  if (Loop->isBlocked() ||
      !(Loop->getBlockingPragmaLevelAndFactors()).empty()) {

    LLVM_DEBUG_PROFIT_REPORT(dbgs() << "Not profitable : loop is already "
                                       "blocked or has blocking pragma\n");

    SkipNode = Loop;
    reset();
    return false;
  }

  if (!Loop->isInnermost()) {
    // Simply recurse into.
    return false;
  }

  if (Loop->getNestingLevel() == 1) {
    reset();
    return false;
  }

  // TODO: do a double check
  if (Loop->isUnknown()) {
    stopAndWork(1, nullptr);
    SkipNode = Loop;
    return false;
  }

  if (!isCleanCut(LastSpatialLoop, Loop)) {
    bailOut();
    return false;
  }

  return true;
}

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

  void visit(HLLoop *Loop);
  void visit(HLIf *HIf) { CheckerVisitor::visit(HIf); }
  void visit(HLInst *HInst) { CheckerVisitor::visit(HInst); }
  void visit(HLNode *Node) { CheckerVisitor::visit(Node); }

private:
  // Stop navigation and check/work on current
  // [FirstSpatialLoop, LastSpatialLoop] so far.
  bool stopAndWork(int CallSiteLoc, const HLLoop *StopLoop = nullptr) override;

  // See if any element of Cur is found in History.
  bool intersects(const BasePtrIndexSetTy &Cur,
                  const BasePtrIndexSetTy &History) const {

    return any_of(Cur, [&History](unsigned BaseIndex) {
      return History.count(BaseIndex);
    });
  }

protected:
  // Examine def/use pattern across innermost loops.
  // We are looking for the following pattern:
  // eg.) For an array, A[], being read(written) for a while,
  // then begin written(read) for a while across a sequence
  // of spatial loops.
  bool analyzeProfitablity(const HLLoop *Loop);

  void reset() override {

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

void ProfitabilityChecker::visit(HLLoop *Loop) {
  markVisited(Loop);

  if (!checkStructure(Loop)) {
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

bool ProfitabilityChecker::stopAndWork(int CallSiteLoc,
                                       const HLLoop *StopLoop) {

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
             dbgs() << IsProfitable << "\n";
             dbgs() << "State: " << State << "\n"; for (auto *Loop
                                                        : InnermostLoops) {
               dbgs() << Loop->getNumber() << " ";
             } dbgs() << "\n");

  setDone(true);
  return true;
}

bool ProfitabilityChecker::analyzeProfitablity(const HLLoop *Loop) {

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
    LLVM_DEBUG(dbgs() << "analyzeProfitablity: " << Loop->getNumber() << "\n");
    LLVM_DEBUG(dbgs() << "ConvertFromUseToDef: " << ConvertFromUseToDef
                      << "\n");
    LLVM_DEBUG(dbgs() << "ConvertFromDefToUse: " << ConvertFromDefToUse
                      << "\n");
    LLVM_DEBUG(dbgs() << "MaintainedAsDef: " << MaintainedAsDef << "\n");
    LLVM_DEBUG(dbgs() << "MaintainedAsUse: " << MaintainedAsUse << "\n");

    return false;
  }

  // Some uses are converted to defs, and some defs
  // continue to be defs (or vice versa).
  // Example:
  // CurDefSet = {13 15 25 26 27}
  // UseHistory = {10 13 14}
  // CurUseSet = {16 17 18 19}
  // DefHistory = {15 16 17}
  // "15" is MaintainedAsDef, while "13" is ConvertFromUseToDef.
  // We just bail out. In the future, more refinements can come.
  // How many are Converted vs. Maintained could be used as a criteria.
  if ((ConvertFromUseToDef && MaintainedAsDef) ||
      (ConvertFromDefToUse && MaintainedAsUse)) {
    LLVM_DEBUG_PROFIT_REPORT(
        dbgs() << "Not profitable due to sustained Defs or Uses.\n");
    return false;
  }

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

// Per-dimension information
// Records the matching loop to a dimension as an offset of levels from
// innermost loop. For example, with a following example
// DO K    // L1
//  DO J   // L2
//   DO I  // L3
//     H[K][J][I]
// A DimInfoTy is described as
// Dim-3 [K] -- LevelOffset is 2
// Dim-2 [J] -- LevelOffset is 1
// Dim-1 [I] -- LevelOffset is 0
// It is assumed that a dimension CE is among the following tree forms.
//  BLOB - blobs + optional constants - LevelOffset is a negative number.
//  KONST - pure constansts - LevelOffset is a negative number.
//  has IV - LevelOffset is valid
// See "Kind" below
class DimInfoTy {
public:
  // BLOB - blobs + optional constants
  // KONST - pure constansts
  // INVALID - dimension form not analyzable.
  enum Kind {
    BLOB = -3,
    KONST = -2,
    INVALID = -1,
  };

  DimInfoTy() : LevelOffset(INVALID) {}

  // Offset from InnermostLevel - (loop level of the IV appearing in this array
  // dimension) Always non-negative because innermost loop has the
  // largest Level. When this dimension does not have IV, but constant and
  // blobs, this field will have negative values.
  int LevelOffset;

  bool hasIV() const { return LevelOffset >= 0; }

  operator int() const { return LevelOffset; }
  void operator=(int Val) { LevelOffset = Val; }

  bool operator==(const DimInfoTy &Other) const {
    return LevelOffset == Other.LevelOffset;
  }

  bool operator<=(const DimInfoTy &Other) const {
    return LevelOffset <= Other.LevelOffset;
  }

  bool operator<(const DimInfoTy &Other) const {
    return LevelOffset < Other.LevelOffset;
  }

  bool operator>(const DimInfoTy &Other) const { return !operator<=(Other); }

  // Used for transformation
  bool isConstant() const { return LevelOffset == KONST; }

  bool isBlob() const { return LevelOffset == BLOB; }
};

typedef SmallVector<DimInfoTy, 4> DimInfoVecTy;
typedef SmallVectorImpl<DimInfoTy> DimInfoVecImplTy;

typedef DenseMap<unsigned, const RegDDRef *> BaseIndexToLowersAndStridesTy;
typedef SmallVector<SmallVector<int64_t, 64>, MaxLoopNestLevel>
    InnermostLoopToShiftTy;
typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

typedef DDRefGrouping::RefGroupVecTy<RegDDRef *> RefGroupVecTy;
typedef DDRefGrouping::RefGroupTy<RegDDRef *> RefGroupTy;

// Legality checker for an innermost loop.
// It examines if the memrefs are spatial accesses. Also it checks
// if an adjustment of dimension indices is possible. Through the
// adjustment, subsequent loops are aligned together to check mutual
// data dependencies.
// For a given innermost loop, its reads dependencies to upward loops
// are verified.
class InnermostLoopAnalyzer {

public:
  InnermostLoopAnalyzer(
      const HLLoop *Loop, unsigned OutermostLoopLevel,
      SmallVectorImpl<DimInfoTy> &DimInfos,
      BaseIndexToLowersAndStridesTy &BaseIndexToLowersAndStrides,
      StringRef FuncName, bool RelaxedMode = false)
      : InnermostLoop(Loop), DimInfos(DimInfos),
        BaseIndexToLowersAndStrides(BaseIndexToLowersAndStrides),
        Func(FuncName), OutermostLoopLevel(OutermostLoopLevel),
        RelaxedMode(RelaxedMode) {

    MemRefGatherer::gatherRange(InnermostLoop->child_begin(),
                                InnermostLoop->child_end(), Refs);
  }

  // The loopnest containing this innermost loop belongs to
  // could be a member of HLNodes to be enclosed by by-strip loops.
  const RegDDRef *couldBeAMember(BasePtrIndexSetTy &DefinedBasePtr,
                                 BasePtrIndexSetTy &ReadOnlyBasePtr,
                                 DDGraph DDG, const HLLoop *LCA = nullptr);

private:
  bool areMostlyStructuallyStencilRefs(RefGroupVecTy &Groups) const {

    // 0.5 is an exprimental choice.
    unsigned StencilGroupCountThreshold =
        static_cast<float>(Groups.size()) * 0.5;

    unsigned StencilGroupCount = 0;
    for (auto &Group : Groups) {
      if (stencilpattern::areStructuallyStencilRefs(Group))
        StencilGroupCount++;
    }

    LLVM_DEBUG(dbgs() << "StencilGroups / Groups : " << StencilGroupCount << "/"
                      << Groups.size() << "\n");

    return StencilGroupCount >= StencilGroupCountThreshold;
  }

  // Check dependencies of this loop against previous loops.
  // It checks if a use of A[i+b] in this loop is dependent to
  // the def to A[i+a] in a lexicographically previous loop.
  // If b <= a, there is no dependency from A[i+a] to A[i+b].
  // Because the loopnests are
  // executed tile by tile, a tile executed eariler than a tile comes later
  // than that.
  // Notice that comparing a rval ddref
  // against RepDepRef of the current loop is sufficient to cover all the def
  // refs in upward loops. This is because, it is verified all def refs in all
  // loops will be aligned in the same fashion as A[i][j][k] regardless of
  // basePtr "A". In other words,
  // All indices with IV + (const) + (blob) will become just IV by substracting
  // (const) + (blob) part. This physical transformation happens later
  // if all checks pass.
  // BasePtrs defined in the previous loops are given as "DefinedBasePtr".
  // After rvals in this loop are checked against "DefinedBasePtr",
  // Notice that RepDefRef's basePtr could be different from that of Rval Refs
  // that are being compared against. This is possible because DimInfoVec are
  // equal over all defined Refs. (Exceptions are const and blob dimInfos)
  bool checkDepToUpwardLoops(BasePtrIndexSetTy &DefinedBasePtr,
                             const RegDDRef *RepDefRef);

  const RegDDRef *getLvalWithMinDims() const {

    const RegDDRef *RepRef = nullptr;
    unsigned MinDimNums = MaxLoopNestLevel;
    for (auto *Ref : Refs) {
      if (Ref->isRval())
        continue;

      if (Ref->getNumDimensions() < MinDimNums) {
        MinDimNums = Ref->getNumDimensions();
        RepRef = Ref;
      }
    }

    return RepRef;
  }

  // Alignment of making every Lval in the form of Array[I_n][I_n+1][I_n+2]
  // will be used for future dep check.
  // Thus, this function makes sure all LvalRefs dimensions CEs are equal.
  // If so, return one of the DefRef as a representative Ref.
  // Otherwise, a nullptr is returned.
  const RegDDRef *checkDefsForAlignment() const;

  bool areEqualLowerBoundsAndStrides(const RegDDRef *FirstRef,
                                     const RefGroupTy &OneGroup);

  // Make sure if lower bounds and strides are the same.
  // For temps, sometimes tracing back towards a load is required.
  // For constants, direct comparison should work.
  //
  // Example:
  // Two memrefs with Baseptr (%5) in the following two loops, have
  // different lowerbounds, %2122 and %4773.
  // However, RHSs of the loads (marked with * and **, respectively) are the
  // same.
  //
  // %2122 = (@A_)[0:0:24([6 x i32]*:0)][0:4:4([6 x
  // i32]:6)]; (*)
  // + DO i2 = 0, sext.i32.i64((1 + %2)) + -2, 1   <DO_LOOP>
  // |   + DO i3 = 0, sext.i32.i64(%1) + -1, 1   <DO_LOOP>
  // |   |   %2230 = (%5)[%2122:i2 + 1:8 * (sext.i32.i64((1 + (-1 * %2120) +
  // %2121)) * sext.i32.i64((1 + (-1 * %2118) + %2119)))(double*:0)][%2120:i3 +
  // 1:8 * sext.i32.i64((1 + (-1 * %2118) +
  // %2119))(double*:0)][%2118:2:8(double*:0)] ...

  // %4773 = (@A_)[0:0:24([6 x i32]*:0)][0:4:4([6 x
  // i32]:6)]; (**)
  // + DO i2 = 0, sext.i32.i64(%1) + -1 * sext.i32.i64(%4925), 1   <DO_LOOP>
  // |   + DO i3 = 0, sext.i32.i64(%0) + -1 * zext.i32.i64(%4837), 1   <DO_LOOP>
  // |   |
  // |   |   %5033 = %5032  +  (%5)[%4773:1:8 * (sext.i32.i64((1 + (-1 * %4771)
  // + %4772)) * sext.i32.i64((1 + (-1 * %4769) + %4770)))(double*:0)][%4771:i2
  // + sext.i32.i64(%492 + (-1 * %4769) + %4770))(double*:0)][%4769:i3 +
  // zext.i32.i64(%4837):8(double*:0)];
  //
  bool tracebackEqualityOfLowersAndStrides(const RegDDRef *Ref1,
                                           const RegDDRef *Ref2, DDGraph DDG,
                                           const HLLoop *LCA = nullptr);

  bool tracebackEqualityOfLowersAndStrides(const RegDDRef *Ref, DDGraph DDG,
                                           const HLLoop *LCA = nullptr);

  // Make sure all dimensions with Blob type, are equal.
  // If not, the memref should be read-only so far.
  // Store that piece of information.
  bool checkEqualityOfBlobDimensions(const RefGroupTy &OneGroup,
                                     const DimInfoVecTy &FirstRefDimInfoVec,
                                     const BasePtrIndexSetTy &DefinedBasePtr,
                                     BasePtrIndexSetTy &ReadOnlyBasePtr,
                                     unsigned CommonDims) const;

  // - DimInfo should be picked from a ref with
  //     1 the largest number of Dimensions
  //     2 also with the largest number of IVs
  //     if the 1, and 2 ties (contradict), we just bail out.
  // - If so, how to take care of dependencies of Refs regarding constant
  //   and blobs.
  // - Actually, co-existence of A[K][J][I] and B[K][1][I] suggests bail-out
  //             we cannot gurantee a safe tiling in that case.
  //             Equality of A and B doesn't matter here.
  bool canCalcDimInfo(const RefGroupVecTy &Groups,
                      BasePtrIndexSetTy &DefinedBasePtr,
                      BasePtrIndexSetTy &ReadOnlyBasePtr, DDGraph DDG,
                      DimInfoVecImplTy &DimInfos, const RegDDRef *RepDef,
                      const HLLoop *LCA = nullptr);

  // Checks each CE has in one of the three forms:
  //  - single IV + (optional constant) + (optional blob)
  //  - only-constant
  //  - only-blob + (optional constant)
  // In the output argument CEKinds, marks CE forms out the the three.
  // In addition, it checks IV's level strictly decreases as dimnum increases.
  //  e.g. A[i1][i2][i3]
  // Return true, if all conditions are met.
  bool analyzeDims(const RegDDRef *Ref, DimInfoVecImplTy &DimInfoVec) const;

  // - Single IV + <optional constant> + <optional blob>
  // - Constant
  // - blob-only + <optional constant>
  bool isValidDim(const CanonExpr *CE, DimInfoTy &DimInfo) const;

  static bool DimInfoCompPred(const DimInfoTy &DI1, const DimInfoTy &DI2);
  static bool DimInfoCompPredRelaxed(const DimInfoTy &DI1,
                                     const DimInfoTy &DI2);
  static bool containsEqualTempBlobs(const CanonExpr *CE1,
                                     const CanonExpr *CE2);

private:
  const HLLoop *InnermostLoop;
  SmallVectorImpl<DimInfoTy> &DimInfos;
  BaseIndexToLowersAndStridesTy &BaseIndexToLowersAndStrides;
  MemRefGatherer::VectorTy Refs;
  StringRef Func;

  // level of the loop enclosing all spatial loops.
  unsigned OutermostLoopLevel;

  bool RelaxedMode;
};

const RegDDRef *
InnermostLoopAnalyzer::couldBeAMember(BasePtrIndexSetTy &DefinedBasePtr,
                                      BasePtrIndexSetTy &ReadOnlyBasePtr,
                                      DDGraph DDG, const HLLoop *LCA) {
  // Check for alignment
  const RegDDRef *RepDefRef = checkDefsForAlignment();

  printMarker("Checking loop: ", {InnermostLoop});

  if (!RepDefRef) {
    printDiag("Aligment ", Func, InnermostLoop);
    return nullptr;
  }

  unsigned DeepestLoopDepth =
      RepDefRef->getNumDimensions() + InnermostLoop->getNestingLevel();
  if (DeepestLoopDepth >= MaxLoopNestLevel) {
    printDiag("Too many loopnests are needed ", Func, InnermostLoop);
    return nullptr;
  }

  printMarker("RepDefRef: ", {RepDefRef});

  RefGroupVecTy Groups;
  DDRefIndexGrouping(Groups, Refs);

  // See if memrefs in the loopbody are in the right form to utilize
  // spatial locality across multiple loopnests.
  if (!canCalcDimInfo(Groups, DefinedBasePtr, ReadOnlyBasePtr, DDG, DimInfos,
                      RepDefRef, LCA)) {
    printDiag("calcDimInfo ", Func, InnermostLoop);
    return nullptr;
  }

  // Interim quick fix
  if (RelaxedMode) {
    return RepDefRef;
  }

  if (!areMostlyStructuallyStencilRefs(Groups)) {
    printDiag("Failed Stencil check", Func, InnermostLoop);
    return nullptr;
  }

  // Check DEP against previous loops
  if (checkDepToUpwardLoops(DefinedBasePtr, RepDefRef)) {
    return RepDefRef;

  } else {
    printDiag("checkDepToUpwardLoops", Func, InnermostLoop);

    return nullptr;
  }
}

bool InnermostLoopAnalyzer::checkDepToUpwardLoops(
    BasePtrIndexSetTy &DefinedBasePtr, const RegDDRef *RepDefRef) {

  printMarker("check dep for innermost loop ", {InnermostLoop});

  // Check Rvals with upward defs
  // Since rvals with defs in upward loopnests(i.e. loopnests lexically before
  // the loopnest where this innermost loop belongs to) are considered,
  // This is how inter-loop dependencies are checked.
  for (auto *Ref : Refs) {
    if (!Ref->isRval() || !DefinedBasePtr.count(Ref->getBasePtrBlobIndex())) {
      continue;
    }

    // Compare use and def, Ref and RepDefRef
    for (auto CEPair :
         zip(make_range(Ref->canon_begin(), Ref->canon_end()),
             make_range(RepDefRef->canon_begin(), RepDefRef->canon_end()))) {

      CanonExpr *CE = std::get<0>(CEPair);
      const CanonExpr *RepDefCE = std::get<1>(CEPair);

      // Means use should NOT be in the form of
      // - A[i + (blob) + konst] or A[i + (blob) + blob2 + konst],
      //   where konst > 0 and RepRefs are in the form
      int64_t Dist = 0;
      if (!CanonExprUtils::getConstDistance(CE, RepDefCE, &Dist) || Dist > 0) {

        printMarker(
            "Dependency violation!! in Region: ",
            {Ref->getHLDDNode()->getParentRegion(), Ref->getHLDDNode()});
        printMarker("Troublesome CE: ", {Ref});

        return false;
      }
    }
  }

  return true;
}

const RegDDRef *InnermostLoopAnalyzer::checkDefsForAlignment() const {

  // Get a Lval ref with maximum numDims.
  // We assume refs with a maximum number of dimensions
  // are spatial references.
  // For example, if a loop body contains
  // A[i][j][k] and B[k], we pick A[i][j][k] because, that reference
  // will have more pieces of information.
  const RegDDRef *Representative = getLvalWithMinDims();

  // No Def
  if (!Representative)
    return nullptr;

  for (auto *Ref : Refs) {

    // Check only Lvals, because Lvals are going to be used as pivoting
    // refs for alignment of CEs.
    if (!Ref->isLval())
      continue;

    // All Lval dimensions are the same.
    for (auto CEPair : zip(make_range(Ref->canon_begin(), Ref->canon_end()),
                           make_range(Representative->canon_begin(),
                                      Representative->canon_end()))) {

      if (!CanonExprUtils::areEqual(std::get<0>(CEPair), std::get<1>(CEPair))) {
        printMarker("checkDefsForAlignemnts() fails: ", {Representative, Ref});
        return nullptr;
      }
    }
  }

  return Representative;
}

bool InnermostLoopAnalyzer::areEqualLowerBoundsAndStrides(
    const RegDDRef *FirstRef, const RefGroupTy &OneGroup) {

  unsigned NumDims = FirstRef->getNumDimensions();

  for (auto *Ref : OneGroup) {

    if (NumDims != Ref->getNumDimensions()) {
      // This function does not check anything for following cases.
      // Fortran dope-vectors.
      // (@b_)[0].6[0].2;
      // (@b_)[0].0;
      // (@a_)[0].6[0].2;
      // (@a_)[0].0;

      // TODO: work around - not correct
      if (FirstRef->hasTrailingStructOffsets() &&
          Ref->hasTrailingStructOffsets()) {
        return true;
      } else {
        return false;
      }
    }

    for (unsigned DimNum :
         make_range(Ref->dim_num_begin(), Ref->dim_num_end())) {
      if (!CanonExprUtils::areEqual(FirstRef->getDimensionLower(DimNum),
                                    Ref->getDimensionLower(DimNum))) {
        printMarker("&& LowerBounds:", {FirstRef, Ref}, false, true);

        return false;
      }

      if (!CanonExprUtils::areEqual(FirstRef->getDimensionStride(DimNum),
                                    Ref->getDimensionStride(DimNum))) {

        printMarker("&& Strides:", {FirstRef, Ref}, false, true);

        return false;
      }
    } // All CEs.
  }   // All Refs.

  return true;
}

bool InnermostLoopAnalyzer::tracebackEqualityOfLowersAndStrides(
    const RegDDRef *Ref1, const RegDDRef *Ref2, DDGraph DDG,
    const HLLoop *LCA) {

  auto GetLoadRval = [DDG](const BlobDDRef *BRef) -> const RegDDRef * {
    for (auto *Edge : DDG.incoming(BRef)) {
      // Only flow edge should exist.
      if (!Edge->isFlow())
        return nullptr;

      if (const HLInst *Inst =
              dyn_cast<HLInst>(Edge->getSrc()->getHLDDNode())) {

        return isa<LoadInst>(Inst->getLLVMInstruction()) ? Inst->getRvalDDRef()
                                                         : nullptr;

      } else {
        return nullptr;
      }
    }

    return nullptr;
  };

  auto compareRefs = [GetLoadRval,
                      LCA](const CanonExpr *CE1, const CanonExpr *CE2,
                           const RegDDRef *Ref1, const RegDDRef *Ref2) -> bool {
    // If one is a constant, both should be the same value.
    int64_t Konstant1 = 0;
    if (CE1->isIntConstant(&Konstant1)) {
      int64_t Konstant2 = 0;
      if (!CE2->isIntConstant(&Konstant2) || Konstant1 != Konstant2) {
        printMarker("Konstants are diff .\n Ref1, Ref2: ", {Ref1, Ref2}, true,
                    true);
        printMarker("CE1, CE2: ", {CE1, CE2}, true);
        printMarker("Ref1 Loop, Ref2 Loop, LCA: ",
                    {Ref1->getParentLoop(), Ref2->getParentLoop(), LCA}, true);

        return false;
      }

      return true;
    }

    // Temp -- Then there should be a blob ddref
    if (CE1->numBlobs() != CE2->numBlobs()) {
      printMarker("NumBlobs are diff.\n Ref1, Ref2: ", {Ref1, Ref2}, true);
      printMarker("CE1, CE2: ", {CE1, CE2}, true);
      printMarker("Ref1 Loop, Ref2 Loop, LCA: ",
                  {Ref1->getParentLoop(), Ref2->getParentLoop(), LCA}, true);

      return false;

    } else if (CE1->numBlobs() != 1) {
      printMarker("NumBlobs is NOT 1.\n Ref1, Ref2: ", {Ref1, Ref2}, true);
      printMarker("CE1, CE2: ", {CE1, CE2}, true);
      printMarker("Ref1 Loop, Ref2 Loop, LCA: ",
                  {Ref1->getParentLoop(), Ref2->getParentLoop(), LCA}, true);

      return false;
    }

    if (CE1->getSingleBlobCoeff() != CE2->getSingleBlobCoeff()) {
      printMarker("SingleBlobCoeffs are diff.\n Ref1, Ref2: ", {Ref1, Ref2},
                  true);
      printMarker("CE1, CE2: ", {CE1, CE2}, true);
      printMarker("Ref1 Loop, Ref2 Loop, LCA: ",
                  {Ref1->getParentLoop(), Ref2->getParentLoop(), LCA}, true);

      return false;
    }

    unsigned Index = CE1->getSingleBlobIndex();
    const BlobDDRef *BRef1 = Ref1->getBlobDDRef(Index);
    const RegDDRef *LoadRval1 = GetLoadRval(BRef1);

    unsigned Index2 = CE2->getSingleBlobIndex();
    const BlobDDRef *BRef2 = Ref2->getBlobDDRef(Index2);
    const RegDDRef *LoadRval2 = GetLoadRval(BRef2);
    if (!LoadRval1 && !LoadRval2) {
      return CanonExprUtils::areEqual(CE1, CE2);
    }

    if (LoadRval1 && LoadRval2 && !DDRefUtils::areEqual(LoadRval1, LoadRval2)) {
      printMarker("LoadRval1 != LoadRval2\n Ref1, Ref2: ", {Ref1, Ref2}, true,
                  true);
      printMarker("LoadRval1, LoadRval2: ", {LoadRval1, LoadRval2}, true, true);
      printMarker("BRef1, BRef2: ", {BRef1, BRef2}, true);
      printMarker("Ref1 Loop, Ref2 Loop, LCA: ",
                  {Ref1->getParentLoop(), Ref2->getParentLoop(), LCA}, true);

      return false;
    }

    // TODO:
    // How do I know that LoadRval1 and 2 are read-only?

    return true;
  };

  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    // TODO: This is a compromise.
    return true;
  }

  for (unsigned DimNum :
       make_range(Ref1->dim_num_begin(), Ref1->dim_num_end())) {

    const CanonExpr *Lower1 = Ref1->getDimensionLower(DimNum);
    const CanonExpr *Lower2 = Ref2->getDimensionLower(DimNum);

    if (!compareRefs(Lower1, Lower2, Ref1, Ref2))
      return false;

#if 0
      // Intentionally kept as a commented-out portion for documenting.
      // Stride CE is Not just a selfblob
      //
      // Ref1 <11652>      %3958 = (%3)[i2 + 1][i3 + 1][1];
      //
      // (LINEAR double* %3)[LINEAR sext.i32.i64(%3828){def@1}:LINEAR i64 i2 + 1:LINEAR i64 8 *
      // (sext.i32.i64((1 + (-1 * %3826) + %3827)) * sext.i32.i64((1 + (-1 * %3824) + %3825))){def@1}
      // double*:0)][LINEAR sext.i32.i64(%3826){def@1}:LINEAR i64 i3 + 1:LINEAR i64 8 * sext.i32.i64(
      // 1 + (-1 * %3824) + %3825)){def@1}(double*:0)][..]
      //
      // StrideCE: LINEAR i64 8 * sext.i32.i64((1 + (-1 * %3824) + %3825)){def@1}
      //
      // Need to trace all blobDDRefs found in StrideCE.
      // For the example above, %3824 and %3825
      // Also, structural equivalence should be checked.
      // e.g.) counterpart StrideCE: LINEAR i64 8 * sext.i32.i64((1 + (-1 * %x) + %y)){def@1}
      //      %3824 <--> %x, %3825 <--> %y
      const CanonExpr* Stride1 = Ref1->getDimensionStride(DimNum);
      const CanonExpr* Stride2 = Ref2->getDimensionStride(DimNum);
      if (!compareRefs(Stride1, Stride2, Ref1, Ref2)) return false;
#endif
  }

  return true;
}

bool InnermostLoopAnalyzer::tracebackEqualityOfLowersAndStrides(
    const RegDDRef *Ref, DDGraph DDG, const HLLoop *LCA) {

  unsigned BasePtrIndex = Ref->getBasePtrBlobIndex();
  if (BaseIndexToLowersAndStrides.find(BasePtrIndex) ==
      BaseIndexToLowersAndStrides.end()) {
    BaseIndexToLowersAndStrides.insert({BasePtrIndex, Ref});
  } else {
    if (!tracebackEqualityOfLowersAndStrides(
            BaseIndexToLowersAndStrides[BasePtrIndex], Ref, DDG, LCA))
      return false;
  }
  return true;
}

bool InnermostLoopAnalyzer::containsEqualTempBlobs(const CanonExpr *CE1,
                                                   const CanonExpr *CE2) {
  if (CE1->numBlobs() != 1 || CE1->numBlobs() != CE2->numBlobs()) {
    return false;
  }

  SmallVector<unsigned> Ind1;
  SmallVector<unsigned> Ind2;
  BlobUtils &BU = CE1->getBlobUtils();

  for (auto I : make_range(CE1->blob_begin(), CE1->blob_end()))
    BU.collectTempBlobs((I).Index, Ind1);

  for (auto I : make_range(CE2->blob_begin(), CE2->blob_end()))
    BU.collectTempBlobs((I).Index, Ind2);

  std::sort(Ind1.begin(), Ind1.end());
  std::sort(Ind2.begin(), Ind2.end());

  return std::equal(Ind1.begin(), Ind1.end(), Ind2.begin());
}

bool InnermostLoopAnalyzer::checkEqualityOfBlobDimensions(
    const RefGroupTy &OneGroup, const DimInfoVecTy &FirstRefDimInfoVec,
    const BasePtrIndexSetTy &DefinedBasePtr, BasePtrIndexSetTy &ReadOnlyBasePtr,
    unsigned CommonDims) const {
  auto *FirstRef = OneGroup.front();

  for (auto *Ref : make_range(std::next(OneGroup.begin()), OneGroup.end())) {

    unsigned MinIVLevel = MaxLoopNestLevel;
    for (unsigned
             DimNum = 1,
             Size = std::min(CommonDims, (unsigned)(FirstRefDimInfoVec.size()));
         DimNum <= Size; DimNum++) {

      if (FirstRefDimInfoVec[DimNum - 1].hasIV())
        MinIVLevel = FirstRefDimInfoVec[DimNum - 1].LevelOffset;

      if (!FirstRefDimInfoVec[DimNum - 1].isBlob())
        continue;

      if (MinIVLevel == 1)
        return true;

      // Blobs should be the same throughout refs.
      if (!CanonExprUtils::areEqual(FirstRef->getDimensionIndex(DimNum),
                                    Ref->getDimensionIndex(DimNum))) {

        if (!DefinedBasePtr.count(FirstRef->getBasePtrBlobIndex())) {

          // Defer the check by storing this piece of information.
          ReadOnlyBasePtr.insert(FirstRef->getBasePtrBlobIndex());

        } else if (RelaxedMode &&
                   containsEqualTempBlobs(FirstRef->getDimensionIndex(DimNum),
                                          Ref->getDimensionIndex(DimNum))) {
          printMarker("blobs are similar enough: ", {FirstRef, Ref});
        } else {
          // Some examples:
          // RepDefRef:  (%50)[%10][i1 + sext.i32.i64(%160)][i2 +
          // sext.i32.i64(%130)] FirstRef: (%1082)[i1 + sext.i32.i64(%160)][i2 +
          // sext.i32.i64(%130) + -1] FirstRef: (%1080)[i1 +
          // sext.i32.i64(%160)][i2 + sext.i32.i64(%130) + -1] FirstRef:
          // (%92)[i1 + sext.i32.i64(%160)][i2 + sext.i32.i64(%130) + -1]
          // FirstRef: (%89)[i1 + sext.i32.i64(%160)][i2 + sext.i32.i64(%130) +
          // -1] FirstRef: (%50)[%9][i1 + sext.i32.i64(%160)][i2 +
          // sext.i32.i64(%130)]
          printMarker("Def but different blobIndex: ", {FirstRef, Ref});

          return false;
        }
      }

    } // end of dimension
  }   // end of one Ref group

  return true;
}

bool InnermostLoopAnalyzer::DimInfoCompPredRelaxed(const DimInfoTy &DI1,
                                                   const DimInfoTy &DI2) {
  return (DI1 == DI2) ||
         (DI1.LevelOffset == DimInfoTy::KONST &&
          DI2.LevelOffset == DimInfoTy::BLOB) ||
         (DI1.LevelOffset == DimInfoTy::BLOB &&
          DI2.LevelOffset == DimInfoTy::KONST);
}

bool InnermostLoopAnalyzer::DimInfoCompPred(const DimInfoTy &DI1,
                                            const DimInfoTy &DI2) {
  return (DI1 == DI2);
}

// - DimInfo should be picked from a ref with
//     1 the largest number of Dimensions
//     2 also with the largest number of IVs
//     if the 1, and 2 ties (contradict), we just bail out.
// - If so, how to take care of dependencies of Refs regarding constant
//   and blobs.
// - Actually, co-existence of A[K][J][I] and B[K][1][I] suggests bail-out
//             we cannot gurantee a safe tiling in that case.
//             Equality of A and B doesn't matter here.
bool InnermostLoopAnalyzer::canCalcDimInfo(
    const RefGroupVecTy &Groups, BasePtrIndexSetTy &DefinedBasePtr,
    BasePtrIndexSetTy &ReadOnlyBasePtr, DDGraph DDG, DimInfoVecImplTy &DimInfos,
    const RegDDRef *RepDefRef, const HLLoop *LCA) {

  // A map from the number of dimensions (of a ref) to DimInfoTy
  // Different refs with the same number of dimensions are
  // expected to have the same DimInfo
  // RegDDRef* is not being actively used. Mostly for debugging.

  auto DimInfoPred = RelaxedMode ? DimInfoCompPredRelaxed : DimInfoCompPred;

  for (auto OneGroup : Groups) {

    auto *FirstRef = OneGroup.front();

    // Collect all lowerbounds of Refs sharing this Ref.
    // And they are all the same.
    // Ideally, the lowerbounds are all the same as a constant,
    // or as a temp.
    // However, in reality, for many fortran programs, they could be
    // different temps but load from the same read-only memory location.
    // Notice that the same lowerbounds for a baseptr should hold across
    // all loops.
    // A lowerbound is one of the three forms.
    // Konst / Temp / MemRef
    // If it is a konst, no other checks are needed.
    // If it is a temp, it should be checked not re-defined.
    // It it is a MemRef, the memory location is read-only
    // throughout the outermost loop.
    // For "read-only" information, consult DDG.
    // In theory, we do not make input edge, thus,
    // No source or sink of a dd-edge.

    // TODO: has some compromises.
    if (!areEqualLowerBoundsAndStrides(FirstRef, OneGroup))
      return false;

    // TODO: has some compromises
    // Eventually if all loads related to lowerbounds and
    // strides are hoisted up to the beginning of the outermost loop
    // or the outside of the outermost loop, chances are
    // the traceback of loads are not needed.
    if (!tracebackEqualityOfLowersAndStrides(FirstRef, DDG, LCA))
      return false;

    DimInfoVecTy FirstRefDimInfoVec;
    if (!analyzeDims(FirstRef, FirstRefDimInfoVec))
      return false;

    // DimInfo congruence check:
    // For the same group, dimInfo should be equal across all refs.
    for (auto *Ref : make_range(std::next(OneGroup.begin()), OneGroup.end())) {
      DimInfoVecTy DimInfoVec;
      if (!analyzeDims(Ref, DimInfoVec))
        return false;

      // Compare against Front
      if (!std::equal(FirstRefDimInfoVec.begin(), FirstRefDimInfoVec.end(),
                      DimInfoVec.begin(), DimInfoPred))
        return false;
    }

    // Among refs in the same group, if one ref has a blob in a dimension,
    // all other refs should have the same blob in the same dimension.
    // Only exception is when the refs are read-only.
    // caveat - at this point only lexically upward defs are available.
    //        - Defs lexically past this point needs to be checked afterwards.

    // Mark Lvals
    llvm::for_each(OneGroup, [&DefinedBasePtr](const RegDDRef *Ref) {
      if (Ref->isLval())
        DefinedBasePtr.insert(Ref->getBasePtrBlobIndex());
    });

    // Check the equality of indices, whose type is blob. Consult
    // upward defined baseptrs and update read-only baseptr as needed.
    if (!checkEqualityOfBlobDimensions(OneGroup, FirstRefDimInfoVec,
                                       DefinedBasePtr, ReadOnlyBasePtr,
                                       RepDefRef->getNumDimensions())) {
      return false;
    }

  } // end Groups

  if (!analyzeDims(RepDefRef, DimInfos)) {
    LLVM_DEBUG(dbgs() << "CalcDimInfo failed 2\n");
    return false;
  }

  return true;
}

bool InnermostLoopAnalyzer::analyzeDims(const RegDDRef *Ref,
                                        DimInfoVecImplTy &DimInfoVec) const {

  for (auto DimNum : make_range(Ref->dim_num_begin(), Ref->dim_num_end())) {
    const CanonExpr *CE = Ref->getDimensionIndex(DimNum);
    DimInfoTy DimInfo;
    // Per-dimension check
    if (!isValidDim(CE, DimInfo))
      return false;

    DimInfoVec.push_back(DimInfo);
  }

  // Inter-dimensional check:
  // Now check if all IV levels strictly increases dimnum decreases.
  // It trivially gurantees that all IVs are different.
  int PrevLevelOffset = -1;
  for (auto DimInfo : make_range(DimInfoVec.begin(), DimInfoVec.end())) {
    if (!DimInfo.hasIV())
      continue;

    if (DimInfo.LevelOffset <= PrevLevelOffset)
      return false;

    PrevLevelOffset = DimInfo.LevelOffset;
  }

  return true;
}

bool InnermostLoopAnalyzer::isValidDim(const CanonExpr *CE,
                                       DimInfoTy &DimInfo) const {
  int64_t Val;
  if (CE->isIntConstant(&Val)) {
    DimInfo = DimInfoTy::KONST;
    return true;
  }

  if (CE->numIVs() == 0 && CE->numBlobs() > 0) {
    DimInfo = DimInfoTy::BLOB;
    return true;
  }

  DimInfo = DimInfoTy::INVALID;
  if (CE->numIVs() != 1)
    return false;

  // Exactly 1 IV
  unsigned IVFoundLevel = 0;
  for (unsigned Level :
       make_range(AllLoopLevelRange::begin(), AllLoopLevelRange::end())) {
    unsigned Index;
    int64_t Coeff;
    CE->getIVCoeff(Level, &Index, &Coeff);

    if (Coeff == 0)
      continue;

    if (Index != InvalidBlobIndex)
      return false;

    if (IVFoundLevel)
      return false;

    IVFoundLevel = Level;
  }

  if (!IVFoundLevel || IVFoundLevel == OutermostLoopLevel)
    return false;

  DimInfo = ((InnermostLoop->getNestingLevel()) - IVFoundLevel);

  return true;
}

typedef std::pair<HLLoop *, SmallVector<DimInfoTy, 4>> LoopAndDimInfoTy;
typedef std::vector<LoopAndDimInfoTy> LoopToDimInfoTy;
typedef std::map<const HLLoop *, RegDDRef *> LoopToRefTy;
typedef std::map<const HLLoop *, const RegDDRef *> LoopToConstRefTy;

// Find the lowest ancestor of InnermostLoop, which is deeper than Limit.
// It is not necessarily a HLLoop.
HLNode *findTheLowestAncestor(HLLoop *InnermostLoop, const HLNode *Limit) {

  assert(isa<HLRegion>(Limit) ||
         Limit->getNodeLevel() < InnermostLoop->getNestingLevel());

  HLNode *Node = InnermostLoop;
  HLNode *Prev = InnermostLoop;
  while (Node != Limit) {
    Prev = Node;
    Node = Node->getParent();
  }

  return Prev;
}

class Transformer {
  struct TopSortCompare {
    bool operator()(const HLInst *Inst1, const HLInst *Inst2) const {
      return Inst1->getTopSortNum() < Inst2->getTopSortNum();
    }
  };

  typedef std::set<const HLInst *, TopSortCompare> InstsToCloneSetTy;

  // Subtract AdjustingRef from a ref in loop's body.
  // by the same def location.
  // Example:
  // Input loop before alignment
  // for i = 0, N
  //  for j = 0, M
  //   a[i][j+1] = b[i][j] + 3;
  //
  // After alignment
  // for i = 0, N
  //  for j = 1, M + 1
  //   a[i][j] = b[i][j - 1] + 3;
  //
  // This function only takes care of loop's body. Loop bounds are taken care
  // of another function.
  // The alignment is achieved by subtracting AdjustingRef, base[0][1] from
  // a memref.
  class LoopBodyAligner final : public HLNodeVisitorBase {
  private:
    HLNode *SkipNode;

    // Loop to update
    HLLoop *Loop;
    const RegDDRef *AdjustingRef;
    const DenseMap<unsigned, unsigned> &MapFromLevelToDim;

  public:
    LoopBodyAligner(HLLoop *Loop, const RegDDRef *AdjustingRef,
                    const DenseMap<unsigned, unsigned> &MapFromLevelToDim)
        : SkipNode(nullptr), Loop(Loop), AdjustingRef(AdjustingRef),
          MapFromLevelToDim(MapFromLevelToDim) {}

    void update() {
      HLNodeUtils::visitRange(*this, Loop->child_begin(), Loop->child_end());
    }

    // Skip any inner level loops. This visitor is supposed to take care of
    // only the refs in its self body, not in bodies of its children.
    void visit(HLLoop *Lp) { SkipNode = Lp; };
    void visit(HLNode *Node) {}
    void postVisit(HLNode *Node) {}

    bool skipRecursion(HLNode *Node) { return SkipNode == Node; }

    // Main logic: update all ddrefs of a HLDDNode
    void visit(HLDDNode *Node) {
      for (auto *Ref : make_range(Node->ddref_begin(), Node->ddref_end())) {

        if (!Ref->hasGEPInfo())
          continue;

        std::unique_ptr<RegDDRef> OrigRef(Ref->clone());

        // For a Ref, go through dimensions, and get IV Level
        // Get the dim and get the CE from RepRef
        for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {

          assert(CE->numIVs() <= 1);

          // get IV Level from CE
          unsigned FoundIVLevel = 0;
          for (auto Level : make_range(AllLoopLevelRange::begin(),
                                       AllLoopLevelRange::end())) {
            int64_t Coeff = 0;
            unsigned Index = 0;
            CE->getIVCoeff(Level, &Index, &Coeff);
            if (Coeff != 0) {
              FoundIVLevel = Level;
              break;
            }
          }

          if (!FoundIVLevel)
            continue;

          auto It = MapFromLevelToDim.find(FoundIVLevel);
          assert(It != MapFromLevelToDim.end());
          unsigned Dim = It->second;
          const CanonExpr *AdjustCE = AdjustingRef->getDimensionIndex(Dim);

          CanonExprUtils::subtract(CE, AdjustCE);
        }

        // Initial makeConsistent using original refs.
        // After this point, nonlinear is truly nonlinear.
        // Also, IV + const should have Def@Level zero.
        printMarker("Orig Ref: ", {OrigRef.get()}, true);
        printMarker("AdjustingRef: ", {AdjustingRef}, true);
        printMarker("Ref: ", {Ref}, true);

        // Cannot use RepRef here as an auxiliary ref
        // because RepRef itself is aligned by this function
        // and get changed. Instead, use AdjustingRef,
        // since it is a RepRef minus IVs.
        // It contains all the blobs.
        Ref->makeConsistent({OrigRef.get(), AdjustingRef});
      }
    }

  private:
    static void alignSpatialLoopBounds(RegDDRef *Ref,
                                       const CanonExpr *AdjustingCE,
                                       const RegDDRef *AdjustingRef);
  };

public:
  HIRDDAnalysis &DDA;

  // Entry value 0 denotes no-blocking.
  // Size of StripmineSizes should be the same as global NumDims
  Transformer(ArrayRef<unsigned> StripmineSizes,
              const LoopToDimInfoTy &InnermostLoopToDimInfos,
              const LoopToConstRefTy &InnermostLoopToRepRef,
              const InnermostLoopToShiftTy &InnermostLoopToShift,
              HLLoop *OutermostLoop, HLIf *OuterIf, HIRDDAnalysis &DDA,
              StringRef Func)
      : DDA(DDA), StripmineSizes(StripmineSizes),
        InnermostLoopToDimInfos(InnermostLoopToDimInfos),
        InnermostLoopToRepRef(InnermostLoopToRepRef),
        InnermostLoopToShift(InnermostLoopToShift),
        OutermostLoop(OutermostLoop), OuterIf(OuterIf), NumByStripLoops(0),
        Func(Func) {
    unsigned NumDims = StripmineSizes.size();
    ByStripLoopLowerBlobs.resize(NumDims);
    ByStripLoopUpperBlobs.resize(NumDims);
    ByStripLoops.resize(NumDims, 0x0);

    // Initialize the number of ByStripLoops.
    NumByStripLoops = getNumByStripLoops(StripmineSizes);

    calcLoopMatchingDimNum();

    assert((!OutermostLoop && OuterIf) || (OutermostLoop && !OuterIf));
  }

  static unsigned getNumByStripLoops(ArrayRef<unsigned> StripmineSizes) {
    return count_if(StripmineSizes, [](unsigned Size) { return Size; });
  }

  // Make sure every dimension has a target loop.
  bool checkDimsToLoops(ArrayRef<unsigned> StripmineSizes,
                        const LoopToDimInfoTy &InnermostLoopToDimInfos) {

    unsigned GlobalNumDims = StripmineSizes.size();

    for (unsigned DimNum = 1; DimNum <= GlobalNumDims; DimNum++) {
      if (isNoBlockDim(DimNum, StripmineSizes))
        continue;

      bool FoundTargetLoop = false;
      // Collect all Lower/AlignedUpperBounds from InnermostLoop
      for (auto &E : InnermostLoopToDimInfos) {
        const HLLoop *InnermostLoop = E.first;

        const HLLoop *TargetLoop =
            getLoopMatchingDimNum(DimNum, E.second.size(), InnermostLoop);
        if (TargetLoop) {
          FoundTargetLoop = true;
          break;
        }
      }

      if (!FoundTargetLoop) {
        // This dimension had no target loop.
        // I.e. all refs have either constants or blobs in this dimension,
        // and no IV.
        return false;
      }
    }

    return true;
  }

  bool rewrite() {

    // This check is done after StripmineSizes are determined.
    // If the check fails, transformation does not happen.
    if (getNumByStripLoops(StripmineSizes) == 0 ||
        !checkDimsToLoops(StripmineSizes, InnermostLoopToDimInfos)) {
      LLVM_DEBUG_PROFIT_REPORT(dbgs() << "No transformation: Some dimensions "
                                         "have no matching loop level.\n");
      return false;
    }

    HLNode *OutermostNode = OutermostLoop ? static_cast<HLNode *>(OutermostLoop)
                                          : static_cast<HLNode *>(OuterIf);
    HLRegion *Region = OutermostNode->getParentRegion();

    printMarker("Initial: ", {OutermostNode}, true, false);
    LLVM_DEBUG(dbgs() << "Region to update: " << Region->getNumber() << "\n");
    LLVM_DEBUG(Region->dump(1));
    LLVM_DEBUG(dbgs() << "== * == AAAA \n"; for (auto &LoopAndDimInfo
                                                 : InnermostLoopToDimInfos) {
      dbgs() << LoopAndDimInfo.first->getNumber() << "\n";
    });

    LoopToRefTy InnermostLoopToAdjustingRef;
    prepareAdjustingRefs(InnermostLoopToAdjustingRef);

    HLNode *AnchorNode = findTheLowestAncestor(
        (*InnermostLoopToDimInfos.begin()).first, OutermostNode);

    printMarker("AnchorNode: ", {AnchorNode});

    // Collect load instructions for loop bounds and RepRefs.
    // NOTE: This collection should be done before alignment.
    //       Alignment of loop bounds potentially modifies
    //       blobDDRefs of loop bounds. Thus, input DDG does
    //       not hold the information to trace back to loads
    //       of temps.
    //       For example, a loop before alignment
    //  DO i = 0, M, 1
    //    A[i + t] = B[i] + 1
    //       will become
    //  DO i = t, M + t, 1
    //    A[i] = B[i - t] + 1
    //  Notice M has chaned to M + t.
    //  For the alignment itself, see comments on alignSpatialLoopBody.
    //  Notice that not all insts cloned are load insts, but the name
    //  is used as the majorities are loads.
    InstsToCloneSetTy LoadInstsToClone;
    SmallVector<std::pair<unsigned, unsigned>, 16> CopyToLoadIndexMap;

    if (CloneDVLoads) {
      if (!collectLoadsToClone(AnchorNode, LoadInstsToClone,
                               CopyToLoadIndexMap)) {
        LLVM_DEBUG_PROFIT_REPORT(dbgs() << "No transformation: Cannot collect"
                                           "insts to clone\n");
        return false;
      }
    }

    // Align original spatial loops
    alignSpatialLoops(InnermostLoopToAdjustingRef);

    assert(isa<HLLoop>(AnchorNode) && AnchorNode->getNodeLevel() ==
                                          (OutermostNode->getNodeLevel() + 1) ||
           !isa<HLLoop>(AnchorNode) &&
               AnchorNode->getNodeLevel() == OutermostNode->getNodeLevel());

    // Step 1. Create a map from NodeToMove to InnermostLoop
    // The vector of OutermostNode should be enough, innermost loop is for
    // debugging.
    SmallVector<std::pair<HLNode *, HLNode *>, 16> OuterNodeToInnermostLoop;

    for (auto &LoopAndDimInfo : InnermostLoopToDimInfos) {
      HLNode *NodeToMove =
          findTheLowestAncestor(LoopAndDimInfo.first, OutermostNode);

      OuterNodeToInnermostLoop.emplace_back(NodeToMove, LoopAndDimInfo.first);
    }

    HLNode *LastByStripNode = findTheLowestAncestor(
        (InnermostLoopToDimInfos.back()).first, OutermostNode);

    // TODO: Verify skipping this phase when OutermostLoop is nullptr (i.e.
    // OuterIf)
    //       Using region's live-in/out in addByStripLoops() is conservative but
    //       should be enough.
    collectLiveInsToByStripLoops(AnchorNode, LastByStripNode);

    // LiveIns defined after ByStripLoops -- omitted for now

    // TODO: consider avoiding this phase for advanced options
    //       for compile time.
    SmallVector<unsigned, 16> LiveOutsOfByStrip =
        collectLiveOutsOfByStripLoops(AnchorNode, LastByStripNode);

    // Note that invalidation only happens here because DDG of original
    // HLNodes are needed before this point. For example,
    // collectLoadsToClone() uses DDG to traceback load instructions.
    if (OutermostLoop) {
      HIRInvalidationUtils::invalidateBody(OutermostLoop);
    } else {
      HIRInvalidationUtils::invalidateNonLoopRegion(Region);
    }

    DenseMap<unsigned, unsigned> OrigToCloneIndexMap;
    SmallVector<const RegDDRef *, 32> AuxRefsForByStripBounds;

    if (CloneDVLoads) {
      cloneAndAddLoadInsts(LoadInstsToClone, AnchorNode, OrigToCloneIndexMap,
                           AuxRefsForByStripBounds);
    }

    // Merge CopyToLoadIndexMap into OrigToCloneIndexMap
    for (auto KV : CopyToLoadIndexMap) {
      unsigned NewKey = KV.first;
      unsigned NewVal = OrigToCloneIndexMap[KV.second];
      assert(NewVal);
      OrigToCloneIndexMap.insert({NewKey, NewVal});
    }

    if (!computeByStripLoopBounds(InnermostLoopToAdjustingRef,
                                  OrigToCloneIndexMap,
                                  AuxRefsForByStripBounds)) {

      LLVM_DEBUG(dbgs() << "== * == Failed computeByStripLoopBounds \n");

      return false;
    }

    HLLoop *InnermostByStripLoop =
        addByStripLoops(AnchorNode, LoadInstsToClone, LiveOutsOfByStrip,
                        AuxRefsForByStripBounds);

    LLVM_DEBUG(dbgs() << "InnermostByStripLoop: \n");
    LLVM_DEBUG(InnermostByStripLoop->dump());

    // Step 2. Move all the HLNodes [AnchorNode, last NodeToMove] into
    // ByStripLoops.
    // There could be nodes (e.g HLInsts outside loops), which are
    // not related to any innermost loops or NodeToMoves
    // Note: Step 2. should come after Step 1. Once moved, findTheLowestAncestor
    //       returns different values.
    assert(AnchorNode == OuterNodeToInnermostLoop.front().first);

    HLNodeUtils::moveAsLastChildren(
        InnermostByStripLoop, AnchorNode->getIterator(),
        std::next(OuterNodeToInnermostLoop.back().first->getIterator()));

    // Cover all the HLNodes in the range of [AnchorNode, last NodeToMove].
    for (HLNode &Node : make_range(
             AnchorNode->getIterator(),
             std::next(OuterNodeToInnermostLoop.back().first->getIterator()))) {
      updateSpatialIVs(&Node, NumByStripLoops, OutermostNode);
      updateDefAtLevelOfSpatialLoops(&Node, OutermostNode);
    }

    // Step 3. apply blocking guards to target loops
    // Note: Step 3 should come after Step 2.
    assert(OuterNodeToInnermostLoop.size() == InnermostLoopToDimInfos.size());

    applyBlockingGuardsToSpatialLoops(InnermostLoopToAdjustingRef);

    // Normalize all spatial Loops and byStripLoops.
    normalizeSpatialLoops();

    LLVM_DEBUG_PROFIT_REPORT(dbgs() << "After updating in " << Func << ": ";
                             OutermostNode->dump());
    printMarker("Detail: After updating inner Loops: ", {OutermostNode}, true,
                true);

    Region->setGenCode();
    return true;
  }

private:
  // Given a representative ref, RepRef, come up with a ref where IVs are
  // cleared. Example: RepRef = A[i][j + 1] --> an AdjustingRef = A[0][1]
  //
  // Later, resulting AdjustingRefs are used for alignment of loops, where
  // a memref B[i + 1][j] will adjusted by A[0][1] being subtracted from
  // and become B[i+1][j-1].
  void prepareAdjustingRefs(LoopToRefTy &InnermostLoopToAdjustingRef) const {
    for (auto &Pair : InnermostLoopToRepRef) {
      RegDDRef *AdjustingRef = Pair.second->clone();
      llvm::for_each(
          make_range(AdjustingRef->canon_begin(), AdjustingRef->canon_end()),
          [](CanonExpr *CE) { CE->clearIVs(); });
      InnermostLoopToAdjustingRef.emplace(Pair.first, AdjustingRef);
    }
    assert(InnermostLoopToRepRef.size() == InnermostLoopToAdjustingRef.size());
  }

  // TODO: Non of the arguments are being changed by this function, but
  //       only scanned. See if "const" canbe used.
  void collectLiveInsToByStripLoops(HLNode *AnchorNode,
                                    HLNode *LastByStripNode) {

    // LiveIns to ByStripLoops
    //
    // DefRange - before the potential ByStripLoop
    // UseRange - the range of the potential ByStripLoop
    HLContainerTy::iterator DefBeginIt;
    DDGraph DDG;

    if (OutermostLoop) {
      DefBeginIt = OutermostLoop->child_begin();
      DDG = DDA.getGraph(OutermostLoop);
    } else {
      HLRegion *Region = OuterIf->getParentRegion();
      DefBeginIt = Region->child_begin();
      DDG = DDA.getGraph(Region);
    }

    HLContainerTy::iterator DefEndIt = AnchorNode->getIterator();
    unsigned UseStartTopSortNum = AnchorNode->getTopSortNum();
    unsigned UseLastTopSortNum = LastByStripNode->getMaxTopSortNum();

    SmallVector<unsigned, 16> LiveInsToByStrip;
    collectLiveInOutForByStripLoops<true>(DefBeginIt, DefEndIt,
                                          UseStartTopSortNum, UseLastTopSortNum,
                                          DDG, LiveInsToByStrip);
    std::copy(LiveInsToByStrip.begin(), LiveInsToByStrip.end(),
              std::back_inserter(LiveInsOfAllSpatialLoop));
  }

  SmallVector<unsigned, 16>
  collectLiveOutsOfByStripLoops(HLNode *AnchorNode, HLNode *LastByStripNode) {
    // LiveOuts of ByStripLoops
    //
    // DefRange - the range of the potential ByStripLoop
    // UseRange - after the potential ByStripLoop

    HLContainerTy::iterator DefBeginIt = AnchorNode->getIterator();
    HLContainerTy::iterator DefEndIt =
        std::next(LastByStripNode->getIterator());
    DDGraph DDG;
    HLNode *OutermostNode = nullptr;
    HLContainerTy::iterator OutermostEndIt;
    HLNode *EnclosingNodeWithLiveInfo = nullptr;
    HLRegion *Region = nullptr;

    if (OutermostLoop) {
      Region = OutermostLoop->getParentRegion();
      DDG = DDA.getGraph(OutermostLoop);
      OutermostNode = EnclosingNodeWithLiveInfo = OutermostLoop;
      OutermostEndIt = OutermostLoop->child_end();
    } else {
      Region = OuterIf->getParentRegion();
      DDG = DDA.getGraph(Region);
      OutermostNode = OuterIf;
      OutermostEndIt = OuterIf->child_end();
      EnclosingNodeWithLiveInfo = Region;
    }

    SmallVector<unsigned, 16> LiveOutsOfByStrip;
    if (DefEndIt != OutermostEndIt) {
      collectLiveInOutForByStripLoops<true>(
          DefBeginIt, DefEndIt, (*DefEndIt).getTopSortNum(),
          EnclosingNodeWithLiveInfo->getMaxTopSortNum(), DDG,
          LiveOutsOfByStrip);
    } else if (std::next(OutermostNode->getIterator()) != Region->child_end()) {
      // Same as the following in effect
      // OuterIf && std::next(OuterIf->getIterator()) != Region->child_end()
      // UseBegin should  be std::next of OuterIf.
      collectLiveInOutForByStripLoops<false>(
          DefBeginIt, DefEndIt,
          (*std::next(OutermostNode->getIterator())).getTopSortNum(),
          EnclosingNodeWithLiveInfo->getMaxTopSortNum(), DDG,
          LiveOutsOfByStrip);
    }

    return LiveOutsOfByStrip;
  }

  // Collect LiveIns and LiveOuts.
  // [DefBeginIt, DefEndIt) is the range where Lvals are found.
  // [UseStartTopSortNum, UseTopSortNum] is the range of TopSortNumbers where
  // uses are found. If an edge from the def-range to use-range exists, the
  // symbase of the corresponding lval(ddref)'s symbase is populated into
  // LiveInOrOut. Being LiveIn or LiveOut are dependent on the caller site of
  // this function.
  template <bool IsAllRefer = false>
  void collectLiveInOutForByStripLoops(HLContainerTy::iterator DefBeginIt,
                                       HLContainerTy::iterator DefEndIt,
                                       unsigned UseStartTopSortNum,
                                       unsigned UseLastTopSortNum, DDGraph DDG,
                                       SmallVectorImpl<unsigned> &LiveInOrOut) {

    for (HLRangeIterator It = HLRangeIterator(DefBeginIt),
                         EIt = HLRangeIterator(DefEndIt);
         It != EIt; ++It) {

      // Currently, EIt is alwasy dereferencible in the caller site.
      if (IsAllRefer && (*It) && (*EIt) && *It == *EIt)
        break;

      const HLDDNode *DDNode = dyn_cast<HLDDNode>(*It);
      if (!DDNode)
        continue;

      const RegDDRef *Lval = DDNode->getLvalDDRef();
      if (!Lval || !Lval->isSelfBlob())
        continue;

      // Assume no blob ddrefs from Lval
      for (auto *Edge : DDG.outgoing(Lval)) {
        const HLDDNode *SinkNode = Edge->getSink()->getHLDDNode();
        unsigned TopSortNum = SinkNode->getTopSortNum();

        if (TopSortNum >= UseStartTopSortNum &&
            TopSortNum <= UseLastTopSortNum) {
          LiveInOrOut.push_back(Lval->getSymbase());
        }
      }
    }
  }

  void updateDefAtLevelOfSpatialLoops(HLNode *Node,
                                      const HLNode *OutermostNode) const {

    // Increase the blob DDRef's defined at level first

    unsigned ThresholdLoopLevel = OutermostNode->getNodeLevel();
    unsigned ByStripLoopDepth = NumByStripLoops;

    ForEach<RegDDRef>::visit(
        Node, [ThresholdLoopLevel, ByStripLoopDepth](RegDDRef *Ref) {
          Transformer::incDefinedAtLevelBy(Ref, ByStripLoopDepth,
                                           ThresholdLoopLevel);
        });
  }

  // Increase def@level of Ref by Increase if current def@level is
  // greater than equal to LevelThreshold.
  static void incDefinedAtLevelBy(RegDDRef *Ref, unsigned Increase,
                                  unsigned LevelThreshold) {

    auto IncreaseDefLevel = [Increase, LevelThreshold](CanonExpr *CE) {
      unsigned PrevDefLevel = CE->getDefinedAtLevel();

      if (PrevDefLevel < LevelThreshold)
        return;

      assert(PrevDefLevel + Increase <= MaxLoopNestLevel);

      CE->setDefinedAtLevel(PrevDefLevel + Increase);
    };

    // Increase Def@Level of Blob CEs.
    if (Ref->isSelfBlob() && !Ref->isNonLinear()) {
      CanonExpr *CE = Ref->getSingleCanonExpr();

      IncreaseDefLevel(CE);
    }

    for (auto *BRef : make_range(Ref->blob_begin(), Ref->blob_end())) {
      CanonExpr *CE = BRef->getSingleCanonExpr();

      if (CE->isNonLinear())
        continue;

      IncreaseDefLevel(CE);
    }

    // Caculate correct def@level using update Blob's def@level
    Ref->updateDefLevel();
  }

  // Add AdjustingRef to loop's bounds.
  CanonExpr *alignSpatialLoopBounds(RegDDRef *Ref, const RegDDRef *AdjustingRef,
                                    unsigned DimNum) const {

    std::unique_ptr<RegDDRef> OrigRef(Ref->clone());
    CanonExpr *CE = Ref->getSingleCanonExpr();
    const CanonExpr *AdjustCE = AdjustingRef->getDimensionIndex(DimNum);
    bool Success = CanonExprUtils::add(CE, AdjustCE);
    assert(Success);
    (void)Success;

    Ref->makeConsistent({OrigRef.get(), AdjustingRef});

    return CE;
  }

  // Adjust a loops LB, and UB and subscripts so that
  // all Lval ddrefs has only IVs but no constant of blob
  // in every dimension.
  // For example, if a lval was a[i + const1][j + blob]
  // it will become a[i][j] by subtraction const1, blob
  // Example:
  // Input loop before alignment
  // for i = 0, N
  //  for j = 0, M
  //   a[i][j+1] = b[i][j] + 3;
  //
  // After alignment
  // for i = 0, N
  //  for j = 1, M + 1
  //   a[i][j] = b[i][j - 1] + 3;
  void alignSpatialLoops(const LoopToRefTy &InnermostLoopToAdjustingRef) {

    // TODO: Why only innermost loop?
    //       This is under the assumption, no other loop body contains
    //       non-loop-invariant memrefs. At least make sure if that is true.

    // Loops are located from an innermost loop.
    // A Loop can contain more than one innermost loop.
    // ProcessedTargetLoops records those (non-innermost) loops
    // so that each may be processed exactly once.
    std::unordered_set<const HLLoop *> ProcessedTargetLoops;
    int GlobalNumDims = StripmineSizes.size();

    // Loop bodies
    for (auto &LoopAndDimInfoVec : InnermostLoopToDimInfos) {

      DenseMap<unsigned, unsigned> MapFromLevelToDim;
      auto &DimInfo = LoopAndDimInfoVec.second;
      unsigned DimInfoVecSize = DimInfo.size();
      HLLoop *InnermostLoop = LoopAndDimInfoVec.first;
      unsigned Level = InnermostLoop->getNestingLevel();

      for (unsigned I = 0, Size = DimInfoVecSize; I < Size; I++) {
        if (!DimInfo[I].hasIV())
          continue;

        MapFromLevelToDim.insert({Level - DimInfo[I].LevelOffset, I + 1});
      }

      for (int DimNum = 1; DimNum <= GlobalNumDims; DimNum++) {

        HLLoop *TargetLoop =
            getLoopMatchingDimNum(DimNum, DimInfoVecSize, InnermostLoop);
        if (!TargetLoop)
          continue;

        if (ProcessedTargetLoops.count(TargetLoop))
          continue;
        else
          ProcessedTargetLoops.insert(TargetLoop);

        const RegDDRef *AdjustingRef =
            InnermostLoopToAdjustingRef.at(InnermostLoop);

        LoopBodyAligner(TargetLoop, AdjustingRef, MapFromLevelToDim).update();
      }
    }

    ProcessedTargetLoops.clear();

    // Loop bounds
    for (int DimNum = 1; DimNum <= GlobalNumDims; DimNum++) {

      // Alignment should be done at all levels, regardless of
      // being loop-tiled or not.

      // Collect all Lower/AlignedUpperBounds from InnermostLoop
      for (auto &LoopAndDimInfoVec : InnermostLoopToDimInfos) {
        HLLoop *InnermostLoop = LoopAndDimInfoVec.first;

        HLLoop *TargetLoop = getLoopMatchingDimNum(
            DimNum, LoopAndDimInfoVec.second.size(), InnermostLoop);
        if (!TargetLoop)
          continue;

        if (ProcessedTargetLoops.count(TargetLoop))
          continue;
        else
          ProcessedTargetLoops.insert(TargetLoop);

        const RegDDRef *AdjustingRef =
            InnermostLoopToAdjustingRef.at(InnermostLoop);

        printMarker("LB before: ", {TargetLoop->getLowerDDRef()}, true);
        alignSpatialLoopBounds(TargetLoop->getLowerDDRef(), AdjustingRef,
                               DimNum);
        printMarker("LB after: ", {TargetLoop->getLowerDDRef()}, true);
        alignSpatialLoopBounds(TargetLoop->getUpperDDRef(), AdjustingRef,
                               DimNum);
      }
    }
  }

  // Only for debugging.
  static void printDDEdges(const HLInst *LoadInst, DDGraph DDG) {
    auto *RvalOfLoad = LoadInst->getRvalDDRef();
    if (DDG.incoming_edges_begin(RvalOfLoad) !=
        DDG.incoming_edges_end(RvalOfLoad)) {
      dbgs() << "== incoming edges\n";
      RvalOfLoad->dump();
      dbgs() << "\n";
      for (auto *Edge : DDG.incoming(RvalOfLoad)) {
        Edge->dump();
      }
    } else if (DDG.outgoing_edges_begin(RvalOfLoad) !=
               DDG.outgoing_edges_end(RvalOfLoad)) {
      dbgs() << "== Outgoing edges\n";
      RvalOfLoad->dump();
      dbgs() << "\n";
    }

    for (auto *BlobRef :
         make_range(RvalOfLoad->blob_begin(), RvalOfLoad->blob_end())) {
      if (DDG.incoming_edges_begin(BlobRef) !=
          DDG.incoming_edges_end(BlobRef)) {
        dbgs() << "= incoming edges\n";
        BlobRef->dump();
        dbgs() << "\n";
      } else if (DDG.outgoing_edges_begin(BlobRef) !=
                 DDG.outgoing_edges_end(BlobRef)) {
        dbgs() << "= Outgoing edges\n";
        BlobRef->dump();
        dbgs() << "\n";
      }
    }
  }

  bool checkInvariance(const HLInst *HInst) const {
    if (OutermostLoop) {
      unsigned Level = OutermostLoop->getNestingLevel();
      for (auto *Rval : make_range(HInst->rval_op_ddref_begin(),
                                   HInst->rval_op_ddref_end())) {
        if (!Rval->isLinearAtLevel(Level)) {
          LLVM_DEBUG(dbgs()
                     << "We don't expect dependency to non-linear ops.\n");
          return false;
        }
      }
    } else {
      for (auto *Rval : make_range(HInst->rval_op_ddref_begin(),
                                   HInst->rval_op_ddref_end())) {
        if (!Rval->isStructurallyRegionInvariant()) {
          LLVM_DEBUG(dbgs() << "Should be region live-in.\n");
          return false;
        }
      }
    }
    return true;
  }

  // Start from the RHS of Copy or any other instruction
  // to find the eventual load instruction or the instruction
  // whose rvals are all liveIn to the region.
  // If not found or meet an unexpected situation, return false.
  // Example 1)
  //   %t1 = %a[..] -- (1)
  //   %t2 = %t1    -- (2)
  // Starting from %t1, a load instruction (1) is found.
  // Example 2)
  //   %t1 = %a[..] -- (1)
  //   %t3 = %b[..] -- (2)
  //   %t2 = %t1 + %t3   -- (3)
  // Starting from  %t1 and %t3, loads (1) and (2) are found.
  // Example 3)
  //   %t1 = %liveIn0 < %liveIn1; --(1)
  //   %t2 = (%liveIn2 != 1) ? -1 : %t1; --(2)
  // From %t1 in (2) inst (1) is found.
  bool tracebackToLoad(const RegDDRef *Rval, DDGraph DDG,
                       SmallVectorImpl<const HLInst *> &Res) const {

    std::deque<const RegDDRef *> RvalQueue;
    RvalQueue.push_back(Rval);

    while (!RvalQueue.empty()) {
      Rval = RvalQueue[0];
      RvalQueue.pop_front();
      if (Rval->isStructurallyRegionInvariant())
        continue;

      for (auto *Edge : DDG.incoming(Rval)) {
        if (!Edge->isFlow())
          continue;

        HLNode *SrcNode = Edge->getSrc()->getHLDDNode();
        const HLInst *LoadOrCopy = dyn_cast<HLInst>(SrcNode);
        if (!LoadOrCopy)
          return false;

        if (isa<LoadInst>(LoadOrCopy->getLLVMInstruction())) {

          printMarker(" == Found Load: \n", {LoadOrCopy});

          if (!checkInvariance(LoadOrCopy))
            return false;

          Res.push_back(LoadOrCopy);

        } else if (LoadOrCopy->isCopyInst()) {

          printMarker("Found Copy: \n", {LoadOrCopy});

          RvalQueue.push_back(LoadOrCopy->getRvalDDRef());

        } else if (LoadOrCopy->isCallInst()) {

          printMarker("Found CallInst: \n", {LoadOrCopy});

          return false;

        } else {
          for (auto *RvalRef : make_range(LoadOrCopy->rval_op_ddref_begin(),
                                          LoadOrCopy->rval_op_ddref_end())) {
            RvalQueue.push_back(RvalRef);
          }
        }
      }
    } // while
    return true;
  }

  // Find the load instruction starting from SrcNode.
  // If SrcNode is a load, return it.
  // If it is a copy, trace back to a load and return it.
  bool findLoad(
      const HLDDNode *SrcNode, DDGraph DDG,
      SmallVectorImpl<std::pair<const HLInst *, const HLInst *>> &Res) const {

    const HLInst *LoadOrCopy = dyn_cast<HLInst>(SrcNode);
    if (!LoadOrCopy)
      return false;

    if (isa<LoadInst>(LoadOrCopy->getLLVMInstruction())) {

      printMarker(" == Found Load: \n", {LoadOrCopy});

      Res.push_back({LoadOrCopy, nullptr});
      return checkInvariance(LoadOrCopy);

    } else if (LoadOrCopy->isCopyInst()) {

      printMarker("Found Copy: \n", {LoadOrCopy});

      SmallVector<const HLInst *> Insts;
      if (!tracebackToLoad(LoadOrCopy->getRvalDDRef(), DDG, Insts) ||
          Insts.size() != 1)
        return false;

      Res.push_back({Insts.back(), LoadOrCopy});

    } else if (LoadOrCopy->isCallInst()) {

      LLVM_DEBUG(dbgs() << "We don't expect a dependency to a call.");

      return false;

    } else { // any other inst

      printMarker("Found non-load clone: \n", {LoadOrCopy});

      Res.push_back({LoadOrCopy, nullptr});
      SmallVector<const HLInst *> Insts;
      for (auto *Rval : make_range(LoadOrCopy->rval_op_ddref_begin(),
                                   LoadOrCopy->rval_op_ddref_end())) {
        if (!tracebackToLoad(Rval, DDG, Insts))
          return false;
      }

      for (auto *Inst : Insts) {
        Res.push_back({Inst, nullptr});
      }
    }
    return true;
  }

  template <typename IteratorTy>
  bool findLoadsOfTemp(
      DDGraph DDG, IteratorTy begin, IteratorTy end,
      unsigned AnchorNodeTopSortNum, InstsToCloneSetTy &LoadInsts,
      std::map<const HLInst *, const HLInst *> &CopyToLoadMap) const {

    for (IteratorTy It = begin; It != end; ++It) {
      auto *DestRef = *It;

      for (auto *Edge : DDG.incoming(DestRef)) {

        if (!Edge->isFlow())
          continue;

        SmallVector<std::pair<const HLInst *, const HLInst *>> InstPairs;
        if (!findLoad(Edge->getSrc()->getHLDDNode(), DDG, InstPairs))
          return false;

        for (auto &LoadAndCopy : InstPairs) {

          if (!LoadAndCopy.first ||
              LoadAndCopy.first->getTopSortNum() < AnchorNodeTopSortNum)
            continue;

          LLVM_DEBUG_DD_EDGES(printDDEdges(LoadAndCopy.first, DDG));

          if (LoadAndCopy.second)
            CopyToLoadMap.emplace(LoadAndCopy.second, LoadAndCopy.first);

          LoadInsts.insert(LoadAndCopy.first);
        }
      }
    }
    return true;
  }

  bool collectLoadsToClone(const HLNode *AnchorNode,
                           InstsToCloneSetTy &LoadInstsToClone,
                           SmallVectorImpl<std::pair<unsigned, unsigned>>
                               &CopyToLoadIndexMap) const {

    DDGraph DDG;
    if (OutermostLoop) {
      DDG = DDA.getGraph(OutermostLoop);
    } else {
      DDG = DDA.getGraph(AnchorNode->getParentRegion());
    }

    unsigned AnchorNodeTopSortNum = AnchorNode->getTopSortNum();

    std::map<const HLInst *, const HLInst *> CopyToLoadMap;
    // Collect loads for RepRef
    for (auto &E : InnermostLoopToDimInfos) {
      const HLLoop *InnermostLoop = E.first;

      const RegDDRef *RepRef = InnermostLoopToRepRef.at(InnermostLoop);

      // RepRef is needed because DDG is based on those refs. (aka original refs
      // before transformation) Assumption: load for BaseCE of RepRef is not
      // needed.
      // It is tempting to try collecting load instructions after
      // alignSpatialLoops because that function has the updated loop bounds by
      // alignment, and loads required for those loop bounds are what we want to
      // collect. But, that doesn't work, because after alignment, ddrefs are
      // updated and we do not have a way to access dependencies in the original
      // DDG. Thus, we need original refs. Alignment will transform the original
      // LB to LB + (RepRef's tmp). Of course RepRef's tmp will be cloned in the
      // newly created LB. But at least we know, before the actual
      // transformation, that clones of tmps in RepRef may appear in the new
      // loop bounds.
      // Notice that only the DDRefs related to the Rep's indices are
      // collected. No lower bound CE or stride CEs are considered because
      // only the index parts contribute to the by-strip loop's LB/UB.
      SmallVector<unsigned, 8> BlobsInIndices;
      for (auto *IndexCE :
           make_range(RepRef->canon_begin(), RepRef->canon_end())) {
        IndexCE->collectTempBlobIndices(BlobsInIndices, false);
      }
      std::sort(BlobsInIndices.begin(), BlobsInIndices.end());
      auto It = std::unique(BlobsInIndices.begin(), BlobsInIndices.end());
      BlobsInIndices.erase(It, BlobsInIndices.end());

      SmallVector<const DDRef *, 8> RefsInIndices;
      for (auto *BRef : make_range(RepRef->blob_begin(), RepRef->blob_end())) {
        auto It = llvm::find(BlobsInIndices, BRef->getBlobIndex());
        if (It != BlobsInIndices.end())
          RefsInIndices.push_back(BRef);
      }

      if (!findLoadsOfTemp(DDG, std::begin(RefsInIndices),
                           std::end(RefsInIndices), AnchorNodeTopSortNum,
                           LoadInstsToClone, CopyToLoadMap))
        return false;
    }

    // Collect loads for Spatial loop's upper bounds
    // Lowerbounds are skipped because of being "0" by normalization
    int GlobalNumDims = StripmineSizes.size();
    for (int DimNum = 1; DimNum <= GlobalNumDims; DimNum++) {
      if (isNoBlockDim(DimNum))
        continue;

      // Collect all Lower/AlignedUpperBounds from InnermostLoop
      for (auto &E : InnermostLoopToDimInfos) {
        const HLLoop *InnermostLoop = E.first;

        const HLLoop *TargetLoop =
            getLoopMatchingDimNum(DimNum, E.second.size(), InnermostLoop);
        if (!TargetLoop)
          continue;

        if (!findLoadsOfTemp(DDG, TargetLoop->getUpperDDRef()->all_dd_begin(),
                             TargetLoop->getUpperDDRef()->all_dd_end(),
                             AnchorNodeTopSortNum, LoadInstsToClone,
                             CopyToLoadMap))
          return false;
      }
    }

    // Create a map of old to new blob indicies
    for (auto &CopyToLoad : CopyToLoadMap) {
      unsigned OldSB = CopyToLoad.first->getLvalDDRef()->getSymbase();
      unsigned OldIndex =
          (CopyToLoad.first->getBlobUtils()).findTempBlobIndex(OldSB);
      unsigned NewIndex = CopyToLoad.second->getLvalDDRef()
                              ->getSingleCanonExpr()
                              ->getSingleBlobIndex();

      assert(OldIndex != InvalidBlobIndex);
      assert(OldIndex != NewIndex);
      CopyToLoadIndexMap.emplace_back(OldIndex, NewIndex);

      LLVM_DEBUG(dbgs() << "Map OldIndex: " << OldIndex
                        << ", NewIndex: " << NewIndex << "\n";
                 CopyToLoad.first->dump(1); dbgs() << "\n";
                 CopyToLoad.second->dump(1); dbgs() << "\n");
    }

    return true;
  }

  // LB(UB) of a By-strip loop is the "min"("max") of all lower bounds of
  // original spatial loop corresponding to the same DimNum.
  // This function collects all LBs(UBs) of original spatial loops, and
  // generate min(max) blobs of them.
  bool computeByStripLoopBounds(
      const LoopToRefTy &InnermostLoopToAdjustingRef,
      const DenseMap<unsigned, unsigned> &OrigToCloneIndexMap,
      SmallVectorImpl<const RegDDRef *> &AuxRefs) {

    assert(InnermostLoopToAdjustingRef.size() > 0);
    LLVMContext &Context = (*InnermostLoopToAdjustingRef.begin())
                               .first->getHLNodeUtils()
                               .getContext();

    int GlobalNumDims = StripmineSizes.size();
    SmallVector<SmallVector<CanonExpr *, 32>, 4> AlignedLowerBounds(
        GlobalNumDims);
    SmallVector<SmallVector<CanonExpr *, 32>, 4> AlignedUpperBounds(
        GlobalNumDims);
    for (int DimNum = 1; DimNum <= GlobalNumDims; DimNum++) {
      if (isNoBlockDim(DimNum))
        continue;

      // Collect all Lower/AlignedUpperBounds from InnermostLoop
      for (auto &E : InnermostLoopToDimInfos) {
        const HLLoop *InnermostLoop = E.first;

        const HLLoop *TargetLoop =
            getLoopMatchingDimNum(DimNum, E.second.size(), InnermostLoop);
        if (!TargetLoop)
          continue;

        RegDDRef *NewLBRef = TargetLoop->getLowerDDRef()->clone();
        bool Replaced = NewLBRef->replaceTempBlobs(OrigToCloneIndexMap);
        (void)Replaced;
        AlignedLowerBounds[DimNum - 1].push_back(
            NewLBRef->getSingleCanonExpr());
        AuxRefs.push_back(TargetLoop->getLowerDDRef());

        LLVM_DEBUG(dbgs() << "LB after (" << Replaced << ") ";
                   NewLBRef->dump(1); dbgs() << "\n");

        RegDDRef *NewUBRef = TargetLoop->getUpperDDRef()->clone();
        Replaced = NewUBRef->replaceTempBlobs(OrigToCloneIndexMap);
        AlignedUpperBounds[DimNum - 1].push_back(
            NewUBRef->getSingleCanonExpr());
        AuxRefs.push_back(TargetLoop->getUpperDDRef());

        LLVM_DEBUG(dbgs() << "UB after (" << Replaced << ") ");
        LLVM_DEBUG(NewUBRef->dump(1); dbgs() << "\n");
      } // end of InnermostLoop

      // Uniq and get minimum of AlignedLowerBounds
      removeDupCanonExprs(AlignedLowerBounds[DimNum - 1]);
      std::pair<BlobTy, unsigned> MinBlob =
          getGlobalMinMaxBlob<true>(AlignedLowerBounds[DimNum - 1]);
      if (!MinBlob.first)
        return false;

      // Uniq and get maxium of AlignedLowerBounds
      removeDupCanonExprs(AlignedUpperBounds[DimNum - 1]);
      std::pair<BlobTy, unsigned> MaxBlob =
          getGlobalMinMaxBlob<false>(AlignedUpperBounds[DimNum - 1]);
      if (!MaxBlob.first)
        return false;

      if (!InnermostLoopToShift[DimNum - 1].empty()) {
        int64_t ShiftVal = InnermostLoopToShift[DimNum - 1].back() + 1;
        BlobUtils &BU = AlignedUpperBounds[DimNum - 1].front()->getBlobUtils();
        BlobTy ShiftBlob = BU.createBlob(ShiftVal, Type::getInt64Ty(Context));

        unsigned BlobIndex;
        ShiftBlob = BU.createAddBlob(MaxBlob.first, ShiftBlob,
                                     true /* insert ? */, &BlobIndex);
        MaxBlob = {ShiftBlob, BlobIndex};
      }

      LLVM_DEBUG(dbgs() << "== Alinged LowerBounds == \n";
                 for (auto *Ref
                      : AlignedLowerBounds[DimNum - 1]) {
                   Ref->dump();
                   dbgs() << "\n";
                 });
      LLVM_DEBUG(dbgs() << "= Aligned UpperBounds == \n";
                 for (auto *Ref
                      : AlignedUpperBounds[DimNum - 1]) {
                   Ref->dump();
                   dbgs() << "\n";
                 });

      ByStripLoopLowerBlobs[DimNum - 1] = MinBlob;
      ByStripLoopUpperBlobs[DimNum - 1] = MaxBlob;

      LLVM_DEBUG_PROFIT_REPORT(dbgs() << "ByStripLoop LB at DimNum " << DimNum
                                      << " : ");
      LLVM_DEBUG_PROFIT_REPORT(
          BlobUtils &BU =
              AlignedUpperBounds[DimNum - 1].front()->getBlobUtils();
          BU.printBlob(dbgs(), MinBlob.first); dbgs() << "\n";);
      LLVM_DEBUG_PROFIT_REPORT(dbgs() << "ByStripLoop UB at DimNum " << DimNum
                                      << " : ");
      LLVM_DEBUG_PROFIT_REPORT(
          BlobUtils &BU =
              AlignedUpperBounds[DimNum - 1].front()->getBlobUtils();
          BU.printBlob(dbgs(), MaxBlob.first); dbgs() << "\n";);

    } // end of dimnum

    return true;
  }

  // Replace all use with new Lval
  void cloneAndAddLoadInsts(
      InstsToCloneSetTy &LoadInstsToClone, HLNode *AnchorNode,
      DenseMap<unsigned, unsigned> &OrigToCloneIndexMap,
      SmallVectorImpl<const RegDDRef *> &AuxRefsForByStripBounds

  ) {

    SmallVector<HLInst *> NewLoads;

    for (auto *Load : LoadInstsToClone) {
      HLInst *NewLoad = Load->clone();
      RegDDRef *NewLval = Load->getHLNodeUtils().createTemp(
          NewLoad->getLvalDDRef()->getDestType(), "clone");
      NewLoad->replaceOperandDDRef(NewLoad->getLvalDDRef(), NewLval);
      NewLoads.push_back(NewLoad);

      unsigned OldIndex =
          Load->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex();

      unsigned NewIndex =
          NewLoad->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex();

      assert(OldIndex != NewIndex);

      OrigToCloneIndexMap.insert({OldIndex, NewIndex});

      printMarker("Orig Load: ", Load, true, true);
      printMarker("New Load: ", NewLoad, true, true);

      HLNodeUtils::insertBefore(AnchorNode, NewLoad);

      LiveInsOfAllSpatialLoop.push_back(NewLoad->getLvalDDRef()->getSymbase());
      AuxRefsForByStripBounds.push_back(NewLoad->getLvalDDRef());
    }

    // Replace rvals in the cloned instructions as needed.
    // The need can arise because of dependencies among the cloned nodes.
    // E.g. before clone
    // A = load ..
    // B = k < 1 ? A : 1;
    // after clone
    // A' = load ..
    // B' = k < 1 ? A' : 1; -- rval A should be replaced by A'
    for (auto *Load : NewLoads) {
      for (auto *Ref :
           make_range(Load->rval_op_ddref_begin(), Load->rval_op_ddref_end())) {
        Ref->replaceTempBlobs(OrigToCloneIndexMap);
      }
    }
  }

  // Generate by-strip loops and insert before AnchorNode.
  // Returns the innermost by-strip loop, where spatial loops will be added.
  // Add ByStrip loops, and also compute UBs of unit-strided Loop
  //   e.g. DO IV = by_strip_lb, by_strip_ub, by_strip_step
  //          tile_end = min(IV + by_strip_step - 1, by_strip_ub)
  //   IV is the tile's begin.
  //   tile_end is the last element of tile, not the past the last.
  HLLoop *addByStripLoops(HLNode *AnchorNode,
                          const InstsToCloneSetTy &LoadInstsToClone,
                          const SmallVectorImpl<unsigned> &LiveOutsOfByStrip,
                          ArrayRef<const RegDDRef *> AuxRefsFromSpatialLoops) {

    HLRegion *Region = AnchorNode->getParentRegion();

    // Use computed Global Loop bounds
    // A higher dimension corresponds to an outer loop
    // Process from the outermost loop to innermost loop.

    SmallVector<const RegDDRef *, 32> AuxRefs;
    for (auto Load : LoadInstsToClone) {
      AuxRefs.push_back(Load->getLvalDDRef());
    }

    AuxRefs.insert(AuxRefs.end(), AuxRefsFromSpatialLoops.begin(),
                   AuxRefsFromSpatialLoops.end());

    LLVM_DEBUG(dbgs() << "Auxrefs ...\n"; for (auto *Aux
                                               : AuxRefs) {
      Aux->dump(1);
      dbgs() << " @" << Aux->getDefinedAtLevel() << "\n";
    });

    HLLoop *ParentByStripLoop = nullptr;
    unsigned PrevTileEndSymbase = 0;
    for (unsigned DimNum = StripmineSizes.size(); DimNum >= 1; DimNum--) {
      if (isNoBlockDim(DimNum))
        continue;

      HLLoop *ByStrip = (*InnermostLoopToDimInfos.begin()).first->cloneEmpty();

      // Ztt will be added later. Make it clean first in case src had ztt.
      ByStrip->removeZtt();

      HIRTransformUtils::setSelfBlobDDRef(
          ByStrip->getLowerDDRef(), ByStripLoopLowerBlobs[DimNum - 1].first,
          ByStripLoopLowerBlobs[DimNum - 1].second);
      HIRTransformUtils::setSelfBlobDDRef(
          ByStrip->getUpperDDRef(), ByStripLoopUpperBlobs[DimNum - 1].first,
          ByStripLoopUpperBlobs[DimNum - 1].second);

      // TODO: These may not be needed. LiveIns are populated in other two
      // places.
      //       (1) Through pre-transformation DDG of the outermost loop or
      //       region. (2) From cloned load instruction. The above two might
      //       cover the following. However, I did not verify fully yet.
      ByStrip->getLowerDDRef()->populateTempBlobSymbases(
          LiveInsOfAllSpatialLoop);
      ByStrip->getUpperDDRef()->populateTempBlobSymbases(
          LiveInsOfAllSpatialLoop);

      CanonExpr *StrideCE = ByStrip->getStrideCanonExpr();
      StrideCE->clear();
      StrideCE->setConstant(StripmineSizes[DimNum - 1]);
      ByStrip->getStrideDDRef()->setSymbase(ConstantSymbase);

      if (!ParentByStripLoop) {
        HLNodeUtils::insertBefore(AnchorNode, ByStrip);
      } else {
        HLNodeUtils::insertAsLastChild(ParentByStripLoop, ByStrip);
      }

      // This if where, Lval refs are passed as AuxRefs.
      // makeConsistent's logic needed some changes.
      unsigned L = ByStrip->getNestingLevel();
      ByStrip->getLowerDDRef()->makeConsistent(AuxRefs, L);
      ByStrip->getUpperDDRef()->makeConsistent(AuxRefs, L);
      ByStrip->createZtt(false, true);

      LLVM_DEBUG(dbgs() << "ByStrip Loop (Number, Level, Def@) "
                        << ByStrip->getNumber() << ", "
                        << ByStrip->getNestingLevel() << ", "
                        << ByStrip->getLowerDDRef()->getDefinedAtLevel()
                        << "\n");
      printMarker(" ", {ByStrip->getLowerDDRef()});

      if (ParentByStripLoop) {
        LiveInsOfAllSpatialLoop.push_back(PrevTileEndSymbase);
      }
      ByStrip->addLiveInTemp(LiveInsOfAllSpatialLoop);

      if (OutermostLoop) {
        ByStrip->addLiveInTemp(ArrayRef<unsigned>(
            OutermostLoop->live_in_begin(), OutermostLoop->live_in_end()));
      } else {
        // TODO: This might not be needed because now region's DDG is scanned
        //       from its child_begin() using function
        //       collectLiveInOutForByStrip. However, using Region's live_in as
        //       shown below is fater in compile time than scanning through DDG
        //       with a tradeoff of accuracy. Consider using the following
        //       instead of collectLiveInOutForBystrip.
        for (auto &Pair :
             make_range(Region->live_in_begin(), Region->live_in_end())) {
          ByStrip->addLiveInTemp(Pair.first);
        }
      }

      // TODO: replace with the latest util eating the array.
      std::for_each(
          LiveOutsOfByStrip.begin(), LiveOutsOfByStrip.end(),
          [ByStrip](unsigned Symbase) { ByStrip->addLiveOutTemp(Symbase); });

      // TileEnd is IV + (step capped by UB
      HLInst *TileEnd = createTileEnd(ByStrip);
      HLNodeUtils::insertAsFirstChild(ByStrip, TileEnd);
      PrevTileEndSymbase = TileEnd->getLvalDDRef()->getSymbase();

      ByStripLoops[DimNum - 1] = ByStrip;

      ParentByStripLoop = ByStrip;
    }

    // ParentByStripLoop designate the innermost by-strip loop
    return ParentByStripLoop;
  }

  // IV update caused by stripmining.
  // Increase all IV levels greater than equal to LowestSpatialLoopLevel
  // by ByStripLoopDepth.
  // For example, if ByStripLoopDepth = 3, and LowestSpatialLoopLevel = 2
  // i1, i2, i3, i4 becomes
  // --> i1, i5, i6, i7
  void updateSpatialIVs(HLNode *Node, unsigned ByStripLoopDepth,
                        const HLNode *OutermostNode) {

    // Now loop levels get as deep as by-strip loopnests
    unsigned OutermostLevel = OutermostNode->getNodeLevel();
    ForEach<RegDDRef>::visit(
        Node, [OutermostLevel, ByStripLoopDepth](RegDDRef *Ref) {
          for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
            for (unsigned Lvl = MaxLoopNestLevel; Lvl > OutermostLevel; Lvl--) {
              unsigned Index;
              int64_t Coeff;
              CE->getIVCoeff(Lvl, &Index, &Coeff);
              if (Coeff == 0)
                continue;

              CE->removeIV(Lvl);
              CE->setIVCoeff(Lvl + ByStripLoopDepth, Index, Coeff);
            }
          }
        });
  }

  // TODO: Make it a local lambda.
  std::pair<const RegDDRef *, unsigned>
  findAuxRefWithCE(const HLLoop *InnermostLoop, const CanonExpr *TargetCE) {

    for (const HLNode &Node :
         make_range(InnermostLoop->child_begin(), InnermostLoop->child_end())) {
      const HLDDNode *DDNode = dyn_cast<HLDDNode>(&Node);
      for (const RegDDRef *Ref :
           make_range(DDNode->ddref_begin(), DDNode->ddref_end())) {
        if (!Ref->isMemRef())
          continue;

        for (const CanonExpr *CE :
             make_range(Ref->canon_begin(), Ref->canon_end())) {
          if (CanonExprUtils::areEqual(CE, TargetCE, false, true))
            return {Ref, CE->getDefinedAtLevel()};
        }
      }
    }

    LLVM_DEBUG(InnermostLoop->dump(1));
    LLVM_DEBUG(TargetCE->dump(1); dbgs() << "\n");
    llvm_unreachable("blob CE should have been found.");
  }

  // Add blocking guards to loop bounds or as an if-stmt.
  // When index CEs are constant or blob, i.e., no loop exists corresponding to
  // that dimension, if-stmt is added.
  // Also, update live-in temps.
  void applyBlockingGuardsToSpatialLoops(
      const LoopToRefTy &InnermostLoopToAdjustingRef) {

    std::unordered_set<HLLoop *> ProcessedTargetLoops;

    for (int InnermostLoopID = 0, E = InnermostLoopToDimInfos.size();
         InnermostLoopID < E; InnermostLoopID++) {

      HLLoop *InnermostLoop = InnermostLoopToDimInfos[InnermostLoopID].first;
      const DimInfoVecTy &DimInfos =
          InnermostLoopToDimInfos[InnermostLoopID].second;

      SmallVector<unsigned, 2> ConstOrBlobDimNums;

      SmallVector<unsigned, 4> AllNewLiveIn;
      SmallVector<HLLoop *, 3> SpatialLoops;

      // Scan through from the innermost spatial loop to facilitate adding
      // LiveIns
      for (unsigned DimNum = 1, Sizes = DimInfos.size(); DimNum <= Sizes;
           DimNum++) {

        if (isNoBlockDim(DimNum))
          continue;

        HLLoop *TargetLoop =
            getLoopMatchingDimNum(DimNum, Sizes, InnermostLoop);

        if (!TargetLoop) {
          ConstOrBlobDimNums.push_back(DimNum);
          continue;
        }

        // Notice that nullptr is not pushed into the set.
        if (ProcessedTargetLoops.count(TargetLoop))
          continue;
        else
          ProcessedTargetLoops.insert(TargetLoop);

        SpatialLoops.push_back(TargetLoop);

        HIRInvalidationUtils::invalidateBounds(TargetLoop);
        HIRInvalidationUtils::invalidateBody(TargetLoop);

        int64_t Shift = 0;
        if (!InnermostLoopToShift[DimNum - 1].empty()) {
          Shift = InnermostLoopToShift[DimNum - 1][InnermostLoopID];
        }
        unsigned NewLiveIn = addLoopBoundsGuards(TargetLoop, DimNum, Shift);

        AllNewLiveIn.push_back(NewLiveIn);
        TargetLoop->addLiveInTemp(AllNewLiveIn);
        TargetLoop->createZtt(true, true);

        // After addLoopBoundsGuards,
        // Lower and Upper bounds are always self blobs.
        assert(TargetLoop->getLowerDDRef()->isSelfBlob() &&
               TargetLoop->getUpperDDRef()->isSelfBlob());
      }

      const RegDDRef *AdjustingRef =
          InnermostLoopToAdjustingRef.at(InnermostLoop);

      HLLoop *OutermostSpatialLoop =
          SpatialLoops.size() > 0 ? SpatialLoops.back() : InnermostLoop;

      for (auto DimNum : ConstOrBlobDimNums) {

        assert(ByStripLoops.size() >= DimNum);

        const HLLoop *ByStripLoop = ByStripLoops[DimNum - 1];
        CanonExpr *CE = AdjustingRef->getDimensionIndex(DimNum)->clone();

        int64_t Shift = 0;
        if (!InnermostLoopToShift[DimNum - 1].empty()) {
          Shift = InnermostLoopToShift[DimNum - 1][InnermostLoopID];
        }

        if (DimInfos[DimNum - 1].isConstant()) {
          RegDDRef *Ref = AdjustingRef->getDDRefUtils().createConstDDRef(
              ByStripLoop->getIVType(), CE->getConstant());

          // TODO: OutermostSpatialLoop is not always correct.
          //       Hoisting might not be valid.
          //       SpatialLoops contains only the blocked loops.
          addIfGuards(Ref, ByStripLoop, OutermostSpatialLoop, Shift);

        } else {
          // blob or blob + constant
          RegDDRef *Ref = AdjustingRef->getDDRefUtils().createScalarRegDDRef(
              GenericRvalSymbase, CE->clone());

          // Look for a ref in the innermost
          // TODO: Consider return a AuxRef, which is a copy of CE.
          //       We only need CE, not whole AuxRef.
          std::pair<const RegDDRef *, unsigned> AuxRef =
              findAuxRefWithCE(InnermostLoop, CE);

          addIfGuards(Ref, ByStripLoop, OutermostSpatialLoop, Shift,
                      AuxRef.first);

          // Add AuxRef's BlobDDRef's symbase to LiveInTemp of ByStripLoop
          // Using getTempBlobSymbase from CE->getSingleBlobIndex() does not
          // work. For example, CE, sext.i32.i64(%2677) + 1, temp blob symbase
          // could be InvalidSymbase.
          // unsigned AuxTempSymbase =
          // AuxRef->getBlobUtils().getTempBlobSymbase(
          //  CE->getSingleBlobIndex());
          // Can use CE->collectTempBlobIndices(.), but we can approximate with
          // blob ddrefs of AuxRef.
          SmallVector<unsigned, 4> AuxTempSymbases;
          for (auto *BRef : make_range(AuxRef.first->blob_begin(),
                                       AuxRef.first->blob_end())) {
            AuxTempSymbases.push_back(BRef->getSymbase());
          }

          llvm::for_each(ByStripLoops, [AuxTempSymbases](HLLoop *Lp) {
            if (!Lp)
              return;

            Lp->addLiveInTemp(AuxTempSymbases);
          });
        }
      }
    }
  }

  static bool isNoBlockDim(unsigned DimNum, ArrayRef<unsigned> StripmineSizes) {
    return (DimNum > StripmineSizes.size()) || StripmineSizes[DimNum - 1] == 0;
  }

  // For a dimension, where blocking is not done for the corresponding loop,
  // stripmine size is set to zero.
  bool isNoBlockDim(unsigned DimNum) const {
    return isNoBlockDim(DimNum, StripmineSizes);
  }

  static void removeDupCanonExprs(SmallVectorImpl<CanonExpr *> &CEs) {
    std::sort(CEs.begin(), CEs.end(),
              [](const CanonExpr *R1, const CanonExpr *R2) {
                return CanonExprUtils::compare(R1, R2);
              });

    auto Last = std::unique(CEs.begin(), CEs.end(),
                            [](const CanonExpr *R1, const CanonExpr *R2) {
                              return CanonExprUtils::areEqual(R1, R2);
                            });

    CEs.erase(Last, CEs.end());
  }

  void calcLoopMatchingDimNum() {
    for (auto Pair : InnermostLoopToDimInfos) {
      int NumDims = Pair.second.size();
      Innermost2TargetLoop[Pair.first].resize(NumDims);

      for (int I = 1; I <= NumDims; I++)
        calcLoopMatchingDimNum(I, Pair.second, Pair.first);
    }
  }

  // Return the loop matching DimNum.
  // InnermostLoop and DimInfos are data to consult with.
  void calcLoopMatchingDimNum(unsigned DimNum, ArrayRef<DimInfoTy> DimInfos,
                              const HLLoop *InnermostLoop) {

    // Dimension for DimNum doesn't exist. This can happen
    // when there are arrays with different dimenseions.
    // e.g. A[][], B[][][][][] - A only has upto DimNum = 2,
    // while B has upto DimNum = 5.
    if (DimNum > DimInfos.size()) {
      return;
    }

    // Subscript is either constant or blobs.
    if (!DimInfos[DimNum - 1].hasIV()) {
      Innermost2TargetLoop[InnermostLoop][DimNum - 1] = nullptr;
      return;
    }

    const HLLoop *TargetLoop = InnermostLoop;
    for (int I = 0; I < DimInfos[DimNum - 1].LevelOffset; I++) {
      TargetLoop = TargetLoop->getParentLoop();
    }

    Innermost2TargetLoop[InnermostLoop][DimNum - 1] = TargetLoop;
  }

  // Return the loop matching DimNum.
  // InnermostLoop and DimInfos are data to consult with.
  const HLLoop *getLoopMatchingDimNum(unsigned DimNum, unsigned DimInfoSize,
                                      const HLLoop *InnermostLoop) const {

    // Dimension for DimNum doesn't exist. This can happen
    // when there are arrays with different dimenseions.
    // e.g. A[][], B[][][][][] - A only has upto DimNum = 2,
    // while B has upto DimNum = 5.
    if (DimNum > DimInfoSize) {
      return nullptr;
    }

    return Innermost2TargetLoop.at(InnermostLoop)[DimNum - 1];
  }

  HLLoop *getLoopMatchingDimNum(unsigned DimNum, unsigned DimInfoSize,
                                HLLoop *InnermostLoop) {

    return const_cast<HLLoop *>(
        static_cast<const Transformer &>(*this).getLoopMatchingDimNum(
            DimNum, DimInfoSize, const_cast<HLLoop *>(InnermostLoop)));
  }

  // Sweep through all rvals by calling makeConsistent().
  // Added to take care of operands of createMin/Max
  // createMin/Max generates extra operands than passed arguments.
  // Temp blobs defAtLevels are all up to date already.
  void MakeConsistentRvals(HLInst *HInst) const {
    for (auto *Ref :
         make_range(HInst->op_ddref_begin(), HInst->op_ddref_end())) {
      Ref->makeConsistent({});
    }
  }

  // Add "if (ByStripIV <= Ref <= TileEndRef)"
  // Ref: dimension index, either constant or a blob.
  // ByStripIV: tile begin,
  // TileEndRef : last element of the tile.
  void addIfGuards(RegDDRef *Ref, const HLLoop *ByStripLoop,
                   HLNode *NodeToEnclose, int64_t Shift,
                   const RegDDRef *AuxRef = nullptr) const {
    RegDDRef *ByStripIV = ByStripLoop->getDDRefUtils().createConstDDRef(
        ByStripLoop->getIVType(), 0);
    ByStripIV->getSingleCanonExpr()->addIV(ByStripLoop->getNestingLevel(),
                                           InvalidBlobIndex, 1, true);

    if (Shift) {
      int64_t temp = 0 - Shift;
      ByStripIV->getSingleCanonExpr()->addConstant(temp, false);
    }

    RegDDRef *TileEndRef =
        cast<HLInst>(ByStripLoop->getFirstChild())->getLvalDDRef()->clone();

    RegDDRef *ShiftedTileEnd = TileEndRef;
    if (Shift) {
      HLNodeUtils &HNU = NodeToEnclose->getHLNodeUtils();
      HLInst *ShiftInst = HNU.createSub(
          TileEndRef,
          (TileEndRef->getDDRefUtils())
              .createConstDDRef(Type::getInt64Ty(HNU.getContext()), Shift));
      ShiftedTileEnd = ShiftInst->getLvalDDRef()->clone();
      HLNodeUtils::insertBefore(NodeToEnclose, ShiftInst);
      MakeConsistentRvals(ShiftInst);
    }

    HLIf *Guard = NodeToEnclose->getHLNodeUtils().createHLIf(
        PredicateTy::ICMP_SLE, ByStripIV, Ref->clone());
    Guard->addPredicate(PredicateTy::ICMP_SLE, Ref->clone(), ShiftedTileEnd);
    if (HLLoop *Loop = dyn_cast<HLLoop>(NodeToEnclose)) {
      Loop->extractPreheaderAndPostexit();
    }
    NodeToEnclose->getHLNodeUtils().insertBefore(NodeToEnclose, Guard);
    HLNodeUtils::moveAsFirstThenChild(Guard, NodeToEnclose);

    SmallVector<const RegDDRef *, 2> AuxRefs({ShiftedTileEnd});
    if (AuxRef)
      AuxRefs.push_back(AuxRef);

    // Consistency of if-conditions
    for (auto PI = Guard->pred_begin(), E = Guard->pred_end(); PI != E; PI++) {
      Guard->getLHSPredicateOperandDDRef(PI)->makeConsistent(AuxRefs);
      Guard->getRHSPredicateOperandDDRef(PI)->makeConsistent(AuxRefs);
    }
  }

  // Add guards to the original spatial loop
  // DO i = LB', UB'
  // -->
  // Do i = max(LB', by-strip-loop's IV), min(UB', min(by-strip-loop's IV + step
  // - 1, by-strip-loop's UB)) TileEnd =
  // min(by-strip-loop's IV + step - 1,
  // by-strip-loop's UB) is already available as the first child of the
  // corresponding by-strip loop.
  // TODO: consider make it a lambda to its caller. It is used only in that
  // context
  unsigned addLoopBoundsGuards(HLLoop *Loop, unsigned DimNum,
                               int64_t Shift) const {

    const HLLoop *ByStripLoop = ByStripLoops[DimNum - 1];

    RegDDRef *ByStripIV = ByStripLoop->getDDRefUtils().createConstDDRef(
        ByStripLoop->getIVType(), 0);
    ByStripIV->getSingleCanonExpr()->addIV(ByStripLoop->getNestingLevel(),
                                           InvalidBlobIndex, 1, true);

    if (Shift) {
      int64_t temp = 0 - Shift;
      ByStripIV->getSingleCanonExpr()->addConstant(temp, false);
    }

    HLNodeUtils &HNU = Loop->getHLNodeUtils();
    // By default, signed min/max
    RegDDRef *MaxOp1 = Loop->getLowerDDRef()->clone();
    HLInst *LBInst = HNU.createMax(MaxOp1, ByStripIV, nullptr, true, true,
                                   FastMathFlags(), "lb_max");
    HLNodeUtils::insertBefore(Loop, LBInst);
    MakeConsistentRvals(LBInst);

    // min(UB', min(by-strip-loop's IV + step - 1, by-strip-loop's UB))
    // min(UB', TileEnd)
    RegDDRef *TileEndRef =
        cast<HLInst>(ByStripLoop->getFirstChild())->getLvalDDRef()->clone();
    RegDDRef *ShiftedTileEnd = TileEndRef;

    if (Shift) {
      HLInst *ShiftInst = HNU.createSub(
          TileEndRef,
          (TileEndRef->getDDRefUtils())
              .createConstDDRef(Type::getInt64Ty(HNU.getContext()), Shift));
      ShiftedTileEnd = ShiftInst->getLvalDDRef()->clone();
      HLNodeUtils::insertBefore(Loop, ShiftInst);
      MakeConsistentRvals(ShiftInst);
    }

    RegDDRef *MinOp1 = Loop->getUpperDDRef()->clone();
    HLInst *UBInst = HNU.createMin(MinOp1, ShiftedTileEnd, nullptr, true, true,
                                   FastMathFlags(), "ub_min");
    HLNodeUtils::insertBefore(Loop, UBInst);
    MakeConsistentRvals(UBInst);

    Loop->setLowerDDRef(LBInst->getLvalDDRef()->clone());
    Loop->addLiveInTemp(LBInst->getLvalDDRef()->getSymbase());
    Loop->setUpperDDRef(UBInst->getLvalDDRef()->clone());
    Loop->addLiveInTemp(UBInst->getLvalDDRef()->getSymbase());

    Loop->getLowerDDRef()->makeConsistent({});
    Loop->getLowerDDRef()->getSingleCanonExpr()->setDefinedAtLevel(
        Loop->getNestingLevel() - 1);
    Loop->getUpperDDRef()->makeConsistent({});
    Loop->getUpperDDRef()->getSingleCanonExpr()->setDefinedAtLevel(
        Loop->getNestingLevel() - 1);

    return TileEndRef->getSymbase();
  }

  // Given a CE, get a corresponding blob.
  static std::pair<BlobTy, unsigned>
  getConstantOrSingleBlob(const CanonExpr *CE) {
    BlobUtils &BU = CE->getBlobUtils();

    int64_t Val;
    if (CE->isIntConstant(&Val)) {
      return {BU.createBlob(Val, CE->getDestType()), InvalidBlobIndex};
    } else {
      return {BU.getBlob(CE->getSingleBlobIndex()), CE->getSingleBlobIndex()};
    }
  }

  // Given an array of CEs, get a min(max) blob of all CEs.
  template <bool IsMin>
  static std::pair<BlobTy, unsigned>
  getGlobalMinMaxBlob(ArrayRef<CanonExpr *> Bounds) {
    assert(!Bounds.empty());

    // TODO: eventually something similar has to go into
    //       the legality checks. What is not so straightforward is that
    //       when legality is checked, adjusted Loop Bounds are not
    //       available.
    for (auto *CE : make_range(Bounds.begin(), Bounds.end())) {
      if (!CE->isIntConstant() && !CE->convertToStandAloneBlobOrConstant()) {
        return {nullptr, InvalidBlobIndex};
      }
    }

    if (Bounds.size() == 1) {
      return getConstantOrSingleBlob(Bounds[0]);
    }

    std::pair<BlobTy, unsigned> Blob1 = getConstantOrSingleBlob(Bounds[0]);
    std::pair<BlobTy, unsigned> Blob2 = getConstantOrSingleBlob(Bounds[1]);
    bool BlobsAreConstant =
        isa<SCEVConstant>(Blob1.first) && isa<SCEVConstant>(Blob2.first);

    BlobUtils &BU = Bounds[0]->getBlobUtils();
    unsigned BlobIndex;
    BlobTy LowerBlob = IsMin ? BU.createSMinBlob(Blob1.first, Blob2.first,
                                                 !BlobsAreConstant, &BlobIndex)
                             : BU.createSMaxBlob(Blob1.first, Blob2.first,
                                                 !BlobsAreConstant, &BlobIndex);

    for (auto *CE : make_range(std::next(Bounds.begin(), 2), Bounds.end())) {

      std::pair<BlobTy, unsigned> Blob = getConstantOrSingleBlob(CE);
      bool BlobsAreConstant =
          isa<SCEVConstant>(LowerBlob) && isa<SCEVConstant>(Blob.first);
      LowerBlob = IsMin ? BU.createSMinBlob(LowerBlob, Blob.first,
                                            !BlobsAreConstant, &BlobIndex)
                        : BU.createSMaxBlob(LowerBlob, Blob.first,
                                            !BlobsAreConstant, &BlobIndex);
    }

    return {LowerBlob, BlobIndex};
  }

  // Given a ByStrip Loop, calculate the UB of the inner unit-strided loop
  //    min (IV + step - 1, UB)
  // where IV, step, and UB are induction var, loop step, and upperbound of
  // the ByStrip loop.
  // TODO: A candidate for a lambda function
  HLInst *createTileEnd(HLLoop *ByStrip) const {
    int64_t LoopStrideVal;
    if (!ByStrip->getStrideCanonExpr()->isIntConstant(&LoopStrideVal)) {
      llvm_unreachable("Only const stripmine sizes are supported!");
    }

    RegDDRef *MinOpr1 = ByStrip->getDDRefUtils().createConstDDRef(
        ByStrip->getIVType(), LoopStrideVal - 1);
    MinOpr1->getSingleCanonExpr()->addIV(ByStrip->getNestingLevel(),
                                         InvalidBlobIndex, 1, true);
    MinOpr1->makeConsistent({}, ByStrip->getNestingLevel());

    // createMin is by-default Smin
    HLInst *TileEnd = ByStrip->getHLNodeUtils().createMin(
        MinOpr1, ByStrip->getUpperDDRef()->clone(), nullptr, true, true,
        FastMathFlags(), "tile_e_min");

    return TileEnd;
  }

  // Notice that ByStripLoops are not normalized.
  // Only the children loops of ByStripLoops are normalized.
  void normalizeSpatialLoops() {

    std::unordered_set<HLLoop *> ProcessedTargetLoops;
    for (auto &LoopAndDimInfo : InnermostLoopToDimInfos) {

      SmallVector<unsigned, 8> LiveInsFromNormalization;

      // SL will be all spatial loops and/or by strip loops.
      for (const HLLoop *Loop :
           make_range(Innermost2TargetLoop[LoopAndDimInfo.first].rbegin(),
                      Innermost2TargetLoop[LoopAndDimInfo.first].rend())) {
        if (!Loop)
          continue;

        HLLoop *SL = const_cast<HLLoop *>(Loop);
        if (ProcessedTargetLoops.count(SL))
          continue;
        else
          ProcessedTargetLoops.insert(SL);

        SL->normalize();
        SL->getLowerDDRef()->populateTempBlobSymbases(LiveInsFromNormalization);
        SL->getUpperDDRef()->populateTempBlobSymbases(LiveInsFromNormalization);
        SL->addLiveInTemp(LiveInsFromNormalization);

        // MarkNotBlock to inhibit regular loop blocking pass coming later
        SL->markDoNotBlock();
      }
    }
  }

private:
  // In the order of DimNum, [DimNum = 1][DimNum = 2] .. and so on.
  ArrayRef<unsigned> StripmineSizes;

  const LoopToDimInfoTy &InnermostLoopToDimInfos;
  const LoopToConstRefTy &InnermostLoopToRepRef;
  const InnermostLoopToShiftTy &InnermostLoopToShift;
  // Loop enclosing all the spatial loopnests.
  // Inside OutermostLoop, by-strip loops are generated.
  HLLoop *OutermostLoop;
  HLIf *OuterIf;

  SmallVector<std::pair<BlobTy, unsigned>, 4> ByStripLoopLowerBlobs;
  SmallVector<std::pair<BlobTy, unsigned>, 4> ByStripLoopUpperBlobs;

  // Newly generated by-strip loops
  SmallVector<HLLoop *, 4> ByStripLoops;
  SmallVector<unsigned, 4> LiveInsOfAllSpatialLoop;

  // Number of ByStrip loops. Could be different from StripmineSizes.size()
  // because StripmineSizes can contain zeros.
  unsigned NumByStripLoops;

  StringRef Func;

  // A map from an innermost loop to its outer enclosing loops
  // matching to dimnum (includes the innermost loop).
  std::unordered_map<const HLLoop *, SmallVector<const HLLoop *, 4>>
      Innermost2TargetLoop;
};

// Collects a candidate set of spatial loops by
// applying profitablity and legality checks.
// Visitor is employed to collect a candidate sequence through
// profitability and legaltiy criteria.
// When an undesirable loop is encountered, the collection process stops.
// Tansformation will be tried with good candidates so far.
//
// Notice that it needs to do profitablity check another round.
// After the first round of profirtablity checker, optVarPred util is called.
// The second round of profitablity check is applied to the new HL structure
// resulted from optVarPred. The HL structure is new object (i.e. new pointer)
// thus previous information, such as innermost loop, is no longer relevant.
// If only legality checks are applied, and profitablity checks are not,
// the visitor may pick up a sequence, [FirstSpatialLoop, LastSpatialLoop],
// which is legal but not profitable.
class ProfitablityAndLegalityChecker : public ProfitabilityChecker {
  typedef SmallVector<HLLoop *, 8> LoopSetTy;

public:
  ProfitablityAndLegalityChecker(HIRFramework &HIRF,
                                 HIRArraySectionAnalysis &HASA,
                                 HIRDDAnalysis &DDA, HLLoop *OutermostLoop,
                                 StringRef FuncName)
      : ProfitabilityChecker(HIRF, HASA, DDA, FuncName),
        OutermostLoop(OutermostLoop), FoundGoodCand(false) {}

  bool run() {
    HLNodeUtils::visit(*this, OutermostLoop);

    return FoundGoodCand;
  }

  LoopToDimInfoTy getInnermostLoopToDimInfos() {
    return InnermostLoopToDimInfos;
  }

  LoopToConstRefTy getInnermostLoopToRepRef() { return InnermostLoopToRepRef; }

public:
  void visit(HLLoop *Loop) {
    markVisited(Loop);

    if (!checkStructure(Loop)) {
      return;
    }

    // Do profitablity check by adding the current loop.
    if (!analyzeProfitablity(Loop)) {
      // Profitablitly check failed with the current loop.
      // See if the candidates so far without current loop
      // look good.
      stopAndWork(10, Loop);
      SkipNode = Loop;
      return;
    }

    // Do legality check by adding the current loop.
    analyzeLegality(Loop);

    // Adding the current loop to the exisiting candidates
    // were both profitable and legal.
    // The candidate set was extended by dding the current loop
    // at this point.

    // Innermost loop's body is already scanned by InnermostLoopAnalyzer.
    SkipNode = Loop;
  }

  void visit(HLIf *HIf) { CheckerVisitor::visit(HIf); }
  void visit(HLInst *HInst) { CheckerVisitor::visit(HInst); }
  void visit(HLNode *Node) { CheckerVisitor::visit(Node); }

private:
  bool analyzeLegality(HLLoop *Loop) {

    // Update DDG to be used for the legality checker.
    DDGraph DDG;
    HLLoop *LCA = nullptr;
    if (!FirstSpatialLoop) {
      DDG = DDA.getGraph(Loop);
    } else {

      LCA = HLNodeUtils::getLowestCommonAncestorLoop(FirstSpatialLoop, Loop);

      if (!LCA) {
        SkipNode = Loop;
        reset();
        return false;
      }

      DDG = DDA.getGraph(LCA);
    }

    SmallVector<DimInfoTy, 4> DimInfos;
    InnermostLoopAnalyzer IA(Loop, OutermostLoop->getNestingLevel(), DimInfos,
                             BaseIndexToLowersAndStrides, Func);

    const RegDDRef *RepRef =
        IA.couldBeAMember(DefinedBasePtr, ReadOnlyBasePtr, DDG, LCA);

    if (!RepRef) {
      printMarker("Fail: ", {Loop});

      // Legality check failed with the current loop.
      // The current loop cannot make a candidate, so stop here.
      // See if the candidates so far without the current loop
      // look good.
      stopAndWork(20, Loop);

      SkipNode = Loop;
      return false;
    }

    // update structural information.
    LastSpatialLoop = Loop;
    if (!FirstSpatialLoop) {
      FirstSpatialLoop = Loop;
    } else {
      // TODO: this check might not needed to be in this else-part.
      //       Explore the further clean-up after transformation is enabled.
      if (PrevLCA && PrevLCA != LCA) {
        // Fail and no transformation
        // In theory, [FirstSpatialLoop, previous LastSpatialLoop] could
        // be a valid candidate. But, we don't care about that, but just
        // stop here.
        bailOut();
        return false;
      } else {
        PrevLCA = LCA;
      }
    }

    // This innermost loop is conforming.
    InnermostLoopToDimInfos.emplace_back(Loop, DimInfos);
    InnermostLoopToRepRef[Loop] = RepRef;

    printMarker("Passed: ", {Loop});
    printMarker("RepRef before move: ", {RepRef});
    printMarker("RepRef after move: ", {InnermostLoopToRepRef[Loop]});

    return true;
  }

  void reset() override {

    ProfitabilityChecker::reset();

    // TODO: Also, DefinedBasePtrOutsideSpatialLp need to track HLInst, HLIf
    //       outside spatial loops.
    DefinedBasePtr.clear();
    ReadOnlyBasePtr.clear();
    BaseIndexToLowersAndStrides.clear();
    InnermostLoopToDimInfos.clear();
    InnermostLoopToRepRef.clear();
    OutermostLoop = nullptr;
    FoundGoodCand = false;
  }

  // Returns true if [FirstSpatialLoop, LastSpatialLoop] can make
  // a good (i.e. legal and profitable) candidate to apply a inter-loop
  // blocking.
  bool stopAndWork(int CallSiteLoc, const HLLoop *StopLoop = nullptr) override {

    if (!postCheck(StopLoop)) {

      LLVM_DEBUG(dbgs() << "Fail @ postLigalityCheck\n");

      bailOut();

      FoundGoodCand = false;
      return false;
    }

    // Passed postCheck(), thus a legit candidate.
    printMarker("Transformation will be done on\n FirstSpatialLoop: ",
                {FirstSpatialLoop});
    printMarker("LastSpatialLoop: ", {LastSpatialLoop});
    printMarker("Input Loop: ", {PrevLCA});
    LLVM_DEBUG(PrevLCA->dump());
    LLVM_DEBUG_PROFIT_REPORT(dbgs() << "Legal\n");

    setDone(true);

    FoundGoodCand = true;
    return true;
  }

  bool verifyTopSortOrderOfInnermostLoops() const {
    unsigned PrevNum = 0;
    for (auto &Pair : InnermostLoopToDimInfos) {
      unsigned Num = Pair.first->getTopSortNum();
      if (PrevNum >= Num) {
        return false;
      }
      PrevNum = Num;
    }
    return true;
  }

  // Contains the checks that can be done after
  // traversals through the current Loop. (i.e. StopLoop)
  // Contains both legality and profitablity checks.
  bool postCheck(const HLLoop *StopLoop) const {

    // We don't work on a single spatial loopnest.
    if (!FirstSpatialLoop)
      return false;

    if (FirstSpatialLoop == LastSpatialLoop) {
      LLVM_DEBUG(dbgs() << "Fail 1: " << FirstSpatialLoop->getNumber() << "\n");
      return false;
    }

    // State in term of profitablity, should be SECONDHALF.
    // Otherwise, it is not profitable
    if (State != SECONDHALF) {
      LLVM_DEBUG(dbgs() << "Fail 2 state: " << State << "\n");
      return false;
    }

    // I/O calls are supposedly removed by OptVarPred by now.
    if (hasIOCall()) {
      LLVM_DEBUG(dbgs() << "Fail 3 : \n");
      return false;
    }

    // Structural check.
    if (!isCleanCut(LastSpatialLoop, StopLoop))
      return false;

    assert(verifyTopSortOrderOfInnermostLoops() &&
           "InnermostLoops are not in Order.");

    // Legality check: check if any of ReadOnlyBasePtr is
    // in DefinedBasePtr.
    if (llvm::any_of(ReadOnlyBasePtr, [this](unsigned BasePtrIndex) {
          return this->DefinedBasePtr.count(BasePtrIndex);
        })) {
      LLVM_DEBUG(dbgs() << "Fail 4 : \n");
      return false;
    }

    return true;
  }

private:
  // OutermostLoop should be the same with PrevLCP.
  HLLoop *OutermostLoop;
  bool FoundGoodCand;

  // BasePtrIndicies of defined(written) and read-only memrefs.
  // Captured as states over multiple loop nests (so multiple innermost loops).
  // Used in legality checks.
  BasePtrIndexSetTy DefinedBasePtr;
  BasePtrIndexSetTy ReadOnlyBasePtr;
  BaseIndexToLowersAndStridesTy BaseIndexToLowersAndStrides;

  LoopToDimInfoTy InnermostLoopToDimInfos;
  LoopToConstRefTy InnermostLoopToRepRef;
};

bool isOptVarPredNeeded(const ProfitabilityChecker &PC) {
  return PC.hasIOCall();
}

bool doTransformation(const LoopToDimInfoTy &InnermostLoopToDimInfos,
                      const LoopToConstRefTy &InnermostLoopToRepRef,
                      const InnermostLoopToShiftTy &InnermostLoopToShift,
                      HLLoop *OutermostLoop, HLIf *OuterIf, HIRDDAnalysis &DDA,
                      StringRef Func, bool UseKnownGoodSizes) {

  if (DisableTransform) {
    LLVM_DEBUG(dbgs() << "Transformation is disabled.\n");

    return false;
  }

  if (!RewriteFilterFunc.empty() && !Func.equals(RewriteFilterFunc)) {
    LLVM_DEBUG(dbgs() << "Transformation is disabled for function " << Func
                      << "\n");

    return false;
  }

  // Magic numbers.
  unsigned Size = InnermostLoopToDimInfos.begin()->second.size();
  SmallVector<unsigned, 4> PreSetStripmineSizes(Size, DefaultStripmineSize);
  if (UseKnownGoodSizes) {
    for (unsigned I = 0; I < Size; I++) {
      switch (I) {
      case 0:
        PreSetStripmineSizes[I] = 0;
        break;
      case 1:
        PreSetStripmineSizes[I] = 1;
        break;
      case 2:
        PreSetStripmineSizes[I] = 8;
        break;
      default:
        PreSetStripmineSizes[I] = DefaultStripmineSize;
        break;
      }
    }
  }

  return Transformer(PreSetStripmineSizes, InnermostLoopToDimInfos,
                     InnermostLoopToRepRef, InnermostLoopToShift, OutermostLoop,
                     OuterIf, DDA, Func)
      .rewrite();
}

template <bool isNeg = false>
static int64_t getMaxUseDist(ArrayRef<const RegDDRef *> Rvals,
                             const RegDDRef *RepRef, unsigned DimIndex) {
  const CanonExpr *RepCE = RepRef->getDimensionIndex(DimIndex);
  int64_t Kmax = 0;
  for (auto *Rval : Rvals) {

    // This can happen for some local rvals (e.g.
    // (@upml_mod_mp_bx_ilow_)[0].0$2)
    if (DimIndex > Rval->getNumDimensions())
      continue;

    const CanonExpr *CE = Rval->getDimensionIndex(DimIndex);
    int64_t Dist = -1;
    if (isNeg)
      CanonExprUtils::getConstDistance(RepCE, CE, &Dist);
    else
      CanonExprUtils::getConstDistance(CE, RepCE, &Dist);
    Kmax = std::max(Kmax, Dist);
  }

  assert(Kmax >= 0);
  return Kmax;
}

static void
calcShiftAmountBeforeDef(const LoopToDimInfoTy &InnermostLoopToDimInfo,
                         const LoopToConstRefTy &InnermostLoopToRepRef,
                         unsigned DimIndex,
                         SmallVectorImpl<int64_t> &ShiftAmt) {

  std::unordered_map<unsigned, int> BaseToLastDefLoopID;

  unsigned NumLoops = InnermostLoopToDimInfo.size();
  assert(ShiftAmt.size() == NumLoops);

  // Base to Kmax(A[i - k] of use, k > 0) for each LoopID
  std::unordered_map<unsigned, SmallVector<int64_t, 16>> MaxNegUseDist;

  // TODO: merge the loop body with the other function
  //       to do gathering and grouping of refs once per loop.
  for (auto &V : enumerate(InnermostLoopToDimInfo)) {
    const HLLoop *Loop = V.value().first;
    int LoopID = V.index();
    const RegDDRef *RepRef = InnermostLoopToRepRef.at(Loop);

    MemRefGatherer::VectorTy Refs;
    MemRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(), Refs);
    RefGroupVecTy Groups;
    DDRefIndexGrouping(Groups, Refs);

    for (auto Group : Groups) { // for each base index
      unsigned BaseIndex = Group.front()->getBasePtrBlobIndex();
      auto &Vec = MaxNegUseDist[BaseIndex];
      if (Vec.size() == 0) {
        Vec.resize(NumLoops);
      }

      bool IsLvalFound = false;
      SmallVector<const RegDDRef *, 16> Rvals;

      for (auto *Ref : Group) {
        if (Ref->isLval()) {
          IsLvalFound = true;
        } else {
          Rvals.push_back(Ref);
        }
      } // Refs

      int64_t KMaxInCurLoop = getMaxUseDist<true>(Rvals, RepRef, DimIndex);
      if (!IsLvalFound) {
        // Record Negative Use Kmax
        MaxNegUseDist[BaseIndex][LoopID] = KMaxInCurLoop;
        continue;
      }

      auto LI = BaseToLastDefLoopID.find(BaseIndex);

      int DefLoopID = LI == BaseToLastDefLoopID.end() ? -1 : LI->second;
      int64_t KNegUseMax = 0;
      int64_t ShiftAmtSoFar = 0;
      for (int I = DefLoopID + 1; I < LoopID; I++) {
        KNegUseMax = std::max(MaxNegUseDist[BaseIndex][I], KNegUseMax);
        ShiftAmtSoFar += ShiftAmt[I];
      }
      ShiftAmtSoFar += ShiftAmt[LoopID];

      // Meaning:
      // ShiftAmt[LoopID] = ShiftAmt[LoopID] + std::max((int64_t)0, KNegUseMax -
      // ShiftAmtSoFar);
      if (ShiftAmtSoFar < KNegUseMax) {
        ShiftAmt[LoopID] += (KNegUseMax - ShiftAmtSoFar);

        LLVM_DEBUG(dbgs() << "++ LoopID: " << Loop->getNumber() << "\n");
        LLVM_DEBUG(dbgs() << "++++Group front: ");
        LLVM_DEBUG(Group.front()->dump(); dbgs() << "\n");
      }

      // This loop has a def
      // BaseToLastDefLoopID.emplace(BaseIndex, LoopID);
      BaseToLastDefLoopID[BaseIndex] = LoopID;

      LLVM_DEBUG(dbgs() << "+Def update: " << Loop->getNumber() << " ");
      LLVM_DEBUG(Group.front()->dump(); dbgs() << "\n");

      MaxNegUseDist[BaseIndex][LoopID] = KMaxInCurLoop;
    } // BaseIndex
  }   // Loop
}

static void
calcShiftAmountBeforeUse(const LoopToDimInfoTy &InnermostLoopToDimInfo,
                         const LoopToConstRefTy &InnermostLoopToRepRef,
                         unsigned DimIndex,
                         SmallVectorImpl<int64_t> &ShiftAmt) {

  unsigned NumLoops = InnermostLoopToDimInfo.size();
  // TODO: do resize outside in the caller.
  ShiftAmt.resize(NumLoops);
  std::unordered_map<unsigned, int> BaseToLastDefLoopID;

  for (auto V : enumerate(InnermostLoopToDimInfo)) {
    const HLLoop *Loop = V.value().first;
    int LoopID = V.index();
    const RegDDRef *RepRef = InnermostLoopToRepRef.at(Loop);

    MemRefGatherer::VectorTy Refs;
    MemRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(), Refs);
    RefGroupVecTy Groups;
    DDRefIndexGrouping(Groups, Refs);

    for (auto Group : Groups) { // for each base index
      unsigned BaseIndex = Group.front()->getBasePtrBlobIndex();

      bool IsLvalFound = false;
      SmallVector<const RegDDRef *, 16> Rvals;

      for (auto *Ref : Group) {
        if (Ref->isLval()) {
          IsLvalFound = true;
        } else {
          Rvals.push_back(Ref);
        }
      } // Refs

      auto LI = BaseToLastDefLoopID.find(BaseIndex);
      if (LI != BaseToLastDefLoopID.end()) {

        // Find Kmax by Ref - RepDefRef
        // Do it for every DimIndex - for now for a given DimIndex
        // To calc for all DimNums, Kmax should be scalar expanded by DimNums
        int64_t Kmax = getMaxUseDist(Rvals, RepRef, DimIndex);

        // No Rval in the form of A[i + k], k > 0 if Kmax <= 0.
        if (Kmax > 0) {

          // Meaning
          // int64_t ShiftAmtSoFar =
          //   std::accumulate(std::next(ShiftAmt.begin(),
          //   BaseToLastDefLoopID[BaseIndex] + 1),
          //                   std::next(ShiftAmt.begin(), LoopID), 0);
          int64_t ShiftAmtSoFar = 0;
          for (int J = BaseToLastDefLoopID[BaseIndex] + 1; J <= LoopID; J++)
            ShiftAmtSoFar += ShiftAmt[J];

          // Meaning
          // ShiftAmt[LoopID] = ShiftAmt[LoopID] + std::max((int64_t) 0, (Kmax -
          // ShiftAmtSoFar));
          if (ShiftAmtSoFar < Kmax) {

            LLVM_DEBUG(dbgs() << "-- LoopID: " << Loop->getNumber() << "\n");
            LLVM_DEBUG(dbgs() << "-----Group front: ");
            LLVM_DEBUG(Group.front()->dump(); dbgs() << "\n");
            LLVM_DEBUG(dbgs() << "-----Base Index: " << BaseIndex << "\n");

            ShiftAmt[LoopID] += (Kmax - ShiftAmtSoFar);
          }
        }
      }

      if (IsLvalFound) {
        BaseToLastDefLoopID[BaseIndex] = LoopID;

        LLVM_DEBUG(dbgs() << "-Def update: " << Loop->getNumber() << " ");
        LLVM_DEBUG(Group.front()->dump(); dbgs() << "\n");
      }

    } // Group - BaseIndex
  }   // Loop
}

void testCalcShiftAmtFuncs(const LoopToDimInfoTy &InnermostLoopToDimInfo,
                           const LoopToConstRefTy &InnermostLoopToRepRef,
                           unsigned DimNum,
                           SmallVectorImpl<int64_t> &ShiftAmt) {

  unsigned NumLoops = InnermostLoopToDimInfo.size();
  ShiftAmt.resize(NumLoops, 0);

  calcShiftAmountBeforeUse(InnermostLoopToDimInfo, InnermostLoopToRepRef,
                           DimNum, ShiftAmt);

  calcShiftAmountBeforeDef(InnermostLoopToDimInfo, InnermostLoopToRepRef,
                           DimNum, ShiftAmt);

  // Print ShiftAmt
  LLVM_DEBUG(dbgs() << "ShiftAmts for Loops\n";
             bool IsNonZeroAmtFound = false; for (auto V
                                                  : enumerate(ShiftAmt)) {
               int I = V.index();
               int64_t Amt = V.value();
               const HLLoop *Loop = InnermostLoopToDimInfo.at(I).first;

               if (Amt) {
                 IsNonZeroAmtFound = true;
                 dbgs() << Loop->getNumber() << ": " << Amt << "\n";
               }
             } dbgs() << "IsNonZeroAmtFound: "
                      << IsNonZeroAmtFound << "\n";);
}

void calcShiftAmtFuncs(const LoopToDimInfoTy &InnermostLoopToDimInfo,
                       const LoopToConstRefTy &InnermostLoopToRepRef,
                       unsigned MinDimNums,
                       InnermostLoopToShiftTy &InnermostLoopToShiftVec) {

  InnermostLoopToShiftVec.resize(MinDimNums);

  for (unsigned DimNum = 1; DimNum <= MinDimNums; DimNum++) {
    SmallVector<int64_t, 32> ShiftAmt;
    testCalcShiftAmtFuncs(InnermostLoopToDimInfo, InnermostLoopToRepRef, DimNum,
                          ShiftAmt);

    int64_t SumShifts = 0;
    for (auto LI : enumerate(InnermostLoopToDimInfo)) {
      int LoopID = LI.index();
      SumShifts += ShiftAmt[LoopID];

      InnermostLoopToShiftVec[DimNum - 1].push_back(SumShifts);
    }

    // All members are zero, then clean-up shifts.
    if (!InnermostLoopToShiftVec[DimNum - 1].empty() &&
        InnermostLoopToShiftVec[DimNum - 1].back() == 0) {
      InnermostLoopToShiftVec[DimNum - 1].clear();
    }
  }

  LLVM_DEBUG(for (unsigned DimNum = 1; DimNum <= MinDimNums;
                  DimNum++) if (!InnermostLoopToShiftVec[DimNum - 1].empty()) {
    dbgs() << "DimNum: " << DimNum
           << " ShiftAmt: " << InnermostLoopToShiftVec[DimNum - 1].back()
           << "\n";
  });
}

bool funcFilter(const Function &F) {
  if (DisablePass) {
    return false;
  }

  // StringRef FuncName = F.getName();

  if (!FilterFunc.empty() && !F.getName().equals(FilterFunc)) {
    return false;
  }

  return true;
}

class testDriver {
public:
  testDriver(HIRFramework &HIRF, HIRArraySectionAnalysis &HASA,
             HIRDDAnalysis &DDA, TargetTransformInfo &TTI,
             HIRLoopStatistics &HLS, const Function &F)
      : HIRF(HIRF), HASA(HASA), DDA(DDA), HLS(HLS), Func(F),
        UseKnownGoodSizes(false), RelaxedCheck(true) {

    UseKnownGoodSizes = TTI.isAdvancedOptEnabled(
        TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2);
  }

  // TODO: return value does not have a lot of meaning.
  bool run();

private:
  // Only two loops and they are in the mutually exclusive paths
  static bool areTwoLoopsInExclusiveFlows(const HLLoop *Loop1,
                                          const HLLoop *Loop2);

  // In general, InnermostLoop is the innermost loops of a Perfect spatial
  // loopnest, with a few exceptions.
  static bool
  isInAlmostPerfectLoopNest(const HLLoop *InnermostLoop, unsigned Index,
                            const SmallVectorImpl<HLLoop *> &InnermostLoops,
                            const HLNode *OuterNode);

  static bool
  hasCommonAncestorThanReg(const SmallVectorImpl<HLLoop *> &InnermostLoops,
                           const HLRegion *Reg);

  static unsigned getCommonDimNum(const LoopToDimInfoTy &InnermostLoopToDimInfo,
                                  bool &AllCommonDimNum);

  bool isProfitableUseDefPattern(
      const SmallVectorImpl<HLLoop *> &InnermostLoops) const;
  void testInnermostLoops(SmallVectorImpl<HLLoop *> &InnermostLoops,
                          const HLRegion *Reg, const HLGoto *HGoto = nullptr,
                          const HLInst *HCall = nullptr) const;

  static void checkTargetsAndShrink(SmallVectorImpl<HLLoop *> &InnermostLoops,
                                    SmallVectorImpl<HLNode *> &OuterNodes,
                                    const HLGoto *HGoto);
  static bool isCallAtValidLoc(SmallVectorImpl<HLNode *> &OuterNodes,
                               const HLInst *HCall);

  SmallVector<HLNode *, 16> static calculateOuterNodes(
      const SmallVectorImpl<HLLoop *> &InnermostLoops, const HLNode *Limit);

private:
  HIRFramework &HIRF;
  HIRArraySectionAnalysis &HASA;
  HIRDDAnalysis &DDA;
  HIRLoopStatistics &HLS;

  const Function &Func;

  bool UseKnownGoodSizes;
  bool RelaxedCheck;
};

unsigned
testDriver::getCommonDimNum(const LoopToDimInfoTy &InnermostLoopToDimInfo,
                            bool &AllCommonDimNum) {
  unsigned Min = InnermostLoopToDimInfo.front().second.size();
  AllCommonDimNum = true;
  for (auto P : InnermostLoopToDimInfo) {
    if (Min > P.second.size()) {
      Min = P.second.size();
      AllCommonDimNum = false;
    }
  }

  return Min;
}

bool testDriver::isInAlmostPerfectLoopNest(
    const HLLoop *InnermostLoop, unsigned Index,
    const SmallVectorImpl<HLLoop *> &InnermostLoops, const HLNode *OuterNode) {

  const HLLoop *Lp = InnermostLoop;
  const HLNode *Parent = Lp->getParent();
  bool NonLoopParent = false;

  unsigned LowestSpatialLoopLevel = Lp->getNestingLevel();
  SmallVector<const HLInst *> InBetweenNodes;
  while (Parent && Parent != OuterNode) {
    if (auto *PLoop = dyn_cast<HLLoop>(Parent)) {
      if (NonLoopParent) {
        LLVM_DEBUG(dbgs() << "0 Lp: " << Lp->getNumber()
                          << " PLoop: " << PLoop->getNumber() << "\n");
        return false;
      }

      // If PLoop's first child is Lp, no further exceptions need to be checked.
      if (PLoop->getFirstChild() != Lp) {

        if (!Lp->isInnermost())
          return false;

        if (Index < 1)
          return false;

        const HLNode *PrevNode = Lp->getPrevNode();
        if (!PrevNode)
          return false;

        if (PrevNode != InnermostLoops[Index - 1]) {
          if (!isa<HLInst>(PrevNode) ||
              InnermostLoops[Index - 1]->getNextNode() != PrevNode) {
            return false;
          } else {
            InBetweenNodes.push_back(cast<HLInst>(PrevNode));
          }
        }
      }

      LowestSpatialLoopLevel = PLoop->getNestingLevel();
      Lp = PLoop;

    } else {
      LLVM_DEBUG(if (Lp) dbgs() << "2 Lp: " << Lp->getNumber();
                 if (PLoop) dbgs() << " PLoop: " << PLoop->getNumber() << "\n");

      NonLoopParent = true;
    }

    Parent = Parent->getParent();
  }

  // Scan InBetweenNodes
  // Example:
  // + DO i1 = 0, -1 * sext.i32.i64(%160) + sext.i32.i64(%190), 1   <DO_LOOP>
  // |   + DO i2 = 0, zext.i32.i64(((-1 * %130) + %140)), 1   <DO_LOOP>
  // |   |   %1705 = (%52)[i1 + sext.i32.i64(%160)][i2 + sext.i32.i64(%130)]...
  // |   |   %1744 = %1743  +  %1737;
  // |   |   ...
  // |   |   (%78)[i1 + sext.i32.i64(%160)][i2 + sext.i32.i64(%130)] = %1745;
  // |   + END LOOP
  // |
  // |   %1754 = (i1 + sext.i32.i64(%160) < %180) ? -1 : %1676; // In-between
  // |
  // |   + DO i2 = 0, zext.i32.i64(((-1 * %110) + %140)), 1   <DO_LOOP>
  // |   |   %1776 = (%513)[i1 + sext.i32.i64(%160)][i2 + sext.i32.i64(%110)]...
  // |   |   %1815 = %1776  *  %1814;
  // |   |   ...
  // |   |   (%77)[i1 + sext.i32.i64(%160)][i2 + sext.i32.i64(%110)] = %1815;
  // |   + END LOOP
  // + END LOOP
  for (auto *Node : InBetweenNodes) {
    for (auto *Ref :
         make_range(Node->rval_op_ddref_begin(), Node->rval_op_ddref_end())) {

      for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
        if (CE->isNonLinear() ||
            CE->getDefinedAtLevel() >= LowestSpatialLoopLevel) {

          LLVM_DEBUG(dbgs() << "Ref is not invariant at Level: "
                            << LowestSpatialLoopLevel << "\n";
                     Ref->dump(); CE->dump(); dbgs() << "\n");

          return false;
        }
      }
    }
  }

  return true;
}

bool testDriver::hasCommonAncestorThanReg(
    const SmallVectorImpl<HLLoop *> &InnermostLoops, const HLRegion *Reg) {

  HLNode *CommonAncestor = findTheLowestAncestor(InnermostLoops.front(), Reg);
  for (auto *Lp :
       make_range(std::next(InnermostLoops.begin()), InnermostLoops.end())) {
    HLNode *Ancestor = findTheLowestAncestor(Lp, Reg);
    if (CommonAncestor != Ancestor) {
      LLVM_DEBUG(dbgs() << "No common outer node other than region\n";
                 dbgs() << Lp->getNumber() << " ";
                 dbgs() << Ancestor->getNumber() << " "
                        << CommonAncestor->getNumber() << "\n");

      return false;
    }
  }
  return true;
}

bool testDriver::isProfitableUseDefPattern(
    const SmallVectorImpl<HLLoop *> &InnermostLoops) const {

  SmallVector<SparseBitVector<>, 64> InnermostLoopToDefs, InnermostLoopToUses;
  SparseBitVector<> BaseUniverse;

  std::map<unsigned, int> BaseToLoopCounts;
  for (auto *Lp : InnermostLoops) {

    SmallVector<RegDDRef *, 32> Refs;
    MemRefGatherer::gather(Lp, Refs);

    // Count BasePtr only once for a loop
    auto &ASAR = HASA.getOrCompute(Lp);
    SparseBitVector<> BasePtrsInLoop;
    std::for_each(Refs.begin(), Refs.end(), [&](const RegDDRef *Ref) {
      unsigned BasePtr = Ref->getBasePtrBlobIndex();
      auto *SectionInfo = ASAR.get(BasePtr);
      if (!SectionInfo)
        return;
      BasePtrsInLoop.set(BasePtr);
    });

    for (auto Base : BasePtrsInLoop) {
      int &Count = BaseToLoopCounts[Base];
      Count++;

      BaseUniverse.set(Base);
    }
  }

  int TotalBases = BaseUniverse.count();

  int NumHighReuse = 0;
  int NumMidReuse = 0;
  int Size = InnermostLoops.size();
  for (auto P : BaseToLoopCounts) {
    float percent = (((float)P.second) / ((float)(Size))) * 100.0;

    if (percent > 25)
      NumHighReuse++;
    else if (percent > 9)
      NumMidReuse++;
  }

  // #(percent >= 25%) > 18%     (in terms of num_bases / total_num_bases)
  // #(percent > 9% && percent < 25%) > 25%
  if ((NumHighReuse <= (TotalBases * 0.18)) &&
      (NumMidReuse <= (TotalBases * 0.25))) {
    LLVM_DEBUG_PROFIT_REPORT(dbgs() << "Not Profitable in testDriver\n");
    LLVM_DEBUG(dbgs() << "NumHighReuse: " << NumHighReuse << ", NumMidResuse: "
                      << NumMidReuse << ", TotalBases: " << TotalBases << "\n");
    return false;
  }

  return true;
}

void testDriver::checkTargetsAndShrink(
    SmallVectorImpl<HLLoop *> &InnermostLoops,
    SmallVectorImpl<HLNode *> &OuterNodes, const HLGoto *HGoto) {

  unsigned MaxNumber = OuterNodes.back()->getMaxTopSortNum();
  unsigned GotoNumber = HGoto->getTopSortNum();
  if (MaxNumber < GotoNumber) {
    return;
  }

  unsigned LabelNumber = HGoto->getTargetLabel()->getTopSortNum();
  if (MaxNumber >= LabelNumber)
    return;

  int64_t I = OuterNodes.size() - 1;
  for (; I >= 0; I--) {
    if (OuterNodes[I]->getMaxTopSortNum() < GotoNumber) {
      // InnermostLoops only upto this OuterNode are valid
      // All innermost loops belong to OuterNode after this OuterNode
      // should be removed.
      break;
    }
  }

  // Remove InnermostLoops from I + 1 to end()
  InnermostLoops.erase(InnermostLoops.begin() + I + 1, InnermostLoops.end());
}

bool testDriver::isCallAtValidLoc(SmallVectorImpl<HLNode *> &OuterNodes,
                                  const HLInst *HCall) {
  unsigned MaxNumber = OuterNodes.back()->getMaxTopSortNum();
  unsigned CallNumber = HCall->getTopSortNum();
  return MaxNumber < CallNumber;
}

SmallVector<HLNode *, 16>
testDriver::calculateOuterNodes(const SmallVectorImpl<HLLoop *> &InnermostLoops,
                                const HLNode *Limit) {
  SmallVector<HLNode *, 16> OuterNodes;
  for (auto InnermostLoop : InnermostLoops) {
    HLNode *OuterNode = findTheLowestAncestor(InnermostLoop, Limit);
    OuterNodes.push_back(OuterNode);
  }

  return OuterNodes;
}

void testDriver::testInnermostLoops(SmallVectorImpl<HLLoop *> &InnermostLoops,
                                    const HLRegion *Reg, const HLGoto *HGoto,
                                    const HLInst *HCallInst) const {

  // Bails out for a few trivial cases.
  if (InnermostLoops.size() < 2) {
    LLVM_DEBUG(dbgs() << "Empty LV -- skip\n");
    return;
  }

  HLNode *OuterNode = findTheLowestAncestor(InnermostLoops.front(), Reg);
  const HLIf *OutermostIf = dyn_cast<HLIf>(OuterNode);
  if (!ForceTestDriver &&
      (!OutermostIf || OutermostIf->getNumElseChildren() > 0))
    return;

  if (InnermostLoops.size() == 2 &&
      areTwoLoopsInExclusiveFlows(InnermostLoops[0], InnermostLoops[1]))
    return;

  unsigned Level = OuterNode->getNodeLevel();

  BasePtrIndexSetTy DefinedBasePtr;
  BasePtrIndexSetTy ReadOnlyBasePtr;
  BaseIndexToLowersAndStridesTy BaseIndexToLowersAndStrides;

  LoopToDimInfoTy InnermostLoopToDimInfo;
  LoopToConstRefTy InnermostLoopToRepRef;

  DDGraph DDG = DDA.getGraph(Reg);

  for (auto V : enumerate(InnermostLoops)) {
    if (!isInAlmostPerfectLoopNest(V.value(), V.index(), InnermostLoops,
                                   OuterNode)) {
      LLVM_DEBUG(dbgs() << "Fail isValidLoopNest: " << V.value()->getNumber()
                        << "\n");
      LLVM_DEBUG(Reg->dump(1));
      return;
    }
  }

  if (!hasCommonAncestorThanReg(InnermostLoops, Reg))
    return;

  SmallVector<HLNode *, 16> OuterNodes =
      calculateOuterNodes(InnermostLoops, OuterNode);

  if (HGoto)
    checkTargetsAndShrink(InnermostLoops, OuterNodes, HGoto);
  else if (HCallInst) {
    if (!isCallAtValidLoc(OuterNodes, HCallInst))
      return;
  }

  if (InnermostLoops.size() == 2 &&
      areTwoLoopsInExclusiveFlows(InnermostLoops[0], InnermostLoops[1]))
    return;

  if (!isProfitableUseDefPattern(InnermostLoops))
    return;

  // Legality check
  for (auto *Lp : InnermostLoops) {

    SmallVector<DimInfoTy, 4> DimInfos;
    InnermostLoopAnalyzer IA(Lp, Level, DimInfos, BaseIndexToLowersAndStrides,
                             Func.getName(), RelaxedCheck);

    const RegDDRef *RepRef =
        IA.couldBeAMember(DefinedBasePtr, ReadOnlyBasePtr, DDG, nullptr);

    if (!RepRef) {
      LLVM_DEBUG(dbgs() << "Fail Loop: " << Lp->getNumber() << "\n");
      return;
    }

    InnermostLoopToDimInfo.emplace_back(Lp, DimInfos);
    InnermostLoopToRepRef.emplace(Lp, RepRef);
  }

  bool AllCommonDimNum = true;
  unsigned MinDimNums =
      getCommonDimNum(InnermostLoopToDimInfo, AllCommonDimNum);

  LLVM_DEBUG(dbgs() << "MinDimNums: " << MinDimNums << "\n");
  LLVM_DEBUG(dbgs() << "AllCommonDimNum: " << AllCommonDimNum << "\n");
  (void)AllCommonDimNum;
  LLVM_DEBUG(for (unsigned I = 0; I < InnermostLoopToDimInfo.size(); I++) {
    const HLLoop *Lp = InnermostLoopToDimInfo[I].first;
    dbgs() << "Loop: " << InnermostLoopToDimInfo[I].first->getNumber() << " ";
    InnermostLoopToRepRef[Lp]->dump();
    for (auto J : InnermostLoopToDimInfo[I].second) {
      dbgs() << J << " ";
    }
    dbgs() << "\n";
  });

  InnermostLoopToShiftTy InnermostLoopToShiftVec;
  calcShiftAmtFuncs(InnermostLoopToDimInfo, InnermostLoopToRepRef, MinDimNums,
                    InnermostLoopToShiftVec);

  HLLoop *OutermostLoop = dyn_cast<HLLoop>(OuterNode);
  HLIf *OuterIf = dyn_cast<HLIf>(OuterNode);
  doTransformation(InnermostLoopToDimInfo, InnermostLoopToRepRef,
                   InnermostLoopToShiftVec, OutermostLoop, OuterIf, DDA,
                   Func.getName(), UseKnownGoodSizes);
}

bool testDriver::areTwoLoopsInExclusiveFlows(const HLLoop *Loop1,
                                             const HLLoop *Loop2) {
  auto *LCAParent =
      HLNodeUtils::getLexicalLowestCommonAncestorParent(Loop1, Loop2);

  // Check if nodes are in mutually exclusive paths of the parent if/switch.
  if (auto *If = dyn_cast<HLIf>(LCAParent)) {
    if (If->isThenChild(Loop1) != If->isThenChild(Loop2)) {
      return true;
    }

  } else if (auto *Switch = dyn_cast<HLSwitch>(LCAParent)) {

    if (Switch->getChildCaseNum(Loop1) != Switch->getChildCaseNum(Loop2)) {
      return true;
    }
  }
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// TODO: Make printMarker work
void printLoopVec(const SmallVectorImpl<HLLoop *> &LV) {
  for (auto *L : LV) {
    dbgs() << "Loop number: " << L->getNumber() << "\n";
  }
  dbgs() << "=======================\n";
}
#endif
bool testDriver::run() {

  if (!funcFilter(Func))
    return false;

  if (HIRF.hir_begin() == HIRF.hir_end())
    return false;

  SmallVector<HLLoop *, 4> InnermostLoops;
  HLRegion *Reg = cast<HLRegion>(&*HIRF.hir_begin());

  for (auto It = HLRangeIterator(Reg->child_begin()),
            EIt = HLRangeIterator(Reg->child_end());
       It != EIt; ++It) {

    if (HLLoop *Lp = dyn_cast<HLLoop>(*It)) {
      if (!Lp->isInnermost())
        continue;

      const LoopStatistics &LS = HLS.getSelfLoopStatistics(Lp);

      if (LS.hasCallsWithUnsafeSideEffects() || LS.hasForwardGotos()) {
        InnermostLoops.clear();
        continue;
      }

      InnermostLoops.push_back(Lp);
    } else if (HLGoto *HGoto = dyn_cast<HLGoto>(*It)) {

      printMarker("Met a goto ", {HGoto}, true);
      LLVM_DEBUG(dbgs() << "1. Innermost loops collected: ");
      LLVM_DEBUG(printLoopVec(InnermostLoops));

      testInnermostLoops(InnermostLoops, Reg, HGoto);
      InnermostLoops.clear();

    } else if (HLInst *HInst = dyn_cast<HLInst>(*It)) {
      if (HInst->isCallInst()) {

        printMarker("Met a CallInst ", {HInst}, true);
        LLVM_DEBUG(dbgs() << "2. Innermost loops collected: ");
        LLVM_DEBUG(printLoopVec(InnermostLoops));

        testInnermostLoops(InnermostLoops, Reg, nullptr, HInst);
        InnermostLoops.clear();
      }
    }
  }

  if (!InnermostLoops.empty())
    testInnermostLoops(InnermostLoops, Reg);

  return true;
}

bool tryTransform(HIRFramework &HIRF, HIRArraySectionAnalysis &HASA,
                  HIRDDAnalysis &DDA, StringRef FuncName, HLLoop *OutermostLoop,
                  bool UseKnownGoodSizes) {

  ProfitablityAndLegalityChecker Checker(HIRF, HASA, DDA, OutermostLoop,
                                         FuncName);

  bool FoundLegalAndProfitableCand = Checker.run();
  LLVM_DEBUG(dbgs() << "Found Legal & Profitable Cand:"
                    << FoundLegalAndProfitableCand << "\n");
  if (FoundLegalAndProfitableCand) {
    InnermostLoopToShiftTy InnermostLoopToShiftVec(3);
    return doTransformation(Checker.getInnermostLoopToDimInfos(),
                            Checker.getInnermostLoopToRepRef(),
                            InnermostLoopToShiftVec, Checker.getOutermostLoop(),
                            nullptr, DDA, FuncName, UseKnownGoodSizes);
  }

  return false;
}

// Main driver for HIRInterLoopBlocking.
bool driver(HIRFramework &HIRF, HIRArraySectionAnalysis &HASA,
            HIRDDAnalysis &DDA, TargetTransformInfo &TTI,
            HIRLoopStatistics &HLS, const Function &F) {
  if (!funcFilter(F)) {
    return false;
  }

  if (!F.isFortran()) {
    return false;
  }

  // Strictly for lit-tests
  if (ForceTestDriver)
    return testDriver(HIRF, HASA, DDA, TTI, HLS, F).run();

  StringRef FuncName = F.getName();

  ProfitabilityChecker PC(HIRF, HASA, DDA, FuncName);

  // Looks profitable if true. Actual profitablity
  // can be decided after optVarPred.
  bool CouldBeProfitable = PC.isProfitable();
  if (!CouldBeProfitable) {
    LLVM_DEBUG(dbgs() << "NOT Profitable at first try.\n");

    // Look for a second chance
    return testDriver(HIRF, HASA, DDA, TTI, HLS, F).run();
  }

  LLVM_DEBUG_PROFIT_REPORT(dbgs() << "Profitable\n");

  assert(PC.getOutermostLoop());

  LLVM_DEBUG(dbgs() << PC.getOutermostLoop()->getNumber() << "\n";);

  bool UseKnownGoodSizes = TTI.isAdvancedOptEnabled(
      TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2);

  if (!isOptVarPredNeeded(PC)) {
    // Needed to lit-test cases
    LLVM_DEBUG(PC.getOutermostLoop()->dump());

    bool Success = tryTransform(HIRF, HASA, DDA, FuncName,
                                PC.getOutermostLoop(), UseKnownGoodSizes);
    if (Success) {
      return true;
    }
  }

  SmallVector<HLLoop *, 3> OutLoops;
  SmallPtrSet<HLNode *, 3> NodesToInvalidate;
  bool OptPred = HIRTransformUtils::doOptVarPredicate(
      PC.getOutermostLoop(), OutLoops, NodesToInvalidate);

  LLVM_DEBUG(dbgs() << " OptPred: " << OptPred << "\n");
  LLVM_DEBUG(dbgs() << "OutLoops: " << OutLoops.size() << "\n";
             for (auto L
                  : OutLoops) {
               dbgs() << "L: \n";
               L->dump();
             });

  if (!OptPred) {
    // I/O but no pred, just bail-out
    return false;
  }

  // invalidate after OptVarPred, because up-to-date DDGs are needed.
  for (HLNode *Node : NodesToInvalidate) {
    if (HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
      assert(Loop->isAttached() && "Every invalidated loop should be attached");
      HIRInvalidationUtils::invalidateBody(Loop);
    } else {
      HIRInvalidationUtils::invalidateNonLoopRegion(cast<HLRegion>(Node));
    }
    HLNodeUtils::removeRedundantNodes(Node, false);
  }

  // Heuristic: try to work only on OutLoops.back()
  bool Success = tryTransform(HIRF, HASA, DDA, FuncName,
                              OutLoops[OutLoops.size() - 1], UseKnownGoodSizes);
  return Success;
}
} // namespace

PreservedAnalyses HIRInterLoopBlockingPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {

  driver(HIRF, AM.getResult<HIRArraySectionAnalysisPass>(F),
         AM.getResult<HIRDDAnalysisPass>(F), AM.getResult<TargetIRAnalysis>(F),
         AM.getResult<HIRLoopStatisticsAnalysis>(F), F);

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
    AU.addRequiredTransitive<TargetTransformInfoWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return driver(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                  getAnalysis<HIRArraySectionAnalysisWrapperPass>().getASA(),
                  getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                  getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F),
                  getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(), F);
  }
};

char HIRInterLoopBlockingLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRInterLoopBlockingLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRArraySectionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(HIRInterLoopBlockingLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRInterLoopBlockingPass() {
  return new HIRInterLoopBlockingLegacyPass();
}
#endif // INTEL_FEATURE_SW_ADVANCED
