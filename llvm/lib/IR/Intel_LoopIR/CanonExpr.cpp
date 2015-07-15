//===- CanonExpr.cpp - Implements the CanonExpr class -----------*- C++ -*-===//
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

CanonExpr::BlobOrConstToVal::BlobOrConstToVal(bool IsBlobCoef, int64_t Coef)
    : IsBlobCoeff(IsBlobCoef), Coeff(Coef) {}

CanonExpr::BlobOrConstToVal::~BlobOrConstToVal() {}

CanonExpr::BlobIndexToCoeff::BlobIndexToCoeff(unsigned Indx, int64_t Coef)
    : Index(Indx), Coeff(Coef) {}

CanonExpr::BlobIndexToCoeff::~BlobIndexToCoeff() {}

CanonExpr::CanonExpr(Type *Typ, unsigned DefLevel, int64_t ConstVal,
                     int64_t Denom)
    : Ty(Typ), DefinedAtLevel(DefLevel), Const(ConstVal) {

  Objs.insert(this);

  setDenominator(Denom);

  /// Start with size = capcity
  IVCoeffs.resize(IVCoeffs.capacity(), BlobOrConstToVal(false, 0));
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
  unsigned Level = 1;
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

  for (auto I = iv_cbegin(), E = iv_cend(); I != E; ++I, ++Level) {
    if (I->Coeff != 0) {
      if (Printed) {
        OS << " + ";
      } else {
        Printed = true;
      }

      if (I->IsBlobCoeff) {
        CanonExprUtils::printBlob(OS, getBlob(I->Coeff), Detailed);
        OS << " * ";
      } else if (I->Coeff != 1) {
        OS << I->Coeff << " * ";
      }
      OS << "i" << Level;
    }
  }

  for (auto I = blob_cbegin(), E = blob_cend(); I != E; ++I) {
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

CanonExpr::BlobTy CanonExpr::getBlob(unsigned BlobIndex) {
  assert(isBlobIndexValid(BlobIndex) && "BlobIndex is out of bounds.");
  return BlobTable[BlobIndex - 1];
}

bool CanonExpr::isSelfBlob() const {
  return (!hasIV() && (numBlobs() == 1) &&
          CanonExprUtils::isTempBlob(getBlob(getSingleBlobIndex())) &&
          (getSingleBlobCoeff() == 1) && !getConstant() &&
          (getDenominator() == 1));
}

void CanonExpr::setDenominator(int64_t Val) {
  assert((Val != 0) && "Denominator cannot be zero!");

  // Negate the canon expr instead of storing negative denominators.
  if (Val < 0) {
    CanonExprUtils::negate(this);
    Denominator = -Val;
  } else {
    Denominator = Val;
  }
}

bool CanonExpr::hasIV() const {

  bool Ret = false;

  for (auto &I : IVCoeffs) {
    if (I.Coeff != 0) {
      Ret = true;
      break;
    }
  }

  return Ret;
}

unsigned CanonExpr::numIVs() const {

  unsigned Count = 0;

  for (auto &I : IVCoeffs) {
    if (I.Coeff != 0) {
      ++Count;
    }
  }

  return Count;
}

bool CanonExpr::hasBlobIVCoeffs() const {

  bool Ret = false;

  for (auto &I : IVCoeffs) {
    if (I.Coeff != 0) {
      if (I.IsBlobCoeff) {
        Ret = true;
        break;
      }
    }
  }

  return Ret;
}

unsigned CanonExpr::numBlobIVCoeffs() const {
  unsigned Count = 0;

  for (auto &I : IVCoeffs) {
    if ((I.Coeff != 0) && I.IsBlobCoeff) {
      ++Count;
    }
  }

  return Count;
}

int64_t CanonExpr::getIVCoeff(unsigned Lvl, bool *IsBlobCoeff) const {

  assert(IsBlobCoeff && "Non-null IsBlobCoeff ptr expected!");
  assert(isLevelValid(Lvl) && "Level is out of bounds.");

  if (IVCoeffs.size() < Lvl) {
    return 0;
  }

  *IsBlobCoeff = IVCoeffs[Lvl - 1].IsBlobCoeff;

  return IVCoeffs[Lvl - 1].Coeff;
}

bool CanonExpr::resizeIVCoeffsToMax(unsigned Lvl) {

  assert(isLevelValid(Lvl) && "Level is out of bounds.");

  if (IVCoeffs.size() < Lvl) {
    IVCoeffs.resize(MaxLoopNestLevel, BlobOrConstToVal(false, 0));
    return true;
  }

  return false;
}

void CanonExpr::addIVInternal(unsigned Lvl, int64_t Coeff, bool IsBlobCoeff,
                              bool overwrite) {

  assert(isLevelValid(Lvl) && " Level is out of bounds.");
  assert((!IsBlobCoeff || isBlobIndexValid(Coeff)) &&
         " Blob Index is invalid.");

  bool resized;

  resized = resizeIVCoeffsToMax(Lvl);

  if (!overwrite && !resized) {
    assert((!IVCoeffs[Lvl - 1].IsBlobCoeff) && "Blob coefficients cannot be "
                                               "added!");
  }

  if (overwrite) {
    IVCoeffs[Lvl - 1].IsBlobCoeff = IsBlobCoeff;
    IVCoeffs[Lvl - 1].Coeff = Coeff;
  } else {
    IVCoeffs[Lvl - 1].Coeff += Coeff;
  }
}

void CanonExpr::setIVCoeff(unsigned Lvl, int64_t Coeff, bool IsBlobCoeff) {
  addIVInternal(Lvl, Coeff, IsBlobCoeff, true);
}

void CanonExpr::addIV(unsigned Lvl, int64_t Coeff) {
  addIVInternal(Lvl, Coeff, false, false);
}

void CanonExpr::removeIV(unsigned Lvl) {

  assert(isLevelValid(Lvl) && "Level is out of bounds.");

  /// Nothing to do as the IV is not present.
  /// Should we assert on this?
  if (IVCoeffs.size() < Lvl) {
    return;
  }

  IVCoeffs[Lvl - 1].Coeff = 0;
  IVCoeffs[Lvl - 1].IsBlobCoeff = false;
}

void CanonExpr::replaceIVByConstant(unsigned Lvl, int64_t Val) {

  int64_t Coeff;

  assert(((IVCoeffs.size() >= Lvl) && (IVCoeffs[Lvl - 1].Coeff != 0)) &&
         "IV at this level not found!");

  Coeff = IVCoeffs[Lvl - 1].Coeff;

  /// IV coefficient is blob index
  if (IVCoeffs[Lvl - 1].IsBlobCoeff) {
    if (Val != 0)
      addBlob(Coeff, Val);
  }
  /// IV coefficient is constant
  else {
    Const += (Coeff * Val);
  }

  removeIV(Lvl);
}

namespace {
struct BlobIndexCompareLess {
  bool operator()(const CanonExpr::BlobIndexToCoeff &B1,
                  const CanonExpr::BlobIndexToCoeff &B2) {
    return B1.Index < B2.Index;
  }
};

struct BlobIndexCompareEqual {
  bool operator()(const CanonExpr::BlobIndexToCoeff &B1,
                  const CanonExpr::BlobIndexToCoeff &B2) {
    return B1.Index == B2.Index;
  }
};
}

int64_t CanonExpr::getBlobCoeff(unsigned BlobIndex) const {

  BlobIndexToCoeff Blob(BlobIndex, 0);

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob,
                            BlobIndexCompareLess());

  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    return I->Coeff;
  }

  return 0;
}

