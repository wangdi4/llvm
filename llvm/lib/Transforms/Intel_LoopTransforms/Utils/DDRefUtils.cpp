//===-------- DDRefUtils.cpp - Implements DDRefUtils class ----------------===//
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
// This file implements DDRefUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/IR/Metadata.h" // needed for MetadataAsValue -> Value
#include "llvm/IR/Constants.h" // needed for UndefValue class

using namespace llvm;
using namespace loopopt;

#define DEBUG_TYPE "ddref-utils"

RegDDRef *DDRefUtils::createRegDDRef(unsigned SB) { return new RegDDRef(SB); }

RegDDRef *DDRefUtils::createScalarRegDDRef(unsigned SB, CanonExpr *CE) {
  assert(CE && " CanonExpr is null.");
  RegDDRef *RegDD = createRegDDRef(SB);
  RegDD->setSingleCanonExpr(CE);
  return RegDD;
}

RegDDRef *DDRefUtils::createConstDDRef(Type *Ty, int64_t Val) {
  RegDDRef *NewRegDD = createRegDDRef(CONSTANT_SYMBASE);
  CanonExpr *CE = CanonExprUtils::createCanonExpr(Ty, 0, Val);
  NewRegDD->setSingleCanonExpr(CE);

  return NewRegDD;
}

RegDDRef *DDRefUtils::createConstDDRef(MetadataAsValue *Val) {
  RegDDRef *NewRegDD = createRegDDRef(CONSTANT_SYMBASE);
  // Create a linear self-blob constant canon expr.
  auto CE = CanonExprUtils::createSelfBlobCanonExpr(Val, CONSTANT_SYMBASE);
  NewRegDD->setSingleCanonExpr(CE);

  return NewRegDD;
}

RegDDRef *DDRefUtils::createConstDDRef(ConstantAggregateZero *Val) {
  RegDDRef *NewRegDD = createRegDDRef(CONSTANT_SYMBASE);
  // Create a linear self-blob constant canon expr.
  auto CE = CanonExprUtils::createSelfBlobCanonExpr(Val, CONSTANT_SYMBASE);
  NewRegDD->setSingleCanonExpr(CE);

  return NewRegDD;
}

RegDDRef *DDRefUtils::createConstDDRef(ConstantDataVector *Val) {
  RegDDRef *NewRegDD = createRegDDRef(CONSTANT_SYMBASE);
  // Create a linear self-blob constant canon expr.
  auto CE = CanonExprUtils::createSelfBlobCanonExpr(Val, CONSTANT_SYMBASE);
  NewRegDD->setSingleCanonExpr(CE);

  return NewRegDD;
}

RegDDRef *DDRefUtils::createUndefDDRef(Type *Ty) {
  auto Blob = BlobUtils::createBlob(UndefValue::get(Ty), false);
  unsigned BlobIndex = BlobUtils::findBlob(Blob);

  if (BlobIndex != INVALID_BLOB_INDEX) {
    return createSelfBlobRef(BlobIndex, 0);
  }
  RegDDRef *Ref = createSelfBlobRef(UndefValue::get(Ty));
  Ref->getSingleCanonExpr()->setDefinedAtLevel(0);
  return Ref;
}

BlobDDRef *DDRefUtils::createBlobDDRef(unsigned Index, int Level) {
  return new BlobDDRef(Index, Level);
}

void DDRefUtils::destroy(DDRef *Ref) { Ref->destroy(); }

void DDRefUtils::destroyAll() { DDRef::destroyAll(); }

unsigned DDRefUtils::getNewSymbase() {
  return getHIRFramework()->getNewSymbase();
}

RegDDRef *DDRefUtils::createSelfBlobRef(Value *Temp) {
  unsigned Symbase = DDRefUtils::getNewSymbase();

  // Create a non-linear self-blob canon expr.
  auto CE = CanonExprUtils::createSelfBlobCanonExpr(Temp, Symbase);

  // Register new lval with HIRFramework for printing.
  getHIRFramework()->insertHIRLval(Temp, Symbase);

  // Create a RegDDRef with the new symbase and canon expr.
  auto Ref = DDRefUtils::createRegDDRef(Symbase);
  Ref->setSingleCanonExpr(CE);

  return Ref;
}

bool DDRefUtils::areEqualImpl(const BlobDDRef *Ref1, const BlobDDRef *Ref2) {

  assert(Ref1 && Ref2 && "Ref1/Ref2 parameter is null.");

  if ((Ref1->getSymbase() != Ref2->getSymbase())) {
    return false;
  }

  // Additional check. Ideally, symbase match should be equal blobs.
  assert(CanonExprUtils::areEqual(Ref1->getCanonExpr(), Ref2->getCanonExpr()));

  return true;
}

bool DDRefUtils::areEqualImpl(const RegDDRef *Ref1, const RegDDRef *Ref2,
                              bool IgnoreDestType) {

  // Match the symbase. Type checking is done inside the CEUtils.
  if ((Ref1->getSymbase() != Ref2->getSymbase())) {
    return false;
  }

  // Check if one is memory ref and other is not.
  if (Ref1->hasGEPInfo() != Ref2->hasGEPInfo()) {
    return false;
  }

  // Check Base Canon Exprs.
  if (Ref1->hasGEPInfo() &&
      !CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE(),
                                IgnoreDestType)) {
    return false;
  }

  // TODO: Think about if we can delinearize the subscripts.
  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return false;
  }

  for (auto Ref1Iter = Ref1->canon_begin(), End = Ref1->canon_end(),
            Ref2Iter = Ref2->canon_begin();
       Ref1Iter != End; ++Ref1Iter, ++Ref2Iter) {

    const CanonExpr *Ref1CE = *Ref1Iter;
    const CanonExpr *Ref2CE = *Ref2Iter;

    if (!CanonExprUtils::areEqual(Ref1CE, Ref2CE)) {
      return false;
    }
  }

  // All the canon expr match.
  return true;
}

bool DDRefUtils::areEqual(const DDRef *Ref1, const DDRef *Ref2,
                          bool IgnoreDestType) {

  assert(Ref1 && Ref2 && "Ref1/Ref2 parameter is null.");

  if (auto BRef1 = dyn_cast<BlobDDRef>(Ref1)) {
    auto BRef2 = dyn_cast<BlobDDRef>(Ref2);
    // Ref2 is Reg/Unknown Type, whereas Ref1 is Blob.
    if (!BRef2) {
      assert(isa<RegDDRef>(Ref2) && "Ref2 is unknown type.");
      return false;
    }

    return areEqualImpl(BRef1, BRef2);

  } else if (auto RRef1 = dyn_cast<RegDDRef>(Ref1)) {
    auto RRef2 = dyn_cast<RegDDRef>(Ref2);
    // Ref2 is Blob/Unknown Type, whereas Ref1 is Blob.
    if (!RRef2) {
      assert(isa<BlobDDRef>(Ref2) && "Ref2 is unknown type.");
      return false;
    }

    return areEqualImpl(RRef1, RRef2, IgnoreDestType);

  } else {
    llvm_unreachable("Unknown DDRef kind!");
  }

  return false;
}

RegDDRef *DDRefUtils::createSelfBlobRef(unsigned Index, int Level) {
  auto CE = CanonExprUtils::createSelfBlobCanonExpr(Index, Level);
  unsigned Symbase = BlobUtils::getBlobSymbase(Index);

  auto Ref = DDRefUtils::createRegDDRef(Symbase);
  Ref->setSingleCanonExpr(CE);

  return Ref;
}
