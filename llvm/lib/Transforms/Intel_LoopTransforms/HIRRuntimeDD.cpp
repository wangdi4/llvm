//===- HIRRuntimeDD.cpp - Implements Multiversioning for Runtime DD -=========//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass for the runtime data dependency multiversioning.
//
// The idea of the pass is to insert runtime checks to make sure that memory
// accesses do not overlap.
//
// 1) For every innermost loop it collects all memory references and
// group them, so in every group the references differ only by a constant.
//
//          BEGIN REGION { }
//<28>         + DO i1 = 0, zext.i32.i64((-1 + %M)), 1   <DO_LOOP>
//<4>          |   %2 = (%a)[i1 + -1];
//<6>          |   %3 = (%a)[i1];
//<10>         |   %4 = (%a)[i1 + 1];
//<13>         |   %5 = (%b)[i1];
//<16>         |   %6 = (%c)[i1];
//<20>         |   (%a)[i1 + sext.i32.i64(%N)] = %2 + %3 + %4 + %5 + %6;
//<22>         |   (%c)[i1 + -1] = 0;
//<28>         + END LOOP
//          END REGION
//
// For this example the following groups will be created:
// Group 0 {sb: 29} contains:
//         (%a)[i1 + -1] -> isWrite:0
//         (%a)[i1] -> isWrite:0
//         (%a)[i1 + 1] -> isWrite:0
// Group 1 {sb: 29} contains:
//         (%a)[i1 + sext.i32.i64(%N)] -> isWrite:1
// Group 2 {sb: 29} contains:
//         (%b)[i1] -> isWrite:0
// Group 3 {sb: 29} contains:
//         (%c)[i1 + -1] -> isWrite:1
//         (%c)[i1] -> isWrite:0
//
// 2) These groups are represented as "IVSegments" in the code. All references
// are sorted inside these groups and there are lower and upper bound of the
// segment.
//
// There will be following segments:
//  1. [(%a)[i1 + -1], (%a)[i1 + 1]]                              - isWrite:0
//  2. [(%a)[i1 + sext.i32.i64(%N)], (%a)[i1 + sext.i32.i64(%N)]] - isWrite:1
//  3. [(%b)[i1], (%b)[i1]]                                       - isWrite:0
//  4. [(%c)[i1 + -1], (%c)[i1]]                                  - isWrite:1
//
// 3) Now the number of required tests can be estimated and if there are too
// many of them (> MaximumNumberOfTests) we just give up and skip the loop.
//
// 4) We have to check every pair of segments that includes an LVAL and
// has the same symbase.
//
//
// The loop IVs, inside lower and upper bounds of the IVSegment,
// are replaced by lower and upper bounds canon expressions of the loop.
// Then we have "Segments" - memory regions accessed inside a loop.
//
//  1. [%a[-1], %a[(-1 + %M) + 1]]
//  2. [%a[%N], %a[(-1 + %M) + %N]]
//  3. [%b[0],  %b[-1 + %M]]
//  4. [%c[-1], %c[-1 + %M]]
//
// 5) The transformation inserts a number of HLIf nodes to check for segment
// intersection:
//
// &(%a[(-1 + %M) + 1])  >= &(%a[%N]) && &(%a[(-1 + %M) + %N]) >= &(%a[-1]))
// &(%a[(-1 + %M) + 1])  >= &(%c[-1]) && &(%c[(-1 + %M)])      >= &(%a[-1]))
// &(%a[(-1 + %M) + %N]) >= &(%b[0])  && &(%b[(-1 + %M)])      >= &(%a[%N]))
// &(%a[(-1 + %M) + %N]) >= &(%c[-1]) && &(%c[(-1 + %M)])      >= &(%a[%N]))
// &(%b[(-1 + %M)])      >= &(%c[-1]) && &(%c[(-1 + %M)])      >= &(%b[0]))
//
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRRuntimeDD.h"

#include "llvm/Pass.h"

#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNodeMapper.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "HIRRuntimeDDImpl.h"

#include <memory>

#define OPT_SWITCH "hir-runtime-dd"
#define OPT_DESCR "HIR RuntimeDD Multiversioning"
#define DEBUG_TYPE OPT_SWITCH

#define LLVM_DEBUG_DDG(X) DEBUG_WITH_TYPE(OPT_SWITCH "-ddg", X)

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::runtimedd;

static cl::opt<bool> DisableRuntimeDD("disable-" OPT_SWITCH, cl::init(false),
                                      cl::Hidden,
                                      cl::desc("Disable " OPT_DESCR "."));

static cl::opt<bool> EnableStructSupport("enable-" OPT_SWITCH "-structs",
                                         cl::init(true), cl::Hidden,
                                         cl::desc("Enable " OPT_DESCR
                                                  " struct support."));

static cl::opt<bool>
    DisableCostModel("disable-" OPT_SWITCH "-cost-model", cl::init(false),
                     cl::Hidden, cl::desc("Disable " OPT_DESCR " cost model."));

static cl::opt<unsigned>
    MaximumNumberOfTests(OPT_SWITCH "-max-tests",
                         cl::init(ExpectedNumberOfTests), cl::Hidden,
                         cl::desc("Maximum number of runtime tests for loop."));

// This will count both innermost and outer transformations
STATISTIC(LoopsMultiversioned, "Number of loops multiversioned by runtime DD");

STATISTIC(OuterLoopsMultiversioned,
          "Number of outer loops multiversioned by runtime DD");

struct HIRRuntimeDD::MemoryAliasAnalyzer final : public HLNodeVisitorBase {
  SmallVector<LoopContext, 16> LoopContexts;
  HIRRuntimeDD &RDD;

  MemoryAliasAnalyzer(HIRRuntimeDD &RDD) : RDD(RDD), SkipNode(nullptr) {}

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

