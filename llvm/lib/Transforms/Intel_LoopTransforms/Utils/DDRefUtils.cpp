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

BlobDDRef *DDRefUtils::createBlobDDRef(int SB, const CanonExpr *CE,
                                       RegDDRef *Parent) {

  return new BlobDDRef(SB, CE, Parent);
}

void DDRefUtils::destroy(DDRef *Ref) { Ref->destroy(); }

void DDRefUtils::destroyAll() { DDRef::destroyAll(); }