void CanonExpr::addBlobInternal(unsigned BlobIndex, int64_t BlobCoeff,
                                bool overwrite) {

  assert((BlobCoeff != 0) && " Blob Coeffs cannot be zero.");
  assert(isBlobIndexValid(BlobIndex) && " Blob Index is out of bounds.");

  BlobIndexToCoeff Blob(BlobIndex, BlobCoeff);

  /// No blobs present, add this one
  if (BlobCoeffs.empty()) {
    BlobCoeffs.push_back(Blob);
    return;
  }

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob,
                            BlobIndexCompareLess());

  /// The blob already exists so just change the coeff
  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    if (overwrite) {
      I->Coeff = BlobCoeff;
    } else {
      I->Coeff += BlobCoeff;
      if (I->Coeff == 0) {
        // Blobs cancel out
        removeBlob(BlobIndex);
      }
    }
  }
  /// We need to insert new blob at this (sorted) position
  else {
    BlobCoeffs.insert(I, Blob);
  }
}

void CanonExpr::setBlobCoeff(unsigned BlobIndex, int64_t BlobCoeff) {
  addBlobInternal(BlobIndex, BlobCoeff, true);
}

void CanonExpr::addBlob(unsigned BlobIndex, int64_t BlobCoeff) {
  addBlobInternal(BlobIndex, BlobCoeff, false);
}

