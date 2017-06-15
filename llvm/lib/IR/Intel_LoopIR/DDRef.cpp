//===- DDRef.cpp - Implements the DDRef class -----------------------------===//
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
// This file implements the DDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

DDRef::DDRef(DDRefUtils &DDRU, unsigned SCID, unsigned SB)
    : DDRU(DDRU), SubClassID(SCID), Symbase(SB) {
  DDRU.Objs.insert(this);
}

DDRef::DDRef(const DDRef &DDRefObj)
    : DDRU(DDRefObj.DDRU), SubClassID(DDRefObj.SubClassID),
      Symbase(DDRefObj.Symbase) {
  DDRU.Objs.insert(this);
}

CanonExprUtils &DDRef::getCanonExprUtils() const {
  return getDDRefUtils().getCanonExprUtils();
}

BlobUtils &DDRef::getBlobUtils() const {
  return getDDRefUtils().getBlobUtils();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DDRef::dump(bool Detailed) const {
  formatted_raw_ostream OS(dbgs());
  print(OS, Detailed);
}

void DDRef::dump() const { dump(false); }
#endif

void DDRef::print(formatted_raw_ostream &OS, bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
  if (Detailed) {
    if (containsUndef()) {
      OS << " {undefined}";
    }

    OS << " {sb:" << getSymbase() << "}";
  }
#endif // !INTEL_PRODUCT_RELEASE
}

unsigned DDRef::getNodeLevel() const {
  HLDDNode *DDNode = getHLDDNode();
  assert(DDNode && " DDRef not attached to any node.");

  return DDNode->getNodeLevel();
}

void DDRef::verify() const {
  assert(getSymbase() != 0 && "Symbase should not be zero");
}

bool DDRef::isLiveOutOfRegion() const {
  assert((isa<BlobDDRef>(this) || cast<RegDDRef>(this)->isTerminalRef()) &&
         "Invalid DDRef!");
  return getHLDDNode()->isLiveOutOfRegion(Symbase);
}

bool DDRef::isLiveIntoParentLoop() const {
  assert((isa<BlobDDRef>(this) || cast<RegDDRef>(this)->isTerminalRef()) &&
         "Invalid DDRef!");
  return getHLDDNode()->isLiveIntoParentLoop(Symbase);
}

bool DDRef::isLiveOutOfParentLoop() const {
  assert((isa<BlobDDRef>(this) || cast<RegDDRef>(this)->isTerminalRef()) &&
         "Invalid DDRef!");
  return getHLDDNode()->isLiveOutOfParentLoop(Symbase);
}

HLLoop *DDRef::getParentLoop() const { return getHLDDNode()->getParentLoop(); }

HLLoop *DDRef::getLexicalParentLoop() const {
  return getHLDDNode()->getLexicalParentLoop();
}
