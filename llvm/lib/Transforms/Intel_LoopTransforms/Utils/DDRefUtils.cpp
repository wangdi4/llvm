//===-------- DDRefUtils.cpp - Implements DDRefUtils class ------*- C++ -*-===//
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

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace loopopt;

RegDDRef *DDRefUtils::createRegDDRef(int SB) { return new RegDDRef(SB); }

BlobDDRef *DDRefUtils::createBlobDDRef(int SB, const CanonExpr *CE) {
  return new BlobDDRef(SB, CE);
}

void DDRefUtils::destroy(DDRef *Ref) { Ref->destroy(); }

void DDRefUtils::destroyAll() { DDRef::destroyAll(); }

void DDRefUtils::printScalarLval(raw_ostream &OS, const RegDDRef *Ref) {
  getHIRParser()->printScalarLval(OS, Ref->getSymBase());
}