  void visit(HLLoop *Loop) {
    LoopContext Context;
    LLVM_DEBUG(dbgs() << "Runtime DD for loop " << Loop->getNumber() << ":\n");

    RuntimeDDResult Result = RDD.computeTests(Loop, Context);

    if (Result == OK) {
      SkipNode = Loop;
      LoopContexts.push_back(std::move(Context));
    } else if (Loop->isInnermost() || analyzedRefs(Result)) {
      SkipNode = Loop;
    }

    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: [RTDD] Loop " << Loop->getNumber()
                      << ": " << HIRRuntimeDD::getResultString(Result) << "\n");
  }

  bool skipRecursion(const HLNode *N) const { return N == SkipNode; }

  bool analyzedRefs(RuntimeDDResult Result) const {
    // If we have analyzed the memrefs inside this loop, we can skip recursing
    // into it. The following result types are the ones where we haven't
    // analyzed the memrefs.
    return (Result != ALREADY_MV) && (Result != NON_DO_LOOP) &&
           (Result != NON_PROFITABLE) && (Result != NON_PERFECT_LOOPNEST) &&
           (Result != NON_NORMALIZED_BLOB_IV_COEFF);
  }

private:
  const HLNode *SkipNode;
};

IVSegment::IVSegment(const RefGroupTy &Group, bool IsWrite) : IsWrite(IsWrite) {
  // Allow dummy IVSegments based on empty groups.
  if (Group.empty()) {
    BaseCE = nullptr;
    return;
  }

  Lower.reset(Group.front()->clone());
  Upper.reset(Group.back()->clone());
  BaseCE = Lower->getBaseCE();

  assert(CanonExprUtils::areEqual(BaseCE, Upper->getBaseCE()) &&
         "Unexpected group. Left and Right refs should have the same base.");

#ifndef NDEBUG
  int64_t DiffValue;
  bool IsConst = DDRefUtils::getConstByteDistance(Upper.get(), Lower.get(),
                                                  &DiffValue, false);
  assert(IsConst && " CanonExpr difference failed.");
  // assert(DiffValue >= 0 && "Segment wrong direction");
  // TODO: Fix the following concern.
  // In case of refs with non-constant stride %p[%m][1]
  //  distance(
  //    (%p)[i1 + 1][i2],
  //    (%p)[i1][i2 + 1]
  //  )
  // The difference is not really a constant but equal to (%m - 1).
#endif
}

IVSegment::IVSegment(IVSegment &&Segment)
    : Lower(std::move(Segment.Lower)), Upper(std::move(Segment.Upper)),
      BaseCE(std::move(Segment.BaseCE)), IsWrite(std::move(Segment.IsWrite)) {

  Segment.Lower = nullptr;
  Segment.Upper = nullptr;
}

// Clone bounds and set isAddressOf flag.
Segment IVSegment::genSegment() const {
  auto *Ref1 = Lower->clone();
  auto *Ref2 = Upper->clone();

  Ref1->setAddressOf(true);
  Ref2->setAddressOf(true);
  return Segment(Ref1, Ref2);
}

static unsigned getMinMaxZeroBlob(BlobUtils &BU, unsigned Index,
                                  bool IsMinBlob) {
  BlobTy Blob = BU.getBlob(Index);

  // We don't have to keep zero blob in the table.
  BlobTy ZeroBlob = BU.createBlob(0, Blob->getType(), false, nullptr);

  unsigned MinMaxBlobIndex;

  if (IsMinBlob) {
    BU.createSMinBlob(Blob, ZeroBlob, true, &MinMaxBlobIndex);
  } else {
    BU.createSMaxBlob(Blob, ZeroBlob, true, &MinMaxBlobIndex);
  }

  return MinMaxBlobIndex;
}

// Returns true if IV replacement succeeded, false if failed and {} if no
// replacement required.
static Optional<bool> replaceIVByBound(CanonExpr *CE, const HLLoop *Loop,
                             const HLLoop *InnerLoop, bool IsLowerBound) {
  unsigned Level = Loop->getNestingLevel();

  unsigned IVBlobIndex;
  int64_t IVCoeff;
  CE->getIVCoeff(Level, &IVBlobIndex, &IVCoeff);

  if (IVCoeff == 0) {
    return {};
  }

  // Determine IV direction: C*B*i, get C and B signs.
  int64_t Direction = 1;
  if (IVBlobIndex != InvalidBlobIndex) {
    // IVBlobExpr is a helper CE to use HLNodeUtils::isKnownNegative
    std::unique_ptr<CanonExpr> IVBlobExpr(
        CE->getCanonExprUtils().createExtCanonExpr(
            CE->getSrcType(), CE->getDestType(), CE->isSExt()));
    IVBlobExpr->addBlob(IVBlobIndex, IVCoeff);

    // Now we check if the sign of the IV blob is known.
    if (HLNodeUtils::isKnownNegative(IVBlobExpr.get(), InnerLoop)) {
      Direction *= -1;
    } else if (!HLNodeUtils::isKnownPositive(IVBlobExpr.get(), InnerLoop)) {
      // The blob sign is unknown.
      assert(Loop->isNormalized() && "Normalized loop is expected");

      // For the lower bound we replace an unknown blob %b with min(%b, 0)
      // and with max(%b, 0) for the upper bound.
      unsigned MinMaxBlobIndex =
          getMinMaxZeroBlob(CE->getBlobUtils(), IVBlobIndex, IsLowerBound);
      CE->setIVBlobCoeff(Level, MinMaxBlobIndex);
      Direction = 0;
    }
  } else {
    Direction *= IVCoeff;
  }

  // Get max reference depending on the direction
  const RegDDRef *Bound;
  if (Direction == 0) {
    // Direction == 0 means that the IV has unknown blob. The result segment
    // bounds will be:
    // Lower = UB * smin(%b, 0);
    // Upper = UB * smax(%b, 0);
    Bound = Loop->getUpperDDRef();
  } else {
    auto *LowerRef = Loop->getLowerDDRef();
    auto *UpperRef = Loop->getUpperDDRef();

    // If IV direction is negative UpperRef will correspond to the lowest
    // memory address.
    if (IsLowerBound) {
      Bound = (Direction > 0) ? LowerRef : UpperRef;
    } else {
      Bound = (Direction > 0) ? UpperRef : LowerRef;
    }
  }
  assert(Bound->isTerminalRef() && "DDRef should be a terminal reference.");

  const CanonExpr *BoundCE = Bound->getSingleCanonExpr();

  // The relaxed mode is safe here as we know that upper bound is always non
  // negative
  if (BoundCE->isTrunc()) {
    // Truncations are not supported
    return false;
  }

  bool Ret = CanonExprUtils::replaceIVByCanonExpr(CE, Level, BoundCE,
                                                  Loop->isNSW(), true);
  if (!Ret) {
    return false;
  }

  CE->simplify(true, true);

  return true;
}

