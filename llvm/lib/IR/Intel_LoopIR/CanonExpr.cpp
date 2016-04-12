//===- CanonExpr.cpp - Implements the CanonExpr class ---------------------===//
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
// This file implements the CanonExpr class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"

using namespace llvm;
using namespace loopopt;

std::set<CanonExpr *> CanonExpr::Objs;

CanonExpr::BlobIndexToCoeff::BlobIndexToCoeff(unsigned Indx, int64_t Coef)
    : Index(Indx), Coeff(Coef) {}

CanonExpr::BlobIndexToCoeff::~BlobIndexToCoeff() {}

CanonExpr::CanonExpr(Type *SrcType, Type *DestType, bool IsSExt,
                     unsigned DefLevel, int64_t ConstVal, int64_t Denom,
                     bool IsSignedDiv)
    : SrcTy(SrcType), DestTy(DestType), IsSExt(IsSExt),
      DefinedAtLevel(DefLevel), Const(ConstVal), IsSignedDiv(IsSignedDiv),
      ContainsUndef(false) {
  assert(CanonExprUtils::isValidDefLevel(DefLevel) && "Invalid def level!");

  Objs.insert(this);

  setDenominator(Denom);

  /// Start with size = capcity
  IVCoeffs.resize(IVCoeffs.capacity(), BlobIndexToCoeff(InvalidBlobIndex, 0));
}

CanonExpr::CanonExpr(const CanonExpr &CE)
    : SrcTy(CE.SrcTy), DestTy(CE.DestTy), IsSExt(CE.IsSExt),
      DefinedAtLevel(CE.DefinedAtLevel), IVCoeffs(CE.IVCoeffs),
      BlobCoeffs(CE.BlobCoeffs), Const(CE.Const), Denominator(CE.Denominator),
      IsSignedDiv(CE.IsSignedDiv), ContainsUndef(CE.ContainsUndef) {

  Objs.insert(this);
}

void CanonExpr::destroy() {
  Objs.erase(this);
  delete this;
}

void CanonExpr::destroyAll() {

  for (auto &I : Objs) {
    delete I;
  }

  Objs.clear();
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

void CanonExpr::print(formatted_raw_ostream &OS, bool Detailed) const {
  auto C0 = getConstant();
  auto Denom = getDenominator();
  bool Printed = false;

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
        BlobUtils::printBlob(OS, BlobUtils::getBlob(I->Index));
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
      BlobUtils::printBlob(OS, BlobUtils::getBlob(I->Index));
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

    if (isLinearAtLevel() && getDefinedAtLevel() > 0) {
      OS << "{def@" << getDefinedAtLevel() << "}";
    }
  }
}

bool CanonExpr::isSelfBlob() const {
  return (isStandAloneBlob() &&
          BlobUtils::isTempBlob(BlobUtils::getBlob(getSingleBlobIndex())));
}

void CanonExpr::setDenominator(int64_t Val, bool Simplify) {
  assert((Val != 0) && "Denominator cannot be zero!");

  // Negate the canon expr instead of storing negative denominators.
  if (Val < 0) {
    negate();
    Denominator = -Val;
  } else {
    Denominator = Val;
  }

  if (Simplify) {
    simplify();
  }
}

void CanonExpr::divide(int64_t Val, bool Simplify) {
  setDenominator(Denominator * Val, Simplify);
}

