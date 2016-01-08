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

#include "llvm/Pass.h"

#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/SymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGrouping.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"

#define OPT_SWITCH "hir-runtime-dd"
#define OPT_DESCR "HIR RuntimeDD Multiversioning"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static const unsigned ExpectedNumberOfTests = 8;

static cl::opt<unsigned>
    MaximumNumberOfTests(OPT_SWITCH "-max-tests",
        cl::init(ExpectedNumberOfTests), cl::Hidden,
        cl::desc("Maximum number of runtime tests for loop."));

STATISTIC(LoopsMultiversioned, "Number of loops multiversioned by runtime DD");

namespace {

// The struct represents a segment of memory. It is used to construct checks
// for memory intersection
struct Segment {
  RegDDRef *Lower;
  RegDDRef *Upper;
  const CanonExpr *BaseCE;

  Segment(RegDDRef *Lower, RegDDRef *Upper)
  : Lower(Lower), Upper(Upper) {
    BaseCE = Lower->getBaseCE();
  }

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dump() {
    dbgs() << "[";
    Lower->dump();
    dbgs() << ", ";
    Upper->dump();
    dbgs() << "]\n";
  }
#endif
};

// The class represents a floating constant length memory segment that depends
// on the IV. This class is used to generate specific memory segments where
// IV is replaced with a Lower and Upper bound.
class IVSegment {
  const RegDDRef *Lower;
  const RegDDRef *Upper;
  const CanonExpr *BaseCE;

  unsigned Symbase1;
  unsigned Symbase2;

  bool IsWrite;

  static void replaceIVByCanonExpr(CanonExpr *Expr, unsigned Level,
      const CanonExpr *By);

  static RegDDRef *genAddressOfAccess(const RegDDRef *Ref, unsigned Level,
      const CanonExpr *Expr);

public:
  IVSegment(const DDRefGrouping::RefGroupTy &Group);

  Segment genLUSegment(unsigned Level, int64_t Stride,
      const CanonExpr *LCE, const CanonExpr *UCE) const;

  bool isWrite() const {
    return IsWrite;
  }

  const RegDDRef *getLower() const {
    return Lower;
  }

  const RegDDRef *getUpper() const {
    return Upper;
  }