// The method replaces \p Loop IV with the appropriate bounds depending on
// \p IsLowerBound value.
static void replaceIVByBound(RegDDRef *Ref, const HLLoop *Loop,
                             const HLLoop *InnerLoop, bool IsLowerBound) {
  for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
    auto Ret = replaceIVByBound(CE, Loop, InnerLoop, IsLowerBound);

    assert((!Ret || *Ret) &&
           "Assuming replace will always succeed as we already checked if both "
           "are mergeable.");
    (void)Ret;
  }
}

static bool sortRefsInSingleGroup(RefGroupTy &Group) {
  for (int I = 0, E = Group.size() - 1; I < E; ++I) {
    if (!DDRefUtils::getConstByteDistance(Group[I], Group[I + 1], nullptr,
                                          false)) {
      return false;
    }
  }

  std::sort(Group.begin(), Group.end(), DDRefUtils::compareMemRefAddress);
  return true;
}

static bool sortRefsInGroups(RefGroupVecTy &Groups,
                             SmallVectorImpl<unsigned> &UnsortedGroups) {
  for (unsigned I = 0, E = Groups.size(); I < E; ++I) {
    if (!sortRefsInSingleGroup(Groups[I])) {
      UnsortedGroups.push_back(I);
      continue;
    }
  }

  return UnsortedGroups.empty();
}

RuntimeDDResult
IVSegment::isSegmentSupported(const HLLoop *OuterLoop,
                              const HLLoop *InnermostLoop) const {
  assert(!isEmpty());

  if (BaseCE->isNonLinear()) {
    return NON_LINEAR_BASE;
  }

  if (!Lower->getDestType()->isSized()) {
    return UNSIZED;
  }

  // We will be replacing every IV inside a RegDDRef: a[i+j+k][j][k]. So we have
  // to check all canon expressions against UB of every loop in loopnest.
  // We skip loops if its IV is absent.
  for (unsigned I = 1, NumDims = Lower->getNumDimensions(); I <= NumDims; ++I) {
    const CanonExpr *CE = Lower->getDimensionIndex(I);
    auto *LowerCE = Lower->getDimensionLower(I);
    auto *StrideCE = Lower->getDimensionStride(I);

    if (CE->isNonLinear() || CE->containsUndef() || LowerCE->isNonLinear() ||
        LowerCE->containsUndef() || StrideCE->isNonLinear() ||
        StrideCE->containsUndef()) {
      return NON_LINEAR_SUBS;
    }

    // TODO: Account for the sign of dimension stride.

    for (const HLLoop *LoopI = InnermostLoop,
                      *LoopE = OuterLoop->getParentLoop();
         LoopI != LoopE; LoopI = LoopI->getParentLoop()) {

      auto Level = LoopI->getNestingLevel();
      if (!CE->hasIV(Level)) {
        continue;
      }

      unsigned IVBlobIndex;
      int64_t IVConstCoeff;
      CE->getIVCoeff(Level, &IVBlobIndex, &IVConstCoeff);

      // Profitability check:
      // This pass helps Vectorization of inner loops and Loop Interchange
      // of loopnests.
      // For Vectorization we skip loops with non-unit stride DDRefs as it will
      // usually result in gather/scatter generation.
      // For Loop Interchange we treat them as profitable.
      if (!DisableCostModel && OuterLoop == InnermostLoop &&
          (!(IVConstCoeff == 1 ||
             // -1 is allowed for memcpy recognition
             (IVConstCoeff == -1 && OuterLoop->getNumChildren() <= 2)) ||
           IVBlobIndex != InvalidBlobIndex)) {
        return NON_PROFITABLE_SUBS;
      }

      const CanonExpr *UpperBoundCE = LoopI->getUpperCanonExpr();

      // Check if CE and UpperBoundCE are mergeable and check if UpperBoundCE
      // denominator equals one as we will not be able to replace IV with such
      // upper bound. This is because b*(x/d) != (b*x)/d.
      if ((UpperBoundCE->getDenominator() != 1 ||
           !CanonExprUtils::mergeable(CE, UpperBoundCE, true)) &&
          !UpperBoundCE->canConvertToStandAloneBlob()) {
        return UPPER_SUB_TYPE_MISMATCH;
      }
      assert((CanonExprUtils::mergeable(CE, LoopI->getLowerCanonExpr(), true) ||
              LoopI->getLowerCanonExpr()->canConvertToStandAloneBlob()) &&
             "Assuming that the Lower bound is also mergeable or can be "
             "represented as a blob if Upper is mergeable or can be represented"
             " as a blob");

      if (IVBlobIndex != InvalidBlobIndex) {
        std::unique_ptr<CanonExpr> IVBlobExpr(
            CE->getCanonExprUtils().createExtCanonExpr(
                CE->getSrcType(), CE->getDestType(), CE->isSExt()));

        IVBlobExpr->addBlob(IVBlobIndex, IVConstCoeff);

        if (!HLNodeUtils::isKnownPositiveOrNegative(IVBlobExpr.get(),
                                                    InnermostLoop) &&
            !LoopI->isNormalized()) {
          return NON_NORMALIZED_BLOB_IV_COEFF;
        }
      }
    }
  }

  return OK;
}

void IVSegment::makeConsistent(ArrayRef<const RegDDRef *> AuxRefs,
                               unsigned Level) {
  assert(!isEmpty());

  Lower->makeConsistent(AuxRefs, Level);
  Upper->makeConsistent(AuxRefs, Level);
}

// The method will replace IV @ Level inside segment bounds, depending on
// direction of IV, constant and blob coefficients. The result segment represent
// lower and upper address accessed inside a loopnest.
void IVSegment::replaceIVWithBounds(const HLLoop *Loop,
                                    const HLLoop *InnerLoop) {
  assert(!isEmpty());

  replaceIVByBound(Lower.get(), Loop, InnerLoop, true);
  replaceIVByBound(Upper.get(), Loop, InnerLoop, false);
}

