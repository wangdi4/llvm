//===----- RegDDRef.cpp - Implements the RegDDRef class -------------------===//
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
// This file implements the RegDDRef class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/Support/Debug.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-regddref"

RegDDRef::RegDDRef(unsigned SB)
    : DDRef(DDRef::RegDDRefVal, SB), GepInfo(nullptr), Node(nullptr) {}

RegDDRef::RegDDRef(const RegDDRef &RegDDRefObj)
    : DDRef(RegDDRefObj), GepInfo(nullptr), Node(nullptr) {

  // Copy GEPInfo.
  if (RegDDRefObj.hasGEPInfo()) {
    GepInfo = new GEPInfo(*(RegDDRefObj.GepInfo));
  }

  // Loop over CanonExprs
  for (auto I = RegDDRefObj.canon_begin(), E = RegDDRefObj.canon_end(); I != E;
       ++I) {
    CanonExpr *NewCE = (*I)->clone();
    CanonExprs.push_back(NewCE);
  }

  // Loop over BlobDDRefs
  for (auto I = RegDDRefObj.blob_cbegin(), E = RegDDRefObj.blob_cend(); I != E;
       ++I) {
    BlobDDRef *NewBlobDDRef = (*I)->clone();
    addBlobDDRef(NewBlobDDRef);
  }
}

RegDDRef::GEPInfo::GEPInfo()
    : BaseCE(nullptr), InBounds(false), AddressOf(false), Volatile(false),
      Alignment(0) {}

RegDDRef::GEPInfo::GEPInfo(const GEPInfo &Info)
    : BaseCE(Info.BaseCE->clone()), InBounds(Info.InBounds),
      AddressOf(Info.AddressOf), Volatile(Info.Volatile),
      Alignment(Info.Alignment), MDNodes(Info.MDNodes) {}

RegDDRef::GEPInfo::~GEPInfo() {}

RegDDRef *RegDDRef::clone() const {

  // Call Copy constructor
  RegDDRef *NewRegDDRef = new RegDDRef(*this);

  return NewRegDDRef;
}

// Used to keep MDNodes sorted by KindID.
struct RegDDRef::GEPInfo::MDKindCompareLess {
  bool operator()(const MDPairTy &MD1, const MDPairTy &MD2) {
    return MD1.first < MD2.first;
  }
};

// Used to keep MDNodes sorted by KindID.
struct RegDDRef::GEPInfo::MDKindCompareEqual {
  bool operator()(const MDPairTy &MD1, const MDPairTy &MD2) {
    return MD1.first == MD2.first;
  }
};

MDNode *RegDDRef::getMetadata(StringRef Kind) const {
  return getMetadata(HIRUtils::getContext().getMDKindID(Kind));
}

MDNode *RegDDRef::getMetadata(unsigned KindID) const {
  if (!hasGEPInfo()) {
    // TODO: Handle DbgLoc.
    return nullptr;
  }

  MDPairTy MD(KindID, nullptr);

  auto Beg = GepInfo->MDNodes.begin();
  auto End = GepInfo->MDNodes.end();

  auto It = std::lower_bound(Beg, End, MD, GEPInfo::MDKindCompareLess());

  if ((It != End) && GEPInfo::MDKindCompareEqual()(*It, MD)) {
    return It->second;
  }

  return nullptr;
}

void RegDDRef::getAllMetadata(MDNodesTy &MDs) const {
  // TODO: Include DbgLoc.
  getAllMetadataOtherThanDebugLoc(MDs);
}

void RegDDRef::getAllMetadataOtherThanDebugLoc(MDNodesTy &MDs) const {
  MDs.clear();

  if (!hasGEPInfo()) {
    return;
  }

  MDs = GepInfo->MDNodes;
}

void RegDDRef::setMetadata(StringRef Kind, MDNode *Node) {
  setMetadata(HIRUtils::getContext().getMDKindID(Kind), Node);
}

