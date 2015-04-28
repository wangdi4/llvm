//===--- CanonExprUtils.cpp - Implements CanonExprUtils class ----- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

void CanonExprUtils::destroyAll() {
  CanonExpr::destroyAll();
  CanonExpr::BlobTable.clear();
}

// Internal Method that calculates the lcm of two positive integers
int64_t CanonExprUtils::lcm(int64_t a, int64_t b) {

  assert((a > 0) && " Denominators must be positive.");
  assert((b > 0) && " Denominators must be positive.");

  // Both inputs are same
  if (a == b)
    return a;

  // Either input is 1, return the other one
  if(a==1)
    return b;
  if(b==1)
    return a;


  int64_t mulVal = a * b;

  // Calculate the GCD
  // Works only for positive inputs
  int64_t gcd = a;
  while (gcd != b) {
    if (gcd > b) {
      gcd = gcd - b;
    } else {
      b = b - gcd;
    }
  }

  return (mulVal / gcd);
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

  assert((CE1 && CE2) && " Canon Expr parameters are null.");
  assert(isTypeEqual(CE1, CE2) && " Canon Expr type mismatch.");

  CanonExpr *Result = CreateNewCE ? CE1->clone() : CE1;

  // Process the denoms
  int64_t denom1 = Result->getDenominator();
  int64_t denom2 = CE2->getDenominator();
  int NewDenom = lcm(denom1, denom2);
  if (NewDenom != denom1) {
    multiplyByConstant(Result, NewDenom / denom1, false);
  }
  if (NewDenom != denom2) {
    // Cannot avoid cloning CE2 here
    CanonExpr *NewCE2 = CE2->clone();
    multiplyByConstant(NewCE2, NewDenom / denom2, false);
  }
  Result->setDenominator(NewDenom);

  // Add the IV's
  // We process the CE2 IV's as Result gets updated
  int64_t Level = 1;
  for (auto I = CE2->iv_cbegin(), End = CE2->iv_cend(); I != End;
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
      // Result and CE2 have constant Coeff
      Result->addIV(Level, I->Coeff);
    } else {
      // Handle cases when either of them is a blob
      HIRParser *HIRP = getHIRParserPtr();
      CanonExpr::BlobTy Blob1 = isResultIVBlobCoeff
                                    ? Result->getBlob(ResultIVBlobCoeff)
                                    : HIRP->createBlob(ResultIVBlobCoeff);

      CanonExpr::BlobTy Blob2 =
          I->IsBlobCoeff ? CE2->getBlob(I->Coeff) : HIRP->createBlob(I->Coeff);
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
  for (auto I = CE2->blob_cbegin(), End = CE2->blob_cend(); I != End; ++I) {

    if (I->Coeff == 0)
      continue;

    // Add this blob to Result
    // The addBlob method will automatically take care if no blob exist
    Result->addBlob(I->Index, I->Coeff);
  }

  // Add the constant
  Result->Const += CE2->Const;

  return Result;
}

CanonExpr *CanonExprUtils::multiplyByConstant(CanonExpr *CE1, int64_t Val,
                                              bool CreateNewCE) {

  assert(CE1 && " Canon Expr parameter is null");

  // TODO: Multiply by 0
  // This should be taken care by a clear out method call for CanonExpr

  CanonExpr *Result = CreateNewCE ? CE1->clone() : CE1;

  // Result will be the same for multiply by 1
  if (Val == 1)
    return Result;

  // Multiply Val by IVCoeff, BlobCoeffs and Const
  int64_t Level = 1;
  for (auto I = Result->iv_begin(), End = Result->iv_end(); I != End;
       ++I, ++Level) {
    if (I->Coeff == 0)
      continue;

    if (!I->IsBlobCoeff) {
      // IV doesn't have Blob Coeff
      I->Coeff *= Val;
      continue;
    }

    // IV is a blob coeff
    CanonExpr::BlobTy ValBlob = getHIRParserPtr()->createBlob(Val);
    unsigned ResultBIndex;
    getHIRParserPtr()->createMulBlob(Result->getBlob(I->Coeff),
                                     ValBlob, true, &ResultBIndex);
    Result->setIVCoeff(Level, ResultBIndex, true);
  }

  for (auto I = Result->blob_begin(), End = Result->blob_end(); I != End; ++I) {
    I->Coeff *= Val;
  }

  Result->Const *= Val;

  return Result;
}

CanonExpr *CanonExprUtils::negate(CanonExpr *CE1, bool CreateNewCE) {
  // Result = -CE1
  return multiplyByConstant(CE1, -1, CreateNewCE);
}

CanonExpr *CanonExprUtils::subtract(CanonExpr *CE1, const CanonExpr *CE2,
                                    bool CreateNewCE) {

  assert((CE1 && CE2) && " Canon Expr parameters are null");

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