#ifndef NDEBUG
const char *HIRRuntimeDD::getResultString(RuntimeDDResult Result) {
  switch (Result) {
  case OK:
    return "OK";
  case NO_OPPORTUNITIES:
    return "No opportunities";
  case NON_PERFECT_LOOPNEST:
    return "Non perfect loopnest/non innermost loop";
  case NON_LINEAR_BASE:
    return "The reference base is non linear";
  case NON_LINEAR_SUBS:
    return "One of the dimensions is non linear";
  case NON_CONSTANT_IV_STRIDE:
    return "Non constant IV stride is not supported";
  case SMALL_TRIPCOUNT:
    return "Small trip count loop is skipped";
  case ALREADY_MV:
    return "The loop is already multiversioned";
  case TOO_MANY_TESTS:
    return "Exceeded maximum number of tests";
  case UPPER_SUB_TYPE_MISMATCH:
    return "Upper bound/sub type mismatch";
  case NON_NORMALIZED_BLOB_IV_COEFF:
    return "Unknown Blob IV coeffs are not supported in non-normalized loops";
  case DELINEARIZATION_FAILED:
    return "Delinearization failed";
  case NON_DO_LOOP:
    return "Non DO loops are not supported";
  case UNROLL_PRAGMA_LOOP:
    return "Unroll pragma loops are not supported";
  case IVDEP_PRAGMA_LOOP:
    return "Loop has IVDEP pragma";
  case NON_PROFITABLE:
    return "Loop considered non-profitable";
  case NON_PROFITABLE_SUBS:
    return "Subscript multiversioning is non-profitable";
  case STRUCT_ACCESS:
    return "Struct refs not supported yet";
  case DIFF_ADDR_SPACE:
    return "Different address spaces";
  case UNSIZED:
    return "Ref type is unsized";
  case SIMD_LOOP:
    return "SIMD Loop";
  case UNKNOWN_MIN_MAX:
    return "Could not find MIN and MAX bounds";
  default:
    llvm_unreachable("Unexpected give up reason");
  }
}
#endif

bool HIRRuntimeDD::isProfitable(const HLLoop *Loop) {
  if (DisableCostModel) {
    return true;
  }

  const LoopStatistics &LS = HLS.getTotalLoopStatistics(Loop);

  return (!LS.hasCalls() && !LS.hasSwitches());
}

void HIRRuntimeDD::processLoopnest(const HLLoop *OuterLoop,
                                   const HLLoop *InnermostLoop,
                                   SmallVectorImpl<IVSegment> &IVSegments) {

  assert(InnermostLoop->isInnermost() &&
         "InnermostLoop is not an innermost loop");

  unsigned SegmentCount = IVSegments.size();

  SmallVector<const RegDDRef *, 6> AuxRefs;

  // Replace every IV in segments with upper and lower bounds
  for (const HLLoop *LoopI = InnermostLoop, *LoopE = OuterLoop->getParentLoop();
       LoopI != LoopE; LoopI = LoopI->getParentLoop()) {

    AuxRefs.push_back(LoopI->getLowerDDRef());
    AuxRefs.push_back(LoopI->getUpperDDRef());

    for (unsigned I = 0; I < SegmentCount; ++I) {
      if (IVSegments[I].isEmpty()) {
        continue;
      }

      IVSegments[I].replaceIVWithBounds(LoopI, InnermostLoop);
    }
  }

  for (unsigned I = 0; I < SegmentCount; ++I) {
    if (IVSegments[I].isEmpty()) {
      continue;
    }

    IVSegments[I].makeConsistent(AuxRefs, OuterLoop->getNestingLevel() - 1);
  }
}

static RuntimeDDResult isTestSupported(const RegDDRef *RefA,
                                       const RegDDRef *RefB) {
  assert(
      !CanonExprUtils::areEqual(RefA->getBaseCE(), RefB->getBaseCE(), true) &&
      "Unexpected test having two references with the same base");

  // Skip loops with different address space references.
  if (RefA->getPointerAddressSpace() != RefB->getPointerAddressSpace()) {
    return DIFF_ADDR_SPACE;
  }

  return OK;
}

static bool computeDelinearizationValidityConditions(
    RefGroupTy &Group, ArrayRef<BlobTy> Sizes, const HLLoop *OuterLoop,
    const HLLoop *InnerLoop, SmallVectorImpl<PredicateTuple> &Conditions) {
  if (Group.empty()) {
    // No extra checks are required if no delinearization happen.
    return true;
  }

  assert(!Sizes.empty() && "Empty sizes are unexpected");

  unsigned Level = OuterLoop->getNestingLevel();

  DDRefUtils &DRU = Group.front()->getDDRefUtils();
  BlobUtils &BU = DRU.getBlobUtils();

  RefGroupTy SortedRefs;
  SortedRefs.append(Group.begin(), Group.end());

  SmallSetVector<const RegDDRef *, 8> AuxRefs;

  for (int I = 0, E = Sizes.size() - 1; I < E; ++I) {
    // We already checked that subscripts may be compared.
    std::sort(SortedRefs.begin(), SortedRefs.end(),
              [&](const RegDDRef *A, const RegDDRef *B) {
                return CanonExprUtils::compare(A->getDimensionIndex(I + 1),
                                               B->getDimensionIndex(I + 1));
              });

    AuxRefs.insert(SortedRefs.back());

    // Generate:
    //   Size[i] > 1
    RegDDRef *DimSizeRef = DRU.createConstDDRef(Sizes[I]->getType(), 0);
    DimSizeRef->getSingleCanonExpr()->addBlob(BU.findOrInsertBlob(Sizes[I]), 1);
    DimSizeRef->makeConsistent(AuxRefs.getArrayRef(), Level - 1);
    RegDDRef *OneRef = DRU.createConstDDRef(Sizes[I]->getType(), 1);
    Conditions.emplace_back(DimSizeRef, CmpInst::ICMP_SGT, OneRef);

    // Generate:
    //   Subscript[i + 1] < Size[i]
    // TODO: need to handle Ref's lower bound

    const CanonExpr *MaxCE = SortedRefs.back()->getDimensionIndex(I + 1);
    RegDDRef *MaxRef =
        DRU.createScalarRegDDRef(GenericRvalSymbase, MaxCE->clone());

    // Replace IVs.
    for (const auto *Loop = InnerLoop, *LoopE = OuterLoop->getParentLoop();
         Loop != LoopE; Loop = Loop->getParentLoop()) {
      auto Ret = replaceIVByBound(MaxRef->getSingleCanonExpr(), Loop, InnerLoop,
                                  false);
      if (Ret) {
        if (*Ret) {
          AuxRefs.insert(Loop->getUpperDDRef());
        } else {
          return false;
        }
      }
    }

    // Subscripts may contain embedded cast even if they are constant.
    MaxRef->getSingleCanonExpr()->simplify(true, false);

    // MaxRef < DimSizeRef
    MaxRef->makeConsistent(AuxRefs.getArrayRef(), Level - 1);
    Conditions.emplace_back(MaxRef, CmpInst::ICMP_SLT, DimSizeRef->clone());
  }

  return true;
}

