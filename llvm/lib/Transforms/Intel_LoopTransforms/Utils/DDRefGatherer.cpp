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

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

// Compares the CanonExpr associated with a memory reference
bool DDRefGathererUtils::compareMemRefCE(const CanonExpr *ACanon,
                                         const CanonExpr *BCanon) {
  // Check the number of IV's.
  if (ACanon->numIVs() != BCanon->numIVs())
    return (ACanon->numIVs() < BCanon->numIVs());

  // Check the IV's.
  for (auto IVIt1 = ACanon->iv_begin(), IVIt2 = BCanon->iv_begin(),
            End = ACanon->iv_end();
       IVIt1 != End; ++IVIt1, ++IVIt2) {
    if (IVIt1->Coeff != IVIt2->Coeff)
      return (IVIt1->Coeff < IVIt2->Coeff);
    if (IVIt1->Index != IVIt2->Index)
      return (IVIt1->Index < IVIt2->Index);
  }

  // Check the number of blobs.
  if (ACanon->numBlobs() != BCanon->numBlobs())
    return (ACanon->numBlobs() < BCanon->numBlobs());

  // Check the Blob's.
  for (auto It1 = ACanon->blob_begin(), End = ACanon->blob_end(),
            It2 = BCanon->blob_begin();
       It1 != End; ++It1, ++It2) {

    if (It1->Coeff != It2->Coeff)
      return (It1->Coeff < It2->Coeff);
    if (It1->Index != It2->Index)
      return (It1->Index < It2->Index);
  }

  if (ACanon->getConstant() != BCanon->getConstant())
    return (ACanon->getConstant() < BCanon->getConstant());

  if (ACanon->getDenominator() != BCanon->getDenominator())
    return (ACanon->getDenominator() < BCanon->getDenominator());

  // Check division type for non-unit denominator.
  if ((ACanon->getDenominator() != 1) &&
      (ACanon->isSignedDiv() != BCanon->isSignedDiv())) {
    return ACanon->isSignedDiv();
  }

  // If ACanon and BCanon have incompatible types, order them using type info.
  if (!CanonExprUtils::mergeable(ACanon, BCanon)) {
    Type *TypeA = ACanon->getDestType();
    Type *TypeB = BCanon->getDestType();

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

  if (ACanon->isNonLinear() != BCanon->isNonLinear()) {
    return ACanon->isNonLinear();
  } else if (!ACanon->isNonLinear()) {
    return ACanon->getDefinedAtLevel() < BCanon->getDefinedAtLevel();
  }

  // Assert, since the two canon expr should differ atleast one case,
  // as we have already checked for their equality.
  llvm_unreachable("CanonExprs should be different.");

  return true;
}

// Sorting comparator operator for two DDRef.
// This sorting compares the two ddref and orders them based on
// the dimensions, IV's, blobs and then writes.
// For example: A[i+5][j], A[i][0] -> Read, A[i][j], A[i+k][0],
// A[i][0] -> Write will be sorted as
// A[i][0] -> Write, A[i][0] -> Read, A[i+k][0], A[i][j], A[i+5][j].
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

  // Check canon expr of the two ddrefs.
  for (auto AIter = Ref1->canon_begin(), End = Ref1->canon_end(),
            BIter = Ref2->canon_begin();
       AIter != End; ++AIter, ++BIter) {

    const CanonExpr *ACanon = *AIter;
    const CanonExpr *BCanon = *BIter;

    if (!CanonExprUtils::areEqual(ACanon, BCanon)) {
      return compareMemRefCE(ACanon, BCanon);
    }
  }

  // Place writes first in case everything matches.
  if (Ref1->isLval() != Ref2->isLval()) {
    return Ref1->isLval();
  }

  return false;
}
