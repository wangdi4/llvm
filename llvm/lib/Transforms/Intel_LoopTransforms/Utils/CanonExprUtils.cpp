//===--- CanonExprUtils.cpp - Implements CanonExprUtils class -------------===//
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

HIRParser *HLUtils::HIRPar(nullptr);

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

unsigned CanonExprUtils::findBlob(CanonExpr::BlobTy Blob) {
  return CanonExpr::findBlob(Blob);
}

unsigned CanonExprUtils::findOrInsertBlob(CanonExpr::BlobTy Blob) {
  return CanonExpr::findOrInsertBlob(Blob);
}

CanonExpr::BlobTy CanonExprUtils::getBlob(unsigned BlobIndex) {
  return CanonExpr::getBlob(BlobIndex);
}

void CanonExprUtils::printBlob(raw_ostream &OS, CanonExpr::BlobTy Blob,
                               bool Detailed) {
  getHIRParser()->printBlob(OS, Blob, Detailed);
}

void CanonExprUtils::printScalar(raw_ostream &OS, unsigned Symbase,
                                 bool Detailed) {
  getHIRParser()->printScalar(OS, Symbase, Detailed);
}

bool CanonExprUtils::isConstantIntBlob(CanonExpr::BlobTy Blob, int64_t *Val) {
  return getHIRParser()->isConstantIntBlob(Blob, Val);
}

bool CanonExprUtils::isTempBlob(CanonExpr::BlobTy Blob) {
  return getHIRParser()->isTempBlob(Blob);
}

CanonExpr::BlobTy CanonExprUtils::createBlob(Value *Val, bool Insert,
                                             unsigned *NewBlobIndex) {
  return getHIRParser()->createBlob(Val, Insert, NewBlobIndex);
}

CanonExpr::BlobTy CanonExprUtils::createBlob(int64_t Val, bool Insert,
                                             unsigned *NewBlobIndex) {
  return getHIRParser()->createBlob(Val, Insert, NewBlobIndex);
}

CanonExpr::BlobTy CanonExprUtils::createAddBlob(CanonExpr::BlobTy LHS,
                                                CanonExpr::BlobTy RHS,
                                                bool Insert,
                                                unsigned *NewBlobIndex) {
  return getHIRParser()->createAddBlob(LHS, RHS, Insert, NewBlobIndex);
}

CanonExpr::BlobTy CanonExprUtils::createMinusBlob(CanonExpr::BlobTy LHS,
                                                  CanonExpr::BlobTy RHS,
                                                  bool Insert,
                                                  unsigned *NewBlobIndex) {
  return getHIRParser()->createMinusBlob(LHS, RHS, Insert, NewBlobIndex);
}

CanonExpr::BlobTy CanonExprUtils::createMulBlob(CanonExpr::BlobTy LHS,
                                                CanonExpr::BlobTy RHS,
                                                bool Insert,
                                                unsigned *NewBlobIndex) {
  return getHIRParser()->createMulBlob(LHS, RHS, Insert, NewBlobIndex);
}

CanonExpr::BlobTy CanonExprUtils::createUDivBlob(CanonExpr::BlobTy LHS,
                                                 CanonExpr::BlobTy RHS,
                                                 bool Insert,
                                                 unsigned *NewBlobIndex) {
  return getHIRParser()->createUDivBlob(LHS, RHS, Insert, NewBlobIndex);
}

CanonExpr::BlobTy CanonExprUtils::createTruncateBlob(CanonExpr::BlobTy Blob,
                                                     Type *Ty, bool Insert,
                                                     unsigned *NewBlobIndex) {
  return getHIRParser()->createTruncateBlob(Blob, Ty, Insert, NewBlobIndex);
}

CanonExpr::BlobTy CanonExprUtils::createZeroExtendBlob(CanonExpr::BlobTy Blob,
                                                       Type *Ty, bool Insert,
                                                       unsigned *NewBlobIndex) {
  return getHIRParser()->createZeroExtendBlob(Blob, Ty, Insert, NewBlobIndex);
}

CanonExpr::BlobTy CanonExprUtils::createSignExtendBlob(CanonExpr::BlobTy Blob,
                                                       Type *Ty, bool Insert,
                                                       unsigned *NewBlobIndex) {
  return getHIRParser()->createSignExtendBlob(Blob, Ty, Insert, NewBlobIndex);
}

