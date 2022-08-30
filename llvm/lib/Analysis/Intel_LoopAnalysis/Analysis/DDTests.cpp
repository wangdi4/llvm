//===- DDTests.cpp - Data dependence testing between two DDRefs -----------===//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file is modified from DependenceAnalysis.cpp
// Thanks for Preston Briggs, the author of DependenceAnalysis.cpp
//
// DependenceAnalysis is an LLVM pass that analyses dependences between memory
// accesses. Currently, it is an (incomplete) implementation of the approach
// described in
//
//            Practical Dependence Testing
//            Goff, Kennedy, Tseng
//            PLDI 1991
//
// There's a single entry point that analyzes the dependence between a pair
// of memory references in a function, returning either NULL, for no dependence,
// or a more-or-less detailed description of the dependence between them.
//
// Currently, the implementation cannot propagate constraints between
// coupled RDIV subscripts and lacks a multi-subscript MIV test.
// Both of these are conservative weaknesses;
// that is, not a source of correctness problems.
//
// The implementation depends on the GEP instruction to differentiate
// subscripts. Since Clang linearizes some array subscripts, the dependence
// analysis is using SCEV->delinearize to recover the representation of multiple
// subscripts, and thus avoid the more expensive and less precise MIV tests. The
// delinearization is controlled by the flag -da-delinearize.
//
// We should pay some careful attention to the possibility of integer overflow
// in the implementation of the various tests. This could happen with Add,
// Subtract, or Multiply, with both APInt's and SCEV's.
//
// Some non-linear subscript pairs can be handled by the GCD test
// (and perhaps other tests).
// Should explore how often these things occur.
//
// Finally, it seems like certain test cases expose weaknesses in the SCEV
// simplification, especially in the handling of sign and zero extensions.
// It could be useful to spend time exploring these.
//
// Please note that this is work in progress and the interface is subject to
// change.
//
//===----------------------------------------------------------------------===//
//                                                                            //
//                   In memory of Ken Kennedy, 1945 - 2007                    //
//                                                                            //
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDTests.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/Delinearization.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

using namespace llvm;
using namespace llvm::loopopt;

// Next Option is set for unit testing only
static cl::opt<bool> AssumeLoopFusion(
    "hir-dd-test-assume-loop-fusion", cl::init(false), cl::Hidden,
    cl::desc("Demand Driven DD test invoked from Loop Fusion"));

enum class LoopCarriedDepMode { None, InnermostOnly, All };

static cl::opt<LoopCarriedDepMode> AssumeNoLoopCarriedDep(
    "hir-dd-test-assume-no-loop-carried-dep",
    cl::init(LoopCarriedDepMode::None), cl::Hidden, cl::ValueOptional,
    cl::desc("Assumes that no loop carried dependencies exist for certain "
             "loops according to mode"),
    cl::values(
        clEnumValN(LoopCarriedDepMode::None, "0",
                   "No assumptions about loop carried dependencies"),
        clEnumValN(LoopCarriedDepMode::InnermostOnly, "1",
                   "Assumes no loop carried dependencies exist for all "
                   "innermost loops"),
        clEnumValN(
            LoopCarriedDepMode::All, "2",
            "Assumes no loop carried dependencies exist for all loops")));

enum class VaryingBaseMode { QueryAlias, AssumeAlias, QueryLoopCarriedAlias };

static cl::opt<VaryingBaseMode> VaryingBaseHandling(
    "hir-dd-test-varying-base-mode",
    cl::init(VaryingBaseMode::QueryLoopCarriedAlias), cl::ReallyHidden,
    cl::desc("Influence how we use AA when encountering pointers with varying "
             "bases"),
    cl::values(clEnumValN(VaryingBaseMode::QueryAlias, "query-alias",
                          "Query the alias() interface, possibly incorrectly"),
               clEnumValN(VaryingBaseMode::AssumeAlias, "assume-alias",
                          "Conservatively assume MayAlias"),
               clEnumValN(VaryingBaseMode::QueryLoopCarriedAlias,
                          "query-loopcarried",
                          "Query the loopCarriedAlias() interface")));

#define DEBUG_TYPE "hir-dd-test"
#define DEBUG_AA(X) DEBUG_WITH_TYPE("hir-dd-test-aa", X)

//===----------------------------------------------------------------------===//
// statistics

STATISTIC(ZIVapplications, "ZIV applications");
STATISTIC(ZIVindependence, "ZIV independence");
STATISTIC(ExactRDIVapplications, "Exact RDIV applications");
STATISTIC(ExactRDIVindependence, "Exact RDIV independence");
STATISTIC(SymbolicRDIVapplications, "Symbolic RDIV applications");
STATISTIC(SymbolicRDIVindependence, "Symbolic RDIV independence");
STATISTIC(DeltaApplications, "Delta applications");
STATISTIC(DeltaSuccesses, "Delta successes");
STATISTIC(GCDapplications, "GCD applications");
STATISTIC(GCDsuccesses, "GCD successes");
STATISTIC(GCDindependence, "GCD independence");
STATISTIC(BanerjeeApplications, "Banerjee applications");
STATISTIC(BanerjeeIndependence, "Banerjee independence");
STATISTIC(BanerjeeSuccesses, "Banerjee successes");
STATISTIC(DeltaPropagations, "Delta propagations");
STATISTIC(DeltaIndependence, "Delta independence");
STATISTIC(TotalArrayPairs, "Array pairs tested");
STATISTIC(SeparableSubscriptPairs, "Separable subscript pairs");
STATISTIC(CoupledSubscriptPairs, "Coupled subscript pairs");
STATISTIC(NonlinearSubscriptPairs, "Nonlinear subscript pairs");
STATISTIC(StrongSIVapplications, "Strong SIV applications");
STATISTIC(StrongSIVsuccesses, "Strong SIV successes");
STATISTIC(StrongSIVindependence, "Strong SIV independence");
STATISTIC(WeakCrossingSIVapplications, "Weak-Crossing SIV applications");
STATISTIC(WeakCrossingSIVsuccesses, "Weak-Crossing SIV successes");
STATISTIC(WeakCrossingSIVindependence, "Weak-Crossing SIV independence");
STATISTIC(ExactSIVapplications, "Exact SIV applications");
STATISTIC(ExactSIVsuccesses, "Exact SIV successes");
STATISTIC(ExactSIVindependence, "Exact SIV independence");
STATISTIC(WeakZeroSIVapplications, "Weak-Zero SIV applications");
STATISTIC(WeakZeroSIVsuccesses, "Weak-Zero SIV successes");
STATISTIC(WeakZeroSIVindependence, "Weak-Zero SIV independence");

//===----------------------------------------------------------------------===//
// FullDependence methods

Dependences::Dependences(const DDRef *Source, const DDRef *Destination,
                         unsigned CommonLevels)
    : Src(Source), Dst(Destination), CommonLevels(CommonLevels) {

  assert(CommonLevels <= MaxLoopNestLevel && "CommonLevel exceeded");
  Consistent = true;
  LoopIndependent = false;
  Reversed = false;
}

Dependences::~Dependences() {}

// The rest are simple getters that hide the implementation.

// getDirection - Returns the direction associated with a particular level.
DVKind Dependences::getDirection(unsigned Level) const {
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  return DV[Level - 1].Direction;
}

// Returns the distance (or NULL) associated with a particular level.
const CanonExpr *Dependences::getDistance(unsigned Level) const {
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  return DV[Level - 1].Distance;
}

// setDirection - sets DV for  with a particular level.
void Dependences::setDirection(const unsigned Level, const DVKind Direction) {
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  DV[Level - 1].Direction = Direction;
}

// sets the distance for a particular level.
void Dependences::setDistance(const unsigned Level, const CanonExpr *CE) {
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  DV[Level - 1].Distance = CE;
}

// Returns true if a particular level is scalar; that is,
// if no subscript in the source or destination mention the induction
// variable associated with the loop at this level.
bool Dependences::isScalar(unsigned Level) const {
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  return DV[Level - 1].Scalar;
}

// Returns true if peeling the first iteration from this loop
// will break this dependence.
bool Dependences::isPeelFirst(unsigned Level) const {
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  return DV[Level - 1].PeelFirst;
}

// Returns true if peeling the last iteration from this loop
// will break this dependence.
bool Dependences::isPeelLast(unsigned Level) const {
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  return DV[Level - 1].PeelLast;
}

// Returns true if splitting this loop will break the dependence.
bool Dependences::isSplitable(unsigned Level) const {
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  return DV[Level - 1].Splitable;
}

// Note: some of the function below are wrappers
// until we decide a way too free the CanonExpr created
// which should be freed
// Some of these will be moved to Util dir

const CanonExpr *DDTest::getInvariant(const CanonExpr *CE) {
  // Return blob + constant
  // get a copy and zero out all iv coeffs

  CanonExpr *CE2 = CE->clone();

  CE2->clearIVs();

  // Check denominator first to avoid spending compile time in the utility.
  bool IsNonNegative = (CE2->getDenominator() != 1) &&
                       HLNodeUtils::isKnownNonNegative(CE2, DeepestLoop);

  CE2->simplify(false, IsNonNegative);

  push(CE2);
  return CE2;
}

const CanonExpr *DDTest::getCoeff(const CanonExpr *CE, unsigned int IVNum,
                                  bool checkSingleIV) {

  // IVnum  indicates getting the first, 2nd, ..  iv
  // IVnum is not directly related to the loop nesting level
  // Returns the coeff as a canon expr
  // e.g.  3 * i1 + 4 * i3,  returns 3 when IVnum is 1
  // The default is asserting 1 single iv in input CE

  CanonExpr *CE2 = CE->getCanonExprUtils().createExtCanonExpr(
      CE->getSrcType(), CE->getDestType(), CE->isSExt());

  unsigned int IVFound = 0;
  assert(CanonExpr::isValidLoopLevel(IVNum) && "IVnum not within range");

  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {
    int64_t ConstCoeff = CE->getIVConstCoeff(CurIVPair);
    unsigned BlobIdx = CE->getIVBlobCoeff(CurIVPair);

    LLVM_DEBUG(dbgs() << "\n\tConst coeff, Blobidx: " << ConstCoeff << " "
                      << BlobIdx);

    if (ConstCoeff == 0) {
      continue;
    }
    IVFound++;

    if (IVFound == IVNum) {
      if (BlobIdx != 0) {
        CE2->addBlob(BlobIdx, ConstCoeff);
      } else {
        CE2->setConstant(ConstCoeff);
      }
    }
    if (checkSingleIV) {
      assert((IVFound == 1) && "found more than 1 iv");
    }
  }

  push(CE2);
  return CE2;
}

const CanonExpr *DDTest::getFirstCoeff(const CanonExpr *CE) {
  // No need to push(CE2) because it's done in getCoeff
  return getCoeff(CE, 1, false);
}

const CanonExpr *DDTest::getSecondCoeff(const CanonExpr *CE) {
  // for  1 * i1 + 3 * i2
  // return second  coeff, 3 in this case

  // No need to push(CE2) because it's done in getCoeff
  return getCoeff(CE, 2, false);
}

static const HLLoop *getLoop(const CanonExpr *CE, const HLLoop *ParentLoop) {

  // Returns Loop correponding to the iv in CE
  // Input CE has only one or no iv
  if (!(CE->hasIV())) {
    return nullptr;
  }

  unsigned IVLevel = 0;
  bool IVfound = false;
  const HLLoop *Loop;

  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {
    if (CE->getIVConstCoeff(CurIVPair)) {
      assert(!IVfound && "found more than 1 iv");
      IVLevel = CE->getLevel(CurIVPair);
      IVfound = true;
      (void)IVfound;
    }
  }

  Loop = ParentLoop->getParentLoopAtLevel(IVLevel);

  return Loop;
}

static const HLLoop *getFirstLoop(const CanonExpr *CE,
                                  const HLLoop *ParentLoop) {
  // returns Loop correponding to first iv
  // e.g. 1 * i1 + 2 *i3 return loop corrs. to level 1
  // Input CE has mutiple ivs

  assert(CE->hasIV() && "Loop has no iv");

  const HLLoop *Loop;
  auto CurIVPair = CE->iv_begin();

  for (auto E = CE->iv_end(); CurIVPair != E; ++CurIVPair) {
    if (CE->getIVConstCoeff(CurIVPair)) {
      break;
    }
  }

  Loop = ParentLoop->getParentLoopAtLevel(CE->getLevel(CurIVPair));
  assert(Loop && "Loop must be found for iv ");
  return Loop;
}

static const HLLoop *getSecondLoop(const CanonExpr *CE,
                                   const HLLoop *ParentLoop) {
  // returns Loop correponding to second iv
  // e.g. 1 * i1 + 2 *i3 return loop corrs. to level 3
  // Input CE has mutiple ivs

  assert(CE->hasIV() && "Loop has no iv");

  HLLoop *Loop;
  int NumIV = 0;
  auto CurIVPair = CE->iv_begin();

  for (auto E = CE->iv_end(); CurIVPair != E; ++CurIVPair) {
    if (CE->getIVConstCoeff(CurIVPair)) {
      if ((++NumIV) == 2) {
        break;
      }
    }
  }
  for (Loop = const_cast<HLLoop *>(ParentLoop); Loop != nullptr;
       Loop = Loop->getParentLoop()) {
    if (CE->getLevel(CurIVPair) == Loop->getNestingLevel()) {
      return Loop;
    }
  }

  assert(Loop && "Loop not found for iv ");
  return nullptr;
}

const CanonExpr *DDTest::getMinus(const CanonExpr *SrcConst,
                                  const CanonExpr *DstConst) {

  if (!SrcConst || !DstConst) {
    return nullptr;
  }

  CanonExpr *CE = CanonExprUtils::cloneAndSubtract(SrcConst, DstConst, true);
  if (!CE) {
    return nullptr;
  }

  // Check denominator first to avoid spending compile time in the utility.
  bool IsNonNegative = (CE->getDenominator() != 1) &&
                       HLNodeUtils::isKnownNonNegative(CE, DeepestLoop);

  CE->simplify(false, IsNonNegative);

  push(CE);
  return CE;
}

const CanonExpr *DDTest::getAdd(const CanonExpr *SrcConst,
                                const CanonExpr *DstConst) {

  if (!SrcConst || !DstConst) {
    return nullptr;
  }
  CanonExpr *CE = CanonExprUtils::cloneAndAdd(SrcConst, DstConst, true);
  if (!CE) {
    return nullptr;
  }

  // Check denominator first to avoid spending compile time in the utility.
  bool IsNonNegative = (CE->getDenominator() != 1) &&
                       HLNodeUtils::isKnownNonNegative(CE, DeepestLoop);

  CE->simplify(false, IsNonNegative);

  push(CE);
  return CE;
}

const CanonExpr *DDTest::getNegative(const CanonExpr *CE) {

  if (!CE) {
    return nullptr;
  }
  CanonExpr *CE2 = CE->getCanonExprUtils().cloneAndNegate(CE);

  push(CE2);
  return CE2;
}

const CanonExpr *DDTest::getNegativeDist(const CanonExpr *CE) {
  return getNegative(CE);
}

// Current support in Util: one of them must be a constant or invariant
const CanonExpr *DDTest::getMulExpr(const CanonExpr *CE1, const CanonExpr *CE2,
                                    bool HasBlob) {
  int64_t CVal = 0;

  if (!CE1 || !CE2) {
    return nullptr;
  }

  // 1. One of canon exprs is a constant
  if (CE2->isIntConstant(&CVal)) {
    std::swap(CE1, CE2);
  }
  if (CE1->isIntConstant(&CVal)) {
    CanonExpr *CE = CE2->clone();
    push(CE);
    if (!CE->multiplyByConstant(CVal)) {
      return nullptr;
    }
    return CE;
  }

  if (!HasBlob)
    return nullptr;

  // 2. One of canon exprs is invariant
  if (CE1->hasIV()) {
    std::swap(CE1, CE2);
  }
  if (CE1->hasIV()) {
    return nullptr;
  }

  // Allow only single blob in CE1
  if (CE2->numBlobs() == 1)
    std::swap(CE1, CE2);
  if (CE1->numBlobs() != 1)
    return nullptr;

  if (CE1->getSrcType() != CE2->getSrcType())
    return nullptr;

  CanonExpr *CEb = CE2->clone();
  push(CEb);
  unsigned Index = CE1->getSingleBlobIndex();
  int64_t Coeff = CE1->getSingleBlobCoeff();
  if (!CEb->multiplyByBlob(Index))
    return nullptr;
  if (!CEb->multiplyByConstant(Coeff))
    return nullptr;
  CanonExpr *CEc = CE2->clone();
  push(CEc);
  int64_t C = CE1->getConstant();
  if (C) {
    if (!CEc->multiplyByConstant(C))
      return nullptr;
    return getAdd(CEb, CEc);
  }

  return CEb;
}

const CanonExpr *DDTest::getConstantfromAPInt(Type *Ty, APInt Value) {
  CanonExpr *CE = HNU.getCanonExprUtils().createCanonExpr(Ty, Value);
  push(CE);
  return CE;
}

const CanonExpr *DDTest::getConstantWithType(Type *Ty, int64_t Val) {
  CanonExpr *CE = HNU.getCanonExprUtils().createCanonExpr(Ty, 0, Val);
  push(CE);
  return CE;
}

const CanonExpr *DDTest::getUDivExpr(const CanonExpr *CE1,
                                     const CanonExpr *CE2) {

  // Only  handles restricted cases when the inputs are constant
  // Caller need to check for null result

  if (!CE1 || !CE2) {
    return nullptr;
  }

  if (!CanonExprUtils::isTypeEqual(CE1, CE2)) {
    return nullptr;
  }
  int64_t CVal1, CVal2;

  if (!(CE1->isIntConstant(&CVal1))) {
    return nullptr;
  }
  if (!(CE2->isIntConstant(&CVal2)) || CVal2 == 0) {
    return nullptr;
  }

  const CanonExpr *CE = getConstantWithType(CE1->getSrcType(), CVal1 / CVal2);

  // Note: no need to do push_back CE here because it's already done
  return CE;
}

const CanonExpr *DDTest::getSMaxExpr(const CanonExpr *CE1,
                                     const CanonExpr *CE2) {
  // Only handles restricted cases when the diff is a constant
  // Will extend to cases like N,  N + conatant
  // Caller need to check for null result
  if (!CE1 || !CE2) {
    return nullptr;
  }

  int64_t CVal;

  if (HNU.getCanonExprUtils().getConstDistance(CE1, CE2, &CVal)) {
    return ((CVal > 0) ? CE1 : CE2);
  }

  return nullptr;
}

const CanonExpr *DDTest::getSMinExpr(const CanonExpr *CE1,
                                     const CanonExpr *CE2) {
  // Only handles restricted cases when the diff is a constant
  // Will extend to cases like N,  N + conatant
  // Caller need to check for null result
  if (!CE1 || !CE2) {
    return nullptr;
  }

  int64_t CVal;

  if (HNU.getCanonExprUtils().getConstDistance(CE1, CE2, &CVal)) {
    return ((CVal < 0) ? CE1 : CE2);
  }

  return nullptr;
}

//===----------------------------------------------------------------------===//
// DependenceAnalysis::Constraint methods

// If constrasint is a point <X, Y>, returns X.
// Otherwise assert.
const CanonExpr *DDTest::Constraint::getX() const {
  assert(Kind == Point && "Kind should be Point");
  return A;
}

// If constraint is a point <X, Y>, returns Y.
// Otherwise assert.
const CanonExpr *DDTest::Constraint::getY() const {
  assert(Kind == Point && "Kind should be Point");
  return B;
}

// If constraint is a line AX + BY = C, returns A.
// Otherwise assert.
const CanonExpr *DDTest::Constraint::getA() const {
  assert((Kind == Line || Kind == Distance) &&
         "Kind should be Line (or Distance)");
  return A;
}

// If constraint is a line AX + BY = C, returns B.
// Otherwise assert.
const CanonExpr *DDTest::Constraint::getB() const {
  assert((Kind == Line || Kind == Distance) &&
         "Kind should be Line (or Distance)");
  return B;
}

// If constraint is a line AX + BY = C, returns C.
// Otherwise assert.
const CanonExpr *DDTest::Constraint::getC() const {
  assert((Kind == Line || Kind == Distance) &&
         "Kind should be Line (or Distance)");
  return C;
}

// If constraint is a distance, returns D.
// Otherwise assert.
const CanonExpr *DDTest::Constraint::getD() const {
  assert(Kind == Distance && "Kind should be Distance");

  return C->getCanonExprUtils().cloneAndNegate(C);
}

// Returns the loop associated with this constraint.
const HLLoop *DDTest::Constraint::getAssociatedLoop() const {
  assert((Kind == Distance || Kind == Line || Kind == Point) &&
         "Kind should be Distance, Line, or Point");
  return AssociatedLoop;
}

void DDTest::Constraint::setPoint(const CanonExpr *X, const CanonExpr *Y,
                                  const HLLoop *CurLoop) {
  Kind = Point;
  A = X;
  B = Y;
  AssociatedLoop = CurLoop;
}

void DDTest::Constraint::setLine(const CanonExpr *AA, const CanonExpr *BB,
                                 const CanonExpr *CC, const HLLoop *CurLoop) {
  Kind = Line;
  A = AA;
  B = BB;
  C = CC;
  AssociatedLoop = CurLoop;
}

void DDTest::Constraint::setDistance(const CanonExpr *D,
                                     const HLLoop *CurLoop) {
  Kind = Distance;
  auto &CEU = D->getCanonExprUtils();

  //      (Type, ivlevel, constval, denom)
  A = CEU.createExtCanonExpr(D->getSrcType(), D->getDestType(), D->isSExt(), 0,
                             1, 1);
  B = CEU.cloneAndNegate(A);
  C = CEU.cloneAndNegate(D);

  AssociatedLoop = CurLoop;
}

void DDTest::Constraint::setEmpty() { Kind = Empty; }

void DDTest::Constraint::setAny() { Kind = Any; }

// For debugging purposes. Dumps the constraint out to OS.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DDTest::Constraint::dump(raw_ostream &OS) const {
  if (isEmpty()) {
    OS << " Empty\n";
  } else if (isAny()) {
    OS << " Any\n";
  } else if (isPoint()) {
    OS << " Point is <";
    getX()->dump();
    OS << ", ";
    getY()->dump();
    OS << ">\n";
  } else if (isDistance()) {
    OS << " Distance is ";
    getD()->dump();
    OS << " (";
    getA()->dump();
    OS << "*X + ";
    getB()->dump();
    OS << "*Y = ";
    getC()->dump();
    OS << ")\n";
  } else if (isLine()) {
    OS << " Line is ";
    getA()->dump();
    OS << "*X + ";
    getB()->dump();
    OS << "*Y = ";
    getC()->dump();
    OS << "\n";
  } else {
    llvm_unreachable("unknown constraint type in Constraint::dump");
  }
}
#endif

// Updates X with the intersection
// of the Constraints X and Y. Returns true if X has changed.
// Corresponds to Figure 4 from the paper
//
//            Practical Dependence Testing
//            Goff, Kennedy, Tseng
//            PLDI 1991
bool DDTest::intersectConstraints(Constraint *X, const Constraint *Y) {
  ++DeltaApplications;
  LLVM_DEBUG(dbgs() << "\nintersect constraints\n");
  LLVM_DEBUG(dbgs() << "\n    X ="; X->dump(dbgs()));
  LLVM_DEBUG(dbgs() << "\n    Y ="; Y->dump(dbgs()));
  assert(!Y->isPoint() && "Y must not be a Point");
  if (X->isAny()) {
    if (Y->isAny())
      return false;
    *X = *Y;
    return true;
  }
  if (X->isEmpty())
    return false;
  if (Y->isEmpty()) {
    X->setEmpty();
    return true;
  }

  if (X->isDistance() && Y->isDistance()) {
    LLVM_DEBUG(dbgs() << "\n    intersect 2 distances\n");
    if (isKnownPredicate(CmpInst::ICMP_EQ, X->getD(), Y->getD()))
      return false;
    if (isKnownPredicate(CmpInst::ICMP_NE, X->getD(), Y->getD())) {
      X->setEmpty();
      ++DeltaSuccesses;
      return true;
    }
    // Hmmm, interesting situation.
    // I guess if either is constant, keep it and ignore the other.
    if (Y->getD()->isConstant()) {
      *X = *Y;
      return true;
    }
    return false;
  }

  // At this point, the pseudo-code in Figure 4 of the paper
  // checks if (X->isPoint() && Y->isPoint()).
  // This case can't occur in our implementation,
  // since a Point can only arise as the result of intersecting
  // two Line constraints, and the right-hand value, Y, is never
  // the result of an intersection.
  assert(!(X->isPoint() && Y->isPoint()) &&
         "We shouldn't ever see X->isPoint() && Y->isPoint()");

  if (X->isLine() && Y->isLine()) {
    LLVM_DEBUG(dbgs() << "\n    intersect 2 lines\n");
    const CanonExpr *Prod1 = getMulExpr(X->getA(), Y->getB(), true);
    const CanonExpr *Prod2 = getMulExpr(X->getB(), Y->getA(), true);
    if (!Prod1 || !Prod2)
      return false;

    if (isKnownPredicate(CmpInst::ICMP_EQ, Prod1, Prod2)) {
      // slopes are equal, so lines are parallel
      LLVM_DEBUG(dbgs() << "\t\tsame slope\n");
      Prod1 = getMulExpr(X->getC(), Y->getB(), true);
      Prod2 = getMulExpr(X->getB(), Y->getC(), true);
      if (!Prod1 || !Prod2)
        return false;
      if (isKnownPredicate(CmpInst::ICMP_EQ, Prod1, Prod2))
        return false;
      if (isKnownPredicate(CmpInst::ICMP_NE, Prod1, Prod2)) {
        X->setEmpty();
        ++DeltaSuccesses;
        return true;
      }
      return false;
    }
    if (isKnownPredicate(CmpInst::ICMP_NE, Prod1, Prod2)) {
      // slopes differ, so lines intersect
      LLVM_DEBUG(dbgs() << "\t\tdifferent slopes\n");
      const CanonExpr *C1B2 = getMulExpr(X->getC(), Y->getB(), true);
      const CanonExpr *C1A2 = getMulExpr(X->getC(), Y->getA(), true);
      const CanonExpr *C2B1 = getMulExpr(Y->getC(), X->getB(), true);
      const CanonExpr *C2A1 = getMulExpr(Y->getC(), X->getA(), true);
      const CanonExpr *A1B2 = getMulExpr(X->getA(), Y->getB(), true);
      const CanonExpr *A2B1 = getMulExpr(Y->getA(), X->getB(), true);
      if (!C1B2 || !C1A2 || !C2B1 || !C2A1 || !A1B2 || !A2B1)
        return false;
      int64_t Xtop, Xbot, Ytop, Ybot;
      const CanonExpr *C1A2_C2A1 = getMinus(C1A2, C2A1);
      const CanonExpr *C1B2_C2B1 = getMinus(C1B2, C2B1);
      const CanonExpr *A1B2_A2B1 = getMinus(A1B2, A2B1);
      const CanonExpr *A2B1_A1B2 = getMinus(A2B1, A1B2);
      if (!C1A2_C2A1 || !C1B2_C2B1 || !A1B2_A2B1 || !A2B1_A1B2)
        return false;
      if (!C1B2_C2B1->isIntConstant(&Xtop) ||
          !A1B2_A2B1->isIntConstant(&Xbot) ||
          !C1A2_C2A1->isIntConstant(&Ytop) || !A2B1_A1B2->isIntConstant(&Ybot))
        return false;
      if (!Xbot || !Ybot)
        return false;
      LLVM_DEBUG(dbgs() << "\t\tXtop = " << Xtop << "\n");
      LLVM_DEBUG(dbgs() << "\t\tXbot = " << Xbot << "\n");
      LLVM_DEBUG(dbgs() << "\t\tYtop = " << Ytop << "\n");
      LLVM_DEBUG(dbgs() << "\t\tYbot = " << Ybot << "\n");
      int64_t Xq = Xtop / Xbot;
      int64_t Xr = Xtop % Xbot;
      int64_t Yq = Ytop / Ybot;
      int64_t Yr = Ytop % Ybot;
      if (Xr != 0 || Yr != 0) {
        X->setEmpty();
        ++DeltaSuccesses;
        return true;
      }
      LLVM_DEBUG(dbgs() << "\t\tX = " << Xq << ", Y = " << Yq << "\n");
      if (Xq < 0 || Yq < 0) {
        X->setEmpty();
        ++DeltaSuccesses;
        return true;
      }
      const CanonExpr *CUB =
          collectUpperBound(X->getAssociatedLoop(), Prod1->getSrcType());
      int64_t UpperBound;
      if (CUB && CUB->isIntConstant(&UpperBound)) {
        LLVM_DEBUG(dbgs() << "\t\tupper bound = " << UpperBound << "\n");
        if (Xq > UpperBound || Yq > UpperBound) {
          X->setEmpty();
          ++DeltaSuccesses;
          return true;
        }
      }
      X->setPoint(getConstantWithType(Prod1->getSrcType(), Xq),
                  getConstantWithType(Prod1->getSrcType(), Yq),
                  X->getAssociatedLoop());
      ++DeltaSuccesses;
      return true;
    }
    return false;
  }

  // if (X->isLine() && Y->isPoint()) This case can't occur.
  assert(!(X->isLine() && Y->isPoint()) && "This case should never occur");

  if (X->isPoint() && Y->isLine()) {
    LLVM_DEBUG(dbgs() << "\t    intersect Point and Line\n");
    const CanonExpr *A1X1 = getMulExpr(Y->getA(), X->getX(), true);
    const CanonExpr *B1Y1 = getMulExpr(Y->getB(), X->getY(), true);
    if (!A1X1 || !B1Y1)
      return false;
    const CanonExpr *Sum = getAdd(A1X1, B1Y1);
    if (isKnownPredicate(CmpInst::ICMP_EQ, Sum, Y->getC()))
      return false;
    if (isKnownPredicate(CmpInst::ICMP_NE, Sum, Y->getC())) {
      X->setEmpty();
      ++DeltaSuccesses;
      return true;
    }
    return false;
  }

  llvm_unreachable("shouldn't reach the end of Constraint intersection");
  return false;
}

// DependenceAnalysis methods
// For debugging purposes. Dumps a dependence to OS.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void Dependences::dump(raw_ostream &OS) const {
  bool Splitable = false;

