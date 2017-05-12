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
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

BlobDDRef::BlobDDRef(DDRefUtils &DDRU, unsigned Index, unsigned Level)
    : DDRef(DDRU, DDRef::BlobDDRefVal, InvalidSymbase), ParentDDRef(nullptr) {

  unsigned Symbase = getBlobUtils().getTempBlobSymbase(Index);

  CE = getCanonExprUtils().createSelfBlobCanonExpr(Index, Level);

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
  unsigned NewSymbase = getBlobUtils().getTempBlobSymbase(NewIndex);

  // Resetting CE's source and dest. to corresp. type
  Type *Ty = getBlobUtils().getBlob(NewIndex)->getType();
  CE->setSrcAndDestType(Ty);

  CE->replaceBlob(OldIndex, NewIndex);
  setSymbase(NewSymbase);
}

void BlobDDRef::verify() const {
  assert(CE != nullptr && "Canon Expr for BlobDDRefObj cannot be NULL");

  CE->verify(getNodeLevel());

  assert(CE->isSelfBlob() && "BlobDDRefs should represent a self blob");

  unsigned Index = CE->getSingleBlobIndex();
  unsigned Symbase = getBlobUtils().getTempBlobSymbase(Index);

  (void)Symbase;
  assert((getSymbase() == Symbase) && "blob index/symbase mismatch!");
  assert((Symbase > ConstantSymbase) &&
         "Found blob DDRef representing a constant!");
  DDRef::verify();
}
