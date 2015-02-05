//===- CanonExpr.cpp - Implements the CanonExpr class -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CanonExpr class.
//
//===----------------------------------------------------------------------===//


#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

using namespace llvm;
using namespace loopopt;

std::set< CanonExpr* >CanonExpr::Objs;
CanonExpr::BlobTableTy CanonExpr::BlobTable;

CanonExpr::CanonExpr(Type* Typ, bool Gen, int DefLevel, int64_t Cons, 
  int64_t Denom)
  : Ty(Typ), Generable(Gen), DefinedAtLevel(DefLevel), Const(Cons)
  , Denominator(Denom) {

  Objs.insert(this);

  /// Start with size = capcity
  IVCoeffs.assign(IVCoeffs.capacity(), BlobOrConstToValTy(false, 0));
}

CanonExpr::CanonExpr(const CanonExpr& CE)
  : Ty(CE.Ty), Generable(CE.Generable), DefinedAtLevel(CE.DefinedAtLevel) 
  , IVCoeffs(CE.IVCoeffs), BlobCoeffs(CE.BlobCoeffs), Const(CE.Const)
  , Denominator(CE.Denominator) {
  
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

CanonExpr* CanonExpr::clone() const {
  
  CanonExpr* CE = new CanonExpr(*this);
  return CE;
}

void CanonExpr::dump() const {
  // TODO: placeholder, implement later
}

void CanonExpr::print() const {
  // TODO: placeholder, implement later
}

bool CanonExpr::hasIV() const {
  
  bool ret = false;

  for (auto &I : IVCoeffs) {
    if (I.second != 0) {
      ret = true;
      break;
    }
  }

  return ret;
}

int64_t CanonExpr::getIVCoeff(unsigned Lvl, bool* isBlobCoeff) const {
  
  assert(isBlobCoeff && "Non-null isBlobCoeff ptr expected");
  assert((Lvl <= MaxLoopNestLevel) && "Level is out of bounds");

  if (IVCoeffs.size() < Lvl) {
    return 0;
  }

  *isBlobCoeff = IVCoeffs[Lvl-1].first;
  
  return IVCoeffs[Lvl-1].second;
}

bool CanonExpr::resizeIVCoeffsToMax(unsigned Lvl) {

  assert((Lvl <= MaxLoopNestLevel) && "Level is out of bounds");

  if (IVCoeffs.size() < Lvl) {
    IVCoeffs.resize(MaxLoopNestLevel, BlobOrConstToValTy(false, 0));
    return true;
  }

  return false;
}

void CanonExpr::addIVInternal(unsigned Lvl, int64_t Coeff, bool IsBlobCoeff,
  bool overwrite) {

  bool resized;

  resized = resizeIVCoeffsToMax(Lvl);

  if (!overwrite && !resized) {
    assert((IVCoeffs[Lvl-1].first == false) && "Blob coefficients cannot be " 
      "added");
  }

  if (overwrite) {
    IVCoeffs[Lvl-1].first = IsBlobCoeff;
    IVCoeffs[Lvl-1].second = Coeff;
  }
  else {
    IVCoeffs[Lvl-1].second += Coeff;
  }
}

void CanonExpr::setIVCoeff(unsigned Lvl, int64_t Coeff, bool isBlobCoeff) {
  addIVInternal(Lvl, Coeff, isBlobCoeff, true);
}

void CanonExpr::addIV(unsigned Lvl, int64_t Coeff) {
  addIVInternal(Lvl, Coeff, false, false);
}

void CanonExpr::removeIV(unsigned Lvl) {

  assert((Lvl <= MaxLoopNestLevel) && "Level is out of bounds");

  /// Nothing to do as the IV is not present.
  /// Should we assert on this?
  if (IVCoeffs.size() < Lvl) {
    return;
  }  

  IVCoeffs[Lvl-1].second = 0;
}

void CanonExpr::replaceIVByConstant(unsigned Lvl, int64_t Val) {

  int64_t Coeff;

  assert(((IVCoeffs.size() >= Lvl) && (IVCoeffs[Lvl-1].second != 0)) && "IV "
    "at this level not found");

  Coeff = IVCoeffs[Lvl-1].second;

  /// IV coefficient is blob index
  if (IVCoeffs[Lvl-1].first == true) {
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
    bool operator() (const CanonExpr::BlobIndexToCoeffTy& B1, 
      const CanonExpr::BlobIndexToCoeffTy& B2) {
      return B1.first < B2.first;
    }
  };

  struct BlobIndexCompareEqual {
    bool operator() (const CanonExpr::BlobIndexToCoeffTy& B1,
      const CanonExpr::BlobIndexToCoeffTy& B2) {
      return B1.first == B2.first;
    }
  };
}

int64_t CanonExpr::getBlobCoeff(unsigned BlobIndex) const {

  BlobIndexToCoeffTy Blob(BlobIndex, 0);

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob, 
            BlobIndexCompareLess());

  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    return I->second;
  }

  return 0;
}

void CanonExpr::addBlobInternal(unsigned BlobIndex, int64_t BlobCoeff, 
  bool overwrite) {

   BlobIndexToCoeffTy Blob(BlobIndex, BlobCoeff);

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
      I->second = BlobCoeff;
    }
    else {
      I->second += BlobCoeff;
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
  
  BlobIndexToCoeffTy Blob(BlobIndex, 0);

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob, 
            BlobIndexCompareLess());

  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) { 
    BlobCoeffs.erase(I);
  }
}

void CanonExpr::replaceBlob(unsigned OldBlobIndex, unsigned NewBlobIndex) {

  int64_t Coeff;
  bool found = false;
  BlobIndexToCoeffTy Blob(OldBlobIndex, 0);

  assert(!BlobCoeffs.empty() && "Old blob index not found");

  auto I = std::lower_bound(BlobCoeffs.begin(), BlobCoeffs.end(), Blob, 
            BlobIndexCompareLess());

  if ((I != BlobCoeffs.end()) && BlobIndexCompareEqual()(*I, Blob)) {
    /// Store the coeff as the iterator might get invalidated after erase.
    Coeff = I->second;
    BlobCoeffs.erase(I);
    addBlob(NewBlobIndex, Coeff);
    found = true;
  }

  /// Replace in IV coefficients
  for (auto &I : IVCoeffs) {
    if ((I.first == true) && 
      BlobIndexCompareEqual()(BlobIndexToCoeffTy(I.second, 0), Blob)) {

      I.second = NewBlobIndex;
      found = true;
    }
  }

  if (!found) {
    assert("Old blob index not found");
  }
}

void CanonExpr::shift(unsigned Lvl, int64_t Val) {

  /// Nothing to do as the IV is not present.
  if (IVCoeffs.size() < Lvl) {
    return;
  }

  /// Handle blob coefficient of IV
  if (IVCoeffs[Lvl-1].first == true) {
    addBlob(IVCoeffs[Lvl-1].second, Val);
  }
  /// Handle constant coefficient of IV
  else {
    Const += (IVCoeffs[Lvl-1].second * Val);
  }
    
}

