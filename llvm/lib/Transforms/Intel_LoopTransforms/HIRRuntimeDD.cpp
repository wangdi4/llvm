//===- HIRRuntimeDD.cpp - Implements Multiversioning for Runtime DD -=========//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
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
// TODO: Handle mem refs with a blob IV coefficient.
// TODO: Attach noalias metadata to RegDDRefs to tell DDA and help other passes
//       to avoid dependency, eliminated by runtime tests.
//
//===----------------------------------------------------------------------===//

#include "HIRRuntimeDD.h"

#include "llvm/Pass.h"

#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/MDBuilder.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include <memory>

#define OPT_SWITCH "hir-runtime-dd"
#define OPT_DESCR "HIR RuntimeDD Multiversioning"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::runtimedd;

static cl::opt<bool> DisableRuntimeDD("disable-" OPT_SWITCH, cl::init(false),
                                      cl::Hidden,
                                      cl::desc("Disable " OPT_DESCR "."));

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
  HIRRuntimeDD *RDD;

  MemoryAliasAnalyzer(HIRRuntimeDD *RDD) : RDD(RDD), SkipNode(nullptr) {}

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

  void visit(HLLoop *Loop) {
    LoopContext Context;
    DEBUG(dbgs() << "Runtime DD for loop " << Loop->getNumber() << ":\n");

    RuntimeDDResult Result = RDD->computeTests(Loop, Context);
    bool IsInnermost = Loop->isInnermost();

    if (Result == OK) {
      SkipNode = Loop;

      LoopContexts.push_back(std::move(Context));
      LoopsMultiversioned++;

      if (!IsInnermost) {
        OuterLoopsMultiversioned++;
      }
    } else if (IsInnermost || analyzedRefs(Result)) {
      SkipNode = Loop;
    }

    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: [RTDD] Loop " << Loop->getNumber()
                 << ": " << HIRRuntimeDD::getResultString(Result) << "\n");
  }

  bool skipRecursion(const HLNode *N) const override { return N == SkipNode; }

  bool analyzedRefs(RuntimeDDResult Result) const {
    // If we have analyzed the memrefs inside this loop, we can skip recursing
    // into it. The following result types are the ones where we haven't
    // analyzed the memrefs.
    return (Result != ALREADY_MV) && (Result != NON_DO_LOOP) &&
           (Result != NON_PROFITABLE) && (Result != NON_PERFECT_LOOPNEST) &&
           (Result != BLOB_IV_COEFF);
  }

private:
  const HLNode *SkipNode;
};

IVSegment::IVSegment(const RefGroupTy &Group) {
  Lower = Group.front()->clone();
  Upper = Group.back()->clone();

  IsWrite = std::any_of(Group.begin(), Group.end(),
                        [](const RegDDRef *Ref) { return Ref->isLval(); });

  BaseCE = Lower->getBaseCE();

  assert(CanonExprUtils::areEqual(BaseCE, Upper->getBaseCE()) &&
         "Unexpected group. Left and Right refs should have the same base.");

#ifndef NDEBUG
  int64_t DiffValue;
  CanonExpr *LowerCE = *Lower->canon_begin();
  CanonExpr *UpperCE = *Upper->canon_begin();
  auto DiffCE =
      UpperCE->getCanonExprUtils().cloneAndSubtract(UpperCE, LowerCE, false);
  assert(DiffCE && " CanonExpr difference failed.");
  DiffCE->simplify(true);
  if (DiffCE->isIntConstant(&DiffValue)) {
    assert(DiffValue >= 0 && "Segment wrong direction");
  } else {
    llvm_unreachable("Non-integer non-constant segment length");
  }
  UpperCE->getCanonExprUtils().destroy(DiffCE);
#endif
}

IVSegment::IVSegment(IVSegment &&Segment)
    : Lower(std::move(Segment.Lower)), Upper(std::move(Segment.Upper)),
      BaseCE(std::move(Segment.BaseCE)), IsWrite(std::move(Segment.IsWrite)) {

  Segment.Lower = nullptr;
  Segment.Upper = nullptr;
}

