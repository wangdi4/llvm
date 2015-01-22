//===-------- DDRefUtils.cpp - Implements DDRefUtils class ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements DDRefUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/ConstDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"

using namespace llvm;
using namespace loopopt;

ConstDDRef* DDRefUtils::createConstDDRef(CanonExpr* CE, HLNode* HNode) {

  return new ConstDDRef(CE, HNode);
}

RegDDRef* DDRefUtils::createRegDDRef(int SB, HLNode* HNode) {
  return new RegDDRef(SB, HNode);
}
  
BlobDDRef* DDRefUtils::createBlobDDRef(int SB, CanonExpr* CE,
  RegDDRef* Parent) {
    
  return new BlobDDRef(SB, CE, Parent);
}

void DDRefUtils::destroy(DDRef* Ref) {
  Ref->destroy();
}

void DDRefUtils::destroyAll() {
  DDRef::destroyAll();
}