#if 0
	
  if (isConfused())
    OS << "confused";
  else {
    if (isConsistent())
      OS << "consistent ";
    if (isFlow())
      OS << "flow";
    else if (isOutput())
      OS << "output";
    else if (isAnti())
      OS << "anti";
    else if (isInput())
      OS << "input";
#endif

  {
    unsigned Levels = getLevels();

    OS << "\nDistance Vector (";
    for (unsigned II = 1; II <= Levels; ++II) {
      const CanonExpr *Distance = getDistance(II);
      if (Distance) {
        Distance->dump();
      } else {
        OS << "nil";
      }
      if (II != Levels) {
        OS << " ";
      }
    }
    OS << ")\n";

    OS << " DV (";
    for (unsigned II = 1; II <= Levels; ++II) {
      DVKind Direction = getDirection(II);
      switch (Direction) {
      case DVKind::ALL:
        OS << "*";
        break;
      case DVKind::LT:
        OS << "<";
        break;
      case DVKind::EQ:
        OS << "=";
        break;
      case DVKind::LE:
        OS << "<=";
        break;
      case DVKind::GT:
        OS << ">";
        break;
      case DVKind::NE:
        OS << "<>";
        break;
      case DVKind::NONE:
        OS << "0";
        break;
      default:
        break;
      }
      if (II != Levels) {
        OS << " ";
      }
    }
    OS << ")\n";

    if (isReversed()) {
      OS << " DV reversed";
    }
    if (isLoopIndependent()) {
      OS << " is LoopIndependent";
    }

    if (Splitable)
      OS << " splitable";
  }
  OS << " \n";
}
#endif

// Examines the loop nesting of the Src and Dst
// instructions and establishes their shared loops. Sets the variables
// CommonLevels, SrcLevels, and MaxLevels.
// The source and destination instructions needn't be contained in the same
// loop. The routine establishNestingLevels finds the level of most deeply
// nested loop that contains them both, CommonLevels. An instruction that's
// not contained in a loop is at level = 0. MaxLevels is equal to the level
// of the source plus the level of the destination, minus CommonLevels.
// This lets us allocate vectors MaxLevels in length, with room for every
// distinct loop referenced in both the source and destination subscripts.
// The variable SrcLevels is the nesting depth of the source instruction.
// It's used to help calculate distinct loops referenced by the destination.
// Here's the map from loops to levels:
//            0 - unused
//            1 - outermost common loop
//          ... - other common loops
// CommonLevels - innermost common loop
//          ... - loops containing Src but not Dst
//    SrcLevels - innermost loop containing Src but not Dst
//          ... - loops containing Dst but not Src
//    MaxLevels - innermost loops containing Dst but not Src
// Consider the follow code fragment:
//   for (a = ...) {
//     for (b = ...) {
//       for (c = ...) {
//         for (d = ...) {
//           A[] = ...;
//         }
//       }
//       for (e = ...) {
//         for (f = ...) {
//           for (g = ...) {
//             ... = A[];
//           }
//         }
//       }
//     }
//   }
// If we're looking at the possibility of a dependence between the store
// to A (the Src) and the load from A (the Dst), we'll note that they
// have 2 loops in common, so CommonLevels will equal 2 and the direction
// vector for Result will have 2 entries. SrcLevels = 4 and MaxLevels = 7.
// A map from loop names to loop numbers would look like
//     a - 1
//     b - 2 = CommonLevels
//     c - 3
//     d - 4 = SrcLevels
//     e - 5
//     f - 6
//     g - 7 = MaxLevels

void DDTest::establishNestingLevels(const DDRef *SrcDDRef,
                                    const DDRef *DstDDRef, bool ForFusion) {

  HLLoop *SrcLoop = SrcDDRef->getHLDDNode()->getLexicalParentLoop();
  HLLoop *DstLoop = DstDDRef->getHLDDNode()->getLexicalParentLoop();

  LCALoop = HLNodeUtils::getLowestCommonAncestorLoop(SrcLoop, DstLoop);

  SrcLevels = SrcLoop ? SrcLoop->getNestingLevel() : 0;
  DstLevels = DstLoop ? DstLoop->getNestingLevel() : 0;

  LCALoopLevel = LCALoop ? LCALoop->getNestingLevel() : 0;

  MaxLevels = SrcLevels + DstLevels - LCALoopLevel;

  DeepestLoop = (SrcLevels > DstLevels) ? SrcLoop : DstLoop;

  // LCALoopLevel is different from CommonLevels.
  // Refer to code below. Handled differently when IVDEP
  // is not used, for Fusion and refs outside Loops
  CommonLevels = LCALoopLevel;

  if (CommonLevels == 0) {
    // Need DD edge to connect
    NoCommonNest = true;
    CommonLevels = 1;
  }

  // For Fusion set to deepest level
  if (ForFusion && DeepestLoop) {
    CommonLevels = DeepestLoop->getNestingLevel();
  }

  if (MaxLevels == 0) {
    MaxLevels = 1;
  }
}

// Given one of the loops containing the source, return
// its level index in our numbering scheme.
unsigned DDTest::mapSrcLoop(const HLLoop *SrcLoop) const {
  return SrcLoop->getNestingLevel();
}

// Given one of the loops containing the destination,
// return its level index in our numbering scheme.
unsigned DDTest::mapDstLoop(unsigned NestingLevel) const {
  if (NestingLevel > CommonLevels) {
    return NestingLevel - CommonLevels + SrcLevels;
  } else {
    return NestingLevel;
  }
}

unsigned DDTest::mapDstLoop(const HLLoop *DstLoop) const {
  unsigned D = DstLoop->getNestingLevel();
  return mapDstLoop(D);
}

#if 0

// Returns true if Expression is loop invariant in LoopNest.
bool DependenceAnalysis::isLoopInvariant(const SCEV *Expression,
                                         const Loop *LoopNest) const {
  if (!LoopNest)
    return true;
  return SE->isLoopInvariant(Expression, LoopNest) &&
    isLoopInvariant(Expression, LoopNest->getParentLoop());
}

#endif

// Finds the set of loops from the LoopNest that
// have a level <= CommonLevels and are referred to by Expression
void DDTest::collectCommonLoops(const CanonExpr *Expression,
                                const HLLoop *LoopNest,
                                SmallBitVector &Loops) const {
  // TODO: linear at level

  while (LoopNest) {
    unsigned Level = LoopNest->getNestingLevel();
    if (Level <= CommonLevels && Expression->isNonLinear()) {
      Loops.set(Level);
    }
    LoopNest = LoopNest->getParentLoop();
  }
}

#if 0

void DependenceAnalysis::unifySubscriptType(Subscript *Pair) {
  const SCEV *Src = Pair->Src;
  const SCEV *Dst = Pair->Dst;
  IntegerType *SrcTy = dyn_cast<IntegerType>(Src->getType());
  IntegerType *DstTy = dyn_cast<IntegerType>(Dst->getType());
  if (SrcTy == nullptr || DstTy == nullptr) {
    assert(SrcTy == DstTy && "This function only unify integer types and "
                             "expect Src and Dst share the same type "
                             "otherwise.");
    return;
  }
  if (SrcTy->getBitWidth() > DstTy->getBitWidth()) {
    // Sign-extend Dst to typeof(Src) if typeof(Src) is wider than typeof(Dst).
    Pair->Dst = SE->getSignExtendExpr(Dst, SrcTy);
  } else if (SrcTy->getBitWidth() < DstTy->getBitWidth()) {
    // Sign-extend Src to typeof(Dst) if typeof(Dst) is wider than typeof(Src).
    Pair->Src = SE->getSignExtendExpr(Src, DstTy);
  }
}

#endif

// removeMatchingExtensions - Examines a subscript pair.
// If the source and destination are identically sign (or zero)
// extended, it strips off the extension in an effect to simplify
// the actual analysis.
void DDTest::removeMatchingExtensions(Subscript *Pair) {

  // TODO:  will handle this later

#if 0
  const CanonExpr *Src = Pair->Src;
  const CanonExpr *Dst = Pair->Dst;
	
  if ((isa<SCEVZeroExtendExpr>(Src) && isa<SCEVZeroExtendExpr>(Dst)) ||
      (isa<SCEVSignExtendExpr>(Src) && isa<SCEVSignExtendExpr>(Dst))) {
    const SCEVIntegralCastExpr *SrcCast = cast<SCEVIntegralCastExpr>(Src);
    const SCEVIntegralCastExpr *DstCast = cast<SCEVIntegralCastExpr>(Dst);
    const SCEV *SrcCastOp = SrcCast->getOperand();
    const SCEV *DstCastOp = DstCast->getOperand();
    if (SrcCastOp->getType() == DstCastOp->getType()) {
      Pair->Src = SrcCastOp;
      Pair->Dst = DstCastOp;
    }
  }
#endif
}

// Examine the CanonExpr and return true iff it's linear.
// Collect any loops mentioned in the set of "Loops".
bool DDTest::checkSrcSubscript(const CanonExpr *Src, const HLLoop *LoopNest,
                               SmallBitVector &Loops) {

  // TODO: Disable denom != 1 now
  // Need more work later. A[(i+1)/2] += 1  has loop carried  dep
  if (Src->isNonLinear() || Src->getDenominator() != 1) {
    return false;
  }

  // Level used in this context are similar to loop level
  // it starts from 1

  if (Src->hasIV()) {
    for (auto CurIVPair = Src->iv_begin(), E = Src->iv_end(); CurIVPair != E;
         ++CurIVPair) {
      if (Src->getIVConstCoeff(CurIVPair)) {
        Loops.set(Src->getLevel(CurIVPair));
      }
    }
  }
  return true;
}

// Examine the CanoExpr  and return true iff it's linear.
// Collect any loops mentioned in the set of "Loops".
bool DDTest::checkDstSubscript(const CanonExpr *Dst, const HLLoop *LoopNest,
                               SmallBitVector &Loops) {

  // TODO: this can be combined with previous one

  if (Dst->isNonLinear() || Dst->getDenominator() != 1) {
    return false;
  }

  if (Dst->hasIV()) {
    for (auto CurIVPair = Dst->iv_begin(), E = Dst->iv_end(); CurIVPair != E;
         ++CurIVPair) {
      if (Dst->getIVConstCoeff(CurIVPair)) {
        Loops.set(mapDstLoop(Dst->getLevel(CurIVPair)));
      }
    }
  }
  return true;
}

// Examines the subscript pair (the Src and Dst SCEVs)
// and classifies it as either ZIV, SIV, RDIV, MIV, or Nonlinear.
// Collects the associated loops in a set.
DDTest::Subscript::ClassificationKind
DDTest::classifyPair(const CanonExpr *Src, const HLLoop *SrcLoopNest,
                     const CanonExpr *Dst, const HLLoop *DstLoopNest,
                     SmallBitVector &Loops) {
  SmallBitVector SrcLoops(MaxLevels + 1);
  SmallBitVector DstLoops(MaxLevels + 1);
  if (!checkSrcSubscript(Src, SrcLoopNest, SrcLoops))
    return Subscript::NonLinear;
  if (!checkDstSubscript(Dst, DstLoopNest, DstLoops))
    return Subscript::NonLinear;
  Loops = SrcLoops;
  Loops |= DstLoops;
  unsigned N = Loops.count();
  if (N == 0) {
    return Subscript::ZIV;
  }
  if (N == 1) {
    return Subscript::SIV;
  }
  if (N == 2 && (SrcLoops.count() == 0 || DstLoops.count() == 0 ||
                 (SrcLoops.count() == 1 && DstLoops.count() == 1))) {
    return Subscript::RDIV;
  }
  return Subscript::MIV;
}

// Given a single blob CE %t, which is marked SExt or ZExt, constuct sext(%t)
// and zext(%t).
const CanonExpr *DDTest::addExt(const CanonExpr *CE) {
  // Only single blob canonical expression with  no IVs and demoninator=1 is
  // allowed.
  if (!CE->isSingleBlob(true /*allow conversion*/)) {
    return CE;
  }

  if (!CE->isZExt() && !CE->isSExt()) {
    return CE;
  }

  auto *DestTy = CE->getDestType();
  auto &BU = CE->getBlobUtils();
  auto Blob = BU.getBlob(CE->getSingleBlobIndex());
  int64_t BlobCoeff = CE->getSingleBlobCoeff();

  unsigned NewBlobIdx;
  if (CE->isZExt() && (BlobCoeff > 0)) {
    BU.createZeroExtendBlob(Blob, DestTy, true, &NewBlobIdx);
  } else if (CE->isSExt()) {
    BU.createSignExtendBlob(Blob, DestTy, true, &NewBlobIdx);
  } else {
    return CE;
  }

  auto *NewCE = CE->getCanonExprUtils().createStandAloneBlobCanonExpr(
      NewBlobIdx, CE->getDefinedAtLevel());

  if (BlobCoeff != 1) {
    NewCE->setBlobCoeff(NewBlobIdx, BlobCoeff);
  }

  push(NewCE);

  return NewCE;
}

// Given a CE of the form-
// 4 * sext(%t) + c   or    sext(4 * %t) + c
//
// It will return a new CE of this form if StripSExt is true-
// 4 * %t + c
const CanonExpr *DDTest::stripExt(const CanonExpr *CE, bool StripSExt,
                                  bool StripZExt) {
  assert((StripSExt || StripZExt) && "Invalid arguments!");

  // Only single blob canonical expression with  no IVs and demoninator=1 is
  // allowed.
  if (CE->hasIV() || (CE->numBlobs() != 1) || (CE->getDenominator() != 1) ||
      (CE->getSrcType() != CE->getDestType())) {
    return CE;
  }

  auto &BU = CE->getBlobUtils();
  auto *Blob = BU.getBlob(CE->getSingleBlobIndex());
  int64_t BlobCoeff = CE->getSingleBlobCoeff();

  auto CEConst = CE->getConstant();

  if (StripSExt && isa<SCEVSignExtendExpr>(Blob)) {
    Blob = cast<SCEVSignExtendExpr>(Blob)->getOperand();
  } else if (StripZExt && (CEConst >= 0) && (BlobCoeff >= 0) &&
             isa<SCEVZeroExtendExpr>(Blob)) {
    Blob = cast<SCEVZeroExtendExpr>(Blob)->getOperand();
  } else {
    return CE;
  }

  if (auto *MulBlob = dyn_cast<SCEVMulExpr>(Blob)) {
    if (MulBlob->getNumOperands() == 2) {
      if (auto *ConstBlob = dyn_cast<SCEVConstant>(MulBlob->getOperand(0))) {
        BlobCoeff *= ConstBlob->getAPInt().getSExtValue();
        Blob = MulBlob->getOperand(1);
      }
    }
  }

  // Constant should fit into smaller type.
  if (!ConstantInt::isValueValidForType(Blob->getType(), CEConst))
      return CE;

  unsigned BlobIdx = BU.findOrInsertBlob(Blob);
  auto *NewCE = CE->getCanonExprUtils().createStandAloneBlobCanonExpr(
      BlobIdx, CE->getDefinedAtLevel());

  if (BlobCoeff != 1) {
    NewCE->setBlobCoeff(BlobIdx, BlobCoeff);
  }

  NewCE->setConstant(CEConst);

  push(NewCE);

  return NewCE;
}


// A wrapper around for dealing special cases of predicates
// Looks for cases where we're interested in comparing for equality.
bool DDTest::isKnownPredicateImpl(ICmpInst::Predicate Pred, const CanonExpr *X,
                              const CanonExpr *Y) {
  const CanonExpr *Delta = getMinus(X, Y);
  if (!Delta) {
    return false;
  }
  switch (Pred) {
  case CmpInst::ICMP_EQ:
    return Delta->isZero();
  case CmpInst::ICMP_NE:
    return HLNodeUtils::isKnownNonZero(Delta, DeepestLoop);
  case CmpInst::ICMP_SGE:
    return HLNodeUtils::isKnownNonNegative(Delta, DeepestLoop);
  case CmpInst::ICMP_SLE:
    return HLNodeUtils::isKnownNonPositive(Delta, DeepestLoop);
  case CmpInst::ICMP_SGT:
    return HLNodeUtils::isKnownPositive(Delta, DeepestLoop);
  case CmpInst::ICMP_SLT:
    return HLNodeUtils::isKnownNegative(Delta, DeepestLoop);
  default:
    llvm_unreachable("unexpected predicate in isKnownPredicate");
  }
}

bool DDTest::isKnownPredicate(ICmpInst::Predicate Pred, const CanonExpr *X,
                              const CanonExpr *Y) {
  // First try original canon exprs.
  if (isKnownPredicateImpl(Pred, X, Y))
    return true;

  // If result is still unknown, try to strip SExt/ZExt from X and Y and compare
  // them.
  bool StripSExt = CmpInst::isSigned(Pred);

  // Cannot strip extension without knowing type.
  if (!StripSExt && !CmpInst::isUnsigned(Pred)) {
    return false;
  }

  bool StripZExtX = !StripSExt;
  bool StripZExtY = !StripSExt;

  // We are testing X >s Y. Since zext(X) >=s X, it should be safe to replace
  // zext(X) with X.
  if (Pred == CmpInst::ICMP_SGE || Pred == CmpInst::ICMP_SGT) {
    StripZExtX = true;
  }

  X = stripExt(X, StripSExt, StripZExtX);

  // We are testing X <s Y. Since zext(Y) >=s Y, it should be safe to replace
  // zext(Y) with Y.
  if (Pred == CmpInst::ICMP_SLE || Pred == CmpInst::ICMP_SLT) {
    StripZExtY = true;
  }

  Y = stripExt(Y, StripSExt, StripZExtY);

  if (isKnownPredicateImpl(Pred, X, Y))
    return true;

  // If result is still unknown, try to add SExt/ZExt to X and Y and compare
  // them.

  if (X->isZExt() || X->isSExt()) {
    X = addExt(X);
  }

  if (Y->isZExt() || Y->isSExt()) {
    Y = addExt(Y);
  }

  return isKnownPredicateImpl(Pred, X, Y);

}

// All subscripts are all the same type.
// Loop bound may be smaller (e.g., a char).
// Should zero extend loop bound, since it's always >= 0.
// This routine collects upper bound and extends if needed.
// Return null if no bound available.
const CanonExpr *DDTest::collectUpperBound(const HLLoop *L, Type *T) {

  if (L->isUnknown()) {
    return nullptr;
  }

  const CanonExpr *UpperBound = L->getUpperCanonExpr();

  uint64_t MaxTC;
  if (!UpperBound->isIntConstant() && (MaxTC = L->getMaxTripCountEstimate()) &&
      L->isMaxTripCountEstimateUsefulForDD()) {
    return getConstantWithType(UpperBound->getDestType(), MaxTC - 1);
  }
  // TODO: test I8 UB
  return UpperBound;
}

#if 0

// Calls collectUpperBound(), then attempts to cast it to SCEVConstant.
// If the cast fails, returns NULL.
const SCEVConstant *DependenceAnalysis::collectConstantUpperBound(const Loop *L,
                                                                  Type *T) const {
  if (const SCEV *UB = collectUpperBound(L, T))
    return dyn_cast<SCEVConstant>(UB);
  return nullptr;
}

#endif

// testZIV -
// When we have a pair of subscripts of the form [c1] and [c2],
// where c1 and c2 are both loop invariant, we attack it using
// the ZIV test. Basically, we test by comparing the two values,
// but there are actually three possible results:
// 1) the values are equal, so there's a dependence
// 2) the values are different, so there's no dependence
// 3) the values might be equal, so we have to assume a dependence.
//
// Return true if dependence disproved.

bool DDTest::testZIV(const CanonExpr *Src, const CanonExpr *Dst,
                     Dependences &Result) {

  LLVM_DEBUG(dbgs() << "\n    src = "; Src->dump());
  LLVM_DEBUG(dbgs() << "\n    dst = "; Dst->dump());

  ++ZIVapplications;
  if (isKnownPredicate(CmpInst::ICMP_EQ, Src, Dst)) {
    LLVM_DEBUG(dbgs() << "\n    provably dependent");
    return false; // provably dependent
  }
  if (isKnownPredicate(CmpInst::ICMP_NE, Src, Dst)) {
    LLVM_DEBUG(dbgs() << "\n    provably independent");
    ++ZIVindependence;
    return true; // provably independent
  }
  LLVM_DEBUG(dbgs() << "\n    possibly dependent\n");
  Result.Consistent = false;
  return false; // possibly dependent
}

// strongSIVtest -
// From the paper, Practical Dependence Testing, Section 4.2.1
//
// When we have a pair of subscripts of the form [c1 + a*i] and [c2 + a*i],
// where i is an induction variable, c1 and c2 are loop invariant,
//  and a is a constant, we can solve it exactly using the Strong SIV test.
//
// Can prove independence. Failing that, can compute distance (and direction).
// In the presence of symbolic terms, we can sometimes make progress.
//
// If there's a dependence,
//
//    c1 + a*i = c2 + a*i'
//
// The dependence distance is
//
//    d = i' - i = (c1 - c2)/a
//
// A dependence only exists if d is an integer and abs(d) <= U, where U is the
// loop's upper bound. If a dependence exists, the dependence direction is
// defined as
//
//                { < if d > 0
//    direction = { = if d = 0
//                { > if d < 0
//
// Return true if dependence disproved.

bool DDTest::strongSIVtest(const CanonExpr *Coeff, const CanonExpr *SrcConst,
                           const CanonExpr *DstConst, const HLLoop *CurLoop,
                           unsigned Level, Dependences &Result,
                           Constraint &NewConstraint) {
  LLVM_DEBUG(dbgs() << "\nStrong SIV test\n");
  LLVM_DEBUG(dbgs() << "\n    Coeff = "; Coeff->dump());
  LLVM_DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  LLVM_DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());
  ++StrongSIVapplications;
  assert(0 < Level && Level <= CommonLevels && "level out of range");
  Level--;

  const CanonExpr *Delta = getMinus(SrcConst, DstConst);

  if (!Delta) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "\n    Delta = "; Delta->dump());

  // check that |Delta| < iteration count
  // TBD: get UB for CurLoop

  if (!CurLoop->isUnknown()) {
    const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr();
    // There is a bug with the original code
    // for a[2 *n ]  n is SI64  in source.
    // Coeff in SCEV is  2*n  I64
    // SE->isKnowNonNegative(Coeff) return true but n could be nagative
    // w/o knowing what compose the whole subscript
    // Our implementation  using canon will return false for 2*n
    // But GCD test should get Indep for 2*n vs 2*n+1

    const CanonExpr *AbsDelta = nullptr;
    if (HLNodeUtils::isKnownNonNegative(Delta, CurLoop)) {
      AbsDelta = Delta;
    } else if (HLNodeUtils::isKnownNegative(Delta, CurLoop)) {
      AbsDelta = getNegative(Delta);
    } else {
      // Delta not known to be positive or negative.
      return false;
    }

    const CanonExpr *AbsCoeff = nullptr;
    if (HLNodeUtils::isKnownNonNegative(Coeff, CurLoop)) {
      AbsCoeff = Coeff;
    } else if (HLNodeUtils::isKnownNegative(Coeff, CurLoop)) {
      AbsCoeff = getNegative(Coeff);
    } else {
      // Coeff not known to be positive or negative.
      return false;
    }

    const CanonExpr *Product = getMulExpr(UpperBound, AbsCoeff);

    LLVM_DEBUG(dbgs() << "\n    UpperBound = "; UpperBound->dump());
    LLVM_DEBUG(dbgs() << "\n    AbsDelta = "; AbsDelta->dump());
    LLVM_DEBUG(dbgs() << "\n    AbsCoeff = "; AbsCoeff->dump());

    // If Delta is zero and coeff is  non-zero then set
    // dv as =
    // e.g.  2*n*i  is non-zero if n is non-zero

    if (Delta->isZero() && HLNodeUtils::isKnownNonZero(Coeff, CurLoop)) {

      Result.DV[Level].Distance = Delta;
      NewConstraint.setDistance(Delta, CurLoop);
      LLVM_DEBUG(dbgs() << "\n\t DV set as EQ - 1\n");
      Result.DV[Level].Direction &= DVKind::EQ;
    }

    if (Product && isKnownPredicate(CmpInst::ICMP_SGT, AbsDelta, Product)) {
      LLVM_DEBUG(dbgs() << "\n    Product = "; Product->dump());
      // Distance greater than trip count - no dependence
      ++StrongSIVindependence;
      ++StrongSIVsuccesses;
      LLVM_DEBUG(dbgs() << "\n StrongSIV finds independence 1\n");
      return true;
    }
  }

  int64_t K1;
  int64_t K2;
  const CanonExpr *K1CE;

  // Can we compute distance?
  if (Coeff->isIntConstant(&K2)) {
    if (Delta->isIntConstant(&K1)) {
      APInt ConstDelta = llvm::APInt(64, K1, true);
      APInt ConstCoeff = llvm::APInt(64, K2, true);
      APInt Distance = ConstDelta; // these need to be initialized
      APInt Remainder = ConstDelta;

      // Note: Distance, Reminder are possibly updated after this util
      APInt::sdivrem(ConstDelta, ConstCoeff, Distance, Remainder);
      LLVM_DEBUG(dbgs() << "\n    Distance = " << Distance << "\n");
      LLVM_DEBUG(dbgs() << "\n    Remainder = " << Remainder << "\n");
      // Make sure Coeff divides Delta exactly
      if (Remainder != 0) {
        // Coeff doesn't divide Distance, no dependence
        ++StrongSIVindependence;
        ++StrongSIVsuccesses;
        LLVM_DEBUG(dbgs() << "\n StrongSIV finds independence 2\n");
        return true;
      }

      K1CE = Result.DV[Level].Distance =
          getConstantfromAPInt(Coeff->getDestType(), Distance);

      NewConstraint.setDistance(K1CE, CurLoop);

      if (Distance.sgt(0)) {
        Result.DV[Level].Direction &= DVKind::LT;
      } else if (Distance.slt(0)) {
        Result.DV[Level].Direction &= DVKind::GT;
      } else {
        Result.DV[Level].Direction &= DVKind::EQ;
      }
      ++StrongSIVsuccesses;
    } else if (Delta->isZero()) {
      // since 0/X == 0
      Result.DV[Level].Distance = Delta;
      NewConstraint.setDistance(Delta, CurLoop);
      LLVM_DEBUG(dbgs() << "\n\t DV set as EQ -2 \n");
      Result.DV[Level].Direction &= DVKind::EQ;
      ++StrongSIVsuccesses;
    } else {
      if (Coeff->isOne()) {
        LLVM_DEBUG(dbgs() << "\n    Distance = "; Delta->dump());
        Result.DV[Level].Distance = Delta; // since X/1 == X
        NewConstraint.setDistance(Delta, CurLoop);
      } else {
        Result.Consistent = false;
        NewConstraint.setLine(Coeff, getNegative(Coeff), getNegative(Delta),
                              CurLoop);
      }

      // maybe we can get a useful direction
      bool DeltaMaybeZero = !(HLNodeUtils::isKnownNonZero(Delta, CurLoop));
      bool DeltaMaybePositive =
          !(HLNodeUtils::isKnownNonPositive(Delta, CurLoop));
      bool DeltaMaybeNegative =
          !(HLNodeUtils::isKnownNonNegative(Delta, CurLoop));
      bool CoeffMaybePositive =
          !(HLNodeUtils::isKnownNonPositive(Coeff, CurLoop));
      bool CoeffMaybeNegative =
          !(HLNodeUtils::isKnownNonNegative(Coeff, CurLoop));
      // The double negatives above are confusing.
      // It helps to read isKnownNonZero(Delta, CurLoop)
      // as "Delta might be Zero"
      DVKind NewDirection = DVKind::NONE;
      if ((DeltaMaybePositive && CoeffMaybePositive) ||
          (DeltaMaybeNegative && CoeffMaybeNegative))
        NewDirection = DVKind::LT;
      if (DeltaMaybeZero)
        NewDirection |= DVKind::EQ;
      if ((DeltaMaybeNegative && CoeffMaybePositive) ||
          (DeltaMaybePositive && CoeffMaybeNegative))
        NewDirection |= DVKind::GT;
      if (NewDirection < Result.DV[Level].Direction)
        ++StrongSIVsuccesses;
      Result.DV[Level].Direction &= NewDirection;
    }
  }

  LLVM_DEBUG(dbgs() << "\n StrongSIV finds dependence\n");
  return false;
}

