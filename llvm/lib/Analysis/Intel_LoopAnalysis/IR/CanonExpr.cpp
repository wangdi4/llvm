//===- CanonExpr.cpp - Implements the CanonExpr class ---------------------===//
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
// This file implements the CanonExpr class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/TypeSize.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;
using namespace loopopt;

CanonExpr::BlobIndexToCoeff::BlobIndexToCoeff(unsigned Indx, int64_t Coef)
    : Index(Indx), Coeff(Coef) {}

CanonExpr::BlobIndexToCoeff::~BlobIndexToCoeff() {}

CanonExpr::CanonExpr(CanonExprUtils &CEU, Type *SrcType, Type *DestType,
                     bool IsSExt, unsigned DefLevel, int64_t ConstVal,
                     int64_t Denom, bool IsSignedDiv)
    : CEU(CEU), SrcTy(SrcType), DestTy(DestType), IsSExt(IsSExt),
      DefinedAtLevel(DefLevel), Const(ConstVal), IsSignedDiv(IsSignedDiv) {
  assert(CanonExpr::isValidDefLevel(DefLevel) && "Invalid def level!");

  CEU.Objs.insert(this);
  setDenominator(Denom);

  /// Start with size = capcity
  IVCoeffs.resize(IVCoeffs.capacity(), BlobIndexToCoeff(InvalidBlobIndex, 0));
}

CanonExpr::CanonExpr(const CanonExpr &CE)
    : CEU(CE.CEU), SrcTy(CE.SrcTy), DestTy(CE.DestTy), IsSExt(CE.IsSExt),
      DefinedAtLevel(CE.DefinedAtLevel), IVCoeffs(CE.IVCoeffs),
      BlobCoeffs(CE.BlobCoeffs), Const(CE.Const), Denominator(CE.Denominator),
      IsSignedDiv(CE.IsSignedDiv), DbgLoc(CE.DbgLoc) {

  CEU.Objs.insert(this);
}

BlobUtils &CanonExpr::getBlobUtils() const {
  return getCanonExprUtils().getBlobUtils();
}