void RegDDRef::setMetadata(unsigned KindID, MDNode *Node) {
  // TODO: Handle DbgLoc.
  createGEP();

  MDPairTy MD(KindID, Node);

  auto Beg = GepInfo->MDNodes.begin();
  auto End = GepInfo->MDNodes.end();

  auto It = std::lower_bound(Beg, End, MD, GEPInfo::MDKindCompareLess());

  if ((It != End) && GEPInfo::MDKindCompareEqual()(*It, MD)) {
    It->second = Node;

  } else {
    GepInfo->MDNodes.insert(It, MD);
  }
}

unsigned RegDDRef::findMaxBlobLevel(
    const SmallVectorImpl<unsigned> &BlobIndices) const {
  unsigned DefLevel = 0, MaxLevel = 0;

  for (auto Index : BlobIndices) {
    bool Found = findBlobLevel(Index, &DefLevel);
    (void)Found;
    assert(Found && "Blob DDRef not found!");

    if (DefLevel == NonLinearLevel) {
      return NonLinearLevel;
    } else if (DefLevel > MaxLevel) {
      MaxLevel = DefLevel;
    }
  }

  return MaxLevel;
}

void RegDDRef::updateCEDefLevel(CanonExpr *CE, unsigned NestingLevel) {
  SmallVector<unsigned, 8> BlobIndices;

  CE->collectTempBlobIndices(BlobIndices);

  auto MaxLevel = findMaxBlobLevel(BlobIndices);

  if (CanonExprUtils::hasNonLinearSemantics(MaxLevel, NestingLevel)) {
    CE->setNonLinear();
  } else {
    CE->setDefinedAtLevel(MaxLevel);
  }
}

