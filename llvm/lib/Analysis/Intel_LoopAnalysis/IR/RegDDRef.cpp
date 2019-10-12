//===----- RegDDRef.cpp - Implements the RegDDRef class -------------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLDDNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"

#if INTEL_INCLUDE_DTRANS
#include "Intel_DTrans/Transforms/PaddedPointerPropagation.h"
#endif // INTEL_INCLUDE_DTRANS

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    PrintDetailsRefs("hir-details-refs", cl::ReallyHidden,
                     cl::desc("Print details of RegDDRef dimensions"));

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
  for (auto I = RegDDRefObj.blob_begin(), E = RegDDRefObj.blob_end(); I != E;
       ++I) {
    BlobDDRef *NewBlobDDRef = (*I)->clone();
    addBlobDDRef(NewBlobDDRef);
  }
}

RegDDRef::GEPInfo::GEPInfo()
    : BaseCE(nullptr), BitCastDestTy(nullptr), InBounds(false),
      AddressOf(false), Volatile(false), IsCollapsed(false), Alignment(0) {}

RegDDRef::GEPInfo::GEPInfo(const GEPInfo &Info)
    : BaseCE(Info.BaseCE->clone()), BitCastDestTy(Info.BitCastDestTy),
      InBounds(Info.InBounds), AddressOf(Info.AddressOf),
      Volatile(Info.Volatile), IsCollapsed(Info.IsCollapsed),
      Alignment(Info.Alignment), DimensionOffsets(Info.DimensionOffsets),
      DimTypes(Info.DimTypes), MDNodes(Info.MDNodes), GepDbgLoc(Info.GepDbgLoc),
      MemDbgLoc(Info.MemDbgLoc) {

  for (auto *Lower : Info.LowerBounds) {
    CanonExpr *LowerClone = Lower->clone();
    LowerBounds.push_back(LowerClone);
  }

  for (auto *Stride : Info.Strides) {
    CanonExpr *StrideClone = Stride->clone();
    Strides.push_back(StrideClone);
  }
}

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

void RegDDRef::updateDefLevel(unsigned Level) {
  if (Level == NonLinearLevel) {
    Level = getNodeLevel();
  }

  updateDefLevelInternal(Level);
}

void RegDDRef::updateDefLevelInternal(unsigned NewLevel) {
  assert(CanonExprUtils::isValidLinearDefLevel(NewLevel) &&
         "Invalid nesting level.");

  // Update attached blob DDRefs' def level first.
  for (auto It = blob_begin(), EndIt = blob_end(); It != EndIt; ++It) {
    auto CE = (*It)->getSingleCanonExpr();

    if (CE->isNonLinear()) {
      continue;
    }

    if (getCanonExprUtils().hasNonLinearSemantics(CE->getDefinedAtLevel(),
                                                  NewLevel)) {
      (*It)->setNonLinear();
    }
  }

  bool HasGEPInfo = hasGEPInfo();

  if (HasGEPInfo) {
    updateCEDefLevel(getBaseCE(), NewLevel);
  }

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    updateCEDefLevel(getDimensionIndex(I), NewLevel);

    if (HasGEPInfo) {
      updateCEDefLevel(getDimensionLower(I), NewLevel);
      updateCEDefLevel(getDimensionStride(I), NewLevel);
    }
  }
}

