//===--- BlobDDRef.cpp - Implements the BlobDDRef class -----------------*-===//
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
// This file implements the BlobDDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"

#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

BlobDDRef::BlobDDRef(unsigned Index, int Level)
    : DDRef(DDRef::BlobDDRefVal, INVALID_SYMBASE), ParentDDRef(nullptr) {

  unsigned Symbase = BlobUtils::getBlobSymbase(Index);

  CE = CanonExprUtils::createSelfBlobCanonExpr(Index, Level);

  setSymbase(Symbase);
}

BlobDDRef::BlobDDRef(const BlobDDRef &BlobDDRefObj)
    : DDRef(BlobDDRefObj), ParentDDRef(nullptr) {

  /// Clone the Canon Expression linked to this BlobDDRef
  assert(BlobDDRefObj.CE && " Canon Expr for BlobDDRefObj cannot be null");
  CE = BlobDDRefObj.CE->clone();
}

BlobDDRef *BlobDDRef::clone() const {

  /// Call Copy constructor
  BlobDDRef *NewBlobDDRef = new BlobDDRef(*this);

  return NewBlobDDRef;
}

void BlobDDRef::print(formatted_raw_ostream &OS, bool Detailed) const {
  CE ? CE->print(OS, Detailed) : (void)(OS << CE);
  DDRef::print(OS, Detailed);
}

HLDDNode *BlobDDRef::getHLDDNode() const {

  if (ParentDDRef) {
    return ParentDDRef->getHLDDNode();
  }

  return nullptr;
}

void BlobDDRef::setHLDDNode(HLDDNode *HNode) {
  llvm_unreachable("Should not set HLDDNode via blob DDRef");
}

void BlobDDRef::replaceBlob(unsigned NewIndex) {
  unsigned OldIndex = CE->getSingleBlobIndex();
  unsigned NewSymbase = BlobUtils::getBlobSymbase(NewIndex);

  CE->replaceBlob(OldIndex, NewIndex);
  setSymbase(NewSymbase);
}

void BlobDDRef::verify() const {
  assert(CE != nullptr && "Canon Expr for BlobDDRefObj cannot be NULL");

  CE->verify();

  assert(CE->isSelfBlob() && "BlobDDRefs should represent a self blob");

  unsigned Index = CE->getSingleBlobIndex();
  unsigned Symbase = BlobUtils::getBlobSymbase(Index);

  (void)Symbase;
  assert((getSymbase() == Symbase) && "blob index/symbase mismatch!");

  DDRef::verify();
}