// weakCrossingSIVtest -
// From the paper, Practical Dependence Testing, Section 4.2.2
//
// When we have a pair of subscripts of the form [c1 + a*i] and [c2 - a*i],
// where i is an induction variable, c1 and c2 are loop invariant,
// and a is a constant, we can solve it exactly using the
// Weak-Crossing SIV test.
//
// Given c1 + a*i = c2 - a*i', we can look for the intersection of
// the two lines, where i = i', yielding
//
//    c1 + a*i = c2 - a*i
//    2a*i = c2 - c1
//    i = (c2 - c1)/2a
//
// If i < 0, there is no dependence.
// If i > upperbound, there is no dependence.
// If i = 0 (i.e., if c1 = c2), there's a dependence with distance = 0.
// If i = upperbound, there's a dependence with distance = 0.
// If i is integral, there's a dependence (all directions).
// If the non-integer part = 1/2, there's a dependence (<> directions).
// Otherwise, there's no dependence.
//
// Can prove independence. Failing that,
// can sometimes refine the directions.
// Can determine iteration for splitting.
//
// Return true if dependence disproved.
bool DDTest::weakCrossingSIVtest(const CanonExpr *Coeff,
                                 const CanonExpr *SrcConst,
                                 const CanonExpr *DstConst,
                                 const HLLoop *CurLoop, unsigned Level,
                                 Dependences &Result, Constraint &NewConstraint,
                                 const CanonExpr *&SplitIter) {
  LLVM_DEBUG(dbgs() << "\tWeak-Crossing SIV test\n");
  LLVM_DEBUG(dbgs() << "\n    Coeff = "; Coeff->dump());
  LLVM_DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  LLVM_DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());

  ++WeakCrossingSIVapplications;
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  Level--;
  Result.Consistent = false;
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);
  if (!Delta) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
  NewConstraint.setLine(Coeff, Coeff, Delta, CurLoop);
  if (Delta->isZero()) {
    Result.DV[Level].Direction &= ~DVKind::LT;
    Result.DV[Level].Direction &= ~DVKind::GT;
    ++WeakCrossingSIVsuccesses;
    if (!Result.DV[Level].Direction) {
      ++WeakCrossingSIVindependence;
      LLVM_DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-0!");
      return true;
    }
    Result.DV[Level].Distance = Delta; // = 0
    return false;
  }

  int64_t CoeffValue;
  const CanonExpr *ConstCoeff = Coeff;
  if (!(Coeff->isIntConstant(&CoeffValue))) {
    return false;
  }

  Result.DV[Level].Splitable = true;
  if (HLNodeUtils::isKnownNegative(ConstCoeff, CurLoop)) {
    ConstCoeff = getNegative(ConstCoeff);
    assert(ConstCoeff &&
           "dynamic cast of negative of ConstCoeff should yield constant");
    Delta = getNegative(Delta);
  }
  assert(HLNodeUtils::isKnownPositive(ConstCoeff) &&
         "ConstCoeff should be positive");

  // compute SplitIter for use by DependenceAnalysis::getSplitIteration()

  const CanonExpr *MaxResult =
      getSMaxExpr(getConstantWithType(Delta->getSrcType(), 0), Delta);

  if (MaxResult == nullptr) {
    LLVM_DEBUG(dbgs() << "\nNeed more support for Max!");
    return false;
  }

  SplitIter = getUDivExpr(
      MaxResult,
      getMulExpr(getConstantWithType(Delta->getSrcType(), 2), ConstCoeff));

  if (SplitIter == nullptr) {
    LLVM_DEBUG(dbgs() << "\nNeed more support for Divide!");
    return false;
  }

  LLVM_DEBUG(dbgs() << "\n    Split iter = "; SplitIter->dump());

  const CanonExpr *ConstantDelta = Delta;
  int64_t DeltaValue;
  if (!(ConstantDelta->isIntConstant(&DeltaValue))) {
    return false;
  }

  // We're certain that ConstCoeff > 0; therefore,
  // if Delta < 0, then no dependence.
  LLVM_DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
  LLVM_DEBUG(dbgs() << "\n    ConstCoeff = "; ConstCoeff->dump());

  if (HLNodeUtils::isKnownNegative(Delta, CurLoop)) {
    // No dependence, Delta < 0s
    ++WeakCrossingSIVindependence;
    ++WeakCrossingSIVsuccesses;
    LLVM_DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-1!");
    return true;
  }

  // We're certain that Delta > 0 and ConstCoeff > 0.
  // Check Delta/(2*ConstCoeff) against upper loop bound
  if (!CurLoop->isUnknown()) {
    const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr();
    LLVM_DEBUG(dbgs() << "\n    UpperBound = "; UpperBound->dump());
    const CanonExpr *ConstantTwo =
        getConstantWithType(UpperBound->getSrcType(), 2);
    const CanonExpr *ML =
        getMulExpr(getMulExpr(ConstCoeff, UpperBound), ConstantTwo);

    if (!ML) {
      return false;
    }

    LLVM_DEBUG(dbgs() << "\n    ML = "; ML->dump());
    if (isKnownPredicate(CmpInst::ICMP_SGT, Delta, ML)) {
      // Delta too big, no dependence
      ++WeakCrossingSIVindependence;
      ++WeakCrossingSIVsuccesses;
      LLVM_DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-2!");
      return true;
    }
    if (isKnownPredicate(CmpInst::ICMP_EQ, Delta, ML)) {
      // i = i' = UB
      Result.DV[Level].Direction &= ~DVKind::LT;
      Result.DV[Level].Direction &= ~DVKind::GT;
      ++WeakCrossingSIVsuccesses;
      if (!Result.DV[Level].Direction) {
        ++WeakCrossingSIVindependence;
        LLVM_DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-3!");
        return true;
      }
      Result.DV[Level].Splitable = false;
      Result.DV[Level].Distance = getConstantWithType(Delta->getSrcType(), 0);
      return false;
    }
  }

  // check that Coeff divides Delta

  APInt APDelta;
  APInt APCoeff;

  APDelta = llvm::APInt(64, DeltaValue, true);
  APCoeff = llvm::APInt(64, CoeffValue, true);

  APInt Distance = APDelta; // these need to be initialzed
  APInt Remainder = APDelta;

  APInt::sdivrem(APDelta, APCoeff, Distance, Remainder);
  LLVM_DEBUG(dbgs() << "\n    Remainder = " << Remainder << "\n");
  if (Remainder != 0) {
    // Coeff doesn't divide Delta, no dependence
    ++WeakCrossingSIVindependence;
    ++WeakCrossingSIVsuccesses;
    LLVM_DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-4!");
    return true;
  }
  LLVM_DEBUG(dbgs() << "\n    Distance = " << Distance << "\n");

  // if 2*Coeff doesn't divide Delta, then the equal direction isn't possible
  APInt Two = APInt(Distance.getBitWidth(), 2, true);
  Remainder = Distance.srem(Two);
  LLVM_DEBUG(dbgs() << "\n    Remainder = " << Remainder << "\n");
  if (Remainder != 0) {
    // Equal direction isn't possible
    Result.DV[Level].Direction &= ~DVKind::EQ;
    ++WeakCrossingSIVsuccesses;
  }
  return false;
}

// Kirch's algorithm, from
//
//        Optimizing Supercompilers for Supercomputers
//        Michael Wolfe
//        MIT Press, 1989
//
// Program 2.1, page 29.
// Computes the GCD of AM and BM.
// Also finds a solution to the equation ax - by = gcd(a, b).
// Returns true if dependence disproved; i.e., gcd does not divide Delta.
static bool findGCD(unsigned Bits, APInt AM, APInt BM, APInt Delta, APInt &G,
                    APInt &X, APInt &Y) {
  APInt A0(Bits, 1, true), A1(Bits, 0, true);
  APInt B0(Bits, 0, true), B1(Bits, 1, true);
  APInt G0 = AM.abs();
  APInt G1 = BM.abs();
  APInt Q = G0; // these need to be initialized
  APInt R = G0;
  APInt::sdivrem(G0, G1, Q, R);
  while (R != 0) {
    APInt A2 = A0 - Q * A1;
    A0 = A1;
    A1 = A2;
    APInt B2 = B0 - Q * B1;
    B0 = B1;
    B1 = B2;
    G0 = G1;
    G1 = R;
    APInt::sdivrem(G0, G1, Q, R);
  }
  G = G1;
  LLVM_DEBUG(dbgs() << "\t    GCD = " << G << "\n");
  X = AM.slt(0) ? -A1 : A1;
  Y = BM.slt(0) ? B1 : -B1;

  // make sure gcd divides Delta
  R = Delta.srem(G);
  if (R != 0)
    return true; // gcd doesn't divide Delta, no dependence
  Q = Delta.sdiv(G);
  X *= Q;
  Y *= Q;
  return false;
}

static APInt floorOfQuotient(APInt A, APInt B) {
  APInt Q = A; // these need to be initialized
  APInt R = A;
  APInt::sdivrem(A, B, Q, R);

  if (R == 0) {
    return Q;
  }

  if ((A.sgt(0) && B.sgt(0)) || (A.slt(0) && B.slt(0))) {
    return Q;
  } else {
    return Q - 1;
  }
}

static APInt ceilingOfQuotient(APInt A, APInt B) {
  APInt Q = A; // these need to be initialized
  APInt R = A;
  APInt::sdivrem(A, B, Q, R);

  if (R == 0) {
    return Q;
  }

  if ((A.sgt(0) && B.sgt(0)) || (A.slt(0) && B.slt(0))) {
    return Q + 1;
  } else {
    return Q;
  }
}

static APInt maxAPInt(APInt A, APInt B) { return A.sgt(B) ? A : B; }

static APInt minAPInt(APInt A, APInt B) { return A.slt(B) ? A : B; }

// exactSIVtest -
// When we have a pair of subscripts of the form [c1 + a1*i] and [c2 + a2*i],
// where i is an induction variable, c1 and c2 are loop invariant, and a1
// and a2 are constant, we can solve it exactly using an algorithm developed
// by Banerjee and Wolfe. See Section 2.5.3 in
//
//        Optimizing Supercompilers for Supercomputers
//        Michael Wolfe
//        MIT Press, 1989
//
// It's slower than the specialized tests (strong SIV, weak-zero SIV, etc),
// so use them if possible. They're also a bit better with symbolics and,
// in the case of the strong SIV test, can compute Distances.
//
// Return true if dependence disproved.
bool DDTest::exactSIVtest(const CanonExpr *SrcCoeff, const CanonExpr *DstCoeff,
                          const CanonExpr *SrcConst, const CanonExpr *DstConst,
                          const HLLoop *CurLoop, unsigned Level,
                          Dependences &Result, Constraint &NewConstraint) {

  LLVM_DEBUG(dbgs() << "\nExact SIV test\n");
  LLVM_DEBUG(dbgs() << "\n    SrcCoeff = "; SrcCoeff->dump());
  LLVM_DEBUG(dbgs() << " = AM\n");
  LLVM_DEBUG(dbgs() << "\n    DstCoeff = "; DstCoeff->dump());
  LLVM_DEBUG(dbgs() << " = BM\n");
  LLVM_DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  LLVM_DEBUG(dbgs() << "\n");
  LLVM_DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());
  LLVM_DEBUG(dbgs() << "\n");

  ++ExactSIVapplications;
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  Level--;
  Result.Consistent = false;
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);

  if (!Delta) {
    return false;
  }
  LLVM_DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
  LLVM_DEBUG(dbgs() << "\n");

  NewConstraint.setLine(SrcCoeff, getNegative(DstCoeff), Delta, CurLoop);

  int64_t ConstDeltaVal, SrcCoeffVal, DstCoeffVal, UBVal;
  if (!(Delta->isIntConstant(&ConstDeltaVal)) ||
      !(SrcCoeff->isIntConstant(&SrcCoeffVal)) ||
      !(DstCoeff->isIntConstant(&DstCoeffVal))) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "\tDelta, SrcCoeff, DstCoeff = " << ConstDeltaVal << ","
                    << SrcCoeffVal << "," << DstCoeffVal);

  // find gcd
  APInt G, X, Y;
  APInt DM = llvm::APInt(64, ConstDeltaVal, true);
  APInt AM = llvm::APInt(64, SrcCoeffVal, true);
  APInt BM = llvm::APInt(64, DstCoeffVal, true);

  unsigned Bits = AM.getBitWidth();
  if (findGCD(Bits, AM, BM, DM, G, X, Y)) {
    // gcd doesn't divide Delta, no dependence
    ++ExactSIVindependence;
    ++ExactSIVsuccesses;
    return true;
  }

  LLVM_DEBUG(dbgs() << "\t    X = " << X << ", Y = " << Y << ", G = " << G
                    << "\n");
  //  Normalized CE implies LM = 0

  APInt UM(Bits, 1, true);
  bool UMValid = false;

  // UM is perhaps unavailable, let's check
  if (!CurLoop->isUnknown()) {
    const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr();
    if (UpperBound->isIntConstant(&UBVal)) {
      UM = llvm::APInt(64, UBVal, true);
      LLVM_DEBUG(dbgs() << "\t    UM = " << UM << "\n");
      UMValid = true;
    }
  }

  APInt TU(APInt::getSignedMaxValue(Bits));
  APInt TL(APInt::getSignedMinValue(Bits));

  // test(BM/G, LM-X) and test(-BM/G, X-UM)
  APInt TMUL = BM.sdiv(G);
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(-X, TMUL));
    LLVM_DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    if (UMValid) {
      TU = minAPInt(TU, floorOfQuotient(UM - X, TMUL));
      LLVM_DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    }
  } else {
    TU = minAPInt(TU, floorOfQuotient(-X, TMUL));
    LLVM_DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    if (UMValid) {
      TL = maxAPInt(TL, ceilingOfQuotient(UM - X, TMUL));
      LLVM_DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    }
  }

  // test(AM/G, LM-Y) and test(-AM/G, Y-UM)
  TMUL = AM.sdiv(G);
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(-Y, TMUL));
    LLVM_DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    if (UMValid) {
      TU = minAPInt(TU, floorOfQuotient(UM - Y, TMUL));
      LLVM_DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    }
  } else {
    TU = minAPInt(TU, floorOfQuotient(-Y, TMUL));
    LLVM_DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    if (UMValid) {
      TL = maxAPInt(TL, ceilingOfQuotient(UM - Y, TMUL));
      LLVM_DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    }
  }
  if (TL.sgt(TU)) {
    ++ExactSIVindependence;
    ++ExactSIVsuccesses;
    return true;
  }

  // There is a bug in this algorithm.
  // Suppress this section of code for now and use Banerjee's Inequalities
  // instead The algorithm can be fixed as follows but will experiment in the
  // future if needed
  //
  // Right before testing for less than and before saving TU,TL
  // Swap the TL and TU if it's range is less than 0
  // TL and TU represents the range of IV that can overlap, in normalized form

  //  if (TU.slt(0)) {
  //    APInt tmp = TU;
  //    TU = -TL;
  //    TL = -tmp;
  //    LLVM_DEBUG (dbgs() << "\nSwapped TL TU=" << TL << " " << TU << "\n");
  //  }

  return false;

#if 0

  // explore directions
  DVKind NewDirection = DVKind::NONE;

  // less than
  APInt SaveTU(TU); // save these
  APInt SaveTL(TL);
  LLVM_DEBUG(dbgs() << "\t    exploring LT direction\n");
  TMUL = AM - BM;
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(X - Y + 1, TMUL));
    LLVM_DEBUG(dbgs() << "\t\t    TL = " << TL << "\n");
  } else {
    TU = minAPInt(TU, floorOfQuotient(X - Y + 1, TMUL));
    LLVM_DEBUG(dbgs() << "\t\t    TU = " << TU << "\n");
  }
  if (TL.sle(TU)) {
    NewDirection |= DVKind::LT;
    ++ExactSIVsuccesses;
  }

  // equal

  // TU = SaveTU;
  // TL = SaveTL;

  // This algorithm does not set "=" in this case:
  //
  // do i=0,21
  //  a(63 - 3*i) =
  //  a(126 -6*i) =
  //  DV should be (<=)  but the "=' part is not set

  // if (TMUL.sgt(0)) {
  //   TL = maxAPInt(TL, ceilingOfQuotient(X - Y, TMUL));
  //   LLVM_DEBUG(dbgs() << "\t\t    TL = " << TL << "\n");
  //  } else {
  //    TU = minAPInt(TU, floorOfQuotient(X - Y, TMUL));
  //   LLVM_DEBUG(dbgs() << "\t\t    TU = " << TU << "\n");
  // }
  // TMUL = BM - AM;
  // if (TMUL.sgt(0)) {
  //   TL = maxAPInt(TL, ceilingOfQuotient(Y - X, TMUL));
  //   LLVM_DEBUG(dbgs() << "\t\t    TL = " << TL << "\n");
  // } else {
  //   TU = minAPInt(TU, floorOfQuotient(Y - X, TMUL));
  //   LLVM_DEBUG(dbgs() << "\t\t    TU = " << TU << "\n");
  // }
  // if (TL.sle(TU)) {
  //  NewDirection |= DVKind::EQ;
  //  ++ExactSIVsuccesses;
  // }

  // Algorithm revised:
  // For tesing (=) it can be done simply as follows.
  // IV  = diff_of_the_constants / diff_of_coeffs
  // Check if IV is within this range:  [0, Upperbound]

  LLVM_DEBUG(dbgs() << "\t    exploring EQ direction\n");

  int64_t CoeffDeltaVal = SrcCoeffVal - DstCoeffVal;
  assert(CoeffDeltaVal != 0 && "Coeffs not expected to be equal here");
  if ((ConstDeltaVal % CoeffDeltaVal) == 0) {
    int64_t IVVal = ConstDeltaVal / CoeffDeltaVal;
    if (IVVal >= 0) {
      bool SetEQ = false;
      if (UMValid) {
        if (IVVal <= UBVal) {
          SetEQ = true;
        }
      } else {
        SetEQ = true;
      }
      if (SetEQ) {
        NewDirection |= DVKind::EQ;
        ++ExactSIVsuccesses;
      }
    }
  }

  TMUL = BM - AM;
  // greater than
  TU = SaveTU; // restore
  TL = SaveTL;
  LLVM_DEBUG(dbgs() << "\t    exploring GT direction\n");
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(Y - X + 1, TMUL));
    LLVM_DEBUG(dbgs() << "\t\t    TL = " << TL << "\n");
  } else {
    TU = minAPInt(TU, floorOfQuotient(Y - X + 1, TMUL));
    LLVM_DEBUG(dbgs() << "\t\t    TU = " << TU << "\n");
  }
  if (TL.sle(TU)) {
    NewDirection |= DVKind::GT;
    ++ExactSIVsuccesses;
  }

  // finished
  Result.DV[Level].Direction &= NewDirection;
  if (Result.DV[Level].Direction == DVKind::NONE)
    ++ExactSIVindependence;
  return Result.DV[Level].Direction == DVKind::NONE;

#endif
}

// Return true if the divisor evenly divides the dividend.
static bool isRemainderZero(const CanonExpr *Dividend,
                            const CanonExpr *Divisor) {

  int64_t K1, K2;

  Dividend->isIntConstant(&K1);
  APInt ConstDividend = llvm::APInt(64, K1, true);
  Divisor->isIntConstant(&K2);
  APInt ConstDivisor = llvm::APInt(64, K2, true);
  return ConstDividend.srem(ConstDivisor) == 0;
}

// weakZeroSrcSIVtest -
// From the paper, Practical Dependence Testing, Section 4.2.2
//
// When we have a pair of subscripts of the form [c1] and [c2 + a*i],
// where i is an induction variable, c1 and c2 are loop invariant,
// and a is a constant, we can solve it exactly using the
// Weak-Zero SIV test.
//
// Given
//
//    c1 = c2 + a*i
//
// we get
//
//    (c1 - c2)/a = i
//
// Src:  X[c1]; Dst:  X(c2 + a *i]
//
// If the dependence happens at first iteration when i = 0,
//       the direction is >= when a > 0
//       the direction is = when a < 0
//       for simplicty, we will create >= because it includes =
//   Peeling the  1st iteration will break the dependence.
//
// If the dependence happens at last iteration when i = UB,
//       the direction is <=
//   Peeling the last iteration will break the dependence.
// Otherwise, the direction is *.
//
// Can prove independence. Failing that, we can sometimes refine
// the directions. Can sometimes show that first or last
// iteration carries all the dependences (so worth peeling).
//
// (see also weakZeroDstSIVtest)
//
// Return true if dependence disproved.
bool DDTest::weakZeroSrcSIVtest(const CanonExpr *DstCoeff,
                                const CanonExpr *SrcConst,
                                const CanonExpr *DstConst,
                                const HLLoop *CurLoop, unsigned Level,
                                Dependences &Result,
                                Constraint &NewConstraint) {
  // For the WeakSIV test, it's possible the loop isn't common to
  // the Src and Dst loops. If it isn't, then there's no need to
  // record a direction.
  LLVM_DEBUG(dbgs() << "\nWeak-Zero (src) SIV test\n");
  LLVM_DEBUG(dbgs() << "\n    DstCoeff = "; DstCoeff->dump());
  LLVM_DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  LLVM_DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());

  ++WeakZeroSIVapplications;
  assert(0 < Level && Level <= MaxLevels && "Level out of range");

  //  Levels are decremented here (as in original version) because the nesting
  //  level
  //  starts from 1 and the indexes below start with 0.
  //  Ideally it should  be hidden by using member functions but it's easier to
  //  read
  //  when different logical operations are possibly used

  Level--;
  Result.Consistent = false;
  const CanonExpr *Delta = getMinus(SrcConst, DstConst);
  if (!Delta) {
    return false;
  }
  NewConstraint.setLine(getConstantWithType(Delta->getSrcType(), 0), DstCoeff,
                        Delta, CurLoop);

  LLVM_DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
  if (isKnownPredicate(CmpInst::ICMP_EQ, SrcConst, DstConst)) {
    if (Level < CommonLevels) {
      // Srce: A[2] ; Dst: A[2*i +2];  DV should be >=
      Result.DV[Level].Direction &= DVKind::GE;
      Result.DV[Level].PeelFirst = true;
      ++WeakZeroSIVsuccesses;
    }
    return false; // dependences caused by first iteration
  }

  int64_t CoeffValue;
  const CanonExpr *ConstCoeff = DstCoeff;
  if (!(DstCoeff->isIntConstant(&CoeffValue))) {
    return false;
  }

  const CanonExpr *AbsCoeff = HLNodeUtils::isKnownNegative(ConstCoeff, CurLoop)
                                  ? getNegative(ConstCoeff)
                                  : ConstCoeff;
  const CanonExpr *NewDelta = nullptr;
  if (HLNodeUtils::isKnownNonNegative(Delta, CurLoop)) {
    NewDelta = Delta;
  } else if (HLNodeUtils::isKnownNegative(Delta, CurLoop)) {
    NewDelta = getNegative(Delta);
  } else {
    // Coeff not known to be positive or negative.
    return false;
  }

  // check that Delta/SrcCoeff < iteration count
  // really check NewDelta < count*AbsCoeff
  if (!CurLoop->isUnknown()) {
    const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr();
    LLVM_DEBUG(dbgs() << "\n    UpperBound = "; UpperBound->dump());

    const CanonExpr *Product = getMulExpr(AbsCoeff, UpperBound);

    if (Product == nullptr) {
      return false;
    }

    if (isKnownPredicate(CmpInst::ICMP_SGT, NewDelta, Product)) {
      ++WeakZeroSIVindependence;
      ++WeakZeroSIVsuccesses;
      return true;
    }
    if (isKnownPredicate(CmpInst::ICMP_EQ, NewDelta, Product)) {
      // dependences caused by last iteration
      if (Level < CommonLevels) {
        Result.DV[Level].Direction &= DVKind::LE;
        Result.DV[Level].PeelLast = true;
        ++WeakZeroSIVsuccesses;
      }
      return false;
    }
  }

  // check that Delta/SrcCoeff >= 0
  // really check that NewDelta >= 0
  if (HLNodeUtils::isKnownNegative(NewDelta, CurLoop)) {
    // No dependence, newDelta < 0
    ++WeakZeroSIVindependence;
    ++WeakZeroSIVsuccesses;
    return true;
  }

  // if SrcCoeff doesn't divide Delta, then no dependence
  int64_t k1;
  if (Delta->isIntConstant(&k1) && !isRemainderZero(Delta, ConstCoeff)) {
    ++WeakZeroSIVindependence;
    ++WeakZeroSIVsuccesses;
    return true;
  }
  return false;
}

// weakZeroDstSIVtest -
// From the paper, Practical Dependence Testing, Section 4.2.2
//
// When we have a pair of subscripts of the form [c1 + a*i] and [c2],
// where i is an induction variable, c1 and c2 are loop invariant,
// and a is a constant, we can solve it exactly using the
// Weak-Zero SIV test.
//
// Given
//
//    c1 + a*i = c2
//
// we get
//
//    i = (c2 - c1)/a
//
// Src:   X(c1 + a *i]  Dst: X[c2]
//
// If the dependence happens at first iteration when i = 0,
//       the direction is = when a > 0
//       the direction is <= when a < 0
//       for simplicty, we will create <= because it includes =
//   Peeling the  1st iteration will break the dependence.
//
// If the dependence happens at last iteration when i = UB,
//       the direction is >=
//   Peeling the last iteration will break the dependence.
// Otherwise, the direction is *.
// Can prove independence. Failing that, we can sometimes refine
// the directions. Can sometimes show that first or last
// iteration carries all the dependences (so worth peeling).
//
// (see also weakZeroSrcSIVtest)
//
// Return true if dependence disproved.
bool DDTest::weakZeroDstSIVtest(const CanonExpr *SrcCoeff,
                                const CanonExpr *SrcConst,
                                const CanonExpr *DstConst,
                                const HLLoop *CurLoop, unsigned Level,
                                Dependences &Result,
                                Constraint &NewConstraint) {
  // For the WeakSIV test, it's possible the loop isn't common to the
  // Src and Dst loops. If it isn't, then there's no need to record a direction.
  LLVM_DEBUG(dbgs() << "\nWeak-Zero (dst) SIV test\n");
  LLVM_DEBUG(dbgs() << "\nSrcCoeff = "; SrcCoeff->dump());
  LLVM_DEBUG(dbgs() << "\nSrcConst = "; SrcConst->dump());
  LLVM_DEBUG(dbgs() << "\nDstConst = "; DstConst->dump());

  ++WeakZeroSIVapplications;
  assert(0 < Level && Level <= SrcLevels && "Level out of range");
  Level--;
  Result.Consistent = false;
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);

  if (!Delta) {
    return false;
  }
  NewConstraint.setLine(SrcCoeff, getConstantWithType(Delta->getSrcType(), 0),
                        Delta, CurLoop);
  LLVM_DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
  if (isKnownPredicate(CmpInst::ICMP_EQ, DstConst, SrcConst)) {
    if (Level < CommonLevels) {
      Result.DV[Level].Direction &= DVKind::LE;
      Result.DV[Level].PeelFirst = true;
      ++WeakZeroSIVsuccesses;
    }
    return false; // dependences caused by first iteration
  }

  int64_t K1;

  const CanonExpr *ConstCoeff = SrcCoeff;

  if (ConstCoeff->isIntConstant(&K1)) {
    return false;
  }
  const CanonExpr *AbsCoeff = HLNodeUtils::isKnownNegative(ConstCoeff, CurLoop)
                                  ? getNegative(ConstCoeff)
                                  : ConstCoeff;
  const CanonExpr *NewDelta = nullptr;
  if (HLNodeUtils::isKnownNonNegative(Delta, CurLoop)) {
    NewDelta = Delta;
  } else if (HLNodeUtils::isKnownNegative(Delta, CurLoop)) {
    NewDelta = getNegative(Delta);
  } else {
    // Coeff not known to be positive or negative.
    return false;
  }

  // check that Delta/SrcCoeff < iteration count
  // really check NewDelta < count*AbsCoeff
  if (!CurLoop->isUnknown()) {
    const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr();
    LLVM_DEBUG(dbgs() << "\n    UpperBound = "; UpperBound->dump());
    const CanonExpr *Product = getMulExpr(AbsCoeff, UpperBound);

    if (Product == nullptr) {
      return false;
    }

    if (isKnownPredicate(CmpInst::ICMP_SGT, NewDelta, Product)) {
      ++WeakZeroSIVindependence;
      ++WeakZeroSIVsuccesses;
      return true;
    }
    if (isKnownPredicate(CmpInst::ICMP_EQ, NewDelta, Product)) {
      // dependences caused by last iteration
      if (Level < CommonLevels) {
        Result.DV[Level].Direction &= DVKind::GE;
        Result.DV[Level].PeelLast = true;
        ++WeakZeroSIVsuccesses;
      }
      return false;
    }
  }

  // check that Delta/SrcCoeff >= 0
  // really check that NewDelta >= 0
  if (HLNodeUtils::isKnownNegative(NewDelta, CurLoop)) {
    // No dependence, newDelta < 0
    ++WeakZeroSIVindependence;
    ++WeakZeroSIVsuccesses;
    return true;
  }

  // if SrcCoeff doesn't divide Delta, then no dependence

  if (!(Delta->isIntConstant(&K1))) {
    return false;
  }

  if (!isRemainderZero(Delta, ConstCoeff)) {
    ++WeakZeroSIVindependence;
    ++WeakZeroSIVsuccesses;
    return true;
  }
  return false;
}

// exactRDIVtest - Tests the RDIV subscript pair for dependence.
// Things of the form [c1 + a*i] and [c2 + b*j],
// where i and j are induction variable, c1 and c2 are loop invariant,
// and a and b are constants.
// Likewise,   [c1] and [a*i + b*j + c2] are RDIV subscripts
// Returns true if any possible dependence is disproved.
// Marks the result as inconsistent.
// Works in some cases that symbolicRDIVtest doesn't, and vice versa.

bool DDTest::exactRDIVtest(const CanonExpr *SrcCoeff, const CanonExpr *DstCoeff,
                           const CanonExpr *SrcConst, const CanonExpr *DstConst,
                           const HLLoop *SrcLoop, const HLLoop *DstLoop,
                           Dependences &Result) {

  LLVM_DEBUG(dbgs() << "\nExact RDIV test\n");
  LLVM_DEBUG(dbgs() << "\n    SrcCoeff = "; SrcCoeff->dump());
  LLVM_DEBUG(dbgs() << " = AM\n");
  LLVM_DEBUG(dbgs() << "\n    DstCoeff = "; DstCoeff->dump());
  LLVM_DEBUG(dbgs() << " = BM\n");
  LLVM_DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  LLVM_DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());
  ++ExactRDIVapplications;
  Result.Consistent = false;
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);

  if (!Delta) {
    return false;
  }
  LLVM_DEBUG(dbgs() << "\n    Delta = "; Delta->dump());

  int64_t deltaVal, srcCoeffVal, dstCoeffVal, ubVal;

  if (!(Delta->isIntConstant(&deltaVal)) ||
      !(SrcCoeff->isIntConstant(&srcCoeffVal)) ||
      !(DstCoeff->isIntConstant(&dstCoeffVal))) {
    return false;
  }

  // find gcd
  APInt G, X, Y;

  APInt AM = llvm::APInt(64, srcCoeffVal, true);
  APInt BM = llvm::APInt(64, dstCoeffVal, true);
  APInt DM = llvm::APInt(64, deltaVal, true);

  unsigned Bits = AM.getBitWidth();
  if (findGCD(Bits, AM, BM, DM, G, X, Y)) {
    // gcd doesn't divide Delta, no dependence
    ++ExactRDIVindependence;
    return true;
  }

  LLVM_DEBUG(dbgs() << "\n    X = " << X << ", Y = " << Y << ", G = " << G
                    << "\n");

  // since CE construction is normalized, LM = 0
  APInt SrcUM(Bits, 1, true);
  bool SrcUMvalid = false;
  // SrcUM is perhaps unavailable, let's check

  if (!SrcLoop->isUnknown()) {
    const CanonExpr *UpperBound = SrcLoop->getUpperCanonExpr();
    if (UpperBound->isIntConstant(&ubVal)) {
      SrcUM = llvm::APInt(64, ubVal, true);
      LLVM_DEBUG(dbgs() << "\t    SrcUM = " << SrcUM << "\n");
      SrcUMvalid = true;
    }
  }

  APInt DstUM(Bits, 1, true);
  bool DstUMvalid = false;

  // UM is perhaps unavailable, let's check
  if (!DstLoop->isUnknown()) {
    const CanonExpr *UpperBound = DstLoop->getUpperCanonExpr();
    if (UpperBound->isIntConstant(&ubVal)) {
      DstUM = llvm::APInt(64, ubVal, true);
      LLVM_DEBUG(dbgs() << "\t    DstUM = " << DstUM << "\n");
      DstUMvalid = true;
    }
  }

  APInt TU(APInt::getSignedMaxValue(Bits));
  APInt TL(APInt::getSignedMinValue(Bits));

  // test(BM/G, LM-X) and test(-BM/G, X-UM)
  APInt TMUL = BM.sdiv(G);
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(-X, TMUL));
    LLVM_DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    if (SrcUMvalid) {
      TU = minAPInt(TU, floorOfQuotient(SrcUM - X, TMUL));
      LLVM_DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    }
  } else {
    TU = minAPInt(TU, floorOfQuotient(-X, TMUL));
    LLVM_DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    if (SrcUMvalid) {
      TL = maxAPInt(TL, ceilingOfQuotient(SrcUM - X, TMUL));
      LLVM_DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    }
  }

  // test(AM/G, LM-Y) and test(-AM/G, Y-UM)
  TMUL = AM.sdiv(G);
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(-Y, TMUL));
    LLVM_DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    if (DstUMvalid) {
      TU = minAPInt(TU, floorOfQuotient(DstUM - Y, TMUL));
      LLVM_DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    }
  } else {
    TU = minAPInt(TU, floorOfQuotient(-Y, TMUL));
    LLVM_DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    if (DstUMvalid) {
      TL = maxAPInt(TL, ceilingOfQuotient(DstUM - Y, TMUL));
      LLVM_DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    }
  }
  if (TL.sgt(TU)) {
    ++ExactRDIVindependence;
  }

  return TL.sgt(TU);
}

