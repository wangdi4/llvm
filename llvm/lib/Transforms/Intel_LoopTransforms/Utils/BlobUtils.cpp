//===--- CanonExprUtils.cpp - Implements CanonExprUtils class -------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements CanonExprUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"

using namespace llvm;
using namespace loopopt;

HIRParser *BlobUtils::HIRP(nullptr);

void BlobUtils::collectTempBlobs(unsigned BlobIndex,
                                 SmallVectorImpl<unsigned> &TempBlobIndices) {
  SmallVector<BlobTy, 8> TempBlobs;

  collectTempBlobs(getBlob(BlobIndex), TempBlobs);
  mapBlobsToIndices(TempBlobs, TempBlobIndices);
}