IVSegment::~IVSegment() {
  if (Lower) {
    Lower->getDDRefUtils().destroy(Lower);
  }

  if (Upper) {
    Upper->getDDRefUtils().destroy(Upper);
  }
}

// Clone bounds and set isAddressOf flag.
Segment IVSegment::genSegment() const {
  auto *Ref1 = getLower()->clone();
  auto *Ref2 = getUpper()->clone();

  Ref1->setAddressOf(true);
  Ref2->setAddressOf(true);
  return Segment(Ref1, Ref2);
}

// The method replaces IV @ Level inside Ref with MaxRef or MinRef depending on
// the IV direction
void IVSegment::updateRefIVWithBounds(RegDDRef *Ref, unsigned Level,
                                      const RegDDRef *MaxRef,
                                      const RegDDRef *MinRef,
                                      const HLLoop *InnerLoop) {
  for (auto CEI = Ref->canon_begin(), CEE = Ref->canon_end(); CEI != CEE;
       ++CEI) {
    CanonExpr *CE = *CEI;

    unsigned IVBlobIndex;
    int64_t IVCoeff;
    CE->getIVCoeff(Level, &IVBlobIndex, &IVCoeff);

    if (IVCoeff == 0) {
      continue;
    }

    // Determine IV direction: C*B*i, get C and B signs.
    int64_t Direction = 1;
    if (IVBlobIndex != InvalidBlobIndex) {
      // IVBlobExpr is a helper CE to use HLNodeUtils::isKnownNegative
      std::unique_ptr<CanonExpr> IVBlobExpr(
          CE->getCanonExprUtils().createExtCanonExpr(
              CE->getSrcType(), CE->getDestType(), CE->isSExt()));
      IVBlobExpr->addBlob(IVBlobIndex, IVCoeff);

      // At this point IVBlobIndex is KnownPositive or KnownNegative, as we
      // dropped others as non supported
      // The utility checks both blob and coeff sign.
      if (InnerLoop->getHLNodeUtils().isKnownNegative(IVBlobExpr.get(),
                                                      InnerLoop)) {
        Direction *= -1;
      }
    } else {
      Direction *= IVCoeff;
    }

    // Get max reference depending on the direction
    const RegDDRef *Bound = (Direction > 0) ? MaxRef : MinRef;
    assert(Bound->isTerminalRef() && "DDRef should be a terminal reference.");

    const CanonExpr *BoundCE = Bound->getSingleCanonExpr();

    // The relaxed mode is safe here as we know that upper bound is always non
    // negative
    assert(!BoundCE->isTrunc() && "Truncations are not supported");

    bool Ret;
    if (BoundCE->getDenominator() == 1 &&
        CanonExprUtils::mergeable(CE, BoundCE, true)) {
      Ret = CE->getCanonExprUtils().replaceIVByCanonExpr(CE, Level, BoundCE,
                                                         true);
      CE->simplify(false);
    } else {
      // Have to treat bound as blob and then truncate or extend it.
      std::unique_ptr<CanonExpr> NewBoundCE(BoundCE->clone());

      Ret = NewBoundCE->castStandAloneBlob(CE->getSrcType(), false);

      assert(Ret && "convertToStandAloneBlob() should always succeed as we"
                    "already checked if it's convertible");

      Ret = CE->getCanonExprUtils().replaceIVByCanonExpr(
          CE, Level, NewBoundCE.get(), true);
    }
    assert(Ret &&
           "Assuming replace will always succeed as we already checked if both "
           "are mergeable.");
    (void)Ret;
  }
}

RuntimeDDResult
IVSegment::isSegmentSupported(const HLLoop *OuterLoop,
                              const HLLoop *InnermostLoop) const {

  if (getBaseCE()->isNonLinear()) {
    return NON_LINEAR_BASE;
  }

  const RegDDRef *Lower = getLower();

  // We will be replacing every IV inside a RegDDRef: a[i+j+k][j][k]. So we have
  // to check all canon expressions against UB of every loop in loopnest.
  // We skip loops if its IV is absent.
  for (auto I = Lower->canon_begin(), E = Lower->canon_end(); E != I; ++I) {
    CanonExpr *CE = *I;

    if (CE->isNonLinear()) {
      return NON_LINEAR_SUBS;
    }

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

        if (!InnermostLoop->getHLNodeUtils().isKnownPositiveOrNegative(
                IVBlobExpr.get(), InnermostLoop)) {
          return BLOB_IV_COEFF;
        }
      }
    }
  }

  return OK;
}