// symbolicRDIVtest -
// In Section 4.5 of the Practical Dependence Testing paper,the authors
// introduce a special case of Banerjee's Inequalities (also called the
// Extreme-Value Test) that can handle some of the SIV and RDIV cases,
// particularly cases with symbolics. Since it's only able to disprove
// dependence (not compute distances or directions), we'll use it as a
// fall back for the other tests.
//
// When we have a pair of subscripts of the form [c1 + a1*i] and [c2 + a2*j]
// where i and j are induction variables and c1 and c2 are loop invariants,
// we can use the symbolic tests to disprove some dependences, serving as a
// backup for the RDIV test. Note that i and j can be the same variable,
// letting this test serve as a backup for the various SIV tests.
//
// For a dependence to exist, c1 + a1*i must equal c2 + a2*j for some
//  0 <= i <= N1 and some 0 <= j <= N2, where N1 and N2 are the (normalized)
// loop bounds for the i and j loops, respectively. So, ...
//
// c1 + a1*i = c2 + a2*j
// a1*i - a2*j = c2 - c1
//
// To test for a dependence, we compute c2 - c1 and make sure it's in the
// range of the maximum and minimum possible values of a1*i - a2*j.
// Considering the signs of a1 and a2, we have 4 possible cases:
//
// 1) If a1 >= 0 and a2 >= 0, then
//        a1*0 - a2*N2 <= c2 - c1 <= a1*N1 - a2*0
//              -a2*N2 <= c2 - c1 <= a1*N1
//
// 2) If a1 >= 0 and a2 <= 0, then
//        a1*0 - a2*0 <= c2 - c1 <= a1*N1 - a2*N2
//                  0 <= c2 - c1 <= a1*N1 - a2*N2
//
// 3) If a1 <= 0 and a2 >= 0, then
//        a1*N1 - a2*N2 <= c2 - c1 <= a1*0 - a2*0
//        a1*N1 - a2*N2 <= c2 - c1 <= 0
//
// 4) If a1 <= 0 and a2 <= 0, then
//        a1*N1 - a2*0  <= c2 - c1 <= a1*0 - a2*N2
//        a1*N1         <= c2 - c1 <=       -a2*N2
//
// return true if dependence disproved
bool DDTest::symbolicRDIVtest(const CanonExpr *A1, const CanonExpr *A2,
                              const CanonExpr *C1, const CanonExpr *C2,
                              const HLLoop *Loop1, const HLLoop *Loop2) {

  ++SymbolicRDIVapplications;

  LLVM_DEBUG(dbgs() << "\ntry symbolic RDIV test\n");
  LLVM_DEBUG(dbgs() << "\n    A1 = "; A1->dump());
  LLVM_DEBUG(dbgs() << ", src type = " << *(A1->getSrcType()));
  LLVM_DEBUG(dbgs() << ", dest type = " << *(A1->getDestType()) << "\n");
  LLVM_DEBUG(dbgs() << "\n    A2 = "; A2->dump());
  LLVM_DEBUG(dbgs() << "\n    C1 = "; C1->dump());
  LLVM_DEBUG(dbgs() << "\n    C2 = "; C2->dump());

  const CanonExpr *N1 =
      !Loop1->isUnknown() ? Loop1->getUpperCanonExpr() : nullptr;
  const CanonExpr *N2 =
      !Loop2->isUnknown() ? Loop2->getUpperCanonExpr() : nullptr;

  if (N1) {
    LLVM_DEBUG(dbgs() << "\n    N1 = "; N1->dump());
  }
  if (N2) {
    LLVM_DEBUG(dbgs() << "\n    N2 = "; N2->dump());
  }
  const CanonExpr *C2_C1 = getMinus(C2, C1);
  if (!C2_C1) {
    return false;
  }
  const CanonExpr *C1_C2 = getNegative(C2_C1);

  LLVM_DEBUG(dbgs() << "\n    C2 - C1 = "; C2_C1->dump());
  LLVM_DEBUG(dbgs() << "\n    C1 - C2 = "; C1_C2->dump());

  if (HLNodeUtils::isKnownNonNegative(A1, DeepestLoop)) {
    if (HLNodeUtils::isKnownNonNegative(A2, DeepestLoop)) {
      // A1 >= 0 && A2 >= 0
      if (N1) {
        // make sure that c2 - c1 <= a1*N1
        const CanonExpr *A1N1 = getMulExpr(A1, N1);
        if (A1N1 == nullptr) {
          return false;
        }

        LLVM_DEBUG(dbgs() << "\n    A1*N1 = "; A1N1->dump());
        if (isKnownPredicate(CmpInst::ICMP_SGT, C2_C1, A1N1)) {
          ++SymbolicRDIVindependence;
          return true;
        }
      }
      if (N2) {
        // make sure that -a2*N2 <= c2 - c1, or a2*N2 >= c1 - c2
        const CanonExpr *A2N2 = getMulExpr(A2, N2);
        if (A2N2 == nullptr) {
          return false;
        }

        LLVM_DEBUG(dbgs() << "\n    A2*N2 = "; A2N2->dump());
        if (isKnownPredicate(CmpInst::ICMP_SLT, A2N2, C1_C2)) {
          ++SymbolicRDIVindependence;
          return true;
        }
      }
    } else if (HLNodeUtils::isKnownNonPositive(A2, DeepestLoop)) {
      // a1 >= 0 && a2 <= 0
      if (N1 && N2) {
        // make sure that c2 - c1 <= a1*N1 - a2*N2
        const CanonExpr *A1N1 = getMulExpr(A1, N1);
        const CanonExpr *A2N2 = getMulExpr(A2, N2);
        const CanonExpr *A1N1_A2N2 = getMinus(A1N1, A2N2);

        if (A1N1 == nullptr || A2N2 == nullptr || A1N1_A2N2 == nullptr) {
          return false;
        }

        LLVM_DEBUG(dbgs() << "\n A1*N1 - A2*N2 = "; A1N1_A2N2->dump());
        if (isKnownPredicate(CmpInst::ICMP_SGT, C2_C1, A1N1_A2N2)) {
          ++SymbolicRDIVindependence;
          return true;
        }
      }
      // make sure that 0 <= c2 - c1
      if (HLNodeUtils::isKnownNegative(C2_C1, DeepestLoop)) {
        ++SymbolicRDIVindependence;
        return true;
      }
    }
  } else if (HLNodeUtils::isKnownNonPositive(A1, DeepestLoop)) {
    if (HLNodeUtils::isKnownNonNegative(A2, DeepestLoop)) {
      // a1 <= 0 && a2 >= 0
      if (N1 && N2) {
        // make sure that a1*N1 - a2*N2 <= c2 - c1
        const CanonExpr *A1N1 = getMulExpr(A1, N1);
        const CanonExpr *A2N2 = getMulExpr(A2, N2);
        const CanonExpr *A1N1_A2N2 = getMinus(A1N1, A2N2);

        if (A1N1 == nullptr || A2N2 == nullptr || A1N1_A2N2 == nullptr) {
          return false;
        }

        LLVM_DEBUG(dbgs() << "\n A1*N1 - A2*N2 = "; A1N1_A2N2->dump());
        if (isKnownPredicate(CmpInst::ICMP_SGT, A1N1_A2N2, C2_C1)) {
          ++SymbolicRDIVindependence;
          return true;
        }
      }
      // make sure that c2 - c1 <= 0
      if (HLNodeUtils::isKnownPositive(C2_C1, DeepestLoop)) {
        ++SymbolicRDIVindependence;
        return true;
      }
    } else if (HLNodeUtils::isKnownNonPositive(A2, DeepestLoop)) {
      // a1 <= 0 && a2 <= 0
      if (N1) {
        // make sure that a1*N1 <= c2 - c1
        const CanonExpr *A1N1 = getMulExpr(A1, N1);
        if (!A1N1) {
          return false;
        }

        LLVM_DEBUG(dbgs() << "\n A1*N1 = "; A1N1->dump());
        if (isKnownPredicate(CmpInst::ICMP_SGT, A1N1, C2_C1)) {
          ++SymbolicRDIVindependence;
          return true;
        }
      }
      if (N2) {
        // make sure that c2 - c1 <= -a2*N2, or c1 - c2 >= a2*N2
        const CanonExpr *A2N2 = getMulExpr(A2, N2);
        if (!A2N2) {
          return false;
        }

        LLVM_DEBUG(dbgs() << "\t    A2*N2 = "; A2N2->dump());
        if (isKnownPredicate(CmpInst::ICMP_SLT, C1_C2, A2N2)) {
          ++SymbolicRDIVindependence;
          return true;
        }
      }
    }
  }
  return false;
}

// testSIV -
// When we have a pair of subscripts of the form [c1 + a1*i] and [c2 - a2*i]
// where i is an induction variable, c1 and c2 are loop invariant, and a1 and
// a2 are constant, we attack it with an SIV test. While they can all be
// solved with the Exact SIV test, it's worthwhile to use simpler tests when
// they apply; they're cheaper and sometimes more precise.
//
// Return true if dependence disproved.
bool DDTest::testSIV(const CanonExpr *Src, const CanonExpr *Dst,
                     unsigned &Level, Dependences &Result,
                     Constraint &NewConstraint, const CanonExpr *&SplitIter,
                     const HLLoop *SrcParentLoop, const HLLoop *DstParentLoop,
                     bool &TestMore, bool ForFusion) {

  LLVM_DEBUG(dbgs() << "\n Test SIV \n");
  LLVM_DEBUG(dbgs() << "\n   src = "; Src->dump());
  LLVM_DEBUG(dbgs() << "\n");
  LLVM_DEBUG(dbgs() << "   dst = "; Dst->dump());
  LLVM_DEBUG(dbgs() << "\n");

  const HLLoop *SrcLoop = getLoop(Src, SrcParentLoop);
  const HLLoop *DstLoop = getLoop(Dst, DstParentLoop);

  TestMore = false;

  if (SrcLoop && DstLoop) {
    const CanonExpr *SrcConst = getInvariant(Src);
    const CanonExpr *DstConst = getInvariant(Dst);
    const CanonExpr *SrcCoeff = getCoeff(Src);
    const CanonExpr *DstCoeff = getCoeff(Dst);
    const HLLoop *CurLoop = SrcLoop;
    assert((SrcLoop == DstLoop || ForFusion) &&
           "both loops in SIV should be same");
    Level = mapSrcLoop(CurLoop);
    bool Disproven;
    bool ExactTestDone = true;

    if (CanonExprUtils::areEqual(SrcCoeff, DstCoeff, true)) {
      Disproven = strongSIVtest(SrcCoeff, SrcConst, DstConst, CurLoop, Level,
                                Result, NewConstraint);
    } else if (CanonExprUtils::areEqual(SrcCoeff, getNegative(DstCoeff),
                                        true)) {
      Disproven = weakCrossingSIVtest(SrcCoeff, SrcConst, DstConst, CurLoop,
                                      Level, Result, NewConstraint, SplitIter);
    } else {
      Disproven = exactSIVtest(SrcCoeff, DstCoeff, SrcConst, DstConst, CurLoop,
                               Level, Result, NewConstraint);

      ExactTestDone = Disproven;
    }

    Disproven = (Disproven ||
                 gcdMIVtest(Src, Dst, SrcParentLoop, DstParentLoop, Result) ||
                 symbolicRDIVtest(SrcCoeff, DstCoeff, SrcConst, DstConst,
                                  CurLoop, CurLoop));

    if (!ExactTestDone && !Disproven) {
      TestMore = true;
    }

    return Disproven;
  }

  if (SrcLoop) {
    const CanonExpr *SrcConst = getInvariant(Src);
    const CanonExpr *SrcCoeff = getCoeff(Src);
    const CanonExpr *DstConst = getInvariant(Dst);
    const HLLoop *CurLoop = SrcLoop;
    Level = mapSrcLoop(CurLoop);
    // WeakZero: src: A[i] vs Dst: A[1]
    return weakZeroDstSIVtest(SrcCoeff, SrcConst, DstConst, CurLoop, Level,
                              Result, NewConstraint) ||
           gcdMIVtest(Src, Dst, SrcParentLoop, DstParentLoop, Result);
  }

  if (DstLoop) {
    const CanonExpr *DstConst = getInvariant(Dst);
    const CanonExpr *DstCoeff = getCoeff(Dst);
    const CanonExpr *SrcConst = getInvariant(Src);
    const HLLoop *CurLoop = DstLoop;
    Level = mapDstLoop(CurLoop);
    return weakZeroSrcSIVtest(DstCoeff, SrcConst, DstConst, CurLoop, Level,
                              Result, NewConstraint) ||
           gcdMIVtest(Src, Dst, SrcParentLoop, DstParentLoop, Result);
  }

  llvm_unreachable("SIV test expected at least one Linear");

  return false;
}

// testRDIV -
// When we have a pair of subscripts of the form [c1 + a1*i] and [c2 + a2*j]
// where i and j are induction variables, c1 and c2 are loop invariant,
// and a1 and a2 are constant, we can solve it exactly with an easy adaptation
// of the Exact SIV test, the Restricted Double Index Variable (RDIV) test.
// It doesn't make sense to talk about distance or direction in this case,
// so there's no point in making special versions of the Strong SIV test or
// the Weak-crossing SIV test.
//
// With minor algebra, this test can also be used for things like
// [c1 + a1*i + a2*j][c2].
//
// Return true if dependence disproved.

bool DDTest::testRDIV(const CanonExpr *Src, const CanonExpr *Dst,
                      Dependences &Result, const HLLoop *SrcParentLoop,
                      const HLLoop *DstParentLoop) {

  // we have 3 possible situations here:
  //   1) [a*i + b] and [c*j + d]
  //   2) [a*i + c*j + b] and [d]
  //   3) [b] and [a*i + c*j + d]
  // We need to find what we've got and get organized

  const CanonExpr *SrcConst = nullptr;
  const CanonExpr *DstConst = nullptr;
  const CanonExpr *SrcCoeff = nullptr;
  const CanonExpr *DstCoeff = nullptr;
  const HLLoop *SrcLoop = nullptr;
  const HLLoop *DstLoop = nullptr;

  LLVM_DEBUG(dbgs() << "\n    src = "; Src->dump());
  LLVM_DEBUG(dbgs() << "\n    Dst = "; Dst->dump());

  if (Src->hasIV() && Dst->hasIV()) {
    // case 1)
    SrcConst = getInvariant(Src);
    SrcCoeff = getCoeff(Src);
    SrcLoop = getLoop(Src, SrcParentLoop);
    DstConst = getInvariant(Dst);
    DstCoeff = getCoeff(Dst);
    DstLoop = getLoop(Dst, DstParentLoop);
  } else if (Src->hasIV()) {
    // 2) move c*j from src to Dst
    SrcConst = getInvariant(Src);
    SrcCoeff = getFirstCoeff(Src);
    SrcLoop = getFirstLoop(Src, SrcParentLoop);
    DstConst = Dst;
    DstCoeff = getNegative(getSecondCoeff(Src));
    DstLoop = getSecondLoop(Src, SrcParentLoop);
  } else if (Dst->hasIV()) {
    // 3) move  c*j from Dst to Src
    DstConst = getInvariant(Dst);
    DstCoeff = getFirstCoeff(Dst);
    DstLoop = getFirstLoop(Dst, DstParentLoop);
    SrcConst = Src;
    SrcCoeff = getNegative(getSecondCoeff(Dst));
    SrcLoop = getSecondLoop(Dst, DstParentLoop);
  } else {
    llvm_unreachable("RDIV expected at least one iv");
  }

  return exactRDIVtest(SrcCoeff, DstCoeff, SrcConst, DstConst, SrcLoop, DstLoop,
                       Result) ||
         gcdMIVtest(Src, Dst, SrcParentLoop, DstParentLoop, Result) ||
         symbolicRDIVtest(SrcCoeff, DstCoeff, SrcConst, DstConst, SrcLoop,
                          DstLoop);
}

// Tests the single-subscript MIV pair (Src and Dst) for dependence.
// Return true if dependence disproved.
// Can sometimes refine direction vectors.
bool DDTest::testMIV(const CanonExpr *Src, const CanonExpr *Dst,
                     const DirectionVector &InputDV,
                     const SmallBitVector &Loops, Dependences &Result,
                     const HLLoop *SrcParentLoop, const HLLoop *DstParentLoop) {

  LLVM_DEBUG(dbgs() << "\n   src = "; Src->dump());
  LLVM_DEBUG(dbgs() << "\n   dst = "; Dst->dump());
  Result.Consistent = false;
  return gcdMIVtest(Src, Dst, SrcParentLoop, DstParentLoop, Result) ||
         banerjeeMIVtest(Src, Dst, InputDV, Loops, Result, SrcParentLoop,
                         DstParentLoop);
}

#if 0
// Given a product, e.g., 10*X*Y, returns the first constant operand,
// in this case 10. If there is no constant part, returns NULL.
static const CanonExpr *getConstantPart(const CanonExpr *Product) {

  // Handles only constants  now.
  // TODO: check blob of the form 10 * N and return 10

  int64_t k1;
  if (Product->isIntConstant(&k1)) {
    return Product;
  }
  return nullptr;
}
#endif

//===----------------------------------------------------------------------===//
// gcdMIVtest -
// Tests an MIV subscript pair for dependence.
// Returns true if any possible dependence is disproved.
// Marks the result as inconsistent.
// Can sometimes disprove the equal direction for 1 or more loops,
// as discussed in Michael Wolfe's book,
// High Performance Compilers for Parallel Computing, page 235.
//
// We spend some effort (code!) to handle cases like
// [10*i + 5*N*j + 15*M + 6], where i and j are induction variables,
// but M and N are just loop-invariant variables.
// This should help us handle linearized subscripts;
// also makes this test a useful backup to the various SIV tests.
//
// It occurs to me that the presence of loop-invariant variables
// changes the nature of the test from "greatest common divisor"
// to "a common divisor".

bool DDTest::gcdMIVtest(const CanonExpr *Src, const CanonExpr *Dst,
                        const HLLoop *SrcParentLoop,
                        const HLLoop *DstParentLoop, Dependences &Result) {

  LLVM_DEBUG(dbgs() << "\nstarting gcd\n");
  LLVM_DEBUG(dbgs() << "\n   src = "; Src->dump());
  LLVM_DEBUG(dbgs() << "\n   dst = "; Dst->dump());

  ++GCDapplications;

  // The next line requires SE handler
  // Basically it is trying to create zero in getNullValue
  // set SizeInBits as 64 for now

  // unsigned BitWidth = SE->getTypeSizeInBits(getType(Src));

  unsigned BitWidth = 64;

  APInt RunningGCD = APInt::getNullValue(BitWidth);

  // Examine Src coefficients.
  // Compute running GCD and record source constant.
  // Because we're looking for the constant at the end of the chain,
  // we can't quit the loop just because the GCD == 1.

  LLVM_DEBUG(dbgs() << "\nRunningGCD  =" << RunningGCD);
  const CanonExpr *CE = Src;
  const CanonExpr *CE2;

  int64_t K1 = 0;
  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {

    unsigned Index;
    int64_t Coeff1;
    CE->getIVCoeff(CurIVPair, &Index, &Coeff1);

    LLVM_DEBUG(dbgs() << "\nindex, coeff  =" << Index << "  " << Coeff1);

    K1 = CE->getIVConstCoeff(CurIVPair);
    if (K1 == 0) {
      continue;
    }
    // Okay to ignore blobcoeff
    // [3*n*i + 2*j] and [n*i' + 2*j' - 1] will also simplify to
    // [2*n*i + 2*j] vs  [2*j' - 1]

    // do i=1,n; do j=2,n; a(i+2*j) = a(i+2*j-1) with input dv (= *)
    // returns INDEP (Tested already)

    APInt ConstCoeff = llvm::APInt(64, K1, true);
    LLVM_DEBUG(dbgs() << "\nRunningGCD in  =" << RunningGCD);
    LLVM_DEBUG(dbgs() << "\n k1  =" << K1);

    RunningGCD = APIntOps::GreatestCommonDivisor(RunningGCD, ConstCoeff.abs());
    LLVM_DEBUG(dbgs() << "\nRunningGCD1  =" << RunningGCD);
  }

  const CanonExpr *SrcConst = getInvariant(Src);

  // Examine Dst coefficients.
  // Compute running GCD and record destination constant.
  // Because we're looking for the constant at the end of the chain,
  // we can't quit the loop just because the GCD == 1.

  CE = Dst;
  K1 = 0;
  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {
    if (!CE->getIVConstCoeff(CurIVPair)) {
      continue;
    }

    // okay to ignore blob coeff

    K1 = CE->getIVConstCoeff(CurIVPair);
    APInt ConstCoeff = llvm::APInt(64, K1, true);
    RunningGCD = APIntOps::GreatestCommonDivisor(RunningGCD, ConstCoeff.abs());
    LLVM_DEBUG(dbgs() << "\nRunningGCD2  =" << RunningGCD);
  }

  const CanonExpr *DstConst = getInvariant(Dst);

  APInt ExtraGCD = APInt::getNullValue(BitWidth);
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);
  if (!Delta) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "    Delta = "; Delta->dump());
  K1 = Delta->getConstant();
  APInt ConstDelta = llvm::APInt(64, K1, true);
  LLVM_DEBUG(dbgs() << "\n ConstDelta = " << ConstDelta << "\n");
  if (ConstDelta == 0) {
    return false;
  }

  for (auto Blob = Delta->blob_begin(), End = Delta->blob_end(); Blob != End;
       ++Blob) {
    APInt ConstCoeff = llvm::APInt(64, Delta->getBlobCoeff(Blob), true);
    ExtraGCD = APIntOps::GreatestCommonDivisor(ExtraGCD, ConstCoeff.abs());
  }
  RunningGCD = APIntOps::GreatestCommonDivisor(RunningGCD, ExtraGCD);
  LLVM_DEBUG(dbgs() << "    RunningGCD = " << RunningGCD << "\n");
  APInt Remainder = ConstDelta.srem(RunningGCD);

  // e.g.  A[2i + 4j +6m +8n +1],  A[16i +4j +2m+6]
  // will derive Independence

  if (Remainder != 0) {
    LLVM_DEBUG(dbgs() << "GCD success\n");
    ++GCDindependence;
    return true;
  }

  // Try to disprove equal directions.
  // For example, given a subscript pair [3*i + 2*j] and [i' + 2*j' - 1],
  // the code above can't disprove the dependence because the GCD = 1.
  // So we consider what happen if i = i' and what happens if j = j'.
  // If i = i', we can simplify the subscript to [2*i + 2*j] and [2*j' - 1],
  // which is infeasible, so we can disallow the = direction for the i level.
  // Setting j = j' doesn't help matters, so we end up with a direction vector
  // of [<>, *]
  //
  // Given A[5*i + 10*j*M + 9*M*N] and A[15*i + 20*j*M - 21*N*M + 5],
  // we need to remember that the constant part is 5 and the RunningGCD should
  // be initialized to ExtraGCD = 30.
  LLVM_DEBUG(dbgs() << "    ExtraGCD = " << ExtraGCD << '\n');

  bool Improved = false;
  CE = Src;

  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {

    if (!CE->getIVConstCoeff(CurIVPair)) {
      continue;
    }

    //  DV is not computed beyond common level
    unsigned Level = CE->getLevel(CurIVPair);
    if (Level > CommonLevels) {
      break;
    }

    RunningGCD = ExtraGCD;

    const CanonExpr *SrcCoeff =
        getConstantWithType(Src->getSrcType(), CE->getIVConstCoeff(CurIVPair));
    const CanonExpr *DstCoeff = getMinus(SrcCoeff, SrcCoeff); // start with  0

    const CanonExpr *Inner = Src;

    CE2 = Inner;

    for (auto CurIVPair2 = CE2->iv_begin(), E2 = CE2->iv_end();
         CurIVPair2 != E2; ++CurIVPair2) {
      if (RunningGCD == 1) {
        break;
      }
      if (!CE2->getIVConstCoeff(CurIVPair2)) {
        continue;
      }
      if (Level == CE2->getLevel(CurIVPair2)) {
        // SrcCoeff == Coeff
      } else {

        K1 = CE2->getIVConstCoeff(CurIVPair2);
        APInt ConstCoeff = llvm::APInt(64, K1, true);
        RunningGCD =
            APIntOps::GreatestCommonDivisor(RunningGCD, ConstCoeff.abs());
      }
    }

    Inner = Dst;
    CE2 = Inner;
    for (auto CurIVPair2 = CE2->iv_begin(), E2 = CE2->iv_end();
         CurIVPair2 != E2; ++CurIVPair2) {
      if (RunningGCD == 1) {
        break;
      }
      if (!CE2->getIVConstCoeff(CurIVPair2)) {
        continue;
      }

      if (Level == CE2->getLevel(CurIVPair2)) {
        DstCoeff = getConstantWithType(Dst->getSrcType(),
                                       CE2->getIVConstCoeff(CurIVPair2));
      } else {

        K1 = CE2->getIVConstCoeff(CurIVPair2);
        APInt ConstCoeff = llvm::APInt(64, K1, true);
        RunningGCD =
            APIntOps::GreatestCommonDivisor(RunningGCD, ConstCoeff.abs());
      }
    }

    if (!DstCoeff) {
      continue;
    }

    Delta = getMinus(SrcCoeff, DstCoeff);
    if (!Delta || !(Delta->isIntConstant(&K1))) {
      continue;
    }

    APInt ConstCoeff = llvm::APInt(64, K1, true);
    RunningGCD = APIntOps::GreatestCommonDivisor(RunningGCD, ConstCoeff.abs());
    LLVM_DEBUG(dbgs() << "\tRunningGCD = " << RunningGCD << "\n");
    if (RunningGCD != 0) {
      Remainder = ConstDelta.srem(RunningGCD);
      LLVM_DEBUG(dbgs() << "\tRemainder = " << Remainder << "\n");
      if (Remainder != 0) {
        LLVM_DEBUG(dbgs() << "\tLevel=" << Level << "\n");
        assert(((Level - 1) < CommonLevels) && "Invalid Level ");
        Result.setDirection(Level, Result.getDirection(Level) & ~DVKind::EQ);
        Improved = true;
      }
    }
  }

  if (Improved) {
    LLVM_DEBUG(dbgs() << "GCD success\n");
    ++GCDsuccesses;
  }

  LLVM_DEBUG(dbgs() << "all done\n");
  return false;
}

//===----------------------------------------------------------------------===//
// banerjeeMIVtest -
// Use Banerjee's Inequalities to test an MIV subscript pair.
// (Wolfe, in the race-car book, calls this the Extreme Value Test.)
// Generally follows the discussion in Section 2.5.2 of
//
//    Optimizing Supercompilers for Supercomputers
//    Michael Wolfe
//
// The inequalities given on page 25 are simplified in that loops are
// normalized so that the lower bound is always 0 and the stride is always 1.
// For example, Wolfe gives
//
//     LB^<_k = (A^-_k - B_k)^- (U_k - L_k - N_k) + (A_k - B_k)L_k - B_k N_k
//
// where A_k is the coefficient of the kth index in the source subscript,
// B_k is the coefficient of the kth index in the destination subscript,
// U_k is the upper bound of the kth index, L_k is the lower bound of the Kth
// index, and N_k is the stride of the kth index. Since all loops are normalized
// by the SCEV package, N_k = 1 and L_k = 0, allowing us to simplify the
// equation to
//
//     LB^<_k = (A^-_k - B_k)^- (U_k - 0 - 1) + (A_k - B_k)0 - B_k 1
//            = (A^-_k - B_k)^- (U_k - 1)  - B_k
//
// Similar simplifications are possible for the other equations.
//
// When we can't determine the number of iterations for a loop,
// we use NULL as an indicator for the worst case, infinity.
// When computing the upper bound, NULL denotes +inf;
// for the lower bound, NULL denotes -inf.
//
// Return true if dependence disproved.

bool DDTest::banerjeeMIVtest(const CanonExpr *Src, const CanonExpr *Dst,
                             const DirectionVector &InputDV,
                             const SmallBitVector &Loops, Dependences &Result,
                             const HLLoop *SrcParentLoop,
                             const HLLoop *DstParentLoop) {

  int64_t Coeff1, Coeff2;
  unsigned BlobIndex1, BlobIndex2;
  LLVM_DEBUG(dbgs() << "\nstarting Banerjee\n");
  ++BanerjeeApplications;
  LLVM_DEBUG(dbgs() << "\n   Src = "; Src->dump());
  const CanonExpr *A0;
  CoefficientInfo ACoeff[MaxPossibleLevels];
  bool IgnoreIVCoeff[MaxLoopNestLevel];

  // When test for (=) and coeffs are the same,
  // e.g. Input DV (= *), [2 * N * i1 + i2]  vs. [2 * N * i1 + i2 + 1]
  // It can be tested as  [i2] vs. [i2 + 1]
  // Denominator not equal 1 will not reach this test

  for (unsigned K = 0; K < MaxLoopNestLevel; ++K) {
    IgnoreIVCoeff[K] = false;
  }

  for (auto CurIVPair = Src->iv_begin(), E = Src->iv_end(); CurIVPair != E;
       ++CurIVPair) {
    unsigned IVLevel = Src->getLevel(CurIVPair);
    if (IVLevel <= InputDV.size() && InputDV[IVLevel - 1] == DVKind::EQ) {
      Src->getIVCoeff(CurIVPair, &BlobIndex1, &Coeff1);
      Dst->getIVCoeff(IVLevel, &BlobIndex2, &Coeff2);
      if (BlobIndex1 == BlobIndex2 && Coeff1 == Coeff2) {
        IgnoreIVCoeff[IVLevel - 1] = true;
      }
    }
  }

  if (!collectCoeffInfo(Src, true, A0, SrcParentLoop, DstParentLoop,
                        IgnoreIVCoeff, ACoeff)) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "\n   Dst = "; Dst->dump());
  const CanonExpr *B0;
  CoefficientInfo BCoeff[MaxPossibleLevels];
  if (!collectCoeffInfo(Dst, false, B0, SrcParentLoop, DstParentLoop,
                        IgnoreIVCoeff, BCoeff)) {
    return false;
  }

  BoundInfo Bound[MaxPossibleLevels];
  const CanonExpr *Delta = getMinus(B0, A0);
  if (!Delta) {
    return false;
  }
  LLVM_DEBUG(dbgs() << "\n    Delta = "; Delta->dump());

  // Compute bounds for all the * directions.
  LLVM_DEBUG(dbgs() << "\n\tBounds[*]\n");
  for (unsigned K = 1; K <= MaxLevels; ++K) {
    Bound[K].Iterations =
        ACoeff[K].Iterations ? ACoeff[K].Iterations : BCoeff[K].Iterations;
    Bound[K].Direction = DVKind::ALL;
    Bound[K].DirSet = DVKind::NONE;
    findBoundsALL(ACoeff, BCoeff, Bound, K);
#ifndef NDEBUG
    LLVM_DEBUG(dbgs() << "\n    " << K << '\t');
    const CanonExpr *BL = Bound[K].Lower[DVKind::ALL];
    const CanonExpr *BU = Bound[K].Upper[DVKind::ALL];
    if (BL) {
      LLVM_DEBUG(dbgs() << " "; BL->dump());
    } else {
      LLVM_DEBUG(dbgs() << "-inf\t");
    }
    if (BU) {
      LLVM_DEBUG(dbgs() << " "; BU->dump());
    } else {
      LLVM_DEBUG(dbgs() << "+inf\n");
    }
#endif
  }

  // Test the *, *, *, ... case.
  bool Disproved = false;
  if (testBounds(DVKind::ALL, 0, Bound, Delta, InputDV)) {
    // Explore the direction vector hierarchy.
    unsigned DepthExpanded = 0;
    unsigned NewDeps = exploreDirections(1, ACoeff, BCoeff, Bound, Loops,
                                         DepthExpanded, Delta, InputDV);
    if (NewDeps > 0) {
      bool Improved = false;
      for (unsigned K = 1; K <= CommonLevels; ++K) {
        if (Loops[K]) {
          DVKind Old = Result.DV[K - 1].Direction;
          Result.DV[K - 1].Direction = Old & Bound[K].DirSet;
          Improved |= Old != Result.DV[K - 1].Direction;
          if (!Result.DV[K - 1].Direction) {
            Improved = false;
            Disproved = true;
            break;
          }
        }
      }
      if (Improved)
        ++BanerjeeSuccesses;
    } else {
      ++BanerjeeIndependence;
      Disproved = true;
    }
  } else {
    ++BanerjeeIndependence;
    Disproved = true;
  }
  return Disproved;
}

