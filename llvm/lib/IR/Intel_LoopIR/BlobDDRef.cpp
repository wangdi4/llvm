//===--- BlobDDRef.cpp - Implements the BlobDDRef class ---------*- C++ -*-===//
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

BlobDDRef::BlobDDRef(int SB, const CanonExpr *CE, RegDDRef *Parent)
    : DDRef(DDRef::BlobDDRefVal, SB), CExpr(CE), ParentDDRef(Parent) {}

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

void BlobDDRef::print(formatted_raw_ostream &OS) const {
  auto CE = getCanonExpr();

  CE ? CE->print(OS) : (void)(OS << CE);
}

void BlobDDRef::detailedPrint(formatted_raw_ostream &OS) const {
  print(OS);

  OS << " Symbase: " << getSymBase();
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