bool CanonExpr::isExtImpl(bool IsSigned, bool IsTrunc) const {
  // Account for vector Src/Dest types.
  auto ScalSrcTy = SrcTy->getScalarType();
  auto ScalDestTy = DestTy->getScalarType();

  if (ScalSrcTy == ScalDestTy) {
    return false;
  }

  if (!ScalSrcTy->isIntegerTy()) {
    return false;
  }

  if (ScalSrcTy->getPrimitiveSizeInBits() > 
      ScalDestTy->getPrimitiveSizeInBits()) {
    if (IsTrunc) {
      return true;
    }

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
    assert(TSrcTy->isVectorTy() && "Vector type expected!");
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

  return BlobUtils::isConstantFPBlob(BlobUtils::getBlob(getSingleBlobIndex()),
                                     Val);
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

bool CanonExpr::isMetadata(MetadataAsValue **Val) const {
  if (!isStandAloneBlob()) {
    return false;
  }

  return BlobUtils::isMetadataBlob(BlobUtils::getBlob(getSingleBlobIndex()),
                                   Val);
}

bool CanonExpr::isConstantVectorImpl(Constant **Val) const {
  if (!isStandAloneBlob()) {
    return false;
  }

  return BlobUtils::isConstantVectorBlob(
                        BlobUtils::getBlob(getSingleBlobIndex()), Val);
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
      
      ConstVal = ConstantInt::get(getDestType()->getScalarType(),
                                  ConstIntVal);
      *Val = ConstantVector::getSplat(getDestType()->getVectorNumElements(),
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
      *Val = ConstantVector::getSplat(getDestType()->getVectorNumElements(),
                                      ConstVal);
    }

    return true;
  }

  return isConstantVectorImpl(Val);
}

bool CanonExpr::isNull() const {
  bool Ret = (getSrcType()->isPointerTy() && isConstInternal());
  assert((!Ret || !getConstant()) && "Invalid pointer type canon expr!");

  return Ret;
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

bool CanonExpr::hasBlobIVCoeffs() const { return numIVImpl(true, true); }

unsigned CanonExpr::numBlobIVCoeffs() const { return numIVImpl(false, true); }

unsigned CanonExpr::getLevel(const_iv_iterator ConstIVIter) const {
  return (ConstIVIter - iv_begin() + 1);
}

void CanonExpr::getIVCoeff(unsigned Lvl, unsigned *Index,
                           int64_t *Coeff) const {
  assert((Index || Coeff) && "Non-null Index or Coeff ptr expected!");
  assert(CanonExprUtils::isValidLinearDefLevel(Lvl) &&
         "Level is out of bounds!");

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
  assert(CanonExprUtils::isValidLinearDefLevel(Lvl) &&
         "Level is out of bounds!");

  if (IVCoeffs.size() < Lvl) {
    IVCoeffs.resize(MaxLoopNestLevel, BlobIndexToCoeff(InvalidBlobIndex, 0));
  }
}

void CanonExpr::setIVInternal(unsigned Lvl, unsigned Index, int64_t Coeff,
                              bool OverwriteIndex, bool OverwriteCoeff) {

  assert(CanonExprUtils::isValidLinearDefLevel(Lvl) &&
         "Level is out of bounds!");
  assert(
      ((Index == InvalidBlobIndex) || BlobUtils::isBlobIndexValid(Index)) &&
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

  assert(CanonExprUtils::isValidLinearDefLevel(Lvl) &&
         "Level is out of bounds!");
  assert(
      ((Index == InvalidBlobIndex) || BlobUtils::isBlobIndexValid(Index)) &&
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

    // Create a mul blob from new index/coeff.
    MulBlob1 = BlobUtils::createBlob(Coeff, getSrcType(), false);

    if (Index != InvalidBlobIndex) {
      MulBlob1 = BlobUtils::createMulBlob(MulBlob1, BlobUtils::getBlob(Index),
                                          true, &NewIndex);
    }

    // Create a mul blob from existing index/coeff.
    if (IVCoeffs[Lvl - 1].Coeff) {
      MulBlob2 =
          BlobUtils::createBlob(IVCoeffs[Lvl - 1].Coeff, getSrcType(), false);

      if (IVCoeffs[Lvl - 1].Index != InvalidBlobIndex) {
        MulBlob2 = BlobUtils::createMulBlob(
            MulBlob2, BlobUtils::getBlob(IVCoeffs[Lvl - 1].Index), false);
      }
    }

    // Create an add blob, if necessary.
    if (MulBlob2) {
      AddBlob = BlobUtils::createAddBlob(MulBlob1, MulBlob2, true, &NewIndex);

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

  assert(CanonExprUtils::isValidLinearDefLevel(Lvl) &&
         "Level is out of bounds!");

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

  assert(CanonExprUtils::isValidLinearDefLevel(Lvl) &&
         "Level is out of bounds!");

  // IV not present, nothing to do.
  if ((IVCoeffs.size() < Lvl) || !IVCoeffs[Lvl - 1].Coeff) {
    return;
  }

  // Val is zero, remove IV and return.
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

  assert(CanonExprUtils::isValidLinearDefLevel(Lvl) &&
         "Level is out of bounds!");

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
  assert(BlobUtils::isBlobIndexValid(Index) && "Index is out of bounds!");

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

  assert(!BlobCoeffs.empty() && "Old blob index not found!");

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

  if (!found) {
    assert("Old blob index not found!");
  }
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

void CanonExpr::collectBlobIndicesImpl(SmallVectorImpl<unsigned> &Indices,
                                       bool MakeUnique,
                                       bool NeedTempBlobs) const {

  assert((!MakeUnique || Indices.empty()) &&
         "Empty container expected for uniquing!");

  /// Push all blobs from BlobCoeffs.
  for (auto &I : BlobCoeffs) {
    if (NeedTempBlobs) {

      // Collect temp blobs inside this blob.
      SmallVector<BlobTy, 6> TempBlobs;

      BlobUtils::collectTempBlobs(BlobUtils::getBlob(I.Index), TempBlobs);
      BlobUtils::mapBlobsToIndices(TempBlobs, Indices);

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

      // Collect temp blobs inside this blob.
      SmallVector<BlobTy, 6> TempBlobs;

      BlobUtils::collectTempBlobs(BlobUtils::getBlob(I.Index), TempBlobs);
      BlobUtils::mapBlobsToIndices(TempBlobs, Indices);

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

int64_t CanonExpr::simplifyGCDHelper(int64_t CurrentGCD, int64_t Num) {
  if (CurrentGCD == -1) {
    CurrentGCD = llabs(Num);
  } else {
    CurrentGCD = CanonExprUtils::gcd(CurrentGCD, llabs(Num));
  }

  return CurrentGCD;
}

void CanonExpr::simplify() {
  int64_t Denom = 0, C0 = 0, NumeratorGCD = -1, CommonGCD = 0;

  // Nothing to simplify...
  if ((Denom = getDenominator()) == 1) {
    return;
  }

  // Cannot simplify any further.
  if ((C0 = getConstant()) == 1) {
    return;
  } else if (C0) {
    NumeratorGCD = simplifyGCDHelper(NumeratorGCD, C0);
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

void CanonExpr::multiplyByConstantImpl(int64_t Val, bool Simplify) {

  // Multiplying by constant is equivalent to clearing the canon expr.
  if (Val == 0) {
    clear();
    return;
  }

  // Simplify instead of multiplying, if possible.
  if (Simplify) {
    int64_t Denom = getDenominator();
    int64_t GCD = CanonExprUtils::gcd(llabs(Val), Denom);

    if (GCD != 1) {
      setDenominator(Denom / GCD);
      Val = Val / GCD;
    }
  }

  // Identity multiplication.
  if (Val == 1)
    return;

  // Multiply Val by IVCoeff, BlobCoeffs and Const
  for (auto I = iv_begin(), End = iv_end(); I != End; ++I) {
    multiplyIVByConstant(I, Val);
  }

  for (auto I = blob_begin(), End = blob_end(); I != End; ++I) {
    setBlobCoeff(I, (I->Coeff * Val));
  }

  setConstant(getConstant() * Val);
}

void CanonExpr::multiplyByConstant(int64_t Val) {
  multiplyByConstantImpl(Val, true);
}

void CanonExpr::multiplyByBlob(unsigned Index) {
  assert(BlobUtils::isBlobIndexValid(Index) && "Must be a valid blob index");

  // The canon expr is looking like:
  //   c1*b1*i1 + c2*b2 + c0
  // After multiplication by blob "b" it should be like:
  //   c1*(b*b1)*i1 + c2*(b*b2) + c0*b
  //
  //  1) (b*b1), (b*b2) are new blobs
  //  2) c1 was not touched, just need to switch b1 to (b1*b)
  //  3) b2 blob is removed, the new (b1*b) is added with the same c2 coeff
  //  4) c0 is removed, the b blob is added with c0 coeff

  // Get blob for "b"
  BlobTy MultiplierBlob = BlobUtils::getBlob(Index);

  // Handle IV blob coeffs
  for (auto I = iv_begin(), End = iv_end(); I != End; ++I) {
    unsigned IVBlobIndex;
    int64_t IVConstCoeff;
    getIVCoeff(I, &IVBlobIndex, &IVConstCoeff);

    if (IVBlobIndex != InvalidBlobIndex) {
      BlobTy IVBlob = BlobUtils::getBlob(IVBlobIndex);
      unsigned NewBlobIndex;
      BlobUtils::createMulBlob(IVBlob, MultiplierBlob, true, &NewBlobIndex);
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
      BlobTy Blob = BlobUtils::getBlob(getBlobIndex(I));
      unsigned NewBlobIndex;
      BlobUtils::createMulBlob(Blob, MultiplierBlob, true, &NewBlobIndex);
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

void CanonExpr::negate() { multiplyByConstant(-1); }

void CanonExpr::verify(unsigned NestingLevel) const {
  assert(getDenominator() > 0 && "Denominator must be greater than zero!");

  assert(CanonExprUtils::isValidDefLevel(DefinedAtLevel) &&
         "DefinedAtLevel is invalid!");
  assert(SrcTy && "SrcTy of CanonExpr is null!");
  assert(DestTy && "DestTy of CanonExpr is null!");

  // Account for vector types.
  auto ScalSrcTy = SrcTy->getScalarType();
  auto ScalDestTy = DestTy->getScalarType();

  if (ScalSrcTy != ScalDestTy) {
    assert(((ScalSrcTy->isIntegerTy() && ScalDestTy->isIntegerTy()) ||
            (ScalSrcTy->isPointerTy() && ScalDestTy->isPointerTy())) &&
           "CanonExpr has invalid type!");
  }

  for (auto I = BlobCoeffs.begin(), E = BlobCoeffs.end(); I != E; ++I) {
    BlobTy B = BlobUtils::getBlob(I->Index);
    (void)B;
    auto BScalTy = B->getType()->getScalarType();
    (void)BScalTy;

    // Allow pointer/integer type mismatch as an integral canon expr can look
    // like (ptr1 - ptr2).
    assert(((BScalTy == ScalSrcTy) ||
            (BScalTy->isPointerTy() && ScalSrcTy->isIntegerTy() &&
             (CanonExprUtils::getTypeSizeInBits(BScalTy) ==
              CanonExprUtils::getTypeSizeInBits(ScalSrcTy)))) &&
           "Scalar type of all blobs should match canon expr scalar type!");
  }

  if (!hasBlob() && !hasBlobIVCoeffs()) {
    assert(!isNonLinear() && "CanonExpr with no blobs cannot be non-linear!");
  }

  if (isConstant()) {
    assert(isProperLinear() &&
           " Defined at Level should be 0 for constant canonexpr!");
  }

  assert((!isLinearAtLevel() ||
          (getDefinedAtLevel() < NestingLevel || isProperLinear())) &&
         "CE is undefined at the attached level or should be non-linear.");

  // Verify that there are no undefined IVs.
  for (auto I = iv_begin(), E = iv_end(); I != E; ++I) {
    assert((!(getLevel(I) > NestingLevel) || !hasIVConstCoeff(I)) &&
           "The RegDDRef with IV is attached outside of the loop");
  }
}