CanonExpr *CanonExpr::clone() const {
  CanonExpr *CE = new CanonExpr(*this);
  return CE;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void CanonExpr::dump(bool Detailed) const {
  formatted_raw_ostream OS(dbgs());
  print(OS, Detailed);
}

void CanonExpr::dump() const { dump(false); }
#endif

void CanonExpr::print(raw_ostream &OS, bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
  auto C0 = getConstant();
  auto Denom = getDenominator();
  bool Printed = false;

  if (isNull()) {
    OS << "null";
    return;
  }

  if (Detailed) {
    if (!isConstant()) {
      if (isNonLinear()) {
        OS << "NON-LINEAR ";
      } else {
        OS << "LINEAR ";
      }
    }

    if (getSrcType() != getDestType()) {
      if (isSExt()) {
        OS << "sext.";
      } else if (isZExt()) {
        OS << "zext.";
      } else if (isTrunc()) {
        OS << "trunc.";
      } else {
        OS << "bitcast.";
      }

      OS << *getSrcType() << ".";
      OS << *getDestType() << "(";
    } else {
      OS << *getSrcType() << " ";
    }
  }

  if (Denom != 1) {
    OS << "(";
  }

  for (auto I = iv_begin(), E = iv_end(); I != E; ++I) {
    if (I->Coeff != 0) {
      if (Printed) {
        OS << " + ";
      } else {
        Printed = true;
      }

      if (I->Coeff != 1) {
        OS << I->Coeff << " * ";
      }
      if (I->Index != InvalidBlobIndex) {
        getBlobUtils().printBlob(OS, getBlobUtils().getBlob(I->Index));
        OS << " * ";
      }
      OS << "i" << getLevel(I);
    }
  }

  for (auto I = blob_begin(), E = blob_end(); I != E; ++I) {
    if (I->Coeff != 0) {
      if (Printed) {
        OS << " + ";
      } else {
        Printed = true;
      }

      if (I->Coeff != 1) {
        OS << I->Coeff << " * ";
      }
      getBlobUtils().printBlob(OS, getBlobUtils().getBlob(I->Index));
    }
  }

  if (!Printed) {
    OS << C0;
  } else if (C0 != 0) {
    OS << " + " << C0;
  }

  if (Denom != 1) {
    OS << ")/";

    if (!isSignedDiv()) {
      OS << "u";
    }

    OS << Denom;
  }

  if (Detailed) {

    if (getSrcType() != getDestType()) {
      OS << ")";
    }

    if (!isNonLinear() && getDefinedAtLevel() > 0) {
      OS << "{def@" << getDefinedAtLevel() << "}";
    }
  }
#endif // !INTEL_PRODUCT_RELEASE
}

bool CanonExpr::isSelfBlob() const {
  return isStandAloneBlob(false /*AllowConversion*/) &&
         BlobUtils::isTempBlob(getBlobUtils().getBlob(getSingleBlobIndex()));
}

bool CanonExpr::isStandAloneUndefBlob() const {
  return (isStandAloneBlob() &&
          BlobUtils::isUndefBlob(getBlobUtils().getBlob(getSingleBlobIndex())));
}

bool CanonExpr::isUnitaryBlob() const {
  if (!isStandAloneBlob(false /*AllowConversion*/)) {
    return false;
  }

  assert(numBlobs() == 1 && "Expected only one blob in standalone blob");
  return !getBlobUtils().isNestedBlob(
      getBlobUtils().getBlob(getSingleBlobIndex()));
}

void CanonExpr::setDenominator(int64_t Val) {
  assert((Val != 0) && "Denominator cannot be zero!");

  // Negate the canon expr instead of storing negative denominators.
  if (Val < 0) {
    negate();
    Denominator = -Val;
  } else {
    Denominator = Val;
  }
}

bool CanonExpr::isExtImpl(bool IsSigned, bool IsTrunc) const {
  // Account for vector Src/Dest types.
  auto ScalSrcTy = SrcTy->getScalarType();
  auto ScalDestTy = DestTy->getScalarType();

  if (ScalSrcTy == ScalDestTy) {
    return false;
  }

  if (!ScalSrcTy->isIntegerTy() || !ScalDestTy->isIntegerTy()) {
    return false;
  }

  if (ScalSrcTy->getPrimitiveSizeInBits() >
      ScalDestTy->getPrimitiveSizeInBits()) {
    return IsTrunc;
  }

  if (IsTrunc) {
    return false;
  }

  return IsSigned ? IsSExt : !IsSExt;
}

bool CanonExpr::isSExt() const { return isExtImpl(true, false); }

bool CanonExpr::isZExt() const { return isExtImpl(false, false); }

bool CanonExpr::isTrunc() const { return isExtImpl(false, true); }

bool CanonExpr::isPtrToPtrCast() const {
  return ((SrcTy != DestTy) && SrcTy->isPointerTy());
}

bool CanonExpr::isIntConstantImpl(int64_t *Val, bool HandleSplat) const {
  auto TSrcTy = getSrcType();

  if (HandleSplat) {
    TSrcTy = TSrcTy->getScalarType();
  }

  if (!TSrcTy->isIntegerTy() || !isConstInternal()) {
    return false;
  }

  if (Val) {
    *Val = getConstant();
  }

  return true;
}

bool CanonExpr::isIntConstant(int64_t *Val) const {
  return isIntConstantImpl(Val, false);
}

bool CanonExpr::isIntConstantSplat(int64_t *Val) const {
  if (!getSrcType()->isVectorTy()) {
    return false;
  }

  return isIntConstantImpl(Val, true);
}

bool CanonExpr::isFPConstantImpl(ConstantFP **Val, bool HandleSplat) const {
  // When HandleSplat is true, we expect CanonExpr to have vector type
  if (HandleSplat) {
    assert(getSrcType()->isVectorTy() && "Vector type expected!");
  }

  if (!isStandAloneBlob()) {
    return false;
  }

  return BlobUtils::isConstantFPBlob(
      getBlobUtils().getBlob(getSingleBlobIndex()), Val);
}

bool CanonExpr::isFPConstant(ConstantFP **Val) const {
  return isFPConstantImpl(Val, false);
}

bool CanonExpr::isFPConstantSplat(ConstantFP **Val) const {
  if (!getSrcType()->isVectorTy()) {
    return false;
  }

  return isFPConstantImpl(Val, true);
}

bool CanonExpr::isConstantData(ConstantData **Val) const {
  if (!isStandAloneBlob()) {
    return false;
  }

  return BlobUtils::isConstantDataBlob(
      getBlobUtils().getBlob(getSingleBlobIndex()), Val);
}

bool CanonExpr::isConstantAggregate(ConstantAggregate **Val) const {
  if (!isStandAloneBlob()) {
    return false;
  }

  return BlobUtils::isConstantAggregateBlob(
      getBlobUtils().getBlob(getSingleBlobIndex()), Val);
}

bool CanonExpr::isMetadata(MetadataAsValue **Val) const {
  if (!isStandAloneBlob()) {
    return false;
  }

  bool IsMetadata = BlobUtils::isMetadataBlob(
      getBlobUtils().getBlob(getSingleBlobIndex()), Val);
  assert((!IsMetadata || getDestType()->isMetadataTy()) &&
         "Expected metadata type for CanonExpr representing a metadata!");
  return IsMetadata;
}

bool CanonExpr::isConstantVectorImpl(Constant **Val) const {
  if (!isStandAloneBlob()) {
    return false;
  }

  return BlobUtils::isConstantVectorBlob(
      getBlobUtils().getBlob(getSingleBlobIndex()), Val);
}

bool CanonExpr::isIntVectorConstant(Constant **Val) const {
  if (!getSrcType()->isVectorTy() ||
      !getSrcType()->getScalarType()->isIntegerTy()) {
    return false;
  }

  // Handle the case of a scalar constant broadcast
  int64_t ConstIntVal;
  if (isIntConstantSplat(&ConstIntVal)) {
    if (Val) {
      Constant *ConstVal;

      ConstVal = ConstantInt::get(getDestType()->getScalarType(), ConstIntVal);
      *Val = ConstantVector::getSplat(
          ElementCount::getFixed(
              cast<FixedVectorType>(getDestType())->getNumElements()),
          ConstVal);
    }

    return true;
  }

  return isConstantVectorImpl(Val);
}

bool CanonExpr::isFPVectorConstant(Constant **Val) const {
  if (!getSrcType()->isVectorTy() ||
      !getSrcType()->getScalarType()->isFloatingPointTy()) {
    return false;
  }

  // Handle the case of a scalar constant broadcast
  ConstantFP *ConstFPVal;
  if (isFPConstantSplat(&ConstFPVal)) {
    if (Val) {
      Constant *ConstVal;

      ConstVal = ConstFPVal;
      *Val = ConstantVector::getSplat(
          ElementCount::getFixed(
              cast<FixedVectorType>(getDestType())->getNumElements()),
          ConstVal);
    }

    return true;
  }

  return isConstantVectorImpl(Val);
}

bool CanonExpr::isNullImpl() const {
  bool Ret = isConstInternal();
  assert((!Ret || !getConstant()) && "Invalid pointer type canon expr!");

  return Ret;
}

bool CanonExpr::isNull() const {
  if (!getSrcType()->isPointerTy()) {
    return false;
  }

  return isNullImpl();
}

bool CanonExpr::isNullVector() const {
  if (!getSrcType()->isVectorTy() || !getSrcType()->isPtrOrPtrVectorTy()) {
    return false;
  }

  return isNullImpl();
}

bool CanonExpr::isStandAloneIV(bool AllowConversion, unsigned *Level) const {

  if ((AllowConversion || (getSrcType() == getDestType())) && !hasBlob() &&
      !getConstant() && (getDenominator() == 1)) {

    unsigned NumIVs = 0;
    // This loop checks that there is only one IV with BlobCoeff == 0 and Coeff
    // == 1
    for (auto IV = iv_begin(), E = iv_end(); IV != E; ++IV) {
      int64_t Coeff;
      unsigned BlobCoeff;
      getIVCoeff(IV, &BlobCoeff, &Coeff);

      if (Coeff == 0) {
        continue;
      }

      ++NumIVs;

      // A standalone IV has only one IV
      if (NumIVs > 1) {
        return false;
      }

      // A standalone IV has no blob coeffs
      if (BlobCoeff != 0) {
        return false;
      }

      // A standalone IV has coeff 1
      if (Coeff != 1) {
        return false;
      }
    }

    // After the loop, if NumIVs == 1, then Coeff == 1 and BlobCoeff == 0 for
    // that IV, so it's standalone
    if (NumIVs == 1) {

      // Save the StandAlone Level:
      if (Level) {
        *Level = getFirstIVLevel();
      }

      return true;
    }
  }

  return false;
}

unsigned CanonExpr::getFirstIVLevel() const {

  // We know there is at least one IV with coeff != 0
  for (auto IV = iv_begin(), E = iv_end(); IV != E; ++IV) {
    if (getIVConstCoeff(IV) != 0) {
      return getLevel(IV);
    }
  }

  return 0;
}

unsigned CanonExpr::numIVImpl(bool CheckIVPresence,
                              bool CheckBlobCoeffs) const {
  unsigned Count = 0;

  for (auto &I : IVCoeffs) {
    if (I.Coeff && (!CheckBlobCoeffs || I.Index)) {
      ++Count;

      if (CheckIVPresence) {
        break;
      }
    }
  }

  return Count;
}

bool CanonExpr::hasIV() const { return numIVImpl(true, false); }

bool CanonExpr::hasIV(unsigned Level) const {

  int64_t Coeff = 0;
  getIVCoeff(Level, nullptr, &Coeff);
  return Coeff;
}

unsigned CanonExpr::numIVs() const { return numIVImpl(false, false); }

bool CanonExpr::hasIVBlobCoeffs() const { return numIVImpl(true, true); }

unsigned CanonExpr::numIVBlobCoeffs() const { return numIVImpl(false, true); }

unsigned CanonExpr::getLevel(const_iv_iterator ConstIVIter) const {
  return (ConstIVIter - iv_begin() + 1);
}

void CanonExpr::getIVCoeff(unsigned Lvl, unsigned *Index,
                           int64_t *Coeff) const {
  assert((Index || Coeff) && "Non-null Index or Coeff ptr expected!");
  assert(CanonExpr::isValidLinearDefLevel(Lvl) && "Level is out of bounds!");

  if (IVCoeffs.size() < Lvl) {
    if (Index) {
      *Index = InvalidBlobIndex;
    }
    if (Coeff) {
      *Coeff = 0;
    }
  } else {
    if (Index) {
      *Index = IVCoeffs[Lvl - 1].Index;
    }
    if (Coeff) {
      *Coeff = IVCoeffs[Lvl - 1].Coeff;
    }
  }
}

void CanonExpr::getIVCoeff(const_iv_iterator ConstIVIter, unsigned *Index,
                           int64_t *Coeff) const {
  if (Index) {
    *Index = ConstIVIter->Index;
  }
  if (Coeff) {
    *Coeff = ConstIVIter->Coeff;
  }
}

unsigned CanonExpr::getIVBlobCoeff(unsigned Lvl) const {
  unsigned Index;

  getIVCoeff(Lvl, &Index, nullptr);
  return Index;
}

unsigned CanonExpr::getIVBlobCoeff(const_iv_iterator ConstIVIter) const {
  return ConstIVIter->Index;
}

bool CanonExpr::hasIVBlobCoeff(unsigned Lvl) const {
  return getIVBlobCoeff(Lvl) != InvalidBlobIndex;
}

bool CanonExpr::hasIVBlobCoeff(const_iv_iterator ConstIVIter) const {
  return getIVBlobCoeff(ConstIVIter) != InvalidBlobIndex;
}

int64_t CanonExpr::getIVConstCoeff(unsigned Lvl) const {
  int64_t Coeff;

  getIVCoeff(Lvl, nullptr, &Coeff);
  return Coeff;
}

int64_t CanonExpr::getIVConstCoeff(const_iv_iterator ConstIVIter) const {
  return ConstIVIter->Coeff;
}

bool CanonExpr::hasIVConstCoeff(unsigned Lvl) const {
  return getIVConstCoeff(Lvl);
}

bool CanonExpr::hasIVConstCoeff(const_iv_iterator ConstIVIter) const {
  return getIVConstCoeff(ConstIVIter);
}

void CanonExpr::resizeIVCoeffsToMax(unsigned Lvl) {
  assert(CanonExpr::isValidLinearDefLevel(Lvl) && "Level is out of bounds!");

  if (IVCoeffs.size() < Lvl) {
    IVCoeffs.resize(MaxLoopNestLevel, BlobIndexToCoeff(InvalidBlobIndex, 0));
  }
}

void CanonExpr::setIVInternal(unsigned Lvl, unsigned Index, int64_t Coeff,
                              bool OverwriteIndex, bool OverwriteCoeff) {

  assert(CanonExpr::isValidLinearDefLevel(Lvl) && "Level is out of bounds!");
  assert(
      ((Index == InvalidBlobIndex) || getBlobUtils().isBlobIndexValid(Index)) &&
      "Blob Index is invalid!");

  resizeIVCoeffsToMax(Lvl);

  if (OverwriteIndex) {
    IVCoeffs[Lvl - 1].Index = Index;
  }
  if (OverwriteCoeff) {
    assert(Coeff && "Use removeIV() instead!");
    IVCoeffs[Lvl - 1].Coeff = Coeff;
  }
}

void CanonExpr::setIVCoeff(unsigned Lvl, unsigned Index, int64_t Coeff) {
  setIVInternal(Lvl, Index, Coeff, true, true);
}

void CanonExpr::setIVCoeff(iv_iterator IVI, unsigned Index, int64_t Coeff) {
  IVI->Index = Index;
  IVI->Coeff = Coeff;
}

void CanonExpr::setIVBlobCoeff(unsigned Lvl, unsigned Index) {
  setIVInternal(Lvl, Index, 0, true, false);
}

void CanonExpr::setIVBlobCoeff(iv_iterator IVI, unsigned Index) {
  IVI->Index = Index;
}

void CanonExpr::setIVConstCoeff(unsigned Lvl, int64_t Coeff) {
  setIVInternal(Lvl, InvalidBlobIndex, Coeff, false, true);
}

void CanonExpr::setIVConstCoeff(iv_iterator IVI, int64_t Coeff) {
  assert(Coeff && "Use removeIV() instead!");
  IVI->Coeff = Coeff;
}

void CanonExpr::addIVInternal(unsigned Lvl, unsigned Index, int64_t Coeff) {

  assert(CanonExpr::isValidLinearDefLevel(Lvl) && "Level is out of bounds!");
  assert(
      ((Index == InvalidBlobIndex) || getBlobUtils().isBlobIndexValid(Index)) &&
      "Blob Index is invalid!");

  resizeIVCoeffsToMax(Lvl);

  // Nothing to add.
  if (!Coeff) {
    return;
  }

  // If IV is not present, add() reduces to set().
  if (!IVCoeffs[Lvl - 1].Coeff) {
    setIVCoeff(Lvl, Index, Coeff);
    return;
  }

  // Create new blob (C1 * b1 + C2 * b2) if current and incoming blob indices
  // are different.
  // At least one of the indices is non-zero here.
  if (IVCoeffs[Lvl - 1].Index != Index) {
    BlobTy AddBlob, MulBlob1 = nullptr, MulBlob2 = nullptr;
    unsigned NewIndex = InvalidBlobIndex;
    int64_t NewCoeff = 1;
    auto *SrcTy = getSrcType()->getScalarType();

    // Create a mul blob from new index/coeff.
    MulBlob1 = getBlobUtils().createBlob(Coeff, SrcTy, false);

    if (Index != InvalidBlobIndex) {
      MulBlob1 = getBlobUtils().createMulBlob(
          MulBlob1, getBlobUtils().getBlob(Index), true, &NewIndex);
    }

    // Create a mul blob from existing index/coeff.
    if (IVCoeffs[Lvl - 1].Coeff) {
      MulBlob2 =
          getBlobUtils().createBlob(IVCoeffs[Lvl - 1].Coeff, SrcTy, false);

      if (IVCoeffs[Lvl - 1].Index != InvalidBlobIndex) {
        MulBlob2 = getBlobUtils().createMulBlob(
            MulBlob2, getBlobUtils().getBlob(IVCoeffs[Lvl - 1].Index), false);
      }
    }

    // Create an add blob, if necessary.
    if (MulBlob2) {
      AddBlob =
          getBlobUtils().createAddBlob(MulBlob1, MulBlob2, true, &NewIndex);

      // Check whether the add blob has been simplified to a constant, if so,
      // set it as a constant coefficient.
      // For example: (%b + 2) + (-%b) = 2
      if (BlobUtils::isConstantIntBlob(AddBlob, &NewCoeff)) {
        NewIndex = InvalidBlobIndex;
      }
    }

    assert(((NewCoeff == 1) || (NewIndex == InvalidBlobIndex)) &&
           "Unexpected merge condition!");

    // Set new index and coefficient.
    IVCoeffs[Lvl - 1].Index = NewIndex;
    IVCoeffs[Lvl - 1].Coeff = NewCoeff;
  } else {
    // Both indices are equal(or zero) so just add the const coefficients.
    IVCoeffs[Lvl - 1].Coeff += Coeff;

    // If coefficient becomes zero, zero out index as well.
    if (!IVCoeffs[Lvl - 1].Coeff) {
      IVCoeffs[Lvl - 1].Index = InvalidBlobIndex;
    }
  }
}

void CanonExpr::addIV(unsigned Lvl, unsigned Index, int64_t Coeff,
                      bool IsMathAdd) {
  addIVInternal(Lvl, Index, getMathCoeff(Coeff, IsMathAdd));
}

void CanonExpr::addIV(iv_iterator IVI, unsigned Index, int64_t Coeff,
                      bool IsMathAdd) {
  addIV(getLevel(IVI), Index, Coeff, IsMathAdd);
}

void CanonExpr::removeIV(unsigned Lvl) {

  assert(CanonExpr::isValidLinearDefLevel(Lvl) && "Level is out of bounds!");

  /// Nothing to do as the IV is not present.
  /// Should we assert on this?
  if (IVCoeffs.size() < Lvl) {
    return;
  }

  IVCoeffs[Lvl - 1].Index = InvalidBlobIndex;
  IVCoeffs[Lvl - 1].Coeff = 0;
}

void CanonExpr::removeIV(iv_iterator IVI) {
  IVI->Index = InvalidBlobIndex;
  IVI->Coeff = 0;
}

void CanonExpr::replaceIVByConstant(unsigned Lvl, int64_t Val) {
  assert(CanonExpr::isValidLinearDefLevel(Lvl) && "Level is out of bounds!");

  // IV not present, nothing to do.
  if ((IVCoeffs.size() < Lvl) || !IVCoeffs[Lvl - 1].Coeff) {
    return;
  }

  // Val is zero, remove IV and return.
  if (!Val) {
    removeIV(Lvl);
    return;
  }

  // Cast constant value to the CE source type.
  Type *SrcType = getSrcType()->getScalarType();
  APInt APVal(SrcType->getPrimitiveSizeInBits(), Val);
  if (!ConstantInt::isValueValidForType(SrcType, (uint64_t)Val)) {
    // In HIR the IV is always non negative and may be safely represented as
    // an unsigned value.
    Val = static_cast<int64_t>(APVal.getZExtValue());
  }

  // Val may becomes zero in the SrcType by truncation.
  if (!Val) {
    removeIV(Lvl);
    return;
  }

  int64_t NewVal = IVCoeffs[Lvl - 1].Coeff * Val;

  if (IVCoeffs[Lvl - 1].Index != InvalidBlobIndex) {
    /// IV has a blob index coefficient.
    addBlob(IVCoeffs[Lvl - 1].Index, NewVal);
  } else {
    /// IV just has a constant coefficient.
    Const += NewVal;
  }

  removeIV(Lvl);
}

void CanonExpr::replaceIVByConstant(iv_iterator IVI, int64_t Val) {
  replaceIVByConstant(getLevel(IVI), Val);
}

void CanonExpr::multiplyIVByConstant(unsigned Lvl, int64_t Val) {

  assert(CanonExpr::isValidLinearDefLevel(Lvl) && "Level is out of bounds!");

  if (IVCoeffs.size() < Lvl) {
    return;
  }

  if (Val == 0) {
    removeIV(Lvl);
  } else {
    IVCoeffs[Lvl - 1].Coeff *= Val;
  }
}

void CanonExpr::multiplyIVByConstant(iv_iterator IVI, int64_t Val) {
  multiplyIVByConstant(getLevel(IVI), Val);
}

void CanonExpr::replaceIV(unsigned OldLevel, unsigned NewLevel) {
  unsigned Index;
  int64_t Coeff;
  getIVCoeff(OldLevel, &Index, &Coeff);
  removeIV(OldLevel);
  addIV(NewLevel, Index, Coeff);
}

int64_t CanonExpr::getBlobCoeff(unsigned Index) const {

  BlobIndexToCoeff Blob(Index, 0);

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob,
                            BlobIndexCompareLess());

  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    return I->Coeff;
  }

  return 0;
}

