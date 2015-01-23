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

#include "llvm/IR/Intel_LoopIR/ConstDDRef.h"

using namespace llvm;
using namespace llvm::loopopt;


ConstDDRef::ConstDDRef(CanonExpr* CE, HLNode* HNode)
  : DDRef(DDRef::ConstDDRefVal, -1), CExpr(CE), Node(HNode) { }

ConstDDRef* ConstDDRef::clone_impl() const {
  // TODO: placeholder, implement later
  return nullptr;
}

