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
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

RegDDRef::RegDDRef(unsigned SB)
    : DDRef(DDRef::RegDDRefVal, SB), GepInfo(nullptr), Node(nullptr) {}

RegDDRef::RegDDRef(const RegDDRef &RegDDRefObj)
    : DDRef(RegDDRefObj), GepInfo(nullptr), Node(nullptr) {

  // Copy base canon expr
  if (auto NewCE = RegDDRefObj.getBaseCE()) {
    setBaseCE(NewCE->clone());
  }

  // Copy inbounds attribute
  if (RegDDRefObj.isInBounds()) {
    setInBounds(true);
  }

  // Copy AddressOf flag.
  if (RegDDRefObj.isAddressOf()) {
    setAddressOf(true);
  }

  // Loop over CanonExprs
  for (auto I = RegDDRefObj.canon_begin(), E = RegDDRefObj.canon_end(); I != E;
       ++I) {
    CanonExpr *NewCE = (*I)->clone();
    CanonExprs.push_back(NewCE);
  }

  // Loop over Strides
  if (RegDDRefObj.hasGEPInfo()) {
    for (auto I = RegDDRefObj.stride_begin(), E = RegDDRefObj.stride_end();
         I != E; ++I) {
      CanonExpr *NewCE = (*I)->clone();
      getStrides().push_back(NewCE);
    }
  }

  // Loop over BlobDDRefs
  for (auto I = RegDDRefObj.blob_cbegin(), E = RegDDRefObj.blob_cend(); I != E;
       ++I) {
    BlobDDRef *NewBlobDDRef = (*I)->clone();
    addBlobDDRef(NewBlobDDRef);
  }
}

RegDDRef::GEPInfo::GEPInfo()
    : BaseCE(nullptr), InBounds(false), AddressOf(false) {}
RegDDRef::GEPInfo::~GEPInfo() {}

RegDDRef *RegDDRef::clone() const {

  // Call Copy constructor
  RegDDRef *NewRegDDRef = new RegDDRef(*this);

  return NewRegDDRef;
}

void RegDDRef::updateCELevel() {

  unsigned Level = getHLDDNodeLevel();

  // Base CE
  if (hasGEPInfo()) {
    getBaseCE()->updateNonLinear(Level);
  }

  // Loop over CanonExprs
  for (auto I = canon_begin(), E = canon_end(); I != E; ++I) {
    (*I)->updateNonLinear(Level);
  }

  // Loop over Strides
  if (hasGEPInfo()) {
    for (auto I = stride_begin(), E = stride_end(); I != E; ++I) {
      (*I)->updateNonLinear(Level);
    }
  }

  // Loop over BlobDDRefs
  for (auto I = blob_begin(), E = blob_end(); I != E; ++I) {
    (*I)->updateCELevelImpl(Level);
  }
}