  const CanonExpr *getBaseCE() const {
    return BaseCE;
  }

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dump() {
    dbgs() << "[";
    Lower->dump();
    dbgs() << ", ";
    Upper->dump();
    dbgs() << "]\n";
  }
#endif
};

IVSegment::IVSegment(const DDRefGrouping::RefGroupTy &Group) {
  Lower = Group.front();
  Upper = Group.back();

  IsWrite = std::any_of(Group.begin(), Group.end(), [](const RegDDRef *Ref) {
    return Ref->isLval();
  });

  BaseCE = Lower->getBaseCE();

  Symbase1 = DDRefUtils::getNewSymbase();
  Symbase2 = DDRefUtils::getNewSymbase();

  assert(CanonExprUtils::areEqual(BaseCE, Upper->getBaseCE()) &&
      "Unexprected group. Left and Right refs should have the same base.");

#ifndef NDEBUG
  int64_t DiffValue;
  CanonExpr *LowerCE = *Lower->canon_begin();
  CanonExpr *UpperCE = *Upper->canon_begin();
  auto DiffCE = CanonExprUtils::cloneAndSubtract(UpperCE, LowerCE, false);
  if (DiffCE->isIntConstant(&DiffValue)) {
    assert(DiffValue >= 0 && "Segment wrong direction");
  } else {
    llvm_unreachable("Non-constant segment length");
  }
  CanonExprUtils::destroy(DiffCE);
#endif
}

// Generates Segment, by replacing IV at Level with the Lower CE and Upper CE.
Segment IVSegment::genLUSegment(unsigned Level, int64_t Stride,
    const CanonExpr *LCE, const CanonExpr *UCE) const {
  int64_t IVCoeff = (*getLower()->canon_begin())->getIVConstCoeff(Level);

  auto *Ref1 = genAddressOfAccess(Lower, Level, Stride > 0 ? LCE : UCE);
  auto *Ref2 = genAddressOfAccess(Upper, Level, Stride > 0 ? UCE : LCE);

  if (DDRefUtils::areEqual(Ref1, Ref2)) {
    Ref1->setSymbase(Symbase1);
    Ref2->setSymbase(Symbase1);
  } else {
    Ref1->setSymbase(Symbase1);
    Ref2->setSymbase(Symbase2);
  }

  if (IVCoeff > 0) {
    return Segment(Ref1, Ref2);
  } else {
    return Segment(Ref2, Ref1);
  }
}

// TODO: Make this method more generic and implement multiplyByBlob
void IVSegment::replaceIVByCanonExpr(CanonExpr *Expr, unsigned Level,
    const CanonExpr *By) {
  auto ConstCoeff = Expr->getIVConstCoeff(Level);
  if (ConstCoeff == 0) {
    return;
  }

  auto Term = By->clone();
  Term->multiplyByConstant(ConstCoeff);

  Term->multiplyDenominator(Expr->getDenominator(), false);

  Expr->removeIV(Level);
  CanonExprUtils::add(Expr, Term, true);
  CanonExprUtils::destroy(Term);
}

RegDDRef *IVSegment::genAddressOfAccess(const RegDDRef *Ref, unsigned Level,
    const CanonExpr *Expr) {

  RegDDRef *Result = Ref->clone();
  CanonExpr *InnerSub = *Result->canon_begin();

  replaceIVByCanonExpr(InnerSub, Level, Expr);

  Result->setAddressOf(true);

  SmallVector<BlobDDRef*, 6> NewBlobs;
  Result->updateBlobDDRefs(NewBlobs);
  // TODO: Also update DefineAtLevel property for newly added blobs.
  // A new utility will consume NewBlobs vector + origin RegDDRef, which can be
  // loop lower or upper bound.

  return Result;
}

struct LoopCandidate {
  HLLoop *Loop;
  llvm::SmallVector<Segment, ExpectedNumberOfTests> SegmentList;
  bool GenTripCountTest;

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dump() {
    dbgs() << "Loop " << Loop->getNumber() << ":\n";
    for (auto &Segment : SegmentList) {
      Segment.dump();
    }
  }
#endif
};

enum RuntimeDDResult {
  OK,
  NO_OPPORTUNITIES,
  NON_INNERMOST,
  NON_LINEAR_BASE,
  NON_LINEAR_SUBS,
  NON_LINEAR_ACCESS,
  NON_CONSTANT_IV_STRIDE,
  SMALL_TRIPCOUNT,
  ALREADY_MV,
  TOO_MANY_TESTS,
  UPPER_SUB_TYPE_MISMATCH,
  BLOB_IV_COEFF,
};

class HIRRuntimeDD : public HIRTransformPass {
public:
  static char ID;

  HIRRuntimeDD() : HIRTransformPass(ID) {
    initializeHIRRuntimeDDPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<SymbaseAssignment>();
    AU.addRequiredTransitive<DDAnalysis>();
    AU.setPreservesAll();
  }

private:
  struct LoopAnalyzer final : public HLNodeVisitorBase {
    SmallVector<LoopCandidate, 16>  Candidates;

#ifndef NDEBUG
    static const char *getResultString(RuntimeDDResult Result) {
      switch (Result) {
      case OK: return "OK";
      case NO_OPPORTUNITIES:
        return "No opportunities";
      case NON_INNERMOST:
        return "Non innermost loop";
      case NON_LINEAR_BASE:
        return "The reference base is non linear";
      case NON_LINEAR_SUBS:
        return "One of the dimensions is non linear";
      case NON_LINEAR_ACCESS:
        return "Memory accessed in non linear pattern";
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
      default:
        llvm_unreachable("Unexpected give up reason");
      }
    }
#endif

    LoopAnalyzer() : SkipNode(nullptr) {}

    void visit(HLNode *) { }
    void postVisit(HLNode *) { }

    void visit(HLLoop *Loop) {
      LoopCandidate Candidate;
      RuntimeDDResult Result = HIRRuntimeDD::computeTests(Loop, Candidate);
      if (Result == OK) {
        SkipNode = Loop;

        Candidates.push_back(Candidate);
        LoopsMultiversioned++;
      } else {
        DEBUG(dbgs() << "LOOPOPT_OPTREPORT: Loop " << Loop->getNumber()
            << " not selected: " << getResultString(Result) << "\n");
      }
    }

    bool skipRecursion(const HLNode *N) const override {
      return N == SkipNode;
    }

  private:
    const HLNode *SkipNode;
  };