void CanonExpr::removeBlob(unsigned BlobIndex) {

  BlobIndexToCoeff Blob(BlobIndex, 0);

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob,
                            BlobIndexCompareLess());

  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    BlobCoeffs.erase(I);
  }
}

void CanonExpr::replaceBlob(unsigned OldBlobIndex, unsigned NewBlobIndex) {

  int64_t Coeff;
  bool found = false;
  BlobIndexToCoeff Blob(OldBlobIndex, 0);

  assert(!BlobCoeffs.empty() && "Old blob index not found!");

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob,
                            BlobIndexCompareLess());

  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    /// Store the coeff as the iterator might get invalidated after erase.
    Coeff = I->Coeff;
    BlobCoeffs.erase(I);
    addBlob(NewBlobIndex, Coeff);
    found = true;
  }

  /// Replace in IV coefficients
  for (auto &I : IVCoeffs) {
    if ((I.IsBlobCoeff) &&
        BlobIndexCompareEqual()(BlobIndexToCoeff(I.Coeff, 0), Blob)) {

      I.Coeff = NewBlobIndex;
      found = true;
    }
  }

  if (!found) {
    assert("Old blob index not found!");
  }
}

void CanonExpr::clear() {
  IVCoeffs.clear();
  BlobCoeffs.clear();

  Const = 0;
  Denominator = 1;

  DefinedAtLevel = 0;
}

void CanonExpr::shift(unsigned Lvl, int64_t Val) {

  /// Nothing to do as the IV is not present.
  if (IVCoeffs.size() < Lvl) {
    return;
  }

  /// Handle blob coefficient of IV
  if (IVCoeffs[Lvl - 1].IsBlobCoeff) {
    addBlob(IVCoeffs[Lvl - 1].Coeff, Val);
  }
  /// Handle constant coefficient of IV
  else {
    Const += (IVCoeffs[Lvl - 1].Coeff * Val);
  }
}

void CanonExpr::extractBlobIndices(SmallVectorImpl<unsigned> &BlobIndices) {

  bool Inserted;

  /// Push all blobs from BlobCoeffs.
  for (auto &I : BlobCoeffs) {
    BlobIndices.push_back(I.Index);
  }

  /// Push all blobs from IVCoeffs which haven't already been inserted.
  for (auto &I : IVCoeffs) {
    if (I.IsBlobCoeff) {
      Inserted = false;

      /// Check whether it has already been inserted.
      for (auto &J : BlobIndices) {
        if (BlobIndexCompareEqual()(BlobIndexToCoeff(J, 0),
                                    BlobIndexToCoeff(I.Coeff, 0))) {
          Inserted = true;
          break;
        }
      }

      if (!Inserted) {
        BlobIndices.push_back(I.Coeff);
      }
    }
  }
}