unsigned CanonExpr::getBlobIndex(const_blob_iterator CBlobI) const {
  return CBlobI->Index;
}

int64_t CanonExpr::getBlobCoeff(const_blob_iterator CBlobI) const {
  return CBlobI->Coeff;
}

void CanonExpr::addBlobInternal(unsigned Index, int64_t Coeff, bool Overwrite) {

  assert((Coeff != 0) && "Coeff cannot be zero!");
  assert(getBlobUtils().isBlobIndexValid(Index) && "Index is out of bounds!");

  BlobIndexToCoeff Blob(Index, Coeff);

  /// No blobs present, add this one.
  if (BlobCoeffs.empty()) {
    BlobCoeffs.push_back(Blob);
    return;
  }

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob,
                            BlobIndexCompareLess());

  /// The blob already exists so just change the coeff.
  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    if (Overwrite) {
      I->Coeff = Coeff;
    } else {
      I->Coeff += Coeff;
      if (I->Coeff == 0) {
        // Blobs cancel out.
        removeBlob(Index);
      }
    }
  }
  /// We need to insert new blob at this (sorted) position.
  else {
    BlobCoeffs.insert(I, Blob);
  }
}

void CanonExpr::setBlobCoeff(unsigned Index, int64_t Coeff) {
  addBlobInternal(Index, Coeff, true);
}

