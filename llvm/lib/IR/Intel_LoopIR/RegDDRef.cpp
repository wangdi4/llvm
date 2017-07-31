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
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-regddref"

RegDDRef::RegDDRef(DDRefUtils &DDRU, unsigned SB)
    : DDRef(DDRU, DDRef::RegDDRefVal, SB), GepInfo(nullptr), Node(nullptr) {}

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
      Alignment(Info.Alignment), DimensionOffsets(Info.DimensionOffsets),
      MDNodes(Info.MDNodes), GepDbgLoc(Info.GepDbgLoc),
      MemDbgLoc(Info.MemDbgLoc) {}

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
  return getMetadata(getDDRefUtils().getContext().getMDKindID(Kind));
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
  setMetadata(getDDRefUtils().getContext().getMDKindID(Kind), Node);
}

void RegDDRef::setMetadata(unsigned KindID, MDNode *Node) {
  // TODO: Handle DbgLoc.
  assert(hasGEPInfo() && "Metadata can be attached only to GEP DDRefs");

  MDPairTy MD(KindID, Node);

  auto Beg = GepInfo->MDNodes.begin();
  auto End = GepInfo->MDNodes.end();

  auto It = std::lower_bound(Beg, End, MD, GEPInfo::MDKindCompareLess());

  if ((It != End) && GEPInfo::MDKindCompareEqual()(*It, MD)) {
    // Update Node
    if (Node) {
      It->second = Node;
    } else {
      // Remove Node
      GepInfo->MDNodes.erase(It);
    }
  } else {
    // Not found in MDNodes
    if (Node) {
      GepInfo->MDNodes.insert(It, MD);
    }
  }
}

void RegDDRef::getAAMetadata(AAMDNodes &AANodes) const {
  AANodes.Scope = getMetadata(LLVMContext::MD_alias_scope);
  AANodes.NoAlias = getMetadata(LLVMContext::MD_noalias);
  AANodes.TBAA = getMetadata(LLVMContext::MD_tbaa);
}

void RegDDRef::setAAMetadata(AAMDNodes &AANodes) {
  setMetadata(LLVMContext::MD_alias_scope, AANodes.Scope);
  setMetadata(LLVMContext::MD_noalias, AANodes.NoAlias);
  setMetadata(LLVMContext::MD_tbaa, AANodes.TBAA);
}