void IVSegment::makeConsistent(const SmallVectorImpl<const RegDDRef *> &AuxRefs,
                               unsigned Level) {
  Lower->makeConsistent(&AuxRefs, Level);
  Upper->makeConsistent(&AuxRefs, Level);
}

// The method will replace IV @ Level inside segment bounds, depending on
// direction of IV, constant and blob coefficients. The result segment represent
// lower and upper address accessed inside a loopnest.
void IVSegment::updateIVWithBounds(unsigned Level, const RegDDRef *LowerBound,
                                   const RegDDRef *UpperBound,
                                   const HLLoop *InnerLoop) {
  updateRefIVWithBounds(getLower(), Level, LowerBound, UpperBound, InnerLoop);
  updateRefIVWithBounds(getUpper(), Level, UpperBound, LowerBound, InnerLoop);
}

char HIRRuntimeDD::ID = 0;
INITIALIZE_PASS_BEGIN(HIRRuntimeDD, OPT_SWITCH, OPT_DESCR, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
INITIALIZE_PASS_END(HIRRuntimeDD, OPT_SWITCH, OPT_DESCR, false, false)

FunctionPass *llvm::createHIRRuntimeDDPass() { return new HIRRuntimeDD(); }

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
  case BLOB_IV_COEFF:
    return "Unknown Blob IV coeffs are not supported yet.";
  case SAME_BASE:
    return "Multiple groups with the same base CE";
  case NON_DO_LOOP:
    return "Non DO loops are not supported";
  case NON_PROFITABLE:
    return "Loop considered non-profitable";
  case NON_PROFITABLE_SUBS:
    return "Subscript multiversioning is non-profitable";
  case STRUCT_ACCESS:
    return "Struct refs not supported yet";
  default:
    llvm_unreachable("Unexpected give up reason");
  }
}
#endif

bool HIRRuntimeDD::isProfitable(const HLLoop *Loop) {
  if (DisableCostModel) {
    return true;
  }

  const LoopStatistics &LS = HLS->getSelfLoopStatistics(Loop);

  return (!LS.hasCalls() && !LS.hasSwitches() && !LS.hasIfs());
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
    auto LowerBoundRef = LoopI->getLowerDDRef();
    auto UpperBoundRef = LoopI->getUpperDDRef();
    AuxRefs.push_back(LowerBoundRef);
    AuxRefs.push_back(UpperBoundRef);

    auto Level = LoopI->getNestingLevel();

    for (unsigned I = 0; I < SegmentCount; ++I) {
      IVSegments[I].updateIVWithBounds(Level, LowerBoundRef, UpperBoundRef,
                                       InnermostLoop);
    }
  }

  for (unsigned I = 0; I < SegmentCount; ++I) {
    IVSegments[I].makeConsistent(AuxRefs, OuterLoop->getNestingLevel() - 1);
  }
}

bool HIRRuntimeDD::isGroupMemRefMatchForRTDD(const RegDDRef *Ref1,
                                             const RegDDRef *Ref2) {

  // TODO: Temporary workaround to make it easier to bail out on loops with
  // structure access.
  if (Ref1->accessesStruct() || Ref2->accessesStruct()) {
    return false;
  }

  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return false;
  }

  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE())) {
    return false;
  }

  auto I = Ref1->canon_begin();
  auto J = Ref2->canon_begin();

  const CanonExpr *Result =
      Ref1->getCanonExprUtils().cloneAndSubtract(*I, *J, true);
  if (!Result) {
    return false;
  }

  if (Result->hasBlob() || Result->hasIV()) {
    return false;
  }

  ++I;
  ++J;

  for (auto E = Ref1->canon_end(); I != E; ++I, ++J) {
    if (!CanonExprUtils::areEqual(*I, *J)) {
      return false;
    }
  }

  return true;
}

