//===- HIRMVForVariableStride.cpp - Multiversion for variable Stride -==//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements multiversioning of the loops with variable strides.
// checking if variable strides are actually unit-strided.
//
// Useful for assumed shape arrays of Fortran.
// e.g. cmplrllvm-10397
// SUBROUTINE step2d_tile (zeta, h, L1,L2,U1,U2, M1,M2)
//  integer L1,L2,U1,U2,M1,M2
//  real*8  zeta (L1:, L2:, :)     !assumed shape array
//  real*8  h (L1:, L2:)           !assumed shape array
//  real*8  Drhs (L1:U1, L2:U2  )
//
//  DO j=-1+MAX(2,M1),M2
//     DO i=-2+M1, M2
//        Drhs(i,j)=zeta(i,j,M1)+h(i,j)
//     END DO
//  END DO
//
// This transformation MV the two-level nested loop based on strides of assumed
// shape arrays.
//
// if (stride1 == 8 && stride2 == 8) {
//  DO j=-1+MAX(2,M1),M2
//     DO i=-2+M1, M2
//        Drhs(i,j)=zeta(i,j,M1)+h(i,j)
//     END DO
//  END DO
// } else {
//  DO j=-1+MAX(2,M1),M2
//     DO i=-2+M1, M2
//        Drhs(i,j)=zeta(i:stride1,j,M1)+h(i:stride2,j)
//     END DO
//  END DO
// }
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRMVForVariableStride.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeIterator.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-mv-variable-stride"
#define OPT_DESCR "HIR Multiversioning for variable stride"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESCR "."));

static cl::opt<int>
    MaxNumPredsAllowed(OPT_SWITCH "-max-num-preds", cl::init(8), cl::Hidden,
                       cl::desc("Maximum number of predicates allowd for a "
                                "candidate to be MVed through " OPT_DESCR "."));

STATISTIC(LoopsMultiversioned,
          "Number of innermost loops multiversioned by MV for variable stride");
STATISTIC(OuterLoopsMultiversioned,
          "Number of Outer loops multiversioned by MV for variable stride");

