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

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"

#define OPT_SWITCH "hir-runtime-dd"
#define OPT_DESCR "HIR RuntimeDD Multiversioning"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<unsigned>
    MaximumNumberOfTests(OPT_SWITCH "-max-tests",
                         cl::init(ExpectedNumberOfTests), cl::Hidden,
                         cl::desc("Maximum number of runtime tests for loop."));

// This will count both innermost and outer transformations
STATISTIC(LoopsMultiversioned,
          "Number of loops multiversioned by runtime DD");

STATISTIC(OuterLoopsMultiversioned,
          "Number of outer loops multiversioned by runtime DD");

struct HIRRuntimeDD::LoopAnalyzer final : public HLNodeVisitorBase {
  SmallVector<LoopCandidate, 16> Candidates;

  LoopAnalyzer() : SkipNode(nullptr) {}

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

  void visit(HLLoop *Loop) {
    LoopCandidate Candidate;
    DEBUG(dbgs() << "Runtime DD for loop " << Loop->getNumber() << ":\n");
    RuntimeDDResult Result = HIRRuntimeDD::computeTests(Loop, Candidate);
    if (Result == OK) {
      SkipNode = Loop;

      Candidates.push_back(std::move(Candidate));
      LoopsMultiversioned++;

      if (!Loop->isInnermost()) {
        OuterLoopsMultiversioned++;
      }
    }
    DEBUG(dbgs() << "LOOPOPT_OPTREPORT: [RTDD] Loop " << Loop->getNumber()
                 << ": " << HIRRuntimeDD::getResultString(Result) << "\n");
  }

  bool skipRecursion(const HLNode *N) const override { return N == SkipNode; }

private:
  const HLNode *SkipNode;
};

IVSegment::IVSegment(const DDRefGrouping::RefGroupTy &Group) {
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
  auto DiffCE = CanonExprUtils::cloneAndSubtract(UpperCE, LowerCE, false);
  assert(DiffCE && " CanonExpr difference failed.");
  if (DiffCE->isIntConstant(&DiffValue)) {
    assert(DiffValue >= 0 && "Segment wrong direction");
  } else {
    llvm_unreachable("Non-constant segment length");
  }
  CanonExprUtils::destroy(DiffCE);
#endif
}

IVSegment::IVSegment(IVSegment &&Segment)
    : Lower(std::move(Segment.Lower)),
      Upper(std::move(Segment.Upper)),
      BaseCE(std::move(Segment.BaseCE)),
      IsWrite(std::move(Segment.IsWrite)) {

  Segment.Lower = nullptr;
  Segment.Upper = nullptr;
}

IVSegment::~IVSegment() {
  if (Lower) {
    DDRefUtils::destroy(Lower);
  }

  if (Upper) {
    DDRefUtils::destroy(Upper);
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
  bool HasChanged = false;

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
      CanonExpr *IVBlobExpr = CanonExprUtils::createExtCanonExpr(
          CE->getSrcType(), CE->getDestType(), CE->isSExt());
      IVBlobExpr->addBlob(IVBlobIndex, IVCoeff);

      // At this point IVBlobIndex is KnownPositive or KnownNegative, as we
      // dropped others as non supported
      // The utility checks both blob and coeff sign.
      if (HLNodeUtils::isKnownNegative(IVBlobExpr, InnerLoop)) {
        Direction *= -1;
      }

      CanonExprUtils::destroy(IVBlobExpr);
    } else {
      Direction *= IVCoeff;
    }

    // Get max reference depending on the direction
    const RegDDRef *Bound = (Direction > 0) ? MaxRef : MinRef;
    assert(Bound->isTerminalRef() && "DDRef should be a terminal reference.");

    // The relaxed mode is safe here as we know that upper bound is always non
    // negative
    assert(!Bound->getSingleCanonExpr()->isTrunc() &&
           "Truncations are not supported");

    bool Ret = CanonExprUtils::replaceIVByCanonExpr(
        CE, Level, Bound->getSingleCanonExpr(), true);
    assert(Ret &&
           "Assuming replace will always succeed as we already checked if both "
           "are mergeable.");
    (void)Ret;

    HasChanged = true;
  }

  if (HasChanged) {
    SmallVector<const RegDDRef *, 2> Aux { MaxRef, MinRef };
    Ref->makeConsistent(&Aux, Level);
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

      const CanonExpr *UpperBoundCE = LoopI->getUpperCanonExpr();

      // Check if CE and UpperBoundCE are mergeable and check if UpperBoundCE
      // denominator equals one as we will not be able to replace IV with such
      // upper bound. This is because b*(x/d) != (b*x)/d.
      if (UpperBoundCE->getDenominator() != 1 ||
          !CanonExprUtils::mergeable(CE, UpperBoundCE, true)) {
        return UPPER_SUB_TYPE_MISMATCH;
      }
      assert(CanonExprUtils::mergeable(CE, LoopI->getLowerCanonExpr(), true) &&
             "Assuming that the Lower bound is also mergeable if Upper is"
             "mergeable");

      auto IVBlobIndex = CE->getIVBlobCoeff(Level);
      if (IVBlobIndex != InvalidBlobIndex) {
        CanonExpr *IVBlobExpr = CanonExprUtils::createExtCanonExpr(
            CE->getSrcType(), CE->getDestType(), CE->isSExt());

        IVBlobExpr->addBlob(IVBlobIndex, CE->getIVConstCoeff(Level));

        bool IsKnownNonZero =
            HLNodeUtils::isKnownPositive(IVBlobExpr, InnermostLoop) ||
            HLNodeUtils::isKnownNegative(IVBlobExpr, InnermostLoop);

        CanonExprUtils::destroy(IVBlobExpr);
        if (!IsKnownNonZero) {
          return BLOB_IV_COEFF;
        }
      }
    }
  }

  return OK;
}

