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
#define LLVM_DEBUG_DD_EDGES(X) DEBUG_WITH_TYPE(OPT_SWITCH "-dd", X)

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(true),
                                 cl::Hidden, cl::desc("Disable " OPT_DESC "."));

static cl::opt<int>
    PresetStripmineSize(OPT_SWITCH "-stripmine-size", cl::init(2),
                        cl::ReallyHidden,
                        cl::desc("Preset stripmine size for " OPT_DESC));

static cl::opt<bool> SkipNormalizingByStripLoops(
    OPT_SWITCH "-skip-normalize", cl::init(true), cl::Hidden,
    cl::desc("Skip normalization of byStripLoops in " OPT_DESC "."));

static cl::opt<std::string>
    FilterFunc(OPT_SWITCH "-filter-func", cl::ReallyHidden,
               cl::desc("Run " OPT_DESC " only on the specified function."));

static cl::opt<std::string> RewriteFilterFunc(
    OPT_SWITCH "-rewrite-filter-func", cl::ReallyHidden,
    cl::desc("If given, only to this function, " OPT_DESC " is applied."));

static cl::opt<bool> DisableTransform("disable-rewrite-" OPT_SWITCH,
                                      cl::init(false), cl::Hidden,
                                      cl::desc("Only check " OPT_DESC "."));

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

  // Returns whether a structural check is passed.
  // Depending on conditions, different subsequent actions
  // can occur. (e.g. reset/bailout/skipnode)
  // TODO: Some of the conditions incurs reset(), while
  // others bailout() or nothing.
  // See if breaking this function further can help.
  bool checkStructure(HLLoop *Loop) {

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
               dbgs() << IsProfitable << "\n";
               dbgs() << "State: " << State << "\n"; for (auto *Loop
                                                          : InnermostLoops) {
                 dbgs() << Loop->getNumber() << " ";
               } dbgs() << "\n");

    setDone(true);
    return true;
  }

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

// Legality checker for an innermost loop.
// It examines if the memrefs are spatial accesses. Also it checks
// if an adjustment of dimension indices is possible. Through the
// adjustment, subsequent loops are aligned together to check mutual
// data dependencies.
// For a given innermost loop, its reads dependencies to upward loops
// are verified.
class InnermostLoopAnalyzer {
  typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;
  typedef DDRefGrouping::RefGroupVecTy<RegDDRef *> RefGroupVecTy;
  typedef DDRefGrouping::RefGroupTy<RegDDRef *> RefGroupTy;

public:
  InnermostLoopAnalyzer(
      const HLLoop *Loop, unsigned OutermostLoopLevel,
      SmallVectorImpl<DimInfoTy> &DimInfos,
      BaseIndexToLowersAndStridesTy &BaseIndexToLowersAndStrides,
      StringRef FuncName)
      : InnermostLoop(Loop), DimInfos(DimInfos),
        BaseIndexToLowersAndStrides(BaseIndexToLowersAndStrides),
        Func(FuncName), OutermostLoopLevel(OutermostLoopLevel) {

    MemRefGatherer::gatherRange(InnermostLoop->child_begin(),
                                InnermostLoop->child_end(), Refs);
  }