unsigned HIRRuntimeDD::findAndGroup(RefGroupVecTy &Groups, RegDDRef *Ref) {
  for (auto I = Groups.begin(), E = Groups.end(); I != E; ++I) {
    if (isGroupMemRefMatchForRTDD(I->front(), Ref)) {
      I->push_back(Ref);
      return std::distance(Groups.begin(), I);
    }
  }

  unsigned NewGroupNum = Groups.size();
  Groups.resize(NewGroupNum + 1);
  Groups.back().push_back(Ref);
  return NewGroupNum;
}

RuntimeDDResult HIRRuntimeDD::computeTests(HLLoop *Loop, LoopContext &Context) {
  Context.Loop = Loop;
  Context.GenTripCountTest = true;

  if (Loop->getMVTag()) {
    return ALREADY_MV;
  }

  // TODO: add a lit test when we start to support unknown loops
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

  // TotalTripCount is used only to decide should we generate runtime small trip
  // test or not.
  bool ConstantTripCount = true;
  uint64_t TotalTripCount = 1;

  for (const HLLoop *LoopI = InnermostLoop, *LoopE = Loop->getParentLoop();
       LoopI != LoopE; LoopI = LoopI->getParentLoop()) {
    uint64_t TripCount;
    if (LoopI->isConstTripLoop(&TripCount)) {
      TotalTripCount *= TripCount;
      if (TotalTripCount >= SmallTripCountTest) {
        Context.GenTripCountTest = false;
      }
    } else {
      ConstantTripCount = false;
    }
  }

  if (!DisableCostModel && ConstantTripCount &&
      TotalTripCount < SmallTripCountTest) {
    return SMALL_TRIPCOUNT;
  }

  RefGroupVecTy &Groups = Context.Groups;
  MemRefGatherer::VectorTy Refs;
  SmallSetVector<std::pair<unsigned, unsigned>, ExpectedNumberOfTests> Tests;

  auto &DDA = getAnalysis<HIRDDAnalysis>();
  DDGraph DDG = DDA.getGraph(Loop);
  DEBUG(dbgs() << "Loop DDG:\n");
  DEBUG(DDG.dump());

  // Gather references which are only inside a loop, excepting loop bounds,
  // pre-header and post-exit.
  MemRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(), Refs);
  DEBUG(dbgs() << "Loop references:\n");
  DEBUG(MemRefGatherer::dump(Refs));

  for (RegDDRef *SrcRef : Refs) {
    unsigned GroupA = findAndGroup(Groups, SrcRef);

    for (const DDEdge *Edge : DDG.outgoing(SrcRef)) {
      assert(!Edge->isINPUTdep() && "Input edges are unexpected");

      RegDDRef *DstRef = cast<RegDDRef>(Edge->getSink());
      unsigned GroupB = findAndGroup(Groups, DstRef);

      if (GroupA != GroupB) {
        // Skip loops with refs where base CEs are the same, as this
        // transformation mostly for cases with different pointers.
        if (CanonExprUtils::areEqual(SrcRef->getBaseCE(), DstRef->getBaseCE(),
                                     true)) {
          return SAME_BASE;
        }

        auto TestPair = GroupA > GroupB ? std::make_pair(GroupB, GroupA)
                                        : std::make_pair(GroupA, GroupB);

        Tests.insert(TestPair);
      }
    }
  }

  if (Tests.size() < 1) {
    return NO_OPPORTUNITIES;
  } else if (Tests.size() > MaximumNumberOfTests) {
    return TOO_MANY_TESTS;
  }

  for (RefGroupTy &Group : Groups) {
    std::sort(Group.begin(), Group.end(), DDRefUtils::compareMemRef);
  }

  DEBUG(DDRefGrouping::dump(Groups));

  unsigned GroupSize = Groups.size();

  SmallVector<IVSegment, ExpectedNumberOfTests> IVSegments;
  for (unsigned I = 0; I < GroupSize; ++I) {
    if (Groups[I].front()->accessesStruct()) {
      return STRUCT_ACCESS;
    }
    IVSegments.emplace_back(Groups[I]);

    // Check every segment for the applicability
    RuntimeDDResult Ret =
        IVSegments.back().isSegmentSupported(Loop, InnermostLoop);
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

    assert(S1.getLower()->getSymbase() == S2.getLower()->getSymbase() &&
           "Segment symbases should be equal");

    // Skip Read-Read segments
    assert((S1.isWrite() || S2.isWrite()) &&
           "At least of the one segments should be a write segment");

    Context.SegmentList.push_back(S1.genSegment());
    Context.SegmentList.push_back(S2.genSegment());
  }

  return OK;
}

