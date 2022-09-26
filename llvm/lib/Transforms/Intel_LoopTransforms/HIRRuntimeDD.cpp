//===- HIRRuntimeDD.cpp - Implements Multiversioning for Runtime DD -=========//
//
// Copyright (C) 2016-2021 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRRuntimeDDPass.h"

#include "llvm/Pass.h"

#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
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
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "HIRRuntimeDD.h"

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

static cl::opt<bool> EnableSExtDelinearization(
    "enable-" OPT_SWITCH "-delin-sext", cl::init(false), cl::Hidden,
    cl::desc("Enable " OPT_DESCR " delinearization with signed extensions."));

static cl::opt<bool>
    DisableCostModel("disable-" OPT_SWITCH "-cost-model", cl::init(false),
                     cl::Hidden, cl::desc("Disable " OPT_DESCR " cost model."));

static cl::opt<bool> DisableLibraryCallSwitch(
    "disable-" OPT_SWITCH "-library-call", cl::init(false), cl::Hidden,
    cl::desc("Disable " OPT_DESCR " library call method."));

static cl::opt<unsigned> RtlThreshold(
    OPT_SWITCH "-rtl-threshold", cl::init(ExpectedNumberOfTests), cl::Hidden,
    cl::desc("Number of tests when LibraryCall method would be used."));

static cl::opt<unsigned>
    MaximumNumberOfTests(OPT_SWITCH "-max-tests", cl::init(60), cl::Hidden,
                         cl::desc("Maximum number of runtime tests for loop."));

static cl::opt<bool> IgnoreIVDepLoopLoops(
    OPT_SWITCH "-ignore-ivdeploop-loops", cl::init(false), cl::Hidden,
    cl::desc("Ignore loops with \"ivdep loop\" in " OPT_DESCR "."));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
namespace DbgMessage {
enum Kind {
  None = 0,
  Positive = 1,
  Negative = 2,
  Both = 3,
};
}

static cl::opt<DbgMessage::Kind> CreateDbgMessages(
    OPT_SWITCH "-dbg", cl::init(DbgMessage::None), cl::Hidden,
    cl::ValueOptional,
    cl::desc(OPT_DESCR " creates stdout messages for loop versions."),
    cl::values(clEnumValN(DbgMessage::None, "none", "Disable debug messages"),
               clEnumValN(DbgMessage::Positive, "positive", "Positive matches"),
               clEnumValN(DbgMessage::Negative, "negative", "Negative matches"),
               clEnumValN(DbgMessage::Both, "both",
                          "Both positive and negative matches"),
               // Value assumed when just -dbg is specified.
               clEnumValN(DbgMessage::Both, "", "")));
#endif

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
           (Result != NON_NORMALIZED_BLOB_IV_COEFF) &&
           (Result != NON_PROFITABLE_ALIAS);
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

static unsigned getMinMaxConstantBlob(BlobUtils &BU, unsigned Index,
                                      int64_t Constant, bool IsSigned,
                                      bool IsMinBlob) {
  BlobTy Blob = BU.getBlob(Index);

  // We don't have to keep the constant blob in the table.
  BlobTy ConstantBlob =
      BU.createBlob(Constant, Blob->getType(), false, nullptr);

  unsigned MinMaxBlobIndex;

  if (IsMinBlob) {
    if (IsSigned) {
      BU.createSMinBlob(Blob, ConstantBlob, true, &MinMaxBlobIndex);
    } else {
      BU.createUMinBlob(Blob, ConstantBlob, true, &MinMaxBlobIndex);
    }
  } else {
    if (IsSigned) {
      BU.createSMaxBlob(Blob, ConstantBlob, true, &MinMaxBlobIndex);
    } else {
      BU.createUMaxBlob(Blob, ConstantBlob, true, &MinMaxBlobIndex);
    }
  }

  return MinMaxBlobIndex;
}

static unsigned getMinMaxZeroBlob(BlobUtils &BU, unsigned Index,
                                  bool IsMinBlob) {
  return getMinMaxConstantBlob(BU, Index, 0, true, IsMinBlob);
}

// Returns true if IV replacement succeeded, false if failed and {} if no
// replacement required.
static Optional<bool> replaceIVByBound(CanonExpr *CE, const HLLoop *Loop,
                                       const HLLoop *InnerLoop,
                                       bool IsLowerBound,
                                       RegDDRef *UnknownLoopUBRef = nullptr) {
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
    const RegDDRef *UpperRef;
    if (Loop->isUnknown()) {
      assert(UnknownLoopUBRef &&
             "Candidate unknown loop does not have an upper-bound ref");
      UpperRef = UnknownLoopUBRef;
    } else {
      UpperRef = Loop->getUpperDDRef();
    }

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
                                                  Loop->hasSignedIV(), true);
  if (!Ret) {
    return false;
  }

  CE->simplify(true, true);

  return true;
}

// The method replaces \p Loop IV with the appropriate bounds depending on
// \p IsLowerBound value.
static void replaceIVByBound(RegDDRef *Ref, const HLLoop *Loop,
                             const HLLoop *InnerLoop, bool IsLowerBound,
                             RegDDRef *UnknownLoopUBRef) {
  for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
    auto Ret =
        replaceIVByBound(CE, Loop, InnerLoop, IsLowerBound, UnknownLoopUBRef);

    assert((!Ret || *Ret) &&
           "Assuming replace will always succeed as we already checked if both "
           "are mergeable.");
    (void)Ret;
  }
}