namespace {

/// Structure used to hoist the multi-versioning condition (i.e. if-stmt)
/// to the outer-most loop possible.
/// A node of a tree is a loop. A child node of a node(loop) is a immediate
/// child loop of the loop.
/// A leaf node is an innermost loop, which is a candidate loop of a
/// multi-versioning.
class LoopTreeForMV {
public:
  typedef int LoopIDTy;

public:
  /// \p InnermostLoops contains the candidate innermost loops of MV.
  /// These are leaf nodes used to construct a tree.
  /// \p Reg is the HLRegion where these innermost candidates are contained.
  LoopTreeForMV(ArrayRef<HLLoop *> InnermostLoops, const HLRegion &Reg)
      : InnermostLoops(InnermostLoops), LoopIDCounter(-1) {

    int Size = 0;

    // Count the number of loops to use it for sizes of tree structures
    // (i.e. upper bound of the number of nodes)
    ForEach<const HLLoop>::visitRange(Reg.child_begin(), Reg.child_end(),
                                      [&Size](const HLLoop *Lp) { Size++; });

    assert(Size);

    ID2Loop.resize(Size);
    NumChildren.resize(Size);
    ParentLoopID.resize(Size);

    for (int I = 0; I < Size; I++) {
      // Initialize parent with -1, denoting past the end(root).
      ParentLoopID[I] = -1;
      NumChildren[I] = 0;
    }

    buildTreeByTrackingAncestors();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print() const;
#endif

  /// Given \p Loop, a candidate of the multiversion,
  /// returns its lowest ancestor without any other child
  /// other than an ancestor of \p Loop.
  /// \p Loop itself, can be returned as its own valid lowest ancestor.
  /// e.g.
  /// For the following loop structure, where L3 is the only MV candidate
  /// getValidLowestAncestor(L3) will return L1.
  /// do L1
  ///    do L2
  ///      do L3 // MV cand.
  ///      end do
  ///    end do
  ///    do L4 // innermost loop but not a MV cand.
  ///    end do
  /// end do
  /// The loop tree built from the above struct will look like
  /// L1 <-- L2 <-- L3
  /// L1 is the lowest ancestor without any other child than L2, L3's ancestor.
  /// On the other hand if L4 were also a MV cand,
  /// getValidLowestAncestor(L1) would have returned L2.
  /// the loop tree built from the above struct will look like
  /// L1 <-- L2 <-- L3
  ///    ^-- L4
  /// L2 is the lowest ancestor without any other child than L3.
  HLLoop *getValidLowestAncestor(HLLoop *Loop) const;

private:
  /// Supposed to be called after buildTreeByTrackingAncestors() are done.
  int getTotalNumLoops() const { return LoopIDCounter + 1; }
  /// Build a tree starting for leaf nodes.
  void buildTreeByTrackingAncestors();

  /// Increase LoopIDCounter and populate LoopID and Loop mappings.
  LoopIDTy addLoop(HLLoop *Lp);

  /// loops to be multi-versioned.
  ArrayRef<HLLoop *> InnermostLoops;
  /// A counter to be used to assigning IDs to loops.
  LoopIDTy LoopIDCounter;

  /// mappings between HLLoop* and ID
  SmallVector<HLLoop *, 8> ID2Loop;
  DenseMap<HLLoop *, LoopIDTy> Loop2ID;

  /// For a loop ID, how many children it has.
  SmallVector<int, 8> NumChildren;
  /// For a loop ID, denotes its immediate parent loop id.
  SmallVector<LoopIDTy, 8> ParentLoopID;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void LoopTreeForMV::print() const {
  dbgs() << "Num Loops: " << getTotalNumLoops() << "\n";

  for (int I = 0, E = ID2Loop.size(); I < E; I++) {
    dbgs() << " ID: " << I << " " << ID2Loop[I] << "\n";
  }
  dbgs() << "\n";

  for (auto &KV : Loop2ID) {
    dbgs() << "L: " << KV.first << " " << KV.second << "\n";
  }
  dbgs() << "\n";

  dbgs() << "NumChildren: \n";
  for (auto I : NumChildren)
    dbgs() << I << " ";
  dbgs() << "\n";

  dbgs() << "ParentLoopID: \n";
  for (auto I : ParentLoopID)
    dbgs() << I << " ";
  dbgs() << "\n";
}
#endif

LoopTreeForMV::LoopIDTy LoopTreeForMV::addLoop(HLLoop *Lp) {
  LoopIDCounter++;
  ID2Loop[LoopIDCounter] = Lp;
  Loop2ID[Lp] = LoopIDCounter;

  return LoopIDCounter;
}

HLLoop *LoopTreeForMV::getValidLowestAncestor(HLLoop *Loop) const {

  // ScanParentLoopID
  // Loop (MyID) does not have the case NumChildren[Id] > 1,
  // because it is a leaf (i.e. innermost loop).
  // Thus, the loop below iterates at least once.
  LoopIDTy MyID = Loop2ID.find(Loop)->second;
  LoopIDTy Prev = MyID;
  for (LoopIDTy Id = MyID; Id != -1; Id = ParentLoopID[Id]) {
    if (NumChildren[Id] > 1) {
      break;
    }
    Prev = Id;
  }

  return ID2Loop[Prev];
}

void LoopTreeForMV::buildTreeByTrackingAncestors() {
  for (auto *Lp : InnermostLoops) {
    addLoop(Lp);

    // Track parent until its parent marked already found.
    HLLoop *ParentLp = Lp->getParentLoop();
    while (ParentLp) {

      LoopIDTy MyID = LoopIDCounter;

      auto It = Loop2ID.find(ParentLp);
      if (It != Loop2ID.end()) {
        LoopIDTy ParentID = It->second;
        ParentLoopID[MyID] = ParentID;
        NumChildren[ParentID]++;

        break;
      }

      LoopIDTy ParentID = addLoop(ParentLp);
      ParentLoopID[MyID] = ParentID;
      NumChildren[ParentID]++;
      ParentLp = ParentLp->getParentLoop();
    }
  }

  assert((Loop2ID.size() == ID2Loop.size()) &&
         ((int)Loop2ID.size() == getTotalNumLoops()));

  LLVM_DEBUG(print());
}

class HIRMVForVariableStride {
  typedef SmallVector<HLLoop *, 8> LoopSetTy;
  typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

public:
  HIRMVForVariableStride(HIRFramework &HIRF) : HIRF(HIRF) {}

  /// This pass only cares about the first dimension of a memref.
  static unsigned getInnermostDimNum() { return 1; }

  /// Returens StrideCE of a Ref this pass works on.
  static const CanonExpr *getStrideCE(const RegDDRef *Ref) {
    return Ref->getDimensionStride(getInnermostDimNum());
  }

  static CanonExpr *getStrideCE(RegDDRef *Ref) {
    return Ref->getDimensionStride(getInnermostDimNum());
  }

  /// Returns an index CE this pass works on.
  static const CanonExpr *getIndexCE(const RegDDRef *Ref) {
    return Ref->getDimensionIndex(HIRMVForVariableStride::getInnermostDimNum());
  }

  static CanonExpr *getDimensionLower(RegDDRef *Ref) {
    return Ref->getDimensionLower(getInnermostDimNum());
  }

  bool run();

private:
  class Analyzer {
  public:
    Analyzer(LoopSetTy &Candidates) : Candidates(Candidates) {}

    /// Check if Loop is a MV candidate and if so collect it.
    /// \p Loop has to an innermost loop.
    bool checkAndAddIfCandidate(HLLoop *Loop);
    bool hasCandidates() { return Candidates.size() > 0; }

    static bool hasVariableStride(const RegDDRef *Ref);

  private:
    /// Set of candidate innermost loops
    LoopSetTy &Candidates;
  };

  class MVTransformer {

    typedef std::pair<CanonExpr *, int64_t> StrideAndSizeTy;

  public:
    MVTransformer(const LoopSetTy &Candidates, const HLRegion &Reg)
        : Candidates(Candidates), LoopStructure(Candidates, Reg) {}

    bool rewrite();

  private:
    /// Update Stride CEs to a valid constant
    void updateStrideCEs(HLLoop *Loop, ArrayRef<RegDDRef *> RefsToRewrite);

    /// Get the outermost loop to be MV given a candidate innermost \p Loop
    HLLoop *
    calcOutermostLoopToMV(HLLoop *Loop,
                          ArrayRef<StrideAndSizeTy> StrideAndConstSize) const;

    bool transformLoop(HLLoop *Loop, ArrayRef<RegDDRef *> RefsToRewrite);

    int64_t getDimensionElementSizeInByte(const RegDDRef *Ref) const {
      Type *DimTy = Ref->getDimensionElementType(
          HIRMVForVariableStride::getInnermostDimNum());
      return (Ref->getCanonExprUtils()).getTypeSizeInBytes(DimTy);
    }

  private:
    /// Set of all loops to be MVed. All of them are innermost loops.
    const LoopSetTy &Candidates;

    /// Helper struct to calculate the outer most loop to hoist
    /// multi-versioning condition.
    LoopTreeForMV LoopStructure;
  };

  HIRFramework &HIRF;
};
} // namespace

void HIRMVForVariableStride::MVTransformer::updateStrideCEs(
    HLLoop *Loop, ArrayRef<RegDDRef *> RefsToRewrite) {

  for (auto *Ref : RefsToRewrite) {
    int64_t Constant = getDimensionElementSizeInByte(Ref);
    CanonExpr *CE = HIRMVForVariableStride::getStrideCE(Ref);
    CE->clear();
    CE->setConstant(Constant);
    Ref->makeConsistent({});
  }
}

HLLoop *HIRMVForVariableStride::MVTransformer::calcOutermostLoopToMV(
    HLLoop *Loop, ArrayRef<StrideAndSizeTy> StrideAndConstSize) const {

  // Calc maxDefAtLevel for all strides to MV.
  unsigned MaxDefAtLevel = 0;
  for (auto StrideCEWithSize : StrideAndConstSize) {
    unsigned DefAtLevel = StrideCEWithSize.first->getDefinedAtLevel();
    if (MaxDefAtLevel < DefAtLevel)
      MaxDefAtLevel = DefAtLevel;
  }

  // Get the lowest level of the loop it can go
  HLLoop *OuterLoopToMV = LoopStructure.getValidLowestAncestor(Loop);

  LLVM_DEBUG(dbgs() << "OutermostLevel from LoopTree: ");
  LLVM_DEBUG(dbgs() << OuterLoopToMV->getNestingLevel() << "\n");

  // Loop at the level of (MaxDefAtLevel + 1) is the Lowest level
  // that can be MVed.
  if (MaxDefAtLevel + 1 > OuterLoopToMV->getNestingLevel()) {
    OuterLoopToMV = Loop->getParentLoopAtLevel(MaxDefAtLevel + 1);
  }

  return OuterLoopToMV;
}

bool HIRMVForVariableStride::MVTransformer::transformLoop(
    HLLoop *Loop, ArrayRef<RegDDRef *> RefsToRewrite) {

  // Collect LHS and RHS of Pred of If
  // Also calculate the maximum (deepest) defined at level
  // of strideCEs
  SmallVector<StrideAndSizeTy, 8> StrideAndConstSize;
  for (auto *Ref : RefsToRewrite) {
    StrideAndConstSize.push_back(
        std::make_pair(HIRMVForVariableStride::getStrideCE(Ref),
                       getDimensionElementSizeInByte(Ref)));
  }

  // Remove redundant checks.
  std::sort(StrideAndConstSize.begin(), StrideAndConstSize.end(),
            [](const StrideAndSizeTy &P1, const StrideAndSizeTy &P2) {
              // When returns true, they are never same.
              if (CanonExprUtils::compare(P1.first, P2.first))
                return true;

              if (!CanonExprUtils::areEqual(P1.first, P2.first))
                return false;

              // tie-breaking
              if (P1.second < P2.second)
                return true;

              return false;
            });

  auto Last =
      std::unique(StrideAndConstSize.begin(), StrideAndConstSize.end(),
                  [](const StrideAndSizeTy &P1, const StrideAndSizeTy &P2) {
                    return P1.second == P2.second &&
                           CanonExprUtils::areEqual(P1.first, P2.first);
                  });

  StrideAndConstSize.erase(Last, StrideAndConstSize.end());

  int NumPreds = StrideAndConstSize.size();
  if (NumPreds > MaxNumPredsAllowed)
    return false;

  // Create If-stmt with Preds
  DDRefUtils &DRU = Loop->getDDRefUtils();
  // makeConsistent updates SB later
  RegDDRef *LHS = DRU.createScalarRegDDRef(
      GenericRvalSymbase, StrideAndConstSize.front().first->clone());
  HLIf *If = Loop->getHLNodeUtils().createHLIf(
      PredicateTy::ICMP_EQ, LHS,
      DRU.createConstDDRef(LHS->getDestType(),
                           StrideAndConstSize.front().second));
  for (auto Pair : make_range(std::next(StrideAndConstSize.begin()),
                              StrideAndConstSize.end())) {
    RegDDRef *LHS =
        DRU.createScalarRegDDRef(GenericRvalSymbase, Pair.first->clone());
    If->addPredicate(PredicateTy::ICMP_EQ, LHS,
                     DRU.createConstDDRef(LHS->getDestType(), Pair.second));
  }

  // Find the outermost loop enclosing Loop to MV
  HLLoop *OuterLoopToMV = calcOutermostLoopToMV(Loop, StrideAndConstSize);

  // Actual MV of OuterLoop
  if (OuterLoopToMV->hasPreheader()) {
    OuterLoopToMV->extractPreheaderAndPostexit();
  }

  // Multiversioned loop is not the innermost loop
  if (OuterLoopToMV != Loop)
    OuterLoopsMultiversioned++;

  // TODO: invalidation might be called redundantly?
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(OuterLoopToMV);
  HLNodeUtils::insertAfter(OuterLoopToMV, If);
  HLNodeUtils::insertAsFirstElseChild(If, OuterLoopToMV->clone());
  HLNodeUtils::moveAsFirstThenChild(If, OuterLoopToMV);

  // Consistency of if-conditions
  for (auto PI = If->pred_begin(), E = If->pred_end(); PI != E; PI++) {
    If->getPredicateOperandDDRef(PI, true)->makeConsistent(
        RefsToRewrite, OuterLoopToMV->getNestingLevel() - 1);
    If->getPredicateOperandDDRef(PI, false)->makeConsistent(
        RefsToRewrite, OuterLoopToMV->getNestingLevel() - 1);
  }

  // Update Loop's body
  updateStrideCEs(Loop, RefsToRewrite);

  return true;
}

bool HIRMVForVariableStride::MVTransformer::rewrite() {

  bool Changed = false;
  for (auto *InnermostLp : Candidates) {

    // Collect all MemRefs with variable strides
    SmallVector<RegDDRef *, 8> RefsToRewrite;
    ForEach<RegDDRef>::visitRange(
        InnermostLp->child_begin(), InnermostLp->child_end(),
        [&RefsToRewrite](RegDDRef *Ref) {
          if (Ref->isMemRef() &&
              HIRMVForVariableStride::Analyzer::hasVariableStride(Ref))
            RefsToRewrite.push_back(Ref);
        });

    Changed = transformLoop(InnermostLp, RefsToRewrite) || Changed;
    if (Changed)
      LoopsMultiversioned++;
  }

  return Changed;
}

bool HIRMVForVariableStride::Analyzer::hasVariableStride(const RegDDRef *Ref) {

  const CanonExpr *StrideCE = HIRMVForVariableStride::getStrideCE(Ref);
  int64_t ConstStride;
  if (StrideCE->isIntConstant(&ConstStride))
    return false;

  return true;
}

bool HIRMVForVariableStride::Analyzer::checkAndAddIfCandidate(
    HLLoop *InnermostLoop) {

  MemRefGatherer::VectorTy Refs;
  MemRefGatherer::gatherRange(InnermostLoop->child_begin(),
                              InnermostLoop->child_end(), Refs);

  bool FoundVariableStride = false;
  for (auto *Ref : Refs) {
    if (HIRMVForVariableStride::getStrideCE(Ref)->isNonLinear() ||
        HIRMVForVariableStride::getDimensionLower(Ref)->isNonLinear())
      return false;

    if (!hasVariableStride(Ref))
      continue;

    const CanonExpr *IndexCE = getIndexCE(Ref);

    if (IndexCE->isNonLinear())
      return false;

    if (IndexCE->getDenominator() != 1)
      return false;
    int64_t Coeff = 0;
    unsigned BlobIndex = InvalidBlobIndex;
    IndexCE->getIVCoeff(InnermostLoop->getNestingLevel(), &BlobIndex, &Coeff);
    if (Coeff != 1 || BlobIndex != InvalidBlobIndex)
      return false;

    FoundVariableStride = true;
  }

  if (!FoundVariableStride)
    return false;

  Candidates.push_back(InnermostLoop);
  return true;
}

bool HIRMVForVariableStride::run() {
  if (DisablePass) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIRMVForVariableStride for function: "
                    << HIRF.getFunction().getName() << "\n");