static RuntimeDDResult
tryDelinearization(const HLLoop *Loop, const HLLoop *InnermostLoop,
                   ArrayRef<unsigned> UnsortedGroupIndices,
                   RefGroupVecTy &Groups, RefGroupVecTy &DelinearizedGroups,
                   SmallVectorImpl<PredicateTuple> &PreConditions) {
  DelinearizedGroups.resize(Groups.size());
  SmallVector<PredicateTuple, ExpectedNumberOfTests> ValidityConditions;

  for (auto I : UnsortedGroupIndices) {
    auto &Group = Groups[I];
    auto &DelinearizedGroup = DelinearizedGroups[I];

    // Bail out if any ref has more than one dimension.
    if (std::any_of(Group.begin(), Group.end(), [](const RegDDRef *Ref) {
          return !Ref->isSingleCanonExpr();
        })) {
      return DELINEARIZATION_FAILED;
    }

    SmallVector<BlobTy, MaxLoopNestLevel> GroupSizes;
    if (!DDRefUtils::delinearizeRefs(Group, DelinearizedGroup, &GroupSizes)) {
      return DELINEARIZATION_FAILED;
    }

    // Try sorting second time after delinearization.
    if (!sortRefsInSingleGroup(DelinearizedGroup)) {
      return UNKNOWN_MIN_MAX;
    }

    if (!computeDelinearizationValidityConditions(DelinearizedGroup, GroupSizes,
                                                  Loop, InnermostLoop,
                                                  ValidityConditions)) {
      return DELINEARIZATION_FAILED;
    }
  }

  // Partition equivalent conditions (O(m*n)).
  for (auto CondI = ValidityConditions.begin(),
            CondE = ValidityConditions.end();
       CondI != CondE;) {
    CondI = std::partition(CondI, CondE, [CondI](const PredicateTuple &Cond) {
      return *CondI == Cond;
    });
  }

  // Remove duplicate conditions.
  ValidityConditions.erase(
      std::unique(ValidityConditions.begin(), ValidityConditions.end()),
      ValidityConditions.end());

  // Check for trivial conditions.
  bool IsTriviallyFalse = false;
  auto IsTrivialCondition = [&IsTriviallyFalse](PredicateTuple &Cond) {
    bool Result;
    if (HLNodeUtils::isKnownPredicate(Cond.Op1->getSingleCanonExpr(), Cond.Pred,
                                      Cond.Op2->getSingleCanonExpr(),
                                      &Result)) {

      if (!Result) {
        IsTriviallyFalse = true;
      }

      return true;
    }

    return false;
  };

  auto TrivialConditionsI = std::remove_if(
      ValidityConditions.begin(), ValidityConditions.end(), IsTrivialCondition);

  if (IsTriviallyFalse) {
    return DELINEARIZATION_FAILED;
  }

  ValidityConditions.erase(TrivialConditionsI, ValidityConditions.end());

  if (ValidityConditions.size() > MaximumNumberOfTests) {
    return TOO_MANY_TESTS;
  }

  PreConditions.append(ValidityConditions.begin(), ValidityConditions.end());
  return OK;
}

RuntimeDDResult HIRRuntimeDD::processDDGToGroupPairs(
    const HLLoop *Loop, MemRefGatherer::VectorTy &Refs,
    DenseMap<RegDDRef *, unsigned> &RefGroupIndex,
    SmallSetVector<std::pair<unsigned, unsigned>, ExpectedNumberOfTests> &Tests)
    const {
  DDGraph DDG = DDA.getGraph(Loop);
  LLVM_DEBUG_DDG(dbgs() << "[RTDD] Loop DDG:\n");
  LLVM_DEBUG_DDG(DDG.dump());

  // Process DDGraph to compute required tests between groups.
  for (RegDDRef *SrcRef : Refs) {
    unsigned GroupA = RefGroupIndex[SrcRef];

    for (const DDEdge *Edge : DDG.outgoing(SrcRef)) {
      assert(!Edge->isInput() && "Input edges are unexpected");

      RegDDRef *DstRef = cast<RegDDRef>(Edge->getSink());
      auto GroupBI = RefGroupIndex.find(DstRef);
      assert(GroupBI != RefGroupIndex.end() &&
             "Found dependence to unknown ref");
      unsigned GroupB = GroupBI->second;

      if (GroupA != GroupB) {
        auto IsSupported = isTestSupported(SrcRef, DstRef);
        if (IsSupported != OK) {
          return IsSupported;
        }

        auto TestPair = GroupA > GroupB ? std::make_pair(GroupB, GroupA)
                                        : std::make_pair(GroupA, GroupB);

        Tests.insert(TestPair);
      }
    }
  }

  return OK;
}

static void clearNotInvolvedGroups(
    RefGroupVecTy &Groups,
    SmallSetVector<std::pair<unsigned, unsigned>, ExpectedNumberOfTests>
        &Tests) {
  BitVector TestedGroups(Groups.size(), true);

  for (auto &Pair : Tests) {
    TestedGroups.reset(Pair.first);
    TestedGroups.reset(Pair.second);
  }

  for (unsigned GroupNo : TestedGroups.set_bits()) {
    Groups[GroupNo].clear();
  }
}

