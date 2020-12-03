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
    if (ConvertFromUseToDef && MaintainedAsDef ||
        ConvertFromDefToUse && MaintainedAsUse) {
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

    auto compareRefs = [GetLoadRval, DDG, LCA](
                           const CanonExpr *CE1, const CanonExpr *CE2,
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

      if (!LoadRval1) {
        printMarker("!LoadRval1\n Ref1, Ref2: ", {Ref1, Ref2}, true);
        printMarker("BRef1: ", {BRef1}, true);
        printMarker("CE1, CE2: ", {CE1, CE2}, true);
        printMarker("Ref1 Loop, Ref2 Loop, LCA: ",
                    {Ref1->getParentLoop(), Ref2->getParentLoop(), LCA}, true);

        return false;
      }

      Index = CE2->getSingleBlobIndex();
      const BlobDDRef *BRef2 = Ref2->getBlobDDRef(Index);
      const RegDDRef *LoadRval2 = GetLoadRval(BRef2);

      if (!LoadRval2) {
        printMarker("!LoadRval2\n Ref1, Ref2: ", {Ref1, Ref2}, true, true);
        printMarker("BRef1, BRef2: ", {BRef1, BRef2}, true);
        printMarker("Ref1 Loop, Ref2 Loop, LCA: ",
                    {Ref1->getParentLoop(), Ref2->getParentLoop(), LCA}, true);

        return false;
      }

      if (!DDRefUtils::areEqual(LoadRval1, LoadRval2)) {
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

  bool checkStructure(HLLoop *Loop) {
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
        IA.couldBeAMember(DefinedBasePtr, ReadOnlyBasePtr, DDG, {LCA});

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
  void reset() {

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

    bailOut();

    FoundGoodCand = true;
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

    // Legality check: check if any of ReadOnlyBasePtr is
    // in DefinedBasePtr.
    if (llvm::any_of(ReadOnlyBasePtr, [this](unsigned BasePtrIndex) {
          return this->DefinedBasePtr.count(BasePtrIndex);
        })) {
      return false;
    }

    return true;
  }

  LoopToDimInfoTy getInnermostLoopToDimInfos() {
    return InnermostLoopToDimInfos;
  }

  LoopToConstRefTy getInnermostLoopToRepRef() { return InnermostLoopToRepRef; }

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
    bool FoundLegalAndProfitableCand =
        ProfitablityAndLegalityChecker(HIRF, HASA, DDA, PC.getOutermostLoop(),
                                       FuncName)
            .run();

    return FoundLegalAndProfitableCand;
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
  bool FoundLegalAndProfitableCand =
      ProfitablityAndLegalityChecker(HIRF, HASA, DDA,
                                     OutLoops[OutLoops.size() - 1], FuncName)
          .run();

  return FoundLegalAndProfitableCand;
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
