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

#include "llvm/Support/ErrorHandling.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"

using namespace llvm;
using namespace loopopt;

HIRParser *BlobUtils::HIRP(nullptr);

unsigned BlobUtils::findBlob(BlobTy Blob) {
  return getHIRParser()->findBlob(Blob);
}

unsigned BlobUtils::findBlobSymbase(BlobTy Blob) {
  return getHIRParser()->findBlobSymbase(Blob);
}

unsigned BlobUtils::findOrInsertBlob(BlobTy Blob) {
  return getHIRParser()->findOrInsertBlob(Blob, 0);
}

void BlobUtils::mapBlobsToIndices(const SmallVectorImpl<BlobTy> &Blobs,
                                  SmallVectorImpl<unsigned> &Indices) {
  getHIRParser()->mapBlobsToIndices(Blobs, Indices);
}

BlobTy BlobUtils::getBlob(unsigned BlobIndex) {
  return getHIRParser()->getBlob(BlobIndex);
}

unsigned BlobUtils::getBlobSymbase(unsigned BlobIndex) {
  return getHIRParser()->getBlobSymbase(BlobIndex);
}

bool BlobUtils::isBlobIndexValid(unsigned BlobIndex) {
  return getHIRParser()->isBlobIndexValid(BlobIndex);
}

void BlobUtils::printBlob(raw_ostream &OS, BlobTy Blob) {
  getHIRParser()->printBlob(OS, Blob);
}

void BlobUtils::printScalar(raw_ostream &OS, unsigned Symbase) {
  getHIRParser()->printScalar(OS, Symbase);
}

bool BlobUtils::isConstantIntBlob(BlobTy Blob, int64_t *Val) {
  return getHIRParser()->isConstantIntBlob(Blob, Val);
}

bool BlobUtils::isTempBlob(BlobTy Blob) {
  return getHIRParser()->isTempBlob(Blob);
}

bool BlobUtils::isGuaranteedProperLinear(BlobTy TempBlob) {
  return getHIRParser()->isGuaranteedProperLinear(TempBlob);
}

bool BlobUtils::isUndefBlob(BlobTy Blob) {
  return getHIRParser()->isUndefBlob(Blob);
}

bool BlobUtils::isConstantFPBlob(BlobTy Blob, ConstantFP **Val) {
  return getHIRParser()->isConstantFPBlob(Blob, Val);
}

bool BlobUtils::isConstantVectorBlob(BlobTy Blob, Constant **Val) {
  return getHIRParser()->isConstantVectorBlob(Blob, Val);
}

bool BlobUtils::isMetadataBlob(BlobTy Blob, MetadataAsValue **Val) {
  return getHIRParser()->isMetadataBlob(Blob, Val);
}

BlobTy BlobUtils::createBlob(Value *TempVal, unsigned Symbase, bool Insert,
                             unsigned *NewBlobIndex) {
  return getHIRParser()->createBlob(TempVal, Symbase, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createBlob(Value *Val, bool Insert, unsigned *NewBlobIndex) {
  return getHIRParser()->createBlob(Val, 0, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createBlob(int64_t Val, Type *Ty, bool Insert,
                             unsigned *NewBlobIndex) {
  return getHIRParser()->createBlob(Val, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createAddBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  return getHIRParser()->createAddBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createMinusBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                  unsigned *NewBlobIndex) {
  return getHIRParser()->createMinusBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createMulBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  return getHIRParser()->createMulBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUDivBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser()->createUDivBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createTruncateBlob(BlobTy Blob, Type *Ty, bool Insert,
                                     unsigned *NewBlobIndex) {
  return getHIRParser()->createTruncateBlob(Blob, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createZeroExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  return getHIRParser()->createZeroExtendBlob(Blob, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createSignExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  return getHIRParser()->createSignExtendBlob(Blob, Ty, Insert, NewBlobIndex);
}

bool BlobUtils::contains(BlobTy Blob, BlobTy SubBlob) {
  return getHIRParser()->contains(Blob, SubBlob);
}

void BlobUtils::collectTempBlobs(BlobTy Blob,
                                 SmallVectorImpl<BlobTy> &TempBlobs) {
  return getHIRParser()->collectTempBlobs(Blob, TempBlobs);
}