void CanonExpr::setBlobCoeff(blob_iterator BlobI, int64_t Coeff) {
  if (!Coeff) {
    removeBlob(BlobI);
  } else {
    BlobI->Coeff = Coeff;
  }
}

void CanonExpr::addBlob(unsigned Index, int64_t Coeff, bool IsMathAdd) {
  addBlobInternal(Index, getMathCoeff(Coeff, IsMathAdd), false);
}

void CanonExpr::addBlob(blob_iterator BlobI, int64_t Coeff, bool IsMathAdd) {
  BlobI->Coeff += getMathCoeff(Coeff, IsMathAdd);

  if (!BlobI->Coeff) {
    removeBlob(BlobI);
  }
}

void CanonExpr::removeBlob(unsigned Index) {

  BlobIndexToCoeff Blob(Index, 0);

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob,
                            BlobIndexCompareLess());

  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    BlobCoeffs.erase(I);
  }
}

void CanonExpr::removeBlob(blob_iterator BlobI) { BlobCoeffs.erase(BlobI); }

void CanonExpr::replaceBlob(unsigned OldIndex, unsigned NewIndex) {

  assert((NewIndex != InvalidBlobIndex) && "NewIndex is invalid!");

  int64_t Coeff = 0;
  bool found = false;
  BlobIndexToCoeff Blob(OldIndex, 0);

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob,
                            BlobIndexCompareLess());

  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    /// Store the coeff as the iterator might get invalidated after erase.
    Coeff = I->Coeff;
    BlobCoeffs.erase(I);
    addBlob(NewIndex, Coeff);
    found = true;
  }

  /// Replace in IV coefficients
  for (auto &I : IVCoeffs) {
    if (I.Index == OldIndex) {
      I.Index = NewIndex;
      found = true;
    }
  }

  assert(found && "Old blob index not found!");
  (void)found;
}