unsigned RegDDRef::findMaxTempBlobLevel(
    const SmallVectorImpl<unsigned> &TempBlobIndices) const {
  unsigned DefLevel = 0, MaxLevel = 0;

  for (auto Index : TempBlobIndices) {
    bool Found = findTempBlobLevel(Index, &DefLevel);
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

unsigned RegDDRef::findMaxBlobLevel(unsigned BlobIndex) const {
  SmallVector<unsigned, 8> Indices;

  getBlobUtils().collectTempBlobs(BlobIndex, Indices);

  return findMaxTempBlobLevel(Indices);
}

void RegDDRef::updateCEDefLevel(CanonExpr *CE, unsigned NestingLevel) {
  SmallVector<unsigned, 8> BlobIndices;

  CE->collectTempBlobIndices(BlobIndices);

  auto MaxLevel = findMaxTempBlobLevel(BlobIndices);

  if (getCanonExprUtils().hasNonLinearSemantics(MaxLevel, NestingLevel)) {
    CE->setNonLinear();
  } else {
    CE->setDefinedAtLevel(MaxLevel);
  }
}

void RegDDRef::updateDefLevel(unsigned NestingLevelIfDetached) {

  unsigned Level = getHLDDNode() ? getNodeLevel() : NestingLevelIfDetached;
  assert(getCanonExprUtils().isValidLinearDefLevel(Level) &&
         "Nesting level not set for detached DDRef!");

  // Update attached blob DDRefs' def level first.
  for (auto It = blob_begin(), EndIt = blob_end(); It != EndIt; ++It) {
    auto CE = (*It)->getCanonExpr();

    if (CE->isNonLinear()) {
      continue;
    }

    if (getCanonExprUtils().hasNonLinearSemantics(CE->getDefinedAtLevel(),
                                                  Level)) {
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
#if !INTEL_PRODUCT_RELEASE
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
    getBlobUtils().printScalar(OS, getSymbase());
  } else {
    if (HasGEP) {
      if (isAddressOf()) {
        OS << "&(";
      } else {
        // Only print these for loads/stores.
        if (isVolatile()) {
          OS << "{vol}";
        }

        if (Detailed && getAlignment()) {
          OS << "{al:" << getAlignment() << "}";
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

    unsigned DimNum = getNumDimensions();
    for (auto I = canon_rbegin(), E = canon_rend(); I != E; ++I, --DimNum) {
      if (HasGEP) {
        OS << "[";
      }

      *I ? (*I)->print(OS, Detailed) : (void)(OS << *I);

      if (HasGEP) {
        OS << "]";

        auto Offsets = getTrailingStructOffsets(DimNum);

        if (Offsets) {
          for (auto OffsetVal : *Offsets) {
            OS << "." << OffsetVal;
          }
        }
      }
    }

    if (HasGEP) {
      if (isAddressOf()) {
        OS << ")";
      }

      if (Detailed) {
        getDDRefUtils().printMDNodes(OS, GepInfo->MDNodes);
      }
    }
  }

  DDRef::print(OS, Detailed);
#endif // !INTEL_PRODUCT_RELEASE
}

Type *RegDDRef::getDimensionType(unsigned DimensionNum) const {
  assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");
  assert(hasGEPInfo() && "Call is only meaningful for GEP DDRefs!");

  unsigned NumDims = getNumDimensions();
  Type *RetTy = getBaseCE()->getSrcType();

  for (unsigned I = NumDims; I > DimensionNum; --I) {
    RetTy = (I == NumDims) ? RetTy->getPointerElementType()
                           : RetTy->getArrayElementType();

    if (RetTy->isStructTy()) {
      auto Offsets = getTrailingStructOffsets(I);
      RetTy = DDRefUtils::getOffsetType(RetTy, *Offsets);
    }
  }

  assert((RetTy->isArrayTy() || RetTy->isPointerTy()) &&
         "Dimension type should be either a pointer or an array type!");
  return RetTy;
}

Type *RegDDRef::getTypeImpl(bool IsSrc) const {
  const CanonExpr *CE = nullptr;

  if (hasGEPInfo()) {
    CE = getBaseCE();

    PointerType *BaseSrcTy = cast<PointerType>(CE->getSrcType());
    auto BaseDestTy = CE->getDestType();

    // If BaseCE's dest type is different that the src type, it refers to Ref's
    // destination type.
    if (!IsSrc && (BaseSrcTy != BaseDestTy)) {
      return isAddressOf() ? BaseDestTy
                           : BaseDestTy->getPointerElementType();
    }

    // Extract the type from the first dimension/offsets.
    auto RefTy = getDimensionElementType(1);
    auto Offsets = getTrailingStructOffsets(1);
    if (Offsets) {
      RefTy = DDRefUtils::getOffsetType(RefTy, *Offsets);
    }

    // For DDRefs representing addresses, we need to return a pointer to
    // RefTy.
    if (isAddressOf()) {
      return PointerType::get(RefTy, BaseSrcTy->getAddressSpace());
    } else {
      return RefTy;
    }

  } else {
    CE = getSingleCanonExpr();
    assert(CE && "DDRef is empty!");
    return IsSrc ? CE->getSrcType() : CE->getDestType();
  }
}

bool RegDDRef::accessesStruct() const {
  if (!hasGEPInfo()) {
    return false;
  }

  auto BaseTy = getBaseSrcType()->getPointerElementType();

  while (isa<ArrayType>(BaseTy)) {
    BaseTy = BaseTy->getArrayElementType();
  };

  return BaseTy->isStructTy();
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

bool RegDDRef::isStandAloneIV(bool AllowConversion) const {
  if (isTerminalRef()) {
    return getSingleCanonExpr()->isStandAloneIV(AllowConversion);
  }

  return false;
}

bool RegDDRef::isSelfBlob() const {
  if (!isTerminalRef()) {
    return false;
  }

  auto CE = getSingleCanonExpr();

  if (!CE->isSelfBlob()) {
    return false;
  }

  unsigned SB = getBlobUtils().getTempBlobSymbase(CE->getSingleBlobIndex());

  return (getSymbase() == SB);
}

bool RegDDRef::isUnitaryBlob() const {
  if (!isTerminalRef()) {
    return false;
  }

  return getSingleCanonExpr()->isUnitaryBlob();
}

bool RegDDRef::isUndefSelfBlob() const {
  if (!isSelfBlob()) {
    return false;
  }

  return getSingleCanonExpr()->isUndefSelfBlob();
}

bool RegDDRef::isStandAloneBlob(bool AllowConversion) const {
  if (!isTerminalRef()) {
    return false;
  }

  auto CE = getSingleCanonExpr();

  if (!CE->isStandAloneBlob(AllowConversion)) {
    return false;
  }

  return true;
}

void RegDDRef::replaceSelfBlobIndex(unsigned NewIndex) {
  assert(isSelfBlob() && "DDRef is not a self blob!");
  getSingleCanonExpr()->replaceSingleBlobIndex(NewIndex);
  setSymbase(getBlobUtils().getTempBlobSymbase(NewIndex));
}

void RegDDRef::makeSelfBlob() {
  assert(isLval() && "DDRef is expected to be an lval ref!");
  assert(isTerminalRef() && "DDRef is expected to be a terminal ref!");

  unsigned Index = getBlobUtils().findOrInsertTempBlobIndex(getSymbase());

  auto CE = getSingleCanonExpr();

  CE->clear();

  // In case there is currently a cast.
  CE->setSrcType(CE->getDestType());

  CE->addBlob(Index, 1);
  CE->setNonLinear();

  removeAllBlobDDRefs();
}

CanonExpr *RegDDRef::getStrideAtLevel(unsigned Level) const {
  const CanonExpr *BaseCE = getBaseCE();
  if (!BaseCE || !BaseCE->isInvariantAtLevel(Level)) {
    return nullptr;
  }

  CanonExpr *StrideAtLevel = nullptr;

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    const CanonExpr *DimCE = getDimensionIndex(I);

    // It's hard to compute the stride of non-unit denominator offset, so we
    // conservatively return nullptr.
    if (DimCE->getDenominator() != 1) {
      return nullptr;
    }

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
    if (!DimCE->isLinearAtLevel() || DimCE->getDefinedAtLevel() >= Level) {
      return nullptr;
    }

    // !hasIV(Level) does not guarantee that IV at Level does
    // not affect this access, as it may be hiding behind a blob;
    // Therefore this check alone is not sufficient (hence the check of the
    // DefinedAtLevel above).
    if (!DimCE->hasIV(Level)) {
      continue;
    }

    if (StrideAtLevel) {
      if (!getCanonExprUtils().mergeable(StrideAtLevel, DimCE)) {
        getCanonExprUtils().destroy(StrideAtLevel);
        return nullptr;
      }
    } else { // Creating the StrideAtLevel for the first time
      StrideAtLevel = getCanonExprUtils().createExtCanonExpr(
          DimCE->getSrcType(), DimCE->getDestType(), DimCE->isSExt());
    }

    int64_t Coeff;
    unsigned Index;

    DimCE->getIVCoeff(Level, &Index, &Coeff);

    uint64_t DimStride = getDimensionStride(I);

    if (Index != InvalidBlobIndex) {
      StrideAtLevel->addBlob(Index, Coeff * DimStride);
    } else {
      StrideAtLevel->addConstant(Coeff * DimStride, false);
    }
  }

  // There is zero stride for invariant references.
  if (!StrideAtLevel) {
    return getCanonExprUtils().createCanonExpr(
        Type::getInt1Ty(getDDRefUtils().getContext()), 0, 0);
  }

  // Collect the temp blobs of strideCE to potentially update the
  // DefinedAtLevel property of StrideCE.
  SmallVector<unsigned, 8> BlobIndices;
  StrideAtLevel->collectTempBlobIndices(BlobIndices);

  unsigned MaxLevel = findMaxTempBlobLevel(BlobIndices);
  assert((MaxLevel != NonLinearLevel) && "Invalid level!");

  StrideAtLevel->setDefinedAtLevel(MaxLevel);

  assert(StrideAtLevel->isInvariantAtLevel(Level) && "Invariant Stride!");
  return StrideAtLevel;
}

uint64_t RegDDRef::getDimensionStride(unsigned DimensionNum) const {
  assert(!isTerminalRef() && "Stride info not applicable for scalar refs!");
  assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");

  auto DimElemType = getDimensionElementType(DimensionNum);
  uint64_t Stride = getCanonExprUtils().getTypeSizeInBits(DimElemType) / 8;

  return Stride;
}

void RegDDRef::addBlobDDRef(BlobDDRef *BlobRef) {
  assert(BlobRef && "Blob DDRef is null!");
  assert(!BlobRef->getParentDDRef() &&
         "BlobRef is already attached to a RegDDRef!");

  BlobDDRefs.push_back(BlobRef);
  BlobRef->setParentDDRef(this);
}

void RegDDRef::addBlobDDRef(unsigned Index, unsigned Level) {
  auto BRef = getDDRefUtils().createBlobDDRef(Index, Level);
  addBlobDDRef(BRef);
}

BlobDDRef *RegDDRef::getBlobDDRef(unsigned Index) {

  // Find the blob DDRef with the input Index
  for (auto BDDR : BlobDDRefs) {
    if (BDDR->getBlobIndex() == Index) {
      return BDDR;
    }
  }

  return nullptr;
}

const BlobDDRef *RegDDRef::getBlobDDRef(unsigned Index) const {
  return const_cast<RegDDRef *>(this)->getBlobDDRef(Index);
}

bool RegDDRef::usesTempBlob(unsigned Index, bool *IsSelfBlob) const {

  if (IsSelfBlob) {
    *IsSelfBlob = false;
  }

  if (isSelfBlob()) {
    unsigned SelfBlobIndex = getSingleCanonExpr()->getSingleBlobIndex();

    if (Index != SelfBlobIndex) {
      return false;
    }

    if (IsSelfBlob) {
      *IsSelfBlob = true;
    }

    return true;
  }

  return getBlobDDRef(Index);
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

bool RegDDRef::replaceTempBlob(unsigned OldIndex, unsigned NewIndex) {
  if (!usesTempBlob(OldIndex)) {
    return false;
  }

  if (isSelfBlob()) {
    replaceSelfBlobIndex(NewIndex);
    return true;
  }

  bool Replaced = false;

  if (hasGEPInfo() && getBaseCE()->replaceTempBlob(OldIndex, NewIndex)) {
    Replaced = true;
  }

  for (auto CEIt = canon_begin(), EndIt = canon_end(); CEIt != EndIt; ++CEIt) {
    if ((*CEIt)->replaceTempBlob(OldIndex, NewIndex)) {
      Replaced = true;
    }
  }

  auto BRef = getBlobDDRef(OldIndex);
  assert(Replaced && BRef && "Inconsistent DDRef found!");
  (void)Replaced;

  BRef->replaceBlob(NewIndex);

  return true;
}

void RegDDRef::removeAllBlobDDRefs() {

  while (!BlobDDRefs.empty()) {

    auto BlobI = blob_cbegin();
    removeBlobDDRef(BlobI);
  }
}

void RegDDRef::removeStaleBlobDDRefs(SmallVectorImpl<unsigned> &BlobIndices,
                                     SmallVectorImpl<BlobDDRef *> &StaleBlobs) {

  auto RemovePred = [&](BlobDDRef *BRef) {

    unsigned Index = BRef->getBlobIndex();

    auto BlobIt =
        std::lower_bound(BlobIndices.begin(), BlobIndices.end(), Index);

    // This Blob DDRef is required, continue on to the next one.
    if ((BlobIt != BlobIndices.end()) && (*BlobIt == Index)) {
      // Remove index for the existing blob DDRef.
      BlobIndices.erase(BlobIt);

      return false;
    }

    // Save stale blob's reference for further potential use
    StaleBlobs.push_back(BRef);

    return true;
  };

  BlobDDRefs.erase(
      std::remove_if(BlobDDRefs.begin(), BlobDDRefs.end(), RemovePred),
      BlobDDRefs.end());
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

void RegDDRef::populateTempBlobImpl(SmallVectorImpl<unsigned> &Blobs,
                                    bool GetIndices) const {

  if (isSelfBlob()) {
    Blobs.push_back(GetIndices ? getSingleCanonExpr()->getSingleBlobIndex()
                               : getSymbase());
    return;
  }

  for (auto BlobIt = blob_cbegin(), E = blob_cend(); BlobIt != E; ++BlobIt) {
    Blobs.push_back(GetIndices ? (*BlobIt)->getBlobIndex()
                               : (*BlobIt)->getSymbase());
  }
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
      if (AuxRef->findTempBlobLevel(Index, &DefLevel)) {
        if (getCanonExprUtils().hasNonLinearSemantics(DefLevel, Level)) {
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
  SmallVector<BlobDDRef *, 8> StaleBlobs;

  bool IsLvalAssumed = getHLDDNode() ? isLval() : AssumeLvalIfDetached;

  if (isTerminalRef() && getSingleCanonExpr()->isSelfBlob()) {
    unsigned SB = getBlobUtils().getTempBlobSymbase(
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
  } else if (getSymbase() == ConstantSymbase) {
    assert(!IsLvalAssumed && "Unexpected LVAL RegDDRef");
    setSymbase(getDDRefUtils().getGenericRvalSymbase());
  }

  collectTempBlobIndices(BlobIndices);

  // Remove stale BlobDDRefs.
  removeStaleBlobDDRefs(BlobIndices, StaleBlobs);

  // Add new BlobDDRefs.
  for (auto &I : BlobIndices) {

    BlobDDRef *BRef;
    if (!StaleBlobs.empty()) {
      BRef = StaleBlobs.pop_back_val();
      BRef->replaceBlob(I);
      BRef->setDefinedAtLevel(0);
      // Detaching blob from parent DDRef
      BRef->setParentDDRef(nullptr);
    } else {
      BRef = getDDRefUtils().createBlobDDRef(I, 0);
    }

    addBlobDDRef(BRef);

    // Defined at level is only applicable for instruction blobs. Other types
    // (like globals, function paramaters) are always proper linear.
    if (!BlobUtils::isGuaranteedProperLinear(getBlobUtils().getBlob(I))) {
      NewBlobs.push_back(BRef);
    }
  }
}

bool RegDDRef::findTempBlobLevel(unsigned BlobIndex, unsigned *DefLevel) const {
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

bool RegDDRef::isNonLinear(void) const {
  // Check BaseCE if available
  const CanonExpr *BaseCE = getBaseCE();
  if (BaseCE && BaseCE->isNonLinear()) {
    return true;
  }

  // Check each dimension
  for (auto I = canon_begin(), E = canon_end(); I != E; ++I) {
    CanonExpr *CE = (*I);

    if (CE->isNonLinear()) {
      return true;
    }
  }

  return false;
}

void RegDDRef::replaceIVByConstant(unsigned LoopLevel, int64_t Val) {
  for (auto I = canon_begin(), E = canon_end(); I != E; ++I) {
    CanonExpr *CE = (*I);

    // Replace IV by constant Val and then simplify the CE
    CE->replaceIVByConstant(LoopLevel, Val);
    CE->simplify();
  }
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

    if (isAddressOf()) {
      // During vectorization DestType is set to a vector of pointers
      assert(isa<PointerType>(CE->getDestType()->getScalarType()) &&
             "Invalid BaseCE dest type!");
    } else {
      assert(isa<PointerType>(CE->getDestType()) &&
             "Invalid BaseCE dest type!");
    }
    assert((CE->isStandAloneBlob() || CE->isNull()) &&
           "BaseCE is not a standalone blob!");

    for (auto CEI = canon_begin(), E = canon_end(); CEI != E; ++CEI) {
      assert((*CEI)->getSrcType()->isIntOrIntVectorTy() &&
             "Subscript should be integer type!");
    }
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

  // Verify symbase value if this DDRef is defined
  DDRef::verify();
}

void std::default_delete<RegDDRef>::operator()(RegDDRef *Ref) const {
  Ref->getDDRefUtils().destroy(Ref);
}

void RegDDRef::setTrailingStructOffsets(
    unsigned DimensionNum, const SmallVectorImpl<unsigned> &Offsets) {
  createGEP();

  if (getGEPInfo()->DimensionOffsets.size() < DimensionNum) {
    // Nothing to do as the incoming offsets for this dimension are empty and
    // no offsets are currently set.
    if (Offsets.empty()) {
      return;
    }

    getGEPInfo()->DimensionOffsets.resize(DimensionNum);
  }

  auto &DimOffsets = getGEPInfo()->DimensionOffsets[DimensionNum - 1];

  DimOffsets.clear();
  DimOffsets.append(Offsets.begin(), Offsets.end());
}

const SmallVectorImpl<unsigned> *
RegDDRef::getTrailingStructOffsets(unsigned DimensionNum) const {
  assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");
  assert(hasGEPInfo() && " Offsets are not meaningful for non-GEP DDRefs!");

  if (getGEPInfo()->DimensionOffsets.size() < DimensionNum) {
    return nullptr;
  }

  if (getGEPInfo()->DimensionOffsets[DimensionNum - 1].empty()) {
    return nullptr;
  }

  return &getGEPInfo()->DimensionOffsets[DimensionNum - 1];
}

bool RegDDRef::hasTrailingStructOffsets() const {
  // If the offset vector is empty return false.
  if (getGEPInfo()->DimensionOffsets.empty()) {
    return false;
  }

  // Look for any non-empty offsets in the vector.
  for (auto &Offsets : getGEPInfo()->DimensionOffsets) {
    if (!Offsets.empty()) {
      return true;
    }
  }

  return false;
}

bool RegDDRef::hasIV(unsigned Level) const {

  for (auto CEIt = canon_begin(), E = canon_end(); CEIt != E; ++CEIt) {
    if ((*CEIt)->hasIV(Level)) {
      return true;
    }
  }

  return false;
}

unsigned RegDDRef::getDefinedAtLevel() const {
  unsigned MaxLevel = 0;

  auto BaseCE = getBaseCE();

  if (BaseCE && BaseCE->isNonLinear()) {
    return NonLinearLevel;
  }

  for (auto CEIt = canon_begin(), E = canon_end(); CEIt != E; ++CEIt) {
    auto CE = *CEIt;

    if (CE->isNonLinear()) {
      return NonLinearLevel;
    }

    MaxLevel = std::max(MaxLevel, CE->getDefinedAtLevel());
  }

  return MaxLevel;
}

void RegDDRef::shift(unsigned LoopLevel, int64_t Amount) {
  unsigned Dim = getNumDimensions();

  // Examine every Dimension
  for (unsigned I = 1; I <= Dim; ++I) {
    CanonExpr *CE = getDimensionIndex(I);

    // Shift to create target CE
    CE->shift(LoopLevel, Amount);
  }
}
