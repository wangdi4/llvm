//===- ConstDDRef.cpp - Implements the ConstDDRef class ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the ConstDDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/ConstDDRef.h"

using namespace llvm;
using namespace llvm::loopopt;

ConstDDRef::ConstDDRef(CanonExpr *CE)
    : DDRef(DDRef::ConstDDRefVal, -1), CExpr(CE), Node(nullptr) {}

ConstDDRef::ConstDDRef(const ConstDDRef &ConstDDRefObj)
    : DDRef(ConstDDRefObj), Node(nullptr) {

  /// Clone the Canon Expression linked to this ConstDDRef
  assert(ConstDDRefObj.CExpr && " Canon Expr for ConstDDRefObj cannot be null");
  CExpr = ConstDDRefObj.CExpr->clone();
}

ConstDDRef *ConstDDRef::clone() const {

  /// Call Copy constructor
  ConstDDRef *NewConstDDRef = new ConstDDRef(*this);

  return NewConstDDRef;
}
