//===--- BlobDDRef.cpp - Implements the BlobDDRef class ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the BlobDDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

using namespace llvm;
using namespace llvm::loopopt;


BlobDDRef::BlobDDRef(int SB, CanonExpr* CE, RegDDRef* Parent)
  : DDRef(DDRef::BlobDDRefVal, SB), CExpr(CE), ParentDDRef(Parent) { }

BlobDDRef::BlobDDRef(const BlobDDRef &BlobDDRefObj)
  : DDRef(BlobDDRefObj), ParentDDRef(nullptr) {

  /// Clone the Canon Expression linked to this BlobDDRef
  assert(BlobDDRefObj.CExpr && " Canon Expr for BlobDDRefObj cannot be null");
  CExpr = BlobDDRefObj.CExpr->clone();
}

BlobDDRef* BlobDDRef::clone() const {

  /// Call Copy constructor
  BlobDDRef *NewBlobDDRef = new BlobDDRef(*this);

  return NewBlobDDRef;
}

HLNode* BlobDDRef::getHLNode() const {

  if (ParentDDRef) {
    return ParentDDRef->getHLNode();
  }

  return nullptr;
}

void BlobDDRef::setHLNode(HLNode* HNode) {
  llvm_unreachable("Should not set HLNode via blob DDRef");
}

