//===--- CanonExprUtils.cpp - Implements CanonExprUtils class ----- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements CanonExprUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/ErrorHandling.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"

using namespace llvm;
using namespace loopopt;

CanonExpr *CanonExprUtils::createCanonExpr(Type *Typ, unsigned Level,
                                           int64_t Const, int64_t Denom) {

  return new CanonExpr(Typ, Level, Const, Denom);
}

CanonExpr *CanonExprUtils::createCanonExpr(const APInt &APVal, int Level) {
  // make apint into a CanonExpr
  int64_t Val = APVal.getSExtValue();
  Type *Int64Type = IntegerType::get(getGlobalContext(), 64);
  return createCanonExpr(Int64Type, Level, Val, 1);
}

void CanonExprUtils::destroy(CanonExpr *CE) { CE->destroy(); }

void CanonExprUtils::destroyAll() { CanonExpr::destroyAll(); }

// Internal Method that calculates the gcd of two positive integers
int64_t CanonExprUtils::gcd(int64_t A, int64_t B) {
  assert((A > 0) && (B > 0) && "Integers must be positive!");

  // Both inputs are same.
  if (A == B) {
    return A;
  }

  // If either input is 1, return 1.
  if ((A == 1) || (B == 1)) {
    return 1;
  }

  // Calculate the GCD.
  // Works only for positive inputs.
  int64_t GCD = A;

  while (GCD != B) {
    if (GCD > B) {
      GCD = GCD - B;
    } else {
      B = B - GCD;
    }
  }

  return GCD;
}

// Internal Method that calculates the lcm of two positive integers
int64_t CanonExprUtils::lcm(int64_t A, int64_t B) {
  return ((A * B) / gcd(A, B));
}

bool CanonExprUtils::isTypeEqual(const CanonExpr *CE1, const CanonExpr *CE2) {
  return (CE1->getLLVMType() == CE2->getLLVMType());
}

bool CanonExprUtils::areEqual(const CanonExpr *CE1, const CanonExpr *CE2) {

  assert((CE1 && CE2) && " Canon Expr parameters are null");

  // Match the types
  if (!isTypeEqual(CE1, CE2))
    return false;

  if ((CE1->Const != CE2->Const) ||
      (CE1->getDenominator() != CE2->getDenominator())) {
    return false;
  }

  // Check the Blobs
  if (CE1->BlobCoeffs.size() != CE2->BlobCoeffs.size())
    return false;

  // Check the IV's
  for (unsigned Level = 1; Level < MaxLoopNestLevel; Level++) {
    bool IsCE1BlobCoeff = false, IsCE2BlobCoeff = false;
    if (CE1->getIVCoeff(Level, &IsCE1BlobCoeff) !=
        CE2->getIVCoeff(Level, &IsCE2BlobCoeff))
      return false;

    if (IsCE1BlobCoeff != IsCE2BlobCoeff)
      return false;
  }

  // Iterate through the blobs as both have same size
  for (auto I1 = CE1->blob_cbegin(), End = CE1->blob_cend(),
            I2 = CE2->blob_cbegin();
       I1 != End; ++I1, ++I2) {

    if ((I1->Index != I2->Index) || (I1->Coeff != I2->Coeff))
      return false;
  }

  return true;
}