template <bool IsConstant, typename T>
bool CanonExpr::replaceTempBlobImpl(unsigned TempIndex, T Operand) {
  assert(BlobUtils::isTempBlob(getBlobUtils().getBlob(TempIndex)) &&
         "Old Index is not a temp!");
  assert(
      (IsConstant || BlobUtils::isTempBlob(getBlobUtils().getBlob(Operand))) &&
      "New Index is not a temp!");

  int64_t SimplifiedConstant;
  bool Replaced = false;
  unsigned NewBlobIndex;

  // Replace in IV blobs coeffs.
  for (auto &IV : IVCoeffs) {
    if (IV.Index == InvalidBlobIndex) {
      continue;
    }

    if (IV.Index == TempIndex) {
      if (IsConstant) {
        IV.Coeff *= Operand;
        IV.Index = InvalidBlobIndex;
      } else {
        IV.Index = Operand;
      }

      Replaced = true;
      continue;
    }

    if (getBlobUtils().replaceTempBlob(IV.Index, TempIndex, Operand,
                                       NewBlobIndex, SimplifiedConstant)) {
      IV.Index = NewBlobIndex;
      if (NewBlobIndex == InvalidBlobIndex) {
        IV.Coeff *= SimplifiedConstant;
      }

      Replaced = true;
    }
  }

  // Replace in blobs.
  BlobCoeffsTy NewBlobs;

  auto RemovePred = [&](BlobIndexToCoeff &BC) {
    if (BC.Index == TempIndex) {
      if (IsConstant) {
        addConstant(BC.Coeff * Operand, false);
      } else {
        NewBlobs.emplace_back(Operand, BC.Coeff);
      }

      return (Replaced = true);
    }

    if (getBlobUtils().replaceTempBlob(BC.Index, TempIndex, Operand,
                                       NewBlobIndex, SimplifiedConstant)) {
      if (NewBlobIndex == InvalidBlobIndex) {
        addConstant(BC.Coeff * SimplifiedConstant, false);
      } else {
        NewBlobs.emplace_back(NewBlobIndex, BC.Coeff);
      }

      return (Replaced = true);
    }

    return false;
  };

  BlobCoeffs.erase(
      std::remove_if(BlobCoeffs.begin(), BlobCoeffs.end(), RemovePred),
      BlobCoeffs.end());

  for (auto &Blob : NewBlobs) {
    addBlob(Blob.Index, Blob.Coeff);
  }
  return Replaced;
}

bool CanonExpr::replaceTempBlob(unsigned TempIndex, unsigned NewTempIndex) {
  return replaceTempBlobImpl<false>(TempIndex, NewTempIndex);
}

bool CanonExpr::replaceTempBlobByConstant(unsigned TempIndex, int64_t Constant,
                                          bool Simplify) {
  bool Replaced = replaceTempBlobImpl<true>(TempIndex, Constant);
  if (Simplify && Replaced) {
    simplify(true, false);
  }
  return Replaced;
}

void CanonExpr::clear() {
  clearIVs();
  clearBlobs();

  Const = 0;
  Denominator = 1;

  DefinedAtLevel = 0;
}

void CanonExpr::clearIVs() {
  // Assign zeros rather than clearing to keep the size same otherwise a
  // reallocation will happen on the addition of the next IV to this CanonExpr.
  // Refer to the logic in resizeIVCoeffsToMax().
  IVCoeffs.assign(IVCoeffs.size(), BlobIndexToCoeff(InvalidBlobIndex, 0));
}

void CanonExpr::shift(unsigned Lvl, int64_t Val) {

  /// Nothing to do if Val is 0 or the IV is not present.
  if (!Val || (IVCoeffs.size() < Lvl)) {
    return;
  }

  int64_t NewVal = IVCoeffs[Lvl - 1].Coeff * Val;

  /// Handle blob coefficient of IV.
  if (IVCoeffs[Lvl - 1].Index != InvalidBlobIndex) {
    addBlob(IVCoeffs[Lvl - 1].Index, NewVal);
  }
  /// Handle constant coefficient of IV.
  else {
    Const += NewVal;
  }
}

void CanonExpr::shift(iv_iterator IVI, int64_t Val) {
  shift(getLevel(IVI), Val);
}

void CanonExpr::demoteIVs(unsigned StartLevel) {
  assert(StartLevel > 1 && "It's invalid to demote i1");
  assert(CanonExpr::isValidLoopLevel(StartLevel) && "Invalid StartLevel");

  assert(IVCoeffs[(StartLevel - 1) - 1].Coeff == 0 &&
         "Shifting to IV with a non zero coeff.");

  unsigned LastIV = IVCoeffs.size() - 1;

  for (int I = StartLevel - 1, E = LastIV; I <= E; ++I) {
    IVCoeffs[I - 1] = IVCoeffs[I];
  }

  IVCoeffs[LastIV].Coeff = 0;
  IVCoeffs[LastIV].Index = InvalidBlobIndex;
}

void CanonExpr::promoteIVs(unsigned StartLevel) {
  assert(StartLevel < MaxLoopNestLevel &&
         "It's invalid to promote MaxLoopNestLevel");
  assert(isValidLoopLevel(StartLevel) && "Invalid StartLevel");

  if (IVCoeffs.back().Coeff != 0) {
    BlobIndexToCoeff IV = IVCoeffs.back();
    IVCoeffs.push_back(IV);
  }

  for (int I = IVCoeffs.size() - 1, E = StartLevel - 1; I > E; --I) {
    IVCoeffs[I] = IVCoeffs[I - 1];
  }

  IVCoeffs[StartLevel - 1].Coeff = 0;
  IVCoeffs[StartLevel - 1].Index = InvalidBlobIndex;
}

void CanonExpr::collectBlobIndicesImpl(SmallVectorImpl<unsigned> &Indices,
                                       bool MakeUnique,
                                       bool NeedTempBlobs) const {

  assert((!MakeUnique || Indices.empty()) &&
         "Empty container expected for uniquing!");

  /// Push all blobs from BlobCoeffs.
  for (auto &I : BlobCoeffs) {
    if (NeedTempBlobs) {
      getBlobUtils().collectTempBlobs(I.Index, Indices);
    } else {
      Indices.push_back(I.Index);
    }
  }

  /// Push all blobs from IVCoeffs.
  for (auto &I : IVCoeffs) {
    if (I.Index == InvalidBlobIndex) {
      continue;
    }

    if (NeedTempBlobs) {
      getBlobUtils().collectTempBlobs(I.Index, Indices);
    } else {
      Indices.push_back(I.Index);
    }
  }

  if (MakeUnique) {
    // Make the indices unique.
    std::sort(Indices.begin(), Indices.end());
    Indices.erase(std::unique(Indices.begin(), Indices.end()), Indices.end());
  }
}

void CanonExpr::collectBlobIndices(SmallVectorImpl<unsigned> &Indices,
                                   bool MakeUnique) const {
  collectBlobIndicesImpl(Indices, MakeUnique, false);
}

void CanonExpr::collectTempBlobIndices(SmallVectorImpl<unsigned> &Indices,
                                       bool MakeUnique) const {
  collectBlobIndicesImpl(Indices, MakeUnique, true);
}

void CanonExpr::simplifyConstantDenom() {
  // Handle non-unit denominator.
  auto DenomConst = getDenominator();
  if (DenomConst == 1) {
    return;
  }

  auto Val = getConstant();

  unsigned SrcBitWidth =
      getSrcType()->getScalarType()->getPrimitiveSizeInBits();

  bool IsSignedDiv = isSignedDiv();

  APInt Constant(SrcBitWidth, Val, IsSignedDiv);
  APInt Denom(SrcBitWidth, DenomConst, IsSignedDiv);

  Val = (IsSignedDiv ? Constant.sdiv(Denom) : Constant.udiv(Denom))
            .getSExtValue();

  setDenominator(1);
  setConstant(Val);
}