RuntimeDDResult HIRRuntimeDD::computeTests(HLLoop *Loop, LoopContext &Context) {
  RuntimeDDResult Ret = OK;
  Context.Loop = Loop;

  if (Loop->hasVectorizeIVDepPragma()) {
    return IVDEP_PRAGMA_LOOP;
  }

  if (Loop->getMVTag()) {
    return ALREADY_MV;
  }

  if (!Loop->isDo()) {
    return NON_DO_LOOP;
  }

  if (!isProfitable(Loop)) {
    return NON_PROFITABLE;
  }

  const HLLoop *InnermostLoop = Loop;
  if (!Loop->isInnermost() &&
      !HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop)) {
    return NON_PERFECT_LOOPNEST;
  }

  bool ConstantTripCount = true;
  uint64_t TotalTripCount = 1;

  for (const HLLoop *LoopI = InnermostLoop, *LoopE = Loop->getParentLoop();
       LoopI != LoopE; LoopI = LoopI->getParentLoop()) {

    if (LoopI->hasUnrollEnablingPragma()) {
      return UNROLL_PRAGMA_LOOP;
    }

    if (LoopI->isSIMD()) {
      return SIMD_LOOP;
    }

    uint64_t TripCount;
    if (LoopI->isConstTripLoop(&TripCount)) {
      // TODO: Max trip count estimation could be used for the small trip test.
      TotalTripCount *= TripCount;
    } else {
      ConstantTripCount = false;
    }
  }

  if (!DisableCostModel && ConstantTripCount &&
      TotalTripCount < SmallTripCountTest) {
    return SMALL_TRIPCOUNT;
  }

  RefGroupVecTy &Groups = Context.Groups;
  RefGroupVecTy DelinearizedGroups;

  auto DestroyDelinearizedGroups = make_scope_exit([&]() {
    if (DelinearizedGroups.empty()) {
      return;
    }

    for (auto &Group : DelinearizedGroups) {
      for (auto *Ref : Group) {
        Ref->getDDRefUtils().destroy(Ref);
      }
    }
  });

  // Gather references which are only inside a loop (no loop bounds,
  // pre-header and post-exit refs).
  MemRefGatherer::VectorTy Refs;
  MemRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(), Refs);
  LLVM_DEBUG(dbgs() << "[RTDD] Loop references:\n");
  LLVM_DEBUG(MemRefGatherer::dump(Refs));

  // Populate reference groups split by base blob index.
  // Populate a reference-to-group-number map.
  DenseMap<RegDDRef *, unsigned> RefGroupIndex;
  {
    DenseMap<unsigned, unsigned> GroupIndex;
    for (RegDDRef *Ref : Refs) {
      unsigned &GroupNo = GroupIndex[Ref->getBasePtrBlobIndex()];
      if (GroupNo == 0) {
        GroupNo = GroupIndex.size();
        Groups.emplace_back();
      }

      RefGroupIndex[Ref] = GroupNo - 1;
      Groups[GroupNo - 1].push_back(Ref);
    }
  }

  SmallSetVector<std::pair<unsigned, unsigned>, ExpectedNumberOfTests> Tests;
  Ret = processDDGToGroupPairs(Loop, Refs, RefGroupIndex, Tests);
  if (Ret != OK) {
    return Ret;
  }

  if (Tests.size() < 1) {
    return NO_OPPORTUNITIES;
  } else if (Tests.size() > MaximumNumberOfTests) {
    return TOO_MANY_TESTS;
  }

  // Empty groups not involved in tests.
  clearNotInvolvedGroups(Groups, Tests);

  bool DelinearizationRequired = false;
  SmallVector<unsigned, 8> UnsortedGroupIndices;
  if (!sortRefsInGroups(Groups, UnsortedGroupIndices)) {
    LLVM_DEBUG(dbgs() << "[RTDD] Could not find min and max address in groups. "
                         "Trying to delinearize refs.\n");

    // Try to delinearize.
    Ret = tryDelinearization(Loop, InnermostLoop, UnsortedGroupIndices, Groups,
                             DelinearizedGroups, Context.PreConditions);
    if (Ret != OK) {
      return Ret;
    }

    LLVM_DEBUG(
        dbgs() << "[RTDD] Delinearization done. Required pre-conditions: "
               << Context.PreConditions.size() << "\n");

    DelinearizationRequired = true;
  }

  unsigned GroupSize = Groups.size();

  auto GetGroupForChecks = [&](unsigned Index) -> RefGroupTy & {
    return (DelinearizationRequired && !DelinearizedGroups[Index].empty())
               ? DelinearizedGroups[Index]
               : Groups[Index];
  };

  // Dump collected ref groups for checking.
  LLVM_DEBUG(for (unsigned I = 0; I < GroupSize; ++I) {
    auto &Group = GetGroupForChecks(I);
    dbgs() << "Group " << I << " contains (" << Group.size() << ") refs:\n";
    for (auto &Ref : Group) {
      Ref->dump();
      dbgs() << "\n";
    }
  });

  // Construct IV Segments from Groups.
  SmallVector<IVSegment, ExpectedNumberOfTests> IVSegments;
  for (unsigned I = 0; I < GroupSize; ++I) {
    if (!EnableStructSupport && !Groups[I].empty() &&
        Groups[I].front()->accessesStruct()) {
      return STRUCT_ACCESS;
    }

    bool IsWriteGroup =
        std::any_of(Groups[I].begin(), Groups[I].end(),
                    [](const RegDDRef *Ref) { return Ref->isLval(); });

    IVSegments.emplace_back(GetGroupForChecks(I), IsWriteGroup);
    if (IVSegments.back().isEmpty()) {
      continue;
    }

    // Check every segment for the applicability
    Ret = IVSegments.back().isSegmentSupported(Loop, InnermostLoop);
    if (Ret != OK) {
      return Ret;
    }
  }

  processLoopnest(Loop, InnermostLoop, IVSegments);

  // Create pairs of segments to intersect and store them into
  // Candidate.SegmentList
  for (auto &Test : Tests) {
    unsigned I = Test.first;
    unsigned J = Test.second;

    IVSegment &S1 = IVSegments[I];
    IVSegment &S2 = IVSegments[J];

    // Skip Read-Read segments
    assert((S1.isWrite() || S2.isWrite()) &&
           "At least of the one segments should be a write segment");

    Context.SegmentList.push_back(S1.genSegment());
    Context.SegmentList.push_back(S2.genSegment());
  }

  return OK;
}

