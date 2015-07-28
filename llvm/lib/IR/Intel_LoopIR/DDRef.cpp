//===- DDRef.cpp - Implements the DDRef class -----------------------------===//
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
// This file implements the DDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

std::set<DDRef *> DDRef::Objs;

DDRef::DDRef(unsigned SCID, int SB) : SubClassID(SCID), SymBase(SB) {

  Objs.insert(this);
}

DDRef::DDRef(const DDRef &DDRefObj)
    : SubClassID(DDRefObj.SubClassID), SymBase(DDRefObj.SymBase) {

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

Type *DDRef::getType() const {
  const CanonExpr *CE;

  if (auto BRef = dyn_cast<BlobDDRef>(this)) {
    CE = BRef->getCanonExpr();
    assert(CE && "DDRef is empty!");
    return CE->getType();
  } else if (auto RRef = dyn_cast<RegDDRef>(this)) {
    if (RRef->hasGEPInfo()) {
      CE = RRef->getBaseCE();
      assert(CE && "BaseCE is absent in RegDDRef containing GEPInfo!");

      if (RRef->isAddressOf()) {
        return CE->getType();
      } else {
        // load/store DDRef is a dereference of the base type.
        return cast<PointerType>(CE->getType())->getElementType();
      }
    } else {
      CE = RRef->getSingleCanonExpr();
      assert(CE && "DDRef is empty!");
      return CE->getType();
    }
  }

  llvm_unreachable("Unknown DDRef kind!");
}

Type *DDRef::getElementType() const {
  Type *RetTy = getType();
  ArrayType *ArrTy;

  while ((ArrTy = dyn_cast<ArrayType>(RetTy))) {
    RetTy = ArrTy->getElementType();
  }

  return RetTy;
}

void DDRef::print(formatted_raw_ostream &OS, bool Detailed) const {
  OS << "{sb:" << getSymBase() << "}";
}
