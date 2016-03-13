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

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

std::set<DDRef *> DDRef::Objs;

DDRef::DDRef(unsigned SCID, unsigned SB) : SubClassID(SCID), Symbase(SB) {

  Objs.insert(this);
}

DDRef::DDRef(const DDRef &DDRefObj)
    : SubClassID(DDRefObj.SubClassID), Symbase(DDRefObj.Symbase) {

  Objs.insert(this);
}

void DDRef::destroy() {
  Objs.erase(this);
  delete this;
}

void DDRef::destroyAll() {

  for (auto &I : Objs) {
    delete I;
  }

  Objs.clear();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DDRef::dump(bool Detailed) const {
  formatted_raw_ostream OS(dbgs());
  print(OS, Detailed);
}

void DDRef::dump() const { dump(false); }
#endif

void DDRef::print(formatted_raw_ostream &OS, bool Detailed) const {
  if (Detailed) {
    if (containsUndef()) {
      OS << " {undefined}";
    }

    OS << " {sb:" << getSymbase() << "}";
  }
}

unsigned DDRef::getHLDDNodeLevel() const {
  HLDDNode *DDNode = getHLDDNode();
  assert(DDNode && " DDRef not attached to any node.");

  return DDNode->getHLNodeLevel();
}

void DDRef::verify() const {
  assert(getSymbase() != 0 && "Symbase should not be zero");
}