// Hierarchically expands the direction vector
// search space, combining the directions of discovered dependences
// in the DirSet field of Bound. Returns the number of distinct
// dependences discovered. If the dependence is disproved,
// it will return 0.
unsigned DDTest::exploreDirections(unsigned Level, CoefficientInfo *A,
                                   CoefficientInfo *B, BoundInfo *Bound,
                                   const SmallBitVector &Loops,
                                   unsigned &DepthExpanded,
                                   const CanonExpr *Delta,
                                   const DirectionVector &InputDV) {
  if (Level > CommonLevels) {
    // record result
    LLVM_DEBUG(dbgs() << "\n\t[");
    for (unsigned K = 1; K <= CommonLevels; ++K) {
      if (Loops[K]) {
        Bound[K].DirSet |= Bound[K].Direction;
#ifndef NDEBUG
        switch (Bound[K].Direction) {
        case DVKind::LT:
          LLVM_DEBUG(dbgs() << " <");
          break;
        case DVKind::EQ:
          LLVM_DEBUG(dbgs() << " =");
          break;
        case DVKind::GT:
          LLVM_DEBUG(dbgs() << " >");
          break;
        case DVKind::ALL:
          LLVM_DEBUG(dbgs() << " *");
          break;
        default:
          llvm_unreachable("unexpected Bound[K].Direction");
        }
#endif
      }
    }
    LLVM_DEBUG(dbgs() << " ]");
    return 1;
  }
  if (Loops[Level]) {
    if (Level > DepthExpanded) {
      DepthExpanded = Level;
      // compute bounds for <, =, > at current level
      findBoundsLT(A, B, Bound, Level);
      findBoundsGT(A, B, Bound, Level);
      findBoundsEQ(A, B, Bound, Level);
#ifndef NDEBUG
      LLVM_DEBUG(dbgs() << "\n\tBound for level = " << Level << '\n');
      LLVM_DEBUG(dbgs() << "\n\t    <\t");
      const CanonExpr *CE;
      CE = Bound[Level].Lower[DVKind::LT];
      if (CE) {
        LLVM_DEBUG(dbgs(); CE->dump());
        LLVM_DEBUG(dbgs() << "\t");
      } else {
        LLVM_DEBUG(dbgs() << "-inf\t");
      }
      CE = Bound[Level].Upper[DVKind::LT];
      if (CE) {
        LLVM_DEBUG(dbgs(); CE->dump());
        LLVM_DEBUG(dbgs() << "\t");
      } else {
        LLVM_DEBUG(dbgs() << "+inf\n");
      }
      LLVM_DEBUG(dbgs() << "\n\t    =\t");
      CE = Bound[Level].Lower[DVKind::EQ];
      if (CE) {
        LLVM_DEBUG(dbgs(); CE->dump());
        LLVM_DEBUG(dbgs() << "\t");
      } else {
        LLVM_DEBUG(dbgs() << "-inf\t");
      }
      CE = Bound[Level].Upper[DVKind::EQ];
      if (CE) {
        LLVM_DEBUG(dbgs(); CE->dump());
        LLVM_DEBUG(dbgs() << "\t");
      } else {
        LLVM_DEBUG(dbgs() << "+inf\n");
      }
      LLVM_DEBUG(dbgs() << "\n\t    >\t");
      CE = Bound[Level].Lower[DVKind::GT];
      if (CE) {
        LLVM_DEBUG(dbgs(); CE->dump());
        LLVM_DEBUG(dbgs() << "\t");
      } else {
        LLVM_DEBUG(dbgs() << "-inf\t");
      }
      CE = Bound[Level].Upper[DVKind::GT];
      if (CE) {
        LLVM_DEBUG(dbgs(); CE->dump());
        LLVM_DEBUG(dbgs() << "\t");
      } else {
        LLVM_DEBUG(dbgs() << "+inf\n");
      }
#endif
    }

    unsigned NewDeps = 0;

    // test bounds for <, *, *, ...
    if (testBounds(DVKind::LT, Level, Bound, Delta, InputDV))
      NewDeps += exploreDirections(Level + 1, A, B, Bound, Loops, DepthExpanded,
                                   Delta, InputDV);

    // Test bounds for =, *, *, ...
    if (testBounds(DVKind::EQ, Level, Bound, Delta, InputDV))
      NewDeps += exploreDirections(Level + 1, A, B, Bound, Loops, DepthExpanded,
                                   Delta, InputDV);

    // test bounds for >, *, *, ...
    if (testBounds(DVKind::GT, Level, Bound, Delta, InputDV))
      NewDeps += exploreDirections(Level + 1, A, B, Bound, Loops, DepthExpanded,
                                   Delta, InputDV);

    Bound[Level].Direction = DVKind::ALL;
    return NewDeps;
  } else
    return exploreDirections(Level + 1, A, B, Bound, Loops, DepthExpanded,
                             Delta, InputDV);
}

// Returns true iff the current bounds are plausible.
// Return false implies no dependences for that level & DirKind

bool DDTest::testBounds(DVKind DirKind, unsigned Level, BoundInfo *Bound,
                        const CanonExpr *Delta,
                        const DirectionVector &InputDV) {

  Bound[Level].Direction = DirKind;
  LLVM_DEBUG(dbgs() << "\n\tTestBound: Level,DirKind\t" << Level << " "
                    << (unsigned)DirKind);

  // Level = 0 is called banerjeeMIVTest before expanding the call for
  // all combinations of DV
  // exploreDircetions will spawn off testing all combinations of DV
  // with Level > 0

  if (Level && (DirKind & InputDV[Level - 1]) == 0) {
    LLVM_DEBUG(dbgs() << "\n\tSkip testBound because no match with inputDV");
    return false;
  }

  if (const CanonExpr *LowerBound = getLowerBound(Bound))
    if (isKnownPredicate(CmpInst::ICMP_SGT, LowerBound, Delta))
      return false;
  if (const CanonExpr *UpperBound = getUpperBound(Bound))
    if (isKnownPredicate(CmpInst::ICMP_SGT, Delta, UpperBound))
      return false;
  return true;
}

// Computes the upper and lower bounds for level K
// using the * direction. Records them in Bound.
// Wolfe gives the equations
//
//    LB^*_k = (A^-_k - B^+_k)(U_k - L_k) + (A_k - B_k)L_k
//    UB^*_k = (A^+_k - B^-_k)(U_k - L_k) + (A_k - B_k)L_k
//
// Since we normalize loops, we can simplify these equations to
//
//    LB^*_k = (A^-_k - B^+_k)U_k
//    UB^*_k = (A^+_k - B^-_k)U_k
//
// We must be careful to handle the case where the upper bound is unknown.
// Note that the lower bound is always <= 0
// and the upper bound is always >= 0.
void DDTest::findBoundsALL(CoefficientInfo *A, CoefficientInfo *B,
                           BoundInfo *Bound, unsigned K) {
  Bound[K].Lower[DVKind::ALL] = nullptr; // Default value = -infinity.
  Bound[K].Upper[DVKind::ALL] = nullptr; // Default value = +infinity.
  if (Bound[K].Iterations) {
    Bound[K].Lower[DVKind::ALL] =
        getMulExpr(getMinus(A[K].NegPart, B[K].PosPart), Bound[K].Iterations);
    Bound[K].Upper[DVKind::ALL] =
        getMulExpr(getMinus(A[K].PosPart, B[K].NegPart), Bound[K].Iterations);
  } else {
    // If the difference is 0, we won't need to know the number of iterations.
    if (isKnownPredicate(CmpInst::ICMP_EQ, A[K].NegPart, B[K].PosPart)) {
      auto CE = A[K].Coeff;
      Bound[K].Lower[DVKind::ALL] = getConstantWithType(CE->getSrcType(), 0);
    }
    if (isKnownPredicate(CmpInst::ICMP_EQ, A[K].PosPart, B[K].NegPart)) {
      auto CE = A[K].Coeff;
      Bound[K].Upper[DVKind::ALL] = getConstantWithType(CE->getSrcType(), 0);
    }
  }
}

// Computes the upper and lower bounds for level K
// using the = direction. Records them in Bound.
// Wolfe gives the equations
//
//    LB^=_k = (A_k - B_k)^- (U_k - L_k) + (A_k - B_k)L_k
//    UB^=_k = (A_k - B_k)^+ (U_k - L_k) + (A_k - B_k)L_k
//
// Since we normalize loops, we can simplify these equations to
//
//    LB^=_k = (A_k - B_k)^- U_k
//    UB^=_k = (A_k - B_k)^+ U_k
//
// We must be careful to handle the case where the upper bound is unknown.
// Note that the lower bound is always <= 0
// and the upper bound is always >= 0.
void DDTest::findBoundsEQ(CoefficientInfo *A, CoefficientInfo *B,
                          BoundInfo *Bound, unsigned K) {
  Bound[K].Lower[DVKind::EQ] = nullptr; // Default value = -infinity.
  Bound[K].Upper[DVKind::EQ] = nullptr; // Default value = +infinity.
  if (Bound[K].Iterations) {
    const CanonExpr *Delta = getMinus(A[K].Coeff, B[K].Coeff);
    const CanonExpr *NegativePart = getNegativePart(Delta);
    Bound[K].Lower[DVKind::EQ] = getMulExpr(NegativePart, Bound[K].Iterations);
    const CanonExpr *PositivePart = getPositivePart(Delta);
    Bound[K].Upper[DVKind::EQ] = getMulExpr(PositivePart, Bound[K].Iterations);
  } else {
    // If the positive/negative part of the difference is 0,
    // we won't need to know the number of iterations.
    const CanonExpr *Delta = getMinus(A[K].Coeff, B[K].Coeff);
    const CanonExpr *NegativePart = getNegativePart(Delta);
    if (NegativePart && NegativePart->isZero()) {
      Bound[K].Lower[DVKind::EQ] = NegativePart; // Zero
    }
    const CanonExpr *PositivePart = getPositivePart(Delta);
    if (PositivePart && PositivePart->isZero()) {
      Bound[K].Upper[DVKind::EQ] = PositivePart; // Zero
    }
  }
}

// Computes the upper and lower bounds for level K
// using the < direction. Records them in Bound.
// Wolfe gives the equations
//
//    LB^<_k = (A^-_k - B_k)^- (U_k - L_k - N_k) + (A_k - B_k)L_k - B_k N_k
//    UB^<_k = (A^+_k - B_k)^+ (U_k - L_k - N_k) + (A_k - B_k)L_k - B_k N_k
//
// Since we normalize loops, we can simplify these equations to
//
//    LB^<_k = (A^-_k - B_k)^- (U_k - 1) - B_k
//    UB^<_k = (A^+_k - B_k)^+ (U_k - 1) - B_k
//
// We must be careful to handle the case where the upper bound is unknown.
void DDTest::findBoundsLT(CoefficientInfo *A, CoefficientInfo *B,
                          BoundInfo *Bound, unsigned K) {
  Bound[K].Lower[DVKind::LT] = nullptr; // Default value = -infinity.
  Bound[K].Upper[DVKind::LT] = nullptr; // Default value = +infinity.
  if (Bound[K].Iterations) {
    auto CE = Bound[K].Iterations;
    const CanonExpr *Iter_1 =
        getMinus(Bound[K].Iterations, getConstantWithType(CE->getSrcType(), 1));
    const CanonExpr *NegPart =
        getNegativePart(getMinus(A[K].NegPart, B[K].Coeff));
    Bound[K].Lower[DVKind::LT] =
        getMinus(getMulExpr(NegPart, Iter_1), B[K].Coeff);
    const CanonExpr *PosPart =
        getPositivePart(getMinus(A[K].PosPart, B[K].Coeff));
    Bound[K].Upper[DVKind::LT] =
        getMinus(getMulExpr(PosPart, Iter_1), B[K].Coeff);
  } else {
    // If the positive/negative part of the difference is 0,
    // we won't need to know the number of iterations.
    const CanonExpr *NegPart =
        getNegativePart(getMinus(A[K].NegPart, B[K].Coeff));
    if (NegPart && NegPart->isZero()) {
      Bound[K].Lower[DVKind::LT] = getNegative(B[K].Coeff);
    }
    const CanonExpr *PosPart =
        getPositivePart(getMinus(A[K].PosPart, B[K].Coeff));
    if (PosPart && PosPart->isZero()) {
      Bound[K].Upper[DVKind::LT] = getNegative(B[K].Coeff);
    }
  }
}

// Computes the upper and lower bounds for level K
// using the > direction. Records them in Bound.
// Wolfe gives the equations
//
//    LB^>_k = (A_k - B^+_k)^- (U_k - L_k - N_k) + (A_k - B_k)L_k + A_k N_k
//    UB^>_k = (A_k - B^-_k)^+ (U_k - L_k - N_k) + (A_k - B_k)L_k + A_k N_k
//
// Since we normalize loops, we can simplify these equations to
//
//    LB^>_k = (A_k - B^+_k)^- (U_k - 1) + A_k
//    UB^>_k = (A_k - B^-_k)^+ (U_k - 1) + A_k
//
// We must be careful to handle the case where the upper bound is unknown.
void DDTest::findBoundsGT(CoefficientInfo *A, CoefficientInfo *B,
                          BoundInfo *Bound, unsigned K) {
  Bound[K].Lower[DVKind::GT] = nullptr; // Default value = -infinity.
  Bound[K].Upper[DVKind::GT] = nullptr; // Default value = +infinity.
  if (Bound[K].Iterations) {
    auto CE = Bound[K].Iterations;
    const CanonExpr *Iter_1 =
        getMinus(Bound[K].Iterations, getConstantWithType(CE->getSrcType(), 1));
    const CanonExpr *NegPart =
        getNegativePart(getMinus(A[K].Coeff, B[K].PosPart));
    Bound[K].Lower[DVKind::GT] =
        getAdd(getMulExpr(NegPart, Iter_1), A[K].Coeff);
    const CanonExpr *PosPart =
        getPositivePart(getMinus(A[K].Coeff, B[K].NegPart));
    Bound[K].Upper[DVKind::GT] =
        getAdd(getMulExpr(PosPart, Iter_1), A[K].Coeff);
  } else {
    // If the positive/negative part of the difference is 0,
    // we won't need to know the number of iterations.
    const CanonExpr *NegPart =
        getNegativePart(getMinus(A[K].Coeff, B[K].PosPart));
    if (NegPart && NegPart->isZero()) {
      Bound[K].Lower[DVKind::GT] = A[K].Coeff;
    }
    const CanonExpr *PosPart =
        getPositivePart(getMinus(A[K].Coeff, B[K].NegPart));
    if (PosPart && PosPart->isZero()) {
      Bound[K].Upper[DVKind::GT] = A[K].Coeff;
    }
  }
}

// X^+ = max(X, 0)
const CanonExpr *DDTest::getPositivePart(const CanonExpr *X) {
  if (!X) {
    return nullptr;
  }
  return getSMaxExpr(X, getConstantWithType(X->getSrcType(), 0));
}

// X^- = min(X, 0)
const CanonExpr *DDTest::getNegativePart(const CanonExpr *X) {
  if (!X) {
    return nullptr;
  }
  return getSMinExpr(X, getConstantWithType(X->getSrcType(), 0));
}

// Walks through the subscript,
// collecting each coefficient, the associated loop bounds,
// and recording its positive and negative parts for later use.
bool DDTest::collectCoeffInfo(const CanonExpr *Subscript, bool SrcFlag,
                              const CanonExpr *&Constant,
                              const HLLoop *SrcParentLoop,
                              const HLLoop *DstParentLoop,
                              const bool IgnoreIVCoeff[],
                              CoefficientInfo CI[]) {

  const CanonExpr *Zero = getConstantWithType(Subscript->getSrcType(), 0);
  for (unsigned K = 1; K <= MaxLevels; ++K) {
    CI[K].Coeff = Zero;
    CI[K].PosPart = Zero;
    CI[K].NegPart = Zero;
    CI[K].Iterations = nullptr;
  }

  const CanonExpr *CE = Subscript;
  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {

    const HLLoop *L;
    unsigned K;

    if (!CE->getIVConstCoeff(CurIVPair)) {
      continue;
    }
    unsigned IVLevel = CE->getLevel(CurIVPair);
    if (IgnoreIVCoeff[IVLevel - 1]) {
      continue;
    }

    if (CE->getIVBlobCoeff(CurIVPair)) {
      return false;
    }
    if (SrcFlag) {
      L = SrcParentLoop->getParentLoopAtLevel(IVLevel);
      K = mapSrcLoop(L);
    } else {
      L = DstParentLoop->getParentLoopAtLevel(IVLevel);
      K = mapDstLoop(L);
    }

    const CanonExpr *CE2 = CI[K].Coeff = getConstantWithType(
        Subscript->getSrcType(), CE->getIVConstCoeff(CurIVPair));

    CI[K].PosPart = getPositivePart(CE2);
    CI[K].NegPart = getNegativePart(CE2);
    // unused type argument
    CI[K].Iterations = collectUpperBound(L, Subscript->getSrcType());
  }

  Constant = getInvariant(CE);

#ifndef NDEBUG
  LLVM_DEBUG(dbgs() << "\tCoefficient Info\n");
  for (unsigned K = 1; K <= MaxLevels; ++K) {
    LLVM_DEBUG(dbgs() << "\n       " << K << "\t"; (CI[K].Coeff)->dump());
    LLVM_DEBUG(dbgs() << "\tPos Part = "; (CI[K].PosPart)->dump());
    LLVM_DEBUG(dbgs() << "\tNeg Part = "; (CI[K].NegPart)->dump());
    LLVM_DEBUG(dbgs() << "\tUpper Bound = ");
    if (CI[K].Iterations)
      LLVM_DEBUG(dbgs(); (CI[K].Iterations)->dump());
    else
      LLVM_DEBUG(dbgs() << "+inf");
    LLVM_DEBUG(dbgs() << '\n');
  }
  LLVM_DEBUG(dbgs() << "\t   Constant = "; Constant->dump());
  LLVM_DEBUG(dbgs() << "\n");

#endif
  return true;
}

// Looks through all the bounds info and
// computes the lower bound given the current direction settings
// at each level. If the lower bound for any level is -inf,
// the result is -inf.
const CanonExpr *DDTest::getLowerBound(BoundInfo *Bound) {
  const CanonExpr *Sum = Bound[1].Lower[Bound[1].Direction];
  for (unsigned K = 2; Sum && K <= MaxLevels; ++K) {
    if (Bound[K].Lower[Bound[K].Direction])
      Sum = getAdd(Sum, Bound[K].Lower[Bound[K].Direction]);
    else
      Sum = nullptr;
  }
  return Sum;
}

// Looks through all the bounds info and
// computes the upper bound given the current direction settings
// at each level. If the upper bound at any level is +inf,
// the result is +inf.
const CanonExpr *DDTest::getUpperBound(BoundInfo *Bound) {
  const CanonExpr *Sum = Bound[1].Upper[Bound[1].Direction];
  for (unsigned K = 2; Sum && K <= MaxLevels; ++K) {
    if (Bound[K].Upper[Bound[K].Direction]) {
      Sum = getAdd(Sum, Bound[K].Upper[Bound[K].Direction]);
    } else {
      Sum = nullptr;
    }
  }
  return Sum;
}

bool DDTest::delinearizeToMultiDim(
    const RegDDRef *DDRef, const CanonExpr *CE,
    SmallVectorImpl<const CanonExpr *> &Subscripts,
    SmallVectorImpl<unsigned> &IVLevels, bool RelaxChecking) {

  // Loops can come in different permutations:
  // A[2 * i1 + 3 * n1 *i2 + 4 * n1 * n2 * i3 ] or
  // A[2 * i2 + 3 * n1 *i3 + 4 * n1 * n2 * i1 ]

  // Steps:
  // -- Save coeffs in a vector, 1, n1, n2 * n2 ..
  // -- While vector not empty    (Illustrated with data from 3rd iteration)
  //      Look for smallest coeff,          ! 4 * n1 * n2
  //            that divides last stride    ! n1
  //      If not dividing, stop.
  //      Verify that the quotient,         ! 4 * n2
  //        is > UB of loop corrs. to last-loop  ! UB of i2 loop
  //      Construct subsubcript by removing stride !  4
  //      Save last stride; save last-loop ! n1 * n2 ; Loop i3
  //      Delete from vector
  //
  // Implemenation: we can asssume the constant part of the coeffs are
  // not part of the stride, otherwise it will take more time.

  SmallVector<const CanonExpr *, 8> Coeffs;

  if (CE->numIVs() == 2) {
    return delinearizeTo2Dim(DDRef, CE, Subscripts, IVLevels, RelaxChecking);
  }
  // TODO:  Most of the kernels that require delinearization
  // have 2 IVs. Not a priority now.
  return false;
}

bool DDTest::delinearizeTo2Dim(const RegDDRef *DDRef, const CanonExpr *CE,
                               SmallVectorImpl<const CanonExpr *> &Subscripts,
                               SmallVectorImpl<unsigned> &IVLevels,
                               bool RelaxChecking) {

  // A common occurence. For fast compile time, special case for 2 IVs

  unsigned LoopLevelForUnitStride = 0;

  unsigned int IVProcessed = 0;
  unsigned int IVNum = 0;
  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {

    int64_t ConstCoeff = CE->getIVConstCoeff(CurIVPair);
    unsigned BlobIdx = CE->getIVBlobCoeff(CurIVPair);
    unsigned IVLevel = CE->getLevel(CurIVPair);

    LLVM_DEBUG(dbgs() << "\n\tConst coeff, Blobidx, IVLevel " << ConstCoeff
                      << " " << BlobIdx << " " << IVLevel);
    if (ConstCoeff == 0) {
      continue;
    }
    IVNum++;
    if (BlobIdx == InvalidBlobIndex) {
      // Extract unit stride subscript
      const CanonExpr *Src = getInvariant(CE);
      CanonExpr *TmpCE = const_cast<CanonExpr *>(Src);
      TmpCE->setIVCoeff(IVLevel, BlobIdx, ConstCoeff);
      Subscripts.push_back(TmpCE);
      LoopLevelForUnitStride = IVLevel;
      IVLevels.push_back(IVLevel);
      IVProcessed = IVNum;
      break;
    }
  }

  if (!LoopLevelForUnitStride) {
    // No constant coeffs. Cannot proceed further
    return false;
  }

  IVNum = 0;

  const HLLoop *Lp = DDRef->getParentLoop();
  const HLLoop *ParentLoop = Lp->getParentLoopAtLevel(LoopLevelForUnitStride);

  if (!RelaxChecking && ParentLoop->isUnknown()) {
    return false;
  }

  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {

    int64_t ConstCoeff = CE->getIVConstCoeff(CurIVPair);
    if (ConstCoeff == 0) {
      continue;
    }

    // Note that IVNum++ needs to be in the same place as in the util getCoeff
    // in this file

    unsigned BlobIdx = CE->getIVBlobCoeff(CurIVPair);
    IVNum++;
    if (IVNum == IVProcessed || BlobIdx == InvalidBlobIndex) {
      continue;
    }

    const CanonExpr *TmpCE = getCoeff(CE, IVNum, false); // 4  * n

    unsigned IVLevel = CE->getLevel(CurIVPair);

    // Verify that Coeff is > UB of LoopForUnitStride

    if (RelaxChecking || isKnownPredicate(CmpInst::ICMP_SGT, TmpCE,
                                          ParentLoop->getUpperCanonExpr())) {
      // Construct the subscript e.g. 4  from 4 * n * i by removing the
      // symbolic stride
      CanonExpr *Tmp = const_cast<CanonExpr *>(TmpCE);
      Tmp->clearBlobs();
      Tmp->setIVCoeff(IVLevel, InvalidBlobIndex, ConstCoeff); // becomes 4
      Subscripts.push_back(Tmp);
      IVLevels.push_back(IVLevel);
    }
  }

  return true;
}

bool DDTest::isDelinearizeCandidate(const RegDDRef *Ref) {

  //  Select CE of this form:    .. + N * i2 + i3 + ..
  //  Can extend it later for more cases

  for (auto CE = Ref->canon_begin(), E = Ref->canon_end(); CE != E; ++CE) {
    unsigned NumBlobCoeffs = (*CE)->numIVBlobCoeffs();
    if (NumBlobCoeffs && NumBlobCoeffs < (*CE)->numIVs()) {
      return true;
    }
  }
  return false;
}

bool DDTest::tryDelinearize(const RegDDRef *SrcDDRef, const RegDDRef *DstDDRef,
                            const DirectionVector &InputDV,
                            SmallVectorImpl<Subscript> &Pair,
                            bool ForDDGBuild) {

  // Do not try to delinearise non-linear dd refs.
  if (SrcDDRef->isNonLinear() || DstDDRef->isNonLinear()) {
    return false;
  }

  // Without loss of generailty, a 3-dim array is used for illustration
  //    A[n1][n2][n3]
  //    do i1=0, n1-1
  //      do i2=0, n2-1
  //        do i3=0, n3-1
  //   a.
  //   It can be linearized with a 1-dim subcript
  //     A[s1 *  i3 + s2 * i2 + s3 * i1]
  //     where s1 = 1;  s2 = s1 * (n3-1);  s3 = s2 * (n2-1)
  //   b.
  //   A 1-dim linearized subscript
  //    A[i3 + nx * i2 + nx * ny * i1]
  //    do i1=0, n1
  //      do i2=0, n2
  //        do i3=0, n3
  //   can be delinearized to multi-dim if nx > n3, ny > n3 * n2
  //   A[i1][i2][i3]
  //
  // Enable for 1 level also
  // e.g. do i1=0, n-1
  //         do i2=0, n-1
  //           A[n*i1 + i2]
  //           do i3
  //                A[n*i1 + i2]

  if (CommonLevels < 1) {
    return false;
  }

  const CanonExpr *SrcCE = Pair[0].Src;
  const CanonExpr *DstCE = Pair[0].Dst;
  bool HasGlobalBaseValue = false;
  Type *AuxTy = nullptr;
  if (Pair.size() == 2) {
    // If base value is global we have a memref that looks like
    //       (@s)[0][i3 + nx * i2 + nx * ny * i1].
    // Skip [0] since it doesn't influence the delinearization.
    // Note: out of bound array access could result in non-zero first index.
    // Be conservative here and only work with zero cases.
    if (SrcDDRef->accessesGlobalVar() &&
        DstDDRef->accessesGlobalVar() &&
        Pair[1].Src->isZero() && Pair[1].Dst->isZero()) {
      HasGlobalBaseValue = true;
      AuxTy = Pair[1].Src->getSrcType();
    } else {
      return false;
    }
  } else if (Pair.size() != 1) {
    return false;
  }

  // For DDGBuild,  check if can be delinearzed
  // Otherwise, already checked before calling RefineDV
  if (ForDDGBuild && (!isDelinearizeCandidate(SrcDDRef) ||
                      !isDelinearizeCandidate(DstDDRef))) {
    return false;
  }

  SmallVector<const CanonExpr *, 3> SrcSubscripts, DstSubscripts;
  SmallVector<unsigned, 3> SrcIVLevels, DstIVLevels;

  unsigned InnermostLoopLevel =
      DeepestLoop ? DeepestLoop->getNestingLevel() : CommonLevels;
  int64_t CVal;
  bool RelaxChecking = false;

  // When testing for (= *) and CEs are in the form  A[i1*N + i2], A[i1*N +
  // i2+3], map them as A[i1][i2], A[i1][i2+3] w/o additional check on
  // properties of N
  if (InputDV.isTestingForInnermostLoop(InnermostLoopLevel) &&
      HNU.getCanonExprUtils().getConstDistance(SrcCE, DstCE, &CVal)) {
    RelaxChecking = true;
  }

  if (!delinearizeToMultiDim(SrcDDRef, SrcCE, SrcSubscripts, SrcIVLevels,
                             RelaxChecking) ||
      SrcSubscripts.size() < 2) {
    return false;
  }

  if (!delinearizeToMultiDim(DstDDRef, DstCE, DstSubscripts, DstIVLevels,
                             RelaxChecking) ||
      DstSubscripts.size() < 2) {
    return false;
  }

  if (SrcSubscripts.size() != DstSubscripts.size()) {
    return false;
  }

  for (unsigned I = 0; I < SrcIVLevels.size(); ++I) {
    if (SrcIVLevels[I] != DstIVLevels[I]) {
      return false;
    }
  }

  unsigned Size = SrcSubscripts.size();
  unsigned FullSize = HasGlobalBaseValue ? (Size + 1) : Size;
  Pair.resize(FullSize);
  for (unsigned I = 0; I < Size; ++I) {
    Pair[I].Src = SrcSubscripts[I];
    Pair[I].Dst = DstSubscripts[I];
  }

  if (HasGlobalBaseValue && AuxTy) {
    Pair[Size].Src = getConstantWithType(AuxTy, 0);
    Pair[Size].Dst = getConstantWithType(AuxTy, 0);
  }

  return true;
}

//===----------------------------------------------------------------------===//
// Constraint manipulation for Delta test.
#if 0
// Given a linear SCEV,
// return the coefficient (the step)
// corresponding to the specified loop.
// If there isn't one, return 0.
// For example, given a*i + b*j + c*k, zeroing the coefficient
// corresponding to the j loop would yield b.
const SCEV *DependenceAnalysis::findCoefficient(const SCEV *Expr,
                                                const Loop *TargetLoop)  const {
  const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(Expr);
  if (!AddRec)
    return SE->getConstant(Expr->getType(), 0);
  if (AddRec->getLoop() == TargetLoop)
    return AddRec->getStepRecurrence(*SE);
  return findCoefficient(AddRec->getStart(), TargetLoop);
}