void CanonExpr::simplifyConstantCast() {
  Type *SrcType = getSrcType()->getScalarType();
  Type *DstType = getDestType()->getScalarType();

  // Handle cast of constant canon expression.
  if (!SrcType->isIntegerTy() || SrcType == DstType) {
    return;
  }

  auto Val = getConstant();
  unsigned DstBitWidth = DstType->getPrimitiveSizeInBits();

  // Just assume the constant is signed. This doesn't matter as the final value
  // is determined by the cast and eventual getSExtValue() call below.
  APInt Constant(SrcType->getPrimitiveSizeInBits(), Val, true);

  // HIRParser uses getSExtValue() to set the C0 integer in CEs. To be
  // consistent we should also use getSExtValue(). This way we can perform
  // equality check between constant CEs. LLVM print also prints the constant in
  // signed mode so this setup seems to make sense. Signed representation also
  // makes sense for DD which needs to perform arithmertic on CanonExprs.
  Val = IsSExt ? Constant.sextOrTrunc(DstBitWidth).getSExtValue()
               : Constant.zextOrTrunc(DstBitWidth).getSExtValue();

  setSrcType(getDestType());
  setConstant(Val);
}

int64_t CanonExpr::simplifyGCDHelper(int64_t CurrentGCD, int64_t Num) {

  if (CurrentGCD == -1) {
    CurrentGCD = llabs(Num);
  } else {
    CurrentGCD = CanonExprUtils::gcd(CurrentGCD, llabs(Num));
  }

  return CurrentGCD;
}

void CanonExpr::simplify(bool SimplifyCast, bool IsUnsignedOrNonNegative) {
  int64_t Denom = 0, NumeratorGCD = -1, CommonGCD = 0;

  // Numerator is constant, we can evaluate value of the CE.
  if (!hasIV() && !hasBlob()) {
    simplifyConstantDenom();
    if (SimplifyCast) {
      simplifyConstantCast();
    }

    return;
  }

  // Cannot simplify unsigned division for negative numerator.
  // ((-3 * %t) /u 9) != ((-1 * %t) /u 3)
  if (!IsUnsignedOrNonNegative && isUnsignedDiv()) {
    return;
  }

  // Nothing to simplify...
  if ((Denom = getDenominator()) == 1) {
    return;
  }

  int64_t C0 = getConstant();

  // Cannot simplify any further.
  if ((C0 == 1) || (C0 == -1)) {
    return;
  }

  // Calculate gcd of all the iv and blob coefficients.
  for (auto I = iv_begin(), E = iv_end(); I != E; ++I) {
    if (!I->Coeff) {
      continue;
    }
    NumeratorGCD = simplifyGCDHelper(NumeratorGCD, I->Coeff);
  }

  for (auto I = blob_begin(), E = blob_end(); I != E; ++I) {
    NumeratorGCD = simplifyGCDHelper(NumeratorGCD, I->Coeff);
  }

  assert(NumeratorGCD != -1 && "Constant numerator is found.");

  if (C0) {
    NumeratorGCD = simplifyGCDHelper(NumeratorGCD, C0);
  }

  CommonGCD = simplifyGCDHelper(NumeratorGCD, Denom);

  // Cannot simplify any further.
  if (CommonGCD == 1) {
    return;
  }

  // Divide numerator and denominator by common gcd.
  setDenominator(Denom / CommonGCD);
  setConstant(C0 / CommonGCD);

  for (auto I = iv_begin(), E = iv_end(); I != E; ++I) {
    if (!I->Coeff) {
      continue;
    }

    setIVConstCoeff(I, (I->Coeff / CommonGCD));
  }

  for (auto I = blob_begin(), E = blob_end(); I != E; ++I) {
    setBlobCoeff(I, (I->Coeff / CommonGCD));
  }
}

bool CanonExpr::canMultiplyNumeratorByUnknown() const {
  // The result of the multiplication may be invalid if there is:
  // 1) Type extension. Ex.: c0*i8.i16(%b) != i8.i16(c0*%b), where %b is 255
  // 2) Non-unit denominator. Ex.: 2*((%b-1)/3) != (2*%b - 2)/3
  return !(isSExt() || isZExt() || getDenominator() != 1);
}

bool CanonExpr::canMultiplyNumeratorByConstant(int64_t Val) const {
  if (Val == 0 || Val == 1) {
    return true;
  }

  if (isIntConstant()) {
    return true;
  }

  if (Val == -1) {
    // Can multiply numerator if CE is not a zero/sign extension and if the
    // division is signed.

    // Multiplication by -1 overflows for signed min values.
    // Restricting the fix to small types to minimize performance impact.
    bool IsSmallSrcType =
        (getSrcType()->getScalarType()->getPrimitiveSizeInBits() < 32);

    return !isZExt() && (!isSExt() || !IsSmallSrcType) &&
           (getDenominator() == 1 || isSignedDiv());
  }

  return canMultiplyNumeratorByUnknown();
}

void CanonExpr::multiplyNumeratorByConstant(int64_t Val, bool Simplify) {

  // Multiplying by constant is equivalent to clearing the canon expr.
  if (Val == 0) {
    clear();
    return;
  }

  // Simplify instead of multiplying, if possible.
  if (Simplify) {
    int64_t Denom = getDenominator();
    int64_t GCD = getCanonExprUtils().gcd(llabs(Val), Denom);

    if (GCD != 1) {
      setDenominator(Denom / GCD);
      Val = Val / GCD;
    }
  }

  // Identity multiplication.
  if (Val == 1) {
    return;
  }

  // Multiply Val by IVCoeff, BlobCoeffs and Const
  for (auto I = iv_begin(), End = iv_end(); I != End; ++I) {
    multiplyIVByConstant(I, Val);
  }

  for (auto I = blob_begin(), End = blob_end(); I != End; ++I) {
    setBlobCoeff(I, (I->Coeff * Val));
  }

  setConstant(getConstant() * Val);
}

bool CanonExpr::multiplyByConstant(int64_t Val) {
  if (!canMultiplyNumeratorByConstant(Val) &&
      !convertToStandAloneBlobOrConstant()) {
    return false;
  }

  multiplyNumeratorByConstant(Val, true);
  return true;
}

bool CanonExpr::multiplyByBlob(unsigned Index) {
  if (!canMultiplyNumeratorByUnknown() &&
      !convertToStandAloneBlobOrConstant()) {
    return false;
  }

  multiplyNumeratorByBlob(Index);
  return true;
}

void CanonExpr::multiplyNumeratorByBlob(unsigned Index) {
  assert(getBlobUtils().isBlobIndexValid(Index) &&
         "Must be a valid blob index");

  // The canon expr is looking like:
  //   c1*b1*i1 + c2*b2 + c0
  // After multiplication by blob "b" it should be like:
  //   c1*(b*b1)*i1 + c2*(b*b2) + c0*b
  //
  //  1) (b*b1), (b*b2) are new blobs
  //  2) c1 was not touched, just need to switch b1 to (b1*b)
  //  3) b2 blob is removed, the new (b2*b) is added with the same c2 coeff
  //  4) c0 is removed, the b blob is added with c0 coeff

  // Get blob for "b"
  BlobTy MultiplierBlob = getBlobUtils().getBlob(Index);

  // Handle IV blob coeffs
  for (auto I = iv_begin(), End = iv_end(); I != End; ++I) {
    unsigned IVBlobIndex;
    int64_t IVConstCoeff;
    getIVCoeff(I, &IVBlobIndex, &IVConstCoeff);

    if (IVBlobIndex != InvalidBlobIndex) {
      BlobTy IVBlob = getBlobUtils().getBlob(IVBlobIndex);
      unsigned NewBlobIndex;
      getBlobUtils().createMulBlob(IVBlob, MultiplierBlob, true, &NewBlobIndex);
      setIVBlobCoeff(I, NewBlobIndex);
    } else if (IVConstCoeff != 0) {
      setIVBlobCoeff(I, Index);
    }
  }

  // Handle blob terms
  if (numBlobs()) {
    BlobCoeffsTy AuxBlobs;
    for (auto I = blob_begin(), End = blob_end(); I != End; ++I) {
      auto BlobCoef = getBlobCoeff(I);
      BlobTy Blob = getBlobUtils().getBlob(getBlobIndex(I));
      unsigned NewBlobIndex;
      getBlobUtils().createMulBlob(Blob, MultiplierBlob, true, &NewBlobIndex);
      AuxBlobs.push_back(BlobIndexToCoeff(NewBlobIndex, BlobCoef));
    }

    std::sort(AuxBlobs.begin(), AuxBlobs.end(), BlobIndexCompareLess());
    BlobCoeffs = AuxBlobs;
  }

  // Handle c0 constant
  int64_t C0 = getConstant();
  if (C0 != 0) {
    addBlob(Index, C0);
    setConstant(0);
  }
}