HLInst *HIRRuntimeDD::createUGECompare(HLNodeUtils &HNU, HLContainerTy &Nodes,
                                       RegDDRef *Ref1, RegDDRef *Ref2) {
  auto &DL = HNU.getDataLayout();

  Type *T1 = Ref1->getDestType();
  Type *T2 = Ref2->getDestType();

  // In case of different types, bitcast one segment bounds to another to
  // be in compliance with LLVM IR. (see ex. in lit test ptr-types.ll)
  if (T1 != T2) {
    Type *SmallerType;
    RegDDRef **LargerTypeRefPtr;
    unsigned LargerTypeSize;
    unsigned SmallerTypeSize;

    auto T1Size = DL.getTypeSizeInBits(T1->getPointerElementType());
    auto T2Size = DL.getTypeSizeInBits(T2->getPointerElementType());

    if (T1Size > T2Size) {
      SmallerType = T2;
      SmallerTypeSize = T2Size;
      LargerTypeRefPtr = &Ref1;
      LargerTypeSize = T1Size;
    } else {
      SmallerType = T1;
      SmallerTypeSize = T1Size;
      LargerTypeRefPtr = &Ref2;
      LargerTypeSize = T2Size;
    }

    (*LargerTypeRefPtr)->setBitCastDestType(SmallerType);

    auto Ceil = [](unsigned A, unsigned B) { return (A + B - 1) / B; };
    unsigned Offset = Ceil(LargerTypeSize, SmallerTypeSize) - 1;

    // If offset needed and the LargestType Ref is an upper reference.
    if (Offset && *LargerTypeRefPtr == Ref1) {
      // %offset_base = (i8*)&(%A)[0]
      // Upper ref will be replaced with &(%offset_base)[<Offset>]

      Type *OffsetTy =
          DL.getIntPtrType(HNU.getContext(), Ref1->getPointerAddressSpace());

      auto *BaseInst = HNU.createCopyInst(*LargerTypeRefPtr, "mv.upper.base");
      Nodes.push_back(*BaseInst);

      auto *BaseDDRef = BaseInst->getLvalDDRef();

      auto *OffsetDDRef = HNU.getDDRefUtils().createAddressOfRef(
          BaseDDRef->getSelfBlobIndex(), NonLinearLevel,
          (*LargerTypeRefPtr)->getSymbase(), true);

      OffsetDDRef->addDimension(
          HNU.getCanonExprUtils().createCanonExpr(OffsetTy, 0, Offset));

      // Replace larger reference with a cast instruction.
      *LargerTypeRefPtr = OffsetDDRef;
    }
  }

  return HNU.createCmp(PredicateTy::ICMP_UGE, Ref1, Ref2, "mv.test");
}

HLInst *HIRRuntimeDD::createIntersectionCondition(HLNodeUtils &HNU,
                                                  HLContainerTy &Nodes,
                                                  Segment &S1, Segment &S2) {
  HLInst *Cmp1 = createUGECompare(HNU, Nodes, S1.Upper, S2.Lower);
  HLInst *Cmp2 = createUGECompare(HNU, Nodes, S2.Upper, S1.Lower);
  HLInst *And = HNU.createAnd(Cmp1->getLvalDDRef()->clone(),
                              Cmp2->getLvalDDRef()->clone(), "mv.and");

  Nodes.push_back(*Cmp1);
  Nodes.push_back(*Cmp2);
  Nodes.push_back(*And);
  return And;
}

template <typename FuncTy>
static void applyForLoopnest(HLLoop *OuterLoop, FuncTy Func) {
  assert(OuterLoop && "OuterLoop should not be nullptr");

  while (OuterLoop) {
    Func(OuterLoop);
    OuterLoop = dyn_cast_or_null<HLLoop>(OuterLoop->getFirstChild());
  }
}

HLIf *HIRRuntimeDD::createCompareCondition(LoopContext &Context,
                                           HLIf *MasterIf,
                                           HLContainerTy &Nodes) {
  // The HIR structure will be the following:
  //
  // ZTT {
  //   <Preheader>
  //
  //   %cmp-0.1 = <test-0.1>
  //   %cmp-0.2 = <test-0.2>
  //   %and-0 = and %cmp-0.1, %cmp-0.2
  //
  //   %cmp-n.1 = <test-n.1>
  //   %cmp-n.2 = <test-n.2>
  //   %and-n = and %cmp-n.1, %cmp-n.2
  //
  //   if (%and-0 == F && ... && %and-n == F) {
  //     <Modified loop>
  //   } else {
  //     <Original loop>
  //   }
  //
  //   <PostExit>
  // }

  auto &HNU = Context.Loop->getHLNodeUtils();

  SmallVector<RegDDRef *, 2 * ExpectedNumberOfTests> TestDDRefs;
  unsigned RefsCount = Context.SegmentList.size();
  for (unsigned i = 0; i < RefsCount; i += 2) {
    auto &S1 = Context.SegmentList[i];
    auto &S2 = Context.SegmentList[i + 1];

    HLInst *And = createIntersectionCondition(HNU, Nodes, S1, S2);
    TestDDRefs.push_back(And->getLvalDDRef()->clone());
  }

  Type *Ty = TestDDRefs.front()->getDestType();
  auto CheckI = TestDDRefs.begin();

  if (!MasterIf) {
    MasterIf = HNU.createHLIf(PredicateTy::ICMP_EQ, *CheckI,
                              HNU.getDDRefUtils().createConstDDRef(Ty, 0));
    ++CheckI;
  }

  for (auto E = TestDDRefs.end(); CheckI != E; ++CheckI) {
    MasterIf->addPredicate(PredicateTy::ICMP_EQ, (*CheckI),
                           HNU.getDDRefUtils().createConstDDRef(Ty, 0));
  }

  return MasterIf;
}

HLIf *HIRRuntimeDD::createMasterCondition(LoopContext &Context,
                                          HLContainerTy &Nodes) {
  auto &HNU = Context.Loop->getHLNodeUtils();
  HLIf *MasterIf = nullptr;

  // Generate Pre-Conditions.
  if (!Context.PreConditions.empty()) {
    auto CheckI = Context.PreConditions.begin();
    MasterIf = HNU.createHLIf(CheckI->Pred, CheckI->Op1, CheckI->Op2);
    ++CheckI;

    for (auto CheckE = Context.PreConditions.end(); CheckI != CheckE;
         ++CheckI) {
      MasterIf->addPredicate(CheckI->Pred, CheckI->Op1, CheckI->Op2);
    }
  }

  // Generate range/overlap checks.
  switch (Context.Method) {
  case RTDDMethod ::Compare:
    return createCompareCondition(Context, MasterIf, Nodes);
  }

  llvm_unreachable("Unknown test generation method");
}

