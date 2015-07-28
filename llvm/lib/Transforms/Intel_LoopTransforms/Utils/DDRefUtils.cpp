//===-------- DDRefUtils.cpp - Implements DDRefUtils class ----------------===//
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
// This file implements DDRefUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/SymbaseAssignment.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace loopopt;

SymbaseAssignment *HLUtils::SA(nullptr);

RegDDRef *DDRefUtils::createRegDDRef(int SB) { return new RegDDRef(SB); }

BlobDDRef *DDRefUtils::createBlobDDRef(int SB, const CanonExpr *CE) {
  return new BlobDDRef(SB, CE);
}

void DDRefUtils::destroy(DDRef *Ref) { Ref->destroy(); }

void DDRefUtils::destroyAll() { DDRef::destroyAll(); }

unsigned DDRefUtils::getNewSymBase() {
  return getSymbaseAssignment()->getNewSymBase();
}

RegDDRef *DDRefUtils::createSelfBlobRef(Value *Val) {
  // Create a non-linear self-blob canon expr.
  auto CE = CanonExprUtils::createSelfBlobCanonExpr(Val);

  unsigned Symbase = DDRefUtils::getNewSymBase();

  // Register new lval with HIRParser for printing.
  getHIRParser()->insertHIRLval(Val, Symbase);

  // Create a RegDDRef with the new symbase and canon expr.
  auto Ref = DDRefUtils::createRegDDRef(Symbase);
  Ref->setSingleCanonExpr(CE);

  return Ref;
}