void RegDDRef::print(formatted_raw_ostream &OS, bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
  const CanonExpr *CE;
  bool HasGEP = hasGEPInfo();

  // Do not print linear forms for scalar lvals
  // Treat disconnected DDRefs as rvals. isLval() asserts for disconnected
  // DDRefs. Being able to print disconnected DDRefs is useful for debugging.
  if (getHLDDNode() && !HasGEP && !Detailed && isLval() && !isFakeLval()) {
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

      if (getBitCastDestType()) {
        OS << "(" << *getBitCastDestType() << ")";
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

      // Print dimensions details [lb:idx:stride(type:numelements)]

      // Print LB
      if (HasGEP && PrintDetailsRefs) {
        getDimensionLower(DimNum)->print(OS, Detailed);
        OS << ":";
      }

      // Print index
      *I ? (*I)->print(OS, Detailed) : (void)(OS << *I);

      // Print stride
      if (HasGEP && PrintDetailsRefs) {
        OS << ":";
        getDimensionStride(DimNum)->print(OS, Detailed);
      }

      // Print Type and Number of elements in dimensions
      if (HasGEP && PrintDetailsRefs) {
        OS << "(";
        getDimensionType(DimNum)->print(OS, true, false);
        OS << ":" << getNumDimensionElements(DimNum) << ")";
      }

      if (HasGEP) {
        OS << "]";

        auto Offsets = getTrailingStructOffsets(DimNum);
        for (auto OffsetVal : Offsets) {
          OS << "." << OffsetVal;
        }
      }
    }

    if (HasGEP) {
      if (isAddressOf()) {
        OS << ")";
      }

      if (Detailed) {
        if (isInBounds()) {
          OS << " inbounds ";
        }
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

  Type *DimTy = GepInfo->DimTypes[DimensionNum - 1];
  assert((DimTy->isArrayTy() || DimTy->isPointerTy()) &&
         "Dimension type should be either a pointer or an array type!");
  return DimTy;
}

Type *RegDDRef::getTypeImpl(bool IsSrc) const {
  const CanonExpr *CE = nullptr;

  if (hasGEPInfo()) {
    CE = getBaseCE();

    auto *DestTy = getBitCastDestType();

    // Derive ref's destination type using BitCastDestType, if available.
    if (!IsSrc && DestTy) {
      return isAddressOf() ? DestTy : DestTy->getPointerElementType();
    }

    // Extract the type from the first dimension/offsets.
    auto RefTy = getDimensionElementType(1);
    RefTy = DDRefUtils::getOffsetType(RefTy, getTrailingStructOffsets(1));

    // For DDRefs representing addresses, we need to return a pointer to
    // RefTy.
    if (isAddressOf()) {
      return PointerType::get(RefTy,
                              CE->getSrcType()->getPointerAddressSpace());
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
  assert(hasGEPInfo() && "GEP ref expected!");

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    if (isa<StructType>(getDimensionElementType(I))) {
      return true;
    }
  }
  return false;
}

MemoryLocation RegDDRef::getMemoryLocation() const {
  MemoryLocation Loc;

  const CanonExpr *BaseCE = getBaseCE();

  if (BaseCE->isNull()) {
    Loc.Ptr = Constant::getNullValue(BaseCE->getDestType());
  } else {
    auto &BU = getBlobUtils();
    auto Blob = BU.getBlob(getBaseCE()->getSingleBlobIndex());
    Loc.Ptr = BU.getTempOrUndefBlobValue(Blob);
  }

  Loc.Size = MemoryLocation::UnknownSize;

  getAAMetadata(Loc.AATags);

  return Loc;
}

Value *RegDDRef::getTempBaseValue() const {
  assert(hasGEPInfo() && "GEP ref expected!");

  auto BaseCE = getBaseCE();

  // Standalone check allows conversion in the base.
  if (!BaseCE->isStandAloneBlob()) {
    return nullptr;
  }

  auto Blob = getBlobUtils().getBlob(BaseCE->getSingleBlobIndex());

  if (!BlobUtils::isTempBlob(Blob)) {
    return nullptr;
  }

  return BlobUtils::getTempBlobValue(Blob);
}

bool RegDDRef::isLval() const {
  auto HNode = getHLDDNode();

  assert(HNode && "DDRef is not attached to any node!");

  return HNode->isLval(this);
}

bool RegDDRef::isFake() const {
  auto HNode = getHLDDNode();

  assert(HNode && "DDRef is not attached to any node!");

  return HNode->isFake(this);
}

bool RegDDRef::isFakeLval() const {
  auto HNode = getHLDDNode();

  assert(HNode && "DDRef is not attached to any node!");

  return HNode->isFakeLval(this);
}

bool RegDDRef::isFakeRval() const {
  auto HNode = getHLDDNode();

  assert(HNode && "DDRef is not attached to any node!");

  return HNode->isFakeRval(this);
}

bool RegDDRef::isStructurallyInvariantAtLevel(unsigned LoopLevel) const {

  bool HasGEPInfo = hasGEPInfo();

  // Check the Base CE.
  if (HasGEPInfo && !getBaseCE()->isInvariantAtLevel(LoopLevel)) {
    return false;
  }

  // Check canon expr of the ddrefs to see if level exist.
  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {

    // Check if CanonExpr is invariant i.e. IV is not present in any form inside
    // the canon expr.
    if (!getDimensionIndex(I)->isInvariantAtLevel(LoopLevel)) {
      return false;
    }

    if (HasGEPInfo && (!getDimensionLower(I)->isInvariantAtLevel(LoopLevel) ||
                       !getDimensionStride(I)->isInvariantAtLevel(LoopLevel))) {
      return false;
    }
  }

  // Level doesn't exist in any of the canon exprs.
  return true;
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

bool RegDDRef::isNonDecomposable() const {
  if (!isTerminalRef()) {
    return false;
  }

  if (isLval()) {
    return true;
  }

  return getSingleCanonExpr()->isUnitaryBlob();
}

bool RegDDRef::isStandAloneUndefBlob() const {
  if (!isStandAloneBlob()) {
    return false;
  }

  return getSingleCanonExpr()->isStandAloneUndefBlob();
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

void RegDDRef::makeSelfBlob(bool AssumeLvalIfDetached) {
  bool IsLval = getHLDDNode() ? isLval() : AssumeLvalIfDetached;
  (void)IsLval;

  assert(IsLval && "DDRef is expected to be an lval ref!");
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
  assert(hasGEPInfo() && "Stride is only valid for GEP refs!");
  assert(getHLDDNode() && "Cannot compute stride of detached ref!");

  if (getDefinedAtLevel() >= Level) {
    return nullptr;
  }

  auto *Node = getHLDDNode();
  // Ref is invariant at deeper levels.
  if (Level > Node->getNodeLevel()) {
    return getCanonExprUtils().createCanonExpr(
        Type::getInt1Ty(getDDRefUtils().getContext()), 0, 0);
  }

  CanonExpr *StrideAtLevel = nullptr;
  bool FitsIn32Bits = false;

  auto *LoopStride = Node->getParentLoopAtLevel(Level)->getStrideDDRef();
  int64_t LoopStrideVal = 0;
  bool HasConstStride = LoopStride->isIntConstant(&LoopStrideVal);
  (void)HasConstStride;
  assert(HasConstStride && "Constant loop stride expected!");

#if INTEL_INCLUDE_DTRANS
  // IPO's padding transformation and propagation is also responsible to
  // generate RT check on malloc site to check that user doesn't try to
  // allocate more than 4GB of memory. As soon as currently there's no
  // other interface to ask about this RT check for 4GB, we have to assume
  // that if pointer is padded, 4GB check is inserted.
  if (Value *Base = getTempBaseValue())
    FitsIn32Bits = llvm::getPaddingForValue(Base) > 0;
#endif // INTEL_INCLUDE_DTRANS

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    const CanonExpr *DimCE = getDimensionIndex(I);

    int64_t Coeff;
    unsigned Index;

    DimCE->getIVCoeff(Level, &Index, &Coeff);

    if (Coeff == 0) {
      continue;
    }

    // It's hard to compute the stride of non-unit denominator offset, so we
    // conservatively return nullptr.
    if (DimCE->getDenominator() != 1) {
      return nullptr;
    }

    // Bail out on vector types.
    if (!DimCE->getSrcType()->isIntegerTy()) {
      return nullptr;
    }

    if (HLNodeUtils::mayWraparound(DimCE, Level, Node, FitsIn32Bits)) {
      return nullptr;
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

    uint64_t DimStride = getDimensionConstStride(I);

    if (!DimStride) {
      // TODO: extend for non-const strides.
      return nullptr;
    }

    if (Index != InvalidBlobIndex) {
      StrideAtLevel->addBlob(Index, Coeff * DimStride * LoopStrideVal);
    } else {
      StrideAtLevel->addConstant(Coeff * DimStride * LoopStrideVal, false);
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

bool RegDDRef::getConstStrideAtLevel(unsigned Level, int64_t *Stride) const {
  assert(hasGEPInfo() && "Stride is only valid for GEP refs!");
  assert(getHLDDNode() && "Cannot compute stride of detached ref!");

  if (getDefinedAtLevel() >= Level) {
    return false;
  }

  auto *Node = getHLDDNode();
  // Ref is invariant at deeper levels.
  if (Level > Node->getNodeLevel()) {
    if (Stride) {
      *Stride = 0;
    }
    return true;
  }

  int64_t StrideVal = 0;
  bool FitsIn32Bits = false;

  auto *LoopStride = Node->getParentLoopAtLevel(Level)->getStrideDDRef();
  int64_t LoopStrideVal = 0;
  bool LoopHasConstStride = LoopStride->isIntConstant(&LoopStrideVal);
  (void)LoopHasConstStride;
  assert(LoopHasConstStride && "Constant loop stride expected!");

#if INTEL_INCLUDE_DTRANS
  // IPO's padding transformation and propagation is also responsible to
  // generate RT check on malloc site to check that user doesn't try to
  // allocate more than 4GB of memory. As soon as currently there's no
  // other interface to ask about this RT check for 4GB, we have to assume
  // that if pointer is padded, 4GB check is inserted.
  if (Value *Base = getTempBaseValue())
    FitsIn32Bits = llvm::getPaddingForValue(Base) > 0;
#endif // INTEL_INCLUDE_DTRANS

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    const CanonExpr *DimCE = getDimensionIndex(I);

    int64_t Coeff;
    unsigned Index;

    DimCE->getIVCoeff(Level, &Index, &Coeff);

    if (Coeff == 0) {
      continue;
    }

    if ((Index != InvalidBlobIndex) || (DimCE->getDenominator() != 1)) {
      return false;
    }

    // Bail out on vector types.
    if (!DimCE->getSrcType()->isIntegerTy()) {
      return false;
    }

    if (HLNodeUtils::mayWraparound(DimCE, Level, Node, FitsIn32Bits)) {
      return false;
    }

    auto DimStride = getDimensionConstStride(I);

    if (!DimStride) {
      return false;
    }

    StrideVal += (Coeff * DimStride * LoopStrideVal);
  }

  if (Stride) {
    *Stride = StrideVal;
  }

  return true;
}

bool RegDDRef::isUnitStride(unsigned Level, bool &IsNegStride) const {
  assert(hasGEPInfo() && "Stride is only valid for GEP refs!");

  int64_t Stride;

  if (!getConstStrideAtLevel(Level, &Stride)) {
    return false;
  }

  uint64_t Size = getDestTypeSizeInBytes();
  IsNegStride = Stride < 0;

  return (Size == (uint64_t)std::abs(Stride));
}

int64_t RegDDRef::getDimensionConstStride(unsigned DimensionNum) const {
  assert(!isTerminalRef() && "Stride info not applicable for scalar refs!");
  assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");

  const CanonExpr *StrideCE = getDimensionStride(DimensionNum);
  int64_t ConstStride;
  if (StrideCE->isIntConstant(&ConstStride)) {
    return ConstStride;
  }

  return 0;
}

int64_t RegDDRef::getDimensionConstLower(unsigned DimensionNum) const {
  assert(!isTerminalRef() && "Stride info not applicable for scalar refs!");
  assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");

  const CanonExpr *LB = getDimensionLower(DimensionNum);
  int64_t ConstLB;
  if (LB->isIntConstant(&ConstLB)) {
    return ConstLB;
  }

  return 0;
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

bool RegDDRef::usesTempBlob(unsigned Index, bool *IsSelfBlob,
                            bool AssumeLvalIfDetached) const {

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

  if (getBlobDDRef(Index)) {
    return true;
  }

  bool IsLval = getHLDDNode() ? isLval() : AssumeLvalIfDetached;
  // Index may refer to the symbase of a terminal lval ref having a canonical
  // expression. For example, in the instruction below Index may refer to 't'
  // which has a canonical form of i1 + 1. Although, the temp does not occur
  // anywhere as a blob, ref is still using it via symbase. t = i1 + 1
  if (IsLval && isTerminalRef() &&
      (getSymbase() == getBlobUtils().getTempBlobSymbase(Index))) {
    return true;
  }

  return false;
}

RegDDRef::blob_iterator
RegDDRef::getNonConstBlobIterator(const_blob_iterator CBlobI) {
  blob_iterator BlobI(blob_begin());
  std::advance(BlobI, std::distance<const_blob_iterator>(BlobI, CBlobI));
  return BlobI;
}

BlobDDRef *RegDDRef::removeBlobDDRef(const_blob_iterator CBlobI) {
  assert((CBlobI != blob_end()) && "End iterator is not a valid input!");

  auto BlobI = getNonConstBlobIterator(CBlobI);
  auto BRef = *BlobI;

  BlobDDRefs.erase(BlobI);

  BRef->setParentDDRef(nullptr);
  return BRef;
}

bool RegDDRef::replaceTempBlob(unsigned OldIndex, unsigned NewIndex,
                               bool AssumeLvalIfDetached) {
  if (!usesTempBlob(OldIndex)) {
    return false;
  }

  if (isSelfBlob()) {
    replaceSelfBlobIndex(NewIndex);
    return true;
  }

  bool IsLval = getHLDDNode() ? isLval() : AssumeLvalIfDetached;
  // Need to handle lval terminal refs having a canonical expr form as a special
  // case. See comment inside usesTempBlob().
  if (IsLval && isTerminalRef() &&
      (getSymbase() == getBlobUtils().getTempBlobSymbase(OldIndex))) {
    setSymbase(getBlobUtils().getTempBlobSymbase(NewIndex));
    return true;
  }

  bool Replaced = false;
  bool HasGEPInfo = hasGEPInfo();

  if (HasGEPInfo && getBaseCE()->replaceTempBlob(OldIndex, NewIndex)) {
    Replaced = true;
  }

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    if (getDimensionIndex(I)->replaceTempBlob(OldIndex, NewIndex)) {
      Replaced = true;
    }

    if (HasGEPInfo) {
      if (getDimensionLower(I)->replaceTempBlob(OldIndex, NewIndex)) {
        Replaced = true;
      }

      if (getDimensionStride(I)->replaceTempBlob(OldIndex, NewIndex)) {
        Replaced = true;
      }
    }
  }

  auto BRef = getBlobDDRef(OldIndex);
  assert(Replaced && BRef && "Inconsistent DDRef found!");
  (void)Replaced;

  BRef->replaceBlob(NewIndex);

  return true;
}

bool RegDDRef::replaceTempBlobs(
    SmallVectorImpl<std::pair<unsigned, unsigned>> &BlobMap,
    bool AssumeLvalIfDetached) {
  bool Res = false;

  for (auto &Pair : BlobMap) {
    Res = replaceTempBlob(Pair.first, Pair.second, AssumeLvalIfDetached) || Res;
  }

  return Res;
}

void RegDDRef::removeAllBlobDDRefs() {

  while (!BlobDDRefs.empty()) {

    auto BlobI = blob_begin();
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

  bool HasGEPInfo = hasGEPInfo();

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    getDimensionIndex(I)->collectTempBlobIndices(Indices, false);

    if (HasGEPInfo) {
      getDimensionLower(I)->collectTempBlobIndices(Indices, false);
      getDimensionStride(I)->collectTempBlobIndices(Indices, false);
    }
  }

  if (HasGEPInfo) {
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

  for (auto BlobIt = blob_begin(), E = blob_end(); BlobIt != E; ++BlobIt) {
    Blobs.push_back(GetIndices ? (*BlobIt)->getBlobIndex()
                               : (*BlobIt)->getSymbase());
  }
}

void RegDDRef::makeConsistent(ArrayRef<const RegDDRef *> AuxRefs,
                              unsigned NewLevel) {
  SmallVector<BlobDDRef *, 8> NewBlobs;

  updateBlobDDRefs(NewBlobs);

  if (NewLevel == NonLinearLevel) {
    NewLevel = getNodeLevel();
  }

  assert(CanonExprUtils::isValidLinearDefLevel(NewLevel) &&
         "Invalid nesting level.");

  // Set def level for the new blobs.
  for (auto &BRef : NewBlobs) {
    unsigned DefLevel = 0;
    bool Found = false;
    unsigned Index = BRef->getBlobIndex();

    assert(!AuxRefs.empty() && "Missing auxiliary refs!");

    for (auto *AuxRef : AuxRefs) {
      assert(AuxRef && "Unexpected nullptr ref");

      if (AuxRef->findTempBlobLevel(Index, &DefLevel)) {
        if (getCanonExprUtils().hasNonLinearSemantics(DefLevel, NewLevel)) {
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

  updateDefLevelInternal(NewLevel);
}

void RegDDRef::updateBlobDDRefs(SmallVectorImpl<BlobDDRef *> &NewBlobs,
                                bool AssumeLvalIfDetached) {
  SmallVector<unsigned, 8> BlobIndices;
  SmallVector<BlobDDRef *, 8> StaleBlobs;

  bool IsLvalAssumed = getHLDDNode() ? isLval() : AssumeLvalIfDetached;

  if (isTerminalRef()) {
    if (getSingleCanonExpr()->isSelfBlob()) {
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

    } else if (!IsLvalAssumed) {
      setSymbase(GenericRvalSymbase);
    }
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
    // (like globals, function parameters) are always proper linear.
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

  for (auto I = blob_begin(), E = blob_end(); I != E; ++I) {
    Index = (*I)->getBlobIndex();

    if (Index == BlobIndex) {
      auto CE = (*I)->getSingleCanonExpr();
      *DefLevel = CE->isNonLinear() ? NonLinearLevel : CE->getDefinedAtLevel();
      return true;
    }
  }

  return false;
}

void RegDDRef::checkDefAtLevelConsistency(
    const CanonExpr *CE, SmallVectorImpl<unsigned> &TempBlobIndices) const {
  SmallVector<unsigned, 8> CEBlobIndices;

  CE->collectTempBlobIndices(CEBlobIndices, false);

  unsigned MaxLevel = findMaxTempBlobLevel(CEBlobIndices);
  (void)MaxLevel;

  assert(MaxLevel == CE->getDefinedAtLevel() &&
         "DefAtLevel of CE is inconsistent!");
  TempBlobIndices.append(CEBlobIndices.begin(), CEBlobIndices.end());
}

void RegDDRef::checkBlobAndDefAtLevelConsistency() const {
  SmallVector<unsigned, 8> BlobIndices;

  bool HasGEPInfo = hasGEPInfo();

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    checkDefAtLevelConsistency(getDimensionIndex(I), BlobIndices);

    if (HasGEPInfo) {
      checkDefAtLevelConsistency(getDimensionLower(I), BlobIndices);
      checkDefAtLevelConsistency(getDimensionStride(I), BlobIndices);
    }
  }

  if (HasGEPInfo) {
    checkDefAtLevelConsistency(getBaseCE(), BlobIndices);
  }

  std::sort(BlobIndices.begin(), BlobIndices.end());

  // Look for stale blob DDRefs.
  for (auto I = blob_begin(), E = blob_end(); I != E; ++I) {
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

  bool HasGEPInfo = hasGEPInfo();

  if (HasGEPInfo && UndefCanonPredicate(getBaseCE())) {
    return true;
  }

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    if (UndefCanonPredicate(getDimensionIndex(I))) {
      return true;
    }

    if (HasGEPInfo && (UndefCanonPredicate(getDimensionLower(I)) ||
                       UndefCanonPredicate(getDimensionStride(I)))) {
      return true;
    }
  }

  return false;
}

bool RegDDRef::isNonLinear(void) const {

  bool HasGEPInfo = hasGEPInfo();

  if (HasGEPInfo && getBaseCE()->isNonLinear()) {
    return true;
  }

  // Check each dimension
  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    if (getDimensionIndex(I)->isNonLinear()) {
      return true;
    }

    if (HasGEPInfo && (getDimensionLower(I)->isNonLinear() ||
                       getDimensionStride(I)->isNonLinear())) {
      return true;
    }
  }

  return false;
}

void RegDDRef::replaceIVByConstant(unsigned LoopLevel, int64_t Val) {
  auto *Node = getHLDDNode();
  bool IsLoopBound = Node && isa<HLLoop>(Node);

  for (auto I = canon_begin(), E = canon_end(); I != E; ++I) {
    CanonExpr *CE = (*I);

    // Replace IV by constant Val and then simplify the CE
    CE->replaceIVByConstant(LoopLevel, Val);

    bool IsNonNegative = IsLoopBound;

    // Check for non-unit denominator to skip calling uitlity for compile time
    // savings.
    if (!IsLoopBound && (CE->getDenominator() != 1)) {
      // Utility may assert for non-attached nodes.
      IsNonNegative =
          (Node->isAttached() && HLNodeUtils::isKnownNonNegative(CE, Node));
    }

    CE->simplify(true, IsNonNegative);
  }
}

void RegDDRef::verify() const {
  bool IsConst = isConstant();

  assert(getNumDimensions() > 0 &&
         "RegDDRef should contain at least one CanonExpr!");

  auto NodeLevel = getNodeLevel();

  bool HasGEPInfo = hasGEPInfo();
  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    auto *IndexCE = getDimensionIndex(I);

    IndexCE->verify(NodeLevel);

    if (HasGEPInfo) {
      assert(IndexCE->getSrcType()->isIntOrIntVectorTy() &&
             "Subscript should be integer type!");

      auto *LowerCE = getDimensionLower(I);
      auto *StrideCE = getDimensionStride(I);

      LowerCE->verify(NodeLevel);
      StrideCE->verify(NodeLevel);

      assert(!LowerCE->hasIV() && "Dimension lower not expected to have IV!");
      assert(!StrideCE->hasIV() && "Dimension stride not expected to have IV!");

      assert(LowerCE->getSrcType()->isIntegerTy() &&
             "Lower is not integer type!");
      assert(StrideCE->getSrcType()->isIntegerTy() &&
             "Stride is not integer type!");
    }
  }

  if (HasGEPInfo) {
    auto CE = getBaseCE();
    (void)CE;
    assert(CE && "BaseCE is absent in RegDDRef containing GEPInfo!");
    assert(isa<PointerType>(CE->getSrcType()->getScalarType()) &&
           "Invalid BaseCE src type!");

    if (isAddressOf()) {
      // During vectorization DestType is set to a vector of pointers
      assert(isa<PointerType>(CE->getDestType()->getScalarType()) &&
             "Invalid BaseCE dest type!");
    } else {
      assert(isa<PointerType>(CE->getDestType()->getScalarType()) &&
             "Invalid BaseCE dest type!");
    }
    assert((CE->isSelfBlob() || CE->isStandAloneUndefBlob() || CE->isNull()) &&
           "BaseCE is not a self blob!");

  } else if (isLval()) {
    assert(getSymbase() > GenericRvalSymbase &&
           "Invalid symbase for lval terminal ref!");

  } else {
    auto *CE = getSingleCanonExpr();

    if (CE->isSelfBlob()) {
      auto &BU = getBlobUtils();
      assert(BU.getTempBlobSymbase(CE->getSingleBlobIndex()) == getSymbase() &&
             "Inconsistent symbase for self blob ref!");
      (void)BU;
    } else if (isConstant()) {
      assert(getSymbase() == ConstantSymbase &&
             "Constant symbase for ref expected!");
    } else {
      assert(getSymbase() == GenericRvalSymbase &&
             "Generic rval symbase for ref expected!");
    }
  }

  for (auto I = blob_begin(), E = blob_end(); I != E; ++I) {
    (*I)->verify();
    assert((*I)->getParentDDRef() == this &&
           "Child blob DDRefs should have this RegDDRef as a parent!");
  }

  if (isSelfBlob() || IsConst) {
    assert((BlobDDRefs.size() == 0) &&
           "Self-blobs couldn't contain any BlobDDRefs!");
  } else {
    checkBlobAndDefAtLevelConsistency();
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

void RegDDRef::addDimensionHighest(CanonExpr *IndexCE,
                                   ArrayRef<unsigned> TrailingOffsets,
                                   CanonExpr *LowerBoundCE, CanonExpr *StrideCE,
                                   Type *DimTy) {
  // addDimension() assumes that the ref IS or WILL become a GEP reference.
  createGEP();

  assert(IndexCE && "IndexCE is null!");
  CanonExprs.push_back(IndexCE);

  if (!TrailingOffsets.empty()) {
    setTrailingStructOffsets(getNumDimensions(), TrailingOffsets);
  }

  if (!LowerBoundCE) {
    // Destination scalar type should be used if vector types are expected in
    // input IR.
    LowerBoundCE = getCanonExprUtils().createCanonExpr(IndexCE->getDestType());
  }

  GepInfo->LowerBounds.push_back(LowerBoundCE);

  assert(StrideCE && "Stride may not be unknown");
  GepInfo->Strides.push_back(StrideCE);

  assert(DimTy && "DimTy may not be unknown");
  assert((DimTy->isArrayTy() || DimTy->isPointerTy()) &&
         "Dimension type should be either array or pointer");
  GepInfo->DimTypes.push_back(DimTy);
}

void RegDDRef::addDimension(CanonExpr *IndexCE,
                            ArrayRef<unsigned> TrailingOffsets,
                            CanonExpr *LowerBoundCE, CanonExpr *StrideCE,
                            Type *DimTy) {
  assert(IndexCE && "IndexCE is null!");
  Type *ScalarIndexCETy = IndexCE->getDestType()->getScalarType();

  // addDimension() assumes that the ref IS or WILL become a GEP reference.
  createGEP();

  if (!LowerBoundCE) {
    LowerBoundCE = getCanonExprUtils().createCanonExpr(ScalarIndexCETy);
  }

  // If no dimension information provided then try to compute it from the BaseCE
  // type.
  if (!StrideCE) {
    assert(!DimTy && "Unexpected DimTy type when StrideCE is defined");

    unsigned DimNum = getNumDimensions();
    Type *ElemTy;
    if (DimNum == 0) {
      DimTy = getBaseCE()->getSrcType();
      ElemTy = DimTy->getPointerElementType();
    } else {
      // Get the lowest dimension type, then apply struct offset if present. The
      // result type will be a new dimension type. Then get the element type, it
      // will be used to compute new stride.
      // Note: for variable stride arrays the StrideCE and DimTy should be
      // provided explicitly.
      DimTy = getDimensionElementType(1);
      DimTy = DDRefUtils::getOffsetType(DimTy, getTrailingStructOffsets(1));
      ElemTy = DimTy->isPointerTy() ? DimTy->getPointerElementType()
                                    : DimTy->getArrayElementType();
    }

    StrideCE = getCanonExprUtils().createCanonExpr(
        ScalarIndexCETy, 0,
        ElemTy->isSized() ? getCanonExprUtils().getTypeSizeInBytes(ElemTy) : 0);
  }

  // Add Index CE
  CanonExprs.insert(CanonExprs.begin(), IndexCE);

  // Add Struct offset
  GepInfo->DimensionOffsets.insert(
      GepInfo->DimensionOffsets.begin(),
      OffsetsTy(TrailingOffsets.begin(), TrailingOffsets.end()));

  // Add Lower bound
  GepInfo->LowerBounds.insert(GepInfo->LowerBounds.begin(), LowerBoundCE);

  // Add Stride
  assert(StrideCE && "Stride may not be unknown");
  GepInfo->Strides.insert(GepInfo->Strides.begin(), StrideCE);

  // Add Dimension type
  assert(DimTy && "DimTy may not be unknown");
  assert((DimTy->isArrayTy() || DimTy->isPointerTy()) &&
         "Dimension type should be either array or pointer");
  GepInfo->DimTypes.insert(GepInfo->DimTypes.begin(), DimTy);
}

void RegDDRef::setTrailingStructOffsets(unsigned DimensionNum,
                                        ArrayRef<unsigned> Offsets) {
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

ArrayRef<unsigned>
RegDDRef::getTrailingStructOffsets(unsigned DimensionNum) const {
  assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");
  assert(hasGEPInfo() && " Offsets are not meaningful for non-GEP DDRefs!");

  if (getGEPInfo()->DimensionOffsets.size() < DimensionNum) {
    return {};
  }

  return getGEPInfo()->DimensionOffsets[DimensionNum - 1];
}

bool RegDDRef::hasNonZeroTrailingStructOffsets(unsigned DimensionNum) const {
  auto Offsets = getTrailingStructOffsets(DimensionNum);

  for (auto Offset : Offsets) {
    if (Offset != 0) {
      return true;
    }
  }

  return false;
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

  bool HasGEPInfo = hasGEPInfo();

  if (HasGEPInfo && getBaseCE()->isNonLinear()) {
    return NonLinearLevel;
  }

  for (unsigned I = 1, NumDims = getNumDimensions(); I <= NumDims; ++I) {
    MaxLevel = std::max(MaxLevel, getDimensionIndex(I)->getDefinedAtLevel());

    if (HasGEPInfo) {
      MaxLevel = std::max(MaxLevel, getDimensionLower(I)->getDefinedAtLevel());
      MaxLevel = std::max(MaxLevel, getDimensionStride(I)->getDefinedAtLevel());
    }

    if (MaxLevel == NonLinearLevel) {
      return NonLinearLevel;
    }
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

void RegDDRef::demoteIVs(unsigned StartLevel) {
  unsigned Dim = getNumDimensions();

  // Examine every Dimension
  for (unsigned I = 1; I <= Dim; ++I) {
    CanonExpr *CE = getDimensionIndex(I);

    // Shift to create target CE
    CE->demoteIVs(StartLevel);
  }
}

unsigned RegDDRef::getBasePtrBlobIndex() const {
  assert(hasGEPInfo() && "Base CE accessed for non-GEP DDRef!");
  return getTempBaseValue() ? getBaseCE()->getSingleBlobIndex()
                            : InvalidBlobIndex;
}

unsigned RegDDRef::getBasePtrSymbase() const {
  assert(hasGEPInfo() && "Base CE accessed for non-GEP DDRef!");

  unsigned Index = getBasePtrBlobIndex();

  if (Index == InvalidBlobIndex) {
    // Base may be undef or null so we can return constant symbase.
    return ConstantSymbase;
  }

  return getBlobUtils().getTempBlobSymbase(Index);
}

void RegDDRef::clear(bool AssumeLvalIfDetached) {
  assert(isTerminalRef() && "Only terminal refs expected!");
  getSingleCanonExpr()->clear();
  removeAllBlobDDRefs();
  bool IsLval = getHLDDNode() ? isLval() : AssumeLvalIfDetached;
  if (!IsLval) {
    setSymbase(ConstantSymbase);
  }
}
