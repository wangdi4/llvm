//===-------- DDRefGatherer.cpp - Implements DDRef gathering utilities ----===//
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
// This file implements DDRefGathererUtils class.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"

#include "llvm/Support/Debug.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

// Compares the CanonExpr associated with a memory reference
bool DDRefGathererUtils::compareMemRefCE(const CanonExpr *CE1,
                                         const CanonExpr *CE2) {
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

  // If CE1 and CE2 have incompatible types, order them using type info.
  if (!CanonExprUtils::mergeable(CE1, CE2)) {
    Type *TypeA = CE1->getDestType();
    Type *TypeB = CE2->getDestType();

    // Get pointer element type (i32 from i32*) to make different pointer types
    // be grouped together during sorting: (i32*, i32*, i64*, i64*, ...)
    while (TypeA->isPointerTy() && TypeB->isPointerTy()) {
      TypeA = TypeA->getPointerElementType();
      TypeB = TypeB->getPointerElementType();
    }

    // Separate by type ID.
    if (TypeA->getTypeID() != TypeB->getTypeID()) {
      return TypeA->getTypeID() < TypeB->getTypeID();
    }

    // Separate types with the same ID by type size.
    return TypeA->getPrimitiveSizeInBits() < TypeB->getPrimitiveSizeInBits();
  }

  if (CE1->isNonLinear() != CE2->isNonLinear()) {
    return CE1->isNonLinear();
  } else if (!CE1->isNonLinear()) {
    return CE1->getDefinedAtLevel() < CE2->getDefinedAtLevel();
  }

  // Assert, since the two canon expr should differ atleast one case,
  // as we have already checked for their equality.
  llvm_unreachable("CanonExprs should be different.");

  return true;
}

// Sorting comparator operator for two DDRef.
// This sorting compares the two ddref and orders them based on Ref's base,
// dimensions, IV's, blobs and then writes. Refs with equal bases (and no blobs)
// are sorted in increasing order of address location.
//
// Consider this set of refs-
// A[i+5][j], A[i][0] (Read), A[i][0] (Write), A[i][j], A[i+k][0]
//
// The sorting order is-
// A[i][0] (Write), A[i][0] (Read), A[i][j], A[i+5][j], A[i+k][0]
//
// As a comparator, compareMemRef must meet the requirements of Compare concept:
// For a long story see http://en.cppreference.com/w/cpp/concept/Compare
//
// Short story:
// Given: comp(a, b), equiv(a, b), an expression equivalent to
//                    !comp(a, b) && !comp(b, a)
//
// For any a, b, c:
// 1) comp(a,a)==false
// 2) if (comp(a,b)==true) comp(b,a)==false
// 3) if (comp(a,b)==true && comp(b,c)==true) comp(a,c)==true
// 4) equiv(a,a)==true
// 5) if (equiv(a,b)==true) equiv(b,a)==true
// 6) if (equiv(a,b)==true && equiv(b,c)==true) equiv(a,c)==true
//
bool DDRefGathererUtils::compareMemRef(const RegDDRef *Ref1,
                                       const RegDDRef *Ref2) {

  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE()))
    return compareMemRefCE(Ref1->getBaseCE(), Ref2->getBaseCE());

  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return (Ref1->getNumDimensions() < Ref2->getNumDimensions());
  }

  // Check dimensions from highest to lowest.
  for (unsigned I = Ref1->getNumDimensions(); I > 0; --I) {
    const CanonExpr *CE1 = Ref1->getDimensionIndex(I);
    const CanonExpr *CE2 = Ref2->getDimensionIndex(I);

    if (!CanonExprUtils::areEqual(CE1, CE2)) {
      return compareMemRefCE(CE1, CE2);
    }

    auto Diff = DDRefUtils::compareOffsets(Ref1, Ref2, I);

    if (Diff != 0) {
      return (Diff < 0);
    }
  }

  // Place writes first in case everything matches.
  if (Ref1->isLval() != Ref2->isLval()) {
    return Ref1->isLval();
  }

  return false;
}