// The method will replace IV @ Level inside segment bounds, depending on
// direction of IV, constant and blob coefficients. The result segment represent
// lower and upper address accessed inside a loopnest.
void IVSegment::updateIVWithBounds(unsigned Level, const RegDDRef *LowerBound,
                                   const RegDDRef *UpperBound,
                                   const HLLoop *InnerLoop) {
  updateRefIVWithBounds(getLower(), Level, LowerBound, UpperBound,
      InnerLoop);
  updateRefIVWithBounds(getUpper(), Level, UpperBound, LowerBound,
      InnerLoop);
}

char HIRRuntimeDD::ID = 0;
INITIALIZE_PASS_BEGIN(HIRRuntimeDD, OPT_SWITCH, OPT_DESCR, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
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
    return "Blob IV coeffs are not supported yet.";
  case SAME_BASE:
    return "Multiple groups with the same base CE";
  case NON_DO_LOOP:
    return "Non DO loops are not supported";
  default:
    llvm_unreachable("Unexpected give up reason");
  }
}
#endif

RuntimeDDResult HIRRuntimeDD::processLoopnest(
    const HLLoop *OuterLoop, const HLLoop *InnermostLoop,
    SmallVectorImpl<IVSegment> &IVSegments,
    SmallVectorImpl<RuntimeDDResult> &SegmentConditions,
    bool &ShouldGenerateTripCount) {

  assert(InnermostLoop->isInnermost() &&
           "InnermostLoop is not an innermost loop");

  // Check the loopnest for applicability
  for (const HLLoop *LoopI = InnermostLoop, *LoopE = OuterLoop->getParentLoop();
       LoopI != LoopE; LoopI = LoopI->getParentLoop()) {
    if (!LoopI->isDo()) {
      return NON_DO_LOOP;
    }

    if (!LoopI->getStrideCanonExpr()->isIntConstant()) {
      return NON_CONSTANT_IV_STRIDE;
    }
  }

  unsigned SegmentCount = IVSegments.size();

  // Check every segment for the applicability
  for (unsigned I = 0; I < SegmentCount; ++I) {
    SegmentConditions.push_back(
        IVSegments[I].isSegmentSupported(OuterLoop, InnermostLoop));
  }

  // TotalTripCount is used only to decide should we generate runtime small trip
  // test or not.
  bool ConstantTripCount = true;
  uint64_t TotalTripCount = 1;

  // Replace every IV in segments with upper and lower bounds
  for (const HLLoop *LoopI = InnermostLoop, *LoopE = OuterLoop->getParentLoop();
       LoopI != LoopE; LoopI = LoopI->getParentLoop()) {
    // TotalTripCount is a minimal estimation of loopnest tripcount. Non-const
    // loops are treated as they execute at least once.
    int64_t TripCount;
    if (LoopI->isConstTripLoop(&TripCount)) {
      TotalTripCount *= TripCount;
      if (TotalTripCount >= SmallTripCountTest) {
        ShouldGenerateTripCount = false;
      }
    } else {
      ConstantTripCount = false;
    }

    auto LowerBoundRef = LoopI->getLowerDDRef();
    auto UpperBoundRef = LoopI->getUpperDDRef();
    auto Level = LoopI->getNestingLevel();

    for (unsigned I = 0; I < SegmentCount; ++I) {
      if (SegmentConditions[I] == OK) {
        IVSegments[I].updateIVWithBounds(Level, LowerBoundRef, UpperBoundRef,
            InnermostLoop);
      }
    }
  }

  if (ConstantTripCount && TotalTripCount < SmallTripCountTest) {
    return SMALL_TRIPCOUNT;
  }

  return OK;
}