  static RuntimeDDResult isSegmentSupported(const IVSegment &Segment,
      const HLLoop *Loop, unsigned Level);

  // \brief Returns required DD tests for an arbitrary loop L.
  static RuntimeDDResult computeTests(HLLoop *Loop, LoopCandidate &Candidate);

  HLIf *createIfStmtForIntersection(Segment &S1, Segment &S2) const;

  // \brief Modifies HIR implementing specified tests.
  void generateDDTest(LoopCandidate &Candidate) const;
};

}

char HIRRuntimeDD::ID = 0;
INITIALIZE_PASS_BEGIN(HIRRuntimeDD, OPT_SWITCH, OPT_DESCR, false, false)
INITIALIZE_PASS_DEPENDENCY(SymbaseAssignment)
INITIALIZE_PASS_DEPENDENCY(DDAnalysis)
INITIALIZE_PASS_END(HIRRuntimeDD, OPT_SWITCH, OPT_DESCR, false, false)

FunctionPass *llvm::createHIRRuntimeDDPass() {
  return new HIRRuntimeDD();
}

RuntimeDDResult HIRRuntimeDD::isSegmentSupported(const IVSegment &Segment,
    const HLLoop *Loop, unsigned Level) {

  if (Segment.getBaseCE()->isNonLinear()) {
    return NON_LINEAR_BASE;
  }

  auto FirstCanonIter = Segment.getLower()->canon_begin();

  if (!CanonExprUtils::mergeable(*FirstCanonIter,
      Loop->getUpperCanonExpr(), true)) {
    return UPPER_SUB_TYPE_MISMATCH;
  }

  for (auto CE = FirstCanonIter,
      E = Segment.getLower()->canon_end(); CE != E; ++CE) {
    if ((*CE)->isNonLinear()) {
      return NON_LINEAR_SUBS;
    }

    if (CE == FirstCanonIter) {
      if ((*CE)->hasIVBlobCoeff(Level)) {
        return BLOB_IV_COEFF;
      }
    } else {
      if ((*CE)->hasIV(Level)) {
        return NON_LINEAR_ACCESS;
      }
    }
  }

  return OK;
}

RuntimeDDResult HIRRuntimeDD::computeTests(HLLoop *Loop,
    LoopCandidate &Candidate) {
  Candidate.Loop = Loop;
  Candidate.GenTripCountTest = true;

  DEBUG(dbgs() << "Runtime DD for loop " << Loop->getNumber() << ":\n");

  if (Loop->getMVTag()) {
    return ALREADY_MV;
  }

  if (!Loop->isInnermost()) {
    return NON_INNERMOST;
  }

  int64_t Stride;
  if (!Loop->getStrideCanonExpr()->isIntConstant(&Stride)) {
    return NON_CONSTANT_IV_STRIDE;
  }

  int64_t TripCount;
  if (Loop->isConstTripLoop(&TripCount)) {
    if (TripCount < 4) {
      return SMALL_TRIPCOUNT;
    }
    Candidate.GenTripCountTest = false;
  }

  auto LCE = Loop->getLowerCanonExpr();
  auto UCE = Loop->getUpperCanonExpr();
  auto Level = Loop->getNestingLevel();

  MemRefGatherer::MapTy RefMap;
  DDRefGrouping::RefGroupsTy Groups;

  MemRefGatherer::gather(Loop, RefMap);
  MemRefGatherer::sort(RefMap);
  MemRefGatherer::makeUnique(RefMap);

  DDRefGrouping::createGroups(Groups, RefMap, Level, 0);

  DEBUG(DDRefGrouping::dump(Groups));

  if (Groups.size() * (Groups.size() - 1) / 2 > MaximumNumberOfTests) {
    return TOO_MANY_TESTS;
  }

  SmallVector<IVSegment, ExpectedNumberOfTests> IVSegments;
  SmallVector<RuntimeDDResult, ExpectedNumberOfTests> Supported;
  for (unsigned i=0; i<Groups.size(); ++i) {
    IVSegments.push_back(IVSegment(Groups[i]));
    Supported.push_back(isSegmentSupported(IVSegments.back(), Loop, Level));
  }

  for (unsigned i=0; i<IVSegments.size() - 1; ++i) {
    IVSegment &S1 = IVSegments[i];

    for (unsigned j=i + 1; j<IVSegments.size(); ++j) {
      if (Groups[i].front()->getSymbase() != Groups[j].front()->getSymbase()) {
        break;
      }

      IVSegment &S2 = IVSegments[j];

      // Skip Read-Read segments
      if (!S1.isWrite() && !S2.isWrite()) {
        continue;
      }

      // Check if both segments are OK. Unsupported segment may
      // not be a problem, if there is no another overlapped segment.
      RuntimeDDResult Res;
      Res = Supported[i];
      if (Res != OK) {
        return Res;
      }
      Res = Supported[j];
      if (Res != OK) {
        return Res;
      }

      Candidate.SegmentList.push_back(
          S1.genLUSegment(Level, Stride, LCE, UCE));
      Candidate.SegmentList.push_back(
          S2.genLUSegment(Level, Stride, LCE, UCE));
    }
  }

  if (Candidate.SegmentList.size() == 0) {
    return NO_OPPORTUNITIES;
  }

  return OK;
}