  // The loopnest containing this innermost loop belongs to
  // could be a member of HLNodes to be enclosed by by-strip loops.
  const RegDDRef *couldBeAMember(BasePtrIndexSetTy &DefinedBasePtr,
                                 BasePtrIndexSetTy &ReadOnlyBasePtr,
                                 DDGraph DDG, const HLLoop *LCA = nullptr) {
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
    if (!canCalcDimInfo(Groups, DefinedBasePtr, ReadOnlyBasePtr, DDG, &DimInfos,
                        LCA)) {
      printDiag("calcDimInfo ", Func, InnermostLoop);
      return nullptr;
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
                             const RegDDRef *RepDefRef) {

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
        if (!CanonExprUtils::getConstDistance(CE, RepDefCE, &Dist) ||
            Dist > 0) {

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

  const RegDDRef *getALval() const {

    for (auto *Ref : Refs) {
      if (Ref->isLval())
        return Ref;
    }

    return nullptr;
  }

  // Alignment of making every Lval in the form of Array[I_n][I_n+1][I_n+2]
  // will be used for future dep check.
  // Thus, this function makes sure all LvalRefs dimensions CEs are equal.
  // If so, return one of the DefRef as a representative Ref.
  // Otherwise, a nullptr is returned.
  const RegDDRef *checkDefsForAlignment() const {

    // Get a Lval ref with maximum numDims.
    // We assume refs with a maximum number of dimensions
    // are spatial references.
    // For example, if a loop body contains
    // A[i][j][k] and B[k], we pick A[i][j][k] because, that reference
    // will have more pieces of information.
    const RegDDRef *Representative = getALval();

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

        if (!CanonExprUtils::areEqual(std::get<0>(CEPair),
                                      std::get<1>(CEPair))) {
          printMarker("checkDefsForAlignemnts() fails: ",
                      {Representative, Ref});
          return nullptr;
        }
      }
    }

    return Representative;
  }

  bool areEqualLowerBoundsAndStrides(const RegDDRef *FirstRef,
                                     const RefGroupTy &OneGroup) {

    unsigned NumDims = FirstRef->getNumDimensions();

    for (auto *Ref : OneGroup) {

      if (NumDims != Ref->getNumDimensions()) {
        // This function does not check anything for following cases.
        // Fortran dope-vectors.
        // (@upml_mod_mp_byh_)[0].6[0].2;
        // (@upml_mod_mp_byh_)[0].0;
        // (@upml_mod_mp_ayh_)[0].6[0].2;
        // (@upml_mod_mp_ayh_)[0].0;

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
  // %2122 = (@globalvar_mod_mp_ezsize_)[0:0:24([6 x i32]*:0)][0:4:4([6 x
  // i32]:6)]; (*)
  // + DO i2 = 0, sext.i32.i64((1 + %2)) + -2, 1   <DO_LOOP>
  // |   + DO i3 = 0, sext.i32.i64(%1) + -1, 1   <DO_LOOP>
  // |   |   %2230 = (%5)[%2122:i2 + 1:8 * (sext.i32.i64((1 + (-1 * %2120) +
  // %2121)) * sext.i32.i64((1 + (-1 * %2118) + %2119)))(double*:0)][%2120:i3 +
  // 1:8 * sext.i32.i64((1 + (-1 * %2118) +
  // %2119))(double*:0)][%2118:2:8(double*:0)] ...

  // %4773 = (@globalvar_mod_mp_ezsize_)[0:0:24([6 x i32]*:0)][0:4:4([6 x
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
                                           const HLLoop *LCA = nullptr) {

    auto GetLoadRval = [DDG](const BlobDDRef *BRef) -> const RegDDRef * {
      for (auto *Edge : DDG.incoming(BRef)) {
        // Only flow edge should exist.
        if (!Edge->isFlow())
          return nullptr;

        if (const HLInst *Inst =
                dyn_cast<HLInst>(Edge->getSrc()->getHLDDNode())) {

          return isa<LoadInst>(Inst->getLLVMInstruction())
                     ? Inst->getRvalDDRef()
                     : nullptr;

        } else {
          return nullptr;
        }
      }

      return nullptr;
    };

    auto compareRefs =
        [GetLoadRval, LCA](const CanonExpr *CE1, const CanonExpr *CE2,
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
                      {Ref1->getParentLoop(), Ref2->getParentLoop(), LCA},
                      true);

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

      if (LoadRval1 && LoadRval2 &&
          !DDRefUtils::areEqual(LoadRval1, LoadRval2)) {
        printMarker("LoadRval1 != LoadRval2\n Ref1, Ref2: ", {Ref1, Ref2}, true,
                    true);
        printMarker("LoadRval1, LoadRval2: ", {LoadRval1, LoadRval2}, true,
                    true);
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

  bool tracebackEqualityOfLowersAndStrides(const RegDDRef *Ref, DDGraph DDG,
                                           const HLLoop *LCA = nullptr) {

    unsigned BasePtrIndex = Ref->getBasePtrBlobIndex();
    if (BaseIndexToLowersAndStrides.find(BasePtrIndex) ==
        BaseIndexToLowersAndStrides.end()) {
      BaseIndexToLowersAndStrides.insert(std::make_pair(BasePtrIndex, Ref));
    } else {
      if (!tracebackEqualityOfLowersAndStrides(
              BaseIndexToLowersAndStrides[BasePtrIndex], Ref, DDG, LCA))
        return false;
    }
    return true;
  }

  // Make sure all dimensions with Blob type, are equal.
  // If not, the memref should be read-only so far.
  // Store that piece of information.
  bool checkEqualityOfBlobDimensions(const RefGroupTy &OneGroup,
                                     const DimInfoVecTy &FirstRefDimInfoVec,
                                     const BasePtrIndexSetTy &DefinedBasePtr,
                                     BasePtrIndexSetTy &ReadOnlyBasePtr) const {
    auto *FirstRef = OneGroup.front();

    for (auto *Ref : make_range(std::next(OneGroup.begin()), OneGroup.end())) {
      for (unsigned DimNum = 1, Size = FirstRefDimInfoVec.size();
           DimNum <= Size; DimNum++) {

        if (!FirstRefDimInfoVec[DimNum - 1].isBlob())
          continue;

        // Blobs should be the same throughout refs.
        if (!CanonExprUtils::areEqual(FirstRef->getDimensionIndex(DimNum),
                                      Ref->getDimensionIndex(DimNum))) {

          if (!DefinedBasePtr.count(FirstRef->getBasePtrBlobIndex())) {

            // Defer the check by storing this piece of information.
            ReadOnlyBasePtr.insert(FirstRef->getBasePtrBlobIndex());

          } else {
            printMarker("Def but different blobIndex: ", {FirstRef, Ref});

            return false;
          }
        }

      } // end of dimension
    }   // end of one Ref group

    return true;
  }

  // Check across all groups if DimInfos are the same.
  using NumDimsToDimInfoVecTy =
      std::unordered_map<unsigned, std::pair<const RegDDRef *, DimInfoVecTy>>;
  bool checkAcrossGroups(NumDimsToDimInfoVecTy &NumDimsToDimInfoVec,
                         const RegDDRef *FirstRef,
                         const DimInfoVecTy &FirstRefDimInfoVec,
                         const BasePtrIndexSetTy &DefinedBasePtr,
                         BasePtrIndexSetTy &ReadOnlyBasePtr) const {

    // For one given innermost loop, and for a number of dimensions,
    // check if all DimInfo is equal.
    if (NumDimsToDimInfoVec.count(FirstRef->getNumDimensions())) {
      // Make sure the equality with the Dim info of the current group and
      // a recorded Dim info.
      std::pair<const RegDDRef *, DimInfoVecTy> &Recorded =
          NumDimsToDimInfoVec[FirstRef->getNumDimensions()];

      // Mismatch of IndexCEs are all right if refs are read-only.
      // caveat - Being read-only are checked only up to this loopnest from
      // lexically upward loopnests. However, to be correct and safe,
      // checking truely being read-only throughout all loopnests in
      // candidate set should be done. At this point, lexically downward
      // loopnests hasn't been examined yet.
      // Notice that DimInfos are considered same if LevelOffset is the same.
      // Const and Blob dim only stores their kinds to LevelOffset.
      if (!std::equal(Recorded.second.begin(), Recorded.second.end(),
                      FirstRefDimInfoVec.begin())) {
        if (!DefinedBasePtr.count(FirstRef->getBasePtrBlobIndex())) {
          ReadOnlyBasePtr.insert(FirstRef->getBasePtrBlobIndex());
        } else {
          printMarker("Mismatch in Refs with the same num of dims", {FirstRef});

          return false;
        }
      }

    } else {
      NumDimsToDimInfoVec.insert(
          {FirstRef->getNumDimensions(),
           {std::make_pair(FirstRef, FirstRefDimInfoVec)}});
    }

    return true;
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
  bool canCalcDimInfo(const RefGroupVecTy &Groups,
                      BasePtrIndexSetTy &DefinedBasePtr,
                      BasePtrIndexSetTy &ReadOnlyBasePtr, DDGraph DDG,
                      DimInfoVecImplTy *DimInfos, const HLLoop *LCA = nullptr) {

    // A map from the number of dimensions (of a ref) to DimInfoTy
    // Different refs with the same number of dimensions are
    // expected to have the same DimInfo
    // RegDDRef* is not being actively used. Mostly for debugging.
    NumDimsToDimInfoVecTy NumDimsToDimInfoVec;

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
      for (auto *Ref :
           make_range(std::next(OneGroup.begin()), OneGroup.end())) {
        DimInfoVecTy DimInfoVec;
        if (!analyzeDims(Ref, DimInfoVec))
          return false;

        // Compare against Front
        if (!std::equal(FirstRefDimInfoVec.begin(), FirstRefDimInfoVec.end(),
                        DimInfoVec.begin()))
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
                                         DefinedBasePtr, ReadOnlyBasePtr)) {
        return false;
      }

      // Check across all groups if DimInfos are the same.
      if (!checkAcrossGroups(NumDimsToDimInfoVec, FirstRef, FirstRefDimInfoVec,
                             DefinedBasePtr, ReadOnlyBasePtr)) {
        return false;
      }
    } // end Groups

    // Now pick a DimInfo with the maximum DimNum
    // TODO: It is not clear if for a konst dim, if the ref with
    //       the smallest value is captured. Same for a blob dim.
    //       Make sure how it affects handling dependencies with
    //       konst and blob dims.
    //
    //       Also, consider just picking DimInfo of any LvalDDRef
    //       (or RepDefRef) since now the equality of num dims are
    //       Verified.
    using PairTy = decltype(NumDimsToDimInfoVec)::value_type;
    auto MaxEntry =
        std::max_element(NumDimsToDimInfoVec.begin(), NumDimsToDimInfoVec.end(),
                         [](const PairTy &P1, const PairTy &P2) {
                           return P1.first <= P2.first;
                         });

    if (DimInfos)
      std::copy((*MaxEntry).second.second.begin(),
                (*MaxEntry).second.second.end(), std::back_inserter(*DimInfos));

    return true;
  }

  // Checks each CE has in one of the three forms:
  //  - single IV + (optional constant) + (optional blob)
  //  - only-constant
  //  - only-blob + (optional constant)
  // In the output argument CEKinds, marks CE forms out the the three.
  // In addition, it checks IV's level strictly decreases as dimnum increases.
  //  e.g. A[i1][i2][i3]
  // Return true, if all conditions are met.
  bool analyzeDims(const RegDDRef *Ref, DimInfoVecImplTy &DimInfoVec) const {

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

  // - Single IV + <optional constant> + <optional blob>
  // - Constant
  // - blob-only + <optional constant>
  bool isValidDim(const CanonExpr *CE, DimInfoTy &DimInfo) const {
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

private:
  const HLLoop *InnermostLoop;
  SmallVectorImpl<DimInfoTy> &DimInfos;
  BaseIndexToLowersAndStridesTy &BaseIndexToLowersAndStrides;
  MemRefGatherer::VectorTy Refs;
  StringRef Func;

  // level of the loop enclosing all spatial loops.
  unsigned OutermostLoopLevel;
};

typedef std::pair<HLLoop *, SmallVector<DimInfoTy, 4>> LoopAndDimInfoTy;
typedef std::vector<LoopAndDimInfoTy> LoopToDimInfoTy;
typedef std::map<const HLLoop *, RegDDRef *> LoopToRefTy;
typedef std::map<const HLLoop *, const RegDDRef *> LoopToConstRefTy;

// Find the lowest ancestor of InnermostLoop, which is deeper than Limit.
// It is not necessarily a HLLoop.
HLNode *findTheLowestAncestor(HLLoop *InnermostLoop, const HLLoop *Limit) {

  assert(Limit->getNestingLevel() < InnermostLoop->getNestingLevel());

  HLNode *Node = InnermostLoop;
  HLNode *Prev = InnermostLoop;
  while (Node != Limit) {
    Prev = Node;
    Node = Node->getParent();
  }

  return Prev;
}

class Transformer {
public:
  HIRDDAnalysis &DDA;

  // Entry value 0 denotes no-blocking.
  // Size of StripmineSizes should be the same as global NumDims
  Transformer(ArrayRef<unsigned> StripmineSizes,
              const LoopToDimInfoTy &InnermostLoopToDimInfos,
              const LoopToConstRefTy &InnermostLoopToRepRef,
              HLLoop *OutermostLoop, HIRDDAnalysis &DDA)
      : DDA(DDA), StripmineSizes(StripmineSizes),
        InnermostLoopToDimInfos(InnermostLoopToDimInfos),
        InnermostLoopToRepRef(InnermostLoopToRepRef),
        OutermostLoop(OutermostLoop), NumByStripLoops(0) {
    unsigned NumDims = StripmineSizes.size();
    ByStripLoopLowerBlobs.resize(NumDims);
    ByStripLoopUpperBlobs.resize(NumDims);
    ByStripLoops.resize(NumDims, 0x0);

    // Initialize the number of ByStripLoops.
    NumByStripLoops = getNumByStripLoops(StripmineSizes);
  }

  static unsigned getNumByStripLoops(ArrayRef<unsigned> StripmineSizes) {
    return count_if(StripmineSizes, [](unsigned Size) { return Size; });
  }

  // Make sure every dimension has a target loop.
  static bool checkDimsToLoops(ArrayRef<unsigned> StripmineSizes,
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
            getLoopMatchingDimNum(DimNum, E.second, InnermostLoop);
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

    printMarker("Initial: ", {OutermostLoop}, true);

    LLVM_DEBUG(dbgs() << "== * == AAAA \n"; for (auto &LoopAndDimInfo
                                                 : InnermostLoopToDimInfos) {
      dbgs() << LoopAndDimInfo.first->getNumber() << "\n";
    });

    LoopToRefTy InnermostLoopToAdjustingRef;
    prepareAdjustingRefs(InnermostLoopToAdjustingRef);

    HLNode *AnchorNode = findTheLowestAncestor(
        (*InnermostLoopToDimInfos.begin()).first, OutermostLoop);

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
    SmallPtrSet<const HLInst *, 32> LoadInstsToClone;
    SmallVector<std::pair<unsigned, unsigned>, 16> CopyToLoadIndexMap;
    collectLoadsToClone(AnchorNode->getTopSortNum(), LoadInstsToClone,
                        CopyToLoadIndexMap);

    // Align original spatial loops
    alignSpatialLoops(InnermostLoopToAdjustingRef);

    assert(isa<HLLoop>(AnchorNode) &&
               AnchorNode->getNodeLevel() ==
                   (OutermostLoop->getNestingLevel() + 1) ||
           !isa<HLLoop>(AnchorNode) &&
               AnchorNode->getNodeLevel() == OutermostLoop->getNestingLevel());

    unsigned StartTopSortNum = AnchorNode->getTopSortNum();
    unsigned LastTopSortNum =
        InnermostLoopToDimInfos.back().first->getTopSortNum();
    collectAdditionalLiveInsToByStripLoops(StartTopSortNum, LastTopSortNum);

    // Note that invalidation only happens here because DDG of original
    // HLNodes are needed before this point. For example,
    // collectLoadsToClone() uses DDG to traceback load instructions.
    HIRInvalidationUtils::invalidateBody(OutermostLoop);

    DenseMap<unsigned, unsigned> OrigToCloneIndexMap;
    SmallVector<const RegDDRef *, 32> AuxRefsForByStripBounds;

    cloneAndAddLoadInsts(LoadInstsToClone, AnchorNode, OrigToCloneIndexMap,
                         AuxRefsForByStripBounds);

    // Merge CopyToLoadIndexMap into OrigToCloneIndexMap
    for (auto KV : CopyToLoadIndexMap) {
      unsigned NewKey = KV.first;
      unsigned NewVal = OrigToCloneIndexMap[KV.second];
      assert(NewVal);
      OrigToCloneIndexMap.insert(std::make_pair(NewKey, NewVal));
    }

    if (!computeByStripLoopBounds(InnermostLoopToAdjustingRef,
                                  OrigToCloneIndexMap,
                                  AuxRefsForByStripBounds)) {

      LLVM_DEBUG(dbgs() << "== * == Failed computeByStripLoopBounds \n");

      return false;
    }

    HLLoop *InnermostByStripLoop =
        addByStripLoops(AnchorNode, LoadInstsToClone, AuxRefsForByStripBounds);

    LLVM_DEBUG(dbgs() << "InnermostByStripLoop: \n");
    LLVM_DEBUG(InnermostByStripLoop->dump());
    // Step 1. Create a map from NodeToMove to InnermostLoop
    // The vector of OuterNode should be enough, innermost loop is for
    // debugging.
    SmallVector<std::pair<HLNode *, HLNode *>, 16> OuterNodeToInnermostLoop;

    for (auto &LoopAndDimInfo : InnermostLoopToDimInfos) {
      HLNode *NodeToMove =
          findTheLowestAncestor(LoopAndDimInfo.first, OutermostLoop);

      OuterNodeToInnermostLoop.emplace_back(NodeToMove, LoopAndDimInfo.first);
    }

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
      updateSpatialIVs(&Node, NumByStripLoops);
      updateDefAtLevelOfSpatialLoops(&Node);
    }

    // Step 3. apply blocking guards to target loops
    // Note: Step 3 should come after Step 2.
    assert(OuterNodeToInnermostLoop.size() == InnermostLoopToDimInfos.size());

    for (int I = 0, E = OuterNodeToInnermostLoop.size(); I < E; I++) {
      assert(OuterNodeToInnermostLoop[I].second ==
             InnermostLoopToDimInfos[I].first);

      applyBlockingGuardsToSpatialLoops(InnermostLoopToDimInfos[I].first,
                                        InnermostLoopToDimInfos[I].second,
                                        InnermostLoopToAdjustingRef);
    }

    // Normalize all spatial Loops and byStripLoops.
    for (auto &LoopAndDimInfo : InnermostLoopToDimInfos) {
      HLLoop *InnermostLoop = LoopAndDimInfo.first;
      SmallVector<HLLoop *, 4> Loops;

      auto CollectSpatialLoops = [InnermostLoop,
                                  &Loops](HLLoop *Limit) -> HLLoop * {
        HLLoop *Lp = InnermostLoop;
        HLLoop *PrevLp = nullptr;
        while (Lp != Limit) {
          PrevLp = Lp;
          Loops.push_back(PrevLp);
          Lp = Lp->getParentLoop();
        }
        return PrevLp;
      };

      if (SkipNormalizingByStripLoops) {
        CollectSpatialLoops(InnermostByStripLoop);
      } else {
        CollectSpatialLoops(OutermostLoop);
      }

      SmallVector<unsigned, 8> LiveInsFromNormalization;
      // SL will be all spatial loops and/or by strip loops.
      for (HLLoop *SL : make_range(Loops.rbegin(), Loops.rend())) {
        SL->normalize();
        SL->getLowerDDRef()->populateTempBlobSymbases(LiveInsFromNormalization);
        SL->getUpperDDRef()->populateTempBlobSymbases(LiveInsFromNormalization);
        SL->addLiveInTemp(LiveInsFromNormalization);

        // MarkNotBlock to inhibit regular loop blocking pass coming later
        SL->markDoNotBlock();
      }
    }

    LLVM_DEBUG_PROFIT_REPORT(dbgs() << "After updating: \n";
                             OutermostLoop->dump());
    printMarker("Detail: After updating inner Loops: ", {OutermostLoop}, true,
                true);

    OutermostLoop->getParentRegion()->setGenCode();
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

  // Collect Lvals in OutermostLoop, that has outgoing edge
  // into ByStripLoops. StartTopSortNum, and LastTopSortNum indicates
  // the beginning and ending of ByStripLoops. Lvals in OutermostLoop
  // before StartTopSortNum, and whose sinks are between StartTopSortNum
  // and  LastTopSortNum  are collected.
  void collectAdditionalLiveInsToByStripLoops(unsigned StartTopSortNum,
                                              unsigned LastTopSortNum) {

    DDGraph DDG = DDA.getGraph(OutermostLoop);

    for (auto &Node :
         make_range(OutermostLoop->child_begin(), OutermostLoop->child_end())) {
      const HLDDNode *DDNode = dyn_cast<HLDDNode>(&Node);
      if (!DDNode)
        continue;
      unsigned CurrentNum = DDNode->getTopSortNum();

      if (CurrentNum >= StartTopSortNum)
        continue;

      const RegDDRef *Lval = DDNode->getLvalDDRef();
      if (!Lval)
        continue;

      // Assume no blob ddrefs from Lval
      for (auto *Edge : DDG.outgoing(Lval)) {
        const HLDDNode *SinkNode = Edge->getSink()->getHLDDNode();
        unsigned UseTopSortNum = SinkNode->getTopSortNum();

        if (UseTopSortNum < StartTopSortNum || UseTopSortNum > LastTopSortNum) {
          continue;
        }

        LiveInsOfAllSpatialLoop.push_back(Lval->getSymbase());
      }
    }
  }

  void updateDefAtLevelOfSpatialLoops(HLNode *Node) const {

    // Increase the blob DDRef's defined at level first

    unsigned ThresholdLoopLevel = OutermostLoop->getNestingLevel();
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
  void
  alignSpatialLoopBody(const LoopAndDimInfoTy &LoopAndDimInfo,
                       const LoopToRefTy &InnermostLoopToAdjustingRef) const {

    HLLoop *InnermostLoop = LoopAndDimInfo.first;
    // Get a Ref to be used for Adjusting
    const RegDDRef *AdjustingRef =
        InnermostLoopToAdjustingRef.at(InnermostLoop);

    LLVM_DEBUG(dbgs() << "*** AdjustingRef for Loop:"
                      << InnermostLoop->getNumber() << "\n");
    LLVM_DEBUG(AdjustingRef->dump(); dbgs() << "\n");

    const RegDDRef *RepRef = InnermostLoopToRepRef.at(InnermostLoop);
    unsigned Level = InnermostLoop->getNestingLevel();
    const auto &DimInfo = LoopAndDimInfo.second;
    DenseMap<unsigned, unsigned> MapFromLevelToDim;
    for (unsigned I = 0, Size = LoopAndDimInfo.second.size(); I < Size; I++) {
      if (!DimInfo[I].hasIV())
        continue;

      MapFromLevelToDim.insert({Level - DimInfo[I].LevelOffset, I + 1});
    }

    printMarker("AdjustingRef at alignRefs: ", {AdjustingRef});
    printMarker("RepRef at alignRefs: ", {RepRef});

    ForEach<RegDDRef>::visitRange(
        InnermostLoop->child_begin(), InnermostLoop->child_end(),
        [AdjustingRef, &MapFromLevelToDim, InnermostLoop](RegDDRef *Ref) {
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
          printMarker("InnermostLoop Number: ", {InnermostLoop});

          // Cannot use RepRef here as an auxiliary ref
          // because RepRef itself is aligned by this function
          // and get changed. Instead, use AdjustingRef,
          // since it is a RepRef minus IVs.
          // It contains all the blobs.
          Ref->makeConsistent({OrigRef.get(), AdjustingRef});
        });
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
  void alignSpatialLoops(const LoopToRefTy &InnermostLoopToAdjustingRef) const {

    // TODO: Why only innermost loop?
    //       This is under the assumption, no other loop body contains
    //       non-loop-invariant memrefs. At least make sure if that is true.
    for (auto &LoopAndDimInfo : InnermostLoopToDimInfos) {
      alignSpatialLoopBody(LoopAndDimInfo, InnermostLoopToAdjustingRef);
    }

    int GlobalNumDims = StripmineSizes.size();
    for (int DimNum = 1; DimNum <= GlobalNumDims; DimNum++) {

      // Alignment should be done at all levels, regardless of
      // being loop-tiled or not.

      // Collect all Lower/AlignedUpperBounds from InnermostLoop
      for (auto &LoopAndDimInfoVec : InnermostLoopToDimInfos) {
        const HLLoop *InnermostLoop = LoopAndDimInfoVec.first;

        const HLLoop *TargetLoop = getLoopMatchingDimNum(
            DimNum, LoopAndDimInfoVec.second, InnermostLoop);
        if (!TargetLoop)
          continue;

        const RegDDRef *AdjustingRef =
            InnermostLoopToAdjustingRef.at(InnermostLoop);
        RegDDRef *LBRef = const_cast<RegDDRef *>(TargetLoop->getLowerDDRef());

        printMarker("LB before: ", {LBRef}, true);

        alignSpatialLoopBounds(LBRef, AdjustingRef, DimNum);

        RegDDRef *UBRef = const_cast<RegDDRef *>(TargetLoop->getUpperDDRef());
        alignSpatialLoopBounds(UBRef, AdjustingRef, DimNum);
        printMarker("LB After: ", {TargetLoop->getLowerDDRef()});
      }
    }
  }

  // Start from the RHS of Copy to find the eventual load instruction.
  // If not found, return nullptr.
  // Example)
  //   %t1 = %a[..] -- (1)
  //   %t2 = %t1    -- (2)
  // Starting from %t1, a load instruction (1) is found.
  const HLInst *tracebackToLoad(const HLInst *Copy, DDGraph DDG) const {
    assert(Copy->isCopyInst());
    const RegDDRef *Rval = Copy->getRvalDDRef();
    const HLInst *Load = nullptr;

    while (Rval) {
      for (auto *Edge : DDG.incoming(Rval)) {
        if (!Edge->isFlow())
          continue;

        HLNode *SrcNode = Edge->getSrc()->getHLDDNode();
        if (const HLInst *LoadOrCopy = dyn_cast<HLInst>(SrcNode)) {
          if (isa<LoadInst>(LoadOrCopy->getLLVMInstruction())) {
            // Found Load
            printMarker(" == Found Load: \n", {LoadOrCopy});

            Load = LoadOrCopy;
            return Load;

          } else if (LoadOrCopy->isCopyInst()) {
            printMarker("Found Copy: \n", {SrcNode});

            // Trace-back to load.
            Rval = LoadOrCopy->getRvalDDRef();
          }
        } else {
          return Load;
        }
      }
      break;
    } // while

    return Load;
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

  // Find the load instruction starting from SrcNode.
  // If SrcNode is a load, return it.
  // If it is a copy, trace back to a load and return it.
  const std::pair<const HLInst *, const HLInst *>
  findLoad(const HLDDNode *SrcNode, DDGraph DDG) const {

    const HLInst *LoadOrCopy = dyn_cast<HLInst>(SrcNode);
    if (!LoadOrCopy)
      return std::make_pair(nullptr, nullptr);

    if (isa<LoadInst>(LoadOrCopy->getLLVMInstruction())) {
      LLVM_DEBUG(dbgs() << "Found Load: \n");
      LLVM_DEBUG(SrcNode->dump());
      LLVM_DEBUG(dbgs() << "\n");

      return std::make_pair(LoadOrCopy, nullptr);
    } else if (LoadOrCopy->isCopyInst()) {
      const HLInst *CopyInst = LoadOrCopy;

      LLVM_DEBUG(dbgs() << "Found Copy: \n");
      LLVM_DEBUG(SrcNode->dump());
      LLVM_DEBUG(dbgs() << "\n");

      // Trace-back to load.
      return std::make_pair(tracebackToLoad(LoadOrCopy, DDG), CopyInst);
    }

    return std::make_pair(nullptr, nullptr);
  }

  // Collect all load instructions loading temps in DestRef.
  // Make sure all such load instructions are after AnchorNode.
  void findLoadsOfTemp(
      DDGraph DDG, const RegDDRef *DestRef, unsigned AnchorNodeTopSortNum,
      SmallPtrSetImpl<const HLInst *> &LoadInsts,
      std::map<const HLInst *, const HLInst *> &CopyToLoadMap) const {

    for (auto *Edge : DDG.incoming(DestRef)) {
      if (!Edge->isFlow())
        continue;

      std::pair<const HLInst *, const HLInst *> LoadAndCopy =
          findLoad(Edge->getSrc()->getHLDDNode(), DDG);

      if (!LoadAndCopy.first ||
          LoadAndCopy.first->getTopSortNum() < AnchorNodeTopSortNum)
        return;

      LLVM_DEBUG_DD_EDGES(printDDEdges(LoadAndCopy.first, DDG));

      if (LoadAndCopy.second)
        CopyToLoadMap.emplace(LoadAndCopy.second, LoadAndCopy.first);

      LoadInsts.insert(LoadAndCopy.first);
    }

    for (auto *BlobRef :
         make_range(DestRef->blob_begin(), DestRef->blob_end())) {

      for (auto *Edge : DDG.incoming(BlobRef)) {
        if (!Edge->isFlow())
          continue;

        LLVM_DEBUG(Edge->dump());
        std::pair<const HLInst *, const HLInst *> LoadAndCopy =
            findLoad(Edge->getSrc()->getHLDDNode(), DDG);

        if (!LoadAndCopy.first ||
            LoadAndCopy.first->getTopSortNum() < AnchorNodeTopSortNum)
          return;

        LLVM_DEBUG_DD_EDGES(printDDEdges(LoadAndCopy.first, DDG));

        if (LoadAndCopy.second)
          CopyToLoadMap.emplace(LoadAndCopy.second, LoadAndCopy.first);

        LoadInsts.insert(LoadAndCopy.first);
      }
    }
  }

  void collectLoadsToClone(
      unsigned AnchorNodeTopSortNum,
      SmallPtrSetImpl<const HLInst *> &LoadInstsToClone,
      SmallVectorImpl<std::pair<unsigned, unsigned>> &CopyToLoadIndexMap) {

    printMarker("Collect: ", {OutermostLoop}, true);
    printMarker("Collect-details: ", {OutermostLoop}, true, true);

    DDGraph DDG = DDA.getGraph(OutermostLoop);

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
      findLoadsOfTemp(DDG, RepRef, AnchorNodeTopSortNum, LoadInstsToClone,
                      CopyToLoadMap);
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
            getLoopMatchingDimNum(DimNum, E.second, InnermostLoop);
        if (!TargetLoop)
          continue;

        findLoadsOfTemp(DDG, TargetLoop->getUpperDDRef(), AnchorNodeTopSortNum,
                        LoadInstsToClone, CopyToLoadMap);
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
  }

  // LB(UB) of a By-strip loop is the "min"("max") of all lower bounds of
  // original spatial loop corresponding to the same DimNum.
  // This function collects all LBs(UBs) of original spatial loops, and
  // generate min(max) blobs of them.
  bool computeByStripLoopBounds(
      const LoopToRefTy &InnermostLoopToAdjustingRef,
      const DenseMap<unsigned, unsigned> &OrigToCloneIndexMap,
      SmallVectorImpl<const RegDDRef *> &AuxRefs) {

    SmallVector<std::pair<unsigned, unsigned>, 16> BlobIndexMap;
    std::copy(OrigToCloneIndexMap.begin(), OrigToCloneIndexMap.end(),
              std::back_inserter(BlobIndexMap));

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
            getLoopMatchingDimNum(DimNum, E.second, InnermostLoop);
        if (!TargetLoop)
          continue;

        RegDDRef *NewLBRef = TargetLoop->getLowerDDRef()->clone();
        bool Replaced = NewLBRef->replaceTempBlobs(BlobIndexMap);
        (void)Replaced;
        AlignedLowerBounds[DimNum - 1].push_back(
            NewLBRef->getSingleCanonExpr());
        AuxRefs.push_back(TargetLoop->getLowerDDRef());

        LLVM_DEBUG(dbgs() << "LB after (" << Replaced << ") ";
                   NewLBRef->dump(1); dbgs() << "\n");

        RegDDRef *NewUBRef = TargetLoop->getUpperDDRef()->clone();
        Replaced = NewUBRef->replaceTempBlobs(BlobIndexMap);
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
      SmallPtrSetImpl<const HLInst *> &LoadInstsToClone, HLNode *AnchorNode,
      DenseMap<unsigned, unsigned> &OrigToCloneIndexMap,
      SmallVectorImpl<const RegDDRef *> &AuxRefsForByStripBounds

  ) {
    for (auto *Load : LoadInstsToClone) {
      HLInst *NewLoad = Load->getHLNodeUtils().createLoad(
          Load->getRvalDDRef()->clone(), "clone_load");

      unsigned OldIndex =
          Load->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex();

      unsigned NewIndex =
          NewLoad->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex();

      assert(OldIndex != NewIndex);

      OrigToCloneIndexMap.insert(std::make_pair(OldIndex, NewIndex));

      printMarker("Orig Load: ", Load, true, true);
      printMarker("New Load: ", NewLoad, true, true);

      HLNodeUtils::insertBefore(AnchorNode, NewLoad);

      LiveInsOfAllSpatialLoop.push_back(NewLoad->getLvalDDRef()->getSymbase());
      AuxRefsForByStripBounds.push_back(NewLoad->getLvalDDRef());
    }
  }

  // Generate by-strip loops and insert before AnchorNode.
  // Returns the innermost by-strip loop, where spatial loops will be added.
  // Add ByStrip loops, and also compute UBs of unit-strided Loop
  //   e.g. DO IV = by_strip_lb, by_strip_ub, by_strip_step
  //          tile_end = min(IV + by_strip_step - 1, by_strip_ub)
  //   IV is the tile's begin.
  //   tile_end is the last element of tile, not the past the last.
  HLLoop *
  addByStripLoops(HLNode *AnchorNode,
                  const SmallPtrSetImpl<const HLInst *> &LoadInstsToClone,
                  ArrayRef<const RegDDRef *> AuxRefsFromSpatialLoops) {

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
      ByStrip->addLiveInTemp(ArrayRef<unsigned>(OutermostLoop->live_in_begin(),
                                                OutermostLoop->live_in_end()));

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
  void updateSpatialIVs(HLNode *Node, unsigned ByStripLoopDepth) {

    // Now loop levels get as deep as by-strip loopnests
    unsigned OutermostLoopLevel = OutermostLoop->getNestingLevel();
    ForEach<RegDDRef>::visit(Node, [OutermostLoopLevel,
                                    ByStripLoopDepth](RegDDRef *Ref) {
      for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
        for (unsigned Lvl = MaxLoopNestLevel; Lvl > OutermostLoopLevel; Lvl--) {
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
            return std::make_pair(Ref, CE->getDefinedAtLevel());
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
      HLLoop *InnermostLoop, ArrayRef<DimInfoTy> DimInfos,
      const LoopToRefTy &InnermostLoopToAdjustingRef) {

    SmallVector<unsigned, 2> ConstOrBlobDimNums;

    SmallVector<unsigned, 4> AllNewLiveIn;
    SmallVector<HLLoop *, 3> SpatialLoops;

    // Scan through from the innermost spatial loop to facilitate adding LiveIns
    for (unsigned DimNum = 1, Sizes = DimInfos.size(); DimNum <= Sizes;
         DimNum++) {

      if (isNoBlockDim(DimNum))
        continue;

      HLLoop *TargetLoop =
          getLoopMatchingDimNum(DimNum, DimInfos, InnermostLoop);
      if (!TargetLoop) {
        ConstOrBlobDimNums.push_back(DimNum);
        continue;
      }
      SpatialLoops.push_back(TargetLoop);

      HIRInvalidationUtils::invalidateBounds(TargetLoop);
      HIRInvalidationUtils::invalidateBody(TargetLoop);

      unsigned NewLiveIn = addLoopBoundsGuards(TargetLoop, DimNum);
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
    HLLoop *OutermostSpatialLoop = SpatialLoops.back();

    for (auto DimNum : ConstOrBlobDimNums) {

      assert(ByStripLoops.size() >= DimNum);

      const HLLoop *ByStripLoop = ByStripLoops[DimNum - 1];
      CanonExpr *CE = AdjustingRef->getDimensionIndex(DimNum)->clone();

      if (DimInfos[DimNum - 1].isConstant()) {
        RegDDRef *Ref = AdjustingRef->getDDRefUtils().createConstDDRef(
            ByStripLoop->getIVType(), CE->getConstant());

        // TODO: OutermostSpatialLoop is not always correct.
        //       Hoisting might not be valid.
        //       SpatialLoops contains only the blocked loops.
        addIfGuards(Ref, ByStripLoop, OutermostSpatialLoop);

      } else {
        // blob or blob + constant
        RegDDRef *Ref = AdjustingRef->getDDRefUtils().createScalarRegDDRef(
            GenericRvalSymbase, CE->clone());

        // Look for a ref in the innermost
        // TODO: Consider return a AuxRef, which is a copy of CE.
        //       We only need CE, not whole AuxRef.
        std::pair<const RegDDRef *, unsigned> AuxRef =
            findAuxRefWithCE(InnermostLoop, CE);

        addIfGuards(Ref, ByStripLoop, OutermostSpatialLoop, AuxRef.first);

        // Add AuxRef's BlobDDRef's symbase to LiveInTemp of ByStripLoop
        // Using getTempBlobSymbase from CE->getSingleBlobIndex() does not
        // work. For example, CE, sext.i32.i64(%2677) + 1, temp blob symbase
        // could be InvalidSymbase.
        // unsigned AuxTempSymbase = AuxRef->getBlobUtils().getTempBlobSymbase(
        //  CE->getSingleBlobIndex());
        // Can use CE->collectTempBlobIndices(.), but we can approximate with
        // blob ddrefs of AuxRef.
        SmallVector<unsigned, 4> AuxTempSymbases;
        for (auto *BRef :
             make_range(AuxRef.first->blob_begin(), AuxRef.first->blob_end())) {
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

  static bool isNoBlockDim(unsigned DimNum, ArrayRef<unsigned> StripmineSizes) {
    return StripmineSizes[DimNum - 1] == 0;
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

  // Return the loop matching DimNum.
  // InnermostLoop and DimInfos are data to consult with.
  static const HLLoop *getLoopMatchingDimNum(unsigned DimNum,
                                             ArrayRef<DimInfoTy> DimInfos,
                                             const HLLoop *InnermostLoop) {

    // Subscript is either constant or blobs.
    if (!DimInfos[DimNum - 1].hasIV())
      return nullptr;

    const HLLoop *TargetLoop = InnermostLoop;
    for (int I = 0; I < DimInfos[DimNum - 1].LevelOffset; I++) {
      TargetLoop = TargetLoop->getParentLoop();
    }

    return TargetLoop;
  }

  HLLoop *getLoopMatchingDimNum(unsigned DimNum, ArrayRef<DimInfoTy> DimInfos,
                                HLLoop *InnermostLoop) {

    return const_cast<HLLoop *>(
        static_cast<const Transformer &>(*this).getLoopMatchingDimNum(
            DimNum, DimInfos, const_cast<HLLoop *>(InnermostLoop)));
  }

  // Add "if (ByStripIV <= Ref <= TileEndRef)"
  // Ref: dimension index, either constant or a blob.
  // ByStripIV: tile begin,
  // TileEndRef : last element of the tile.
  void addIfGuards(RegDDRef *Ref, const HLLoop *ByStripLoop,
                   HLNode *NodeToEnclose,
                   const RegDDRef *AuxRef = nullptr) const {
    RegDDRef *ByStripIV = ByStripLoop->getDDRefUtils().createConstDDRef(
        ByStripLoop->getIVType(), 0);
    ByStripIV->getSingleCanonExpr()->addIV(ByStripLoop->getNestingLevel(),
                                           InvalidBlobIndex, 1, true);

    RegDDRef *TileEndRef =
        cast<HLInst>(ByStripLoop->getFirstChild())->getLvalDDRef()->clone();

    HLIf *Guard = NodeToEnclose->getHLNodeUtils().createHLIf(
        PredicateTy::ICMP_SLE, ByStripIV, Ref->clone());
    Guard->addPredicate(PredicateTy::ICMP_SLE, Ref->clone(), TileEndRef);
    if (HLLoop *Loop = dyn_cast<HLLoop>(NodeToEnclose)) {
      Loop->extractPreheaderAndPostexit();
    }
    NodeToEnclose->getHLNodeUtils().insertBefore(NodeToEnclose, Guard);
    HLNodeUtils::moveAsFirstThenChild(Guard, NodeToEnclose);

    SmallVector<const RegDDRef *, 2> AuxRefs({TileEndRef});
    if (AuxRef)
      AuxRefs.push_back(AuxRef);

    // Consistency of if-conditions
    for (auto PI = Guard->pred_begin(), E = Guard->pred_end(); PI != E; PI++) {
      Guard->getPredicateOperandDDRef(PI, true)->makeConsistent(AuxRefs);
      Guard->getPredicateOperandDDRef(PI, false)->makeConsistent(AuxRefs);
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
  unsigned addLoopBoundsGuards(HLLoop *Loop, unsigned DimNum) const {

    // Sweep through all rvals by calling makeConsistent().
    // Added to take care of operands of createMin/Max
    // createMin/Max generates extra operands than passed arguments.
    // Temp blobs defAtLevels are all up to date already.
    auto MakeConsistentRvals = [](HLInst *HInst) {
      for (auto *Ref :
           make_range(HInst->op_ddref_begin(), HInst->op_ddref_end())) {
        Ref->makeConsistent({});
      }
    };

    const HLLoop *ByStripLoop = ByStripLoops[DimNum - 1];

    RegDDRef *ByStripIV = ByStripLoop->getDDRefUtils().createConstDDRef(
        ByStripLoop->getIVType(), 0);
    ByStripIV->getSingleCanonExpr()->addIV(ByStripLoop->getNestingLevel(),
                                           InvalidBlobIndex, 1, true);

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
    RegDDRef *MinOp1 = Loop->getUpperDDRef()->clone();
    HLInst *UBInst = HNU.createMin(MinOp1, TileEndRef, nullptr, true, true,
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
      return std::make_pair(BU.createBlob(Val, CE->getDestType()),
                            InvalidBlobIndex);
    } else {
      return std::make_pair(BU.getBlob(CE->getSingleBlobIndex()),
                            CE->getSingleBlobIndex());
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
      if (!CE->isIntConstant() && !CE->convertToStandAloneBlob()) {
        return std::make_pair(nullptr, InvalidBlobIndex);
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

    return std::make_pair(LowerBlob, BlobIndex);
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

private:
  // In the order of DimNum, [DimNum = 1][DimNum = 2] .. and so on.
  ArrayRef<unsigned> StripmineSizes;

  const LoopToDimInfoTy &InnermostLoopToDimInfos;
  const LoopToConstRefTy &InnermostLoopToRepRef;
  // Loop enclosing all the spatial loopnests.
  // Inside OutermostLoop, by-strip loops are generated.
  HLLoop *OutermostLoop;

  SmallVector<std::pair<BlobTy, unsigned>, 4> ByStripLoopLowerBlobs;
  SmallVector<std::pair<BlobTy, unsigned>, 4> ByStripLoopUpperBlobs;

  // Newly generated by-strip loops
  SmallVector<HLLoop *, 4> ByStripLoops;
  SmallVector<unsigned, 4> LiveInsOfAllSpatialLoop;

  // Number of ByStrip loops. Could be different from StripmineSizes.size()
  // because StripmineSizes can contain zeros.
  unsigned NumByStripLoops;
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
  typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

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
    if (!FirstSpatialLoop || FirstSpatialLoop == LastSpatialLoop) {
      return false;
    }

    // State in term of profitablity, should be SECONDHALF.
    // Otherwise, it is not profitable
    if (State != SECONDHALF) {
      return false;
    }

    // I/O calls are supposedly removed by OptVarPred by now.
    if (hasIOCall()) {
      return false;
    }

    // Structural check.
    if (!isCleanCut(LastSpatialLoop, StopLoop)) {
      return false;
    }

    assert(verifyTopSortOrderOfInnermostLoops() &&
           "InnermostLoops are not in Order.");

    // Legality check: check if any of ReadOnlyBasePtr is
    // in DefinedBasePtr.
    if (llvm::any_of(ReadOnlyBasePtr, [this](unsigned BasePtrIndex) {
          return this->DefinedBasePtr.count(BasePtrIndex);
        })) {
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
                      HLLoop *OutermostLoop, HIRDDAnalysis &DDA,
                      StringRef Func) {

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
  SmallVector<unsigned, 4> PreSetStripmineSizes(Size, PresetStripmineSize);
  if (Size == 3) {
    PreSetStripmineSizes[0] = 0;
    PreSetStripmineSizes[1] = 1;
    PreSetStripmineSizes[2] = 8;
  }

  // This check is done after StripmineSizes are determined.
  // If the check fails, transformation does not happen.
  if ((Transformer::getNumByStripLoops(PreSetStripmineSizes) == 0) ||
      !Transformer::checkDimsToLoops(PreSetStripmineSizes,
                                     InnermostLoopToDimInfos)) {
    LLVM_DEBUG_PROFIT_REPORT(
        dbgs()
        << "No transformation: Some dimensions have no matching loop level.\n");
    return false;
  }

  Transformer(PreSetStripmineSizes, InnermostLoopToDimInfos,
              InnermostLoopToRepRef, OutermostLoop, DDA)
      .rewrite();

  return true;
}

// Main driver for HIRInterLoopBlocking.
bool driver(HIRFramework &HIRF, HIRArraySectionAnalysis &HASA,
            HIRDDAnalysis &DDA, const Function &F) {
  if (DisablePass) {
    return false;
  }

  if (!F.isFortran()) {
    return false;
  }

  StringRef FuncName = F.getName();

  if (!FilterFunc.empty() && !FuncName.equals(FilterFunc)) {
    return false;
  }

  ProfitabilityChecker PC(HIRF, HASA, DDA, FuncName);

  // Looks profitable if true. Actual profitablity
  // can be decided after optVarPred.
  bool CouldBeProfitable = PC.isProfitable();

  if (!CouldBeProfitable) {
    LLVM_DEBUG(dbgs() << "NOT Profitable\n");
    return false;
  }

  LLVM_DEBUG_PROFIT_REPORT(dbgs() << "Profitable\n");

  assert(PC.getOutermostLoop());

  LLVM_DEBUG(dbgs() << PC.getOutermostLoop()->getNumber() << "\n";);

  if (!isOptVarPredNeeded(PC)) {
    // Needed to lit-test cases
    LLVM_DEBUG(PC.getOutermostLoop()->dump());

    ProfitablityAndLegalityChecker Checker(HIRF, HASA, DDA,
                                           PC.getOutermostLoop(), FuncName);
    bool FoundLegalAndProfitableCand = Checker.run();
    if (FoundLegalAndProfitableCand) {
      return doTransformation(Checker.getInnermostLoopToDimInfos(),
                              Checker.getInnermostLoopToRepRef(),
                              Checker.getOutermostLoop(), DDA, FuncName);
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
  ProfitablityAndLegalityChecker Checker(
      HIRF, HASA, DDA, OutLoops[OutLoops.size() - 1], FuncName);
  bool FoundLegalAndProfitableCand = Checker.run();
  if (FoundLegalAndProfitableCand) {
    return doTransformation(Checker.getInnermostLoopToDimInfos(),
                            Checker.getInnermostLoopToRepRef(),
                            Checker.getOutermostLoop(), DDA, FuncName);
  }

  return false;
}
} // namespace

PreservedAnalyses
HIRInterLoopBlockingPass::run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM) {
  driver(AM.getResult<HIRFrameworkAnalysis>(F),
         AM.getResult<HIRArraySectionAnalysisPass>(F),
         AM.getResult<HIRDDAnalysisPass>(F), F);
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
                  getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(), F);
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