bool HIRRuntimeDD::isGroupMemRefMatchForRTDD(const RegDDRef *Ref1,
                                             const RegDDRef *Ref2) {
  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return false;
  }

  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE())) {
    return false;
  }

  auto I = Ref1->canon_begin();
  auto J = Ref2->canon_begin();

  const CanonExpr *Result = CanonExprUtils::cloneAndSubtract(*I, *J, true);
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

  assert(Result->getConstant() != 0 && "Duplicate DDRef found.");

  return true;
}

RuntimeDDResult HIRRuntimeDD::computeTests(HLLoop *Loop,
                                           LoopCandidate &Candidate) {
  Candidate.Loop = Loop;
  Candidate.GenTripCountTest = true;

  if (Loop->getMVTag()) {
    return ALREADY_MV;
  }

  const HLLoop *InnermostLoop = Loop;
  if (!Loop->isInnermost() &&
      !HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop, false, false)) {
    return NON_PERFECT_LOOPNEST;
  }

  MemRefGatherer::MapTy RefMap;
  DDRefGrouping::RefGroupsTy Groups;

  // Gather references which are only inside a loop, excepting loop bounds,
  // pre-header and post-exit.
  MemRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(), RefMap);
  DEBUG(MemRefGatherer::dump(RefMap));

  MemRefGatherer::sort(RefMap);
  DEBUG(MemRefGatherer::dump(RefMap));
  MemRefGatherer::makeUnique(RefMap);
  DEBUG(MemRefGatherer::dump(RefMap));

  DDRefGrouping::createGroups(Groups, RefMap, isGroupMemRefMatchForRTDD);
  DEBUG(DDRefGrouping::dump(Groups));

  unsigned GroupSize = Groups.size();

  if (GroupSize < 2) {
    return NO_OPPORTUNITIES;
  }

  SmallVector<IVSegment, ExpectedNumberOfTests> IVSegments;
  SmallVector<RuntimeDDResult, ExpectedNumberOfTests> Supported;
  for (unsigned I = 0; I < GroupSize; ++I) {
    IVSegments.push_back(IVSegment(Groups[I]));
  }

  RuntimeDDResult Res;
  Res = processLoopnest(Loop, InnermostLoop, IVSegments, Supported,
                        Candidate.GenTripCountTest);
  if (Res != OK)  {
    return Res;
  }

  assert(IVSegments.size() == Supported.size() && "Elements of Supported array "
                                                  "should correspond to "
                                                  "elements in IVSegments");

  // Create pairs of segments to intersect and store them into
  // Candidate.SegmentList
  unsigned NumOfTests = 0;
  for (unsigned I = 0, IE = IVSegments.size() - 1; I < IE; ++I) {
    IVSegment &S1 = IVSegments[I];

    for (unsigned J = I + 1, JE = IVSegments.size(); J < JE; ++J) {
      IVSegment &S2 = IVSegments[J];

      if (S1.getLower()->getSymbase() != S2.getLower()->getSymbase()) {
        break;
      }

      // Skip Read-Read segments
      if (!S1.isWrite() && !S2.isWrite()) {
        continue;
      }

      // Skip loops with refs where base CEs are the same, as this
      // transformation mostly for cases with different pointers.
      if (CanonExprUtils::areEqual(S1.getBaseCE(), S2.getBaseCE())) {
        return SAME_BASE;
      }

      // Check if both segments are OK. Unsupported segment may
      // not be a problem, if there is no another overlapped segment.
      RuntimeDDResult Res;
      Res = Supported[I];
      if (Res != OK) {
        return Res;
      }
      Res = Supported[J];
      if (Res != OK) {
        return Res;
      }

      Candidate.SegmentList.push_back(S1.genSegment());
      Candidate.SegmentList.push_back(S2.genSegment());

      NumOfTests++;
      if (NumOfTests > MaximumNumberOfTests) {
        return TOO_MANY_TESTS;
      }
    }
  }

  if (Candidate.SegmentList.size() == 0) {
    return NO_OPPORTUNITIES;
  }

  return OK;
}