// Given a linear SCEV,
// return the SCEV given by zeroing out the coefficient
// corresponding to the specified loop.
// For example, given a*i + b*j + c*k, zeroing the coefficient
// corresponding to the j loop would yield a*i + c*k.
const SCEV *DependenceAnalysis::zeroCoefficient(const SCEV *Expr,
                                                const Loop *TargetLoop)  const {
  const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(Expr);
  if (!AddRec)
    return Expr; // ignore
  if (AddRec->getLoop() == TargetLoop)
    return AddRec->getStart();
  return SE->getAddRecExpr(zeroCoefficient(AddRec->getStart(), TargetLoop),
                           AddRec->getStepRecurrence(*SE),
                           AddRec->getLoop(),
                           AddRec->getNoWrapFlags());
}


// Given a linear SCEV Expr,
// return the SCEV given by adding some Value to the
// coefficient corresponding to the specified TargetLoop.
// For example, given a*i + b*j + c*k, adding 1 to the coefficient
// corresponding to the j loop would yield a*i + (b+1)*j + c*k.
const SCEV *DependenceAnalysis::addToCoefficient(const SCEV *Expr,
                                                 const Loop *TargetLoop,
                                                 const SCEV *Value)  const {
  const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(Expr);
  if (!AddRec) // create a new addRec
    return SE->getAddRecExpr(Expr,
                             Value,
                             TargetLoop,
                             SCEV::FlagAnyWrap); // Worst case, with no info.
  if (AddRec->getLoop() == TargetLoop) {
    const SCEV *Sum = SE->getAddExpr(AddRec->getStepRecurrence(*SE), Value);
    if (Sum->isZero())
      return AddRec->getStart();
    return SE->getAddRecExpr(AddRec->getStart(),
                             Sum,
                             AddRec->getLoop(),
                             AddRec->getNoWrapFlags());
  }
  if (SE->isLoopInvariant(AddRec, TargetLoop))
    return SE->getAddRecExpr(AddRec, Value, TargetLoop, SCEV::FlagAnyWrap);
  return SE->getAddRecExpr(
		addToCoefficient(AddRec->getStart(), TargetLoop, Value),
		AddRec->getStepRecurrence(SE), AddRec->getLoop(),
		AddRec->getNoWrapFlags());
}

#endif
// Review the constraints, looking for opportunities
// to simplify a subscript pair (Src and Dst).
// Return true if some simplification occurs.
// If the simplification isn't exact (that is, if it is conservative
// in terms of dependence), set consistent to false.
// Corresponds to Figure 5 from the paper
//
//            Practical Dependence Testing
//            Goff, Kennedy, Tseng
//            PLDI 1991
bool DDTest::propagate(const CanonExpr *&Src, const CanonExpr *&Dst,
                       SmallBitVector &Loops,
                       SmallVectorImpl<Constraint> &Constraints,
                       bool &Consistent) {
  bool Result = false;
  for (int LI = Loops.find_first(); LI >= 0; LI = Loops.find_next(LI)) {
    LLVM_DEBUG(dbgs() << "\t    Constraint[" << LI << "] is");
    LLVM_DEBUG(Constraints[LI].dump(dbgs()));
    if (Constraints[LI].isDistance())
      Result |= propagateDistance(Src, Dst, Constraints[LI], Consistent);
    else if (Constraints[LI].isLine())
      Result |= propagateLine(Src, Dst, Constraints[LI], Consistent);
    else if (Constraints[LI].isPoint())
      Result |= propagatePoint(Src, Dst, Constraints[LI]);
  }
  return Result;
}

// Attempt to propagate a distance
// constraint into a subscript pair (Src and Dst).
// Return true if some simplification occurs.
// If the simplification isn't exact (that is, if it is conservative
// in terms of dependence), set consistent to false.
bool DDTest::propagateDistance(const CanonExpr *&Src, const CanonExpr *&Dst,
                               Constraint &CurConstraint, bool &Consistent) {
  const HLLoop *CurLoop = CurConstraint.getAssociatedLoop();
  if (!CurLoop)
    return false;
  unsigned CurLoopLevel = CurLoop->getNestingLevel();
  CanonExpr *D = CurConstraint.getD()->clone();
  LLVM_DEBUG(dbgs() << "\n\tSrc is "; Src->dump());
  CanonExpr *NewSrc = Src->clone();
  CanonExpr *NewDst = Dst->clone();
  push(NewSrc);
  push(NewDst);
  push(D);

  // e = e - a_k * D
  unsigned Index;
  int64_t Coeff;
  NewSrc->getIVCoeff(CurLoopLevel, &Index, &Coeff);
  if (Coeff == 0)
    return false;
  if (!D->multiplyByConstant(0 - Coeff))
    return false;
  if (Index != InvalidBlobIndex && !D->multiplyByBlob(Index))
    return false;
  if (D->numBlobs() > 1)
    return false;
  if (D->hasBlob()) {
    int64_t MulCoeff = D->getSingleBlobCoeff();
    unsigned MulIndex = D->getSingleBlobIndex();
    NewSrc->addBlob(MulIndex, MulCoeff);
  } else {
    NewSrc->addConstant(D->getConstant(), false);
  }

  // a_k = 0
  NewSrc->removeIV(CurLoopLevel);

  LLVM_DEBUG(dbgs() << "\n\tDst is "; Dst->dump());
  // a_k' = a_k' - a_k
  NewDst->addIV(CurLoopLevel, Index, 0 - Coeff);

  NewDst->getIVCoeff(CurLoopLevel, &Index, &Coeff);
  if (Coeff == 0)
    Consistent = false;
  LLVM_DEBUG(dbgs() << "\t\tnew Src is "; NewSrc->dump());
  LLVM_DEBUG(dbgs() << "\t\tnew Dst is "; NewDst->dump());
  Src = NewSrc;
  Dst = NewDst;
  return true;
}

// Attempt to propagate a line
// constraint into a subscript pair (Src and Dst).
// Return true if some simplification occurs.
// If the simplification isn't exact (that is, if it is conservative
// in terms f dependence), set consistent to false.
bool DDTest::propagateLine(const CanonExpr *&OrigSrc, const CanonExpr *&OrigDst,
                           Constraint &CurConstraint, bool &Consistent) {
  const HLLoop *CurLoop = CurConstraint.getAssociatedLoop();
  if (!CurLoop)
    return false;
  const CanonExpr *Src = OrigSrc;
  const CanonExpr *Dst = OrigDst;
  unsigned CurLoopLevel = CurLoop->getNestingLevel();
  const CanonExpr *A = CurConstraint.getA();
  const CanonExpr *B = CurConstraint.getB();
  const CanonExpr *C = CurConstraint.getC();
  CanonExpr *NewSrc = Src->clone();
  CanonExpr *NewDst = Dst->clone();
  push(NewSrc);
  push(NewDst);
  LLVM_DEBUG(dbgs() << "\n\tA = "; A->dump(); dbgs() << "\t\tB = "; B->dump();
             dbgs() << "\t\tC = "; C->dump(); dbgs() << "\n\tSrc = ";
             Src->dump(); dbgs() << "\n\tDst = "; Dst->dump());
  if (A->isZero()) {
    int64_t Bconst, Cconst;
    if (!B->isIntConstant(&Bconst) || !C->isIntConstant(&Cconst))
      return false;
    // e = e - a_k' * C / B
    int64_t CdivB = Cconst / Bconst;
    assert(Cconst % Bconst == 0 && "C should be evenly divisible by B");
    int64_t Coeff;
    unsigned Index;
    NewDst->getIVCoeff(CurLoopLevel, &Index, &Coeff);
    if (Index != InvalidBlobIndex)
      NewSrc->addBlob(Index, (0 - Coeff) * CdivB);
    else
      NewSrc->addConstant((0 - Coeff) * CdivB, false);

    // a_k' = 0
    NewDst->removeIV(CurLoopLevel);

    NewSrc->getIVCoeff(CurLoopLevel, &Index, &Coeff);
    if (Coeff != 0)
      Consistent = false;

  } else if (B->isZero()) {
    // e = e + a_k * C / A
    int64_t Aconst, Cconst;
    if (!A->isIntConstant(&Aconst) || !C->isIntConstant(&Cconst))
      return false;
    int64_t CdivA = Cconst / Aconst;
    assert(Cconst % Aconst == 0 && "C should be evenly divisible by A");
    int64_t Coeff;
    unsigned Index;
    NewSrc->getIVCoeff(CurLoopLevel, &Index, &Coeff);
    if (Index != InvalidBlobIndex)
      NewSrc->addBlob(Index, Coeff * CdivA);
    else
      NewSrc->addConstant(Coeff * CdivA, false);

    // a_k = 0
    NewSrc->removeIV(CurLoopLevel);

    NewDst->getIVCoeff(CurLoopLevel, &Index, &Coeff);
    if (Coeff != 0)
      Consistent = false;

  } else if (isKnownPredicate(CmpInst::ICMP_EQ, A, B)) {
    int64_t Aconst, Cconst;
    if (!A->isIntConstant(&Aconst) || !C->isIntConstant(&Cconst))
      return false;

    // e = e + a_k * C / A
    int64_t CdivA = Cconst / Aconst;
    assert((Cconst % Aconst) == 0 && "C should be evenly divisible by A");
    int64_t Coeff;
    unsigned Index;
    NewSrc->getIVCoeff(CurLoopLevel, &Index, &Coeff);
    if (Index != InvalidBlobIndex)
      NewSrc->addBlob(Index, Coeff * CdivA);
    else
      NewSrc->addConstant(Coeff * CdivA, false);

    // a_k = 0
    NewSrc->removeIV(CurLoopLevel);

    // a_k' = a_k' + a_k
    NewDst->addIV(CurLoopLevel, Index, Coeff);

    NewDst->getIVCoeff(CurLoopLevel, &Index, &Coeff);
    if (Coeff != 0)
      Consistent = false;

  } else {
    // paper is incorrect here, or perhaps just misleading

    // scr = src * A; dst = dst * A;
    unsigned Index;
    int64_t Coeff;
    Src->getIVCoeff(CurLoopLevel, &Index, &Coeff);
    Src = getMulExpr(Src, A, true);
    Dst = getMulExpr(Dst, A, true);
    if (!Src || !Dst)
      return false;

    // e = e + a_k * C
    CanonExpr *TempC = C->clone();
    push(TempC);
    if (Index != InvalidBlobIndex && !TempC->multiplyByBlob(Index))
      return false;
    if (!TempC->multiplyByConstant(Coeff))
      return false;
    Src = getAdd(Src, TempC);
    if (!Src)
      return false;

    // a_k = 0
    NewSrc = Src->clone();
    push(NewSrc);
    NewSrc->removeIV(CurLoopLevel);

    // a_k' = a_k' + a_k * B
    CanonExpr *TempB = B->clone();
    push(TempB);
    if (Index != InvalidBlobIndex && !TempB->multiplyByBlob(Index))
      return false;
    if (!TempB->multiplyByConstant(Coeff))
      return false;
    if (TempB->numBlobs() != 1)
      return false;
    Index = TempB->getSingleBlobIndex();
    Coeff = TempB->getSingleBlobCoeff();
    NewDst = Dst->clone();
    push(NewDst);
    NewDst->addIV(CurLoopLevel, Index, Coeff);

    NewDst->getIVCoeff(CurLoopLevel, &Index, &Coeff);
    if (Coeff != 0)
      Consistent = false;
  }

  OrigSrc = NewSrc;
  OrigDst = NewDst;
  LLVM_DEBUG(dbgs() << "\n\tnew Src = "; Src->dump();
             dbgs() << "\n\tnew Dst = "; Dst->dump(););
  return true;
}

// Attempt to propagate a point
// constraint into a subscript pair (Src and Dst).
// Return true if some simplification occurs.
bool DDTest::propagatePoint(const CanonExpr *&OrigSrc,
                            const CanonExpr *&OrigDst,
                            Constraint &CurConstraint) {
  const HLLoop *CurLoop = CurConstraint.getAssociatedLoop();
  if (!CurLoop)
    return false;
  const CanonExpr *Src = OrigSrc;
  const CanonExpr *Dst = OrigDst;
  unsigned CurLoopLevel = CurLoop->getNestingLevel();
  CanonExpr *NewDst = Dst->clone();
  push(NewDst);

  // e = e + a_k * X - a_k' * Y
  unsigned Index;
  int64_t Coeff;
  LLVM_DEBUG(dbgs() << "\n\tSrc is "; Src->dump(););
  Src->getIVCoeff(CurLoopLevel, &Index, &Coeff);
  CanonExpr *Inv = CurConstraint.getX()->clone();
  push(Inv);
  if (Index != InvalidBlobIndex)
    if (!Inv->multiplyByBlob(Index))
      return false;
  if (!Inv->multiplyByConstant(Coeff))
    return false;
  Src = getAdd(Src, Inv);
  if (!Src)
    return false;
  Dst->getIVCoeff(CurLoopLevel, &Index, &Coeff);
  Inv = CurConstraint.getY()->clone();
  push(Inv);
  if (Index != InvalidBlobIndex && !Inv->multiplyByBlob(Index))
    return false;
  if (!Inv->multiplyByConstant(Coeff))
    return false;
  Src = getAdd(Src, Inv);
  if (!Src)
    return false;

  // a_k = 0
  CanonExpr *NewSrc = Src->clone();
  push(NewSrc);
  NewSrc->removeIV(CurLoopLevel);
  LLVM_DEBUG(dbgs() << "\t\tnew Src is "; NewSrc->dump();
             dbgs() << "\n\tDst is "; Dst->dump(););
  // a_k' = 0
  NewDst->removeIV(CurLoopLevel);
  LLVM_DEBUG(dbgs() << "\t\tnew Dst is "; NewDst->dump(););

  OrigSrc = NewSrc;
  OrigDst = NewDst;
  return true;
}

// Update direction vector entry based on the current constraint.
void DDTest::updateDirection(Dependences::DVEntry &Level,
                             const Constraint &CurConstraint) {
  LLVM_DEBUG(dbgs() << "\n\tUpdate direction, constraint =");
  LLVM_DEBUG(CurConstraint.dump(dbgs()));
  if (CurConstraint.isAny())
    ; // use defaults
  else if (CurConstraint.isDistance()) {
    // this one is consistent, the others aren't
    Level.Scalar = false;
    Level.Distance = CurConstraint.getD();
    auto NewDirection = DVKind::NONE;
    // if may be zero
    if (!HLNodeUtils::isKnownNonZero(Level.Distance, DeepestLoop))
      NewDirection = DVKind::EQ;
    // if may be positive
    if (!HLNodeUtils::isKnownNonPositive(Level.Distance, DeepestLoop))
      NewDirection |= DVKind::LT;
    // if may be negative
    if (!HLNodeUtils::isKnownNonNegative(Level.Distance, DeepestLoop))
      NewDirection |= DVKind::GT;
    Level.Direction &= NewDirection;
  } else if (CurConstraint.isLine()) {
    Level.Scalar = false;
    Level.Distance = nullptr;
    // direction should be accurate
  } else if (CurConstraint.isPoint()) {
    Level.Scalar = false;
    Level.Distance = nullptr;
    auto NewDirection = DVKind::NONE;
    if (!isKnownPredicate(CmpInst::ICMP_NE, CurConstraint.getY(),
                          CurConstraint.getX()))
      // if X may be = Y
      NewDirection |= DVKind::EQ;
    if (!isKnownPredicate(CmpInst::ICMP_SLE, CurConstraint.getY(),
                          CurConstraint.getX()))
      // if Y may be > X
      NewDirection |= DVKind::LT;
    if (!isKnownPredicate(CmpInst::ICMP_SGE, CurConstraint.getY(),
                          CurConstraint.getX()))
      // if Y may be < X
      NewDirection |= DVKind::GT;
    Level.Direction &= NewDirection;
  } else
    llvm_unreachable("constraint has unexpected kind");
}

#if 0
/// Check if we can delinearize the subscripts. If the SCEVs representing the
/// source and destination array references are recurrences on a nested loop,
/// this function flattens the nested recurrences into separate recurrences
/// for each loop level.
bool DependenceAnalysis::tryDelinearize(const SCEV *SrcSCEV,
                                        const SCEV *DstSCEV,
                                        SmallVectorImpl<Subscript> &Pair,
                                        const SCEV *ElementSize) {
  const SCEVUnknown *SrcBase =
      dyn_cast<SCEVUnknown>(SE->getPointerBase(SrcSCEV));
  const SCEVUnknown *DstBase =
      dyn_cast<SCEVUnknown>(SE->getPointerBase(DstSCEV));

  if (!SrcBase || !DstBase || SrcBase != DstBase)
    return false;

  SrcSCEV = SE->getMinusSCEV(SrcSCEV, SrcBase);
  DstSCEV = SE->getMinusSCEV(DstSCEV, DstBase);

  const SCEVAddRecExpr *SrcAR = dyn_cast<SCEVAddRecExpr>(SrcSCEV);
  const SCEVAddRecExpr *DstAR = dyn_cast<SCEVAddRecExpr>(DstSCEV);
  if (!SrcAR || !DstAR || !SrcAR->isAffine() || !DstAR->isAffine())
    return false;

  // First step: collect parametric terms in both array references.
  SmallVector<const SCEV *, 4> Terms;
  collectParametricTerms(*SrcAR, SE, Terms);
  collectParametricTerms(*DstAR, SE, Terms);

  // Second step: find subscript sizes.
  SmallVector<const SCEV *, 4> Sizes;
  findArrayDimensions(*SE, Terms, Sizes, ElementSize);

  // Third step: compute the access functions for each subscript.
  SmallVector<const SCEV *, 4> SrcSubscripts, DstSubscripts;
  computeAccessFunctions(*SrcAR, SE, SrcSubscripts, Sizes);
  computeAccessFunctions(*DstAr, SE, DstSubscripts, Sizes);

  // Fail when there is only a subscript: that's a linearized access function.
  if (SrcSubscripts.size() < 2 || DstSubscripts.size() < 2 ||
      SrcSubscripts.size() != DstSubscripts.size())
    return false;

  int size = SrcSubscripts.size();

  LLVM_DEBUG({
      dbgs() << "\nSrcSubscripts: ";
    for (int i = 0; i < size; i++)
      dbgs() << *SrcSubscripts[i];
    dbgs() << "\nDstSubscripts: ";
    for (int i = 0; i < size; i++)
      dbgs() << *DstSubscripts[i];
    });

  // The delinearization transforms a single-subscript MIV dependence test into
  // a multi-subscript SIV dependence test that is easier to compute. So we
  // resize Pair to contain as many pairs of subscripts as the delinearization
  // has found, and then initialize the pairs following the delinearization.
  Pair.resize(size);
  for (int i = 0; i < size; ++i) {
    Pair[i].Src = SrcSubscripts[i];
    Pair[i].Dst = DstSubscripts[i];
    unifySubscriptType(&Pair[i]);

    // FIXME: we should record the bounds SrcSizes[i] and DstSizes[i] that the
    // delinearization has found, and add these constraints to the dependence
    // check to avoid memory accesses overflow from one dimension into another.
    // This is related to the problem of determining the existence of data
    // dependences in array accesses using a different number of subscripts: in
    // C one can access an array A[100][100]; as A[0][9999], *A[9999], etc.
  }

  return true;
}
#endif

//===----------------------------------------------------------------------===//

#ifndef NDEBUG
// For debugging purposes, dump a small bit vector to dbgs().
static void dumpSmallBitVector(SmallBitVector &BV) {
  dbgs() << "{";
  for (int VI = BV.find_first(); VI >= 0; VI = BV.find_next(VI)) {
    dbgs() << VI;
    if (BV.find_next(VI) >= 0) {
      dbgs() << ' ';
    }
  }
  dbgs() << "}\n";
}
#endif

DDTest::DDTest(AAResults &AAR, HLNodeUtils &HNU) : AAR(AAR), HNU(HNU) {
  LLVM_DEBUG(dbgs() << "DDTest initiated\n");
  WorkCE.clear();
}

DDTest::~DDTest() {
  LLVM_DEBUG(dbgs() << "\n ~DDTest called\n");
  for (auto I = WorkCE.begin(), E = WorkCE.end(); I != E; ++I) {
    // const CanonExpr *CE = *I;
    // LLVM_DEBUG(dbgs() << "CE: " << CE << " "; CE->dump());
    HNU.getCanonExprUtils().destroy(const_cast<CanonExpr *>(*I));
  }

  WorkCE.clear();
}

bool DDTest::queryAAIndep(const RegDDRef *SrcDDRef, const RegDDRef *DstDDRef,
                          unsigned LoopLevel) {
  assert(SrcDDRef->isMemRef() && DstDDRef->isMemRef() &&
         "Both should be mem refs");

  if (SrcDDRef == DstDDRef) {
    return false;
  }

  // Note that we do not check that the indexing is also invariant because
  // RegDDRef::getMemoryLocation guarantees that a precise (i.e., known size)
  // footprint is returned only when the pointer is completely region
  // invariant. This ensures that the MemoryLocations are loop invariant if the
  // base is invariant.
  MemoryLocation SrcLoc = SrcDDRef->getMemoryLocation();
  MemoryLocation DstLoc = DstDDRef->getMemoryLocation();

  // Alias analysis only reasons about a pair of contemporary SSA values. In
  // order to use its results to break dependencies across a loop, (that is,
  // involving non-contemporary pairs of values,) we must know additional
  // properties about the two memory footprints. If we can't show that it is
  // safe to use the normal "alias" semantics, then we must resort to
  // "loopCarriedAlias" semantics, which will provide less precise results.
  bool RequiresLoopCarriedAA = true;
  if (SrcDDRef->isStructurallyInvariantAtLevel(LoopLevel, false) ||
      DstDDRef->isStructurallyInvariantAtLevel(LoopLevel, false)) {
    // If we can prove that either memory footprint is completely loop
    // invariant, then "alias" semantics are sufficient. For example, consider
    // the references to A and B below:
    //
    //     n = ...
    //     do i1:
    //       A = call()
    //       A[f(i1)] = ... + B[n]
    //
    // Both the base pointer and indexing for the A reference vary. However,
    // &B[n] is completely invariant w.r.t. the i1 loop. Because of this,
    // we know that any possible dependence would imply aliasing (between
    // contemporary SSA values) within one iteration of i1, meaning that
    // "alias" semantics cannot yield "NoAlias". It follows that "isNoAlias"
    // conclusively precludes any possible dependence.
    RequiresLoopCarriedAA = false;
  } else if (SrcDDRef->getBaseCE()->isLinearAtLevel(LoopLevel) ||
             DstDDRef->getBaseCE()->isLinearAtLevel(LoopLevel)) {
    // Alternatively, we can show that at least one base pointer is invariant
    // and query alias analysis with "unknown" access size. Consider the
    // example below, which borrows array slice notation from Fortran to depict
    // that the size of the access is unknown:
    //
    //     do i1:
    //       A = call()
    //       A[i1:] = ... + B[i1+1:]
    //
    // Both the base pointer and indexing for the A reference vary again.
    // However, because the base pointer B is not varying and we have unknown
    // size footprints, any possible dependence would still imply aliasing
    // within at least one i1 loop iteration. (Note that if the A footprint
    // were single element, e.g., "A[i1]", then this would not be the case.)
    RequiresLoopCarriedAA = false;

    // We don't need to explicitly check that the locations are UnknownSize.
    // This should always be the case since RegDDRef::getMemoryLocation only
    // returns precise sizes for fully region-invariant pointers, and we have
    // already checked that's not the case.
    assert(!SrcLoc.Size.isPrecise() &&
           "Unexpected precise size from getMemoryLocation()");
    assert(!DstLoc.Size.isPrecise() &&
           "Unexpected precise size from getMemoryLocation()");
  }

  DEBUG_AA(dbgs() << "call queryAAIndep() with respect to loop level "
                  << LoopLevel << ":\n");
  DEBUG_AA(SrcDDRef->dump());
  DEBUG_AA(dbgs() << "\n");
  DEBUG_AA(DstDDRef->dump());
  DEBUG_AA(dbgs() << "\nR: ");

  if (VaryingBaseHandling == VaryingBaseMode::QueryAlias ||
      !RequiresLoopCarriedAA) {
    if (AAR.isNoAlias(SrcLoc, DstLoc)) {
      DEBUG_AA(dbgs() << "No Alias\n\n");
      return true;
    }
  } else if (VaryingBaseHandling == VaryingBaseMode::QueryLoopCarriedAlias &&
             AAR.isLoopCarriedNoAlias(SrcLoc, DstLoc)) {
    // If the base pointer is not invariant to the loop, then we need to query
    // a stronger AA interface.
    DEBUG_AA(dbgs() << "Loop-carried No Alias\n\n");
    return true;
  }

  DEBUG_AA(dbgs() << "May Alias\n\n");
  return false;
}

// The function check if there could be a dependence between two mem refs
// with same base and shape, but different destination type sizes.
static bool mayIntersectDueToTypeCast(const RegDDRef *Ref1,
                                      const RegDDRef *Ref2) {
  assert(Ref1->isMemRef() && Ref2->isMemRef() && "Memref expected");

  if (!Ref1->getBitCastDestVecOrElemType() &&
      !Ref2->getBitCastDestVecOrElemType())
    return false;

  uint64_t Size1Dst = Ref1->getDestTypeSizeInBytes();
  uint64_t Size1Src =
      Ref1->getSrcType()->isSized() ? Ref1->getSrcTypeSizeInBytes() : 0;
  uint64_t Size2Dst = Ref2->getDestTypeSizeInBytes();
  uint64_t Size2Src =
      Ref2->getSrcType()->isSized() ? Ref2->getSrcTypeSizeInBytes() : 0;

  // Only proceed if casting changes type size
  if ((Size1Src >= Size1Dst) && (Size2Src >= Size2Dst))
    return false;

  int64_t Distance;
  if (!DDRefUtils::getConstByteDistance(Ref1, Ref2, &Distance)) {
    // If distance is unknown go conservative.
    // TODO: add more logic to recognize special cases like:
    //     for(i=0;i<n;i++)
    //       a[i] = ...;
    //       ...  = (i32*a)[n+1];
    return true;
  } else {
    if (Distance <= 0) {
      // Handles this case-
      //
      // %A is i8* type
      // SrcDDRef - (i16*)(%A)[0]
      // DstDDRef - (%A)[1]
      //
      if ((uint64_t)(-Distance) < Size1Dst)
        return true;
    } else if ((uint64_t)Distance < Size2Dst) {
      // Handles this case-
      //
      // %A is i8* type
      // SrcDDRef - (%A)[1]
      // DstDDRef - (i16*)(%A)[0]
      //
      return true;
    }
  }
  return false;
}

// depends:
// Returns nullptr if there is no dependence.
// Otherwise, return a Dependence with as many details as possible.
// Corresponds to Section 3.1 in the paper
//
//            Practical Dependence Testing
//            Goff, Kennedy, Tseng
//            PLDI 1991
//
// Care is required to keep the routine below, getSplitIteration(),
// up to date with respect to this routine.
//
// ForDDGBuild flag:
//   1) Set to true when Building DDG,
//       Returned DV includes reverse direction if needed
//   2) Set to false when DV in DDEdge needs to be refined
//      The DV returned is from SrcDDRef to DstDDRef. No flipping will be
//      done.
//
// ForFusion: Assumes both Src and Dst DDRef are in the same loopnest