CanonExpr *CanonExprUtils::createSelfBlobCanonExpr(Value *Val) {
  unsigned Index;

  getHIRParser()->createBlob(Val, true, &Index);
  auto CE = createCanonExpr(Val->getType());
  CE->addBlob(Index, 1);
  CE->setNonLinear();

  return CE;
}

bool CanonExprUtils::isTypeEqual(const CanonExpr *CE1, const CanonExpr *CE2) {
  return (CE1->getType() == CE2->getType());
}

bool CanonExprUtils::areEqual(const CanonExpr *CE1, const CanonExpr *CE2) {

  assert((CE1 && CE2) && " Canon Expr parameters are null");

  // Match the types.
  if (!isTypeEqual(CE1, CE2))
    return false;

  if ((CE1->Const != CE2->Const) ||
      (CE1->getDenominator() != CE2->getDenominator())) {
    return false;
  }

  // Check the number of blobs.
  if (CE1->numBlobs() != CE2->numBlobs())
    return false;

  auto IVIt1 = CE1->iv_begin();
  auto IVIt2 = CE2->iv_begin();
  auto End1 = CE1->iv_end();
  auto End2 = CE2->iv_end();

  // Check the IV's.
  for (; IVIt1 != End1 && IVIt2 != End2; ++IVIt1, ++IVIt2) {
    if ((IVIt1->Index != IVIt2->Index) || (IVIt1->Coeff != IVIt2->Coeff)) {
      return false;
    }
  }

  // If CE1 has some IV iterators left check whether they are all zeroes.
  for (; IVIt1 != End1; ++IVIt1) {
    if (IVIt1->Coeff) {
      return false;
    }
  }

  // If CE2 has some IV iterators left check whether they are all zeroes.
  for (; IVIt2 != End2; ++IVIt2) {
    if (IVIt2->Coeff) {
      return false;
    }
  }

  // Iterate through the blobs as both have same size.
  for (auto I1 = CE1->blob_begin(), End = CE1->blob_end(),
            I2 = CE2->blob_begin();
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
  bool CreatedAuxCE = false;

  // Process the denoms.
  int64_t Denom1 = Result->getDenominator();
  int64_t Denom2 = CE2->getDenominator();
  int NewDenom = lcm(Denom1, Denom2);

  if (NewDenom != Denom1) {
    // Do not simplify while multiplying as this is an intermediate result of
    // add.
    Result->multiplyByConstantImpl(NewDenom / Denom1, false);
  }
  if (NewDenom != Denom2) {
    // Cannot avoid cloning CE2 here
    NewCE2 = CE2->clone();
    // Do not simplify while multiplying as this is an intermediate result of
    // add.
    NewCE2->multiplyByConstantImpl(NewDenom / Denom2, false);
    CreatedAuxCE = true;
  }

  Result->setDenominator(NewDenom);

  // Add NewCE2's IVs to Result.
  for (auto I = NewCE2->iv_begin(), End = NewCE2->iv_end(); I != End; ++I) {
    if (I->Coeff == 0)
      continue;

    Result->addIV(NewCE2->getLevel(I), I->Index, I->Coeff);
  }

  // Add NewCE2's Blobs to Result.
  for (auto I = NewCE2->blob_begin(), End = NewCE2->blob_end(); I != End; ++I) {
    if (I->Coeff == 0)
      continue;

    Result->addBlob(I->Index, I->Coeff);
  }

  // Add the constant.
  Result->setConstant(Result->getConstant() + NewCE2->getConstant());

  // Simplify resulting canon expr before returning.
  Result->simplify();

  // Destroy auxiliary canon expr.
  if (CreatedAuxCE) {
    NewCE2->destroy();
  }

  return Result;
}

CanonExpr *CanonExprUtils::subtract(CanonExpr *CE1, const CanonExpr *CE2,
                                    bool CreateNewCE) {

  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  CanonExpr *Result;

  if (CreateNewCE) {
    // Result = -CE2 + CE1
    Result = CE2->clone();
    Result->negate();
    Result = add(Result, CE1, false);
  } else {
    // -(-CE1+CE2) => CE1-CE2
    // Here, we avoid cloning by doing negation twice.
    Result = CE1;
    Result->negate();
    Result = add(Result, CE2, false);
    Result->negate();
  }

  return Result;
}

CanonExpr *CanonExprUtils::negate(CanonExpr *CE1, bool CreateNewCE) {
  assert(CE1 && " Canon Expr is null!");

  CanonExpr *Result;

  if (CreateNewCE) {
    Result = CE1->clone();
  } else {
    Result = CE1;
  }

  Result->negate();

  return Result;
}
