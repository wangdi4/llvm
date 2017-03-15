//===--- CanonExprUtils.cpp - Implements CanonExprUtils class -------------===//
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
// This file implements CanonExprUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"

#define DEBUG_TYPE "hir-canon-utils"

using namespace llvm;
using namespace loopopt;

Function &CanonExprUtils::getFunction() const {
  return getHIRParser().getFunction();
}

Module &CanonExprUtils::getModule() const { return getHIRParser().getModule(); }

LLVMContext &CanonExprUtils::getContext() const {
  return getHIRParser().getContext();
}

const DataLayout &CanonExprUtils::getDataLayout() const {
  return getHIRParser().getDataLayout();
}

CanonExpr *CanonExprUtils::createCanonExpr(Type *Ty, unsigned Level,
                                           int64_t Const, int64_t Denom,
                                           bool IsSignedDiv) {
  return new CanonExpr(*this, Ty, Ty, false, Level, Const, Denom, IsSignedDiv);
}

CanonExpr *CanonExprUtils::createExtCanonExpr(Type *SrcType, Type *DestType,
                                              bool IsSExt, unsigned Level,
                                              int64_t Const, int64_t Denom,
                                              bool IsSignedDiv) {
  return new CanonExpr(*this, SrcType, DestType, IsSExt, Level, Const, Denom,
                       IsSignedDiv);
}

CanonExpr *CanonExprUtils::createCanonExpr(Type *Ty, APInt Value) {
  // make apint into a CanonExpr
  int64_t Val = Value.getSExtValue();
  return createCanonExpr(Ty, 0, Val);
}

void CanonExprUtils::destroy(CanonExpr *CE) {
  auto Count = Objs.erase(CE);
  assert(Count && "CE not found in objects!");
  delete CE;
}

void CanonExprUtils::destroyAll() {
  for (auto &I : Objs) {
    delete I;
  }

  Objs.clear();
}

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

  auto GCD = APIntOps::GreatestCommonDivisor(APInt(64, A), APInt(64, B));

  return GCD.getZExtValue();
}

// Internal Method that calculates the lcm of two positive integers
int64_t CanonExprUtils::lcm(int64_t A, int64_t B) {
  // If A and B are both big numbers, LCM can overflow. We can avoid artificial
  // overflow if we divide by GCD first before multiplying A and B. For example,
  // when both A and B are the same but (A * B) overflows.
  APInt Result(64, A / gcd(A, B), true);
  APInt Op(64, B, true);
  bool Overflows = false;

  Result = Result.smul_ov(Op, Overflows);

  return Overflows ? 0 : Result.getZExtValue();
}

CanonExpr *CanonExprUtils::createSelfBlobCanonExpr(Value *Val,
                                                   unsigned Symbase) {
  unsigned Index = 0;

  getBlobUtils().createBlob(Val, Symbase, true, &Index);
  auto CE = createSelfBlobCanonExpr(Index, NonLinearLevel);

  return CE;
}

CanonExpr *CanonExprUtils::createMetadataCanonExpr(MetadataAsValue *Val) {
  unsigned Index;

  getBlobUtils().createBlob(Val, ConstantSymbase, true, &Index);
  auto CE = createStandAloneBlobCanonExpr(Index, 0);

  return CE;
}

CanonExpr *CanonExprUtils::createStandAloneBlobCanonExpr(unsigned Index,
                                                         unsigned Level) {
  auto Blob = getBlobUtils().getBlob(Index);

  assert((BlobUtils::isTempBlob(Blob) ||
          BlobUtils::isMetadataBlob(Blob) ||
          BlobUtils::isConstantVectorBlob(Blob)) &&
         "Unexpected temp blob!");
  assert(isValidDefLevel(Level) && "Invalid level!");

  auto CE = createCanonExpr(Blob->getType());
  CE->addBlob(Index, 1);

  if (Level == NonLinearLevel) {
    CE->setNonLinear();
  } else {
    CE->setDefinedAtLevel(Level);
  }

  return CE;
}

uint64_t CanonExprUtils::getTypeSizeInBits(Type *Ty) const {
  return getHIRParser().getDataLayout().getTypeSizeInBits(Ty);
}

bool CanonExprUtils::isTypeEqual(const CanonExpr *CE1, const CanonExpr *CE2,
                                 bool RelaxedMode) {
  return (CE1->getSrcType() == CE2->getSrcType()) &&
         (RelaxedMode || (CE1->getDestType() == CE2->getDestType() &&
                          (CE1->isSExt() == CE2->isSExt())));
}

