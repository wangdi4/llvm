//===- RegDDRef.cpp - Implements the RegDDRef class ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the RegDDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

RegDDRef::RegDDRef(int SB) : DDRef(DDRef::RegDDRefVal, SB), Node(nullptr) {}

RegDDRef::RegDDRef(const RegDDRef &RegDDRefObj)
    : DDRef(RegDDRefObj), Node(nullptr) {

  /// Copy base canon expr
  if (auto NewCE = RegDDRefObj.getBaseCE()) {
    setBaseCE(NewCE->clone());
  }

  /// Copy inbounds attribute
  if (RegDDRefObj.isInBounds()) {
    setInBounds(true);
  }

  /// Loop over CanonExprs
  for (auto I = RegDDRefObj.canon_begin(), E = RegDDRefObj.canon_end(); I != E;
       ++I) {
    CanonExpr *NewCE = (*I)->clone();
    CanonExprs.push_back(NewCE);
  }

  /// Loop over Strides
  for (auto I = RegDDRefObj.stride_begin(), E = RegDDRefObj.stride_end();
       I != E; ++I) {
    CanonExpr *NewCE = (*I)->clone();
    getStrides().push_back(NewCE);
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

RegDDRef::GEPInfo::GEPInfo() : BaseCE(nullptr), InBounds(false) {}
RegDDRef::GEPInfo::~GEPInfo() {}

RegDDRef *RegDDRef::clone() const {

  /// Call Copy constructor
  RegDDRef *NewRegDDRef = new RegDDRef(*this);

  return NewRegDDRef;
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