std::unique_ptr<Dependences> DDTest::depends(const DDRef *SrcDDRef,
                                             const DDRef *DstDDRef,
                                             const DirectionVector &InputDV,
                                             bool ForDDGBuild, bool ForFusion) {

  //
  //
  // This query is useful for  loop fusion or other loop transformations
  // where only DV is required or additional results such as distances,
  // or peelable to remove dependences
  // InputDVs are normally (* * * * .. *) or (= = ..  * * )
  // for testing certain inner levels.
  // But it will accept other DV (except  NONE), up to the nesting levels.
  // e.g.   (=  <>) will check if the innermost loop can be parallelized.
  //
  // Sample for calling this function & dump result:
  //    DVectorTy InputDV;
  //   (which is actually  DVType InputDV[MaxLoopNestLevel])
  //    DA.setInputDV((InputDV, 1, CommonLevels);
  // 	  auto D = DA.depends(ddref[0], ddref[1], InputDV);
  //    if (D == nullptr)
  //       OS << "is Independent!";
  //    else
  //      D->dump(OS);
  // result is acquired via unique_ptr where destructor are called
  // automatically to free memory.
  //
  // Output structure: DV, distance, indicator for DV being reversed.
  // (when true, the result is from  DstDDRef to SrcDDef.
  // e.g.  for a[i] = a[i+1], assuming the caller
  //        makes SrcDDRef:   a[i] =
  //              DstDDRef:        = a[i+1]
  //
  //        Result returned (non nullptr):  (<),  isReversed=true
  //        anti dep can be implied based on the lval/rval
  //
  // (1) ForFusion - when invoked from Fusion using Demenad Driven DD,
  //     it is set as true. DDTest assumes the 2 DDRefs are within
  //     the same loop nest.
  //     For compile time saving,  Refs that have no common nests,
  //     DV (=) is used  if the one of the refs has no enclosing loop,
  //     otherwise  DV (*) is used.
  //     Loop Fusion has to invoke the Demand Driven DD for refinement.
  //
  // (2) Current code builds DV up to common levels only.
  //     This would be okay for most practical purposes.
  //     Extension can be made later for other needs such as fusion or
  //     privatization.
  //     Need some work for refs outside loop - should always has a DV
  //     instead of  empty

  bool EqualBaseAndShape = false;

  LLVM_DEBUG(dbgs() << "\n Src, Dst DDRefs\n"; SrcDDRef->dump());
  LLVM_DEBUG(dbgs() << ",  "; DstDDRef->dump());
  LLVM_DEBUG(dbgs() << "\n"
                    << SrcDDRef->getHLDDNode()->getNumber() << ":"
                    << DstDDRef->getHLDDNode()->getNumber());

  LLVM_DEBUG(dbgs() << "\n Input DV "; InputDV.print(dbgs()));

  assert(SrcDDRef->getSymbase() == DstDDRef->getSymbase() &&
         "Asking DDA for distinct references is useless");
  assert((SrcDDRef->isLval() || DstDDRef->isLval()) &&
         "DDA is not handling input dependencies");

  const RegDDRef *SrcRegDDRef = dyn_cast<RegDDRef>(SrcDDRef);
  const RegDDRef *DstRegDDRef = dyn_cast<RegDDRef>(DstDDRef);

  // Set loop nesting levels, NoCommonNest flag
  establishNestingLevels(SrcDDRef, DstDDRef, ForFusion);

  // If both are memory refs
  bool TestingMemRefs = SrcRegDDRef && SrcRegDDRef->isMemRef();

  if (TestingMemRefs) {
    assert(DstRegDDRef && DstRegDDRef->isMemRef() &&
           "Only one of the refs is a memref");
    // We can skip creating edges for constant memory
    if (SrcRegDDRef->accessesConstantArray() ||
        DstRegDDRef->accessesConstantArray()) {
      return nullptr;
    }

    // If we're refining a result for an inner loop, determine the level of
    // this inner loop. (Otherwise, we're effectively concerned with the
    // outermost loop.) Conservatively, assume level 1 when running for fusion.
    unsigned RefiningLevel = 1;
    if (!ForDDGBuild) {
      // Look for the outermost level that we're testing. For example, if the
      // DV is (=, *, *), then we want to compute that we're refining level 2.
      for (; RefiningLevel <= CommonLevels; ++RefiningLevel) {
        auto Direction = InputDV[RefiningLevel - 1];
        if (Direction != DVKind::EQ) {
          break;
        }
      }
      assert(InputDV.isTestingForInnermostLoop(RefiningLevel) &&
             "Unexpected refinement level");
    }

    // Inquire disam util to get INDEP based on type/scope based analysis.
    LLVM_DEBUG(dbgs() << "AA query: ");
    if (queryAAIndep(SrcRegDDRef, DstRegDDRef, RefiningLevel)) {
      LLVM_DEBUG(dbgs() << "no alias\n");
      return nullptr;
    }
    LLVM_DEBUG(dbgs() << "may alias\n");

    // TODO: MaxLoopNestLevel should be computed from InputDV.
    EqualBaseAndShape =
        DDRefUtils::haveEqualBaseAndShape(SrcRegDDRef, DstRegDDRef, true);
  }

  LLVM_DEBUG(dbgs() << "\ncommon nesting levels = " << CommonLevels << "\n");
  LLVM_DEBUG(dbgs() << "\nmaximum nesting levels = " << MaxLevels << "\n");

  if (NoCommonNest && (SrcDDRef == DstDDRef)) {
    // No edge needed
    return nullptr;
  }

  Dependences Result(SrcDDRef, DstDDRef, CommonLevels);

  if (NoCommonNest && !ForFusion && (SrcLevels == 0 || DstLevels == 0)) {
    Result.setDirection(1, DVKind::EQ);
  }

  // Earlier we tried to break the entire dependence using alias analysis, but
  // were not able to say anything conclusive. However, alias analysis may be
  // able to at least tell us that there's no dependence at some inner loop
  // level. If this is the case, it will update the direction vector so that
  // the inner level (and levels within) have "NONE" directions.
  if (TestingMemRefs)
    refineAAIndep(Result, SrcRegDDRef, DstRegDDRef);

  ++TotalArrayPairs;
  WorkCE.clear();

  //  Number of dimemsion are different or different base: need to bail out,
  //  except for IVDEP
  if (TestingMemRefs) {
    // Refine fake-to-real refs edge if canUsePointeeSize is set for fake ref.
    bool SrcBadFakeRef =
        SrcRegDDRef->isFake() && !SrcRegDDRef->canUsePointeeSize();
    bool DstBadFakeRef =
        DstRegDDRef->isFake() && !DstRegDDRef->canUsePointeeSize();
    if (!EqualBaseAndShape || SrcBadFakeRef || DstBadFakeRef) {
      adjustDV(Result, EqualBaseAndShape, SrcRegDDRef, DstRegDDRef);
      return std::make_unique<Dependences>(Result);
    }
  }

  if (!EqualBaseAndShape || (NoCommonNest && !ForFusion)) {
    LLVM_DEBUG(dbgs() << "\nDiff dim,  base, or no common nests\n");
    // DV has been initialized as *
    return std::make_unique<Dependences>(Result);
  }

  // Same base subscripts with different types could cause a dependency
  if (TestingMemRefs && EqualBaseAndShape &&
      mayIntersectDueToTypeCast(SrcRegDDRef, DstRegDDRef))
    return std::make_unique<Dependences>(Result);

  unsigned Pairs = SrcRegDDRef->getNumDimensions();

  LLVM_DEBUG(dbgs() << " # of Pairs " << Pairs << "\n");

  SmallVector<Subscript, 4> Pair(Pairs);

  // TODO: compare lower and stride of src/dst ref.
  if (SrcRegDDRef) {
    int P = 0;
    for (auto CE = SrcRegDDRef->canon_begin(), E = SrcRegDDRef->canon_end();
         CE != E; CE++, P++) {
      Pair[P].Src = *CE;
    }

  } else {
    const auto *BRef = cast<BlobDDRef>(SrcDDRef);
    Pair[0].Src = BRef->getSingleCanonExpr();
  }

  if (DstRegDDRef) {
    int P = 0;
    for (auto CE = DstRegDDRef->canon_begin(), E = DstRegDDRef->canon_end();
         CE != E; CE++, P++) {
      Pair[P].Dst = *CE;
    }
  } else {
    const auto *BRef = cast<BlobDDRef>(DstDDRef);
    Pair[0].Dst = BRef->getSingleCanonExpr();
  }

  // Note: Couple of original functionality were skipped
  //  UnifyingSubscriptType due to different sign extension

  const HLLoop *SrcLoop = nullptr;
  const HLLoop *DstLoop = nullptr;

  HLLoop *SrcParent = SrcDDRef->getHLDDNode()->getParentLoop();
  HLLoop *DstParent = DstDDRef->getHLDDNode()->getParentLoop();

  if (SrcParent) {
    SrcLoop = SrcParent;
  }
  if (DstParent) {
    DstLoop = DstParent;
  }

  if (TestingMemRefs &&
      tryDelinearize(SrcRegDDRef, DstRegDDRef, InputDV, Pair, ForDDGBuild)) {
    LLVM_DEBUG(dbgs() << "\nDelinearized!");
    Pairs = Pair.size();
  }

  for (unsigned P = 0; P < Pairs; ++P) {
    Pair[P].Loops.resize(MaxLevels + 1);
    Pair[P].GroupLoops.resize(MaxLevels + 1);
    Pair[P].Group.resize(Pairs);
    removeMatchingExtensions(&Pair[P]);
    Pair[P].Classification =
        classifyPair(Pair[P].Src, SrcLoop, Pair[P].Dst, DstLoop, Pair[P].Loops);
    Pair[P].GroupLoops = Pair[P].Loops;
    Pair[P].Group.set(P);
    LLVM_DEBUG(dbgs() << "\n    subscript " << P << "\n");
    LLVM_DEBUG(dbgs() << "\nsrc = "; (Pair[P].Src)->dump());
    LLVM_DEBUG(dbgs() << "\ndst = "; (Pair[P].Dst)->dump());
    LLVM_DEBUG(dbgs() << "\nclass = " << Pair[P].Classification << "\n");
    LLVM_DEBUG(dbgs() << "\nloops = ");
    LLVM_DEBUG(dumpSmallBitVector(Pair[P].Loops));
  }

  SmallBitVector Separable(Pairs);
  SmallBitVector Coupled(Pairs);

  // Partition subscripts into separable and minimally-coupled groups
  // Algorithm in paper is algorithmically better;
  // this may be faster in practice. Check someday.
  //
  // Here's an example of how it works. Consider this code:
  //
  //   for (i = ...) {
  //     for (j = ...) {
  //       for (k = ...) {
  //         for (l = ...) {
  //           for (m = ...) {
  //             A[i][j][k][m] = ...;
  //             ... = A[0][j][l][i + j];
  //           }
  //         }
  //       }
  //     }
  //   }
  //
  // There are 4 subscripts here:
  //    0 [i] and [0]
  //    1 [j] and [j]
  //    2 [k] and [l]
  //    3 [m] and [i + j]
  //
  // We've already classified each subscript pair as ZIV, SIV, etc.,
  // and collected all the loops mentioned by pair P in Pair[P].Loops.
  // In addition, we've initialized Pair[P].GroupLoops to Pair[P].Loops
  // and set Pair[P].Group = {P}.
  //
  //      Src Dst    Classification Loops  GroupLoops Group
  //    0 [i] [0]         SIV       {1}      {1}        {0}
  //    1 [j] [j]         SIV       {2}      {2}        {1}
  //    2 [k] [l]         RDIV      {3,4}    {3,4}      {2}
  //    3 [m] [i + j]     MIV       {1,2,5}  {1,2,5}    {3}
  //
  // For each subscript SI 0 .. 3, we consider each remaining subscript, SJ.
  // So, 0 is compared against 1, 2, and 3; 1 is compared against 2 and 3, etc.
  //
  // We begin by comparing 0 and 1. The intersection of the GroupLoops is empty.
  // Next, 0 and 2. Again, the intersection of their GroupLoops is empty.
  // Next 0 and 3. The intersection of their GroupLoop = {1}, not empty,
  // so Pair[3].Group = {0,3} and Done = false (that is, 0 will not be added
  // to either Separable or Coupled).
  //
  // Next, we consider 1 and 2. The intersection of the GroupLoops is empty.
  // Next, 1 and 3. The intersectionof their GroupLoops = {2}, not empty,
  // so Pair[3].Group = {0, 1, 3} and Done = false.
  //
  // Next, we compare 2 against 3. The intersection of the GroupLoops is empty.
  // Since Done remains true, we add 2 to the set of Separable pairs.
  //
  // Finally, we consider 3. There's nothing to compare it with,
  // so Done remains true and we add it to the Coupled set.
  // Pair[3].Group = {0, 1, 3} and GroupLoops = {1, 2, 5}.
  //
  // In the end, we've got 1 separable subscript and 1 coupled group.
  for (unsigned SI = 0; SI < Pairs; ++SI) {
    if (Pair[SI].Classification == Subscript::NonLinear) {
      // ignore these, but collect loops for later
      ++NonlinearSubscriptPairs;
      collectCommonLoops(Pair[SI].Src, SrcLoop, Pair[SI].Loops);
      collectCommonLoops(Pair[SI].Dst, DstLoop, Pair[SI].Loops);
      Result.Consistent = false;
    } else if (Pair[SI].Classification == Subscript::ZIV) {
      // always separable
      Separable.set(SI);
    } else {
      // SIV, RDIV, or MIV, so check for coupled group
      bool Done = true;
      for (unsigned SJ = SI + 1; SJ < Pairs; ++SJ) {
        SmallBitVector Intersection = Pair[SI].GroupLoops;
        Intersection &= Pair[SJ].GroupLoops;
        if (Intersection.any()) {
          // accumulate set of all the loops in group
          Pair[SJ].GroupLoops |= Pair[SI].GroupLoops;
          // accumulate set of all subscripts in group
          Pair[SJ].Group |= Pair[SI].Group;
          Done = false;
        }
      }
      if (Done) {
        if (Pair[SI].Group.count() == 1) {
          Separable.set(SI);
          ++SeparableSubscriptPairs;
        } else {
          Coupled.set(SI);
          ++CoupledSubscriptPairs;
        }
      }
    }
  }

  LLVM_DEBUG(dbgs() << "    Separable = ");
  LLVM_DEBUG(dumpSmallBitVector(Separable));
  LLVM_DEBUG(dbgs() << "    Coupled = ");
  LLVM_DEBUG(dumpSmallBitVector(Coupled));

  Constraint NewConstraint;

  NewConstraint.setAny();

  bool IsCollapsedRefs = (SrcRegDDRef->isCollapsed() && DstRegDDRef->isCollapsed());
  bool IsCollapsedWithDifferentHigherDim = false;
  if (!ForFusion) {
    // test separable subscripts
    for (int SI = Separable.find_first(); SI >= 0;
         SI = Separable.find_next(SI)) {
      LLVM_DEBUG(dbgs() << "\ntesting subscript " << SI);
      switch (Pair[SI].Classification) {
      case Subscript::ZIV: {
        LLVM_DEBUG(dbgs() << ", ZIV\n");
        if (IsCollapsedRefs) {
          const CanonExpr *Delta = getMinus(Pair[SI].Src, Pair[SI].Dst);
          int64_t Distance = 0;
          if (Delta && Delta->isIntConstant(&Distance)) {
            IsCollapsedWithDifferentHigherDim |= (Distance != 0);
          }
        }
        if (testZIV(Pair[SI].Src, Pair[SI].Dst, Result))
          if (!IsCollapsedWithDifferentHigherDim) {
            return nullptr;
          }
        break;
      }

      case Subscript::SIV: {
        LLVM_DEBUG(dbgs() << ", SIV\n");
        unsigned Level;
        const CanonExpr *SplitIter = nullptr;
        bool TestMore;

        if (testSIV(Pair[SI].Src, Pair[SI].Dst, Level, Result, NewConstraint,
                    SplitIter, SrcLoop, DstLoop, TestMore, ForFusion)) {
          return nullptr;
        }
        if (TestMore &&
            banerjeeMIVtest(Pair[SI].Src, Pair[SI].Dst, InputDV, Pair[SI].Loops,
                            Result, SrcLoop, DstLoop)) {
          return nullptr;
        }

        break;
      }

      case Subscript::RDIV:
        LLVM_DEBUG(dbgs() << ", RDIV\n");
        if (testRDIV(Pair[SI].Src, Pair[SI].Dst, Result, SrcLoop, DstLoop)) {
          return nullptr;
        }
        break;
      case Subscript::MIV:

        LLVM_DEBUG(dbgs() << ", MIV\n");

        if (testMIV(Pair[SI].Src, Pair[SI].Dst, InputDV, Pair[SI].Loops, Result,
                    SrcLoop, DstLoop)) {
          return nullptr;
        }
        break;
      default:
        llvm_unreachable("subscript has unexpected classification");
      }

      // Refine DV based on input DV
      // result[Level-1].DV already computed for SIV
      // Take interection of result with input dv for a particular level
      // if empty, INDEP is obtained

      if (Pair[SI].Classification == Subscript::SIV) {

        const HLLoop *IVLoop = getLoop(Pair[SI].Src, SrcLoop);
        if (IVLoop == nullptr) {
          IVLoop = getLoop(Pair[SI].Dst, DstLoop);
        }
        assert(IVLoop && "SIV must have an assoicated loop");

        unsigned Level = mapSrcLoop(IVLoop);

        if (Level <= Result.CommonLevels) {
          // DV is computed up to Common Level of the 2 DDRef
          Result.setDirection(Level,
                              Result.getDirection(Level) & InputDV[Level - 1]);

          if (Result.getDirection(Level) == DVKind::NONE) {
            return nullptr;
          }
        }
      }
    }
  } else {
    //  Note: The other tests - SIV, RDIV - can only be used when the 2 Refs are
    //  actually within the same  nests
    for (int SI = Separable.find_first(); SI >= 0;
         SI = Separable.find_next(SI)) {
      switch (Pair[SI].Classification) {
      case Subscript::ZIV:
        if (testZIV(Pair[SI].Src, Pair[SI].Dst, Result))
          return nullptr;
        break;
      default:
        if (testMIV(Pair[SI].Src, Pair[SI].Dst, InputDV, Pair[SI].Loops, Result,
                    SrcLoop, DstLoop)) {
          return nullptr;
        }
      }
    }
  }

  // Delta test
  if (Coupled.count()) {
    // test coupled subscript groups
    LLVM_DEBUG(dbgs() << "\nstarting on coupled subscripts\n");
    LLVM_DEBUG(dbgs() << "MaxLevels + 1 = " << MaxLevels + 1 << "\n");
    SmallVector<Constraint, 4> Constraints(MaxLevels + 1);

    for (unsigned II = 0; II <= MaxLevels; ++II) {
      Constraints[II].setAny();
    }
    for (int SI = Coupled.find_first(); SI >= 0; SI = Coupled.find_next(SI)) {
      LLVM_DEBUG(dbgs() << "testing subscript group " << SI << " { ");
      SmallBitVector Group(Pair[SI].Group);
      SmallBitVector Sivs(Pairs);
      SmallBitVector Mivs(Pairs);
      SmallBitVector ConstrainedLevels(MaxLevels + 1);
      for (int SJ = Group.find_first(); SJ >= 0; SJ = Group.find_next(SJ)) {
        LLVM_DEBUG(dbgs() << SJ << " ");
        if (Pair[SJ].Classification == Subscript::SIV)
          Sivs.set(SJ);
        else
          Mivs.set(SJ);
      }
      LLVM_DEBUG(dbgs() << "}\n");
      while (Sivs.any()) {
        bool Changed = false;
        bool TestMore;
        for (int SJ = Sivs.find_first(); SJ >= 0; SJ = Sivs.find_next(SJ)) {
          LLVM_DEBUG(dbgs() << "testing subscript " << SJ << ", SIV\n");
          // SJ is an SIV subscript that's part of the current coupled group
          unsigned Level;
          const CanonExpr *SplitIter = nullptr;
          LLVM_DEBUG(dbgs() << "SIV\n");
          if (testSIV(Pair[SJ].Src, Pair[SJ].Dst, Level, Result, NewConstraint,
                      SplitIter, SrcLoop, DstLoop, TestMore, ForFusion))
            return nullptr;

          if (TestMore &&
              banerjeeMIVtest(Pair[SI].Src, Pair[SI].Dst, InputDV,
                              Pair[SI].Loops, Result, SrcLoop, DstLoop)) {
            return nullptr;
          }
          ConstrainedLevels.set(Level);
          if (intersectConstraints(&Constraints[Level], &NewConstraint)) {
            if (Constraints[Level].isEmpty()) {
              ++DeltaIndependence;
              return nullptr;
            }
            Changed = true;
          }
          Sivs.reset(SJ);
        }
        if (Changed) {
          // propagate, possibly creating new SIVs and ZIVs
          LLVM_DEBUG(dbgs() << "    propagating\n");
          LLVM_DEBUG(dbgs() << "\tMivs = ");
          LLVM_DEBUG(dumpSmallBitVector(Mivs));
          for (int SJ = Mivs.find_first(); SJ >= 0; SJ = Mivs.find_next(SJ)) {
            // SJ is an MIV subscript that's part of the current coupled group
            LLVM_DEBUG(dbgs() << "\tSJ = " << SJ << "\n");
            if (propagate(Pair[SJ].Src, Pair[SJ].Dst, Pair[SJ].Loops,
                          Constraints, Result.Consistent)) {
              LLVM_DEBUG(dbgs() << "\t    Changed\n");
              ++DeltaPropagations;
              Pair[SJ].Classification = classifyPair(
                  Pair[SJ].Src, SrcLoop, Pair[SJ].Dst, DstLoop, Pair[SJ].Loops);
              switch (Pair[SJ].Classification) {
              case Subscript::ZIV:
                LLVM_DEBUG(dbgs() << "ZIV\n");
                if (testZIV(Pair[SJ].Src, Pair[SJ].Dst, Result))
                  return nullptr;
                Mivs.reset(SJ);
                break;
              case Subscript::SIV:
                Sivs.set(SJ);
                Mivs.reset(SJ);
                break;
              case Subscript::RDIV:
              case Subscript::MIV:
                break;
              default:
                llvm_unreachable("bad subscript classification");
              }
            }
          }
        }
      }

      // test & propagate remaining RDIVs
      for (int SJ = Mivs.find_first(); SJ >= 0; SJ = Mivs.find_next(SJ)) {
        if (Pair[SJ].Classification == Subscript::RDIV) {
          LLVM_DEBUG(dbgs() << "RDIV test\n");
          if (testRDIV(Pair[SJ].Src, Pair[SJ].Dst, Result, SrcLoop, DstLoop))
            return nullptr;
          // I don't yet understand how to propagate RDIV results
          Mivs.reset(SJ);
        }
      }

      // test remaining MIVs
      // This code is temporary.
      // Better to somehow test all remaining subscripts simultaneously.
      for (int SJ = Mivs.find_first(); SJ >= 0; SJ = Mivs.find_next(SJ)) {
        if (Pair[SJ].Classification == Subscript::MIV) {
          LLVM_DEBUG(dbgs() << "MIV test\n");
          if (testMIV(Pair[SJ].Src, Pair[SJ].Dst, InputDV, Pair[SJ].Loops,
                      Result, SrcLoop, DstLoop))
            return nullptr;
        } else
          llvm_unreachable("expected only MIV subscripts at this point");
      }

      // update Result.DV from constraint vector
      LLVM_DEBUG(dbgs() << "    updating\n");
      for (int SJ = ConstrainedLevels.find_first(); SJ >= 0;
           SJ = ConstrainedLevels.find_next(SJ)) {
        updateDirection(Result.DV[SJ - 1], Constraints[SJ]);
        if (Result.DV[SJ - 1].Direction == DVKind::NONE)
          return nullptr;
      }
    }
  }

  // Make sure the Scalar flags are set correctly.
  // Note: getDirection(level):  [level-1] is used inside the function

  SmallBitVector CompleteLoops(MaxLevels + 1);
  for (unsigned SI = 0; SI < Pairs; ++SI) {
    CompleteLoops |= Pair[SI].Loops;
  }

  for (unsigned II = 1; II <= CommonLevels; ++II) {
    if (CompleteLoops[II]) {
      Result.DV[II - 1].Scalar = false;
    }
  }

  for (unsigned II = 1; II <= CommonLevels; ++II) {
    unsigned Level = II - 1;
    assert(Level < InputDV.size() && "Incorrect InputDV size");
    Result.DV[Level].Direction &= InputDV[Level];
    if (Result.DV[Level].Direction == DVKind::NONE) {
      LLVM_DEBUG(dbgs() << "\n\t return INDEP-09\n");
      return nullptr;
    }
  }

  bool AllEqual = true;
  for (unsigned II = 1; II <= CommonLevels; ++II) {
    if (Result.getDirection(II) != DVKind::EQ) {
      AllEqual = false;
      break;
    }
  }

  // LoopIndepedent init'ed as false
  // set it to true when all DV are equal

  if (AllEqual && !IsCollapsedWithDifferentHigherDim) {
    Result.LoopIndependent = true;
    // If src & sink are the same ddref  and the DV are all =
    // there is no DEP.
    if (SrcDDRef == DstDDRef) {
      LLVM_DEBUG(dbgs() << "\n\t return INDEP-11\n" << SrcDDRef << DstDDRef);
      return nullptr;
    }
  }

  adjustDV(Result, true, SrcRegDDRef, DstRegDDRef); // SameBase = true

  //
  //  Reverse DV when needed
  //

  bool NeedReversal = false;
  bool Done = false;

  if (ForDDGBuild) {
    for (unsigned II = 1; II <= CommonLevels && !Done; ++II) {
      switch (Result.getDirection(II)) {
      case DVKind::GT:
      case DVKind::GE:
      // ALL does not need reversal. The  check is in the caller
      case DVKind::ALL:
        NeedReversal = true;
        Done = true;
        break;
      case DVKind::LT:
      case DVKind::LE:
        Done = true;
        break;
      default:
        break;
      }
    }
  }

  if (NeedReversal) {
    Result.Reversed = true;
    LLVM_DEBUG(dbgs() << "\nDV based on reversing ddref src & sink!\n");
    for (unsigned II = 1; II <= CommonLevels; ++II) {
      switch (Result.getDirection(II)) {
      case DVKind::LT:
        Result.setDirection(II, DVKind::GT);
        Result.setDistance(II, getNegativeDist(Result.getDistance(II)));
        break;
      case DVKind::LE:
        Result.setDirection(II, DVKind::GE);
        Result.setDistance(II, getNegativeDist(Result.getDistance(II)));
        break;
      case DVKind::GT:
        Result.setDirection(II, DVKind::LT);
        Result.setDistance(II, getNegativeDist(Result.getDistance(II)));
        break;
      case DVKind::GE:
        Result.setDirection(II, DVKind::LE);
        Result.setDistance(II, getNegativeDist(Result.getDistance(II)));
        break;
      default:
        // =, *, <> remain the same
        Result.setDirection(II, Result.getDirection(II));
        Result.setDistance(II, Result.getDistance(II));
        break;
      }
    }
  }

  return std::make_unique<Dependences>(Result);
}

///  Create  DV for Backward Edge
///  ForwardDV will be changed if it has a leading  (<>)
///    Called when both forward and backward edges are needed
///           ( *  >  =)  returns  (*  <  =)
///           ( =  *  =)  returns  (=  *  =)
///           (<=  *  >)  returns  (<= *  <)
///           Explanation:
///           (<=  * >)  is equivalent to
/// --------------------
///  (<  * >)    no backedge needed
///  (=  * >)
///  ----------------------------
///  (= * >) is equivalent to
///  ----------------------------
///  (= < >)  no backedge needed
///  (= > >)  Need backedge DV (= <  <)
///  (= = >)  no backedge needed
///  -----------------------------
///  For simplicity we derive from it as  (<= * <)

void DDTest::splitDVForForwardBackwardEdge(DirectionVector &ForwardDV,
                                           DirectionVector &BackwardDV,
                                           unsigned MaxLevel) const {

  unsigned SplitLevel = 1;

  // Scan for leftmost * or <>

  for (unsigned II = 1; II <= MaxLevel; ++II) {
    // for (<> >)
    // ForwardDV will be changed  as (< >)
    // BackwardDV will be flipped as (< <)
    if (ForwardDV[II - 1] == DVKind::NE) {
      BackwardDV[II - 1] = ForwardDV[II - 1] = DVKind::LT;
      SplitLevel = II;
      break;
    }

    BackwardDV[II - 1] = ForwardDV[II - 1];

    if (ForwardDV[II - 1] == DVKind::ALL) {
      SplitLevel = II;
      break;
    }
  }

  for (unsigned II = SplitLevel + 1; II <= MaxLevel; ++II) {
    switch (ForwardDV[II - 1]) {
    case DVKind::LT:
      BackwardDV[II - 1] = DVKind::GT;
      break;
    case DVKind::LE:
      BackwardDV[II - 1] = DVKind::GE;
      break;
    case DVKind::GT:
      BackwardDV[II - 1] = DVKind::LT;
      break;
    case DVKind::GE:
      BackwardDV[II - 1] = DVKind::LE;
      break;
    default:
      BackwardDV[II - 1] = ForwardDV[II - 1];
      break;
    }
  }
}

static void printDirDistVectors(DirectionVector &ForwardDV,
                                DirectionVector &BackwardDV,
                                DistanceVector &ForwardDistV,
                                DistanceVector &BackwardDistV) {

  LLVM_DEBUG(dbgs() << "\nforward DV: "; ForwardDV.print(dbgs()));
  LLVM_DEBUG(dbgs() << "\nforward DistV: "; ForwardDistV.print(dbgs()));

  LLVM_DEBUG(dbgs() << "\nbackward DV: "; BackwardDV.print(dbgs()));
  LLVM_DEBUG(dbgs() << "\nbackward DistV: "; BackwardDistV.print(dbgs()));
}

DistTy DDTest::mapDVToDist(DVKind DV, unsigned Level,
                           const Dependences &Result) {
  if (DV == DVKind::EQ) {
    return 0;
  }
  if (DV == DVKind::ALL || DV == DVKind::NE) {
    return UnknownDistance;
  }
  int64_t CVal;
  const CanonExpr *DistCE = Result.getDistance(Level);

  if (DistCE && DistCE->isIntConstant(&CVal)) {
    if (MinDistance <= CVal && CVal <= MaxDistance) {
      int64_t PosVal = std::llabs(CVal);
      if (DV & DVKind::LT) {
        return PosVal;
      }
      if (DV & DVKind::GT) {
        return -PosVal;
      }
    }
  }
  return UnknownDistance;
}

void DDTest::populateDistanceVector(const DirectionVector &ForwardDV,
                                    const DirectionVector &BackwardDV,
                                    const Dependences &Result,
                                    DistanceVector &ForwardDistV,
                                    DistanceVector &BackwardDistV) {
  assert(ForwardDV.size() == BackwardDV.size() && "Mismatched DV sizes");
  assert(ForwardDistV.size() == BackwardDistV.size() &&
         "Mismatched DistV sizes");
  assert(ForwardDV.size() == ForwardDistV.size() &&
         "Mismatched DV/DistV sizes");

  unsigned Levels = ForwardDV.size();
  for (unsigned II = 1; II <= Levels; ++II) {
    ForwardDistV[II - 1] = mapDVToDist(ForwardDV[II - 1], II, Result);
    BackwardDistV[II - 1] = mapDVToDist(BackwardDV[II - 1], II, Result);
  }
}

void DDTest::adjustDV(Dependences &Result, bool SameBase,
                      const RegDDRef *SrcRegDDRef,
                      const RegDDRef *DstRegDDRef) {
  const HLInst *SrcInst = dyn_cast<HLInst>(SrcRegDDRef->getHLDDNode());
  const HLInst *DstInst = dyn_cast<HLInst>(DstRegDDRef->getHLDDNode());

  // Mark the innermost level DV of an edge as (=) when both src and sink refs
  // belong to 'sinked' instructions, belong to the innermost loop and no
  // optimizations happened in the loop after sinking.
  if (SrcInst && DstInst && SrcInst->isSinked() && DstInst->isSinked()) {
    auto *SrcParentLoop = SrcRegDDRef->getParentLoop();
    auto *DstParentLoop = DstRegDDRef->getParentLoop();
    if (SrcParentLoop && (SrcParentLoop == DstParentLoop) &&
        SrcParentLoop->isInnermost() &&
        SrcParentLoop->isUndoSinkingCandidate()) {
      adjustForInnermostAssumedDeps(Result);
    }
  }

  adjustDVforIVDEP(Result, SameBase);

  if (!SrcRegDDRef->isMemRef()) {
    return;
  }

  if (SrcRegDDRef->isCollapsed() && DstRegDDRef->isCollapsed()) {
    int64_t VecLen1 = SrcRegDDRef->getMaxVecLenAllowed();
    int64_t VecLen2 = DstRegDDRef->getMaxVecLenAllowed();
    int64_t FinalVecLen = (VecLen1 < VecLen2) ? VecLen1 : VecLen2;
    if (FinalVecLen) {
      auto Int64Ty = Type::getInt64Ty(HNU.getContext());
      adjustCollapsedDepsForInnermostLoop(
          Result, getConstantWithType(Int64Ty, FinalVecLen));
      return;
    }
  }

  //  DV cannot be overridden to (=) when
  // -   Src and Dst mem-refs are the same and
  // -   no IV and
  // -   Src does not dominate or post dominate the sink

  if (DDRefUtils::areEqual(SrcRegDDRef, DstRegDDRef) &&
      (!LCALoopLevel ||
       SrcRegDDRef->isStructurallyInvariantAtLevel(LCALoopLevel)) &&
      (SrcInst && DstInst &&
       (!HLNodeUtils::strictlyDominates(SrcInst, DstInst) ||
        !HLNodeUtils::strictlyPostDominates(SrcInst, DstInst)))) {
    return;
  }

  if (AssumeNoLoopCarriedDep == LoopCarriedDepMode::InnermostOnly) {
    adjustForInnermostAssumedDeps(Result);
  } else if (AssumeNoLoopCarriedDep == LoopCarriedDepMode::All) {
    adjustForAllAssumedDeps(Result);
  }
}

