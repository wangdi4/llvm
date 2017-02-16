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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"

using namespace llvm;
using namespace loopopt;

Function &BlobUtils::getFunction() const {
  return getHIRParser().getFunction();
}

Module &BlobUtils::getModule() const { return getHIRParser().getModule(); }

LLVMContext &BlobUtils::getContext() const {
  return getHIRParser().getContext();
}

const DataLayout &BlobUtils::getDataLayout() const {
  return getHIRParser().getDataLayout();
}

unsigned BlobUtils::findBlob(BlobTy Blob) {
  return getHIRParser().findBlob(Blob);
}

unsigned BlobUtils::findTempBlobSymbase(BlobTy Blob) {
  return getHIRParser().findTempBlobSymbase(Blob);
}

unsigned BlobUtils::findTempBlobIndex(unsigned Symbase) {
  return getHIRParser().findTempBlobIndex(Symbase);
}

unsigned BlobUtils::findOrInsertTempBlobIndex(unsigned Symbase) {
  return getHIRParser().findOrInsertTempBlobIndex(Symbase);
}

unsigned BlobUtils::findOrInsertBlob(BlobTy Blob) {
  return getHIRParser().findOrInsertBlob(Blob, InvalidBlobIndex);
}

void BlobUtils::mapBlobsToIndices(const SmallVectorImpl<BlobTy> &Blobs,
                                  SmallVectorImpl<unsigned> &Indices) {
  getHIRParser().mapBlobsToIndices(Blobs, Indices);
}

BlobTy BlobUtils::getBlob(unsigned BlobIndex) {
  return getHIRParser().getBlob(BlobIndex);
}

unsigned BlobUtils::getTempBlobSymbase(unsigned BlobIndex) {
  return getHIRParser().getTempBlobSymbase(BlobIndex);
}

bool BlobUtils::isBlobIndexValid(unsigned BlobIndex) {
  return getHIRParser().isBlobIndexValid(BlobIndex);
}

void BlobUtils::printBlob(raw_ostream &OS, BlobTy Blob) {
  getHIRParser().printBlob(OS, Blob);
}

void BlobUtils::printScalar(raw_ostream &OS, unsigned Symbase) {
  getHIRParser().printScalar(OS, Symbase);
}

bool BlobUtils::isConstantIntBlob(BlobTy Blob, int64_t *Val) {
  return getHIRParser().isConstantIntBlob(Blob, Val);
}

bool BlobUtils::isTempBlob(BlobTy Blob) {
  return getHIRParser().isTempBlob(Blob);
}

bool BlobUtils::isNestedBlob(BlobTy Blob) {
  return getHIRParser().isNestedBlob(Blob);
}

bool BlobUtils::isGuaranteedProperLinear(BlobTy TempBlob) {
  return getHIRParser().isGuaranteedProperLinear(TempBlob);
}

bool BlobUtils::isUndefBlob(BlobTy Blob) {
  return getHIRParser().isUndefBlob(Blob);
}

bool BlobUtils::isConstantFPBlob(BlobTy Blob, ConstantFP **Val) {
  return getHIRParser().isConstantFPBlob(Blob, Val);
}

bool BlobUtils::isConstantVectorBlob(BlobTy Blob, Constant **Val) {
  return getHIRParser().isConstantVectorBlob(Blob, Val);
}

bool BlobUtils::isMetadataBlob(BlobTy Blob, MetadataAsValue **Val) {
  return getHIRParser().isMetadataBlob(Blob, Val);
}

bool BlobUtils::isSignExtendBlob(BlobTy Blob, BlobTy *Val) {
  return getHIRParser().isSignExtendBlob(Blob, Val);
}

BlobTy BlobUtils::createBlob(Value *TempVal, unsigned Symbase, bool Insert,
                             unsigned *NewBlobIndex) {
  return getHIRParser().createBlob(TempVal, Symbase, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createBlob(Value *Val, bool Insert, unsigned *NewBlobIndex) {
  return getHIRParser().createBlob(Val, InvalidSymbase, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createBlob(int64_t Val, Type *Ty, bool Insert,
                             unsigned *NewBlobIndex) {
  return getHIRParser().createBlob(Val, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUndefBlob(Type *Ty, bool Insert,
                                  unsigned *NewBlobIndex) {
  Value *UndefValue = UndefValue::get(Ty);
  auto Blob = createBlob(UndefValue, false);
  unsigned BlobIndex = findBlob(Blob);

  if (Insert && BlobIndex == InvalidBlobIndex) {
    HIRSymbaseAssignment &HSA = getHIRSymbaseAssignment();
    return createBlob(UndefValue, HSA.getNewSymbase(), true, NewBlobIndex);
  }

  if (NewBlobIndex) {
    *NewBlobIndex = BlobIndex;
  }

  return Blob;
}

BlobTy BlobUtils::createAddBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  return getHIRParser().createAddBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createMinusBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                  unsigned *NewBlobIndex) {
  return getHIRParser().createMinusBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createMulBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                unsigned *NewBlobIndex) {
  return getHIRParser().createMulBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUDivBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createUDivBlob(LHS, RHS, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createTruncateBlob(BlobTy Blob, Type *Ty, bool Insert,
                                     unsigned *NewBlobIndex) {
  return getHIRParser().createTruncateBlob(Blob, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createZeroExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  return getHIRParser().createZeroExtendBlob(Blob, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createSignExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                                       unsigned *NewBlobIndex) {
  return getHIRParser().createSignExtendBlob(Blob, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createCastBlob(BlobTy Blob, bool IsSExt, Type *Ty,
                                 bool Insert, unsigned *NewBlobIndex) {
  return getHIRParser().createCastBlob(Blob, IsSExt, Ty, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createSMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createSMinBlob(BlobA, BlobB, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createSMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createSMaxBlob(BlobA, BlobB, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createUMinBlob(BlobA, BlobB, Insert, NewBlobIndex);
}

BlobTy BlobUtils::createUMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                                 unsigned *NewBlobIndex) {
  return getHIRParser().createUMaxBlob(BlobA, BlobB, Insert, NewBlobIndex);
}

bool BlobUtils::contains(BlobTy Blob, BlobTy SubBlob) {
  return getHIRParser().contains(Blob, SubBlob);
}

void BlobUtils::collectTempBlobs(BlobTy Blob,
                                 SmallVectorImpl<BlobTy> &TempBlobs) {
  getHIRParser().collectTempBlobs(Blob, TempBlobs);
}

void BlobUtils::collectTempBlobs(unsigned BlobIndex,
                                 SmallVectorImpl<unsigned> &TempBlobIndices) {
  SmallVector<BlobTy, 8> TempBlobs;

  collectTempBlobs(getBlob(BlobIndex), TempBlobs);
  mapBlobsToIndices(TempBlobs, TempBlobIndices);
}

bool BlobUtils::replaceTempBlob(unsigned BlobIndex, unsigned OldTempIndex,
                                unsigned NewTempIndex, unsigned &NewBlobIndex) {
  return getHIRParser().replaceTempBlob(BlobIndex, OldTempIndex, NewTempIndex,
                                        NewBlobIndex);
}

Value *BlobUtils::getTempBlobValue(unsigned BlobIndex) {
  BlobTy Blob = getBlob(BlobIndex);
  assert(isTempBlob(Blob) && "BlobIndex is not a temp blob");
  return cast<SCEVUnknown>(Blob)->getValue();
}