bool CanonExpr::canConvertToStandAloneBlobOrConstant() const {

  // Not applicable to other types.
  if (!getSrcType()->isIntegerTy()) {
    return false;
  }

  if (hasIV()) {
    return false;
  }

  // SCEV doesn't have signed division.
  if ((getDenominator() != 1) && isSignedDiv()) {
    return false;
  }

  return true;
}

// TODO: now it's allowed to convert constants to blobs. This is done to be able
// to use min/max SCEVs within the HIR. After implementing MIN/MAX operation in
// RegDDRef this should be changed and asserts should be added.
bool CanonExpr::convertToStandAloneBlobOrConstant() {
  assert(!isIntConstant() && "Attempt to convert constant to blob!");

  if (!canConvertToStandAloneBlobOrConstant()) {
    return false;
  }

  BlobTy MergedBlob = nullptr;

  // Create one merged blob by adding all the individual blobs.
  for (auto BIt = blob_begin(), E = blob_end(); BIt != E; ++BIt) {
    BlobTy CurBlob = getBlobUtils().getBlob(BIt->Index);

    if (BIt->Coeff != 1) {
      auto CoeffBlob = getBlobUtils().createBlob(BIt->Coeff, getSrcType());
      CurBlob = getBlobUtils().createMulBlob(CoeffBlob, CurBlob, false);
    }

    if (MergedBlob) {
      MergedBlob = getBlobUtils().createAddBlob(MergedBlob, CurBlob, false);
    } else {
      MergedBlob = CurBlob;
    }
  }

  // Add constant part.
  if (!MergedBlob || getConstant() != 0) {
    auto ConstBlob = getBlobUtils().createBlob(getConstant(), getSrcType());
    if (MergedBlob) {
      MergedBlob = getBlobUtils().createAddBlob(MergedBlob, ConstBlob, false);
    } else {
      MergedBlob = ConstBlob;
    }
  }

  assert(MergedBlob && "MergedBlob should not be null at this point");

  // Create division for the denominator.
  if (getDenominator() != 1) {
    auto DenomBlob = getBlobUtils().createBlob(getDenominator(), getSrcType());
    MergedBlob = getBlobUtils().createUDivBlob(MergedBlob, DenomBlob, false);
  }

  // Create cast for mismatched types.
  if (getSrcType() != getDestType()) {
    MergedBlob = getBlobUtils().createCastBlob(MergedBlob, isSExt(),
                                               getDestType(), false);
  }

  // Clear all the current fields.
  clearBlobs();
  Denominator = 1;
  Const = 0;

  // Set dest type as also the src type.
  setSrcType(getDestType());

  if (auto *ConstBlob = dyn_cast<SCEVConstant>(MergedBlob)) {
    // In some cases casting can result in a constant blob. For example
    // truncating (8 * %t) to i1 type is simplified to zero by scalar evolution.
    // In such a case, we make CE a constant instead of a blob.
    Const = ConstBlob->getValue()->getSExtValue();

  } else {
    // Set merged blob in the CE.
    unsigned MergedBlobIndex = getBlobUtils().findOrInsertBlob(MergedBlob);
    setBlobCoeff(MergedBlobIndex, 1);
  }

  return true;
}

bool CanonExpr::convertToCastedStandAloneBlobOrConstant(Type *Ty, bool IsSExt) {
  assert(Ty && "Ty is null!");
  assert(getDestType()->isIntegerTy() && Ty->isIntegerTy() && "Invalid cast!");

  if (!convertToStandAloneBlobOrConstant()) {
    return false;
  }

  // Cast is a no-op.
  if (Ty == getDestType()) {
    return true;
  }

  if (isIntConstant()) {
    setDestType(Ty);
    simplifyConstantCast();
    return true;
  }

  unsigned OldIndex = getSingleBlobIndex();
  unsigned NewIndex = InvalidBlobIndex;

  getBlobUtils().createCastBlob(getBlobUtils().getBlob(getSingleBlobIndex()),
                                IsSExt, Ty, true, &NewIndex);

  replaceBlob(OldIndex, NewIndex);
  setSrcAndDestType(Ty);

  return true;
}

bool CanonExpr::convertToSExtStandAloneBlobOrConstant(Type *Ty) {
  assert(Ty && "Ty is null!");
  assert((getCanonExprUtils().getTypeSizeInBits(Ty) >
          getCanonExprUtils().getTypeSizeInBits(getDestType())) &&
         "Invalid cast!");

  return convertToCastedStandAloneBlobOrConstant(Ty, true);
}

bool CanonExpr::convertToZExtStandAloneBlobOrConstant(Type *Ty) {
  assert(Ty && "Ty is null!");
  assert((getCanonExprUtils().getTypeSizeInBits(Ty) >
          getCanonExprUtils().getTypeSizeInBits(getDestType())) &&
         "Invalid cast!");

  return convertToCastedStandAloneBlobOrConstant(Ty, false);
}

bool CanonExpr::convertToTruncStandAloneBlobOrConstant(Type *Ty) {
  assert(Ty && "Ty is null!");
  assert((getCanonExprUtils().getTypeSizeInBits(Ty) <
          getCanonExprUtils().getTypeSizeInBits(getDestType())) &&
         "Invalid cast!");

  return convertToCastedStandAloneBlobOrConstant(Ty, false);
}

bool CanonExpr::verifyIVs(unsigned NestingLevel) const {
  // Verify that there are no undefined IVs.
  for (auto I = iv_begin(), E = iv_end(); I != E; ++I) {
    assert((!(getLevel(I) > NestingLevel) || !hasIVConstCoeff(I)) &&
           "The RegDDRef with IV is attached outside of the loop");
  }

  return true;
}

bool CanonExpr::verifyNestingLevel(unsigned NestingLevel) const {
  assert((isNonLinear() ||
          (getDefinedAtLevel() < NestingLevel || isProperLinear())) &&
         "CE is undefined at the attached level or should be non-linear.");

  verifyIVs(NestingLevel);

  return true;
}