/// Here we treat "ivdep back" somewhat conservatively, attempting to roughly
/// break only "assumed" dependencies. However, "ivdep loop" is treated as a
/// clear assertion that there are no loop-carried dependencies.
///
/// For "ivdep back", the adjustment is as follows:
/// (1) If Base Exprs are different, set DV as =
/// (2) For constant distance, do not override.
///     IVDEP means distance >= trip count
///     Vectorizer should choose VL <= Distance
/// (3) Otherwise adjust DV
///
/// For "ivdep loop", the adjustment is a simple mask with DVKind::EQ.
bool DDTest::adjustDVforIVDEP(Dependences &Result, bool SameBase) {

  const HLLoop *Lp = LCALoop;
  bool IVDEPFound = false;
  unsigned II = LCALoopLevel;

  // TODO: Currently we only benefit from "ivdep loop" on the lowest common
  // ancestor loop. (That is, the innermost common ancestor loop.)
  // This is to avoid creating DVs such as (NONE, *), which is not really
  // understood properly by the rest of DD; the "NONE" may be interpreted as
  // meaning that there's complete independence.
  if (Lp && Lp->hasVectorizeIVDepLoopPragma()) {
    IVDEPFound = true;
    Result.setDirection(II, DVKind::EQ & Result.getDirection(II));
    // If we made an adjustment, skip this innermost loop when considering
    // "ivdep" below. It will not improve the result.
    --II;
    Lp = Lp->getParentLoop();
  }

  // Looping through parents allows IVDEP for more than 1 level
  // to be supported. But multiple levels vectorization is not
  // currently generated
  for (; II >= 1; --II, Lp = Lp->getParentLoop()) {
    if (Lp->hasVectorizeIVDepPragma()) {
      IVDEPFound = true;
      if (!SameBase) {
        // Do not change the result if it was already better than '='
        if (Result.getDirection(II) == DVKind::NONE)
          continue;
        Result.setDirection(II, DVKind::EQ);
        continue;
      }
      DistTy Distance = mapDVToDist(Result.getDirection(II), II, Result);
      if (Distance == UnknownDistance) {
        Result.setDirection(II, DVKind::EQ);
      }
    }
  }
  return IVDEPFound;
}

void DDTest::adjustForInnermostAssumedDeps(Dependences &Result) {
  if (!LCALoop) {
    return;
  }

  if (!LCALoop->isInnermost()) {
    return;
  }

  unsigned InnermostLoopLevel = LCALoop->getNestingLevel();

  DVKind Direction = Result.getDirection(InnermostLoopLevel);

  if (Direction == DVKind::ALL) {
    Result.setDirection(InnermostLoopLevel, DVKind::EQ);
  }
}

void DDTest::adjustForAllAssumedDeps(Dependences &Result) {
  for (unsigned II = 1; II <= LCALoopLevel; ++II) {
    DVKind Direction = Result.getDirection(II);

    if (Direction == DVKind::ALL) {
      Result.setDirection(II, DVKind::EQ);
    }
  }
}

void DDTest::adjustCollapsedDepsForInnermostLoop(Dependences &Result,
                                                 const CanonExpr *NewDist) {
  if (!LCALoop) {
    return;
  }

  if (!LCALoop->isInnermost()) {
    return;
  }

  unsigned InnermostLoopLevel = LCALoop->getNestingLevel();

  DVKind Direction = Result.getDirection(InnermostLoopLevel);

  if (Direction == DVKind::EQ) {
    Result.setDirection(InnermostLoopLevel, DVKind::LT);
    Result.setDistance(InnermostLoopLevel, NewDist);
  }
}

void DDTest::refineAAIndep(Dependences &Result, const RegDDRef *SrcRegDDRef,
                           const RegDDRef *DstRegDDRef) {
  // As an optimization, don't attempt to refine in the common case where the
  // bases are known to be the same. Refinement is not possible in this case.
  if (SrcRegDDRef->getBaseValue() == DstRegDDRef->getBaseValue())
    return;

  bool BrokeDep = false;
  // We don't re-examine the outermost level. If there was no aliasing there
  // then depends() would have concluded no dependence already.
  for (unsigned InnerLevel = 2, MaxLevel = Result.getLevels();
       InnerLevel <= MaxLevel; ++InnerLevel) {
    DVKind Direction = Result.getDirection(InnerLevel);
    if (Direction != DVKind::NONE) {
      // If we broke a dependence at an outer level, it applies to all inner
      // levels.
      BrokeDep = BrokeDep || queryAAIndep(SrcRegDDRef, DstRegDDRef, InnerLevel);
      if (BrokeDep)
        Result.setDirection(InnerLevel, DVKind::NONE);
    }
  }
}

void DDTest::setDVForPeelFirstAndReversed(DirectionVector &ForwardDV,
                                          DirectionVector &BackwardDV,
                                          const Dependences &Result,
                                          unsigned Levels) {

  // Result coming back from weakZeroSrcSIVtest
  // e.g. for i=0, 2
  //        x[2*i +2] = x[2];
  // Need special casing:
  // Forward DV is  (=)  Backward DV is (<)
  for (unsigned II = 1; II < Levels; ++II) {
    ForwardDV[II - 1] = Result.getDirection(II);
  }
  ForwardDV[Levels - 1] = DVKind::EQ;
  splitDVForForwardBackwardEdge(ForwardDV, BackwardDV, Levels);
  BackwardDV[Levels - 1] = DVKind::LT;
}

void DDTest::setDVForBiDirection(DirectionVector &ForwardDV,
                                 DirectionVector &BackwardDV,
                                 const Dependences &Result, unsigned Levels,
                                 unsigned LTGTLevel) {

  // Both directions
  // Leftmost non-equal is a *
  // Need to reverse one of the DV
  // e.g. one edge is ( * < >), the other shoud be (* > <)

  for (unsigned II = 1; II <= Levels; ++II) {
    // Computed from Src -> Dst (Forward edge)
    ForwardDV[II - 1] = Result.getDirection(II);
  }
  splitDVForForwardBackwardEdge(ForwardDV, BackwardDV, Levels);
  if (LTGTLevel) {
    // e.g. (= <> < =)
    // Forward  edge DV: (= < < =)
    // Backward edge DV: (= < > =)
    ForwardDV[LTGTLevel - 1] = BackwardDV[LTGTLevel - 1] = DVKind::LT;
  }
}

void DDTest::setDVForLoopIndependent(DirectionVector &ForwardDV,
                                     DirectionVector &BackwardDV,
                                     const Dependences &Result, unsigned Levels,
                                     unsigned SrcNum, unsigned DstNum) {
  //  DV are all =

  LLVM_DEBUG(dbgs() << "\nTopSortNum: " << SrcNum << " " << DstNum);
  if (SrcNum <= DstNum) {
    for (unsigned II = 1; II <= Levels; ++II) {
      ForwardDV[II - 1] = Result.getDirection(II);
    }
  } else {
    for (unsigned II = 1; II <= Levels; ++II) {
      BackwardDV[II - 1] = Result.getDirection(II);
    }
  }
}

void DDTest::setDVForLE(DirectionVector &ForwardDV, DirectionVector &BackwardDV,
                        const Dependences &Result, unsigned Levels) {

  // A forward edge (=) is needed here
  // do i1
  //    do i2
  //      a(-i1 + i2 + 25)=   (Src
  //    end
  //     = a(-i1 +25)         (Dst
  // end
  // This is done mostly for Loop Dist.
  // Problem only shows up with single backward edge.
  // For other loop Transformations, single edge of <= should be sufficent.
  // Only needed for non-temps. Actually, for temps, if it comes here,
  // The DV would be a *.

  for (unsigned II = 1; II < Levels; ++II) {
    ForwardDV[II - 1] = Result.getDirection(II);
  }
  ForwardDV[Levels - 1] = DVKind::EQ;
  BackwardDV[Levels - 1] = DVKind::LT;
}

void DDTest::setDVForCollapsedRefs(DirectionVector &BackwardDV,
                                   const Dependences &Result, unsigned Levels) {
  for (unsigned II = 1; II < Levels; ++II) {
    BackwardDV[II - 1] = Result.getDirection(II);
  }
  BackwardDV[Levels - 1] = DVKind::LT;
}

bool DDTest::findDependencies(DDRef *SrcDDRef, DDRef *DstDDRef,
                              const DirectionVector &InputDV,
                              DirectionVector &ForwardDV,
                              DirectionVector &BackwardDV,
                              DistanceVector &ForwardDistV,
                              DistanceVector &BackwardDistV,
                              bool *IsLoopIndepDepTemp) {

  // This interface is created to facilitate the building of DDG when forward or
  // backward  edges are needed.
  // Returns true    if Dependence is found
  //         false   if Independent is found
  //
  //  DO i1;  DO I2;
  //     A[i1][i2] =  A[i1][i2+1]
  // 	e.g. with Srce  A[i1][i2],  Dest  A[i1][i2+1]
  //
  //  true returned
  //  forwardDV[0:1]  is (0,0) Leftmost dv=0 implies NO EDGE needed in this
  //  direction
  //  backwardDV[0:1] is (=,<)
  //
  //  The orginal code in "depends function" always return * for temps (treated
  //  like scalar vars).
  //  New code added here for temps with more precision  in DV

  //  IsLoopIndepDepTemp, is returned as true for temps with
  //  Loop independent dependence
  //    t1 =
  //       = t1

  // the argument after InputDV indicates calling to rebuild DDG
  auto Result = depends(SrcDDRef, DstDDRef, InputDV, true, AssumeLoopFusion);

  *IsLoopIndepDepTemp = false;

  if (Result == nullptr) {
    LLVM_DEBUG(dbgs() << "\nIs Independent!\n");
    return false;
  } else {
    LLVM_DEBUG(Result->dump(dbgs()));
  }

  unsigned Levels = Result->getLevels();

  // DVs and DistVs must always be smaller than the constructed capacity, so
  // this should not result in reallocations.
  ForwardDV.resize(Levels);
  BackwardDV.resize(Levels);

  ForwardDistV.resize(Levels);
  BackwardDistV.resize(Levels);

  ///  Bidirectional DV is needed when scanning from L to R, it encounters
  ///  a * before hitting <.
  ///  If * is preceeded by <. then no backward edge is needed.
  ///  Exception:  when src == dst, 1 edge is enough for self output dep.
  ///  e.g.
  ///  (= = *   =)  Yes
  ///  (= = <=  *)  Yes
  ///  (= = <=  =)  no
  ///  (= = <   >)  no
  ///  See more details in splitDVForForwardBackwardEdge

  bool BiDirection = false;
  //  <> Level
  unsigned LTGTLevel = 0;
  if (SrcDDRef != DstDDRef) {
    for (unsigned II = 1; II <= Levels; ++II) {
      DVKind Direction = Result->getDirection(II);
      if (Direction == DVKind::LT) {
        break;
      }
      if (Direction == (DVKind::LT | DVKind::GT)) {
        BiDirection = true;
        LTGTLevel = II;
        break;
      }
      if (Direction == DVKind::ALL) {
        BiDirection = true;
        LLVM_DEBUG(dbgs() << "BiDirection needed!\n");
        break;
      }
    }
  }

  bool IsSrcRval = true;
  bool IsDstRval = true;
  bool IsCollapsedWithBackwardDep = false;


  HLNode *SrcHIR = SrcDDRef->getHLDDNode();
  HLNode *DstHIR = DstDDRef->getHLDDNode();
  assert(SrcHIR && "SrcHIR not null expected");
  assert(DstHIR && "DstHIR not null expected");

  unsigned SrcNum = SrcHIR->getTopSortNum();
  unsigned DstNum = DstHIR->getTopSortNum();

  if (RegDDRef *RegRef = dyn_cast<RegDDRef>(SrcDDRef)) {
    if (RegRef->isLval()) {
      IsSrcRval = false;
    }
    if (RegRef->isMemRef() && RegRef->isCollapsed() &&
        RegRef->getMaxVecLenAllowed()) {
      IsCollapsedWithBackwardDep = true;
    }
  }

  if (RegDDRef *RegRef = dyn_cast<RegDDRef>(DstDDRef)) {
    if (RegRef->isLval()) {
      IsDstRval = false;
    }
    if (RegRef->isMemRef() && RegRef->isCollapsed() &&
        RegRef->getMaxVecLenAllowed()) {
      IsCollapsedWithBackwardDep &= true;
    }
  }

  bool IsTemp = SrcDDRef->isTerminalRef();

  if (IsTemp) {

    // DV for Scalar temps could be refined. Calls to DA.depends
    // is still needed so it can set up the nesting level info.
    // It's a fast return because temps are classifed as non-linear.
    // result DV is all * at this stage

    bool IsFlow = false;
    bool IsAnti = false;

    // Skip self output dep for temp for now.
    // In theory, it's needed. but for analysis, we can just rely on flow &
    // anti alone
    assert(SrcDDRef != DstDDRef && "DD does not create self output edges.");

    //  Make SrcDDRef to be one that comes first in lexical order
    //  and switch DVs when reversed.

    bool IsReversed = false;
    LLVM_DEBUG(dbgs() << " src/dst num " << SrcNum << " " << DstNum);

    if (DstNum < SrcNum || ((DstNum == SrcNum) && IsDstRval && !IsSrcRval)) {
      std::swap(SrcDDRef, DstDDRef);
      std::swap(SrcHIR, DstHIR);
      std::swap(SrcNum, DstNum);
      IsReversed = true;
    }

    if (!IsSrcRval && IsDstRval) {
      if (DstNum == SrcNum) {
        IsAnti = true;
      } else {
        IsFlow = true;
      }
    } else if (IsSrcRval && !IsDstRval) {
      IsAnti = true;
    } else if (!IsSrcRval && !IsDstRval) {
      // IsOutput
    } else {
      // IsInput, no edge needed
      return false;
    }

    if (HLNodeUtils::dominates(SrcHIR, DstHIR)) {
      if (IsFlow) {
        // If src can reach Dst lexically
        //   assuming 2 level loop
        // a)  x = ;
        //       = x ;
        //
        //   set flow (= =)
        for (unsigned II = 1; II <= Levels; ++II) {
          ForwardDV[II - 1] = DVKind::EQ;
        }
        // Suppress ANTI (< ) edge to save Compile time for
        // StrictlyDominates case.
        // Instead, set a flag as below
        // Most Transformations would have to scan and drop this kind
        // of Anti Dep

        *IsLoopIndepDepTemp = true;
      } else if (IsAnti) {
        // b)    = x ;
        //     x =  ;
        //   1. single nest:
        //      anti (=)
        //      flow (<)
        //   2. Multi nests
        //      anti (=   =)
        //      flow (=<  *)

        for (unsigned II = 1; II <= Levels; ++II) {
          ForwardDV[II - 1] = DVKind::EQ;
        }
        if (Levels == 1) {
          BackwardDV[0] = DVKind::LT;
        } else {
          for (unsigned II = 1; II <= Levels; ++II) {
            if (II == 1) {
              BackwardDV[II - 1] = DVKind::LE;
            } else {
              BackwardDV[II - 1] = DVKind::ALL;
            }
          }
        }
      } else {
        // c) output when x = ;
        //                x = ;
        //    make it all = for now
        //    Strictly speaking, there should be another backedge (<)
        for (unsigned II = 1; II <= Levels; ++II) {
          ForwardDV[II - 1] = DVKind::EQ;
        }
      }
      if (IsReversed) {
        BackwardDV.swap(ForwardDV);
      }
    } else if ((IsFlow || IsAnti) && LCALoop &&
               !LCALoop->isLiveIn(SrcDDRef->getSymbase())) {
      // Make non-live-in temps all (=) for FLOW and ANTI dep
      for (unsigned II = 1; II <= Levels; ++II) {
        ForwardDV[II - 1] = DVKind::EQ;
      }
      if (IsReversed) {
        BackwardDV.swap(ForwardDV);
      }
    }

    if (ForwardDV[0] != DVKind::NONE || BackwardDV[0] != DVKind::NONE) {
      // If either forward or backward DV is filled, okay to return
      goto L1;
    }
  }

  // How to determine whether the edge is forward or backward:
  //  (1) bidirection: both forward & backward
  //  (2) if all EQ, look at TopSort order
  //  (3) if leftmost non-EQ dv is <
  //      isReversed implies backward else forward

  if (IsCollapsedWithBackwardDep) {
    setDVForCollapsedRefs(BackwardDV, *Result, Levels);
  } else if (Result->isPeelFirst(Levels) && Result->isReversed()) {
    setDVForPeelFirstAndReversed(ForwardDV, BackwardDV, *Result, Levels);
  } else if (BiDirection) {
    setDVForBiDirection(ForwardDV, BackwardDV, *Result, Levels, LTGTLevel);
  } else if (Result->isLoopIndependent()) {
    setDVForLoopIndependent(ForwardDV, BackwardDV, *Result, Levels, SrcNum,
                            DstNum);
  } else {
    // (3) Leftmost is <
    //     Src->Dest
    if (!Result->isReversed()) {
      for (unsigned II = 1; II <= Levels; ++II) {
        ForwardDV[II - 1] = Result->getDirection(II);
      }
    } else {
      //  Dest->Src
      for (unsigned II = 1; II <= Levels; ++II) {
        BackwardDV[II - 1] = Result->getDirection(II);
      }
      if ((!IsDstRval || !IsSrcRval) && (DstNum > SrcNum) && !IsTemp &&
          BackwardDV[Levels - 1] == DVKind::LE) {
        setDVForLE(ForwardDV, BackwardDV, *Result, Levels);
      }
    }
  }

L1:

  populateDistanceVector(ForwardDV, BackwardDV, *Result, ForwardDistV,
                         BackwardDistV);
  printDirDistVectors(ForwardDV, BackwardDV, ForwardDistV, BackwardDistV);
  return true;
}

void DirectionVector::setAsInput(const unsigned int StartLevel,
                                 const unsigned int EndLevel) {
  DirectionVector &InputDV = *this;
  InputDV.resize(EndLevel);

  // setInputDV (&InputDV, 3,4)
  // will construct (= = * *)

  for (unsigned II = 1; II < StartLevel; ++II) {
    InputDV[II - 1] = DVKind::EQ;
  }

  for (unsigned II = StartLevel; II <= EndLevel; ++II) {
    InputDV[II - 1] = DVKind::ALL;
  }
}

void DirectionVector::print(raw_ostream &OS, bool ShowLevelDetail) const {
  const DirectionVector &DV = *this;
  if (DV[0] == DVKind::NONE) {
    OS << "nil\n";
    return;
  }

  OS << "(";
  for (unsigned II = 1, Levels = size(); II <= Levels; ++II) {
    if (ShowLevelDetail) {
      OS << II << ": ";
    }

    switch (DV[II - 1]) {
    case DVKind::ALL:
      OS << "*";
      break;
    case DVKind::LT:
      OS << "<";
      break;
    case DVKind::EQ:
      OS << "=";
      break;
    case DVKind::LE:
      OS << "<=";
      break;
    case DVKind::GE:
      OS << ">=";
      break;
    case DVKind::GT:
      OS << ">";
      break;
    case DVKind::NE:
      OS << "<>";
      break;
    case DVKind::NONE:
      OS << "0";
      break;
    } // end:switch
    if (II != Levels) {
      OS << " ";
    }
  }
  OS << ") ";
}

void DistanceVector::print(raw_ostream &OS) const {
  const DistanceVector &DistV = *this;

  OS << "(";
  for (unsigned II = 1, Levels = size(); II <= Levels; ++II) {
    DistTy Distance = DistV[II - 1];
    if (Distance == UnknownDistance) {
      OS << "?";
    } else {
      OS << +Distance;
    }
    if (II != Levels) {
      OS << " ";
    }
  }
  OS << ") ";
}

/// Is  DV all ( = = = .. =)?
bool DirectionVector::isEQ() const {
  return std::all_of(begin(), end(),
                     [](DVKind Dir) { return Dir == DVKind::EQ; });
}

/// DV with leading = in leftmost:  (= = = *)
/// Notice that the rightmost DV can be anything
/// e.g. for testing for auto-parallel, it will be (= = = =)
bool DirectionVector::isTestingForInnermostLoop(
    unsigned InnermostLoopLevel) const {

  for (unsigned II = 1; II < InnermostLoopLevel; ++II) {
    auto Direction = (*this)[II - 1];
    if (Direction != DVKind::EQ) {
      return false;
    }
  }
  return true;
}

/// Is DV implying INDEP for level L to end?
/// e.g.  DV = (< *)	 implies INDEP for innermost loop
/// In this example, isDVIndepFromLevel(&DV, 2) return true
bool DirectionVector::isIndepFromLevel(unsigned Level) const {

  assert(CanonExpr::isValidLoopLevel(Level) && "incorrect Level");

  // If the Level is beyond the DV's common levels return true.
  if (Level > size())
    return true;

  // A DVKind::NONE at Level indicates no loop carried nor loop independent
  // dependence.
  unsigned LevelDir = (*this)[Level - 1];
  if (LevelDir == DVKind::NONE)
    return true;

  // DVKind::LT:  001
  // DVKind::GT : 100
  // LT | GT :    101 <=> Either LT or GT <=> DVKind::NE
  // DVKind::EQ : 010
  for (unsigned Lvl = 1; Lvl <= Level - 1; ++Lvl) {
    unsigned Dir = (*this)[Lvl - 1];
    assert(Dir != DVKind::NONE);
    if ((Dir & DVKind::EQ) == DVKind::NONE) {
      return true;
    }
  }

  return false;
}

#if 0


//===----------------------------------------------------------------------===//
// getSplitIteration -
// Rather than spend rarely-used space recording the splitting iteration
// during the Weak-Crossing SIV test, we re-compute it on demand.
// The re-computation is basically a repeat of the entire dependence test,
// though simplified since we know that the dependence exists.
// It's tedious, since we must go through all propagations, etc.
//
// Care is required to keep this code up to date with respect to the routine
// above, depends().
//
// Generally, the dependence analyzer will be used to build
// a dependence graph for a function (basically a map from instructions
// to dependences). Looking for cycles in the graph shows us loops
// that cannot be trivially vectorized/parallelized.
//
// We can try to improve the situation by examining all the dependences
// that make up the cycle, looking for ones we can break.
// Sometimes, peeling the first or last iteration of a loop will break
// dependences, and we've got flags for those possibilities.
// Sometimes, splitting a loop at some other iteration will do the trick,
// and we've got a flag for that case. Rather than waste the space to
// record the exact iteration (since we rarely know), we provide
// a method that calculates the iteration. It's a drag that it must work
// from scratch, but wonderful in that it's possible.
//
// Here's an example:
//
//    for (i = 0; i < 10; i++)
//        A[i] = ...
//        ... = A[11 - i]
//
// There's a loop-carried flow dependence from the store to the load,
// found by the weak-crossing SIV test. The dependence will have a flag,
// indicating that the dependence can be broken by splitting the loop.
// Calling getSplitIteration will return 5.
// Splitting the loop breaks the dependence, like so:
//
//    for (i = 0; i <= 5; i++)
//        A[i] = ...
//        ... = A[11 - i]
//    for (i = 6; i < 10; i++)
//        A[i] = ...
//        ... = A[11 - i]
//
// breaks the dependence and allows us to vectorize/parallelize
// both loops.
const  SCEV *DependenceAnalysis::getSplitIteration(const Dependence &Dep,
                                                   unsigned SplitLevel) {
  assert(Dep.isSplitable(SplitLevel) &&
         "Dep should be splitable at SplitLevel");
  Instruction *Src = Dep.getSrc();
  Instruction *Dst = Dep.getDst();
  assert(Src->mayReadFromMemory() || Src->mayWriteToMemory());
  assert(Dst->mayReadFromMemory() || Dst->mayWriteToMemory());
  assert(isLoadOrStore(Src));
  assert(isLoadOrStore(Dst));
  Value *SrcPtr = getPointerOperand(Src);
  Value *DstPtr = getPointerOperand(Dst);
  assert(underlyingObjectsAlias(AA, DstPtr, SrcPtr) ==
         AAResults::MustAlias);

  // establish loop nesting levels
  establishNestingLevels(Src, Dst);

  Dependences Result(Src, Dst, false, CommonLevels);

  // See if there are GEPs we can use.
  bool UsefulGEP = false;
  GEPOperator *SrcGEP = dyn_cast<GEPOperator>(SrcPtr);
  GEPOperator *DstGEP = dyn_cast<GEPOperator>(DstPtr);
  if (SrcGEP && DstGEP &&
      SrcGEP->getPointerOperandType() == DstGEP->getPointerOperandType()) {
    const SCEV *SrcPtrSCEV = SE->getSCEV(SrcGEP->getPointerOperand());
    const SCEV *DstPtrSCEV = SE->getSCEV(DstGEP->getPointerOperand());
    UsefulGEP =
      isLoopInvariant(SrcPtrSCEV, LI->getLoopFor(Src->getParent())) &&
      isLoopInvariant(DstPtrSCEV, LI->getLoopFor(Dst->getParent()));
  }
  unsigned Pairs = UsefulGEP ? SrcGEP->idx_end() - SrcGEP->idx_begin() : 1;
  SmallVector<Subscript, 4> Pair(Pairs);
  if (UsefulGEP) {
    unsigned P = 0;
    for (GEPOperator::const_op_iterator SrcIdx = SrcGEP->idx_begin(),
           SrcEnd = SrcGEP->idx_end(),
           DstIdx = DstGEP->idx_begin();
         SrcIdx != SrcEnd;
         ++SrcIdx, ++DstIdx, ++P) {
      Pair[P].Src = SE->getSCEV(*SrcIdx);
      Pair[P].Dst = SE->getSCEV(*DstIdx);
    }
  }
  else {
    const SCEV *SrcSCEV = SE->getSCEV(SrcPtr);
    const SCEV *DstSCEV = SE->getSCEV(DstPtr);
    Pair[0].Src = SrcSCEV;
    Pair[0].Dst = DstSCEV;
  }

  if (Delinearize && Pairs == 1 && CommonLevels > 1 &&
      tryDelinearize(Pair[0].Src, Pair[0].Dst, Pair, SE->getElementSize(Src))) {
    LLVM_DEBUG(dbgs() << "    delinerized GEP\n");
    Pairs = Pair.size();
  }

  for (unsigned P = 0; P < Pairs; ++P) {
    Pair[P].Loops.resize(MaxLevels + 1);
    Pair[P].GroupLoops.resize(MaxLevels + 1);
    Pair[P].Group.resize(Pairs);
    removeMatchingExtensions(&Pair[P]);
    Pair[P].Classification =
      classifyPair(Pair[P].Src, LI->getLoopFor(Src->getParent()),
                   Pair[P].Dst, LI->getLoopFor(Dst->getParent()),
                   Pair[P].Loops);
    Pair[P].GroupLoops = Pair[P].Loops;
    Pair[P].Group.set(P);
  }

  SmallBitVector Separable(Pairs);
  SmallBitVector Coupled(Pairs);

  // partition subscripts into separable and minimally-coupled groups
  for (unsigned SI = 0; SI < Pairs; ++SI) {
    if (Pair[SI].Classification == Subscript::NonLinear) {
      // ignore these, but collect loops for later
      collectCommonLoops(Pair[SI].Src,
                         LI->getLoopFor(Src->getParent()),
                         Pair[SI].Loops);
      collectCommonLoops(Pair[SI].Dst,
                         LI->getLoopFor(Dst->getParent()),
                         Pair[SI].Loops);
      Result.Consistent = false;
    }
    else if (Pair[SI].Classification == Subscript::ZIV)
      Separable.set(SI);
    else {
      // SIV, RDIV, or MIV, so check for coupled group
      bool Done = true;
      for (unsigned SJ = SI + 1; SJ < Pairs; ++SJ) {
        SmallBitVector Intersection = Pair[SI].GroupLoops;
        Intersection &= Pair[SJ].GroupLoops;
        if (Intersection.any()) {
          // accumulate set of all the loops in group
          Pair[SJ].GroupLoops |= Pair[SI].GroupLoops;
          // accumulate set of all subscripts in group
          Pair[SJ].Group |= Pair[SI].Group;
          Done = false;
        }
      }
      if (Done) {
        if (Pair[SI].Group.count() == 1)
          Separable.set(SI);
        else
          Coupled.set(SI);
      }
    }
  }

  Constraint NewConstraint;
  NewConstraint.setAny();

  // test separable subscripts
  for (int SI = Separable.find_first(); SI >= 0; SI = Separable.find_next(SI)) {
    switch (Pair[SI].Classification) {
    case Subscript::SIV: {
      unsigned Level;
      const SCEV *SplitIter = nullptr;
      (void) testSIV(Pair[SI].Src, Pair[SI].Dst, Level,
                     Result, NewConstraint, SplitIter, false);
      if (Level == SplitLevel) {
        assert(SplitIter != nullptr);
        return SplitIter;
      }
      break;
    }
    case Subscript::ZIV:
    case Subscript::RDIV:
    case Subscript::MIV:
      break;
    default:
      llvm_unreachable("subscript has unexpected classification");
    }
  }

  if (Coupled.count()) {
    // test coupled subscript groups
    SmallVector<Constraint, 4> Constraints(MaxLevels + 1);
    for (unsigned II = 0; II <= MaxLevels; ++II)
      Constraints[II].setAny();
    for (int SI = Coupled.find_first(); SI >= 0; SI = Coupled.find_next(SI)) {
      SmallBitVector Group(Pair[SI].Group);
      SmallBitVector Sivs(Pairs);
      SmallBitVector Mivs(Pairs);
      SmallBitVector ConstrainedLevels(MaxLevels + 1);
      for (int SJ = Group.find_first(); SJ >= 0; SJ = Group.find_next(SJ)) {
        if (Pair[SJ].Classification == Subscript::SIV)
          Sivs.set(SJ);
        else
          Mivs.set(SJ);
      }
      while (Sivs.any()) {
        bool Changed = false;
        for (int SJ = Sivs.find_first(); SJ >= 0; SJ = Sivs.find_next(SJ)) {
          // SJ is an SIV subscript that's part of the current coupled group
          unsigned Level;
          const SCEV *SplitIter = nullptr;
          (void) testSIV(Pair[SJ].Src, Pair[SJ].Dst, Level,
                         Result, NewConstraint, SplitIter, false);
          if (Level == SplitLevel && SplitIter)
            return SplitIter;
          ConstrainedLevels.set(Level);
          if (intersectConstraints(&Constraints[Level], &NewConstraint))
            Changed = true;
          Sivs.reset(SJ);
        }
        if (Changed) {
          // propagate, possibly creating new SIVs and ZIVs
          for (int SJ = Mivs.find_first(); SJ >= 0; SJ = Mivs.find_next(SJ)) {
            // SJ is an MIV subscript that's part of the current coupled group
            if (propagate(Pair[SJ].Src, Pair[SJ].Dst,
                          Pair[SJ].Loops, Constraints, Result.Consistent)) {
              Pair[SJ].Classification =
                classifyPair(Pair[SJ].Src, LI->getLoopFor(Src->getParent()),
                             Pair[SJ].Dst, LI->getLoopFor(Dst->getParent()),
                             Pair[SJ].Loops);
              switch (Pair[SJ].Classification) {
              case Subscript::ZIV:
                Mivs.reset(SJ);
                break;
              case Subscript::SIV:
                Sivs.set(SJ);
                Mivs.reset(SJ);
                break;
              case Subscript::RDIV:
              case Subscript::MIV:
                break;
              default:
                llvm_unreachable("bad subscript classification");
              }
            }
          }
        }
      }
    }
  }
	llvm_unreachable("somehow reached end of routine");
		
  return nullptr;
}

#endif
