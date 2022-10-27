//===--- CanonExprUtils.cpp - Implements CanonExprUtils class -------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"

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
  (void)Count;
  assert(Count && "CE not found in objects!");
  delete CE;
}

CanonExprUtils::~CanonExprUtils() {
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

CanonExpr *CanonExprUtils::createConstStandAloneBlobCanonExpr(Value *Val) {
  assert((isa<MetadataAsValue>(Val) || isa<ConstantData>(Val) ||
          isa<ConstantVector>(Val)) &&
         "Unexpected constant type!");
  unsigned Index;

  getBlobUtils().createBlob(Val, ConstantSymbase, true, &Index);
  auto CE = createStandAloneBlobCanonExpr(Index, 0);

  return CE;
}

CanonExpr *CanonExprUtils::createStandAloneBlobCanonExpr(unsigned Index,
                                                         unsigned Level) {
  auto Blob = getBlobUtils().getBlob(Index);
  assert(CanonExpr::isValidDefLevel(Level) && "Invalid level!");

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
  return getHIRParser().getDataLayout().getTypeAllocSizeInBits(Ty);
}

uint64_t CanonExprUtils::getTypeSizeInBytes(Type *Ty) const {
  return getHIRParser().getDataLayout().getTypeAllocSize(Ty);
}

bool CanonExprUtils::isTypeEqual(const CanonExpr *CE1, const CanonExpr *CE2,
                                 bool RelaxedMode) {

  Type *Ty1 = CE1->getSrcType();
  Type *Ty2 = CE2->getSrcType();

  // Return true for Ty1 = <2 x i64> and Ty2 = i64.
  // Vector CEs are allowed to contain scalar terms for ease of analysis. These
  // are widened during CG so we can allow merging of a scalar CE into a vector
  // CE. This check is asymmetric because we don't want to merge vector CE into
  // scalar CE.
  // This logic breaks non-relaxed areEqual() functionality. The problem is that
  // this function is being used to both compare and merge (add/subtract) CEs.
  // The workaround is in areEqual().
  if (Ty1->isVectorTy() && !Ty2->isVectorTy()) {
    Ty1 = Ty1->getScalarType();
  }

  bool SameSrcType = (Ty1 == Ty2);
  // Change related to normalize i2+2 in sitofp.i32.float(i2 + 2) to
  // (64 * i1 + i2 + 2).
  if (RelaxedMode && !CE1->hasBlob() && !CE2->hasBlob() &&
      CE1->getDenominator() == 1 && CE2->getDenominator() == 1 &&
      !CE1->hasIVBlobCoeffs() && !CE2->hasIVBlobCoeffs()) {
    return true;
  }

  return (SameSrcType &&
          (RelaxedMode || ((CE1->getDestType()->getScalarType() ==
                            CE2->getDestType()->getScalarType()) &&
                           (CE1->isSExt() == CE2->isSExt()))));
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
                              bool RelaxedMode, bool IgnoreDefAtLevel) {

  assert((CE1 && CE2) && " Canon Expr parameters are null");

  // Match the types.
  // isTypeEqual() can return true for vector CE1 and scalar CE2 in non-relaxed
  // mode but we should not return true here.
  if (!RelaxedMode) {
    if (CE1->getSrcType() != CE2->getSrcType()) {
      return false;
    }

    if (CE1->getDestType() != CE2->getDestType()) {
      return false;
    }

  } else if (!isTypeEqual(CE1, CE2, true)) {
    return false;
  }

  // Match defined at level.
  if (!IgnoreDefAtLevel) {
    if ((CE1->isNonLinear() != CE2->isNonLinear()) ||
        (!CE1->isNonLinear() &&
         CE1->getDefinedAtLevel() != CE2->getDefinedAtLevel())) {
      return false;
    }
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

void CanonExprUtils::updateSrcType(CanonExpr *CE1, const CanonExpr *CE2,
                                   bool RelaxedMode) {
  // Sanity
  assert((CE1 && CE2) && "Not expecting any nullptr\n");

  // Skip if RelaxedMode is false
  if (!RelaxedMode) {
    return;
  }

  // Skip if CE1 and CE2's types match
  if (isTypeEqual(CE1, CE2, true)) {
    return;
  }

  int64_t Val1 = 0, Val2 = 0;
  bool IsCE1Const = CE1->isIntConstant(&Val1);
  bool IsCE2Const = CE2->isIntConstant(&Val2);

  // Check if either is a constant
  if (!IsCE1Const && !IsCE2Const) {
    llvm_unreachable("Expect at least 1 constant between CE1 and CE2\n");
    return;
  }

  if (IsCE1Const) {
    assert(ConstantInt::isValueValidForType(CE2->getSrcType(), Val1) &&
           ConstantInt::isValueValidForType(CE1->getDestType(), Val1) &&
           " Constant value cannot be updated.");
    CE1->setSrcType(CE2->getSrcType());
  } else {
    assert(ConstantInt::isValueValidForType(CE1->getSrcType(), Val2) &&
           ConstantInt::isValueValidForType(CE2->getDestType(), Val2) &&
           " Constant value cannot be updated.");
  }
}

bool CanonExprUtils::canAdd(const CanonExpr *CE1, const CanonExpr *CE2,
                            bool RelaxedMode) {
  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  if (CE2->isZero()) {
    return true;
  }

  if (!mergeable(CE1, CE2, RelaxedMode)) {
    return false;
  }

  if (lcm(CE1->getDenominator(), CE2->getDenominator()) == 0) {
    return false;
  }

  // In non-RelaxedMode the canon expressions source and destination types need
  // to match. The mergeable check already checks that the source types match
  // for this case. Here we check that the dest types of CE1 and CE2 match CE1
  // src type.
  // TODO: This check needs to be moved to the checks in mergeable function. The
  // check is being done here to limit its impact.
  auto *SrcType = CE1->getSrcType();
  if (!RelaxedMode &&
      (CE1->getDestType() != SrcType || CE2->getDestType() != SrcType)) {
    return false;
  }

  return true;
}

bool CanonExprUtils::canSubtract(const CanonExpr *CE1, const CanonExpr *CE2,
                                 bool RelaxedMode) {
  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  // const_cast to prevent memory allocation. CE2 will be reverted before
  // returning.
  const_cast<CanonExpr *>(CE2)->negate();

  bool Res = canAdd(CE1, CE2, RelaxedMode);

  // Revert CE2 to original state.
  const_cast<CanonExpr *>(CE2)->negate();

  return Res;
}

void CanonExprUtils::addImpl(CanonExpr *CE1, const CanonExpr *CE2,
                             bool RelaxedMode) {
  assert(canAdd(CE1, CE2, RelaxedMode) && "Cannot add CE1 and CE2");

  if (CE2->isZero()) {
    return;
  }

  updateSrcType(CE1, CE2, RelaxedMode);

  CanonExpr *NewCE2 = const_cast<CanonExpr *>(CE2);
  bool CreatedAuxCE = false;

  // Process the denoms.
  int64_t Denom1 = CE1->getDenominator();
  int64_t Denom2 = NewCE2->getDenominator();
  int64_t NewDenom = lcm(Denom1, Denom2);

  assert((NewDenom != 0) && "LCM of denominators overflows!");

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
    NewCE2 = NewCE2->clone();
    CreatedAuxCE = true;

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
}

void CanonExprUtils::subtractImpl(CanonExpr *CE1, const CanonExpr *CE2,
                                  bool RelaxedMode) {
  assert(canSubtract(CE1, CE2, RelaxedMode) && "Cannot subtract CE1 and CE2");

  // The result of canSubtract(CE1, CE2) and canSubtract(CE2, CE1) can be
  // different in RelaxedMode. For stability reasons, we negate CE2 and
  // implement this as (CE1 + -CE2) rather than negating CE1 and implementing it
  // as (-(-CE1 + CE2)) to preserve const correctness.
  const_cast<CanonExpr *>(CE2)->negate();

  addImpl(CE1, CE2, RelaxedMode);

  // Revert CE2 to original state.
  const_cast<CanonExpr *>(CE2)->negate();
}

bool CanonExprUtils::add(CanonExpr *CE1, const CanonExpr *CE2,
                         bool RelaxedMode) {
  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  if (!canAdd(CE1, CE2, RelaxedMode)) {
    return false;
  }

  addImpl(CE1, CE2, RelaxedMode);
  return true;
}

CanonExpr *CanonExprUtils::cloneAndAdd(const CanonExpr *CE1,
                                       const CanonExpr *CE2, bool RelaxedMode) {
  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  if (!canAdd(CE1, CE2, RelaxedMode)) {
    return nullptr;
  }

  CanonExpr *Clone = CE1->clone();

  addImpl(Clone, CE2, RelaxedMode);

  return Clone;
}

bool CanonExprUtils::subtract(CanonExpr *CE1, const CanonExpr *CE2,
                              bool RelaxedMode) {
  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  if (!canSubtract(CE1, CE2, RelaxedMode)) {
    return false;
  }

  subtractImpl(CE1, CE2, RelaxedMode);

  return true;
}

CanonExpr *CanonExprUtils::cloneAndSubtract(const CanonExpr *CE1,
                                            const CanonExpr *CE2,
                                            bool RelaxedMode) {
  assert((CE1 && CE2) && " Canon Expr parameters are null!");

  if (!canSubtract(CE1, CE2, RelaxedMode)) {
    return nullptr;
  }

  CanonExpr *Result = CE1->clone();

  subtractImpl(Result, CE2, RelaxedMode);

  return Result;
}

CanonExpr *CanonExprUtils::cloneAndNegate(const CanonExpr *CE) {
  assert(CE && " Canon Expr is null!");

  CanonExpr *Result = CE->clone();
  Result->negate();

  return Result;
}

bool CanonExprUtils::canReplaceIVByCanonExpr(const CanonExpr *CE1,
                                             unsigned Level,
                                             const CanonExpr *CE2,
                                             bool RelaxedMode) {
  // Perform cheap checks here to avoid allocating memory.
  if (!CE1->hasIV(Level) || CE2->isIntConstant()) {
    return true;
  }

  std::unique_ptr<CanonExpr> CE1Clone(CE1->clone());

  // IsSigned flag has no bearing on whether the replacement can be performed so
  // we can always pass it as false.
  return replaceIVByCanonExpr(CE1Clone.get(), Level, CE2, false, RelaxedMode);
}

bool CanonExprUtils::replaceIVByCanonExpr(CanonExpr *CE1, unsigned Level,
                                          const CanonExpr *CE2, bool IsSigned,
                                          bool RelaxedMode) {
  // CE1 = C1*B1*i1 + C3*i2 + ..., Level 1
  // CE2 = C2*B2
  assert(CE2->getDestType()->isIntegerTy() && "Invalid CE2 type!");

  auto ConstCoeff = CE1->getIVConstCoeff(Level);
  if (ConstCoeff == 0) {
    return true;
  }

  int64_t CE2Value;
  if (CE2->isIntConstant(&CE2Value)) {
    CE1->replaceIVByConstant(Level, CE2Value);
    return true;
  }

  // Try to merge first
  bool Mergeable = mergeable(CE1, CE2, RelaxedMode);
  if (!Mergeable) {
    if (!CE2->canConvertToStandAloneBlobOrConstant()) {
      // If can not be casted
      return false;
    }
  }

  std::unique_ptr<CanonExpr> Term(CE2->clone());

  if (!Mergeable) {
    // Not meargeable but could be casted to a blob with a correspondent type.
    // This allows merging into vector type CE.
    Term->convertToCastedStandAloneBlobOrConstant(
        CE1->getSrcType()->getScalarType(), IsSigned);
  }

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
  if (!add(CE1, Term.get(), /*RelaxedMode*/ true)) {
    return false;
  }

  return true;
}

void CanonExprUtils::replaceStandAloneBlobByCanonExpr(CanonExpr *CE1,
                                                      unsigned BlobIndex,
                                                      const CanonExpr *CE2) {
  assert(CE1->containsStandAloneBlob(BlobIndex) &&
         "Blob is not standalone in CE1!");
  assert(CE2->getDenominator() == 1 && "Cannot handle CE2 with denominator!");

  CE1->removeBlob(BlobIndex);

  CanonExprUtils::add(CE1, CE2, true);
}

bool CanonExprUtils::getConstIterationDistance(const CanonExpr *CE1,
                                               const CanonExpr *CE2,
                                               unsigned LoopLevel,
                                               int64_t *Distance,
                                               bool RelaxedMode) {
  assert(CanonExpr::isValidLoopLevel(LoopLevel) && "Invalid loop level!");

  int64_t Coeff1, Coeff2;
  unsigned Index1, Index2;

  CE1->getIVCoeff(LoopLevel, &Index1, &Coeff1);
  CE2->getIVCoeff(LoopLevel, &Index2, &Coeff2);

  if ((Coeff1 != Coeff2) || (Index1 != Index2) ||
      (CE1->getDenominator() != CE2->getDenominator())) {
    return false;
  }

  // Both CE1 and CE2 are invariant w.r.t IV. Return distance of 0 if both are
  // equal.
  if (!Coeff1) {
    if (areEqual(CE1, CE2, RelaxedMode)) {
      if (Distance) {
        *Distance = 0;
      }
      return true;
    }
    return false;
  }

  bool HasBlobCoeff = (Index1 != InvalidBlobIndex);
  int64_t StoredCoeff1 = 0, StoredCoeff2 = 0;

  if (HasBlobCoeff) {
    // When IV has a blob coefficient, the diff is in the form of a single blob.
    // For example-
    // CE1 = 2*%b*i1 + 2*%b
    // CE2 = 2*%b*i1
    // CE1 - CE2 = 2*%b
    // Distance = 1
    StoredCoeff1 = CE1->getBlobCoeff(Index1);
    StoredCoeff2 = CE2->getBlobCoeff(Index2);
  } else {
    StoredCoeff1 = CE1->getConstant();
    StoredCoeff2 = CE2->getConstant();
  }

  int64_t Diff = StoredCoeff1 - StoredCoeff2;
  Coeff1 = std::llabs(Coeff1);

  if ((Diff % Coeff1) != 0) {
    return false;
  }

  // We cheat to avoid cloning the CE.
  auto NonConstCE1 = const_cast<CanonExpr *>(CE1);
  auto NonConstCE2 = const_cast<CanonExpr *>(CE2);

  // Reset blob or constant field and compare CEs.
  if (HasBlobCoeff) {
    if (StoredCoeff1) {
      NonConstCE1->removeBlob(Index1);
    }
    if (StoredCoeff2) {
      NonConstCE2->removeBlob(Index1);
    }
  } else {
    NonConstCE1->setConstant(0);
    NonConstCE2->setConstant(0);
  }

  bool Res = false;

  if (areEqual(NonConstCE1, NonConstCE2, RelaxedMode)) {
    if (Distance) {
      *Distance = Diff / Coeff1;
    }
    Res = true;
  }

  // Restore CEs to original state.
  if (HasBlobCoeff) {
    if (StoredCoeff1) {
      NonConstCE1->setBlobCoeff(Index1, StoredCoeff1);
    }
    if (StoredCoeff2) {
      NonConstCE2->setBlobCoeff(Index1, StoredCoeff2);
    }
  } else {
    NonConstCE1->setConstant(StoredCoeff1);
    NonConstCE2->setConstant(StoredCoeff2);
  }

  return Res;
}

bool CanonExprUtils::getConstDistance(const CanonExpr *CE1,
                                      const CanonExpr *CE2, int64_t *Distance,
                                      bool RelaxedMode) {
  int64_t Denom = CE1->getDenominator();

  if (Denom != CE2->getDenominator()) {
    return false;
  }

  int64_t Const1 = CE1->getConstant();
  int64_t Const2 = CE2->getConstant();

  int64_t Diff = Const1 - Const2;

  if ((Diff % Denom) != 0) {
    return false;
  }

  // We cheat to avoid cloning the CE.
  auto NonConstCE1 = const_cast<CanonExpr *>(CE1);
  auto NonConstCE2 = const_cast<CanonExpr *>(CE2);

  // Reset constants to compare CEs.
  NonConstCE1->setConstant(0);
  NonConstCE2->setConstant(0);

  bool Res = false;

  if (areEqual(NonConstCE1, NonConstCE2, RelaxedMode)) {
    if (Distance) {
      *Distance = Diff / Denom;
    }
    Res = true;
  }

  // Restore CEs to original state.
  NonConstCE1->setConstant(Const1);
  NonConstCE2->setConstant(Const2);

  return Res;
}

bool CanonExprUtils::compare(const CanonExpr *CE1, const CanonExpr *CE2) {
  // Check the number of IV's.
  if (CE1->numIVs() != CE2->numIVs()) {
    return (CE1->numIVs() < CE2->numIVs());
  }

  // Check the IV's (temp fix: use level)
  for (unsigned Lvl = 1; Lvl <= MaxLoopNestLevel; ++Lvl) {
    int64_t Iv1Coeff, Iv2Coeff;
    unsigned Iv1BlobIndex, Iv2BlobIndex;
    CE1->getIVCoeff(Lvl, &Iv1BlobIndex, &Iv1Coeff);
    CE2->getIVCoeff(Lvl, &Iv2BlobIndex, &Iv2Coeff);

    if (Iv1Coeff != Iv2Coeff) {
      return (Iv1Coeff < Iv2Coeff);
    }

    if (Iv1BlobIndex != Iv2BlobIndex) {
      return (Iv1BlobIndex < Iv2BlobIndex);
    }
  }

  // Check the number of blobs.
  if (CE1->numBlobs() != CE2->numBlobs()) {
    return (CE1->numBlobs() < CE2->numBlobs());
  }

  // Check the Blob's.
  for (auto Blob1 = CE1->blob_begin(), End = CE1->blob_end(),
            Blob2 = CE2->blob_begin();
       Blob1 != End; ++Blob1, ++Blob2) {
    if (Blob1->Index != Blob2->Index) {
      return (Blob1->Index < Blob2->Index);
    }

    if (Blob1->Coeff != Blob2->Coeff) {
      return (Blob1->Coeff < Blob2->Coeff);
    }
  }

  if (CE1->getConstant() != CE2->getConstant()) {
    return (CE1->getConstant() < CE2->getConstant());
  }

  if (CE1->getDenominator() != CE2->getDenominator()) {
    return (CE1->getDenominator() < CE2->getDenominator());
  }

  // Check division type for non-unit denominator.
  if ((CE1->getDenominator() != 1) &&
      (CE1->isSignedDiv() != CE2->isSignedDiv())) {
    return CE1->isSignedDiv();
  }

  auto &CEU = CE1->getCanonExprUtils();

  // If CE1 and CE2 have incompatible types, order them using type info.
  auto Res = CEU.compare(CE1->getSrcType(), CE2->getSrcType());

  if (Res != 0) {
    return Res < 0;
  }

  Res = CEU.compare(CE1->getDestType(), CE2->getDestType());

  if (Res != 0) {
    return Res < 0;
  }

  if (CE1->isSExt() != CE2->isSExt()) {
    return CE1->isSExt();
  }

  if (CE1->isNonLinear() != CE2->isNonLinear()) {
    return CE1->isNonLinear();
  } else if (!CE1->isNonLinear()) {
    return CE1->getDefinedAtLevel() < CE2->getDefinedAtLevel();
  }

  return false;
}

int64_t CanonExprUtils::compareRecursive(
    Type *Ty1, Type *Ty2,
    DenseSet<std::pair<Type *, Type *>> &InProcessQueries) const {
  // Perform trivial equality check first.
  if (Ty1 == Ty2) {
    return 0;
  }

  // Separate by type ID.
  if (Ty1->getTypeID() != Ty2->getTypeID()) {
    return Ty1->getTypeID() - Ty2->getTypeID();
  }

  bool IsTy1Sized = Ty1->isSized();
  bool IsTy2Sized = Ty2->isSized();

  if (IsTy1Sized != IsTy2Sized) {
    return IsTy1Sized - IsTy2Sized;

  } else if (IsTy1Sized && IsTy2Sized) {
    auto Size1 = getDataLayout().getTypeSizeInBits(Ty1);
    auto Size2 = getDataLayout().getTypeSizeInBits(Ty2);

    if (Size1 != Size2) {
      return Size1 - Size2;
    }
  }

  // Compare subtypes. This is required to differentiate between two structures
  // of same size, for example.
  unsigned NumSubTypes1 = Ty1->getNumContainedTypes();
  unsigned NumSubTypes2 = Ty2->getNumContainedTypes();

  if (NumSubTypes1 != NumSubTypes2) {
    return NumSubTypes1 - NumSubTypes2;
  }

  // Skip if the query is already in process.
  if (InProcessQueries.count(std::make_pair(Ty1, Ty2))) {
    return 0;
  }

  // Add entry so we don't cycle on the same types.
  InProcessQueries.insert(std::make_pair(Ty1, Ty2));

  for (unsigned I = 0; I < NumSubTypes1; ++I) {
    auto Res = compareRecursive(Ty1->getContainedType(I),
                                Ty2->getContainedType(I), InProcessQueries);

    if (Res != 0) {
      return Res;
    }
  }

  // This is not preferred as it can generate non-deterministic results across
  // compiler runs if order of creation of type objects is non-deterministic but
  // it is better than the alternative of returning 0 which can cause stability
  // issues. This can happen if, for example, Ty1 and Ty2 are both structures
  // containing function pointer members. As function is not a sized type, we
  // need alternative checks to compare them. This is left as a TODO for now.
  return Ty1 - Ty2;
}

int64_t CanonExprUtils::compare(Type *Ty1, Type *Ty2) const {
  DenseSet<std::pair<Type *, Type *>> InProcessQueries;

  return compareRecursive(Ty1, Ty2, InProcessQueries);
}
