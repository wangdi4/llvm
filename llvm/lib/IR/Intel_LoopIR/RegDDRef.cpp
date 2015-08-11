//===----- RegDDRef.cpp - Implements the RegDDRef class -------------------===//
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
// This file implements the RegDDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

RegDDRef::RegDDRef(int SB)
    : DDRef(DDRef::RegDDRefVal, SB), GepInfo(nullptr), Node(nullptr) {}

RegDDRef::RegDDRef(const RegDDRef &RegDDRefObj)
    : DDRef(RegDDRefObj), GepInfo(nullptr), Node(nullptr) {

  /// Copy base canon expr
  if (auto NewCE = RegDDRefObj.getBaseCE()) {
    setBaseCE(NewCE->clone());
  }

  /// Copy inbounds attribute
  if (RegDDRefObj.isInBounds()) {
    setInBounds(true);
  }

  /// Copy AddressOf flag.
  if (RegDDRefObj.isAddressOf()) {
    setAddressOf(true);
  }

  /// Loop over CanonExprs
  for (auto I = RegDDRefObj.canon_begin(), E = RegDDRefObj.canon_end(); I != E;
       ++I) {
    CanonExpr *NewCE = (*I)->clone();
    CanonExprs.push_back(NewCE);
  }

  /// Loop over Strides
  if (RegDDRefObj.hasGEPInfo()) {
    for (auto I = RegDDRefObj.stride_begin(), E = RegDDRefObj.stride_end();
         I != E; ++I) {
      CanonExpr *NewCE = (*I)->clone();
      getStrides().push_back(NewCE);
    }
  }

  /// Loop over BlobDDRefs
  for (auto I = RegDDRefObj.blob_cbegin(), E = RegDDRefObj.blob_cend(); I != E;
       ++I) {

    BlobDDRef *NewBlobDDRef = (*I)->clone();
    /// TODO: Check if push_back call sets the parent DDRef appropriately
    /// NewBlobDDRef->setParentDDRef(this);
    BlobDDRefs.push_back(NewBlobDDRef);
  }
}

RegDDRef::GEPInfo::GEPInfo()
    : BaseCE(nullptr), InBounds(false), AddressOf(false) {}
RegDDRef::GEPInfo::~GEPInfo() {}

RegDDRef *RegDDRef::clone() const {

  /// Call Copy constructor
  RegDDRef *NewRegDDRef = new RegDDRef(*this);

  return NewRegDDRef;
}

void RegDDRef::print(formatted_raw_ostream &OS, bool Detailed) const {
  const CanonExpr *CE;
  bool HasGEP = hasGEPInfo();

  bool printSymbase = Detailed && !isConstant();

  // Do not print linear forms for scalar lvals
  if (isLval() && !HasGEP && !Detailed) {
    CanonExprUtils::printScalar(OS, getSymBase(), Detailed);
  } else {
    if (HasGEP) {
      if (isAddressOf()) {
        OS << "&(";
      }

      OS << "(";
      CE = getBaseCE();
      CE ? CE->print(OS, Detailed) : (void)(OS << CE);
      OS << ")";
    }

    for (auto I = canon_rbegin(), E = canon_rend(); I != E; I++) {
      if (hasGEPInfo()) {
        OS << "[";
      }

      *I ? (*I)->print(OS, Detailed) : (void)(OS << *I);

      if (HasGEP) {
        OS << "]";
      }
    }

    if (isAddressOf()) {
      OS << ")";
    }
  }

  if (printSymbase) {
    OS << " ";
    DDRef::print(OS, Detailed);
  }
}

Type *RegDDRef::getBaseType() const {
  if (hasGEPInfo()) {
    return getBaseCE()->getType();
  }

  return nullptr;
}

bool RegDDRef::isLval() const {
  auto HNode = getHLDDNode();

  assert(HNode && "DDRef is not attached to any node!");

  if (auto HInst = dyn_cast<HLInst>(HNode)) {
    return (HInst->getLvalDDRef() == this);
  }

  return false;
}

bool RegDDRef::isRval() const { return !isLval(); }

bool RegDDRef::isFake() const {
  auto HNode = getHLDDNode();

  assert(HNode && "DDRef is not attached to any node!");

  if (auto HInst = dyn_cast<HLInst>(HNode)) {
    for (auto I = HInst->fake_ddref_begin(), E = HInst->fake_ddref_end();
         I != E; I++) {

      if ((*I) == this) {
        return true;
      }
    }
  }

  return false;
}

bool RegDDRef::isScalarRef() const {

  // Check GEP and Single CanonExpr
  if (!hasGEPInfo()) {
    assert(isSingleCanonExpr() && "Scalar ref has more than one dimension!");
    return true;
  }

  return false;
}

bool RegDDRef::isIntConstant(int64_t *Val) const {
  if (!isScalarRef())
    return false;

  const CanonExpr *CE = getSingleCanonExpr();
  if (!CE->isConstant())
    return false;

  if (Val)
    *Val = CE->getConstant();

  return true;
}

void RegDDRef::addDimension(CanonExpr *Canon, CanonExpr *Stride) {
  assert(Canon && "Canon is null!");
  assert((CanonExprs.empty() || Stride) && "Stride is null!");

  /// First dimension may not have stride. If it does, create GEP info.
  if (CanonExprs.empty()) {
    if (Stride) {
      if (!hasGEPInfo()) {
        createGEP();
      }
      getStrides().push_back(Stride);
    }
  } else {
    assert(hasGEPInfo() && !getStrides().empty() && "Stride is null for the "
                                                    "first dimension!");
    getStrides().push_back(Stride);
  }

  CanonExprs.push_back(Canon);
}

void RegDDRef::removeDimension(unsigned DimensionNum) {
  assert(DimensionNum && "DimensionNum cannot be zero!");
  assert(DimensionNum <= getNumDimensions() && "DimensionNum is out of range!");
  assert((getNumDimensions() > 1) && "Attempt to remove the only dimension!");

  CanonExprs.erase(CanonExprs.begin() + (DimensionNum - 1));
  getStrides().erase(getStrides().begin() + (DimensionNum - 1));
}

void RegDDRef::addBlobDDRef(BlobDDRef *BlobRef) {
  assert(BlobRef && "Blob DDRef is null!");
  assert(!BlobRef->getParentDDRef() &&
         "BlobRef is already attached to a RegDDRef!");

  BlobDDRefs.push_back(BlobRef);
  BlobRef->setParentDDRef(this);
}

RegDDRef::blob_iterator
RegDDRef::getNonConstBlobIterator(const_blob_iterator CBlobI) {
  blob_iterator BlobI(blob_begin());
  std::advance(BlobI, std::distance<const_blob_iterator>(BlobI, CBlobI));
  return BlobI;
}

BlobDDRef *RegDDRef::removeBlobDDRef(const_blob_iterator CBlobI) {
  assert((CBlobI != blob_cend()) && "End iterator is not a valid input!");

  auto BlobI = getNonConstBlobIterator(CBlobI);
  auto BRef = *BlobI;

  BlobDDRefs.erase(BlobI);

  BRef->setParentDDRef(nullptr);
  return BRef;
}

void RegDDRef::updateBlobDDRefs() {
  /* TODO implement when we have blob table
    SmallVector<unsigned, 8> BlobIndices;

    for(auto I = canon_begin(), E = canon_end(); I != E, I++) {
      I->extractBlobIndices(BlobIndices);
    }

    if (hasGEPInfo()) {
      if (auto CE = getBaseCE()) {
        for(auto I = CE->blob_begin(), E = CE->blob_end(); I != E; I++) {

        }
      }
    }
  */
}
