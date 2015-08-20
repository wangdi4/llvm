//===- CanonExpr.cpp - Implements the CanonExpr class ---------------------===//
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
// This file implements the CanonExpr class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"

using namespace llvm;
using namespace loopopt;

std::set<CanonExpr *> CanonExpr::Objs;
CanonExpr::BlobTableTy CanonExpr::BlobTable;

CanonExpr::BlobIndexToCoeff::BlobIndexToCoeff(unsigned Indx, int64_t Coef)
    : Index(Indx), Coeff(Coef) {}

CanonExpr::BlobIndexToCoeff::~BlobIndexToCoeff() {}

CanonExpr::CanonExpr(Type *Typ, unsigned DefLevel, int64_t ConstVal,
                     int64_t Denom)
    : Ty(Typ), DefinedAtLevel(DefLevel), Const(ConstVal) {

  Objs.insert(this);

  setDenominator(Denom);

  /// Start with size = capcity
  IVCoeffs.resize(IVCoeffs.capacity(), BlobIndexToCoeff(0, 0));
}

CanonExpr::CanonExpr(const CanonExpr &CE)
    : Ty(CE.Ty), DefinedAtLevel(CE.DefinedAtLevel), IVCoeffs(CE.IVCoeffs),
      BlobCoeffs(CE.BlobCoeffs), Const(CE.Const), Denominator(CE.Denominator) {

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
  BlobTable.clear();
}