static bool sortRefsInSingleGroup(RefGroupTy &Group) {
  for (int I = 0, E = Group.size() - 1; I < E; ++I) {
    if (!DDRefUtils::haveConstDimensionDistances(Group[I], Group[I + 1],
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
  if (isEmpty()) {
    return OK;
  }

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

      if (!LoopI->isUnknown()) {
        const CanonExpr *UpperBoundCE = LoopI->getUpperCanonExpr();

        // Check if CE and UpperBoundCE are mergeable and check if UpperBoundCE
        // denominator equals one as we will not be able to replace IV with such
        // upper bound. This is because b*(x/d) != (b*x)/d.
        if ((UpperBoundCE->getDenominator() != 1 ||
             !CanonExprUtils::mergeable(CE, UpperBoundCE, true)) &&
            !UpperBoundCE->canConvertToStandAloneBlobOrConstant()) {
          return UPPER_SUB_TYPE_MISMATCH;
        }
      }
      assert((CanonExprUtils::mergeable(CE, LoopI->getLowerCanonExpr(), true) ||
              LoopI->getLowerCanonExpr()
                  ->canConvertToStandAloneBlobOrConstant()) &&
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
void IVSegment::replaceIVWithBounds(const HLLoop *Loop, const HLLoop *InnerLoop,
                                    RegDDRef *UnknownLoopUBRef) {
  assert(!isEmpty());

  replaceIVByBound(Lower.get(), Loop, InnerLoop, true, UnknownLoopUBRef);
  replaceIVByBound(Upper.get(), Loop, InnerLoop, false, UnknownLoopUBRef);
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
    return "Loop is either not a DO loop or cannot be converted to a DO loop";
  case UNROLL_PRAGMA_LOOP:
    return "Unroll pragma loops are not supported";
  case IVDEP_PRAGMA_LOOP:
    return "Loop has IVDEP pragma";
  case NON_PROFITABLE:
    return "Loop considered non-profitable";
  case NON_PROFITABLE_SUBS:
    return "Subscript multiversioning is non-profitable";
  case NON_PROFITABLE_ALIAS:
    return "Loop considered non-profitable due to partial/must alias between "
           "groups";
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
  case UNKNOWN_ADDR_RANGE:
    return "Mem ref has unknown address range";
  }
  llvm_unreachable("Unexpected give up reason");
}
#endif

bool HIRRuntimeDD::isProfitable(const HLLoop *Loop) {
  if (DisableCostModel) {
    return true;
  }

  const LoopStatistics &LS = HLS.getTotalLoopStatistics(Loop);

  return (!LS.hasCallsWithUnknownAliasing() && !LS.hasSwitches());
}

// Creates the following two HLInsts for an Unknown Loop's Upper Bound and saves
// them in the Loop's Context (to be inserted into HIR during transformation):
//
// (1) Load Inst for the Upper Bound, e.g.,
//
//       %2 = (%LoopCount)[i1];
//
// (2) Copy Inst that processes the RHS Temp from the Bottom Test as well as the
// Temp from the previous Load to create the upper bound ref, e.g., one of:
//
//       %ub = smax(1, %2) + -1;
//       %ub = smax(1, sext.i32.i64(%2)) + -1;
//       %ub = umax(1, %2) + -1;
//       %ub = umax(1, zext.i32.i64(%2)) + -1;
//
//     where %ub represents the Upper Bound of the loop.
void HIRRuntimeDD::createUnknownLoopUBInsts(LoopContext &Context) {
  const HLLoop *Loop = Context.Loop;

  auto *BottomTest = Loop->getBottomTest();

  // For now, we only handle the UB Load in the bottom tests's previous node.
  // TODO: Generalize so that the UB Load could be anywhere in the loop body.
  Context.UnknownLoopUBLoad = cast<HLInst>(BottomTest->getPrevNode())->clone();

  // Clone temp from the bottom test's RHS instead of the cloned Load's lval.
  // (This way we don't have to check sext/zext).
  auto *PredIt = BottomTest->pred_begin();
  auto *UBRef = BottomTest->getRHSPredicateOperandDDRef(PredIt)->clone();
  auto *UBCE = UBRef->getSingleCanonExpr();
  UBCE->convertToStandAloneBlobOrConstant();

  // Replace the blob with its smax(1, ...), or umax, and add -1,
  // e.g., '%2' is replaced with 'smax(1, %2) + -1' if comparison is signed.
  unsigned BlobIndex = UBCE->getSingleBlobIndex();
  unsigned MaxBlobIndex =
      getMinMaxConstantBlob(UBRef->getBlobUtils(), BlobIndex, 1,
                            (*PredIt) == CmpInst::ICMP_SLT, false);
  UBCE->replaceBlob(BlobIndex, MaxBlobIndex);
  UBCE->addConstant(-1, true);

  Context.UnknownLoopUBMax = Loop->getHLNodeUtils().createCopyInst(UBRef, "ub");
}

void HIRRuntimeDD::processLoopnest(const HLLoop *OuterLoop,
                                   const HLLoop *InnermostLoop,
                                   SmallVectorImpl<IVSegment> &IVSegments,
                                   RegDDRef *UnknownLoopUBRef) {

  assert(InnermostLoop->isInnermost() &&
         "InnermostLoop is not an innermost loop");

  unsigned SegmentCount = IVSegments.size();

  SmallVector<const RegDDRef *, 6> AuxRefs;

  // Replace every IV in segments with upper and lower bounds
  for (const HLLoop *LoopI = InnermostLoop, *LoopE = OuterLoop->getParentLoop();
       LoopI != LoopE; LoopI = LoopI->getParentLoop()) {

    AuxRefs.push_back(LoopI->getLowerDDRef());
    if (LoopI->isUnknown()) {
      assert((LoopI == OuterLoop && LoopI->isInnermost()) &&
             "Unknown loop is either not a candidate or not an innermost loop");
      assert(UnknownLoopUBRef &&
             "Candidate unknown loop does not have an upper-bound ref");
      AuxRefs.push_back(UnknownLoopUBRef);
    } else {
      AuxRefs.push_back(LoopI->getUpperDDRef());
    }

    for (unsigned I = 0; I < SegmentCount; ++I) {
      if (IVSegments[I].isEmpty()) {
        continue;
      }

      IVSegments[I].replaceIVWithBounds(LoopI, InnermostLoop, UnknownLoopUBRef);
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

  SmallSetVector<const RegDDRef *, 8> AuxRefs;

  for (int I = 0, E = Sizes.size() - 1; I < E; ++I) {
    // We already checked that subscripts may be compared.
    auto MinMax = std::minmax_element(
        Group.begin(), Group.end(), [&](const RegDDRef *A, const RegDDRef *B) {
          return CanonExprUtils::compare(A->getDimensionIndex(I + 1),
                                         B->getDimensionIndex(I + 1));
        });

    AuxRefs.insert(*MinMax.first);
    AuxRefs.insert(*MinMax.second);

    // Generate:
    //   Subscript[i + 1] < Size[i]
    // TODO: need to handle Ref's lower bound

    const CanonExpr *MinCE = (*MinMax.first)->getDimensionIndex(I + 1);
    std::unique_ptr<CanonExpr> MinCEClone(MinCE->clone());

    const CanonExpr *MaxCE = (*MinMax.second)->getDimensionIndex(I + 1);
    RegDDRef *MaxRef =
        DRU.createScalarRegDDRef(GenericRvalSymbase, MaxCE->clone());

    // Replace IVs.
    for (const auto *Loop = InnerLoop, *LoopE = OuterLoop->getParentLoop();
         Loop != LoopE; Loop = Loop->getParentLoop()) {
      auto Ret = replaceIVByBound(MaxRef->getSingleCanonExpr(), Loop, InnerLoop,
                                  false);
      if (Ret && !*Ret) {
        return false;
      }

      Ret = replaceIVByBound(MinCEClone.get(), Loop, InnerLoop, true);
      if (Ret && !*Ret) {
        return false;
      }

      // replaceIVByBound() has non-trivial logic for choosing between Lower and
      // Upper bound RefDDRef - add both.
      AuxRefs.insert(Loop->getUpperDDRef());
      AuxRefs.insert(Loop->getLowerDDRef());
    }

    bool Subtracted = CanonExprUtils::subtract(MaxRef->getSingleCanonExpr(),
                                               MinCEClone.get(), false);
    (void)Subtracted;
    assert(Subtracted && "Can not subtract!");

    // Subscripts may contain embedded cast even if they are constant.
    MaxRef->getSingleCanonExpr()->simplify(true, false);

    // Generate:
    //   Size[i] > 1
    auto SizeBlob = Sizes[E - I - 1];
    RegDDRef *DimSizeRef = DRU.createConstDDRef(SizeBlob->getType(), 0);
    DimSizeRef->getSingleCanonExpr()->addBlob(BU.findOrInsertBlob(SizeBlob), 1);
    if (DimSizeRef->getDestType() != MaxRef->getDestType()) {
      auto *DimSizeCE = DimSizeRef->getSingleCanonExpr();
      DimSizeCE->setDestType(MaxRef->getDestType());
      DimSizeCE->setExtType(true);
    }

    DimSizeRef->makeConsistent(AuxRefs.getArrayRef(), Level - 1);

    RegDDRef *OneRef = DRU.createConstDDRef(MaxRef->getDestType(), 1);
    Conditions.emplace_back(DimSizeRef, CmpInst::ICMP_SGT, OneRef);

    // (MaxCE - MinCE) < DimSizeRef
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

  auto IsAlreadySorted = [&](unsigned Index) {
    return std::find(UnsortedGroupIndices.begin(), UnsortedGroupIndices.end(),
                     Index) == UnsortedGroupIndices.end();
  };

  for (unsigned I = 0, E = Groups.size(); I < E; ++I) {
    auto &Group = Groups[I];
    auto &DelinearizedGroup = DelinearizedGroups[I];

    // The group may be empty because it was cleared before as it's not involved
    // in DD.
    if (Group.empty()) {
      continue;
    }

    // Bail out if any ref has more than one dimension.
    if (std::any_of(Group.begin(), Group.end(), [](const RegDDRef *Ref) {
          return !Ref->isSingleDimension();
        })) {

      if (IsAlreadySorted(I)) {
        continue;
      }

      return DELINEARIZATION_FAILED;
    }

    SmallVector<BlobTy, MaxLoopNestLevel> GroupSizes;
    if (!DDRefUtils::delinearizeRefs(Group, DelinearizedGroup, &GroupSizes,
                                     EnableSExtDelinearization)) {
      if (IsAlreadySorted(I)) {
        DelinearizedGroup.clear();
        continue;
      }

      return DELINEARIZATION_FAILED;
    }

    // Try sorting second time after delinearization.
    if (!sortRefsInSingleGroup(DelinearizedGroup)) {
      if (IsAlreadySorted(I)) {
        DelinearizedGroup.clear();
        continue;
      }

      return UNKNOWN_MIN_MAX;
    }

    if (!computeDelinearizationValidityConditions(DelinearizedGroup, GroupSizes,
                                                  Loop, InnermostLoop,
                                                  ValidityConditions)) {
      if (IsAlreadySorted(I)) {
        DelinearizedGroup.clear();
        continue;
      }

      LLVM_DEBUG(
          dbgs()
          << "[RTDD] computeDelinearizationValidityConditions() failed.\n");
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
    LLVM_DEBUG(dbgs() << "[RTDD] Infeasible delinearization conditions\n");
    return DELINEARIZATION_FAILED;
  }

  ValidityConditions.erase(TrivialConditionsI, ValidityConditions.end());

  if (ValidityConditions.size() > RtlThreshold) {
    return TOO_MANY_TESTS;
  }

  PreConditions.append(ValidityConditions.begin(), ValidityConditions.end());
  return OK;
}

static bool isStarEdge(const DDEdge &Edge) {
  for (DVKind Kind : Edge.getDV()) {
    if (Kind == DVKind::NONE) {
      break;
    }

    if (Kind == DVKind::ALL) {
      return true;
    }
  }

  return false;
}

// Check for temps in the loop. If there are live-in temps
// and they are not in a safe-reduction chain, RTDD will not help vectorization
// and should be avoided.
bool HIRRuntimeDD::canHelpVectorization(const HLLoop *InnermostLoop) const {
  // Gather terminal refs which are only inside the innermost loop
  DDRefGatherer<RegDDRef, TerminalRefs>::VectorTy TempRefs;
  DDRefGatherer<RegDDRef, TerminalRefs>::gather(InnermostLoop, TempRefs);

  SRA.computeSafeReductionChains(InnermostLoop);

  DDGraph DDG = DDA.getGraph(InnermostLoop);
  unsigned LoopLevel = InnermostLoop->getNestingLevel();

  for (RegDDRef *TempRef : TempRefs) {
    if (TempRef->isRval() || !InnermostLoop->isLiveIn(TempRef->getSymbase())) {
      continue;
    }

    for (const DDEdge *Edge : DDG.outgoing(TempRef)) {
      if (!Edge->preventsVectorization(LoopLevel)) {
        continue;
      }

      unsigned OpCode;
      if (!SRA.isReductionRef(TempRef, OpCode)) {
        return false;
      }
    }
  }
  return true;
}

RuntimeDDResult HIRRuntimeDD::processDDGToGroupPairs(
    const HLLoop *Loop, MemRefGatherer::VectorTy &Refs,
    DenseMap<const RegDDRef *, unsigned> &RefGroupIndex,
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

      // Do not multiversion if the dependency is well defined.
      if (!isStarEdge(*Edge)) {
        continue;
      }

      RegDDRef *DstRef = cast<RegDDRef>(Edge->getSink());

      auto GroupBI = RefGroupIndex.find(DstRef);
      assert(GroupBI != RefGroupIndex.end() &&
             "Found dependence to unknown ref");
      unsigned GroupB = GroupBI->second;

      if (GroupA != GroupB) {
        if (DDA.areRefsMustAliasOrPartialAlias(SrcRef, DstRef)) {
          return NON_PROFITABLE_ALIAS;
        }

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

static bool unknownLoopInLoopNest(const HLLoop *Loop,
                                  const HLLoop *InnermostLoop) {
  assert(InnermostLoop->isInnermost() &&
         "InnermostLoop is not an innermost loop");
  for (const HLLoop *LoopI = InnermostLoop, *LoopE = Loop->getParentLoop();
       LoopI != LoopE; LoopI = LoopI->getParentLoop()) {
    if (LoopI->isUnknown()) {
      return true;
    }
  }

  return false;
}

// Check whether RTDD can happen before a relaxed non-perfect loopnest.
// The pattern is matched by Geekbench6.0/Camera
static bool canLoopBeRelaxed(HLLoop *Loop, const HLLoop *&InnermostLoop) {
  HLIf *If = dyn_cast<HLIf>(Loop->getLastChild());

  if (!If) {
    return false;
  }

  if (If->getNumThenChildren() != 1 || If->hasElseChildren()) {
    return false;
  }

  const HLNode *FirstThenChild = If->getFirstThenChild();

  InnermostLoop = dyn_cast<HLLoop>(FirstThenChild);

  if (!InnermostLoop || !InnermostLoop->isInnermost()) {
    return false;
  }

  if (unknownLoopInLoopNest(Loop, InnermostLoop)) {
    return false;
  }

  for (const HLNode &Node :
       make_range(Loop->child_begin(), std::prev(Loop->child_end()))) {
    const HLInst *HInst = dyn_cast<HLInst>(&Node);
    if (!HInst) {
      return false;
    }

    const RegDDRef *Lval = HInst->getLvalDDRef();

    if (!Lval || !Lval->isTerminalRef()) {
      return false;
    }
  }

  return true;
}

static bool
canHelpScalarReplacementOrMemoryMotion(const HLLoop *InnermostLoop) {
  return HIRLoopLocality::hasTemporalLocality(InnermostLoop, 2, true, false);
}

// We look for the UNKNOWN loop of the following form:
//
// UNKNOWN LOOP iN       <--- Loop is innermost.
//   ...                 <--- All mem-refs are linear-at-level.
//   ...                      There is only one Lval mem-ref.
//   %2 = <mem-ref>;     <--- Load for UB just before the Bottom test.
//   if (iN + 1 < %2)    <--- Bottom test of this form.
//   {                        Comparison can be signed (<) or unsigned (<u).
//     ...
//   }
// END LOOP
bool HIRRuntimeDD::isConvertibleUnknownLoop(const HLLoop *Loop) {
  if (!Loop->isUnknown() || !Loop->isInnermost()) {
    return false;
  }

  auto *BottomTest = Loop->getBottomTest();
  auto *PredIt = BottomTest->pred_begin();
  if ((*PredIt) != CmpInst::ICMP_SLT && (*PredIt) != CmpInst::ICMP_ULT) {
    return false;
  }

  auto *LHSRef = BottomTest->getLHSPredicateOperandDDRef(PredIt);
  auto *RHSRef = BottomTest->getRHSPredicateOperandDDRef(PredIt);
  if (!LHSRef->isTerminalRef() || !RHSRef->isTerminalRef()) {
    return false;
  }

  unsigned Level = Loop->getNestingLevel();

  // Check for the pattern iN + 1 in the BottomTest LHS.
  auto *CE = LHSRef->getSingleCanonExpr();
  if (CE->numIVs() != 1 || CE->numBlobs() != 0 || CE->getDenominator() != 1 ||
      CE->getIVConstCoeff(Level) != 1 || CE->getConstant() != 1 ||
      CE->getIVBlobCoeff(Level) != InvalidBlobIndex) {
    return false;
  }

  // For now, we only handle the UB Load in the bottom tests's previous node.
  // TODO: Generalize so that the UB Load could be anywhere in the loop body.
  auto *UBLoad = dyn_cast_or_null<HLInst>(BottomTest->getPrevNode());
  if (!UBLoad || !isa<LoadInst>(UBLoad->getLLVMInstruction())) {
    return false;
  }

  if (!UBLoad->getRvalDDRef()->isStructurallyInvariantAtLevel(Level)) {
    return false;
  }

  // Refs might have sext or zext, so we need to compare CEs instead of
  // RegDDRefs.
  auto *LvalCE = UBLoad->getLvalDDRef()->getSingleCanonExpr();
  if (!CanonExprUtils::areEqual(RHSRef->getSingleCanonExpr(), LvalCE, true)) {
    return false;
  }

  // For the loop body (i.e., not including the Bottom Test and the Load for the
  // Upper Bound), determine:
  // (1) If all mem-refs are linear at level.
  // (2) The number of Lval mem-refs.
  bool MemRefsAreLinearAtLevel = true;
  unsigned NumLvalMemRefs = 0;
  ForEach<const HLDDNode>::visitRange(
      Loop->child_begin(), UBLoad->getIterator(),
      [&Level, &MemRefsAreLinearAtLevel,
       &NumLvalMemRefs](const HLDDNode *Node) {
        for (const RegDDRef *Ref :
             make_range(Node->ddref_begin(), Node->ddref_end())) {
          if (Ref->isMemRef()) {
            if (!Ref->isLinearAtLevel(Level)) {
              MemRefsAreLinearAtLevel = false;
            }
            if (Ref->isLval()) {
              NumLvalMemRefs++;
            }
          }
        }
      });

  return MemRefsAreLinearAtLevel && NumLvalMemRefs == 1;
}

static bool isLoadOnly(RefGroupTy &Group) {
  for (auto &Ref : Group) {
    if (Ref->isLval()) {
      return false;
    }
  }
  return true;
}

// Split the group if two adjacent refs do not have constant distance and they
// are from different parent loop. For example, A[i1], A[i1+1], A[i2], A[i2+1]
// will be split into 2 groups-
// A[i1], A[i1+1] and  A[i2], A[i2+1], if A[i1+1] and A[i2] are from different
// parent loops. We will bail out the case if the rest of refs in the split
// group do not have constant distance.
static void
splitRefGroups(RefGroupVecTy &Groups,
               DenseMap<const RegDDRef *, unsigned> &RefGroupIndex,
               DenseMap<unsigned, unsigned> &SplitedGroupsOriginalIndices) {
  unsigned GroupSize = Groups.size();

  for (unsigned I = 0; I < GroupSize; ++I) {
    unsigned CurGroupSize = Groups[I].size();
    // Restrict splitting to load-only groups for profitability
    if (CurGroupSize < 2) {
      continue;
    }

    if (!isLoadOnly(Groups[I])) {
      continue;
    }

    bool CanGroupBeSplitted = true;

    for (unsigned J = 0, EE = CurGroupSize - 1; J < EE; ++J) {
      if (DDRefUtils::haveConstDimensionDistances(Groups[I][J],
                                                  Groups[I][J + 1], false)) {
        continue;
      }

      if (Groups[I][J]->getParentLoop() == Groups[I][J + 1]->getParentLoop()) {
        continue;
      }

      // Check whether two adjacent refs in the group have constant distance.
      // If not, the group cannot be the slpit group candidate.
      for (unsigned M = J + 1, E = CurGroupSize - 1; M < E; ++M) {
        if (!DDRefUtils::haveConstDimensionDistances(Groups[I][M],
                                                     Groups[I][M + 1], false)) {
          CanGroupBeSplitted = false;
          break;
        }
      }

      if (!CanGroupBeSplitted) {
        break;
      }

      // Group size increased by 1 due to a new splited group
      Groups.resize(Groups.size() + 1);

      unsigned NewGroupIdx = Groups.size() - 1;

      // Record the new group's original group index which will be used in
      // DDRefsIndep().
      SplitedGroupsOriginalIndices[NewGroupIdx] = I;

      // Update the group index for the refs which will be in the new group
      for (unsigned K = J + 1, End = CurGroupSize; K < End; ++K) {
        RefGroupIndex[Groups[I][K]] = NewGroupIdx;
        Groups.back().push_back(Groups[I][K]);
      }

      Groups[I].resize(J + 1);

      break;
    }
  }

  return;
}

// Returns true if all refs in \p PreOrPostLoopMemRefs have constant distance
// with some ref in \p LoopMemRefs.
static bool haveConstantDistance(
    const HIRRuntimeDD::MemRefGatherer::VectorTy &PreOrPostLoopMemRefs,
    const HIRRuntimeDD::MemRefGatherer::VectorTy &LoopMemRefs) {

  for (auto *Ref1 : PreOrPostLoopMemRefs) {
    bool FoundConstDist = false;

    for (auto *Ref2 : LoopMemRefs) {
      if (DDRefUtils::haveConstDimensionDistances(Ref1, Ref2, false)) {
        FoundConstDist = true;
        break;
      }
    }

    if (!FoundConstDist) {
      return false;
    }
  }

  return true;
}

RuntimeDDResult HIRRuntimeDD::computeTests(HLLoop *Loop, LoopContext &Context) {
  RuntimeDDResult Ret = OK;
  Context.Loop = Loop;

  if (Loop->hasVectorizeIVDepPragma()) {
    // Historically we don't consider loops with "ivdep" pragmas. However,
    // OpenMP "parallel for" semantics may be preserved for LoopOpt as "ivdep
    // loop." Don't ignore these.
    if (!Loop->hasVectorizeIVDepLoopPragma() || IgnoreIVDepLoopLoops)
      return IVDEP_PRAGMA_LOOP;
  }

  if (Loop->getMVTag()) {
    return ALREADY_MV;
  }

  bool ConvertibleUnknownLoop = false;
  if (!Loop->isDo()) {
    ConvertibleUnknownLoop = isConvertibleUnknownLoop(Loop);
    if (!ConvertibleUnknownLoop) {
      return NON_DO_LOOP;
    }
  }

  if (!isProfitable(Loop)) {
    return NON_PROFITABLE;
  }

  const HLLoop *InnermostLoop = Loop;
  bool CanLoopBeRelaxed = false;

  bool IsNearPerfect = false;
  if (!Loop->isInnermost() &&
      !HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop, false,
                                      &IsNearPerfect) &&
      !IsNearPerfect) {
    CanLoopBeRelaxed = canLoopBeRelaxed(Loop, InnermostLoop);

    if (!CanLoopBeRelaxed) {
      return NON_PERFECT_LOOPNEST;
    }
  }

  Context.InnermostLoop = const_cast<HLLoop *>(InnermostLoop);

  bool ConstantTripCount = true;
  uint64_t TotalTripCount = 1;

  if (Loop->isInSIMDRegion()) {
    return SIMD_LOOP;
  }

  for (const HLLoop *LoopI = InnermostLoop, *LoopE = Loop->getParentLoop();
       LoopI != LoopE; LoopI = LoopI->getParentLoop()) {

    if (LoopI->hasUnrollEnablingPragma()) {
      return UNROLL_PRAGMA_LOOP;
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

  MemRefGatherer::VectorTy MemRefs;

  if (!IsNearPerfect) {
    // Using Loop instead of InnermostLoop to handle 'CanLoopBeRelaxed' case.
    MemRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(),
                                MemRefs);

  } else {
    // Gather pre and post innermost loop memrefs separately to do some
    // validation.
    MemRefGatherer::VectorTy PreLoopMemRefs;
    MemRefGatherer::VectorTy LoopMemRefs;
    MemRefGatherer::VectorTy PostLoopMemRefs;

    const HLLoop *OuterLoop = InnermostLoop->getParentLoop();

    MemRefGatherer::gatherRange(OuterLoop->child_begin(),
                                InnermostLoop->getIterator(), PreLoopMemRefs);
    MemRefGatherer::gatherRange(InnermostLoop->pre_begin(),
                                InnermostLoop->pre_end(), PreLoopMemRefs);

    MemRefGatherer::gatherRange(InnermostLoop->child_begin(),
                                InnermostLoop->child_end(), LoopMemRefs);

    MemRefGatherer::gatherRange(InnermostLoop->post_begin(),
                                InnermostLoop->post_end(), PostLoopMemRefs);
    MemRefGatherer::gatherRange(std::next(InnermostLoop->getIterator()),
                                OuterLoop->child_end(), PostLoopMemRefs);

    // Give up on near perfect loopnest if any of the pre or post loop memrefs
    // do not have constant distance with loop body refs as we will not be able
    // to from valid grouping. We return with 'NON_PERFECT_LOOPNEST' so the
    // caller can try multiversioning the innermost loop.
    if (!haveConstantDistance(PreLoopMemRefs, LoopMemRefs) ||
        !haveConstantDistance(PostLoopMemRefs, LoopMemRefs)) {
      return NON_PERFECT_LOOPNEST;
    }

    MemRefs.append(PreLoopMemRefs.begin(), PreLoopMemRefs.end());
    MemRefs.append(LoopMemRefs.begin(), LoopMemRefs.end());
    MemRefs.append(PostLoopMemRefs.begin(), PostLoopMemRefs.end());
  }

  LLVM_DEBUG(dbgs() << "[RTDD] Loop references:\n");
  LLVM_DEBUG(MemRefGatherer::dump(MemRefs));

  // Populate reference groups split by base blob index.
  // Populate a reference-to-group-number map.
  DDRefIndexGrouping Grouping(Groups, MemRefs);
  DenseMap<unsigned, unsigned> &SplitedGroupsOriginalIndices =
      Context.SplitedGroupsOriginalIndices;

  // Split the Group if the size of Group is larger than 2 and the elements are
  // from different parent loops and they do not have constant distance
  if (CanLoopBeRelaxed) {
    splitRefGroups(Groups, Grouping.getIndex(), SplitedGroupsOriginalIndices);
  }

  // Dump ref groups after split.
  LLVM_DEBUG(dbgs() << "Ref groups after split:\n ";
             for (unsigned I = 0; I < Groups.size(); ++I) {
               auto &Group = Groups[I];
               dbgs() << "Group " << I << " contains (" << Group.size()
                      << ") refs:\n";
               for (auto &Ref : Group) {
                 Ref->dump();
                 dbgs() << "\n";
               }
             });

  SmallSetVector<std::pair<unsigned, unsigned>, ExpectedNumberOfTests> Tests;
  Ret = processDDGToGroupPairs(Loop, MemRefs, Grouping.getIndex(), Tests);

  if (Ret != OK) {
    return Ret;
  }

  if (Tests.size() < 1) {
    return NO_OPPORTUNITIES;
  }

  if (Tests.size() > RtlThreshold) {
    if (EnableLibraryCallMethod && Groups.size() <= MaximumNumberOfTests) {
      LLVM_DEBUG(dbgs() << "[RTDD] LibraryCall method selected.\n");
      Context.Method = LibraryCall;

    } else {
      LLVM_DEBUG(
          dbgs() << "[RTDD] LibraryCall method required but disabled.\n");
      return TOO_MANY_TESTS;
    }
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

    // Populate DelinearizedGroupIndices with indexes where
    // DelinearizedGroups[I] is not empty.
    for (auto Group : enumerate(DelinearizedGroups)) {
      if (!Group.value().empty()) {
        Context.DelinearizedGroupIndices.push_back(Group.index());
      }
    }
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

  unsigned LoopLevel = Loop->getNestingLevel();

  // Construct IV Segments from Groups.
  SmallVector<IVSegment, ExpectedNumberOfTests> IVSegments;
  for (unsigned I = 0; I < GroupSize; ++I) {
    if (!EnableStructSupport && !Groups[I].empty() &&
        Groups[I].front()->accessesStruct()) {
      return STRUCT_ACCESS;
    }

    // Allow fake ref if canUsePointeeSize is set.
    if (std::any_of(Groups[I].begin(), Groups[I].end(),
                    [](const RegDDRef *Ref) {
                      return (Ref->isFake() && !Ref->canUsePointeeSize());
                    })) {
      return UNKNOWN_ADDR_RANGE;
    }

    if (!Loop->isInnermost()) {
      // Check whether any pre-loop inst's lval is used as a blob inside the
      // memref. If yes, RT cannot be allowed to put before the relaxed loop,
      // because the definition is after the use.
      if (std::any_of(Groups[I].begin(), Groups[I].end(),
                      [&](const RegDDRef *Ref) {
                        if (!Ref->isLinearAtLevel(LoopLevel)) {
                          return true;
                        }
                        return false;
                      })) {
        return NON_PERFECT_LOOPNEST;
      }

      // CHeck whether innermost loop bound uses any pre-loop inst's lval
      if (!(InnermostLoop->getUpperDDRef()->isStructurallyInvariantAtLevel(
              LoopLevel))) {
        return NON_PERFECT_LOOPNEST;
      }
    }

    bool IsWriteGroup =
        std::any_of(Groups[I].begin(), Groups[I].end(),
                    [](const RegDDRef *Ref) { return Ref->isLval(); });

    IVSegments.emplace_back(GetGroupForChecks(I), IsWriteGroup);

    // Check every segment for the applicability
    Ret = IVSegments.back().isSegmentSupported(Loop, InnermostLoop);
    if (Ret != OK) {
      return Ret;
    }
  }

  if (!canHelpVectorization(InnermostLoop) &&
      !canHelpScalarReplacementOrMemoryMotion(InnermostLoop)) {
    return NON_PROFITABLE;
  }

  if (ConvertibleUnknownLoop) {
    createUnknownLoopUBInsts(Context);
  }

  processLoopnest(Loop, InnermostLoop, IVSegments,
                  ConvertibleUnknownLoop
                      ? Context.UnknownLoopUBMax->getLvalDDRef()
                      : nullptr);

  // Check if LibraryCall method is required.
  if (Context.Method == RTDDMethod::LibraryCall) {
    for (auto &Seg : IVSegments) {
      if (Seg.isEmpty()) {
        continue;
      }

      Context.SegmentList.push_back(Seg.genSegment());
    }

    return OK;
  }

  // Create pairs of segments to intersect and store them into
  // Candidate.SegmentList
  for (auto &Test : Tests) {
    unsigned I = Test.first;
    unsigned J = Test.second;

    IVSegment &S1 = IVSegments[I];
    IVSegment &S2 = IVSegments[J];

    // Skip Read-Read segments
    assert((S1.isWrite() || S2.isWrite()) &&
           "At least one of the segments should be a write segment");

    Context.SegmentList.push_back(S1.genSegment());
    Context.SegmentList.push_back(S2.genSegment());
  }

  return OK;
}

// Update \p Lower and \p Upper address RegDDRefs element types to be equal to
// \p PtrElementType.
// * If PtrElementType is null then smallest type between Lower and Upper is
// selected.
// * If Upper type is larger than PtrElementType then non-linear address offset
// will be generated and added to \p Nodes to address the last word of larger
// type.
static void normalizeRefTypes(HLNodeUtils &HNU, HLContainerTy &Nodes,
                              RegDDRef *&Lower, RegDDRef *&Upper,
                              Type *PtrElementType) {
  assert(Lower->isAddressOf() && Upper->isAddressOf() &&
         "Expected to be isAddressOf DDRefs");

  auto &DL = HNU.getDataLayout();

  Type *LowerElementType = Lower->getDereferencedType();
  Type *UpperElementType = Upper->getDereferencedType();

  // Determine smallest type.
  if (!PtrElementType) {
    auto LowerSize = DL.getTypeSizeInBits(LowerElementType);
    auto UpperSize = DL.getTypeSizeInBits(UpperElementType);

    if (LowerSize < UpperSize) {
      PtrElementType = LowerElementType;
    } else {
      PtrElementType = UpperElementType;
    }
  }

  if (LowerElementType != PtrElementType) {
    Lower->setBitCastDestVecOrElemType(PtrElementType);
  }

  if (UpperElementType != PtrElementType) {
    Upper->setBitCastDestVecOrElemType(PtrElementType);

    auto UpperTypeSize = DL.getTypeSizeInBits(UpperElementType);
    auto PtrTypeSize = DL.getTypeSizeInBits(PtrElementType);

    auto Ceil = [](unsigned A, unsigned B) { return (A + B - 1) / B; };
    unsigned Offset = Ceil(UpperTypeSize, PtrTypeSize) - 1;

    // If offset needed for an upper reference.
    if (Offset) {
      // %offset_base = (i8*)&(%A)[0]
      // Upper ref will be replaced with &(%offset_base)[<Offset>]

      Type *OffsetTy =
          DL.getIntPtrType(HNU.getContext(), Upper->getPointerAddressSpace());

      auto *BaseInst = HNU.createCopyInst(Upper, "mv.upper.base");
      Nodes.push_back(*BaseInst);

      auto *BaseDDRef = BaseInst->getLvalDDRef();

      auto *OffsetDDRef = HNU.getDDRefUtils().createAddressOfRef(
          Upper->getDereferencedType(), BaseDDRef->getSelfBlobIndex(),
          NonLinearLevel, Upper->getSymbase(), true);

      OffsetDDRef->addDimension(
          HNU.getCanonExprUtils().createCanonExpr(OffsetTy, 0, Offset));

      // Replace larger reference with a cast instruction.
      Upper = OffsetDDRef;
    }
  }
}

HLInst *HIRRuntimeDD::createUGECompare(HLNodeUtils &HNU, HLContainerTy &Nodes,
                                       RegDDRef *Lower, RegDDRef *Upper) {
  normalizeRefTypes(HNU, Nodes, Lower, Upper, nullptr);
  return HNU.createCmp(PredicateTy::ICMP_UGE, Upper, Lower, "mv.test");
}

HLInst *HIRRuntimeDD::createIntersectionCondition(HLNodeUtils &HNU,
                                                  HLContainerTy &Nodes,
                                                  Segment &S1, Segment &S2) {
  HLInst *Cmp1 = createUGECompare(HNU, Nodes, S2.Lower, S1.Upper);
  HLInst *Cmp2 = createUGECompare(HNU, Nodes, S1.Lower, S2.Upper);
  HLInst *And = HNU.createAnd(Cmp1->getLvalDDRef()->clone(),
                              Cmp2->getLvalDDRef()->clone(), "mv.and");

  Nodes.push_back(*Cmp1);
  Nodes.push_back(*Cmp2);
  Nodes.push_back(*And);
  return And;
}

template <typename FuncTy>
static void applyForLoopnest(HLLoop *OutermostLoop, HLLoop *InnermostLoop,
                             FuncTy Func) {
  assert(OutermostLoop && "OutermostLoop should not be nullptr");
  assert(InnermostLoop && "InnermostLoop should not be nullptr");

  auto *Loop = InnermostLoop;
  auto *EndLoop = OutermostLoop->getParentLoop();

  do {
    Func(Loop);
    Loop = Loop->getParentLoop();
  } while (Loop != EndLoop);
}

static Type *getMinimalElementSizeType(const DataLayout &DL,
                                       ArrayRef<Segment> Segments) {
  assert(!Segments.empty() && "Segments is empty");

  uint64_t MinTypeSize = std::numeric_limits<uint64_t>::max();
  Type *MinType = nullptr;

  for (auto &Segment : Segments) {
    for (auto &Ref : {Segment.Lower, Segment.Upper}) {
      Type *RefElementType = Ref->getDereferencedType();
      auto Size = DL.getTypeSizeInBits(RefElementType);
      if (Size < MinTypeSize) {
        MinTypeSize = Size;
        MinType = RefElementType;
      }
    }
  }

  assert(MinType && "Can not find min type");
  return MinType;
}

// The method is using __intel_rtdd_indep function from libirc which returns
// zero if memory regions do not overlap.
//
// typedef struct {
//    void *start;
//    void *end;
// } mem_region_t;
//
// size_t __intel_rtdd_indep(mem_region_t *regions, size_t n);
//
// Each ref group would be represented by a mem_region_t.
HLIf *HIRRuntimeDD::createLibraryCallCondition(
    LoopContext &Context, HLIf *MasterIf, HLContainerTy &Nodes,
    SmallVectorImpl<unsigned> &NewSymbases) {
  auto &HNU = Context.Loop->getHLNodeUtils();
  auto &BU = HNU.getBlobUtils();
  auto &DRU = HNU.getDDRefUtils();
  auto &CEU = HNU.getCanonExprUtils();
  auto &LLVMContext = HNU.getContext();

  Type *I8PtrType = Type::getInt8PtrTy(LLVMContext, 0);
  Type *PtrElementType =
      getMinimalElementSizeType(HNU.getDataLayout(), Context.SegmentList);

  // TODO: bailout of library call logic if any ref is not in address space 0.
  Type *PtrType = PointerType::get(PtrElementType, 0);

  // Create a type for [N * %mem_region_t]
  Type *SegmentRuntimeTy = StructType::get(PtrType, PtrType);
  Type *SegmentArrayRuntimeTy =
      ArrayType::get(SegmentRuntimeTy, Context.SegmentList.size());

  // Create a temporary alloca to store mem_region_t.
  unsigned TestArrayBlobIndex = HNU.createAlloca(
      SegmentArrayRuntimeTy, Context.Loop->getParentRegion(), "dd");

  // New alloca should be set live-in to the loops up to the target loop.
  NewSymbases.push_back(BU.getTempBlobSymbase(TestArrayBlobIndex));

  auto IVType = Context.Loop->getIVType();
  unsigned TestIdx = 0;

  // Create stores of region addresses to the temporary alloca.
  // ex.:
  //   (%dd)[0].0 = &(%A)[%lower]
  //   (%dd)[0].1 = &(%A)[%upper]
  //   ...
  //   (%dd)[49].0 = &(%Q)[%lower]
  //   (%dd)[49].1 = &(%Q)[%upper]
  for (auto &S : Context.SegmentList) {
    RegDDRef *LBDDRef =
        DRU.createMemRef(SegmentArrayRuntimeTy, TestArrayBlobIndex);
    LBDDRef->addDimension(CEU.createCanonExpr(IVType));
    LBDDRef->addDimension(CEU.createCanonExpr(IVType, 0, TestIdx));
    LBDDRef->setTrailingStructOffsets(1, 0);

    RegDDRef *UBDDRef = LBDDRef->clone();
    UBDDRef->setTrailingStructOffsets(1, 1);

    normalizeRefTypes(HNU, Nodes, S.Lower, S.Upper, PtrElementType);
    Nodes.push_back(*HNU.createStore(S.Lower, "lb", LBDDRef));
    Nodes.push_back(*HNU.createStore(S.Upper, "ub", UBDDRef));

    TestIdx += 1;
  }

  AttrBuilder AB(LLVMContext);
  AB.addAttribute(Attribute::Speculatable)
      .addAttribute(Attribute::ReadOnly)
      .addAttribute(Attribute::ArgMemOnly);

  AttributeList Attrs =
      AttributeList::get(LLVMContext, AttributeList::FunctionIndex, AB);

  Type *IntPtrType = HNU.getDataLayout().getIntPtrType(PtrType);
  FunctionCallee RtddIndep = HNU.getModule().getOrInsertFunction(
      "__intel_rtdd_indep", Attrs, IntPtrType, I8PtrType, IntPtrType);

  RegDDRef *ArrayRef =
      DRU.createMemRef(SegmentArrayRuntimeTy, TestArrayBlobIndex);
  ArrayRef->setAddressOf(true);
  ArrayRef->addDimension(CEU.createCanonExpr(IVType));
  ArrayRef->setBitCastDestVecOrElemType(Type::getInt8Ty(LLVMContext));

  RegDDRef *SegementSizeRef =
      DRU.createConstDDRef(IntPtrType, Context.SegmentList.size());

  HLInst *Call = HNU.createCall(RtddIndep, {ArrayRef, SegementSizeRef});
  Nodes.push_back(*Call);

  RegDDRef *ZeroRef = DRU.createConstDDRef(IntPtrType, 0);

  // Add predicate (%__intel_rtdd_indep(...) == 0) where true indicates no
  // memory overlap.
  if (!MasterIf) {
    MasterIf = HNU.createHLIf(CmpInst::ICMP_EQ, Call->getLvalDDRef()->clone(),
                              ZeroRef);
  } else {
    MasterIf->addPredicate(CmpInst::ICMP_EQ, Call->getLvalDDRef()->clone(),
                           ZeroRef);
  }

  return MasterIf;
}

HLIf *HIRRuntimeDD::createCompareCondition(LoopContext &Context, HLIf *MasterIf,
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

HLIf *HIRRuntimeDD::createMasterCondition(
    LoopContext &Context, HLContainerTy &Nodes,
    SmallVectorImpl<unsigned> &NewLiveinSymbases) {
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
  case RTDDMethod::Compare:
    return createCompareCondition(Context, MasterIf, Nodes);
  case RTDDMethod::LibraryCall:
    return createLibraryCallCondition(Context, MasterIf, Nodes,
                                      NewLiveinSymbases);
  }

  llvm_unreachable("Unknown test generation method");
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static void createDbgMessages(HLLoop *NoAliasLoop, HLLoop *ClonedLoop,
                              SmallVectorImpl<unsigned> &NewLiveinSymbases,
                              const TargetLibraryInfo &TLI) {
  if (CreateDbgMessages == DbgMessage::None) {
    return;
  }

  std::string LoopInfoMsg;
  raw_string_ostream LIMO(LoopInfoMsg);

  LIMO << "[RTDD]: " << NoAliasLoop->getHLNodeUtils().getFunction().getName()
       << "(), <Loop " << NoAliasLoop->getNumber();
  if (NoAliasLoop->getDebugLoc()) {
    LIMO << "@";
    NoAliasLoop->getDebugLoc().print(LIMO);
  }
  LIMO << ">:";
  LIMO.flush();

  auto InsertDbgPuts = [&NewLiveinSymbases, TLI](HLLoop *Loop,
                                                 StringRef Message) {
    auto *Call = Loop->getHLNodeUtils().createDbgPuts(
        TLI, Loop->getParentRegion(), Message);
    assert(Call &&
           "Could not create 'puts' call - libraries are not available");

    HLNodeUtils::insertBefore(Loop, Call);

    NewLiveinSymbases.push_back(Call->getOperandDDRef(1)->getBasePtrSymbase());
  };

  if (CreateDbgMessages.getValue() & DbgMessage::Positive) {
    InsertDbgPuts(NoAliasLoop, LoopInfoMsg + " OK");
  }

  if (CreateDbgMessages.getValue() & DbgMessage::Negative) {
    InsertDbgPuts(ClonedLoop, LoopInfoMsg + " FAILED");
  }
}
#endif

void HIRRuntimeDD::generateHLNodes(LoopContext &Context,
                                   const TargetLibraryInfo &TLI) {
  (void)TLI;

  Context.Loop->extractZttPreheaderAndPostexit();

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

  // If the candidate loop is UNKNOWN, we convert it into a DO loop for the Then
  // block of the RTDD checks.
  if (NoAliasLoop->isUnknown()) {
    // Remove UNKNOWN-loop-specific elements: label, load for UB, bottom-test.
    HLNodeUtils::remove(NoAliasLoop->getHeaderLabel());
    auto *BottomTest = NoAliasLoop->getBottomTest();
    HLNodeUtils::remove(BottomTest->getPrevNode());
    HLNodeUtils::remove(BottomTest);

    auto *UBLoad = Context.UnknownLoopUBLoad;
    auto *UBMax = Context.UnknownLoopUBMax;

    // Insert the Load and Max HLInsts in HIR before the loop.
    // (Add live-in for the Max Lval, and make Rval consistent).
    HLNodeUtils::insertBefore(NoAliasLoop, UBLoad);
    HLNodeUtils::insertBefore(NoAliasLoop, UBMax);
    NoAliasLoop->addLiveInTemp(UBMax->getLvalDDRef()->getSymbase());
    UBMax->getRvalDDRef()->makeConsistent({UBLoad->getLvalDDRef()});

    // Set loop's Upper Bound and Stride to convert the UKNOWN loop into a DO
    // loop. (Call makeConsistent for Upper-ref after setting the Stride).
    NoAliasLoop->setUpperDDRef(UBMax->getLvalDDRef()->clone());
    NoAliasLoop->getStrideDDRef()->getSingleCanonExpr()->setConstant(1);
    NoAliasLoop->getUpperDDRef()->makeConsistent({UBMax->getLvalDDRef()});
  }

  OptReportBuilder &ORBuilder =
      NoAliasLoop->getHLNodeUtils().getHIRFramework().getORBuilder();

  // Remark: Loop multiversioned for Data Dependence
  ORBuilder(*NoAliasLoop)
      .addOrigin(25474u, 1)
      .addRemark(OptReportVerbosity::Low, 25228u);

  // Remark: Multiversioned Loop 2
  ORBuilder(*ClonedLoop).addOrigin(25474u, 2);

  HLContainerTy Nodes;
  SmallVector<unsigned, 1> NewLiveinSymbases;
  HLIf *MemcheckIf = createMasterCondition(Context, Nodes, NewLiveinSymbases);

  HLNodeUtils::insertBefore(NoAliasLoop, &Nodes);
  HLNodeUtils::insertBefore(NoAliasLoop, MemcheckIf);

  HLNodeUtils::moveAsFirstThenChild(MemcheckIf, NoAliasLoop);
  HLNodeUtils::insertAsFirstElseChild(MemcheckIf, ClonedLoop);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  createDbgMessages(NoAliasLoop, ClonedLoop, NewLiveinSymbases, TLI);
#endif

  // Add new live-in symbases to the parent loops.
  for (HLLoop *ParentLoop = MemcheckIf->getParentLoop(); ParentLoop != nullptr;
       ParentLoop = ParentLoop->getParentLoop()) {

    for (unsigned Symbase : NewLiveinSymbases) {
      ParentLoop->addLiveInTemp(Symbase);
    }
  }

  // Implementation Note: The transformation adds NoAlias/Scope metadata to the
  // original loop and creates a clone for the unmodified loop.
  // 1) When RTDD will be used on-demand the clients may continue to work
  //    with the original loop and just ignore dependencies.
  // 2) Adding metadata to the cloned loop will require DDG for the new loop or
  //    creation of a mapping mechanism between original and cloned
  //    DDRefs.
  markDDRefsIndep(Context);

  // Populate loop MVDelinearizableBlobIndices with base indices which were
  // delinearized.
  std::transform(
      Context.DelinearizedGroupIndices.begin(),
      Context.DelinearizedGroupIndices.end(),
      std::back_inserter(NoAliasLoop->getMVDelinearizableBlobIndices()),
      [&](unsigned Index) {
        return Context.Groups[Index].front()->getBasePtrBlobIndex();
      });

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(MemcheckIf);
  applyForLoopnest(
      NoAliasLoop, Context.InnermostLoop, [&LoopMapper](HLLoop *Loop) {
        auto MVTag = Loop->getNumber();
        Loop->setMVTag(MVTag);

        HLLoop *ElseCaseLoop = LoopMapper.getMapped(Loop);
        ElseCaseLoop->setMVTag(MVTag);
        ElseCaseLoop->markDoNotVectorize();
        ElseCaseLoop->markDoNotUnroll();

        if (Loop->isInnermost()) {
          HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Loop);
        }
      });
}

void HIRRuntimeDD::markDDRefsIndep(LoopContext &Context) {
  auto &LLVMContext =
      Context.Loop->getHLNodeUtils().getHIRFramework().getContext();
  RefGroupVecTy &Groups = Context.Groups;

  DenseMap<unsigned, unsigned> &SplitedGroupsOriginalIndices =
      Context.SplitedGroupsOriginalIndices;

  // The splited group should have the same ScopeId as its original group.
  // Thus, we need to merge the split group back to the original group.
  for (auto Idx : SplitedGroupsOriginalIndices) {
    unsigned GroupId = Idx.first;
    unsigned OriginalId = Idx.second;
    Groups[OriginalId].append(Groups[GroupId].begin(), Groups[GroupId].end());
  }

  auto Size = Groups.size() - SplitedGroupsOriginalIndices.size();

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

      MDNode *ScopeMD = MDNode::get(LLVMContext, NewScopes[ScopeId]);
      AANodes.Scope = MDNode::concatenate(AANodes.Scope, ScopeMD);

      SmallVector<Metadata *, ExpectedNumberOfTests> NoAliasScopes;
      NoAliasScopes.reserve(Size - 1);
      NoAliasScopes.append(NewScopes.begin(), NewScopes.begin() + ScopeId);
      NoAliasScopes.append(NewScopes.begin() + ScopeId + 1, NewScopes.end());

      MDNode *NoAliasScopesMD = MDNode::get(LLVMContext, NoAliasScopes);
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

  if (DisableLibraryCallSwitch ||
      !TTI.isAdvancedOptEnabled(
          TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelSSE42) ||
      !TTI.isLibIRCAllowed() || !TLI.has(LibFunc_qsort)) {
    LLVM_DEBUG(dbgs() << "[RTDD] Libraries are not available. The Library Call "
                         "Method will be disabled.\n");
    EnableLibraryCallMethod = false;
  }

  // Multiversion for memory aliasing.
  MemoryAliasAnalyzer AliasAnalyzer(*this);
  HIRF.getHLNodeUtils().visitAll(AliasAnalyzer);

  if (AliasAnalyzer.LoopContexts.size() != 0) {
    for (LoopContext &Candidate : AliasAnalyzer.LoopContexts) {
      generateHLNodes(Candidate, TLI);

      // Statistics
      ++LoopsMultiversioned;
      if (!Candidate.Loop->isInnermost()) {
        ++OuterLoopsMultiversioned;
      }
    }
  }

  return true;
}

PreservedAnalyses HIRRuntimeDDPass::runImpl(llvm::Function &F,
                                            llvm::FunctionAnalysisManager &AM,
                                            HIRFramework &HIRF) {
  HIRRuntimeDD(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
               AM.getResult<HIRLoopStatisticsAnalysis>(F),
               AM.getResult<TargetLibraryAnalysis>(F),
               AM.getResult<TargetIRAnalysis>(F),
               AM.getResult<HIRSafeReductionAnalysisPass>(F))
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
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRRuntimeDD(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
               getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
               getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F),
               getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F),
               getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR())
        .run();
  }
};

char HIRRuntimeDDLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRRuntimeDDLegacyPass, OPT_SWITCH, OPT_DESCR, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRRuntimeDDLegacyPass, OPT_SWITCH, OPT_DESCR, false, false)

FunctionPass *llvm::createHIRRuntimeDDPass() {
  return new HIRRuntimeDDLegacyPass();
}
