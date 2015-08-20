//===--- BlobDDRef.cpp - Implements the BlobDDRef class -----------------*-===//
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
// This file implements the BlobDDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;
using namespace llvm::loopopt;

BlobDDRef::BlobDDRef(int SB, const CanonExpr *CE)
    : DDRef(DDRef::BlobDDRefVal, SB), CExpr(CE), ParentDDRef(nullptr) {}

BlobDDRef::BlobDDRef(const BlobDDRef &BlobDDRefObj)
    : DDRef(BlobDDRefObj), ParentDDRef(nullptr) {

  /// Clone the Canon Expression linked to this BlobDDRef
  assert(BlobDDRefObj.CExpr && " Canon Expr for BlobDDRefObj cannot be null");
  CExpr = BlobDDRefObj.CExpr->clone();
}

BlobDDRef *BlobDDRef::clone() const {

  /// Call Copy constructor
  BlobDDRef *NewBlobDDRef = new BlobDDRef(*this);

  return NewBlobDDRef;
}

void BlobDDRef::print(formatted_raw_ostream &OS, bool Detailed) const {
  auto CE = getCanonExpr();
  CE ? CE->print(OS, Detailed) : (void)(OS << CE);
  OS << " ";
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

void BlobDDRef::verify() const {
  const CanonExpr *CE = getCanonExpr();

  assert(CE != nullptr && "Canon Expr for BlobDDRefObj cannot be NULL");

  CE->verify();

  assert(isSelfBlob() && "BlobDDRefs should represent a self blob");

  DDRef::verify();
}