bool CanonExprUtils::mergeable(const CanonExpr *CE1, const CanonExpr *CE2,
                               bool RelaxedMode) {

  // TODO: allow merging if only one of the CEs has a distinct destination type?
  if (!isTypeEqual(CE1, CE2, RelaxedMode)) {
    return canMergeConstants(CE1, CE2, RelaxedMode);
  }

  // TODO: Look into the safety of merging signed/unsigned divisions.
  // We allow merging if one of the denominators is 1 even if the signed
  // division flag is different. The merged canon expr takes the flag from the
  // canon expr with non-unit denominator.
  if ((CE1->getDenominator() != 1) && (CE2->getDenominator() != 1)) {
    return (CE1->isSignedDiv() == CE2->isSignedDiv());
  }

  return true;
}

bool CanonExprUtils::areEqual(const CanonExpr *CE1, const CanonExpr *CE2,
                              bool RelaxedMode) {

  assert((CE1 && CE2) && " Canon Expr parameters are null");

  // Match the types.
  if (!mergeable(CE1, CE2, RelaxedMode)) {
    return false;
  }

  // Match defined at level.
  if ((CE1->isNonLinear() != CE2->isNonLinear()) ||
      (!CE1->isNonLinear() &&
       CE1->getDefinedAtLevel() != CE2->getDefinedAtLevel())) {
    return false;
  }

  if ((CE1->Const != CE2->Const) ||
      (CE1->getDenominator() != CE2->getDenominator())) {
    return false;
  }

  // Check the number of blobs.
  if (CE1->numBlobs() != CE2->numBlobs()) {
    return false;
  }

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

bool CanonExprUtils::canMergeConstants(const CanonExpr *CE1,
                                       const CanonExpr *CE2, bool RelaxedMode) {

  if (!RelaxedMode) {
    return false;
  }

  // Check: vector type not supported
  if (isa<VectorType>(CE2->getSrcType()) ||
      isa<VectorType>(CE1->getSrcType())) {
    return false;
  }

  // Check if either is constant.
  int64_t Val1 = 0, Val2 = 0;
  bool IsCE1Const = CE1->isIntConstant(&Val1);
  bool IsCE2Const = CE2->isIntConstant(&Val2);
  if (!IsCE1Const && !IsCE2Const) {
    return false;
  }

  // Check for zero condition.
  // These can be merged.
  if ((IsCE1Const && Val1 == 0) || (IsCE2Const && Val2 == 0)) {
    return true;
  }

  // Check for overflow condition.
  // TODO: add it

  // Check if we can update the type of canon expr without
  // loss of information.
  if (IsCE1Const) {
    return (ConstantInt::isValueValidForType(CE2->getSrcType(), Val1) &&
            ConstantInt::isValueValidForType(CE1->getDestType(), Val1));
  } else {
    assert(IsCE2Const && " CE2 should be a int constant.");
    return (ConstantInt::isValueValidForType(CE1->getSrcType(), Val2) &&
            ConstantInt::isValueValidForType(CE2->getDestType(), Val2));
  }
}

void CanonExprUtils::updateConstantTypes(CanonExpr *CE1, CanonExpr **CE2,
                                         bool RelaxedMode, bool *CreatedAuxCE) {
  // Sanity
  assert((CE1 && CE2 && CreatedAuxCE) && "Not expecting any nullptr\n");

  // Skip if RelaxedMode is false
  if (!RelaxedMode) {
    return;
  }

  // Skip if CE1 and CE2's types match
  if (isTypeEqual(CE1, *CE2, true)) {
    return;
  }

  int64_t Val1 = 0, Val2 = 0;
  bool IsCE1Const = CE1->isIntConstant(&Val1);
  bool IsCE2Const = (*CE2)->isIntConstant(&Val2);

  // Check if either is a constant
  if (!IsCE1Const && !IsCE2Const) {
    llvm_unreachable("Expect at least 1 constant between CE1 and CE2\n");
    return;
  }

  if (IsCE1Const) {
    assert(ConstantInt::isValueValidForType((*CE2)->getSrcType(), Val1) &&
           ConstantInt::isValueValidForType(CE1->getDestType(), Val1) &&
           " Constant value cannot be updated.");
    CE1->setSrcType((*CE2)->getSrcType());
  } else {
    assert(ConstantInt::isValueValidForType(CE1->getSrcType(), Val2) &&
           ConstantInt::isValueValidForType((*CE2)->getDestType(), Val2) &&
           " Constant value cannot be updated.");
    // Cannot avoid cloning here.
    *CE2 = (*CE2)->clone();
    (*CE2)->setSrcType(CE1->getSrcType());
    *CreatedAuxCE = true;
  }
}

bool CanonExprUtils::addImpl(CanonExpr *CE1, const CanonExpr *CE2,
                             bool RelaxedMode) {

  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  bool CreatedAuxCE = false;

  CanonExpr *NewCE2 = const_cast<CanonExpr *>(CE2);

  if (CE2->isZero()) {
    return true;
  }

  bool IsMergeable = mergeable(CE1, NewCE2, RelaxedMode);
  // assert(IsMergeable && " Canon Expr are not mergeable!");
  // Bail out if we cannot merge the canon expr.
  if (!IsMergeable) {
    return false;
  }

  updateConstantTypes(CE1, &NewCE2, RelaxedMode, &CreatedAuxCE);

  // Process the denoms.
  int64_t Denom1 = CE1->getDenominator();
  int64_t Denom2 = NewCE2->getDenominator();
  int64_t NewDenom = lcm(Denom1, Denom2);

  // Bail out if LCM overflows signed 64 bit range.
  if (NewDenom == 0) {
    return false;
  }

  if (NewDenom != Denom1) {
    // Do not simplify while multiplying as this is an intermediate result of
    // add.
    CE1->multiplyNumeratorByConstant(NewDenom / Denom1, false);

    // Since the denominator has changed, we should set the flag based on CE2.
    // This is safe to do because the division type difference is only allowed
    // if one of the denominators is 1 which in this case is Denom1.
    CE1->setDivisionType(CE2->isSignedDiv());
  }
  if (NewDenom != Denom2) {
    // Cannot avoid cloning CE2 here
    if (!CreatedAuxCE) {
      NewCE2 = NewCE2->clone();
      CreatedAuxCE = true;
    }
    // Do not simplify while multiplying as this is an intermediate result of
    // add.
    NewCE2->multiplyNumeratorByConstant(NewDenom / Denom2, false);
  }

  CE1->setDenominator(NewDenom);

  // Add NewCE2's IVs to Result.
  for (auto I = NewCE2->iv_begin(), End = NewCE2->iv_end(); I != End; ++I) {
    if (I->Coeff == 0) {
      continue;
    }

    CE1->addIV(NewCE2->getLevel(I), I->Index, I->Coeff);
  }

  // Add NewCE2's Blobs to Result.
  for (auto I = NewCE2->blob_begin(), End = NewCE2->blob_end(); I != End; ++I) {
    if (I->Coeff == 0) {
      continue;
    }

    CE1->addBlob(I->Index, I->Coeff);
  }

  // Add the constant.
  int64_t CVal = CE1->getConstant() + NewCE2->getConstant();
  CE1->setConstant(CVal);

  // Update DefinedAtLevel.
  if (NewCE2->isNonLinear()) {
    CE1->setNonLinear();

  } else if (!CE1->isNonLinear() &&
             NewCE2->getDefinedAtLevel() > CE1->getDefinedAtLevel()) {
    CE1->setDefinedAtLevel(NewCE2->getDefinedAtLevel());
  }

  // Destroy auxiliary canon expr.
  if (CreatedAuxCE) {
    NewCE2->getCanonExprUtils().destroy(NewCE2);
  }

  return true;
}

bool CanonExprUtils::add(CanonExpr *CE1, const CanonExpr *CE2,
                               bool RelaxedMode) {
  return addImpl(CE1, CE2, RelaxedMode);
}

CanonExpr *CanonExprUtils::cloneAndAdd(const CanonExpr *CE1,
                                       const CanonExpr *CE2, bool RelaxedMode) {
  CanonExpr *Clone = CE1->clone();

  if (!addImpl(Clone, CE2, RelaxedMode)) {
    CE1->getCanonExprUtils().destroy(Clone);
    return nullptr;
  }

  return Clone;
}

bool CanonExprUtils::subtract(CanonExpr *CE1, const CanonExpr *CE2,
                              bool RelaxedMode) {
  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  // Here, we avoid cloning by doing negation twice.
  // -(-CE1+CE2) => CE1-CE2
  CE1->negate();
  if (!add(CE1, CE2, RelaxedMode)) {
    return false;
  }
  CE1->negate();
  return true;
}

CanonExpr *CanonExprUtils::cloneAndSubtract(const CanonExpr *CE1,
                                            const CanonExpr *CE2,
                                            bool RelaxedMode) {
  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  // Result = -CE2 + CE1
  CanonExpr *Result = cloneAndNegate(CE2);
  if (!add(Result, CE1, RelaxedMode)) {
    return nullptr;
  }
  Result->setDestType(CE1->getDestType());

  return Result;
}

CanonExpr *CanonExprUtils::cloneAndNegate(const CanonExpr *CE) {
  assert(CE && " Canon Expr is null!");

  CanonExpr *Result = CE->clone();
  Result->negate();

  return Result;
}

bool CanonExprUtils::hasNonLinearSemantics(unsigned DefLevel,
                                           unsigned NestingLevel) {
  assert(isValidDefLevel(DefLevel) && "DefLevel is invalid!");
  assert(isValidLinearDefLevel(NestingLevel) && "NestingLevel is invalid!");

  return ((DefLevel == NonLinearLevel) ||
          (DefLevel && (DefLevel >= NestingLevel)));
}

bool CanonExprUtils::replaceIVByCanonExpr(CanonExpr *CE1, unsigned Level,
                                          const CanonExpr *CE2,
                                          bool RelaxedMode) {
  assert(mergeable(CE1, CE2, RelaxedMode) && "CanonExprs are not mergeable");

  // CE1 = C1*B1*i1 + C3*i2 + ..., Level 1
  // CE2 = C2*B2

  auto ConstCoeff = CE1->getIVConstCoeff(Level);
  if (ConstCoeff == 0) {
    return true;
  }

  // If CE2 is zero - just remove the IV
  if (CE2->isZero()) {
    CE1->removeIV(Level);
    return true;
  }

  std::unique_ptr<CanonExpr> Term(CE2->clone());

  // It's safe to change the Term type as CE1 and CE2 are mergeable.
  Term->setSrcAndDestType(CE1->getSrcType());

  // CE2 <- CE2 * C1
  if (!Term->multiplyByConstant(ConstCoeff)) {
    return false;
  }

  auto BlobCoeff = CE1->getIVBlobCoeff(Level);
  if (CE1->getBlobUtils().isBlobIndexValid(BlobCoeff)) {
    // CE2 <- CE2 * B1
    if (!Term->multiplyByBlob(BlobCoeff)) {
      return false;
    }
  }

  CE1->removeIV(Level);
  // At this point:
  // CE1 = C3*i2 + ...
  // CE2 = C1*C2 * B1*B2

  // Set denominator from CE1 to CE2
  Term->divide(CE1->getDenominator());

  // CE1 = C1*C2 * B1*B2 + C3*i2 + ...
  if (!add(CE1, Term.get(), RelaxedMode)) {
    return false;
  }

  return true;
}

bool CanonExprUtils::getConstIterationDistance(const CanonExpr *CE1,
                                               const CanonExpr *CE2,
                                               unsigned LoopLevel,
                                               int64_t *Distance) {
  assert(isValidLoopLevel(LoopLevel) && "Invalid loop level!");

  int64_t Coeff1, Coeff2;
  unsigned Index1, Index2;

  CE1->getIVCoeff(LoopLevel, &Index1, &Coeff1);
  CE2->getIVCoeff(LoopLevel, &Index2, &Coeff2);

  if ((Coeff1 != Coeff2) || (Index1 != Index2)) {
    return false;
  }

  // Both CE1 and CE2 are invariant w.r.t IV. Return distance of 0 if both are
  // equal.
  if (!Coeff1) {
    *Distance = 0;
    return areEqual(CE1, CE2);
  }

  std::unique_ptr<CanonExpr> Result(cloneAndSubtract(CE1, CE2));

  // Subtraction failed, return false.
  if (!Result) {
    return false;
  }

  if (Result->hasIV() || (Result->getDenominator() > 1)) {
    return false;
  }

  auto NumBlobs = Result->numBlobs();
  int64_t Diff = Result->getConstant();

  if (NumBlobs > 1) {
    return false;

  } else if (NumBlobs == 1) {

    // When IV has a blob coefficient, the diff is in the form of a single
    // blob.
    // For example-
    // CE1 = 2*%b*i1 + 2*%b
    // CE2 = 2*%b*i1
    // CE1 - CE2 = 2*%b
    // Distance = 1
    if (Diff != 0) {
      return false;
    }

    auto BlobIndex = Result->getSingleBlobIndex();

    if (BlobIndex != Index1) {
      return false;
    }

    Diff = Result->getSingleBlobCoeff();

  } else if (Index1 != InvalidBlobIndex) {
    // IV has a blob coefficient so Result should have a blob term to have valid
    // iteration distance.
    assert((NumBlobs == 0) && "Unexpected nuumber of blobs in result!");

    // Allow identical CEs to pass through.
    if (Diff != 0) {
      return false;
    }
  }

  if ((Diff % std::llabs(Coeff1)) != 0) {
    return false;
  }

  *Distance = Diff / Coeff1;

  return true;
}

bool CanonExprUtils::getConstDistance(const CanonExpr *CE1,
                                      const CanonExpr *CE2, int64_t *Distance) {
  std::unique_ptr<CanonExpr> Result(cloneAndSubtract(CE1, CE2));

  // Subtract operation can fail.
  if (!Result) {
    return false;
  }

  if (Result->hasIV() || Result->hasBlob() || (Result->getDenominator() > 1)) {
    return false;
  }

  *Distance = Result->getConstant();
  return true;
}