CanonExpr *CanonExpr::clone() const {
  CanonExpr *CE = new CanonExpr(*this);
  return CE;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void CanonExpr::dump() const {
  formatted_raw_ostream OS(dbgs());
  print(OS);
}
#endif

void CanonExpr::print(formatted_raw_ostream &OS, bool Detailed) const {
  auto C0 = getConstant();
  auto Denom = getDenominator();
  bool Printed = false;

  if (Detailed && !isConstant()) {
    if (isNonLinear()) {
      OS << "NON-LINEAR ";
    } else {
      OS << "LINEAR ";
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
      if (I->Index) {
        CanonExprUtils::printBlob(OS, getBlob(I->Index), Detailed);
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
      CanonExprUtils::printBlob(OS, getBlob(I->Index), Detailed);
    }
  }

  if (!Printed) {
    OS << C0;
  } else if (C0 != 0) {
    OS << " + " << C0;
  }

  if (Denom != 1) {
    OS << ")/" << Denom;
  }

  if (Detailed) {
    if (isLinearAtLevel() && getDefinedAtLevel() > 0) {
      OS << "{def@" << getDefinedAtLevel() << "}";
    }
  }
}

unsigned CanonExpr::findOrInsertBlobImpl(BlobTy Blob, bool Insert) {
  assert(Blob && "Blob is null!");

  for (auto I = BlobTable.begin(), E = BlobTable.end(); I != E; ++I) {
    if (*I == Blob) {
      return (I - BlobTable.begin() + 1);
    }
  }

  if (Insert) {
    BlobTable.push_back(Blob);
    return BlobTable.size();
  }

  return 0;
}

unsigned CanonExpr::findBlob(BlobTy Blob) {
  return findOrInsertBlobImpl(Blob, false);
}

unsigned CanonExpr::findOrInsertBlob(BlobTy Blob) {
  return findOrInsertBlobImpl(Blob, true);
}

bool CanonExpr::isBlobIndexValid(unsigned Index) {
  return ((Index > 0) && (Index <= BlobTable.size()));
}

bool CanonExpr::isLevelValid(unsigned Level) {
  return ((Level > 0) && (Level <= MaxLoopNestLevel));
}

CanonExpr::BlobTy CanonExpr::getBlob(unsigned Index) {
  assert(isBlobIndexValid(Index) && "Index is out of bounds!");
  return BlobTable[Index - 1];
}

bool CanonExpr::isSelfBlob() const {
  return (!hasIV() && (numBlobs() == 1) &&
          CanonExprUtils::isTempBlob(getBlob(getSingleBlobIndex())) &&
          (getSingleBlobCoeff() == 1) && !getConstant() &&
          (getDenominator() == 1));
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

unsigned CanonExpr::numIVs() const { return numIVImpl(false, false); }

bool CanonExpr::hasBlobIVCoeffs() const { return numIVImpl(true, true); }

unsigned CanonExpr::numBlobIVCoeffs() const { return numIVImpl(false, true); }

unsigned CanonExpr::getLevel(const_iv_iterator ConstIVIter) const {
  return (ConstIVIter - iv_begin() + 1);
}

void CanonExpr::getIVCoeff(unsigned Lvl, unsigned *Index,
                           int64_t *Coeff) const {
  assert((Index || Coeff) && "Non-null Index or Coeff ptr expected!");
  assert(isLevelValid(Lvl) && "Level is out of bounds!");

  if (IVCoeffs.size() < Lvl) {
    if (Index) {
      *Index = 0;
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
  return getIVBlobCoeff(Lvl);
}

bool CanonExpr::hasIVBlobCoeff(const_iv_iterator ConstIVIter) const {
  return getIVBlobCoeff(ConstIVIter);
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
  assert(isLevelValid(Lvl) && "Level is out of bounds!");

  if (IVCoeffs.size() < Lvl) {
    IVCoeffs.resize(MaxLoopNestLevel, BlobIndexToCoeff(0, 0));
  }
}

void CanonExpr::setIVInternal(unsigned Lvl, unsigned Index, int64_t Coeff,
                              bool OverwriteIndex, bool OverwriteCoeff) {

  assert(isLevelValid(Lvl) && "Level is out of bounds!");
  assert((!Index || isBlobIndexValid(Index)) && "Blob Index is invalid!");

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
  setIVInternal(Lvl, 0, Coeff, false, true);
}

void CanonExpr::setIVConstCoeff(iv_iterator IVI, int64_t Coeff) {
  assert(Coeff && "Use removeIV() instead!");
  IVI->Coeff = Coeff;
}

void CanonExpr::addIVInternal(unsigned Lvl, unsigned Index, int64_t Coeff) {

  assert(isLevelValid(Lvl) && "Level is out of bounds!");
  assert((!Index || isBlobIndexValid(Index)) && "Blob Index is invalid!");

  resizeIVCoeffsToMax(Lvl);

  // Nothing to add.
  if (!Coeff) {
    return;
  }

  // Create new blob (C1 * b1 + C2 * b2) if current and incoming blob indices
  // are different.
  // At least one of the indices is non-zero here.
  if (IVCoeffs[Lvl - 1].Index != Index) {
    BlobTy AddBlob, MulBlob1 = nullptr, MulBlob2 = nullptr;
    unsigned NewIndex = 0;
    int64_t NewCoeff = 1;

    // Create a mul blob from new index/coeff.
    MulBlob1 = CanonExprUtils::createBlob(Coeff, false);

    if (Index) {
      MulBlob1 = CanonExprUtils::createMulBlob(MulBlob1, getBlob(Index), true,
                                               &NewIndex);
    }

    // Create a mul blob from existing index/coeff.
    if (IVCoeffs[Lvl - 1].Coeff) {
      MulBlob2 = CanonExprUtils::createBlob(IVCoeffs[Lvl - 1].Coeff, false);

      if (IVCoeffs[Lvl - 1].Index) {
        MulBlob2 = CanonExprUtils::createMulBlob(
            MulBlob2, getBlob(IVCoeffs[Lvl - 1].Index), false);
      }
    }

    // Create an add blob, if necessary.
    if (MulBlob2) {
      AddBlob =
          CanonExprUtils::createAddBlob(MulBlob1, MulBlob2, true, &NewIndex);

      // Check whether the add blob has been simplified to a constant, if so,
      // set it as a constant coefficient.
      // For example: (%b + 2) + (-%b) = 2
      if (CanonExprUtils::isConstantIntBlob(AddBlob, &NewCoeff)) {
        NewIndex = 0;
      }
    }

    assert(((NewCoeff == 1) || (NewIndex == 0)) &&
           "Unexpected merge condition!");

    // Set new index and coefficient.
    IVCoeffs[Lvl - 1].Index = NewIndex;
    IVCoeffs[Lvl - 1].Coeff = NewCoeff;
  } else {
    // Both indices are equal(or zero) so just add the const coefficients.
    IVCoeffs[Lvl - 1].Coeff += Coeff;

    // If coefficient becomes zero, zero out index as well.
    if (!IVCoeffs[Lvl - 1].Coeff) {
      IVCoeffs[Lvl - 1].Index = 0;
    }
  }
}

void CanonExpr::addIV(unsigned Lvl, unsigned Index, int64_t Coeff) {
  addIVInternal(Lvl, Index, Coeff);
}

void CanonExpr::addIV(iv_iterator IVI, unsigned Index, int64_t Coeff) {
  addIV(getLevel(IVI), Index, Coeff);
}

void CanonExpr::removeIV(unsigned Lvl) {

  assert(isLevelValid(Lvl) && "Level is out of bounds!");

  /// Nothing to do as the IV is not present.
  /// Should we assert on this?
  if (IVCoeffs.size() < Lvl) {
    return;
  }

  IVCoeffs[Lvl - 1].Index = 0;
  IVCoeffs[Lvl - 1].Coeff = 0;
}

void CanonExpr::removeIV(iv_iterator IVI) {
  IVI->Index = 0;
  IVI->Coeff = 0;
}

void CanonExpr::replaceIVByConstant(unsigned Lvl, int64_t Val) {

  assert(isLevelValid(Lvl) && "Level is out of bounds!");

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

  if (IVCoeffs[Lvl - 1].Index) {
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

  assert(isLevelValid(Lvl) && "Level is out of bounds!");

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
  assert(isBlobIndexValid(Index) && "Index is out of bounds!");

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

void CanonExpr::addBlob(unsigned Index, int64_t Coeff) {
  addBlobInternal(Index, Coeff, false);
}

void CanonExpr::addBlob(blob_iterator BlobI, int64_t Coeff) {
  BlobI->Coeff += Coeff;

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

  int64_t Coeff;
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
  IVCoeffs.assign(IVCoeffs.size(), BlobIndexToCoeff(0, 0));
}

void CanonExpr::shift(unsigned Lvl, int64_t Val) {

  /// Nothing to do if Val is 0 or the IV is not present.
  if (!Val || (IVCoeffs.size() < Lvl)) {
    return;
  }

  int64_t NewVal = IVCoeffs[Lvl - 1].Coeff * Val;

  /// Handle blob coefficient of IV.
  if (IVCoeffs[Lvl - 1].Index) {
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

void CanonExpr::extractBlobIndices(SmallVectorImpl<unsigned> &Indices) {

  bool Inserted;

  /// Push all blobs from BlobCoeffs.
  for (auto &I : BlobCoeffs) {
    Indices.push_back(I.Index);
  }

  /// Push all blobs from IVCoeffs which haven't already been inserted.
  for (auto &I : IVCoeffs) {
    if (I.Index) {
      Inserted = false;

      /// Check whether it has already been inserted.
      for (auto &J : Indices) {
        if (BlobIndexCompareEqual()(BlobIndexToCoeff(J, 0),
                                    BlobIndexToCoeff(I.Index, 0))) {
          Inserted = true;
          break;
        }
      }

      if (!Inserted) {
        Indices.push_back(I.Index);
      }
    }
  }
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
  int64_t Denom, C0, NumeratorGCD = -1, CommonGCD;

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

  // Numerator is zero, nothing to simplify...
  if (NumeratorGCD == -1) {
    return;
  }

  // Get common gcd of numerator and denominator.
  CommonGCD = CanonExprUtils::gcd(NumeratorGCD, Denom);

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

void CanonExpr::negate() { multiplyByConstant(-1); }

void CanonExpr::verify() const {
  assert(getDenominator() > 0 && "Denominator must be greater than zero");
  assert(DefinedAtLevel >= -1 && DefinedAtLevel <= MaxLoopNestLevel && "DefinedAtLevel must be within range [-1, MaxLoopNestLevel]");

  for (auto I = BlobCoeffs.begin(), E = BlobCoeffs.end(); I != E; ++I) {
    BlobTy B = CanonExpr::getBlob(I->Index);
    assert(B->getType() == getType() && "Types of all blobs should match canon expr type");
  }
}