CanonExpr *CanonExprUtils::add(CanonExpr *CE1, const CanonExpr *CE2,
                               bool CreateNewCE) {

  assert((CE1 && CE2) && " Canon Expr parameters are null!");
  assert(isTypeEqual(CE1, CE2) && " Canon Expr type mismatch!");

  CanonExpr *Result = CreateNewCE ? CE1->clone() : CE1;
  CanonExpr *NewCE2 = const_cast<CanonExpr *>(CE2);

  // Process the denoms
  int64_t Denom1 = Result->getDenominator();
  int64_t Denom2 = CE2->getDenominator();
  int NewDenom = lcm(Denom1, Denom2);

  if (NewDenom != Denom1) {
    // Do not simplify while multipying as this is intermediate result of add.
    multiplyByConstantImpl(Result, NewDenom / Denom1, false, false);
  }
  if (NewDenom != Denom2) {
    // Cannot avoid cloning CE2 here
    NewCE2 = CE2->clone();
    // Do not simplify while multipying as this is intermediate result of add.
    multiplyByConstantImpl(NewCE2, NewDenom / Denom2, false, false);
  }

  Result->setDenominator(NewDenom);

  // Add the IV's
  // We process the NewCE2 IV's as Result gets updated
  int64_t Level = 1;
  for (auto I = NewCE2->iv_cbegin(), End = NewCE2->iv_cend(); I != End;
       ++I, ++Level) {
    if (I->Coeff == 0)
      continue;

    bool isResultIVBlobCoeff;
    int64_t ResultIVBlobCoeff = Result->getIVCoeff(Level, &isResultIVBlobCoeff);

    // Result/CE1 doesn't have IV
    if (ResultIVBlobCoeff == 0) {
      Result->setIVCoeff(Level, I->Coeff, I->IsBlobCoeff);
      continue;
    }

    if ((!I->IsBlobCoeff) && (!isResultIVBlobCoeff)) {
      // Result and NewCE2 have constant Coeff
      Result->addIV(Level, I->Coeff);
    } else {
      // Handle cases when either of them is a blob
      HIRParser *HIRP = getHIRParserPtr();
      CanonExpr::BlobTy Blob1 = isResultIVBlobCoeff
                                    ? Result->getBlob(ResultIVBlobCoeff)
                                    : HIRP->createBlob(ResultIVBlobCoeff);

      CanonExpr::BlobTy Blob2 = I->IsBlobCoeff ? NewCE2->getBlob(I->Coeff)
                                               : HIRP->createBlob(I->Coeff);
      CanonExpr::BlobTy ResultBlob = HIRP->createAddBlob(Blob1, Blob2, false);
      int64_t BlobConst;
      if (HIRP->isConstIntBlob(ResultBlob, &BlobConst)) {
        Result->setIVCoeff(Level, BlobConst, false);
      } else {
        unsigned ResultBIndex = HIRP->findOrInsertBlob(ResultBlob);
        Result->setIVCoeff(Level, ResultBIndex, true);
      }
    }
  }

  // Process the Blobs
  for (auto I = NewCE2->blob_cbegin(), End = NewCE2->blob_cend(); I != End;
       ++I) {

    if (I->Coeff == 0)
      continue;

    // Add this blob to Result
    // The addBlob method will automatically take care if no blob exist
    Result->addBlob(I->Index, I->Coeff);
  }

  // Add the constant
  Result->setConstant(Result->getConstant() + NewCE2->getConstant());

  // Simplify resulting canon expr before returning.
  simplify(Result);

  return Result;
}

void CanonExprUtils::multiplyIVByConstant(CanonExpr *CE, unsigned Level,
                                          int64_t Val) {
  assert(CE && " CanonExpr is null ");

  // Identity multiplication
  if (Val == 1)
    return;

  // Remove the IV for multp. by 0
  if (Val == 0) {
    CE->removeIV(Level);
    return;
  }

  bool IsBlobCoeff;
  int64_t Coeff = CE->getIVCoeff(Level, &IsBlobCoeff);

  // Zero coefficient
  if (Coeff == 0)
    return;

  if (!IsBlobCoeff) {
    // IV doesn't have Blob Coeff
    CE->setIVCoeff(Level, Coeff * Val, false);
    return;
  }

  // IV is a blob coeff
  CanonExpr::BlobTy ValBlob = getHIRParserPtr()->createBlob(Val);
  unsigned CEBIndex;
  getHIRParserPtr()->createMulBlob(CE->getBlob(Coeff), ValBlob, true,
                                   &CEBIndex);
  CE->setIVCoeff(Level, CEBIndex, true);
}

CanonExpr *CanonExprUtils::multiplyByConstantImpl(CanonExpr *CE1, int64_t Val,
                                                  bool CreateNewCE,
                                                  bool Simplify) {

  assert(CE1 && " Canon Expr parameter is null!");

  CanonExpr *Result = CreateNewCE ? CE1->clone() : CE1;

  // Multiplying by constant is equivalent to clearing the canon expr.
  if (Val == 0) {
    Result->clear();
    return Result;
  }

  // Simplify instead of multiplying, if possible.
  if (Simplify) {
    int64_t Denom = Result->getDenominator();
    int64_t GCD = gcd(llabs(Val), Denom);

    if (GCD != 1) {
      Result->setDenominator(Denom / GCD);
      Val = Val / GCD;
    }
  }

  // Result will be the same for multiply by 1
  if (Val == 1)
    return Result;

  // Multiply Val by IVCoeff, BlobCoeffs and Const
  int64_t Level = 1;
  for (auto I = Result->iv_begin(), End = Result->iv_end(); I != End;
       ++I, ++Level) {
    multiplyIVByConstant(Result, Level, Val);
  }

  for (auto I = Result->blob_begin(), End = Result->blob_end(); I != End; ++I) {
    Result->setBlobCoeff(I->Index, (I->Coeff * Val));
  }

  Result->setConstant(Result->getConstant() * Val);

  return Result;
}

