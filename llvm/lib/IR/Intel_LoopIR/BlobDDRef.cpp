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

using namespace llvm;
using namespace llvm::loopopt;


BlobDDRef::BlobDDRef(int SB, CanonExpr* CE, RegDDRef* Parent)
  : DDRef(DDRef::BlobDDRefVal, SB), CExpr(CE), ParentDDRef(Parent) { }

BlobDDRef* BlobDDRef::clone_impl() const {
  // TODO: placeholder, implement later
  return nullptr;
}

HLDDNode* BlobDDRef::getHLNode() const {

  if (ParentDDRef) {
    return ParentDDRef->getHLNode();
  }

  return nullptr;
}

void BlobDDRef::setHLNode(HLDDNode* HNode) {
  llvm_unreachable("Should not set HLNode via blob DDRef");
}

