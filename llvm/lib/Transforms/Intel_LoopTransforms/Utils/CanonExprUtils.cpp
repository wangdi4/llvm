//===--- CanonExprUtils.cpp - Implements CanonExprUtils class ----- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements CanonExprUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

using namespace llvm;
using namespace loopopt;

CanonExpr *CanonExprUtils::createCanonExpr(Type *Typ, int Level, int64_t Const,
                                           int64_t Denom) {

  return new CanonExpr(Typ, Level, Const, Denom);
}

void CanonExprUtils::destroy(CanonExpr *CE) { CE->destroy(); }

void CanonExprUtils::destroyAll() {
  CanonExpr::destroyAll();
  CanonExpr::BlobTable.clear();
}

unsigned CanonExprUtils::findOrInsertBlobImpl(CanonExpr::BlobTy Blob,
                                              bool Insert) {
  assert(Blob && "Blob is null!");

  for (auto I = CanonExpr::BlobTable.begin(), E = CanonExpr::BlobTable.end();
       I != E; I++) {
    if (*I == Blob) {
      return (I - CanonExpr::BlobTable.begin() + 1);
    }
  }

  if (Insert) {
    CanonExpr::BlobTable.push_back(Blob);
    return CanonExpr::BlobTable.size();
  }

  return 0;
}

unsigned CanonExprUtils::findBlob(CanonExpr::BlobTy Blob) {
  return findOrInsertBlobImpl(Blob, false);
}

unsigned CanonExprUtils::findOrInsertBlob(CanonExpr::BlobTy Blob) {
  return findOrInsertBlobImpl(Blob, true);
}

CanonExpr::BlobTy CanonExprUtils::getBlob(unsigned BlobIndex) {
  assert((BlobIndex > 0) && (BlobIndex <= CanonExpr::BlobTable.size()) &&
         "BlobIndex is out of range!");
  return CanonExpr::BlobTable[BlobIndex - 1];
}