HLIf *HIRRuntimeDD::createIfStmtForIntersection(
    Segment &S1, Segment &S2) const {
  RegDDRef *Bounds[] = {
      S1.Lower /* 0 */, S1.Upper /* 1 */,
      S2.Lower /* 2 */, S2.Upper /* 3 */,
  };

  HLIf *If = HLNodeUtils::createHLIf(PredicateTy::ICMP_UGE,
      Bounds[1], Bounds[2]);
  If->addPredicate(PredicateTy::ICMP_UGE, Bounds[3], Bounds[0]);

  return If;
}

void HIRRuntimeDD::generateDDTest(LoopCandidate &Candidate) const {
  HLLabel *OrigLabel = HLNodeUtils::createHLLabel("mv.orig");
  HLLabel *EscapeLabel = HLNodeUtils::createHLLabel("mv.escape");

  HLGoto *OrigGoto = HLNodeUtils::createHLGoto(OrigLabel);

  HLNodeUtils::insertBefore(Candidate.Loop, OrigLabel);
  HLNodeUtils::insertAfter(Candidate.Loop, EscapeLabel);

  // Generate tripcount test
  if (Candidate.GenTripCountTest) {
    uint64_t MinTripCount = Candidate.SegmentList.size();
    RegDDRef *TripCountRef = Candidate.Loop->getTripCountDDRef();
    HLIf *LowTripCountIf = HLNodeUtils::createHLIf(
        PredicateTy::ICMP_ULT,
        TripCountRef,
        DDRefUtils::createConstDDRef(
            TripCountRef->getDestType(), MinTripCount));

    HLNodeUtils::insertAsFirstChild(LowTripCountIf, OrigGoto, true);
    HLNodeUtils::insertBefore(OrigLabel, LowTripCountIf);
  }
  //////////////////////////

  unsigned RefsCount = Candidate.SegmentList.size();
  for (unsigned i=0; i<RefsCount; i+=2) {
    auto &S1 = Candidate.SegmentList[i];
    auto &S2 = Candidate.SegmentList[i+1];

    HLIf *DDCheck = createIfStmtForIntersection(S1, S2);

    HLNodeUtils::insertAsFirstChild(DDCheck, OrigGoto->clone(), true);
    HLNodeUtils::insertBefore(OrigLabel, DDCheck);
  }

  HLGoto *EscapeGoto = HLNodeUtils::createHLGoto(EscapeLabel);
  HLNodeUtils::insertBefore(OrigLabel, EscapeGoto);

  HLLoop *MVLoop = Candidate.Loop->clone();
  unsigned MVTag = Candidate.Loop->getNumber();
  Candidate.Loop->setMVTag(MVTag);
  MVLoop->setMVTag(MVTag);
  // TODO: Mark MVLoop ddrefs to say DDAnalysis that they do not intersect.

  HLNodeUtils::insertBefore(EscapeGoto, MVLoop);

  HLRegion *ParentRegion = Candidate.Loop->getParentRegion();
  ParentRegion->setGenCode(true);

  auto PreserveFunc = [](const HIRAnalysisPass *HAP) { return false; };
  if (HLLoop *ParentLoop = Candidate.Loop->getParentLoop()) {
    HIRInvalidationUtils::invalidateLoopBodyAnalysis(ParentLoop, PreserveFunc);
  } else {
    HIRInvalidationUtils::invalidateNonLoopRegionAnalysis(ParentRegion,
        PreserveFunc);
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
