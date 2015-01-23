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

#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

using namespace llvm;
using namespace llvm::loopopt;

RegDDRef::RegDDRef(int SB, HLNode* HNode)
  : DDRef(DDRef::RegDDRefVal, SB), Node(HNode) { }

RegDDRef* RegDDRef::clone_impl() const {
  // TODO: placeholder, implement later
  return nullptr;
}