HLInst *HIRRuntimeDD::createIntersectionCondition(HLNodeUtils &HNU,
                                                  HLContainerTy &Nodes,
                                                  Segment &S1, Segment &S2) {
  Segment *S[] = {&S1, &S2};
  Type *S1Type = S[0]->getType()->getPointerElementType();
  Type *S2Type = S[1]->getType()->getPointerElementType();

  // In case of different types, bitcast one segment bounds to another to
  // be in compliance with LLVM IR. (see ex. in lit test ptr-types.ll)
  if (S1Type != S2Type) {
    unsigned BiggerTypeIdx =
        S1Type->getPrimitiveSizeInBits() > S2Type->getPrimitiveSizeInBits() ? 0
                                                                            : 1;

    Segment *BS = S[BiggerTypeIdx];
    Type *DestType = S[!BiggerTypeIdx]->getType();

    HLInst *BCIL = HNU.createBitCast(DestType, BS->Lower);
    HLInst *BCIU = HNU.createBitCast(DestType, BS->Upper);
    Nodes.push_back(*BCIL);
    Nodes.push_back(*BCIU);

    BS->Lower = BCIL->getLvalDDRef()->clone();
    BS->Upper = BCIU->getLvalDDRef()->clone();
  }

  HLInst *Cmp1 =
      HNU.createCmp(PredicateTy::ICMP_UGE, S1.Upper, S2.Lower, "mv.test");
  HLInst *Cmp2 =
      HNU.createCmp(PredicateTy::ICMP_UGE, S2.Upper, S1.Lower, "mv.test");
  HLInst *And = HNU.createAnd(Cmp1->getLvalDDRef()->clone(),
                              Cmp2->getLvalDDRef()->clone(), "mv.and");

  Nodes.push_back(*Cmp1);
  Nodes.push_back(*Cmp2);
  Nodes.push_back(*And);
  return And;
}