void HIRRuntimeDD::generateHLNodes(LoopContext &Context) {
  Context.Loop->extractZtt();
  Context.Loop->extractPreheaderAndPostexit();

  // The HIR structure will be the following:
  //
  // ZTT {
  //   <Preheader>
  //
  //   if (<tests>) {
  //     <Original loop with noalias>
  //   } else {
  //     <Cloned untouched loop>
  //   }
  //
  //   <PostExit>
  // }

  auto LoopMapper = HLNodeLambdaMapper::mapper(
      [](const HLNode *Node) { return isa<HLLoop>(Node); });

  HLLoop *NoAliasLoop = Context.Loop;
  HLLoop *ClonedLoop = Context.Loop->clone(&LoopMapper);

  LoopOptReportBuilder &LORBuilder =
      NoAliasLoop->getHLNodeUtils().getHIRFramework().getLORBuilder();

  LORBuilder(*NoAliasLoop).addOrigin("Multiversioned loop");
  LORBuilder(*ClonedLoop).addRemark(OptReportVerbosity::Low,
                                  "The loop has been multiversioned");

  HLContainerTy Nodes;
  HLIf *MemcheckIf = createMasterCondition(Context, Nodes);

  HLNodeUtils::insertBefore(NoAliasLoop, &Nodes);
  HLNodeUtils::insertBefore(NoAliasLoop, MemcheckIf);

  HLNodeUtils::moveAsFirstThenChild(MemcheckIf, NoAliasLoop);
  HLNodeUtils::insertAsFirstElseChild(MemcheckIf, ClonedLoop);

  // Implementation Note: The transformation adds NoAlias/Scope metadata to the
  // original loop and creates a clone for the unmodified loop.
  // 1) When RTDD will be used on-demand the clients may continue to work
  //    with the original loop and just ignore dependencies.
  // 2) Adding metadata to the cloned loop will require DDG for the new loop or
  //    creation of a mapping mechanism between original and cloned
  //    DDRefs.
  markDDRefsIndep(Context);

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(MemcheckIf);
  applyForLoopnest(NoAliasLoop, [&LoopMapper](HLLoop *Loop) {
    auto MVTag = Loop->getNumber();
    Loop->setMVTag(MVTag);

    HLLoop *OrigLoop = LoopMapper.getMapped(Loop);
    OrigLoop->setMVTag(MVTag);
    OrigLoop->markDoNotVectorize();
    OrigLoop->markDoNotUnroll();

    if (Loop->isInnermost()) {
      HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Loop);
    }
  });
}

void HIRRuntimeDD::markDDRefsIndep(LoopContext &Context) {
  auto &LLVMContext =
      Context.Loop->getHLNodeUtils().getHIRFramework().getContext();
  RefGroupVecTy &Groups = Context.Groups;

  auto Size = Groups.size();
  MDBuilder MDB(LLVMContext);

  MDNode *Domain = MDB.createAnonymousAliasScopeDomain();
  SmallVector<MDNode *, ExpectedNumberOfTests> NewScopes;
  NewScopes.reserve(Size);
  for (unsigned I = 0; I < Size; ++I) {
    NewScopes.push_back(MDB.createAnonymousAliasScope(Domain));
  }

  for (unsigned I = 0, E = Size; I < E; ++I) {
    auto ScopeId = I;

    for (RegDDRef *Ref : Groups[I]) {
      AAMDNodes AANodes;
      Ref->getAAMetadata(AANodes);

      AANodes.Scope = MDNode::concatenate(AANodes.Scope, NewScopes[ScopeId]);

      SmallVector<Metadata *, ExpectedNumberOfTests> NoAliasScopes;
      NoAliasScopes.reserve(Size - 1);
      NoAliasScopes.append(NewScopes.begin(), NewScopes.begin() + ScopeId);
      NoAliasScopes.append(NewScopes.begin() + ScopeId + 1, NewScopes.end());

      MDNode *NoAliasScopesMD;
      if (NoAliasScopes.size() == 1) {
        NoAliasScopesMD = cast<MDNode>(NoAliasScopes.front());
      } else {
        NoAliasScopesMD = MDNode::get(LLVMContext, NoAliasScopes);
      }

      AANodes.NoAlias = MDNode::concatenate(AANodes.NoAlias, NoAliasScopesMD);

      Ref->setAAMetadata(AANodes);
    }
  }
}

bool HIRRuntimeDD::run() {
  if (DisableRuntimeDD) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIRRuntimeDD for function: "
                    << HIRF.getFunction().getName() << "\n");

  // Multiversion for memory aliasing.
  MemoryAliasAnalyzer AliasAnalyzer(*this);
  HIRF.getHLNodeUtils().visitAll(AliasAnalyzer);

  if (AliasAnalyzer.LoopContexts.size() != 0) {
    for (LoopContext &Candidate : AliasAnalyzer.LoopContexts) {
      generateHLNodes(Candidate);

      // Statistics
      ++LoopsMultiversioned;
      if (!Candidate.Loop->isInnermost()) {
        ++OuterLoopsMultiversioned;
      }
    }
  }

  return true;
}

PreservedAnalyses HIRRuntimeDDPass::run(llvm::Function &F,
                                        llvm::FunctionAnalysisManager &AM) {
  HIRRuntimeDD(AM.getResult<HIRFrameworkAnalysis>(F),
               AM.getResult<HIRDDAnalysisPass>(F),
               AM.getResult<HIRLoopStatisticsAnalysis>(F))
      .run();

  return PreservedAnalyses::all();
}

class HIRRuntimeDDLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRRuntimeDDLegacyPass() : HIRTransformPass(ID) {
    initializeHIRRuntimeDDLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRRuntimeDD(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                        getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                        getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS())
        .run();
  }
};

char HIRRuntimeDDLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRRuntimeDDLegacyPass, OPT_SWITCH, OPT_DESCR, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRRuntimeDDLegacyPass, OPT_SWITCH, OPT_DESCR, false, false)

FunctionPass *llvm::createHIRRuntimeDDPass() {
  return new HIRRuntimeDDLegacyPass();
}
