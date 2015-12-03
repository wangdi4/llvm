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
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

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

Type *DDRef::getTypeImpl(bool IsSrc) const {
  const CanonExpr *CE;

  if (auto BRef = dyn_cast<BlobDDRef>(this)) {
    CE = BRef->getCanonExpr();
    assert(CE && "DDRef is empty!");
    return IsSrc ? CE->getSrcType() : CE->getDestType();
  } else if (auto RRef = dyn_cast<RegDDRef>(this)) {
    if (RRef->hasGEPInfo()) {
      CE = RRef->getBaseCE();
      assert(CE && "BaseCE is absent in RegDDRef containing GEPInfo!");
      assert(isa<PointerType>(CE->getSrcType()) && "Invalid baseCE src type!");
      assert(isa<PointerType>(CE->getDestType()) &&
             "Invalid baseCE dest type!");

      PointerType *BaseTy = IsSrc ? cast<PointerType>(CE->getSrcType())
                                  : cast<PointerType>(CE->getDestType());

      // Get base pointer's contained type.
      // Assuming the base type is [7 x [101 x float]]*, this will give us [7 x
      // [101 x float]].
      Type *RetTy = BaseTy->getElementType();

      unsigned I = 0;
      // Subtract 1 for the pointer dereference.
      unsigned NumDim = RRef->getNumDimensions() - 1;

      // Recurse into the array type(s).
      // Assuming NumDim is 2 and RetTy is [7 x [101 x float]], the following
      // loop will set RetTy as float.
      for (I = 0; I < NumDim; ++I) {
        if (auto ArrTy = dyn_cast<ArrayType>(RetTy)) {
          RetTy = ArrTy->getElementType();
        } else {
          break;
        }
      }

      // 'I' can be less than NumDim for destination type for cases like this-
      // %arrayidx = getelementptr [10 x float], [10 x float]* %p, i64 0, i64 %k
      // %190 = bitcast float* %arrayidx to i32*
      // store i32 %189, i32* %190
      //
      // The DDRef looks like this in HIR-
      // *(i32*)(%ex1)[0][i1]
      //
      // The base canon expr is stored like this-
      // bitcast.[1001 x float]*.i32*(%ex1)
      // The represented cast is imprecise because the actual casting occurs
      // from float* to i32*. The stored information is enough to generate the
      // correct code, though. This setup needs to be rethought if the
      // transformations want to access three different types involved here-
      // [1001 x float]*, float* and i32*. We are currently not storing the
      // intermediate float* type but it can be computed on the fly.
      //
      // TODO: Rethink the setup, if required.
      assert((!IsSrc || (I == NumDim)) && "Malformed DDRef!");

      // For DDRefs representing addresses, we need to return a pointer to
      // RetTy.
      if (RRef->isAddressOf()) {
        return PointerType::get(RetTy, BaseTy->getAddressSpace());
      } else {
        return RetTy;
      }

    } else {
      CE = RRef->getSingleCanonExpr();
      assert(CE && "DDRef is empty!");
      return IsSrc ? CE->getSrcType() : CE->getDestType();
    }
  }

  llvm_unreachable("Unknown DDRef kind!");
}

Type *DDRef::getSrcType() const { return getTypeImpl(true); }

Type *DDRef::getDestType() const { return getTypeImpl(false); }

void DDRef::print(formatted_raw_ostream &OS, bool Detailed) const {
  if (containsUndef() && Detailed) {
    OS << "{undefined} ";
  }
  OS << "{sb:" << getSymbase() << "}";
}

unsigned DDRef::getHLDDNodeLevel() const {
  HLDDNode *DDNode = getHLDDNode();
  assert(DDNode && " DDRef not attached to any node.");

  return DDNode->getHLNodeLevel();
}

bool DDRef::isSelfBlob() const {
  if (auto Ref = dyn_cast<RegDDRef>(this)) {
    return Ref->isScalarRef() && Ref->getSingleCanonExpr()->isSelfBlob();
  } else if (auto Ref = dyn_cast<BlobDDRef>(this)) {
    (void)Ref;
    assert(Ref->getCanonExpr()->isSelfBlob() &&
           "Blob DDRef is not a self blob!");
    return true;
  }
  llvm_unreachable("Unknown DDRef kind!");
}

bool DDRef::containsUndef() const {
  auto UndefCanonPredicate = [](const CanonExpr *CE) {
    return CE->containsUndef();
  };

  if (auto Ref = dyn_cast<RegDDRef>(this)) {
    if (Ref->hasGEPInfo() && UndefCanonPredicate(Ref->getBaseCE())) {
      return true;
    }

    return std::any_of(Ref->canon_begin(), Ref->canon_end(),
                       UndefCanonPredicate) ||
           std::any_of(Ref->stride_begin(), Ref->stride_end(),
                       UndefCanonPredicate);
  } else if (auto Ref = dyn_cast<BlobDDRef>(this)) {
    return UndefCanonPredicate(Ref->getCanonExpr());
  }

  llvm_unreachable("Unknown DDRef kind!");
}

void DDRef::verify() const {
  assert(getSymbase() != 0 && "Symbase should not be zero");
}