  bool Changed = false;
  for (auto &Reg : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    // Process one region by one.

    LoopSetTy InnermostLoops;
    Analyzer MVAnalyzer(InnermostLoops);
    // TODO: Get it to skip the innermost loop body either through
    //       gatherInnermostLoop or adding such featrue to
    //       HLPreOrderIterator
    for (auto Node :
         make_range(HLPreOrderIterator::begin(&(cast<HLRegion>(Reg))),
                    HLPreOrderIterator::end(&(cast<HLRegion>(Reg))))) {
      // Collect all candidate innermost loops for a region
      if (!isa<HLLoop>(Node))
        continue;
      if (!cast<HLLoop>(Node)->isInnermost())
        continue;

      MVAnalyzer.checkAndAddIfCandidate(cast<HLLoop>(Node));
    }

    if (!MVAnalyzer.hasCandidates())
      continue;

    // For a list of loops in a region, do multiversion.
    // We collect information about all loops in region before
    // do actual transformation of each loop to get some help
    // in hoisting MV conditions to outermost loop possible.
    Changed =
        MVTransformer(InnermostLoops, cast<HLRegion>(Reg)).rewrite() || Changed;
  }

  return Changed;
}

PreservedAnalyses
HIRMVForVariableStridePass::run(llvm::Function &F,
                                llvm::FunctionAnalysisManager &AM) {
  HIRMVForVariableStride(AM.getResult<HIRFrameworkAnalysis>(F)).run();
  return PreservedAnalyses::all();
}

class HIRMVForVariableStrideLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRMVForVariableStrideLegacyPass() : HIRTransformPass(ID) {
    initializeHIRMVForVariableStrideLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRMVForVariableStride(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR())
        .run();
  }
};

char HIRMVForVariableStrideLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRMVForVariableStrideLegacyPass, OPT_SWITCH, OPT_DESCR,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRMVForVariableStrideLegacyPass, OPT_SWITCH, OPT_DESCR,
                    false, false)

FunctionPass *llvm::createHIRMVForVariableStridePass() {
  return new HIRMVForVariableStrideLegacyPass();
}