void RegDDRef::updateDefLevel(unsigned NestingLevelIfDetached) {

  unsigned Level = getHLDDNode() ? getNodeLevel() : NestingLevelIfDetached;
  assert(CanonExprUtils::isValidLinearDefLevel(Level) &&
         "Nesting level not set for detached DDRef!");

  // Update attached blob DDRefs' def level first.
  for (auto It = blob_begin(), EndIt = blob_end(); It != EndIt; ++It) {
    auto CE = (*It)->getCanonExpr();

    if (CE->isNonLinear()) {
      continue;
    }

    if (CanonExprUtils::hasNonLinearSemantics(CE->getDefinedAtLevel(), Level)) {
      (*It)->setNonLinear();
    }
  }

  // Update base CE.
  if (hasGEPInfo()) {
    updateCEDefLevel(getBaseCE(), Level);
  }

  // Update CanonExprs.
  for (auto I = canon_begin(), E = canon_end(); I != E; ++I) {
    updateCEDefLevel(*I, Level);
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
    BlobUtils::printScalar(OS, getSymbase());
  } else {
    if (HasGEP) {
      if (isAddressOf()) {
        OS << "&(";
      } else {
        // Only print these for loads/stores.
        if (isVolatile()) {
          OS << "{vol}";
        }

        if (auto Alignment = getAlignment()) {
          OS << "{al:" << Alignment << "}";
        }
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

    if (HasGEP) {
      if (isAddressOf()) {
        OS << ")";
      }

      if (Detailed) {
        DDRefUtils::printMDNodes(OS, GepInfo->MDNodes);
      }
    }
  }

  DDRef::print(OS, Detailed);
}

Type *RegDDRef::getTypeImpl(bool IsSrc) const {
  const CanonExpr *CE = nullptr;

  if (hasGEPInfo()) {
    CE = getBaseCE();

    PointerType *BaseTy = IsSrc ? cast<PointerType>(CE->getSrcType())
                                : cast<PointerType>(CE->getDestType());

    // Get base pointer's contained type.
    // Assuming the base type is [7 x [101 x float]]*, this will give us [7 x
    // [101 x float]].
    Type *RetTy = BaseTy->getElementType();

    unsigned I = 0;
    // Subtract 1 for the pointer dereference.
    unsigned NumDim = getNumDimensions() - 1;

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
    if (isAddressOf()) {
      return PointerType::get(RetTy, BaseTy->getAddressSpace());
    } else {
      return RetTy;
    }

  } else {
    CE = getSingleCanonExpr();
    assert(CE && "DDRef is empty!");
    return IsSrc ? CE->getSrcType() : CE->getDestType();
  }
}

Type *RegDDRef::getSrcType() const { return getTypeImpl(true); }

Type *RegDDRef::getDestType() const { return getTypeImpl(false); }

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

bool RegDDRef::isTerminalRef() const {
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
}

bool RegDDRef::isMemRef() const { return hasGEPInfo() && !isAddressOf(); }

bool RegDDRef::isSelfBlob() const {
  if (!isTerminalRef()) {
    return false;
  }

  auto CE = getSingleCanonExpr();

  if (!CE->isSelfBlob()) {
    return false;
  }

  unsigned SB = BlobUtils::getBlobSymbase(CE->getSingleBlobIndex());

  return (getSymbase() == SB);
}

CanonExpr *RegDDRef::getStrideAtLevel(unsigned Level) const {
  const CanonExpr *BaseCE = getBaseCE();
  if (!BaseCE || !BaseCE->isInvariantAtLevel(Level))
    return nullptr;

  CanonExpr *StrideAtLevel = nullptr;

  for (unsigned I = 1; I <= getNumDimensions(); ++I) {
    const CanonExpr *DimCE = getDimensionIndex(I);
    uint64_t DimStride = getDimensionStride(I);

    // We want to guarantee that we return a Stride that is invariant at Level,
    // or otherwise to bail out.
    // IsLinearAtLevel guarantees that DimCE is defined at a lower (outer)
    // level than where this RegDDRef is used. We also require that DimCE
    // does not consist of any elements (blobs) that are defined at a
    // level >= Level.
    // Checking that DimCE->getDefinedAtLevel< Level guarantees that all the
    // blobs of this RegDDRef are invariant in Level, which in turn means that
    // any evolution of this RegDDRef in Level is associated with the IV of
    // Level.
    if (!DimCE->isLinearAtLevel() || DimCE->getDefinedAtLevel() >= Level)
      return nullptr;

    // !hasIV(Level) does not guarantee that IV at Level does
    // not affect this access, as it may be hiding behind a blob;
    // Therefore this check alone is not sufficient (hence the check of the
    // DefinedAtLevel above).
    if (!DimCE->hasIV(Level))
      continue;

    if (StrideAtLevel) {
      if (!CanonExprUtils::mergeable(StrideAtLevel, DimCE)) {
        CanonExprUtils::destroy(StrideAtLevel);
        return nullptr;
      }
    } else { // Creating the StrideAtLevel for the first time
      StrideAtLevel = CanonExprUtils::createExtCanonExpr(
          DimCE->getSrcType(), DimCE->getDestType(), DimCE->isSExt());
    }

    unsigned Index = DimCE->getIVBlobCoeff(Level);
    if (Index != InvalidBlobIndex) {
      StrideAtLevel->addBlob(Index, DimCE->getIVConstCoeff(Level) * DimStride);
    } else {
      StrideAtLevel->addConstant(DimCE->getIVConstCoeff(Level) * DimStride);
    }
  }

  // Collect the temp blobs of strideCE to potentially update the
  // DefinedAtLevel property of StrideCE.
  SmallVector<unsigned, 8> BlobIndices;
  StrideAtLevel->collectTempBlobIndices(BlobIndices);

  unsigned MaxLevel = findMaxBlobLevel(BlobIndices);
  assert((MaxLevel != NonLinearLevel) && "Invalid level!");

  StrideAtLevel->setDefinedAtLevel(MaxLevel);

  assert(StrideAtLevel->isInvariantAtLevel(Level) && "Invariant Stride!");
  return StrideAtLevel;
}

void RegDDRef::addDimension(CanonExpr *IndexCE) {
  assert(IndexCE && "IndexCE is null!");
  CanonExprs.push_back(IndexCE);
}

void RegDDRef::removeDimension(unsigned DimensionNum) {
  assert(isDimensionValid(DimensionNum) && "DimensionNum is out of range!");
  assert((getNumDimensions() > 1) && "Attempt to remove the only dimension!");

  CanonExprs.erase(CanonExprs.begin() + (DimensionNum - 1));
}

uint64_t RegDDRef::getDimensionStride(unsigned DimensionNum) const {
  assert(!isTerminalRef() && "Stride info not applicable for scalar refs!");
  assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");

  SmallVector<uint64_t, 9> Strides;
  Type *BaseTy = getBaseSrcType();

  assert(isa<PointerType>(BaseTy) && "DDRef base type is not a pointer type!");

  BaseTy = cast<PointerType>(BaseTy)->getElementType();

  // Collect number of elements in each dimension.
  for (; ArrayType *ArrType = dyn_cast<ArrayType>(BaseTy);
       BaseTy = ArrType->getElementType()) {
    Strides.push_back(ArrType->getNumElements());
  }

  assert((BaseTy->isIntegerTy() || BaseTy->isFloatingPointTy() ||
          BaseTy->isPointerTy()) &&
         "Unexpected DDRef primary element type!");

  uint64_t ElementSize = CanonExprUtils::getTypeSizeInBits(BaseTy) / 8;

  // If the actual number of dimensions differ from the maximum number of
  // dimensions we need to account for the offset. For example, suppose the base
  // type is [10 x [10 x i32]]* which has a maximum of 3 dimensions, one for
  // pointer and two for the arrays. If the actual number of dimensions is 3,
  // the innermost type is i32 and its stride is 4 bytes. But if the actual
  // number of dimensions is 2, the innermost type is [10 x i32] which has a
  // stride of 40 bytes.
  //
  // Added one to include the pointer dimension.
  unsigned Offset = (Strides.size() + 1) - getNumDimensions();

  // We subtract 1 for the innermost dimension as ElementSize already represents
  // the innermost stride.
  unsigned Count = DimensionNum + Offset - 1;

  // Multiply number of elements in each dimension by the element size.
  // We need to do a reverse traversal from the smallest(innermost) to
  // largest(outermost) dimension.
  for (auto I = Strides.rbegin(); Count > 0; --Count, ++I) {
    ElementSize *= (*I);
  }

  return ElementSize;
}

void RegDDRef::addBlobDDRef(BlobDDRef *BlobRef) {
  assert(BlobRef && "Blob DDRef is null!");
  assert(!BlobRef->getParentDDRef() &&
         "BlobRef is already attached to a RegDDRef!");

  BlobDDRefs.push_back(BlobRef);
  BlobRef->setParentDDRef(this);
}

void RegDDRef::addBlobDDRef(unsigned Index, unsigned Level) {
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
  }

  // Make the indices unique.
  std::sort(Indices.begin(), Indices.end());
  Indices.erase(std::unique(Indices.begin(), Indices.end()), Indices.end());
}

void RegDDRef::makeConsistent(const SmallVectorImpl<const RegDDRef *> *AuxRefs,
                              unsigned NestingLevelIfDetached) {
  SmallVector<BlobDDRef *, 8> NewBlobs;

  updateBlobDDRefs(NewBlobs);

  unsigned Level = getHLDDNode() ? getNodeLevel() : NestingLevelIfDetached;

  // Set def level for the new blobs.
  for (auto &BRef : NewBlobs) {
    unsigned DefLevel = 0;
    bool Found = false;
    unsigned Index = BRef->getBlobIndex();

    assert(AuxRefs && "Missing auxiliary refs!");

    for (auto &AuxRef : (*AuxRefs)) {
      if (AuxRef->findBlobLevel(Index, &DefLevel)) {
        if (CanonExprUtils::hasNonLinearSemantics(DefLevel, Level)) {
          BRef->setNonLinear();
        } else {
          BRef->setDefinedAtLevel(DefLevel);
        }

        Found = true;
        break;
      }
    }

    (void)Found;
    assert(Found && "Blob was not found in any auxiliary DDRef!");
  }

  updateDefLevel(NestingLevelIfDetached);
}

void RegDDRef::updateBlobDDRefs(SmallVectorImpl<BlobDDRef *> &NewBlobs,
                                bool AssumeLvalIfDetached) {
  SmallVector<unsigned, 8> BlobIndices;

  bool IsLvalAssumed = getHLDDNode() ? isLval() : AssumeLvalIfDetached;

  if (isTerminalRef() && getSingleCanonExpr()->isSelfBlob()) {
    unsigned SB =
        BlobUtils::getBlobSymbase(getSingleCanonExpr()->getSingleBlobIndex());

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
    if (IsLvalAssumed) {
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

    if (!IsLvalAssumed) {
      setSymbase(ConstantSymbase);
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
    if (!BlobUtils::isGuaranteedProperLinear(BlobUtils::getBlob(I))) {
      NewBlobs.push_back(BRef);
    }
  }
}

bool RegDDRef::findBlobLevel(unsigned BlobIndex, unsigned *DefLevel) const {
  assert(DefLevel && "DefLevel ptr should not be null!");

  unsigned Index = 0;

  if (isTerminalRef() && getSingleCanonExpr()->isSelfBlob()) {
    auto CE = getSingleCanonExpr();
    Index = CE->getSingleBlobIndex();

    if (Index == BlobIndex) {
      *DefLevel = CE->isNonLinear() ? NonLinearLevel : CE->getDefinedAtLevel();
      return true;
    }

    return false;
  }

  for (auto I = blob_cbegin(), E = blob_cend(); I != E; ++I) {
    Index = (*I)->getBlobIndex();

    if (Index == BlobIndex) {
      auto CE = (*I)->getCanonExpr();
      *DefLevel = CE->isNonLinear() ? NonLinearLevel : CE->getDefinedAtLevel();
      return true;
    }
  }

  return false;
}

void RegDDRef::checkBlobDDRefsConsistency() const {
  SmallVector<unsigned, 8> BlobIndices;

  collectTempBlobIndices(BlobIndices);

  // Check that the DDRef contains a blob DDRef for each contained temp blob.
  for (auto &BI : BlobIndices) {
    bool BlobFound = false;

    for (auto I = blob_cbegin(), E = blob_cend(); I != E; ++I) {
      if (BI == (*I)->getBlobIndex()) {
        BlobFound = true;
        break;
      }
    }

    (void)BlobFound;
    assert(BlobFound && "Temp blob not found in blob DDRefs!");
  }

  // Look for stale blob DDRefs.
  for (auto I = blob_cbegin(), E = blob_cend(); I != E; ++I) {
    unsigned Index = (*I)->getBlobIndex();

    auto It = std::lower_bound(BlobIndices.begin(), BlobIndices.end(), Index);
    (void)It;
    assert(((It != BlobIndices.end()) && (*It == Index)) &&
           "Stale blob DDRef found!");
  }
}

bool RegDDRef::containsUndef() const {
  auto UndefCanonPredicate = [](const CanonExpr *CE) {
    return CE->containsUndef();
  };

  if (hasGEPInfo() && UndefCanonPredicate(getBaseCE())) {
    return true;
  }

  return std::any_of(canon_begin(), canon_end(), UndefCanonPredicate);
}

void RegDDRef::verify() const {
  bool IsConst = isConstant();

  assert(getNumDimensions() > 0 &&
         "RegDDRef should contain at least one CanonExpr!");

  auto NodeLevel = getNodeLevel();
  for (auto I = canon_begin(), E = canon_end(); I != E; ++I) {
    (*I)->verify(NodeLevel);
  }

  if (hasGEPInfo()) {
    auto CE = getBaseCE();
    (void)CE;
    assert(CE && "BaseCE is absent in RegDDRef containing GEPInfo!");
    assert(isa<PointerType>(CE->getSrcType()) && "Invalid BaseCE src type!");
    assert(isa<PointerType>(CE->getDestType()) && "Invalid BaseCE dest type!");
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
    assert((getSymbase() > ConstantSymbase) && "DDRef has invalid symbase!");

  } else {
    assert((getSymbase() == ConstantSymbase) &&
           "Constant DDRef's symbase is incorrect!");
  }

  assert((!hasGEPInfo() || getBaseCE() != nullptr) &&
         "GEP DDRefs should have a base canon expression!");

  // Verify symbase value if this DDRef is defined
  DDRef::verify();
}
