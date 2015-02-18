//===- RegDDRef.cpp - Implements the RegDDRef class ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the RegDDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

using namespace llvm;
using namespace llvm::loopopt;

RegDDRef::RegDDRef(int SB, HLNode* HNode)
  : DDRef(DDRef::RegDDRefVal, SB), Node(HNode) { }

RegDDRef::RegDDRef(const RegDDRef &RegDDRefObj)
  : DDRef(RegDDRefObj), Node(nullptr) {

  /// Loop over Canon Exprs and BlobDDRefs
  /// TODO: Replace iterator by Vector iters when available
  for (std::vector<CanonExpr*>::const_iterator Iter =
       RegDDRefObj.CanonExprs.begin(), IterEnd = RegDDRefObj.CanonExprs.end();
       Iter != IterEnd; ++Iter) {
    CanonExpr *NewCE = (*Iter)->clone();
    CanonExprs.push_back(NewCE);
  }

  for (std::vector<BlobDDRef*>::const_iterator Iter =
       RegDDRefObj.BlobDDRefs.begin(),
       IterEnd = RegDDRefObj.BlobDDRefs.end(); Iter != IterEnd; ++Iter) {
    BlobDDRef *NewBlobDDRef = (*Iter)->clone();
    /// TODO: Check if push_back call sets the parent DDRef appropriately
    /// NewBlobDDRef->setParentDDRef(this);
    BlobDDRefs.push_back(NewBlobDDRef);
  }

  /// TODO: Check later if we need more handling for GEP
  if(RegDDRefObj.GepInfo)
    setGEP(RegDDRefObj.GepInfo->BaseCE, RegDDRefObj.GepInfo->Strides,
           RegDDRefObj.GepInfo->inbounds);
}

RegDDRef* RegDDRef::clone() const {

  /// Call Copy constructor
  RegDDRef *NewRegDDRef = new RegDDRef(*this);

  return NewRegDDRef;
}