void HIRRuntimeDD::generateDDTest(LoopContext &Context) {
  Context.Loop->extractZtt();
  Context.Loop->extractPreheaderAndPostexit();

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
  //   if (<low trip test> && %and-0 == F && ... && %and-n == F) {
  //     <Modified loop>
  //   } else {
  //     <Original loop>
  //   }
  //
  //   <PostExit>
  // }

  HLLoop *ModifiedLoop = Context.Loop;
  HLLoop *OrigLoop = Context.Loop->clone();

  HLIf *MemcheckIf = nullptr;
  auto &HNU = OrigLoop->getHLNodeUtils();

  /// Generate tripcount test
  if (Context.GenTripCountTest) {
    // TODO: generation of small tripcount tests for a loopnest
    uint64_t MinTripCount = SmallTripCountTest;
    RegDDRef *TripCountRef = Context.Loop->getTripCountDDRef();
    assert(TripCountRef != nullptr &&
           "getTripCountDDRef() unexpectedly returned nullptr");
    MemcheckIf = HNU.createHLIf(PredicateTy::ICMP_UGE, TripCountRef,
                                HNU.getDDRefUtils().createConstDDRef(
                                    TripCountRef->getDestType(), MinTripCount));
  }
  //////////////////////////

  HLContainerTy Nodes;
  SmallVector<RegDDRef *, 2 * ExpectedNumberOfTests> TestDDRefs;
  unsigned RefsCount = Context.SegmentList.size();
  for (unsigned i = 0; i < RefsCount; i += 2) {
    auto &S1 = Context.SegmentList[i];
    auto &S2 = Context.SegmentList[i + 1];

    HLInst *And =
        createIntersectionCondition(OrigLoop->getHLNodeUtils(), Nodes, S1, S2);
    TestDDRefs.push_back(And->getLvalDDRef()->clone());
  }
  HNU.insertBefore(Context.Loop, &Nodes);

  Type *Ty = TestDDRefs.front()->getDestType();
  if (!MemcheckIf) {
    MemcheckIf = HNU.createHLIf(PredicateTy::ICMP_EQ, TestDDRefs.front(),
                                HNU.getDDRefUtils().createConstDDRef(Ty, 0));
  } else {
    MemcheckIf->addPredicate(PredicateTy::ICMP_EQ, TestDDRefs.front(),
                             HNU.getDDRefUtils().createConstDDRef(Ty, 0));
  }

  for (auto I = std::next(TestDDRefs.begin()), E = TestDDRefs.end(); I != E;
       ++I) {
    MemcheckIf->addPredicate(PredicateTy::ICMP_EQ, (*I),
                             HNU.getDDRefUtils().createConstDDRef(Ty, 0));
  }

  HNU.insertBefore(Context.Loop, MemcheckIf);

  HNU.moveAsFirstChild(MemcheckIf, ModifiedLoop, true);
  HNU.insertAsFirstChild(MemcheckIf, OrigLoop, false);

  unsigned MVTag = ModifiedLoop->getNumber();
  ModifiedLoop->setMVTag(MVTag);
  OrigLoop->setMVTag(MVTag);

  OrigLoop->markDoNotVectorize();

  // Implementation Note: The transformation adds NoAlias/Scope metadata to the
  // original loop and creates a clone for the unmodified loop.
  // 1) When RTDD will be used on-demand the clients may continue to work
  //    with the original loop and just ignore dependencies.
  // 2) Adding metadata to the cloned loop will require DDG for the new loop or
  //    creation of a mapping mechanism between original and cloned
  //    DDRefs.
  markDDRefsIndep(Context);

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(ModifiedLoop);
  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(ModifiedLoop);
}

void HIRRuntimeDD::markDDRefsIndep(LoopContext &Context) {
  RefGroupVecTy &Groups = Context.Groups;

  auto Size = Groups.size();
  MDBuilder MDB(Context.Loop->getHLNodeUtils().getHIRFramework().getContext());

  MDNode *Domain = MDB.createAnonymousAliasScopeDomain();
  SmallVector<MDNode *, ExpectedNumberOfTests> NewScopes;
  NewScopes.reserve(Size);
  for (unsigned I = 0; I < Size; ++I) {
    NewScopes.push_back(MDB.createAnonymousAliasScope(Domain));
  }

  for (unsigned I = 0, E = Groups.size(); I < E; ++I) {
    auto ScopeId = I;

    for (RegDDRef *Ref : Groups[I]) {
      AAMDNodes AANodes;
      Ref->getAAMetadata(AANodes);

      AANodes.Scope = MDNode::concatenate(AANodes.Scope, NewScopes[ScopeId]);

      for (unsigned I = 0; I < ScopeId; ++I) {
        AANodes.NoAlias = MDNode::concatenate(AANodes.NoAlias, NewScopes[I]);
      }
      for (unsigned I = ScopeId + 1; I < Size; ++I) {
        AANodes.NoAlias = MDNode::concatenate(AANodes.NoAlias, NewScopes[I]);
      }

      Ref->setAAMetadata(AANodes);
    }
  }
}

bool HIRRuntimeDD::runOnFunction(Function &F) {
  if (DisableRuntimeDD || skipFunction(F)) {
    return false;
  }

  HLS = &getAnalysis<HIRLoopStatistics>();
  auto &HIRF = getAnalysis<HIRFramework>();
  auto &HNU = HIRF.getHLNodeUtils();

  DEBUG(dbgs() << "HIRRuntimeDD for function: " << F.getName() << "\n");

  // Multiversion for memory aliasing.
  MemoryAliasAnalyzer AliasAnalyzer(this);
  HNU.visitAll(AliasAnalyzer);

  if (AliasAnalyzer.LoopContexts.size() != 0) {
    for (LoopContext &Candidate : AliasAnalyzer.LoopContexts) {
      generateDDTest(Candidate);
    }
  }

  return true;
}

void HIRRuntimeDD::releaseMemory() {}