CanonExpr *CanonExprUtils::multiplyByConstant(CanonExpr *CE1, int64_t Val,
                                              bool CreateNewCE) {

  return multiplyByConstantImpl(CE1, Val, CreateNewCE, true);
}

CanonExpr *CanonExprUtils::negate(CanonExpr *CE1, bool CreateNewCE) {
  // Result = -CE1
  return multiplyByConstant(CE1, -1, CreateNewCE);
}

CanonExpr *CanonExprUtils::subtract(CanonExpr *CE1, const CanonExpr *CE2,
                                    bool CreateNewCE) {

  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  CanonExpr *Result;

  // If Blob IV Coeffs exist it is better to clone CE2, to avoid
  // any temp Blob creates during the negate operation
  bool BlobExist = CE2->hasBlobIVCoeffs() || CE1->hasBlobIVCoeffs();

  if (CreateNewCE || BlobExist) {
    // -CE2 , Result = -CE2 + CE1
    CanonExpr *NewCE2 = CE2->clone();
    Result = negate(NewCE2, false);
    Result = add(Result, CE1, false);
  } else {
    // -(-CE1+CE2) => CE1-CE2
    // Here, we avoid cloning as no blob exist
    // Thus, no temp Blobs are created for negation
    Result = negate(CE1, false);
    Result = add(Result, CE2, false);
    Result = negate(Result, false);
  }

  return Result;
}

int64_t CanonExprUtils::simplifyGCDHelper(int64_t CurrentGCD, int64_t Num) {
  if (CurrentGCD == -1) {
    CurrentGCD = llabs(Num);
  } else {
    CurrentGCD = gcd(CurrentGCD, llabs(Num));
  }

  return CurrentGCD;
}

void CanonExprUtils::simplify(CanonExpr *CE) {
  int64_t Denom, C0, NumeratorGCD = -1, CommonGCD;

  // Don't handle blob coeffs for now.
  if (CE->hasBlobIVCoeffs()) {
    return;
  }

  // Nothing to simplify...
  if ((Denom = CE->getDenominator()) == 1) {
    return;
  }

  // Cannot simplify any further.
  if ((C0 = CE->getConstant()) == 1) {
    return;
  } else if (C0) {
    NumeratorGCD = simplifyGCDHelper(NumeratorGCD, C0);
  }

  // Calculate gcd of all the iv and blob coefficients.
  for (auto I = CE->iv_cbegin(), E = CE->iv_cend(); I != E; ++I) {
    if (!I->Coeff) {
      continue;
    }
    NumeratorGCD = simplifyGCDHelper(NumeratorGCD, I->Coeff);
  }
  for (auto I = CE->blob_cbegin(), E = CE->blob_cend(); I != E; ++I) {
    NumeratorGCD = simplifyGCDHelper(NumeratorGCD, I->Coeff);
  }

  // Numerator is zero, nothing to simplify...
  if (NumeratorGCD == -1) {
    return;
  }

  // Get common gcd of numerator and denominator.
  CommonGCD = gcd(NumeratorGCD, Denom);

  // Cannot simplify any further.
  if (CommonGCD == 1) {
    return;
  }

  // Divide numerator and denominator by common gcd.
  CE->setDenominator(Denom / CommonGCD);
  CE->setConstant(C0 / CommonGCD);

  unsigned Level = 1;

  for (auto I = CE->iv_cbegin(), E = CE->iv_cend(); I != E; ++I, ++Level) {
    if (!I->Coeff) {
      continue;
    }

    CE->setIVCoeff(Level, (I->Coeff / CommonGCD), false);
  }

  for (auto I = CE->blob_cbegin(), E = CE->blob_cend(); I != E; ++I) {
    CE->setBlobCoeff(I->Index, (I->Coeff / CommonGCD));
  }
}