HLIf *HIRRuntimeDD::createIfStmtForIntersection(HLContainerTy &Nodes,
                                                Segment &S1,
                                                Segment &S2) const {
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

    HLInst *BCIL = HLNodeUtils::createBitCast(DestType, BS->Lower);
    HLInst *BCIU = HLNodeUtils::createBitCast(DestType, BS->Upper);
    Nodes.push_back(BCIL);
    Nodes.push_back(BCIU);

    BS->Lower = BCIL->getLvalDDRef()->clone();
    BS->Upper = BCIU->getLvalDDRef()->clone();
  }

  HLIf *If = HLNodeUtils::createHLIf(PredicateTy::ICMP_UGE, S1.Upper, S2.Lower);
  If->addPredicate(PredicateTy::ICMP_UGE, S2.Upper, S1.Lower);

  Nodes.push_back(If);
  return If;
}

void HIRRuntimeDD::generateDDTest(LoopCandidate &Candidate) const {
  Candidate.Loop->extractPreheaderAndPostexit();

  HLLabel *OrigLabel = HLNodeUtils::createHLLabel("mv.orig");
  HLLabel *EscapeLabel = HLNodeUtils::createHLLabel("mv.escape");

  HLGoto *OrigGoto = HLNodeUtils::createHLGoto(OrigLabel);

  HLNodeUtils::insertBefore(Candidate.Loop, OrigLabel);
  HLNodeUtils::insertAfter(Candidate.Loop, EscapeLabel);

  // Generate tripcount test
  if (Candidate.GenTripCountTest) {
    // TODO: generation of small tripcount tests for a loopnest
    uint64_t MinTripCount = SmallTripCountTest;
    RegDDRef *TripCountRef = Candidate.Loop->getTripCountDDRef();
    assert(TripCountRef != nullptr &&
           "getTripCountDDRef() unexpectedly returned nullptr");
    HLIf *LowTripCountIf =
        HLNodeUtils::createHLIf(PredicateTy::ICMP_ULT, TripCountRef,
                                DDRefUtils::createConstDDRef(
                                    TripCountRef->getDestType(), MinTripCount));

    HLNodeUtils::insertAsFirstChild(LowTripCountIf, OrigGoto, true);
    HLNodeUtils::insertBefore(OrigLabel, LowTripCountIf);
  }
  //////////////////////////

  unsigned RefsCount = Candidate.SegmentList.size();
  for (unsigned i = 0; i < RefsCount; i += 2) {
    auto &S1 = Candidate.SegmentList[i];
    auto &S2 = Candidate.SegmentList[i + 1];

    HLContainerTy Nodes;
    HLIf *DDCheck = createIfStmtForIntersection(Nodes, S1, S2);

    HLNodeUtils::insertAsFirstChild(DDCheck, OrigGoto->clone(), true);
    HLNodeUtils::insertBefore(OrigLabel, &Nodes);
  }

  HLGoto *EscapeGoto = HLNodeUtils::createHLGoto(EscapeLabel);
  HLNodeUtils::insertBefore(OrigLabel, EscapeGoto);

  HLLoop *MVLoop = Candidate.Loop->clone();
  unsigned MVTag = Candidate.Loop->getNumber();
  Candidate.Loop->setMVTag(MVTag);
  MVLoop->setMVTag(MVTag);
  // TODO: Mark MVLoop ddrefs to say HIRDDAnalysis that they do not intersect.

  HLNodeUtils::insertBefore(EscapeGoto, MVLoop);

  HLRegion *ParentRegion = Candidate.Loop->getParentRegion();
  assert(ParentRegion && "Processed loop is not attached.");
  ParentRegion->setGenCode(true);

  if (HLLoop *ParentLoop = Candidate.Loop->getParentLoop()) {
    HIRInvalidationUtils::invalidateBody(ParentLoop);
  } else {
    HIRInvalidationUtils::invalidateNonLoopRegion(ParentRegion);
  }
}

bool HIRRuntimeDD::runOnFunction(Function &F) {
  DEBUG(dbgs() << "HIRRuntimeDD for function: " << F.getName() << "\n");

  LoopAnalyzer LA;
  HLNodeUtils::visitAll(LA);

  if (LA.Candidates.size() == 0) {
    return false;
  }

  for (LoopCandidate &Candidate : LA.Candidates) {
    generateDDTest(Candidate);
  }

  return true;
}

void HIRRuntimeDD::releaseMemory() {}