void RegDDRef::print(formatted_raw_ostream &OS, bool Detailed) const {
  const CanonExpr *CE;
  bool HasGEP = hasGEPInfo();

  bool PrintBaseCast = false;

  if (HasGEP) {
    PrintBaseCast = !Detailed && (getBaseSrcType() != getBaseDestType());
  }

  // Do not print linear forms for scalar lvals
  // Treat disconnected DDRefs as rvals. isLval() asserts for disconnected
  // DDRefs. Being able to print disconnected DDRefs is useful for debugging.
  if (getHLDDNode() && isLval() && !HasGEP && !Detailed) {
    CanonExprUtils::printScalar(OS, getSymbase());
  } else {
    if (HasGEP) {
      if (isAddressOf()) {
        OS << "&(";
      }

      if (PrintBaseCast) {
        OS << "(" << *getBaseDestType() << ")";
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

  DDRef::print(OS, Detailed);
}

Type *RegDDRef::getBaseTypeImpl(bool IsSrc) const {
  if (hasGEPInfo()) {
    return IsSrc ? getBaseCE()->getSrcType() : getBaseCE()->getDestType();
  }

  return nullptr;
}

Type *RegDDRef::getBaseSrcType() const { return getBaseTypeImpl(true); }

Type *RegDDRef::getBaseDestType() const { return getBaseTypeImpl(false); }

void RegDDRef::setBaseSrcType(Type *SrcTy) {
  assert(hasGEPInfo() && "Base CE accessed for non-GEP DDRef!");
  getBaseCE()->setSrcType(SrcTy);
}

void RegDDRef::setBaseDestType(Type *DestTy) {
  assert(hasGEPInfo() && "Base CE accessed for non-GEP DDRef!");
  getBaseCE()->setDestType(DestTy);
}

bool RegDDRef::isLval() const {
  auto HNode = getHLDDNode();

  assert(HNode && "DDRef is not attached to any node!");

  return HNode->isLval(this);
}

bool RegDDRef::isRval() const {
  auto HNode = getHLDDNode();

  assert(HNode && "DDRef is not attached to any node!");

  return HNode->isRval(this);
}

bool RegDDRef::isFake() const {
  auto HNode = getHLDDNode();

  assert(HNode && "DDRef is not attached to any node!");

  return HNode->isFake(this);
}

bool RegDDRef::isScalarRef() const {
  // Check GEP and Single CanonExpr
  if (!hasGEPInfo()) {
    assert(isSingleCanonExpr() && "Scalar ref has more than one dimension!");
    return true;
  }

  return false;
}

bool RegDDRef::isStructurallyInvariantAtLevel(unsigned LoopLevel) const {
  // Check the Base CE.
  if (hasGEPInfo() && !getBaseCE()->isInvariantAtLevel(LoopLevel)) {
    return false;
  }

  // Check canon expr of the ddrefs to see if level exist.
  for (auto Iter = canon_begin(), End = canon_end(); Iter != End; ++Iter) {

    const CanonExpr *Canon = *Iter;
    // Check if CanonExpr is invariant i.e. IV is not present in any form inside
    // the canon expr.
    if (!Canon->isInvariantAtLevel(LoopLevel)) {
      return false;
    }
  }

  // Level doesn't exist in any of the canon exprs.
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
  assert(isDimensionValid(DimensionNum) && "DimensionNum is out of range!");
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

void RegDDRef::addBlobDDRef(unsigned Index, int Level) {
  auto BRef = DDRefUtils::createBlobDDRef(Index, Level);
  addBlobDDRef(BRef);
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

void RegDDRef::removeAllBlobDDRefs() {

  while (!BlobDDRefs.empty()) {

    auto BlobI = blob_cbegin();
    removeBlobDDRef(BlobI);
  }
}

void RegDDRef::removeStaleBlobDDRefs(SmallVectorImpl<unsigned> &BlobIndices) {

  // Iteratre through blob DDRefs.
  for (auto It = blob_cbegin(); It != blob_cend();) {

    unsigned Index = (*It)->getBlobIndex();

    auto BlobIt =
        std::lower_bound(BlobIndices.begin(), BlobIndices.end(), Index);

    // This Blob DDRef is required, continue on to the next one.
    if ((BlobIt != BlobIndices.end()) && (*BlobIt == Index)) {
      // Remove index for the existing blob DDRef.
      BlobIndices.erase(BlobIt);

      ++It;
      continue;
    }

    bool ResetToBegin = (It == blob_cbegin());
    const_blob_iterator J(It);

    // Save the previous iterator for recovering the next iterator to be
    // processed after erasure. vector erase() invalidates all the iterators at
    // and after the point of erasure.
    if (!ResetToBegin) {
      --It;
    }

    removeBlobDDRef(J);

    // Get the next iterator to be processed.
    if (!ResetToBegin) {
      ++It;
    } else {
      It = blob_cbegin();
    }
  }
}

void RegDDRef::collectTempBlobIndices(
    SmallVectorImpl<unsigned> &Indices) const {

  for (auto I = canon_begin(), E = canon_end(); I != E; ++I) {
    (*I)->collectTempBlobIndices(Indices, false);
  }

  if (hasGEPInfo()) {
    getBaseCE()->collectTempBlobIndices(Indices, false);

    for (auto I = stride_begin(), E = stride_end(); I != E; ++I) {
      (*I)->collectTempBlobIndices(Indices, false);
    }
  }

  // Make the indices unique.
  std::sort(Indices.begin(), Indices.end());
  Indices.erase(std::unique(Indices.begin(), Indices.end()), Indices.end());
}

void RegDDRef::updateBlobDDRefs(SmallVectorImpl<BlobDDRef *> &NewBlobs) {
  SmallVector<unsigned, 8> BlobIndices;

  if (isScalarRef() && getSingleCanonExpr()->isSelfBlob()) {
    unsigned SB = CanonExprUtils::getBlobSymbase(
        getSingleCanonExpr()->getSingleBlobIndex());

    // We need to modify the symbase if this DDRef was turned into a self blob
    // as the associated blob DDRef is removed.
    // Here's an illustration of why this is required-
    //
    // Before modification DDRef looks like this-
    // <REG> LINEAR i32 2 * %k {sb:4}
    // <BLOB> LINEAR i32 %k {sb:8}
    //
    // After modification it looks like this-
    // <REG> LINEAR i32 %k {sb:4}   <<< symbase is not updated from 4 to 8
    // which is wrong.
    //
    // We should not update symbase of lval DDRefs as lvals represent a store
    // into that symbase. Changing it can affect correctness.
    if (isLval()) {
      if ((getSymbase() == SB)) {
        removeAllBlobDDRefs();
        return;
      }
    } else {
      removeAllBlobDDRefs();
      setSymbase(SB);
      return;
    }
  } else if (isConstant()) {
    removeAllBlobDDRefs();

    if (!isLval()) {
      setSymbase(CONSTANT_SYMBASE);
    }

    return;
  }

  collectTempBlobIndices(BlobIndices);

  // Remove stale BlobDDRefs.
  removeStaleBlobDDRefs(BlobIndices);

  // Add new BlobDDRefs.
  for (auto &I : BlobIndices) {
    auto BRef = DDRefUtils::createBlobDDRef(I, 0);

    addBlobDDRef(BRef);

    // Defined at level is only applicable for instruction blobs. Other types
    // (like globals, function paramaters) are always proper linear.
    if (!CanonExprUtils::isGuaranteedProperLinear(CanonExprUtils::getBlob(I))) {
      NewBlobs.push_back(BRef);
    }
  }
}

bool RegDDRef::findBlobLevel(unsigned BlobIndex, int *DefLevel) const {
  assert(DefLevel && "DefLevel ptr should not be null!");

  unsigned Index = 0;

  if (isScalarRef() && getSingleCanonExpr()->isSelfBlob()) {
    auto CE = getSingleCanonExpr();
    Index = CE->getSingleBlobIndex();

    if (Index == BlobIndex) {
      *DefLevel = CE->isNonLinear() ? -1 : CE->getDefinedAtLevel();
      return true;
    }

    return false;
  }

  for (auto I = blob_cbegin(), E = blob_cend(); I != E; ++I) {
    Index = (*I)->getBlobIndex();

    if (Index == BlobIndex) {
      auto CE = (*I)->getCanonExpr();
      *DefLevel = CE->isNonLinear() ? -1 : CE->getDefinedAtLevel();
      return true;
    }
  }

  return false;
}

void RegDDRef::checkBlobDDRefsConsistency() const {
  SmallVector<unsigned, 8> BlobIndices;

  collectTempBlobIndices(BlobIndices);

  // Check that the DDRef constains a blob DDRef for each contained temp blob.
  for (auto &BI : BlobIndices) {
    bool BlobFound = false;

    for (auto I = blob_cbegin(), E = blob_cend(); I != E; ++I) {
      if (BI == (*I)->getBlobIndex()) {
        BlobFound = true;
        break;
      }
    }

    assert(BlobFound && "Temp blob not found in blob DDRefs!");
  }

  // Look for stale blob DDRefs.
  for (auto I = blob_cbegin(), E = blob_cend(); I != E; ++I) {
    unsigned Index = (*I)->getBlobIndex();

    auto It = std::lower_bound(BlobIndices.begin(), BlobIndices.end(), Index);

    assert(((It != BlobIndices.end()) && (*It == Index)) &&
           "Stale blob DDRef found!");
  }
}

void RegDDRef::verify() const {
  bool IsConst = isConstant();

  assert(getNumDimensions() > 0 &&
         "RegDDRef should contain at least one CanonExpr!");

  for (auto I = canon_begin(), E = canon_end(); I != E; ++I) {
    (*I)->verify();
  }

  for (auto I = blob_cbegin(), E = blob_cend(); I != E; ++I) {
    (*I)->verify();
    assert((*I)->getParentDDRef() == this &&
           "Child blob DDRefs should have this RegDDRef as a parent!");
  }

  if (isSelfBlob() || IsConst) {
    assert((BlobDDRefs.size() == 0) &&
           "Self-blobs couldn't contain any BlobDDRefs!");
  } else {
    checkBlobDDRefsConsistency();
  }

  if (!IsConst || isLval()) {
    assert((getSymbase() > CONSTANT_SYMBASE) && "DDRef has invalid symbase!");

  } else {
    assert((getSymbase() == CONSTANT_SYMBASE) &&
           "Constant DDRef's symbase is incorrect!");
  }

  assert((!hasGEPInfo() || getBaseCE() != nullptr) &&
         "GEP DDRefs should have a base canon expression!");
  assert((!hasGEPInfo() || getNumDimensions() == getStrides().size()) &&
         "Stride should be present for every dimension!");

  // Verify symbase value if this DDRef is defined
  DDRef::verify();
}