void CanonExpr::verify(unsigned NestingLevel) const {
  assert(getDenominator() > 0 && "Denominator must be greater than zero!");

  assert(CanonExpr::isValidDefLevel(DefinedAtLevel) &&
         "DefinedAtLevel is invalid!");
  assert(SrcTy && "SrcTy of CanonExpr is null!");
  assert(DestTy && "DestTy of CanonExpr is null!");

  if (SrcTy != DestTy) {
    Instruction::CastOps CastOp;

    if (isSExt()) {
      CastOp = Instruction::SExt;
    } else if (isZExt()) {
      CastOp = Instruction::ZExt;
    } else {
      assert(isTrunc() && "Invalid canon expression conversion");
      CastOp = Instruction::Trunc;
    }

    (void)CastOp;
    assert(CastInst::castIsValid(CastOp, SrcTy, DestTy) &&
           "Inconsistent Src/Dest Types");
  }

  // Account for vector types.
  auto ScalSrcTy = SrcTy->getScalarType();
  auto ScalDestTy = DestTy->getScalarType();

  if (ScalSrcTy != ScalDestTy) {
    assert(((ScalSrcTy->isIntegerTy() && ScalDestTy->isIntegerTy()) ||
            (ScalSrcTy->isPointerTy() && ScalDestTy->isPointerTy())) &&
           "CanonExpr has invalid type!");
  }

  for (auto I = BlobCoeffs.begin(), E = BlobCoeffs.end(); I != E; ++I) {
    BlobTy B = getBlobUtils().getBlob(I->Index);
    (void)B;

    auto BScalTy = B->getType()->getScalarType();
    (void)BScalTy;

    // Allow pointer/integer type mismatch as an integral canon expr can look
    // like (ptr1 - ptr2).
    assert(((BScalTy == ScalSrcTy) ||
            (BScalTy->isPointerTy() && ScalSrcTy->isIntegerTy() &&
             (getCanonExprUtils().getTypeSizeInBits(BScalTy) ==
              getCanonExprUtils().getTypeSizeInBits(ScalSrcTy)))) &&
           "Scalar type of all blobs should match canon expr scalar type!");

    // Check that blobs have valid type if this CanonExpr is of VectorType. We
    // allow only scalar or matching VectorType blobs in a vector CE.
    if (SrcTy->isVectorTy()) {
      auto BTy = B->getType();
      (void)BTy;

      assert((!BTy->isVectorTy() || BTy == SrcTy) &&
             "Only scalar or matching VectorTy blobs are allowed in vector "
             "canon expr.");
    }
  }

  if (!hasBlob() && !hasIVBlobCoeffs()) {
    assert(!isNonLinear() && "CanonExpr with no blobs cannot be non-linear!");
  }

  if (isConstant()) {
    assert(isProperLinear() &&
           " Defined at Level should be 0 for constant canonexpr!");
    assert(getDenominator() == 1 &&
           "Constant CEs should have unit denominator");

    // Allow non simplified casts in HIR.
    // assert(!isTrunc() && !isSExt() && !isZExt() &&
    //       "Casts in constant CEs should be simplified");
  }

  verifyNestingLevel(NestingLevel);
}

void std::default_delete<CanonExpr>::operator()(CanonExpr *CE) const {
  CE->getCanonExprUtils().destroy(CE);
}

bool CanonExpr::containsUndef() const {
  SmallVector<unsigned, 8> Indices;

  collectBlobIndices(Indices, false);
  return std::any_of(
      Indices.begin(), Indices.end(), [this](unsigned BlobIndex) {
        return BlobUtils::containsUndef(getBlobUtils().getBlob(BlobIndex));
      });
}

bool CanonExpr::containsStandAloneBlob(unsigned BlobIndex, bool AllowConversion,
                                       bool AllowExtStandAloneBlob) const {
  if (getDenominator() != 1 ||
      !(AllowConversion || (getSrcType() == getDestType()))) {
    return false;
  }

  auto &BU = getBlobUtils();
  auto StandAloneBlob = BU.getBlob(BlobIndex);

  for (auto I = iv_begin(), E = iv_end(); I != E; ++I) {
    unsigned BlobIdx = getIVBlobCoeff(I);

    if (BlobIdx == InvalidBlobIndex) {
      continue;
    }

    auto Blob = BU.getBlob(BlobIdx);

    if (BU.contains(Blob, StandAloneBlob)) {
      return false;
    }
  }

  bool Found = false;

  for (auto I = blob_begin(), E = blob_end(); I != E; ++I) {
    unsigned BlobIdx = getBlobIndex(I);

    if (BlobIdx == BlobIndex) {

      if (getBlobCoeff(I) != 1) {
        return false;
      }

      Found = true;
      continue;
    }

    auto Blob = BU.getBlob(BlobIdx);

    if (BU.contains(Blob, StandAloneBlob)) {

      if (AllowExtStandAloneBlob && getBlobCoeff(BlobIdx) == 1) {

        BlobTy InnerBlob;
        if (BU.isZeroExtendBlob(Blob, &InnerBlob) ||
            BU.isSignExtendBlob(Blob, &InnerBlob)) {

          if (InnerBlob == StandAloneBlob) {

            Found = true;
            continue;
          }
        }
      }

      return false;
    }
  }

  return Found;
}

bool CanonExpr::containsTempBlob(unsigned TempBlobIndex) const {
  SmallVector<unsigned, 8> TempIndices;

  collectTempBlobIndices(TempIndices);

  return std::any_of(
      TempIndices.begin(), TempIndices.end(), [&](unsigned BlobIndex) {
        return (BlobIndex == TempBlobIndex);
      });
}


bool CanonExpr::isInvariantAtLevel(unsigned Level, bool IgnoreInnerIVs) const {
  assert(CanonExpr::isValidLoopLevel(Level) &&
         "Cannot compute invariance without a valid loop level");

  if (isNonLinear() || DefinedAtLevel >= Level) {
    return false;
  }

  if (IgnoreInnerIVs) {
    return !hasIV(Level);
  }

  for (unsigned I = Level; I <= MaxLoopNestLevel; I++) {
    if (hasIV(I)) {
      return false;
    }
  }
  return true;
}

unsigned CanonExpr::getNumOperations() const {
  auto &BU = getBlobUtils();
  unsigned Count = 0;
  bool IsFirstTerm = true;

  // Add 1 Operation if the iv has non-unit coeff, which refers to a
  // multiplication operation of the iv
  for (auto IV = iv_begin(), E = iv_end(); IV != E; ++IV) {
    unsigned Index = InvalidBlobIndex;
    int64_t Coeff = 0;
    getIVCoeff(IV, &Index, &Coeff);

    if (Coeff == 0) {
      continue;
    }

    // Add 1 operation between two iv blobs
    if (!IsFirstTerm) {
      ++Count;
    }

    if (Index != InvalidBlobIndex) {
      Count += BU.getNumOperations(Index, nullptr);
    }

    if (Coeff != 1) {
      ++Count;
    }

    IsFirstTerm = false;
  }

  for (auto Blob = blob_begin(), E = blob_end(); Blob != E; ++Blob) {
    // Add 1 operation between two blobs
    if (!IsFirstTerm) {
      ++Count;
    }

    Count += BU.getNumOperations(Blob->Index, nullptr);

    // Add 1 Operation if the blob has non-unit coeff, which refers to a
    // multiplication operation of the blob
    if (Blob->Coeff != 1) {
      ++Count;
    }

    IsFirstTerm = false;
  }

  if (!IsFirstTerm && getConstant() != 0) {
    ++Count;
  }

  if (getSrcType() != getDestType()) {
    ++Count;
  }

  if (getDenominator() != 1) {
    ++Count;
  }

  return Count;
}
