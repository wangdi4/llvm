//===- DDTests.cpp - Data dependence testing between two DDRefs -*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

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
#if 0
STATISTIC(DeltaApplications, "Delta applications");
STATISTIC(DeltaSuccesses, "Delta successes");
#endif
STATISTIC(GCDapplications, "GCD applications");
STATISTIC(GCDsuccesses, "GCD successes");
STATISTIC(GCDindependence, "GCD independence");
STATISTIC(BanerjeeApplications, "Banerjee applications");
STATISTIC(BanerjeeIndependence, "Banerjee independence");
STATISTIC(BanerjeeSuccesses, "Banerjee successes");
#if 0
STATISTIC(DeltaPropagations, "Delta propagations");
STATISTIC(DeltaIndependence, "Delta independence");
#endif
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

// Returns true if a particular level is scalar; that is,
// if no subscript in the source or destination mention the induction
// variable associated with the loop at this level.
// Leave this out of line, so it will serve as a virtual method anchor
bool Dependences::isScalar(unsigned level) const { return false; }

//===----------------------------------------------------------------------===//
// FullDependence methods

FullDependences::FullDependences(DDRef *Source, DDRef *Destination,
                                 unsigned CommonLevels)
    : Dependences(Source, Destination), Levels(CommonLevels) {
  Consistent = true;
  LoopIndependent = false;
  DV = CommonLevels ? new DVEntry[CommonLevels] : nullptr;
}

FullDependences::~FullDependences() { delete[] DV; }

// The rest are simple getters that hide the implementation.

// getDirection - Returns the direction associated with a particular level.
DVKind FullDependences::getDirection(unsigned Level) const {
  assert(0 < Level && Level <= Levels && "Level out of range");
  return DV[Level - 1].Direction;
}

// Returns the distance (or NULL) associated with a particular level.
const CanonExpr *FullDependences::getDistance(unsigned Level) const {
  assert(0 < Level && Level <= Levels && "Level out of range");
  return DV[Level - 1].Distance;
}

// setDirection - sets DV for  with a particular level.
void FullDependences::setDirection(const unsigned Level,
                                   const DVKind Direction) const {
  assert(0 < Level && Level <= Levels && "Level out of range");
  DV[Level - 1].Direction = Direction;
}

// sets the distance for a particular level.
void FullDependences::setDistance(const unsigned Level,
                                  const CanonExpr *CE) const {
  assert(0 < Level && Level <= Levels && "Level out of range");
  DV[Level - 1].Distance = CE;
}

// Returns true if a particular level is scalar; that is,
// if no subscript in the source or destination mention the induction
// variable associated with the loop at this level.
bool FullDependences::isScalar(unsigned Level) const {
  assert(0 < Level && Level <= Levels && "Level out of range");
  return DV[Level - 1].Scalar;
}

// Returns true if peeling the first iteration from this loop
// will break this dependence.
bool FullDependences::isPeelFirst(unsigned Level) const {
  assert(0 < Level && Level <= Levels && "Level out of range");
  return DV[Level - 1].PeelFirst;
}

// Returns true if peeling the last iteration from this loop
// will break this dependence.
bool FullDependences::isPeelLast(unsigned Level) const {
  assert(0 < Level && Level <= Levels && "Level out of range");
  return DV[Level - 1].PeelLast;
}

// Returns true if splitting this loop will break the dependence.
bool FullDependences::isSplitable(unsigned Level) const {
  assert(0 < Level && Level <= Levels && "Level out of range");
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

  // TODO: note that it could be unsafe to simplify denominator in constant CEs.
  CE2->simplify(false);

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
  assert(CanonExprUtils::isValidLoopLevel(IVNum) && "IVnum not within range");

  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {
    int64_t ConstCoeff = CE->getIVConstCoeff(CurIVPair);
    unsigned BlobIdx = CE->getIVBlobCoeff(CurIVPair);

    DEBUG(dbgs() << "\n\tConst coeff, Blobidx: " << ConstCoeff << " "
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

  CE->simplify(false);

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

  CE->simplify(false);

  push(CE);
  return CE;
}

bool DDTest::areCEEqual(const CanonExpr *CE1, const CanonExpr *CE2,
                        bool RelaxedMode) const {

  if (!CE1 || !CE2) {
    return false;
  }
  return CanonExprUtils::areEqual(CE1, CE2, RelaxedMode);
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

// Current support in Util: one of them must be a constant
const CanonExpr *DDTest::getMulExpr(const CanonExpr *CE1,
                                    const CanonExpr *CE2) {
  int64_t CVal = 0;

  if (!CE1 || !CE2) {
    return nullptr;
  }

  if (CE2->isIntConstant(&CVal)) {
    std::swap(CE1, CE2);
  } else if (!CE1->isIntConstant(&CVal)) {
    return nullptr;
  }

  CanonExpr *CE = CE2->clone();
  push(CE);
  if (!CE->multiplyByConstant(CVal)) {
    return nullptr;
  }

  return CE;
}

const CanonExpr *DDTest::getConstantfromAPInt(Type *Ty, APInt Value) {
  CanonExpr *CE = HNU.getCanonExprUtils().createCanonExpr(Ty, Value);
  push(CE);
  return CE;
}

const CanonExpr *DDTest::getConstantWithType(Type *SrcTy, Type *DestTy,
                                             bool IsSExt, int64_t Val) {
  CanonExpr *CE = HNU.getCanonExprUtils().createExtCanonExpr(SrcTy, DestTy,
                                                             IsSExt, 0, Val, 1);
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
  const CanonExpr *CE = getConstantWithType(
      CE1->getSrcType(), CE1->getDestType(), CE1->isSExt(), CVal1 / CVal2);

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

#if 0

// Updates X with the intersection
// of the Constraints X and Y. Returns true if X has changed.
// Corresponds to Figure 4 from the paper
//
//            Practical Dependence Testing
//            Goff, Kennedy, Tseng
//            PLDI 1991
bool DependenceAnalysis::intersectConstraints(Constraint *X,
                                              const Constraint *Y) {
  ++DeltaApplications;
  DEBUG(dbgs() << "\\nintersect constraints\n");
  DEBUG(dbgs() << "\n    X ="; X->dump(dbgs()));
  DEBUG(dbgs() << "\n    Y ="; Y->dump(dbgs()));
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
    DEBUG(dbgs() << "\n    intersect 2 distances\n");
    if (isKnownPredicate(CmpInst::ICMP_EQ, X->getD(), Y->getD()))
      return false;
    if (isKnownPredicate(CmpInst::ICMP_NE, X->getD(), Y->getD())) {
      X->setEmpty();
      ++DeltaSuccesses;
      return true;
    }
    // Hmmm, interesting situation.
    // I guess if either is constant, keep it and ignore the other.
    if (isa<SCEVConstant>(Y->getD())) {
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
    DEBUG(dbgs() << "\n    intersect 2 lines\n");
    const SCEV *Prod1 = SE->getMulExpr(X->getA(), Y->getB());
    const SCEV *Prod2 = SE->getMulExpr(X->getB(), Y->getA());
    if (isKnownPredicate(CmpInst::ICMP_EQ, Prod1, Prod2)) {
      // slopes are equal, so lines are parallel
      DEBUG(dbgs() << "\t\tsame slope\n");
      Prod1 = SE->getMulExpr(X->getC(), Y->getB());
      Prod2 = SE->getMulExpr(X->getB(), Y->getC());
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
      DEBUG(dbgs() << "\t\tdifferent slopes\n");
      const SCEV *C1B2 = SE->getMulExpr(X->getC(), Y->getB());
      const SCEV *C1A2 = SE->getMulExpr(X->getC(), Y->getA());
      const SCEV *C2B1 = SE->getMulExpr(Y->getC(), X->getB());
      const SCEV *C2A1 = SE->getMulExpr(Y->getC(), X->getA());
      const SCEV *A1B2 = SE->getMulExpr(X->getA(), Y->getB());
      const SCEV *A2B1 = SE->getMulExpr(Y->getA(), X->getB());
      const SCEVConstant *C1A2_C2A1 =
        dyn_cast<SCEVConstant>(SE->getMinusSCEV(C1A2, C2A1));
      const SCEVConstant *C1B2_C2B1 =
        dyn_cast<SCEVConstant>(SE->getMinusSCEV(C1B2, C2B1));
      const SCEVConstant *A1B2_A2B1 =
        dyn_cast<SCEVConstant>(SE->getMinusSCEV(A1B2, A2B1));
      const SCEVConstant *A2B1_A1B2 =
        dyn_cast<SCEVConstant>(SE->getMinusSCEV(A2B1, A1B2));
      if (!C1B2_C2B1 || !C1A2_C2A1 ||
          !A1B2_A2B1 || !A2B1_A1B2)
        return false;
      APInt Xtop = C1B2_C2B1->getValue()->getValue();
      APInt Xbot = A1B2_A2B1->getValue()->getValue();
      APInt Ytop = C1A2_C2A1->getValue()->getValue();
      APInt Ybot = A2B1_A1B2->getValue()->getValue();
      DEBUG(dbgs() << "\t\tXtop = " << Xtop << "\n");
      DEBUG(dbgs() << "\t\tXbot = " << Xbot << "\n");
      DEBUG(dbgs() << "\t\tYtop = " << Ytop << "\n");
      DEBUG(dbgs() << "\t\tYbot = " << Ybot << "\n");
      APInt Xq = Xtop; // these need to be initialized, even
      APInt Xr = Xtop; // though they're just going to be overwritten
      APInt::sdivrem(Xtop, Xbot, Xq, Xr);
      APInt Yq = Ytop;
      APInt Yr = Ytop;
      APInt::sdivrem(Ytop, Ybot, Yq, Yr);
      if (Xr != 0 || Yr != 0) {
        X->setEmpty();
        ++DeltaSuccesses;
        return true;
      }
      DEBUG(dbgs() << "\t\tX = " << Xq << ", Y = " << Yq << "\n");
      if (Xq.slt(0) || Yq.slt(0)) {
        X->setEmpty();
        ++DeltaSuccesses;
        return true;
      }
      if (const SCEVConstant *CUB =
          collectConstantUpperBound(X->getAssociatedLoop(), Prod1->getType())) {
        APInt UpperBound = CUB->getValue()->getValue();
        DEBUG(dbgs() << "\t\tupper bound = " << UpperBound << "\n");
        if (Xq.sgt(UpperBound) || Yq.sgt(UpperBound)) {
          X->setEmpty();
          ++DeltaSuccesses;
          return true;
        }
      }
      X->setPoint(SE->getConstant(Xq),
                  SE->getConstant(Yq),
                  X->getAssociatedLoop());
      ++DeltaSuccesses;
      return true;
    }
    return false;
  }

  // if (X->isLine() && Y->isPoint()) This case can't occur.
  assert(!(X->isLine() && Y->isPoint()) && "This case should never occur");

  if (X->isPoint() && Y->isLine()) {
    DEBUG(dbgs() << "\t    intersect Point and Line\n");
    const SCEV *A1X1 = SE->getMulExpr(Y->getA(), X->getX());
    const SCEV *B1Y1 = SE->getMulExpr(Y->getB(), X->getY());
    const SCEV *Sum = SE->getAddExpr(A1X1, B1Y1);
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
#endif

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

#if 0	

static
AAResults::AliasResult underlyingObjectsAlias(AAResults *AA,
                                                  const Value *A,
                                                  const Value *B) {
  const Value *AObj = GetUnderlyingObject(A);
  const Value *BObj = GetUnderlyingObject(B);
  return AA->alias(AObj, AA->getTypeStoreSize(AObj->getType()),
                   BObj, AA->getTypeStoreSize(BObj->getType()));
}


// Returns true if the load or store can be analyzed. Atomic and volatile
// operations have properties which this analysis does not understand.
static
bool isLoadOrStore(const Instruction *I) {
  if (const LoadInst *LI = dyn_cast<LoadInst>(I))
    return LI->isUnordered();
  else if (const StoreInst *SI = dyn_cast<StoreInst>(I))
    return SI->isUnordered();
  return false;
}


static
Value *getPointerOperand(Instruction *I) {
  if (LoadInst *LI = dyn_cast<LoadInst>(I))
    return LI->getPointerOperand();
  if (StoreInst *SI = dyn_cast<StoreInst>(I))
    return SI->getPointerOperand();
  llvm_unreachable("Value is not load or store instruction");
  return nullptr;
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
                                    const DDRef *DstDDRef) {

  HLDDNode *SrcDDNode = SrcDDRef->getHLDDNode();
  HLDDNode *DstDDNode = DstDDRef->getHLDDNode();

  HLNode *SrcHIR = dyn_cast<HLNode>(SrcDDNode);
  assert(SrcHIR && "HIR not found for Src DDRef");
  HLNode *DstHIR = dyn_cast<HLNode>(DstDDNode);
  assert(DstHIR && "HIR not found for Dst DDRef");
  HLLoop *SrcParent = SrcHIR->getParentLoop();
  HLLoop *DstParent = DstHIR->getParentLoop();
  HLLoop *SrcLoop = nullptr;
  HLLoop *DstLoop = nullptr;
  unsigned SrcLevel = 0;
  unsigned DstLevel = 0;

  DeepestLoop = nullptr;

  if (!SrcParent) {
    CommonLevels = 0;
    if (!DstParent) {
      MaxLevels = 0;
      return;
    }
    DstLoop = DstParent;
    DstLevel = DstLoop->getNestingLevel();
    MaxLevels = DstLevel;
  } else if (!DstParent) {
    CommonLevels = 0;
    SrcLoop = SrcParent;
    SrcLevel = SrcLoop->getNestingLevel();
    MaxLevels = SrcLevel;
    return;
  }

  SrcLoop = SrcParent;
  DstLoop = DstParent;

  if (SrcLoop) {
    SrcLevel = SrcLoop->getNestingLevel();
  }

  DstLevel = DstLoop->getNestingLevel();
  DeepestLoop = (SrcLevel > DstLevel) ? SrcParent : DstParent;

  // TODO: Need to understand how is MaxLevels consumed

  SrcLevels = SrcLevel;
  MaxLevels = SrcLevel + DstLevel;
  while (SrcLevel > DstLevel) {
    SrcLoop = SrcLoop->getParentLoop();
    SrcLevel--;
  }
  while (DstLevel > SrcLevel) {
    DstLoop = DstLoop->getParentLoop();
    DstLevel--;
  }
  while (SrcLoop != DstLoop) {
    SrcLoop = SrcLoop->getParentLoop();
    DstLoop = DstLoop->getParentLoop();
    SrcLevel--;
  }

  CommonLevels = SrcLevel;
  MaxLevels -= CommonLevels;
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
    const SCEVCastExpr *SrcCast = cast<SCEVCastExpr>(Src);
    const SCEVCastExpr *DstCast = cast<SCEVCastExpr>(Dst);
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

// A wrapper around for dealing special cases of predicates
// Looks for cases where we're interested in comparing for equality.

bool DDTest::isKnownPredicate(ICmpInst::Predicate Pred, const CanonExpr *X,
                              const CanonExpr *Y) {

  const CanonExpr *Delta = getMinus(X, Y);
  if (!Delta) {
    return false;
  }
  switch (Pred) {
  case CmpInst::ICMP_EQ:
    return Delta->isZero();
  case CmpInst::ICMP_NE:
    return HNU.isKnownNonZero(Delta, DeepestLoop);
  case CmpInst::ICMP_SGE:
    return HNU.isKnownNonNegative(Delta, DeepestLoop);
  case CmpInst::ICMP_SLE:
    return HNU.isKnownNonPositive(Delta, DeepestLoop);
  case CmpInst::ICMP_SGT:
    return HNU.isKnownPositive(Delta, DeepestLoop);
  case CmpInst::ICMP_SLT:
    return HNU.isKnownNegative(Delta, DeepestLoop);
  default:
    llvm_unreachable("unexpected predicate in isKnownPredicate");
  }
}

// All subscripts are all the same type.
// Loop bound may be smaller (e.g., a char).
// Should zero extend loop bound, since it's always >= 0.
// This routine collects upper bound and extends if needed.
// Return null if no bound available.
const CanonExpr *DDTest::collectUpperBound(const HLLoop *L, Type *T) const {
  const CanonExpr *UpperBound = L->getUpperCanonExpr();
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
                     FullDependences &Result) {

  DEBUG(dbgs() << "\n    src = "; Src->dump());
  DEBUG(dbgs() << "\n    dst = "; Dst->dump());

  ++ZIVapplications;
  if (isKnownPredicate(CmpInst::ICMP_EQ, Src, Dst)) {
    DEBUG(dbgs() << "\n    provably dependent");
    return false; // provably dependent
  }
  if (isKnownPredicate(CmpInst::ICMP_NE, Src, Dst)) {
    DEBUG(dbgs() << "\n    provably independent");
    ++ZIVindependence;
    return true; // provably independent
  }
  DEBUG(dbgs() << "\n    possibly dependent\n");
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
                           unsigned Level, FullDependences &Result,
                           Constraint &NewConstraint) {
  DEBUG(dbgs() << "\nStrong SIV test\n");
  DEBUG(dbgs() << "\n    Coeff = "; Coeff->dump());
  DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());
  ++StrongSIVapplications;
  assert(0 < Level && Level <= CommonLevels && "level out of range");
  Level--;

  const CanonExpr *Delta = getMinus(SrcConst, DstConst);

  if (!Delta) {
    return false;
  }

  DEBUG(dbgs() << "\n    Delta = "; Delta->dump());

  // check that |Delta| < iteration count
  // TBD: get UB for CurLoop

  if (const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr()) {

    // There is a bug with the original code
    // for a[2 *n ]  n is SI64  in source.
    // Coeff in SCEV is  2*n  I64
    // SE->isKnowNonNegative(Coeff) return true but n could be nagative
    // w/o knowing what compose the whole subscript
    // Our implementation  using canon will return false for 2*n
    // But GCD test should get Indep for 2*n vs 2*n+1

    const CanonExpr *AbsDelta =
        HNU.isKnownNonNegative(Delta) ? Delta : getNegative(Delta);
    const CanonExpr *AbsCoeff =
        HNU.isKnownNonNegative(Coeff) ? Coeff : getNegative(Coeff);
    const CanonExpr *Product = getMulExpr(UpperBound, AbsCoeff);

    DEBUG(dbgs() << "\n    UpperBound = "; UpperBound->dump());
    DEBUG(dbgs() << "\n    AbsDelta = "; AbsDelta->dump());
    DEBUG(dbgs() << "\n    AbsCoeff = "; AbsCoeff->dump());

    // If Delta is zero and coeff is  non-zero then set
    // dv as =
    // e.g.  2*n*i  is non-zero if n is non-zero

    if (Delta->isZero() && HNU.isKnownNonZero(Coeff, CurLoop)) {

      Result.DV[Level].Distance = Delta;
      NewConstraint.setDistance(Delta, CurLoop);
      DEBUG(dbgs() << "\n\t DV set as EQ - 1\n");
      Result.DV[Level].Direction &= DVKind::EQ;
    }

    if (Product && isKnownPredicate(CmpInst::ICMP_SGT, AbsDelta, Product)) {
      DEBUG(dbgs() << "\n    Product = "; Product->dump());
      // Distance greater than trip count - no dependence
      ++StrongSIVindependence;
      ++StrongSIVsuccesses;
      DEBUG(dbgs() << "\n StrongSIV finds independence 1\n");
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
      DEBUG(dbgs() << "\n    Distance = " << Distance << "\n");
      DEBUG(dbgs() << "\n    Remainder = " << Remainder << "\n");
      // Make sure Coeff divides Delta exactly
      if (Remainder != 0) {
        // Coeff doesn't divide Distance, no dependence
        ++StrongSIVindependence;
        ++StrongSIVsuccesses;
        DEBUG(dbgs() << "\n StrongSIV finds independence 2\n");
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
      DEBUG(dbgs() << "\n\t DV set as EQ -2 \n");
      Result.DV[Level].Direction &= DVKind::EQ;
      ++StrongSIVsuccesses;
    } else {
      if (Coeff->isOne()) {
        DEBUG(dbgs() << "\n    Distance = "; Delta->dump());
        Result.DV[Level].Distance = Delta; // since X/1 == X
        NewConstraint.setDistance(Delta, CurLoop);
      } else {
        Result.Consistent = false;
        NewConstraint.setLine(Coeff, getNegative(Coeff), getNegative(Delta),
                              CurLoop);
      }

      // maybe we can get a useful direction
      bool DeltaMaybeZero = !(HNU.isKnownNonZero(Delta));
      bool DeltaMaybePositive = !(HNU.isKnownNonPositive(Delta));
      bool DeltaMaybeNegative = !(HNU.isKnownNonNegative(Delta));
      bool CoeffMaybePositive = !(HNU.isKnownNonPositive(Coeff));
      bool CoeffMaybeNegative = !(HNU.isKnownNonNegative(Coeff));
      // The double negatives above are confusing.
      // It helps to read isKnownNonZero(Delta)
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

  DEBUG(dbgs() << "\n StrongSIV finds dependence\n");
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
                                 FullDependences &Result,
                                 Constraint &NewConstraint,
                                 const CanonExpr *&SplitIter) {
  DEBUG(dbgs() << "\tWeak-Crossing SIV test\n");
  DEBUG(dbgs() << "\n    Coeff = "; Coeff->dump());
  DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());

  ++WeakCrossingSIVapplications;
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  Level--;
  Result.Consistent = false;
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);
  if (!Delta) {
    return false;
  }

  DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
  NewConstraint.setLine(Coeff, Coeff, Delta, CurLoop);
  if (Delta->isZero()) {
    Result.DV[Level].Direction &= ~DVKind::LT;
    Result.DV[Level].Direction &= ~DVKind::GT;
    ++WeakCrossingSIVsuccesses;
    if (!Result.DV[Level].Direction) {
      ++WeakCrossingSIVindependence;
      DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-0!");
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
  if (HNU.isKnownNegative(ConstCoeff, CurLoop)) {
    ConstCoeff = getNegative(ConstCoeff);
    assert(ConstCoeff &&
           "dynamic cast of negative of ConstCoeff should yield constant");
    Delta = getNegative(Delta);
  }
  assert(HNU.isKnownPositive(ConstCoeff) && "ConstCoeff should be positive");

  // compute SplitIter for use by DependenceAnalysis::getSplitIteration()

  const CanonExpr *MaxResult =
      getSMaxExpr(getConstantWithType(Delta->getSrcType(), Delta->getDestType(),
                                      Delta->isSExt(), 0),
                  Delta);

  if (MaxResult == nullptr) {
    DEBUG(dbgs() << "\nNeed more support for Max!");
    return false;
  }

  SplitIter = getUDivExpr(
      MaxResult,
      getMulExpr(getConstantWithType(Delta->getSrcType(), Delta->getDestType(),
                                     Delta->isSExt(), 2),
                 ConstCoeff));

  if (SplitIter == nullptr) {
    DEBUG(dbgs() << "\nNeed more support for Divide!");
    return false;
  }

  DEBUG(dbgs() << "\n    Split iter = "; SplitIter->dump());

  const CanonExpr *ConstantDelta = Delta;
  int64_t DeltaValue;
  if (!(ConstantDelta->isIntConstant(&DeltaValue))) {
    return false;
  }

  // We're certain that ConstCoeff > 0; therefore,
  // if Delta < 0, then no dependence.
  DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
  DEBUG(dbgs() << "\n    ConstCoeff = "; ConstCoeff->dump());

  if (HNU.isKnownNegative(Delta, CurLoop)) {
    // No dependence, Delta < 0s
    ++WeakCrossingSIVindependence;
    ++WeakCrossingSIVsuccesses;
    DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-1!");
    return true;
  }

  // We're certain that Delta > 0 and ConstCoeff > 0.
  // Check Delta/(2*ConstCoeff) against upper loop bound
  if (const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr()) {
    DEBUG(dbgs() << "\n    UpperBound = "; UpperBound->dump());
    const CanonExpr *ConstantTwo =
        getConstantWithType(UpperBound->getSrcType(), UpperBound->getDestType(),
                            UpperBound->isSExt(), 2);
    const CanonExpr *ML =
        getMulExpr(getMulExpr(ConstCoeff, UpperBound), ConstantTwo);

    if (!ML) {
      return false;
    }

    DEBUG(dbgs() << "\n    ML = "; ML->dump());
    if (isKnownPredicate(CmpInst::ICMP_SGT, Delta, ML)) {
      // Delta too big, no dependence
      ++WeakCrossingSIVindependence;
      ++WeakCrossingSIVsuccesses;
      DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-2!");
      return true;
    }
    if (isKnownPredicate(CmpInst::ICMP_EQ, Delta, ML)) {
      // i = i' = UB
      Result.DV[Level].Direction &= ~DVKind::LT;
      Result.DV[Level].Direction &= ~DVKind::GT;
      ++WeakCrossingSIVsuccesses;
      if (!Result.DV[Level].Direction) {
        ++WeakCrossingSIVindependence;
        DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-3!");
        return true;
      }
      Result.DV[Level].Splitable = false;
      Result.DV[Level].Distance = getConstantWithType(
          Delta->getSrcType(), Delta->getDestType(), Delta->isSExt(), 0);
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
  DEBUG(dbgs() << "\n    Remainder = " << Remainder << "\n");
  if (Remainder != 0) {
    // Coeff doesn't divide Delta, no dependence
    ++WeakCrossingSIVindependence;
    ++WeakCrossingSIVsuccesses;
    DEBUG(dbgs() << "\n\tWeakCrossingSIV INDEP-4!");
    return true;
  }
  DEBUG(dbgs() << "\n    Distance = " << Distance << "\n");

  // if 2*Coeff doesn't divide Delta, then the equal direction isn't possible
  APInt Two = APInt(Distance.getBitWidth(), 2, true);
  Remainder = Distance.srem(Two);
  DEBUG(dbgs() << "\n    Remainder = " << Remainder << "\n");
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
  DEBUG(dbgs() << "\t    GCD = " << G << "\n");
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
                          FullDependences &Result, Constraint &NewConstraint) {

  DEBUG(dbgs() << "\nExact SIV test\n");
  DEBUG(dbgs() << "\n    SrcCoeff = "; SrcCoeff->dump());
  DEBUG(dbgs() << " = AM\n");
  DEBUG(dbgs() << "\n    DstCoeff = "; DstCoeff->dump());
  DEBUG(dbgs() << " = BM\n");
  DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  DEBUG(dbgs() << "\n");
  DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());
  DEBUG(dbgs() << "\n");

  ++ExactSIVapplications;
  assert(0 < Level && Level <= CommonLevels && "Level out of range");
  Level--;
  Result.Consistent = false;
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);

  if (!Delta) {
    return false;
  }
  DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
  DEBUG(dbgs() << "\n");

  NewConstraint.setLine(SrcCoeff, getNegative(DstCoeff), Delta, CurLoop);

  int64_t ConstDeltaVal, SrcCoeffVal, DstCoeffVal, UBVal;
  if (!(Delta->isIntConstant(&ConstDeltaVal)) ||
      !(SrcCoeff->isIntConstant(&SrcCoeffVal)) ||
      !(DstCoeff->isIntConstant(&DstCoeffVal))) {
    return false;
  }

  DEBUG(dbgs() << "\tDelta, SrcCoeff, DstCoeff = " << ConstDeltaVal << ","
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

  DEBUG(dbgs() << "\t    X = " << X << ", Y = " << Y << ", G = " << G << "\n");
  //  Normalized CE implies LM = 0

  APInt UM(Bits, 1, true);
  bool UMValid = false;

  // UM is perhaps unavailable, let's check
  if (const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr()) {
    if (UpperBound->isIntConstant(&UBVal)) {
      UM = llvm::APInt(64, UBVal, true);
      DEBUG(dbgs() << "\t    UM = " << UM << "\n");
      UMValid = true;
    }
  }

  APInt TU(APInt::getSignedMaxValue(Bits));
  APInt TL(APInt::getSignedMinValue(Bits));

  // test(BM/G, LM-X) and test(-BM/G, X-UM)
  APInt TMUL = BM.sdiv(G);
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(-X, TMUL));
    DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    if (UMValid) {
      TU = minAPInt(TU, floorOfQuotient(UM - X, TMUL));
      DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    }
  } else {
    TU = minAPInt(TU, floorOfQuotient(-X, TMUL));
    DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    if (UMValid) {
      TL = maxAPInt(TL, ceilingOfQuotient(UM - X, TMUL));
      DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    }
  }

  // test(AM/G, LM-Y) and test(-AM/G, Y-UM)
  TMUL = AM.sdiv(G);
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(-Y, TMUL));
    DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    if (UMValid) {
      TU = minAPInt(TU, floorOfQuotient(UM - Y, TMUL));
      DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    }
  } else {
    TU = minAPInt(TU, floorOfQuotient(-Y, TMUL));
    DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    if (UMValid) {
      TL = maxAPInt(TL, ceilingOfQuotient(UM - Y, TMUL));
      DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    }
  }
  if (TL.sgt(TU)) {
    ++ExactSIVindependence;
    ++ExactSIVsuccesses;
    return true;
  }

  // explore directions
  DVKind NewDirection = DVKind::NONE;

  // less than
  APInt SaveTU(TU); // save these
  APInt SaveTL(TL);
  DEBUG(dbgs() << "\t    exploring LT direction\n");
  TMUL = AM - BM;
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(X - Y + 1, TMUL));
    DEBUG(dbgs() << "\t\t    TL = " << TL << "\n");
  } else {
    TU = minAPInt(TU, floorOfQuotient(X - Y + 1, TMUL));
    DEBUG(dbgs() << "\t\t    TU = " << TU << "\n");
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
  //   DEBUG(dbgs() << "\t\t    TL = " << TL << "\n");
  //  } else {
  //    TU = minAPInt(TU, floorOfQuotient(X - Y, TMUL));
  //   DEBUG(dbgs() << "\t\t    TU = " << TU << "\n");
  // }
  // TMUL = BM - AM;
  // if (TMUL.sgt(0)) {
  //   TL = maxAPInt(TL, ceilingOfQuotient(Y - X, TMUL));
  //   DEBUG(dbgs() << "\t\t    TL = " << TL << "\n");
  // } else {
  //   TU = minAPInt(TU, floorOfQuotient(Y - X, TMUL));
  //   DEBUG(dbgs() << "\t\t    TU = " << TU << "\n");
  // }
  // if (TL.sle(TU)) {
  //  NewDirection |= DVKind::EQ;
  //  ++ExactSIVsuccesses;
  // }

  // Algorithm revised:
  // For tesing (=) it can be done simply as follows.
  // IV  = diff_of_the_constants / diff_of_coeffs
  // Check if IV is within this range:  [0, Upperbound]

  DEBUG(dbgs() << "\t    exploring EQ direction\n");

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
  DEBUG(dbgs() << "\t    exploring GT direction\n");
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(Y - X + 1, TMUL));
    DEBUG(dbgs() << "\t\t    TL = " << TL << "\n");
  } else {
    TU = minAPInt(TU, floorOfQuotient(Y - X + 1, TMUL));
    DEBUG(dbgs() << "\t\t    TU = " << TU << "\n");
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
                                FullDependences &Result,
                                Constraint &NewConstraint) {
  // For the WeakSIV test, it's possible the loop isn't common to
  // the Src and Dst loops. If it isn't, then there's no need to
  // record a direction.
  DEBUG(dbgs() << "\nWeak-Zero (src) SIV test\n");
  DEBUG(dbgs() << "\n    DstCoeff = "; DstCoeff->dump());
  DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());

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
  NewConstraint.setLine(getConstantWithType(Delta->getSrcType(),
                                            Delta->getDestType(),
                                            Delta->isSExt(), 0),
                        DstCoeff, Delta, CurLoop);

  DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
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

  const CanonExpr *AbsCoeff = HNU.isKnownNegative(ConstCoeff, CurLoop)
                                  ? getNegative(ConstCoeff)
                                  : ConstCoeff;
  const CanonExpr *NewDelta =
      HNU.isKnownNegative(ConstCoeff, CurLoop) ? getNegative(Delta) : Delta;

  // check that Delta/SrcCoeff < iteration count
  // really check NewDelta < count*AbsCoeff
  if (const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr()) {
    DEBUG(dbgs() << "\n    UpperBound = "; UpperBound->dump());

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
  if (HNU.isKnownNegative(NewDelta, CurLoop)) {
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
                                FullDependences &Result,
                                Constraint &NewConstraint) {
  // For the WeakSIV test, it's possible the loop isn't common to the
  // Src and Dst loops. If it isn't, then there's no need to record a direction.
  DEBUG(dbgs() << "\nWeak-Zero (dst) SIV test\n");
  DEBUG(dbgs() << "\nSrcCoeff = "; SrcCoeff->dump());
  DEBUG(dbgs() << "\nSrcConst = "; SrcConst->dump());
  DEBUG(dbgs() << "\nDstConst = "; DstConst->dump());

  ++WeakZeroSIVapplications;
  assert(0 < Level && Level <= SrcLevels && "Level out of range");
  Level--;
  Result.Consistent = false;
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);

  if (!Delta) {
    return false;
  }
  NewConstraint.setLine(SrcCoeff, getConstantWithType(Delta->getSrcType(),
                                                      Delta->getDestType(),
                                                      Delta->isSExt(), 0),
                        Delta, CurLoop);
  DEBUG(dbgs() << "\n    Delta = "; Delta->dump());
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

  const CanonExpr *AbsCoeff = HNU.isKnownNegative(ConstCoeff, CurLoop)
                                  ? getNegative(ConstCoeff)
                                  : ConstCoeff;
  const CanonExpr *NewDelta =
      HNU.isKnownNegative(ConstCoeff, CurLoop) ? getNegative(Delta) : Delta;

  // check that Delta/SrcCoeff < iteration count
  // really check NewDelta < count*AbsCoeff
  if (const CanonExpr *UpperBound = CurLoop->getUpperCanonExpr()) {
    DEBUG(dbgs() << "\n    UpperBound = "; UpperBound->dump());
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
  if (HNU.isKnownNegative(NewDelta, CurLoop)) {
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
                           FullDependences &Result) {

  DEBUG(dbgs() << "\nExact RDIV test\n");
  DEBUG(dbgs() << "\n    SrcCoeff = "; SrcCoeff->dump());
  DEBUG(dbgs() << " = AM\n");
  DEBUG(dbgs() << "\n    DstCoeff = "; DstCoeff->dump());
  DEBUG(dbgs() << " = BM\n");
  DEBUG(dbgs() << "\n    SrcConst = "; SrcConst->dump());
  DEBUG(dbgs() << "\n    DstConst = "; DstConst->dump());
  ++ExactRDIVapplications;
  Result.Consistent = false;
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);

  if (!Delta) {
    return false;
  }
  DEBUG(dbgs() << "\n    Delta = "; Delta->dump());

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

  DEBUG(dbgs() << "\n    X = " << X << ", Y = " << Y << ", G = " << G << "\n");

  // since CE construction is normalized, LM = 0
  APInt SrcUM(Bits, 1, true);
  bool SrcUMvalid = false;
  // SrcUM is perhaps unavailable, let's check

  if (const CanonExpr *UpperBound = SrcLoop->getUpperCanonExpr()) {
    if (UpperBound->isIntConstant(&ubVal)) {
      SrcUM = llvm::APInt(64, ubVal, true);
      DEBUG(dbgs() << "\t    SrcUM = " << SrcUM << "\n");
      SrcUMvalid = true;
    }
  }

  APInt DstUM(Bits, 1, true);
  bool DstUMvalid = false;

  // UM is perhaps unavailable, let's check
  if (const CanonExpr *UpperBound = DstLoop->getUpperCanonExpr()) {
    if (UpperBound->isIntConstant(&ubVal)) {
      DstUM = llvm::APInt(64, ubVal, true);
      DEBUG(dbgs() << "\t    DstUM = " << DstUM << "\n");
      DstUMvalid = true;
    }
  }

  APInt TU(APInt::getSignedMaxValue(Bits));
  APInt TL(APInt::getSignedMinValue(Bits));

  // test(BM/G, LM-X) and test(-BM/G, X-UM)
  APInt TMUL = BM.sdiv(G);
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(-X, TMUL));
    DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    if (SrcUMvalid) {
      TU = minAPInt(TU, floorOfQuotient(SrcUM - X, TMUL));
      DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    }
  } else {
    TU = minAPInt(TU, floorOfQuotient(-X, TMUL));
    DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    if (SrcUMvalid) {
      TL = maxAPInt(TL, ceilingOfQuotient(SrcUM - X, TMUL));
      DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    }
  }

  // test(AM/G, LM-Y) and test(-AM/G, Y-UM)
  TMUL = AM.sdiv(G);
  if (TMUL.sgt(0)) {
    TL = maxAPInt(TL, ceilingOfQuotient(-Y, TMUL));
    DEBUG(dbgs() << "\t    TL = " << TL << "\n");
    if (DstUMvalid) {
      TU = minAPInt(TU, floorOfQuotient(DstUM - Y, TMUL));
      DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    }
  } else {
    TU = minAPInt(TU, floorOfQuotient(-Y, TMUL));
    DEBUG(dbgs() << "\t    TU = " << TU << "\n");
    if (DstUMvalid) {
      TL = maxAPInt(TL, ceilingOfQuotient(DstUM - Y, TMUL));
      DEBUG(dbgs() << "\t    TL = " << TL << "\n");
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

  DEBUG(dbgs() << "\ntry symbolic RDIV test\n");
  DEBUG(dbgs() << "\n    A1 = "; A1->dump());
  DEBUG(dbgs() << ", src type = " << *(A1->getSrcType()));
  DEBUG(dbgs() << ", dest type = " << *(A1->getDestType()) << "\n");
  DEBUG(dbgs() << "\n    A2 = "; A2->dump());
  DEBUG(dbgs() << "\n    C1 = "; C1->dump());
  DEBUG(dbgs() << "\n    C2 = "; C2->dump());

  const CanonExpr *N1 = Loop1->getUpperCanonExpr();
  const CanonExpr *N2 = Loop2->getUpperCanonExpr();

  if (N1) {
    DEBUG(dbgs() << "\n    N1 = "; N1->dump());
  }
  if (N2) {
    DEBUG(dbgs() << "\n    N2 = "; N2->dump());
  }
  const CanonExpr *C2_C1 = getMinus(C2, C1);
  if (!C2_C1) {
    return false;
  }
  const CanonExpr *C1_C2 = getNegative(C2_C1);

  DEBUG(dbgs() << "\n    C2 - C1 = "; C2_C1->dump());
  DEBUG(dbgs() << "\n    C1 - C2 = "; C1_C2->dump());

  if (HNU.isKnownNonNegative(A1, DeepestLoop)) {
    if (HNU.isKnownNonNegative(A2, DeepestLoop)) {
      // A1 >= 0 && A2 >= 0
      if (N1) {
        // make sure that c2 - c1 <= a1*N1
        const CanonExpr *A1N1 = getMulExpr(A1, N1);
        if (A1N1 == nullptr) {
          return false;
        }

        DEBUG(dbgs() << "\n    A1*N1 = "; A1N1->dump());
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

        DEBUG(dbgs() << "\n    A2*N2 = "; A2N2->dump());
        if (isKnownPredicate(CmpInst::ICMP_SLT, A2N2, C1_C2)) {
          ++SymbolicRDIVindependence;
          return true;
        }
      }
    } else if (HNU.isKnownNonPositive(A2, DeepestLoop)) {
      // a1 >= 0 && a2 <= 0
      if (N1 && N2) {
        // make sure that c2 - c1 <= a1*N1 - a2*N2
        const CanonExpr *A1N1 = getMulExpr(A1, N1);
        const CanonExpr *A2N2 = getMulExpr(A2, N2);
        const CanonExpr *A1N1_A2N2 = getMinus(A1N1, A2N2);

        if (A1N1 == nullptr || A2N2 == nullptr || A1N1_A2N2 == nullptr) {
          return false;
        }

        DEBUG(dbgs() << "\n A1*N1 - A2*N2 = "; A1N1_A2N2->dump());
        if (isKnownPredicate(CmpInst::ICMP_SGT, C2_C1, A1N1_A2N2)) {
          ++SymbolicRDIVindependence;
          return true;
        }
      }
      // make sure that 0 <= c2 - c1
      if (HNU.isKnownNegative(C2_C1, DeepestLoop)) {
        ++SymbolicRDIVindependence;
        return true;
      }
    }
  } else if (HNU.isKnownNonPositive(A1, DeepestLoop)) {
    if (HNU.isKnownNonNegative(A2, DeepestLoop)) {
      // a1 <= 0 && a2 >= 0
      if (N1 && N2) {
        // make sure that a1*N1 - a2*N2 <= c2 - c1
        const CanonExpr *A1N1 = getMulExpr(A1, N1);
        const CanonExpr *A2N2 = getMulExpr(A2, N2);
        const CanonExpr *A1N1_A2N2 = getMinus(A1N1, A2N2);

        if (A1N1 == nullptr || A2N2 == nullptr || A1N1_A2N2 == nullptr) {
          return false;
        }

        DEBUG(dbgs() << "\n A1*N1 - A2*N2 = "; A1N1_A2N2->dump());
        if (isKnownPredicate(CmpInst::ICMP_SGT, A1N1_A2N2, C2_C1)) {
          ++SymbolicRDIVindependence;
          return true;
        }
      }
      // make sure that c2 - c1 <= 0
      if (HNU.isKnownPositive(C2_C1, DeepestLoop)) {
        ++SymbolicRDIVindependence;
        return true;
      }
    } else if (HNU.isKnownNonPositive(A2, DeepestLoop)) {
      // a1 <= 0 && a2 <= 0
      if (N1) {
        // make sure that a1*N1 <= c2 - c1
        const CanonExpr *A1N1 = getMulExpr(A1, N1);
        if (!A1N1) {
          return false;
        }

        DEBUG(dbgs() << "\n A1*N1 = "; A1N1->dump());
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

        DEBUG(dbgs() << "\t    A2*N2 = "; A2N2->dump());
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
                     unsigned &Level, FullDependences &Result,
                     Constraint &NewConstraint, const CanonExpr *&SplitIter,
                     const HLLoop *SrcParentLoop, const HLLoop *DstParentLoop) {

  DEBUG(dbgs() << "\n Test SIV \n");
  DEBUG(dbgs() << "\n   src = "; Src->dump());
  DEBUG(dbgs() << "\n");
  DEBUG(dbgs() << "   dst = "; Dst->dump());
  DEBUG(dbgs() << "\n");

  const HLLoop *SrcLoop = getLoop(Src, SrcParentLoop);
  const HLLoop *DstLoop = getLoop(Dst, DstParentLoop);

  if (SrcLoop && DstLoop) {
    const CanonExpr *SrcConst = getInvariant(Src);
    const CanonExpr *DstConst = getInvariant(Dst);
    const CanonExpr *SrcCoeff = getCoeff(Src);
    const CanonExpr *DstCoeff = getCoeff(Dst);
    const HLLoop *CurLoop = SrcLoop;
    assert(SrcLoop == DstLoop && "both loops in SIV should be same");
    Level = mapSrcLoop(CurLoop);
    bool Disproven;

    if (areCEEqual(SrcCoeff, DstCoeff)) {
      Disproven = strongSIVtest(SrcCoeff, SrcConst, DstConst, CurLoop, Level,
                                Result, NewConstraint);
    } else if (areCEEqual(SrcCoeff, getNegative(DstCoeff))) {
      Disproven = weakCrossingSIVtest(SrcCoeff, SrcConst, DstConst, CurLoop,
                                      Level, Result, NewConstraint, SplitIter);
    } else {
      Disproven = exactSIVtest(SrcCoeff, DstCoeff, SrcConst, DstConst, CurLoop,
                               Level, Result, NewConstraint);
    }

    return Disproven ||
           gcdMIVtest(Src, Dst, SrcParentLoop, DstParentLoop, Result) ||
           symbolicRDIVtest(SrcCoeff, DstCoeff, SrcConst, DstConst, CurLoop,
                            CurLoop);
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
                      FullDependences &Result, const HLLoop *SrcParentLoop,
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

  DEBUG(dbgs() << "\n    src = "; Src->dump());
  DEBUG(dbgs() << "\n    Dst = "; Dst->dump());

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
                     const SmallBitVector &Loops, FullDependences &Result,
                     const HLLoop *SrcParentLoop, const HLLoop *DstParentLoop) {

  DEBUG(dbgs() << "\n   src = "; Src->dump());
  DEBUG(dbgs() << "\n   dst = "; Dst->dump());
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
                        const HLLoop *DstParentLoop, FullDependences &Result) {

  DEBUG(dbgs() << "\nstarting gcd\n");
  DEBUG(dbgs() << "\n   src = "; Src->dump());
  DEBUG(dbgs() << "\n   dst = "; Dst->dump());

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

  DEBUG(dbgs() << "\nRunningGCD  =" << RunningGCD);
  const CanonExpr *CE = Src;
  const CanonExpr *CE2;

  int64_t K1 = 0;
  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {

    unsigned Index;
    int64_t Coeff1;
    CE->getIVCoeff(CurIVPair, &Index, &Coeff1);

    DEBUG(dbgs() << "\nindex, coeff  =" << Index << "  " << Coeff1);

    K1 = CE->getIVConstCoeff(CurIVPair);
    if (K1 == 0) {
      continue;
    }
    //  okay to ignore blobcoeff
    //  BlobCoeff(CurIVPair)

    // do i=1,n; do j=2,n; a(i+2*j) = a(i+2*j-1) with input dv (= *)
    // returns INDEP (Tested already)

    APInt ConstCoeff = llvm::APInt(64, K1, true);
    DEBUG(dbgs() << "\nRunningGCD in  =" << RunningGCD);
    DEBUG(dbgs() << "\n k1  =" << K1);

    RunningGCD = APIntOps::GreatestCommonDivisor(RunningGCD, ConstCoeff.abs());
    DEBUG(dbgs() << "\nRunningGCD1  =" << RunningGCD);
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
    DEBUG(dbgs() << "\nRunningGCD2  =" << RunningGCD);
  }

  const CanonExpr *DstConst = getInvariant(Dst);

  APInt ExtraGCD = APInt::getNullValue(BitWidth);
  const CanonExpr *Delta = getMinus(DstConst, SrcConst);
  if (!Delta) {
    return false;
  }

  DEBUG(dbgs() << "    Delta = "; Delta->dump());
  K1 = Delta->getConstant();
  APInt ConstDelta = llvm::APInt(64, K1, true);
  DEBUG(dbgs() << "\n ConstDelta = " << ConstDelta << "\n");
  if (ConstDelta == 0) {
    return false;
  }

  for (auto Blob = Delta->blob_begin(), End = Delta->blob_end(); Blob != End;
       ++Blob) {
    APInt ConstCoeff = llvm::APInt(64, Delta->getBlobCoeff(Blob), true);
    ExtraGCD = APIntOps::GreatestCommonDivisor(ExtraGCD, ConstCoeff.abs());
  }
  RunningGCD = APIntOps::GreatestCommonDivisor(RunningGCD, ExtraGCD);
  DEBUG(dbgs() << "    RunningGCD = " << RunningGCD << "\n");
  APInt Remainder = ConstDelta.srem(RunningGCD);

  // e.g.  A[2i + 4j +6m +8n +1],  A[16i +4j +2m+6]
  // will derive Independence

  if (Remainder != 0) {
    DEBUG(dbgs() << "GCD success\n");
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
  DEBUG(dbgs() << "    ExtraGCD = " << ExtraGCD << '\n');

  bool Improved = false;
  CE = Src;

  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {

    if (!CE->getIVConstCoeff(CurIVPair)) {
      continue;
    }
    if (CE->getIVBlobCoeff(CurIVPair)) {
      // TODO   3 * N .. return false for now
      // return false;
    }

    // based on level, get to corrs. parent loop
    const HLLoop *CurLoop =
        SrcParentLoop->getParentLoopAtLevel(CE->getLevel(CurIVPair));

    assert(CurLoop && "Expecting parent loop not null");

    RunningGCD = ExtraGCD;

    const CanonExpr *SrcCoeff =
        getConstantWithType(Src->getSrcType(), Src->getDestType(),
                            Src->isSExt(), CE->getIVConstCoeff(CurIVPair));
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
      if (CE->getLevel(CurIVPair) == CE2->getLevel(CurIVPair2)) {
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

      if (CE->getLevel(CurIVPair) == CE2->getLevel(CurIVPair2)) {
        DstCoeff = getConstantWithType(Dst->getSrcType(), Dst->getDestType(),
                                       Dst->isSExt(),
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
    DEBUG(dbgs() << "\tRunningGCD = " << RunningGCD << "\n");
    if (RunningGCD != 0) {
      Remainder = ConstDelta.srem(RunningGCD);
      DEBUG(dbgs() << "\tRemainder = " << Remainder << "\n");
      if (Remainder != 0) {
        unsigned Level = mapSrcLoop(CurLoop);
        DEBUG(dbgs() << "\tLevel=" << Level << "\n");
        Result.DV[Level - 1].Direction &= ~DVKind::EQ;
        Improved = true;
      }
    }
  }

  if (Improved) {
    DEBUG(dbgs() << "GCD success\n");
    ++GCDsuccesses;
  }

  DEBUG(dbgs() << "all done\n");
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
                             const SmallBitVector &Loops,
                             FullDependences &Result,
                             const HLLoop *SrcParentLoop,
                             const HLLoop *DstParentLoop) {

  DEBUG(dbgs() << "\nstarting Banerjee\n");
  ++BanerjeeApplications;
  DEBUG(dbgs() << "\n   Src = "; Src->dump());
  const CanonExpr *A0;
  CoefficientInfo *A =
      collectCoeffInfo(Src, true, A0, SrcParentLoop, DstParentLoop);
  DEBUG(dbgs() << "\n   Dst = "; Dst->dump());
  const CanonExpr *B0;
  CoefficientInfo *B =
      collectCoeffInfo(Dst, false, B0, SrcParentLoop, DstParentLoop);
  BoundInfo *Bound = new BoundInfo[MaxLevels + 1];
  const CanonExpr *Delta = getMinus(B0, A0);
  if (!Delta) {
    return false;
  }
  DEBUG(dbgs() << "\n    Delta = "; Delta->dump());

  // Compute bounds for all the * directions.
  DEBUG(dbgs() << "\n\tBounds[*]\n");
  for (unsigned K = 1; K <= MaxLevels; ++K) {
    Bound[K].Iterations = A[K].Iterations ? A[K].Iterations : B[K].Iterations;
    Bound[K].Direction = DVKind::ALL;
    Bound[K].DirSet = DVKind::NONE;
    findBoundsALL(A, B, Bound, K);
#ifndef NDEBUG
    DEBUG(dbgs() << "\n    " << K << '\t');
    const CanonExpr *BL = Bound[K].Lower[DVKind::ALL];
    const CanonExpr *BU = Bound[K].Upper[DVKind::ALL];
    if (BL) {
      DEBUG(dbgs() << " "; BL->dump());
    } else {
      DEBUG(dbgs() << "-inf\t");
    }
    if (BU) {
      DEBUG(dbgs() << " "; BU->dump());
    } else {
      DEBUG(dbgs() << "+inf\n");
    }
#endif
  }

  // Test the *, *, *, ... case.
  bool Disproved = false;
  if (testBounds(DVKind::ALL, 0, Bound, Delta, InputDV)) {
    // Explore the direction vector hierarchy.
    unsigned DepthExpanded = 0;
    unsigned NewDeps =
        exploreDirections(1, A, B, Bound, Loops, DepthExpanded, Delta, InputDV);
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
  delete[] Bound;
  delete[] A;
  delete[] B;
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
    DEBUG(dbgs() << "\n\t[");
    for (unsigned K = 1; K <= CommonLevels; ++K) {
      if (Loops[K]) {
        Bound[K].DirSet |= Bound[K].Direction;
#ifndef NDEBUG
        switch (Bound[K].Direction) {
        case DVKind::LT:
          DEBUG(dbgs() << " <");
          break;
        case DVKind::EQ:
          DEBUG(dbgs() << " =");
          break;
        case DVKind::GT:
          DEBUG(dbgs() << " >");
          break;
        case DVKind::ALL:
          DEBUG(dbgs() << " *");
          break;
        default:
          llvm_unreachable("unexpected Bound[K].Direction");
        }
#endif
      }
    }
    DEBUG(dbgs() << " ]");
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
      DEBUG(dbgs() << "\n\tBound for level = " << Level << '\n');
      DEBUG(dbgs() << "\n\t    <\t");
      const CanonExpr *CE;
      CE = Bound[Level].Lower[DVKind::LT];
      if (CE) {
        DEBUG(dbgs(); CE->dump());
        DEBUG(dbgs() << "\t");
      } else {
        DEBUG(dbgs() << "-inf\t");
      }
      CE = Bound[Level].Upper[DVKind::LT];
      if (CE) {
        DEBUG(dbgs(); CE->dump());
        DEBUG(dbgs() << "\t");
      } else {
        DEBUG(dbgs() << "+inf\n");
      }
      DEBUG(dbgs() << "\n\t    =\t");
      CE = Bound[Level].Lower[DVKind::EQ];
      if (CE) {
        DEBUG(dbgs(); CE->dump());
        DEBUG(dbgs() << "\t");
      } else {
        DEBUG(dbgs() << "-inf\t");
      }
      CE = Bound[Level].Upper[DVKind::EQ];
      if (CE) {
        DEBUG(dbgs(); CE->dump());
        DEBUG(dbgs() << "\t");
      } else {
        DEBUG(dbgs() << "+inf\n");
      }
      DEBUG(dbgs() << "\n\t    >\t");
      CE = Bound[Level].Lower[DVKind::GT];
      if (CE) {
        DEBUG(dbgs(); CE->dump());
        DEBUG(dbgs() << "\t");
      } else {
        DEBUG(dbgs() << "-inf\t");
      }
      CE = Bound[Level].Upper[DVKind::GT];
      if (CE) {
        DEBUG(dbgs(); CE->dump());
        DEBUG(dbgs() << "\t");
      } else {
        DEBUG(dbgs() << "+inf\n");
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
  DEBUG(dbgs() << "\n\tTestBound: Level,DirKind\t" << Level << " "
               << (unsigned)DirKind);

  // Level = 0 is called banerjeeMIVTest before expanding the call for
  // all combinations of DV
  // exploreDircetions will spawn off testing all combinations of DV
  // with Level > 0

  if (Level && (DirKind & InputDV[Level - 1]) == 0) {
    DEBUG(dbgs() << "\n\tSkip testBound because no match with inputDV");
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
      Bound[K].Lower[DVKind::ALL] = getConstantWithType(
          CE->getSrcType(), CE->getDestType(), CE->isSExt(), 0);
    }
    if (isKnownPredicate(CmpInst::ICMP_EQ, A[K].PosPart, B[K].NegPart)) {
      auto CE = A[K].Coeff;
      Bound[K].Upper[DVKind::ALL] = getConstantWithType(
          CE->getSrcType(), CE->getDestType(), CE->isSExt(), 0);
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
        getMinus(Bound[K].Iterations,
                 getConstantWithType(CE->getSrcType(), CE->getDestType(),
                                     CE->isSExt(), 1));
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
        getMinus(Bound[K].Iterations,
                 getConstantWithType(CE->getSrcType(), CE->getDestType(),
                                     CE->isSExt(), 1));
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
  return getSMaxExpr(X, getConstantWithType(X->getSrcType(), X->getDestType(),
                                            X->isSExt(), 0));
}

// X^- = min(X, 0)
const CanonExpr *DDTest::getNegativePart(const CanonExpr *X) {
  if (!X) {
    return nullptr;
  }
  return getSMinExpr(X, getConstantWithType(X->getSrcType(), X->getDestType(),
                                            X->isSExt(), 0));
}

// Walks through the subscript,
// collecting each coefficient, the associated loop bounds,
// and recording its positive and negative parts for later use.
DDTest::CoefficientInfo *DDTest::collectCoeffInfo(const CanonExpr *Subscript,
                                                  bool SrcFlag,
                                                  const CanonExpr *&Constant,
                                                  const HLLoop *SrcParentLoop,
                                                  const HLLoop *DstParentLoop) {

  const CanonExpr *Zero =
      getConstantWithType(Subscript->getSrcType(), Subscript->getDestType(),
                          Subscript->isSExt(), 0);
  CoefficientInfo *CI = new CoefficientInfo[MaxLevels + 1];
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
    if (CE->getIVBlobCoeff(CurIVPair)) {
      // TODO: for blob coeff
      continue;
    }
    if (SrcFlag) {
      L = SrcParentLoop->getParentLoopAtLevel(CE->getLevel(CurIVPair));
      K = mapSrcLoop(L);
    } else {
      L = DstParentLoop->getParentLoopAtLevel(CE->getLevel(CurIVPair));
      K = mapDstLoop(L);
    }

    const CanonExpr *CE2 = CI[K].Coeff = getConstantWithType(
        Subscript->getSrcType(), Subscript->getDestType(), Subscript->isSExt(),
        CE->getIVConstCoeff(CurIVPair));
    CI[K].PosPart = getPositivePart(CE2);
    CI[K].NegPart = getNegativePart(CE2);
    // unused type argument
    CI[K].Iterations = collectUpperBound(L, Subscript->getSrcType());
  }

  Constant = getInvariant(CE);

#ifndef NDEBUG
  DEBUG(dbgs() << "\tCoefficient Info\n");
  for (unsigned K = 1; K <= MaxLevels; ++K) {
    DEBUG(dbgs() << "\n       " << K << "\t"; (CI[K].Coeff)->dump());
    DEBUG(dbgs() << "\tPos Part = "; (CI[K].PosPart)->dump());
    DEBUG(dbgs() << "\tNeg Part = "; (CI[K].NegPart)->dump());
    DEBUG(dbgs() << "\tUpper Bound = ");
    if (CI[K].Iterations)
      DEBUG(dbgs(); (CI[K].Iterations)->dump());
    else
      DEBUG(dbgs() << "+inf");
    DEBUG(dbgs() << '\n');
  }
  DEBUG(dbgs() << "\t   Constant = "; Constant->dump());
  DEBUG(dbgs() << "\n");

#endif
  return CI;
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

#if 0

//===----------------------------------------------------------------------===//
// Constraint manipulation for Delta test.

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
bool DependenceAnalysis::propagate(const SCEV *&Src,
                                   const SCEV *&Dst,
                                   SmallBitVector &Loops,
                                   SmallVectorImpl<Constraint> &Constraints,
                                   bool &Consistent) {
  bool Result = false;
  for (int LI = Loops.find_first(); LI >= 0; LI = Loops.find_next(LI)) {
    DEBUG(dbgs() << "\t    Constraint[" << LI << "] is");
    DEBUG(Constraints[LI].dump(dbgs()));
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
bool DependenceAnalysis::propagateDistance(const SCEV *&Src,
                                           const SCEV *&Dst,
                                           Constraint &CurConstraint,
                                           bool &Consistent) {
  const Loop *CurLoop = CurConstraint.getAssociatedLoop();
  DEBUG(dbgs() << "\t\tSrc is " << *Src << "\n");
  const SCEV *A_K = findCoefficient(Src, CurLoop);
  if (A_K->isZero())
    return false;
  const SCEV *DA_K = SE->getMulExpr(A_K, CurConstraint.getD());
  Src = SE->getMinusSCEV(Src, DA_K);
  Src = zeroCoefficient(Src, CurLoop);
  DEBUG(dbgs() << "\t\tnew Src is " << *Src << "\n");
  DEBUG(dbgs() << "\t\tDst is " << *Dst << "\n");
  Dst = addToCoefficient(Dst, CurLoop, SE->getNegativeSCEV(A_K));
  DEBUG(dbgs() << "\t\tnew Dst is " << *Dst << "\n");
  if (!findCoefficient(Dst, CurLoop)->isZero())
    Consistent = false;
  return true;
}


// Attempt to propagate a line
// constraint into a subscript pair (Src and Dst).
// Return true if some simplification occurs.
// If the simplification isn't exact (that is, if it is conservative
// in terms of dependence), set consistent to false.
bool DependenceAnalysis::propagateLine(const SCEV *&Src,
                                       const SCEV *&Dst,
                                       Constraint &CurConstraint,
                                       bool &Consistent) {
  const Loop *CurLoop = CurConstraint.getAssociatedLoop();
  const SCEV *A = CurConstraint.getA();
  const SCEV *B = CurConstraint.getB();
  const SCEV *C = CurConstraint.getC();
  DEBUG(dbgs() << "\t\tA = " << *A << ", B = " << *B << ", C = " << *C << "\n");
  DEBUG(dbgs() << "\t\tSrc = " << *Src << "\n");
  DEBUG(dbgs() << "\t\tDst = " << *Dst << "\n");
  if (A->isZero()) {
    const SCEVConstant *Bconst = dyn_cast<SCEVConstant>(B);
    const SCEVConstant *Cconst = dyn_cast<SCEVConstant>(C);
    if (!Bconst || !Cconst) return false;
    APInt Beta = Bconst->getValue()->getValue();
    APInt Charlie = Cconst->getValue()->getValue();
    APInt CdivB = Charlie.sdiv(Beta);
    assert(Charlie.srem(Beta) == 0 && "C should be evenly divisible by B");
    const SCEV *AP_K = findCoefficient(Dst, CurLoop);
    //    Src = SE->getAddExpr(Src, SE->getMulExpr(AP_K, SE->getConstant(CdivB)));
    Src = SE->getMinusSCEV(Src, SE->getMulExpr(AP_K, SE->getConstant(CdivB)));
    Dst = zeroCoefficient(Dst, CurLoop);
    if (!findCoefficient(Src, CurLoop)->isZero())
      Consistent = false;
  }
  else if (B->isZero()) {
    const SCEVConstant *Aconst = dyn_cast<SCEVConstant>(A);
    const SCEVConstant *Cconst = dyn_cast<SCEVConstant>(C);
    if (!Aconst || !Cconst) return false;
    APInt Alpha = Aconst->getValue()->getValue();
    APInt Charlie = Cconst->getValue()->getValue();
    APInt CdivA = Charlie.sdiv(Alpha);
    assert(Charlie.srem(Alpha) == 0 && "C should be evenly divisible by A");
    const SCEV *A_K = findCoefficient(Src, CurLoop);
    Src = SE->getAddExpr(Src, SE->getMulExpr(A_K, SE->getConstant(CdivA)));
    Src = zeroCoefficient(Src, CurLoop);
    if (!findCoefficient(Dst, CurLoop)->isZero())
      Consistent = false;
  }
  else if (isKnownPredicate(CmpInst::ICMP_EQ, A, B)) {
    const SCEVConstant *Aconst = dyn_cast<SCEVConstant>(A);
    const SCEVConstant *Cconst = dyn_cast<SCEVConstant>(C);
    if (!Aconst || !Cconst) return false;
    APInt Alpha = Aconst->getValue()->getValue();
    APInt Charlie = Cconst->getValue()->getValue();
    APInt CdivA = Charlie.sdiv(Alpha);
    assert(Charlie.srem(Alpha) == 0 && "C should be evenly divisible by A");
    const SCEV *A_K = findCoefficient(Src, CurLoop);
    Src = SE->getAddExpr(Src, SE->getMulExpr(A_K, SE->getConstant(CdivA)));
    Src = zeroCoefficient(Src, CurLoop);
    Dst = addToCoefficient(Dst, CurLoop, A_K);
    if (!findCoefficient(Dst, CurLoop)->isZero())
      Consistent = false;
  }
  else {
    // paper is incorrect here, or perhaps just misleading
    const SCEV *A_K = findCoefficient(Src, CurLoop);
    Src = SE->getMulExpr(Src, A);
    Dst = SE->getMulExpr(Dst, A);
    Src = SE->getAddExpr(Src, SE->getMulExpr(A_K, C));
    Src = zeroCoefficient(Src, CurLoop);
    Dst = addToCoefficient(Dst, CurLoop, SE->getMulExpr(A_K, B));
    if (!findCoefficient(Dst, CurLoop)->isZero())
      Consistent = false;
  }
  DEBUG(dbgs() << "\t\tnew Src = " << *Src << "\n");
  DEBUG(dbgs() << "\t\tnew Dst = " << *Dst << "\n");
  return true;
}


// Attempt to propagate a point
// constraint into a subscript pair (Src and Dst).
// Return true if some simplification occurs.
bool DependenceAnalysis::propagatePoint(const SCEV *&Src,
                                        const SCEV *&Dst,
                                        Constraint &CurConstraint) {
  const Loop *CurLoop = CurConstraint.getAssociatedLoop();
  const SCEV *A_K = findCoefficient(Src, CurLoop);
  const SCEV *AP_K = findCoefficient(Dst, CurLoop);
  const SCEV *XA_K = SE->getMulExpr(A_K, CurConstraint.getX());
  const SCEV *YAP_K = SE->getMulExpr(AP_K, CurConstraint.getY());
  DEBUG(dbgs() << "\t\tSrc is " << *Src << "\n");
  Src = SE->getAddExpr(Src, SE->getMinusSCEV(XA_K, YAP_K));
  Src = zeroCoefficient(Src, CurLoop);
  DEBUG(dbgs() << "\t\tnew Src is " << *Src << "\n");
  DEBUG(dbgs() << "\t\tDst is " << *Dst << "\n");
  Dst = zeroCoefficient(Dst, CurLoop);
  DEBUG(dbgs() << "\t\tnew Dst is " << *Dst << "\n");
  return true;
}


// Update direction vector entry based on the current constraint.
void DependenceAnalysis::updateDirection(Dependence::DVEntry &Level,
                                         const Constraint &CurConstraint
                                         ) const {
  DEBUG(dbgs() << "\tUpdate direction, constraint =");
  DEBUG(CurConstraint.dump(dbgs()));
  if (CurConstraint.isAny())
    ; // use defaults
  else if (CurConstraint.isDistance()) {
    // this one is consistent, the others aren't
    Level.Scalar = false;
    Level.Distance = CurConstraint.getD();
    unsigned NewDirection = DVKind::NONE;
    if (!SE->isKnownNonZero(Level.Distance)) // if may be zero
      NewDirection = DVKind::EQ;
    if (!SE->isKnownNonPositive(Level.Distance)) // if may be positive
      NewDirection |= DVKind::LT;
    if (!SE->isKnownNonNegative(Level.Distance)) // if may be negative
      NewDirection |= DVKind::GT;
    Level.Direction &= NewDirection;
  }
  else if (CurConstraint.isLine()) {
    Level.Scalar = false;
    Level.Distance = nullptr;
    // direction should be accurate
  }
  else if (CurConstraint.isPoint()) {
    Level.Scalar = false;
    Level.Distance = nullptr;
    unsigned NewDirection = DVKind::NONE;
    if (!isKnownPredicate(CmpInst::ICMP_NE,
                          CurConstraint.getY(),
                          CurConstraint.getX()))
      // if X may be = Y
      NewDirection |= DVKind::EQ;
    if (!isKnownPredicate(CmpInst::ICMP_SLE,
                          CurConstraint.getY(),
                          CurConstraint.getX()))
      // if Y may be > X
      NewDirection |= DVKind::LT;
    if (!isKnownPredicate(CmpInst::ICMP_SGE,
                          CurConstraint.getY(),
                          CurConstraint.getX()))
      // if Y may be < X
      NewDirection |= DVKind::GT;
    Level.Direction &= NewDirection;
  }
  else
    llvm_unreachable("constraint has unexpected kind");
}

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
  SrcAR->collectParametricTerms(SE, Terms);
  DstAR->collectParametricTerms(SE, Terms);

  // Second step: find subscript sizes.
  SmallVector<const SCEV *, 4> Sizes;
  SE->findArrayDimensions(Terms, Sizes, ElementSize);

  // Third step: compute the access functions for each subscript.
  SmallVector<const SCEV *, 4> SrcSubscripts, DstSubscripts;
  SrcAR->computeAccessFunctions(SE, SrcSubscripts, Sizes);
  DstAR->computeAccessFunctions(SE, DstSubscripts, Sizes);

  // Fail when there is only a subscript: that's a linearized access function.
  if (SrcSubscripts.size() < 2 || DstSubscripts.size() < 2 ||
      SrcSubscripts.size() != DstSubscripts.size())
    return false;

  int size = SrcSubscripts.size();

  DEBUG({
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

//===----------------------------------------------------------------------===//

#endif

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

DDTest::DDTest(AAResults &AAR, HLNodeUtils &HNU, HIRLoopStatistics &HLS)
    : AAR(AAR), HNU(HNU), HLS(HLS) {
  DEBUG(dbgs() << "DDTest initiated\n");
  WorkCE.clear();
}

DDTest::~DDTest() {
  DEBUG(dbgs() << "\n ~DDTest called\n");
  for (auto I = WorkCE.begin(), E = WorkCE.end(); I != E; ++I) {
    // const CanonExpr *CE = *I;
    // DEBUG(dbgs() << "CE: " << CE << " "; CE->dump());
    HNU.getCanonExprUtils().destroy(const_cast<CanonExpr *>(*I));
  }

  WorkCE.clear();
}

static MemoryLocation getMemoryLocation(const RegDDRef *Ref) {
  MemoryLocation Loc;

  const CanonExpr *BaseCE = Ref->getBaseCE();
  if (BaseCE->isNull()) {
    Loc.Ptr = Constant::getNullValue(BaseCE->getDestType());
  } else {
    auto BaseBlobIndex = Ref->getBaseCE()->getSingleBlobIndex();
    Loc.Ptr = Ref->getBlobUtils().getTempBlobValue(BaseBlobIndex);
  }

  Loc.Size = MemoryLocation::UnknownSize;

  Ref->getAAMetadata(Loc.AATags);

  return Loc;
}

bool DDTest::queryAAIndep(RegDDRef *SrcDDRef, RegDDRef *DstDDRef) {
  assert(SrcDDRef->isMemRef() && DstDDRef->isMemRef() &&
         "Both should be mem refs");

  if (SrcDDRef == DstDDRef) {
    return false;
  }

  DEBUG_AA(dbgs() << "call queryAAIndep():\n");
  DEBUG_AA(SrcDDRef->dump());
  DEBUG_AA(dbgs() << "\n");
  DEBUG_AA(DstDDRef->dump());
  DEBUG_AA(dbgs() << "\nR: ");

  if (AAR.isNoAlias(getMemoryLocation(SrcDDRef), getMemoryLocation(DstDDRef))) {
    DEBUG_AA(dbgs() << "No Alias\n\n");
    return true;
  }

  DEBUG_AA(dbgs() << "May Alias\n\n");
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

std::unique_ptr<Dependences> DDTest::depends(DDRef *SrcDDRef, DDRef *DstDDRef,
                                             const DirectionVector &InputDV,
                                             bool fromFusion) {
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
  // (1) fromFusion will assume the 2 DDRefs are within the same loop (not
  //     implemented yet)
  //
  // (2) Current code builds DV up to common levels only.
  //     This would be okay for most practical purposes.
  //     Extension can be made later for other needs such as fusion or
  //     privatization.
  //     Need some work for refs outside loop - should always has a DV
  //     instead of  empty
  //

  bool IsSrcRval = false;
  bool IsDstRval = false;

  int NumSrcDim = 1;
  int NumDstDim = 1;

  bool EqualBaseCE = false;

  DEBUG(dbgs() << "\n Src, Dst DDRefs\n"; SrcDDRef->dump());
  DEBUG(dbgs() << ",  "; DstDDRef->dump());
  DEBUG(dbgs() << "\n"
               << SrcDDRef->getHLDDNode()->getNumber() << ":"
               << DstDDRef->getHLDDNode()->getNumber());

  assert(SrcDDRef->getSymbase() == DstDDRef->getSymbase() &&
         "Asking DDA for distinct references is useless");

  // Normally it should not be called here. but check anyway
  //	if (isa<ConstDDRef>(SrcDDRef) ||  isa<ConstDDRef>(DstDDRef)) {
  //    return nullptr;
  //	}

  RegDDRef *SrcRegDDRef = dyn_cast<RegDDRef>(SrcDDRef);
  if (SrcRegDDRef) {
    NumSrcDim = SrcRegDDRef->getNumDimensions();
    if (!SrcRegDDRef->isLval()) {
      IsSrcRval = true;
    }
  } else {
    IsSrcRval = true;
  }

  RegDDRef *DstRegDDRef = dyn_cast<RegDDRef>(DstDDRef);
  if (DstRegDDRef) {
    NumDstDim = DstRegDDRef->getNumDimensions();
    if (!DstRegDDRef->isLval()) {
      IsDstRval = true;
    }
  } else {
    IsDstRval = true;
  }

  DEBUG(dbgs() << "\nSrc/Dst Blob?  " << (SrcRegDDRef == nullptr) << " "
               << (DstRegDDRef == nullptr) << "\n");

  if ((IsSrcRval && IsDstRval)) {
    // if both instructions don't reference memory, there's no dependence
    // TODO: need to handle input DEP when sc_repl is ready
    // okay to skip input dep now
    // We only need to generate input dep when there is no dd_ref with lval
    // so it requires a scan first
    return nullptr;
  }

  // If both are memory refs
  if (SrcRegDDRef && DstRegDDRef && SrcRegDDRef->isMemRef() &&
      DstRegDDRef->isMemRef()) {

    // Inquire disam util to get INDEP based on type/scope based analysis.
    DEBUG(dbgs() << "AA query: ");
    if (queryAAIndep(SrcRegDDRef, DstRegDDRef)) {
      DEBUG(dbgs() << "no alias\n");
      return nullptr;
    }
    DEBUG(dbgs() << "may alias\n");

    auto SrcBaseCE = SrcRegDDRef->getBaseCE();
    auto DstBaseCE = DstRegDDRef->getBaseCE();

    // We check for equal base CE
    EqualBaseCE = areCEEqual(SrcBaseCE, DstBaseCE);
  }

  // establish loop nesting levels
  establishNestingLevels(SrcDDRef, DstDDRef);

  DEBUG(dbgs() << "\ncommon nesting levels = " << CommonLevels << "\n");
  DEBUG(dbgs() << "\nmaximum nesting levels = " << MaxLevels << "\n");

  FullDependences Result(SrcDDRef, DstDDRef, CommonLevels);
  ++TotalArrayPairs;
  WorkCE.clear();

  //  Number of dimemsion are different or different base: need to bail out
  if (!EqualBaseCE || NumSrcDim != NumDstDim) {
    DEBUG(dbgs() << "\nDiff dim or base\n");
    auto Final = make_unique<FullDependences>(Result);
    Result.DV = nullptr;
    return std::move(Final);
  }

  unsigned Pairs = NumSrcDim;

  DEBUG(dbgs() << " # of Pairs " << Pairs << "\n");

  SmallVector<Subscript, 4> Pair(Pairs);

  if (SrcRegDDRef) {
    int P = 0;
    for (auto CE = SrcRegDDRef->canon_begin(), E = SrcRegDDRef->canon_end();
         CE != E; CE++, P++) {
      Pair[P].Src = *CE;
    }

  } else {
    BlobDDRef *BRef = cast<BlobDDRef>(SrcDDRef);
    Pair[0].Src = BRef->getCanonExpr();
  }

  if (DstRegDDRef) {
    int P = 0;
    for (auto CE = DstRegDDRef->canon_begin(), E = DstRegDDRef->canon_end();
         CE != E; CE++, P++) {
      Pair[P].Dst = *CE;
    }
  } else {
    BlobDDRef *BRef = cast<BlobDDRef>(DstDDRef);
    Pair[0].Dst = BRef->getCanonExpr();
  }

  // Note: Couple of original functionality were skipped
  //  UnifyingSubscriptType due to different sign extension
  //  Delinearize: assuming handle in framework

  const HLLoop *SrcLoop = nullptr;
  const HLLoop *DstLoop = nullptr;

  HLDDNode *SrcDDNode = SrcDDRef->getHLDDNode();
  HLDDNode *DstDDNode = DstDDRef->getHLDDNode();

  HLNode *SrcHIR = dyn_cast<HLNode>(SrcDDNode);
  assert(SrcHIR && "HIR not found for Src DDRef");
  HLNode *DstHIR = dyn_cast<HLNode>(DstDDNode);
  assert(DstHIR && "HIR not found for Dst DDRef");

  HLLoop *SrcParent = SrcHIR->getParentLoop();
  HLLoop *DstParent = DstHIR->getParentLoop();

  if (SrcParent) {
    SrcLoop = SrcParent;
  }
  if (DstParent) {
    DstLoop = DstParent;
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
    DEBUG(dbgs() << "\n    subscript " << P << "\n");
    DEBUG(dbgs() << "\nsrc = "; (Pair[P].Src)->dump());
    DEBUG(dbgs() << "\ndst = "; (Pair[P].Dst)->dump());
    DEBUG(dbgs() << "\nclass = " << Pair[P].Classification << "\n");
    DEBUG(dbgs() << "\nloops = ");
    DEBUG(dumpSmallBitVector(Pair[P].Loops));
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

  DEBUG(dbgs() << "    Separable = ");
  DEBUG(dumpSmallBitVector(Separable));
  DEBUG(dbgs() << "    Coupled = ");
  DEBUG(dumpSmallBitVector(Coupled));

  Constraint NewConstraint;

  NewConstraint.setAny();

  // test separable subscripts
  for (int SI = Separable.find_first(); SI >= 0; SI = Separable.find_next(SI)) {
    DEBUG(dbgs() << "testing subscript " << SI);
    switch (Pair[SI].Classification) {
    case Subscript::ZIV:
      DEBUG(dbgs() << ", ZIV\n");
      if (testZIV(Pair[SI].Src, Pair[SI].Dst, Result))
        return nullptr;
      break;

    case Subscript::SIV: {
      DEBUG(dbgs() << ", SIV\n");
      unsigned Level;
      const CanonExpr *SplitIter = nullptr;
      if (testSIV(Pair[SI].Src, Pair[SI].Dst, Level, Result, NewConstraint,
                  SplitIter, SrcLoop, DstLoop)) {
        return nullptr;
      }
      break;
    }

    case Subscript::RDIV:
      DEBUG(dbgs() << ", RDIV\n");
      if (testRDIV(Pair[SI].Src, Pair[SI].Dst, Result, SrcLoop, DstLoop)) {
        return nullptr;
      }
      break;
    case Subscript::MIV:

      DEBUG(dbgs() << ", MIV\n");

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

      if (Level <= Result.Levels) {
        // DV is computed up to Common Level of the 2 DDRef
        Result.setDirection(Level,
                            Result.getDirection(Level) & InputDV[Level - 1]);

        if (Result.getDirection(Level) == DVKind::NONE) {
          return nullptr;
        }
      }
    }
  }

// TODO
#if 0
  if (Coupled.count()) {
    // test coupled subscript groups
    DEBUG(dbgs() << "starting on coupled subscripts\n");
    DEBUG(dbgs() << "MaxLevels + 1 = " << MaxLevels + 1 << "\n");
    SmallVector<Constraint, 4> Constraints(MaxLevels + 1);

    for (unsigned II = 0; II <= MaxLevels; ++II) {
      Constraints[II].setAny();
    }
    for (int SI = Coupled.find_first(); SI >= 0; SI = Coupled.find_next(SI)) {
      DEBUG(dbgs() << "testing subscript group " << SI << " { ");
      SmallBitVector Group(Pair[SI].Group);
      SmallBitVector Sivs(Pairs);
      SmallBitVector Mivs(Pairs);
      SmallBitVector ConstrainedLevels(MaxLevels + 1);
      for (int SJ = Group.find_first(); SJ >= 0; SJ = Group.find_next(SJ)) {
        DEBUG(dbgs() << SJ << " ");
        if (Pair[SJ].Classification == Subscript::SIV)
          Sivs.set(SJ);
        else
          Mivs.set(SJ);
      }
      DEBUG(dbgs() << "}\n");
      while (Sivs.any()) {
        bool Changed = false;
        for (int SJ = Sivs.find_first(); SJ >= 0; SJ = Sivs.find_next(SJ)) {
          DEBUG(dbgs() << "testing subscript " << SJ << ", SIV\n");
          // SJ is an SIV subscript that's part of the current coupled group
          unsigned Level;
          const CanonExpr *SplitIter = nullptr;
          DEBUG(dbgs() << "SIV\n");
          if (testSIV(Pair[SJ].Src, Pair[SJ].Dst, Level, Result, NewConstraint,
                      SplitIter, SrcLoop, DstLoop))
            return nullptr;
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
          DEBUG(dbgs() << "    propagating\n");
          DEBUG(dbgs() << "\tMivs = ");
          DEBUG(dumpSmallBitVector(Mivs));
          for (int SJ = Mivs.find_first(); SJ >= 0; SJ = Mivs.find_next(SJ)) {
            // SJ is an MIV subscript that's part of the current coupled group
            DEBUG(dbgs() << "\tSJ = " << SJ << "\n");
            if (propagate(Pair[SJ].Src, Pair[SJ].Dst, Pair[SJ].Loops,
                          Constraints, Result.Consistent)) {
              DEBUG(dbgs() << "\t    Changed\n");
              ++DeltaPropagations;
              Pair[SJ].Classification = classifyPair(
                  Pair[SJ].Src, SrcLoop, Pair[SJ].Dst, DstLoop, Pair[SJ].Loops);
              switch (Pair[SJ].Classification) {
              case Subscript::ZIV:
                DEBUG(dbgs() << "ZIV\n");
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
          DEBUG(dbgs() << "RDIV test\n");
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
          DEBUG(dbgs() << "MIV test\n");
          if (testMIV(Pair[SJ].Src, Pair[SJ].Dst, InputDV, Pair[SJ].Loops, Result))
            return nullptr;
        } else
          llvm_unreachable("expected only MIV subscripts at this point");
      }

      // update Result.DV from constraint vector
      DEBUG(dbgs() << "    updating\n");
      for (int SJ = ConstrainedLevels.find_first(); SJ >= 0;
           SJ = ConstrainedLevels.find_next(SJ)) {
        updateDirection(Result.DV[SJ - 1], Constraints[SJ]);
        if (Result.DV[SJ - 1].Direction == DVKind::NONE)
          return nullptr;
      }
    }
  }

#endif

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
    Result.DV[Level].Direction &= InputDV[Level];
    if (Result.DV[Level].Direction == DVKind::NONE) {
      DEBUG(dbgs() << "\n\t return INDEP-09\n");
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

  if (AllEqual) {
    Result.LoopIndependent = true;
    // If src & sink are the same ddref  and the DV are all =
    // there is no DEP.
    if (SrcDDRef == DstDDRef) {
      DEBUG(dbgs() << "\n\t return INDEP-11\n" << SrcDDRef << DstDDRef);
      return nullptr;
    }
  }

  Result.Reversed = false;

  //
  //  Reverse DV when needed
  //

  bool NeedReversal = false;
  bool Done = false;

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

  if (NeedReversal) {
    Result.Reversed = true;
    DEBUG(dbgs() << "\nDV based on reversing ddref src & sink!\n");
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

  auto Final = make_unique<FullDependences>(Result);
  Result.DV = nullptr;
  return std::move(Final);
}

///  Create  DV for Backward Edge
///  ForwardDV will be changed if it has a leading  (<>)
///    Called when both forward and backward edges are needed
///           ( *  >  =)  returns  (*  <  =)
///           ( =  *  =)  returns  (=  *  =)
///           (<   *  <)  Not supposed to call here
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

    assert(ForwardDV[II - 1] != DVKind::LT &&
           "Unexpected Input DV for reversal");

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
                                DistanceVector &BackwardDistV,
                                unsigned Levels) {

  DEBUG(dbgs() << "\nforward DV: "; ForwardDV.print(dbgs(), Levels));
  if (ForwardDV[0] != DVKind::NONE) {
    DEBUG(dbgs() << "\nforward DistV: "; ForwardDistV.print(dbgs(), Levels));
  }

  DEBUG(dbgs() << "\nbackward DV: "; BackwardDV.print(dbgs(), Levels));
  if (BackwardDV[0] != DVKind::NONE) {
    DEBUG(dbgs() << "\nbackward DistV: "; BackwardDistV.print(dbgs(), Levels));
  }
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
                                    DistanceVector &BackwardDistV,
                                    unsigned Levels) {

  if (ForwardDV[0] != DVKind::NONE) {
    for (unsigned II = 1; II <= Levels; ++II) {
      ForwardDistV[II - 1] = mapDVToDist(ForwardDV[II - 1], II, Result);
    }
  }
  if (BackwardDV[0] != DVKind::NONE) {
    for (unsigned II = 1; II <= Levels; ++II) {
      BackwardDistV[II - 1] = mapDVToDist(BackwardDV[II - 1], II, Result);
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

  DEBUG(dbgs() << "\nTopSortNum: " << SrcNum << " " << DstNum);
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

bool DDTest::findDependences(DDRef *SrcDDRef, DDRef *DstDDRef,
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

  bool IsTemp = false;

  ForwardDV.setZero();
  BackwardDV.setZero();

  ForwardDistV.setZero();
  BackwardDistV.setZero();

  auto Result = depends(SrcDDRef, DstDDRef, InputDV);

  *IsLoopIndepDepTemp = false;

  if (Result == nullptr) {
    DEBUG(dbgs() << "\nIs Independent!\n");
  } else {
    DEBUG(Result->dump(dbgs()));
  }

  // Independent?
  if (Result == nullptr) {
    return false;
  }

  unsigned Levels = Result->getLevels();

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
        DEBUG(dbgs() << "BiDirection needed!\n");
        break;
      }
    }
  }

  if (isa<BlobDDRef>(SrcDDRef)) {
    IsTemp = true;
  } else {
    RegDDRef *RegRef = cast<RegDDRef>(SrcDDRef);
    if (RegRef->isTerminalRef()) {
      IsTemp = true;
    }
  }

  bool IsSrcRval = true;
  bool IsDstRval = true;

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
  }

  if (RegDDRef *RegRef = dyn_cast<RegDDRef>(DstDDRef)) {
    if (RegRef->isLval()) {
      IsDstRval = false;
    }
  }

  if (IsTemp) {

    // DV for Scalar temps could be refined. Calls to DA.depends
    // is still needed so it can set up the nesting level info.
    // It's a fast return because temps are classifed as non-linear.
    // result DV is all * at this stage

    bool IsFlow = false;
    bool IsAnti = false;

    if (SrcDDRef == DstDDRef) {
      // Skip self output dep for temp for now.
      // In theory, it's needed. but for analysis, we can just rely on flow &
      // anti alone
      return false;
    }

    //  Make SrcDDRef to be one that comes first in lexical order
    //  and switch DVs when reversed.

    bool IsReversed = false;
    DEBUG(dbgs() << " src/dst num " << SrcNum << " " << DstNum);

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

    if (HLNodeUtils::dominates(SrcHIR, DstHIR, &HLS)) {
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
        //    one edge (*) from Src to sink is sufficient
        for (unsigned II = 1; II <= Levels; ++II) {
          ForwardDV[II - 1] = DVKind::ALL;
        }
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

  if (Result->isPeelFirst(Levels) && Result->isReversed()) {
    setDVForPeelFirstAndReversed(ForwardDV, BackwardDV, *Result, Levels);
  } else if (BiDirection) {
    setDVForBiDirection(ForwardDV, BackwardDV, *Result, Levels, LTGTLevel);
  } else if (Result->isLoopIndependent()) {
    setDVForLoopIndependent(ForwardDV, BackwardDV, *Result, Levels, SrcNum,
                            DstNum);
  } else {
    // (3) Leftmost is <
    //     Srce->Dest
    if (!Result->isReversed()) {
      for (unsigned II = 1; II <= Levels; ++II) {
        ForwardDV[II - 1] = Result->getDirection(II);
      }
    } else {
      //  Dest->Srce
      for (unsigned II = 1; II <= Levels; ++II) {
        BackwardDV[II - 1] = Result->getDirection(II);
      }
      if ((!IsDstRval || !IsSrcRval) && (DstNum > SrcNum) && !IsTemp &&
          (SrcLevels != Levels) && BackwardDV[Levels - 1] == DVKind::LE) {
        setDVForLE(ForwardDV, BackwardDV, *Result, Levels);
      }
    }
  }

L1:
  populateDistanceVector(ForwardDV, BackwardDV, *Result, ForwardDistV,
                         BackwardDistV, Levels);
  printDirDistVectors(ForwardDV, BackwardDV, ForwardDistV, BackwardDistV,
                      Levels);
  return true;
}

// Returns last level used  in DV
// e.g  ( = = >)  return 3
unsigned DirectionVector::getLastLevel() const {
  for (unsigned II = 1; II <= MaxLoopNestLevel; ++II) {
    if ((*this)[II - 1] == DVKind::NONE) {
      return II - 1;
    }
  }

  return MaxLoopNestLevel;
}

void DirectionVector::setAsInput(const unsigned int StartLevel,
                                 const unsigned int EndLevel) {
  DirectionVector &InputDV = *this;

  // setInputDV (&InputDV, 3,4)
  // will construct (= = * *)

  for (unsigned II = 1; II < StartLevel; ++II) {
    InputDV[II - 1] = DVKind::EQ;
  }

  for (unsigned II = StartLevel; II <= EndLevel; ++II) {
    InputDV[II - 1] = DVKind::ALL;
  }

  for (unsigned II = EndLevel + 1; II <= MaxLoopNestLevel; ++II) {
    InputDV[II - 1] = DVKind::NONE;
  }
}

void DirectionVector::setZero() {
  // Construct all  0 (NONE)
  fill(DVKind::NONE);
}

void DirectionVector::print(raw_ostream &OS, unsigned Levels,
                            bool ShowLevelDetail) const {
  const DirectionVector &DV = *this;
  if (DV[0] == DVKind::NONE) {
    OS << "nil\n";
    return;
  }

  OS << "(";
  for (unsigned II = 1; II <= Levels; ++II) {
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
    default:
      break;
    } // end:switch
    if (II != Levels) {
      OS << " ";
    }
  }
  OS << ") ";
}

void DistanceVector::setZero() {
  // Construct all  0 (NONE)
  fill(0);
}

void DistanceVector::print(raw_ostream &OS, unsigned Levels) const {
  const DistanceVector &DistV = *this;

  OS << "(";
  for (unsigned II = 1; II <= Levels; ++II) {
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

void DirectionVector::print(raw_ostream &OS, bool ShowLevelDetail) const {
  print(OS, getLastLevel(), ShowLevelDetail);
}

/// Is  DV all ( = = = .. =)?
bool DirectionVector::isEQ() const {
  for (unsigned II = 1; II <= MaxLoopNestLevel; ++II) {
    auto Direction = (*this)[II - 1];
    if (Direction == DVKind::NONE) {
      break;
    }
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

  assert(CanonExprUtils::isValidLoopLevel(Level) && "incorrect Level");

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

  FullDependences Result(Src, Dst, false, CommonLevels);

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
    DEBUG(dbgs() << "    delinerized GEP\n");
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
                     Result, NewConstraint, SplitIter);
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
                         Result, NewConstraint, SplitIter);
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
