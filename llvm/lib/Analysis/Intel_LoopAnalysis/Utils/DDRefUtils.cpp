//===-------- DDRefUtils.cpp - Implements DDRefUtils class ----------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements DDRefUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"

using namespace llvm;
using namespace loopopt;

#define DEBUG_TYPE "ddref-utils"

Function &DDRefUtils::getFunction() const {
  return getHIRParser().getFunction();
}

Module &DDRefUtils::getModule() const { return getHIRParser().getModule(); }

LLVMContext &DDRefUtils::getContext() const {
  return getHIRParser().getContext();
}

const DataLayout &DDRefUtils::getDataLayout() const {
  return getHIRParser().getDataLayout();
}

RegDDRef *DDRefUtils::createRegDDRef(unsigned SB) {
  return new RegDDRef(*this, SB);
}

RegDDRef *DDRefUtils::createScalarRegDDRef(unsigned SB, CanonExpr *CE) {
  assert(CE && " CanonExpr is null.");
  RegDDRef *RegDD = createRegDDRef(SB);
  RegDD->setSingleCanonExpr(CE);
  return RegDD;
}

RegDDRef *DDRefUtils::createGEPRef(Type *BasePtrElementType, unsigned BasePtrBlobIndex, unsigned Level,
                                   unsigned SB, bool IsMemRef,
                                   bool IsInBounds) {
  if (SB == InvalidSymbase) {
    SB = getNewSymbase();
  }

  RegDDRef *Ref = createRegDDRef(SB);
  auto BaseCE =
      getCanonExprUtils().createSelfBlobCanonExpr(BasePtrBlobIndex, Level);

  assert(cast<PointerType>(BaseCE->getDestType()->getScalarType())->isOpaqueOrPointeeTypeMatches(BasePtrElementType)
         && "Incorrect base ptr element type!");

  Ref->setBaseCE(BaseCE);
  Ref->setBasePtrElementType(BasePtrElementType);
  Ref->setInBounds(IsInBounds);
  Ref->addBlobDDRef(BasePtrBlobIndex, Level);

  if (!IsMemRef) {
    Ref->setAddressOf(true);
  }

  return Ref;
}

RegDDRef *DDRefUtils::createMemRef(Type *BasePtrElementType, unsigned BasePtrBlobIndex, unsigned Level,
                                   unsigned SB, bool IsInBounds) {
  return createGEPRef(BasePtrElementType, BasePtrBlobIndex, Level, SB, true, IsInBounds);
}

RegDDRef *DDRefUtils::createAddressOfRef(Type *BasePtrElementType, unsigned BasePtrBlobIndex,
                                         unsigned Level, unsigned SB,
                                         bool IsInBounds) {
  return createGEPRef(BasePtrElementType, BasePtrBlobIndex, Level, SB, false, IsInBounds);
}

RegDDRef *DDRefUtils::createSelfAddressOfRef(Type *BasePtrElementType, unsigned BasePtrBlobIndex,
                                             unsigned Level, unsigned SB) {
  auto *SelfAddrRef =
      createAddressOfRef(BasePtrElementType, BasePtrBlobIndex, Level, SB, true /*IsInBounds*/);
  auto *BlobTy =
      cast<PointerType>(getBlobUtils().getBlob(BasePtrBlobIndex)->getType());
  SelfAddrRef->addDimension(getCanonExprUtils().createCanonExpr(
      getDataLayout().getIndexType(BlobTy)));

  assert(SelfAddrRef->isSelfAddressOf() &&
         "Self address-of ref was not created for blob index.");
  return SelfAddrRef;
}

RegDDRef *DDRefUtils::createConstDDRef(Type *Ty, int64_t Val) {
  RegDDRef *NewRegDD = createRegDDRef(ConstantSymbase);
  CanonExpr *CE = getCanonExprUtils().createCanonExpr(Ty, 0, Val);
  NewRegDD->setSingleCanonExpr(CE);

  return NewRegDD;
}

RegDDRef *DDRefUtils::createConstDDRef(Value *Val) {
  auto *CI = dyn_cast<ConstantInt>(Val);

  // If we are dealing with a constant int that fits in 64 bits, avoid creating
  // a blob for the constant value. The interface which takes the constant value
  // stores it in the Const field of the canon expression.
  if (CI && CI->getBitWidth() <= 64) {
    return createConstDDRef(Val->getType(), CI->getSExtValue());
  }

  RegDDRef *NewRegDD = createRegDDRef(ConstantSymbase);
  // Create a linear self-blob constant canon expr.
  auto CE = getCanonExprUtils().createConstStandAloneBlobCanonExpr(Val);
  NewRegDD->setSingleCanonExpr(CE);

  return NewRegDD;
}

RegDDRef *DDRefUtils::createNullDDRef(Type *Ty) {
  if (Ty->isIntOrPtrTy()) {
    return createConstDDRef(Ty, 0);
  }

  return createConstDDRef(Constant::getNullValue(Ty));
}

RegDDRef *DDRefUtils::createConstOneDDRef(Type *Ty) {
  if (Ty->isIntegerTy()) {
    return createConstDDRef(Ty, 1);
  }

  if (Ty->isFloatingPointTy()) {
    return createConstDDRef(ConstantFP::get(Ty, 1.0));
  }

  llvm_unreachable("Unknown One DDRef type!");
  return nullptr;
}

RegDDRef *DDRefUtils::createUndefDDRef(Type *Ty) {
  Value *UndefVal = UndefValue::get(Ty);

  return createConstDDRef(UndefVal);
}

BlobDDRef *DDRefUtils::createBlobDDRef(unsigned Index, unsigned Level) {
  return new BlobDDRef(*this, Index, Level);
}

void DDRefUtils::destroy(DDRef *Ref) {
  auto Count = Objs.erase(Ref);
  (void)Count;
  assert(Count && "Ref not found in objects!");
  delete Ref;
}

DDRefUtils::~DDRefUtils() {
  for (auto &I : Objs) {
    delete I;
  }

  Objs.clear();
}

RegDDRef *DDRefUtils::createSelfBlobRef(Value *Temp) {
  unsigned Symbase = getNewSymbase();

  // Create a non-linear self-blob canon expr.
  auto CE = getCanonExprUtils().createSelfBlobCanonExpr(Temp, Symbase);

  // Create a RegDDRef with the new symbase and canon expr.
  auto Ref = createRegDDRef(Symbase);
  Ref->setSingleCanonExpr(CE);

  return Ref;
}

unsigned DDRefUtils::getNewSymbase() {
  return getHIRParser().getHIRFramework().getNewSymbase();
}

bool DDRefUtils::areEqualImpl(const BlobDDRef *Ref1, const BlobDDRef *Ref2) {

  assert(Ref1 && Ref2 && "Ref1/Ref2 parameter is null.");

  if ((Ref1->getSymbase() != Ref2->getSymbase())) {
    return false;
  }

  // Additional check. Ideally, symbase match should be equal blobs.
  assert(CanonExprUtils::areEqual(Ref1->getSingleCanonExpr(),
                                  Ref2->getSingleCanonExpr()));

  return true;
}

int64_t DDRefUtils::getOffsetDistance(Type *Ty, const DataLayout &DL,
                                      ArrayRef<unsigned> Offsets) {
  int64_t DistInBytes = 0;

  for (auto OffsetVal : Offsets) {
    assert(Ty->isStructTy() && "StructType expected!");
    auto STy = cast<StructType>(Ty);
    DistInBytes += DL.getStructLayout(STy)->getElementOffset(OffsetVal);
    Ty = STy->getElementType(OffsetVal);
  }

  return DistInBytes;
}

int DDRefUtils::compareOffsets(ArrayRef<unsigned> Offsets1,
                               ArrayRef<unsigned> Offsets2) {
  unsigned MinSize = std::min(Offsets1.size(), Offsets2.size());

  for (unsigned I = 0, E = MinSize; I < E; ++I) {
    if (Offsets1[I] < Offsets2[I]) {
      return -1;
    } else if (Offsets1[I] > Offsets2[I]) {
      return 1;
    }
  }

  if (MinSize < Offsets1.size()) {
    return 1;
  } else if (MinSize < Offsets2.size()) {
    return -1;
  }

  return 0;
}

int DDRefUtils::compareOffsets(const RegDDRef *Ref1, const RegDDRef *Ref2,
                               unsigned DimensionNum) {
  assert(Ref1->hasGEPInfo() && "Ref1 is not a GEP DDRef!");
  assert(Ref2->hasGEPInfo() && "Ref2 is not a GEP DDRef!");
  assert((Ref1->getNumDimensions() >= DimensionNum) &&
         "DimensionNum is invalid for Ref1");
  assert((Ref2->getNumDimensions() >= DimensionNum) &&
         "DimensionNum is invalid for Ref2");
  assert((Ref1->getDimensionType(DimensionNum) ==
          Ref2->getDimensionType(DimensionNum)) &&
         "Invalid offset comparison for refs!");

  auto Offsets1 = Ref1->getTrailingStructOffsets(DimensionNum);
  auto Offsets2 = Ref2->getTrailingStructOffsets(DimensionNum);

  return compareOffsets(Offsets1, Offsets2);
}

bool DDRefUtils::haveEqualOffsets(const RegDDRef *Ref1, const RegDDRef *Ref2) {
  assert(haveEqualBaseAndShape(Ref1, Ref2, true) &&
         "Same base and shape expected!");

  for (unsigned I = Ref1->getNumDimensions(); I > 0; --I) {
    if (compareOffsets(Ref1, Ref2, I)) {
      return false;
    }
  }

  return true;
}

// TODO: merge with areEqualImpl.
bool DDRefUtils::haveEqualBaseAndShape(const RegDDRef *Ref1,
                                       const RegDDRef *Ref2, bool RelaxedMode) {
  assert(Ref1->hasGEPInfo() && Ref2->hasGEPInfo() &&
         "Ref1 and Ref2 should be GEP DDRef");
  auto BaseCE1 = Ref1->getBaseCE();
  auto BaseCE2 = Ref2->getBaseCE();

  if (!CanonExprUtils::areEqual(BaseCE1, BaseCE2, RelaxedMode) ||
      Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return false;
  }

  // Check that dimension lowers and strides are the same.
  for (unsigned DimI = 1, DimE = Ref1->getNumDimensions(); DimI <= DimE;
       ++DimI) {
    if (!CanonExprUtils::areEqual(Ref1->getDimensionLower(DimI),
                                  Ref2->getDimensionLower(DimI), RelaxedMode) ||
        !CanonExprUtils::areEqual(Ref1->getDimensionStride(DimI),
                                  Ref2->getDimensionStride(DimI),
                                  RelaxedMode)) {
      return false;
    }
  }

  return true;
}

bool DDRefUtils::haveConstDimensionDistances(const RegDDRef *Ref1,
                                             const RegDDRef *Ref2,
                                             bool RelaxedMode) {
  // Dealing with GEP refs only
  assert(Ref1->hasGEPInfo() && Ref2->hasGEPInfo() &&
         "Both refs are expected to be memrefs");
  if (Ref1 == Ref2) {
    return true;
  }

  if (!DDRefUtils::haveEqualBaseAndShape(Ref1, Ref2, RelaxedMode)) {
    return false;
  }

  for (unsigned I = Ref1->getNumDimensions(); I > 0; --I) {
    const CanonExpr *Ref1CE = Ref1->getDimensionIndex(I);
    const CanonExpr *Ref2CE = Ref2->getDimensionIndex(I);

    // Do not allow different offsets in outer dimensions.
    if (I != 1 && DDRefUtils::compareOffsets(Ref1, Ref2, I)) {
      return false;
    }

    bool Res =
        CanonExprUtils::getConstDistance(Ref1CE, Ref2CE, nullptr, RelaxedMode);
    if (!Res) {
      return false;
    }
  }

  return true;
}

bool DDRefUtils::areEqualImpl(const RegDDRef *Ref1, const RegDDRef *Ref2,
                              bool RelaxedMode, bool IgnoreAddressOf) {

  // Match the symbase. Type checking is done inside the CEUtils.
  if ((Ref1->getSymbase() != Ref2->getSymbase())) {
    return false;
  }

  bool HasGEPInfo = Ref1->hasGEPInfo();

  // Check if one is GEP ref and other is not.
  if (HasGEPInfo != Ref2->hasGEPInfo()) {
    return false;
  }

  if (HasGEPInfo) {
    // TODO: compare attributes like volatile, alignment etc.

    if (!IgnoreAddressOf && (Ref1->isMemRef() != Ref2->isMemRef())) {
      return false;
    }

    if (!RelaxedMode && Ref1->getBitCastDestVecOrElemType() !=
                            Ref2->getBitCastDestVecOrElemType()) {
      return false;
    }

    if (!haveEqualBaseAndShape(Ref1, Ref2, RelaxedMode)) {
      return false;
    }
  }

  unsigned NumDims = Ref1->getNumDimensions();

  for (unsigned I = NumDims; I > 0; --I) {
    const CanonExpr *Ref1CE = Ref1->getDimensionIndex(I);
    const CanonExpr *Ref2CE = Ref2->getDimensionIndex(I);

    if (!CanonExprUtils::areEqual(Ref1CE, Ref2CE, RelaxedMode)) {
      return false;
    }

    if (HasGEPInfo && compareOffsets(Ref1, Ref2, I)) {
      return false;
    }
  }

  // All the dimensions match.
  return true;
}

bool DDRefUtils::areEqual(const DDRef *Ref1, const DDRef *Ref2,
                          bool RelaxedMode) {

  assert(Ref1 && Ref2 && "Ref1/Ref2 parameter is null.");

  if (auto BRef1 = dyn_cast<BlobDDRef>(Ref1)) {
    auto BRef2 = dyn_cast<BlobDDRef>(Ref2);
    // Ref2 is Reg/Unknown Type, whereas Ref1 is Blob.
    if (!BRef2) {
      assert(isa<RegDDRef>(Ref2) && "Ref2 is unknown type.");
      return false;
    }

    return areEqualImpl(BRef1, BRef2);
  }

  if (auto RRef1 = dyn_cast<RegDDRef>(Ref1)) {
    auto RRef2 = dyn_cast<RegDDRef>(Ref2);
    // Ref2 is Blob/Unknown Type, whereas Ref1 is Blob.
    if (!RRef2) {
      assert(isa<BlobDDRef>(Ref2) && "Ref2 is unknown type.");
      return false;
    }

    return areEqualImpl(RRef1, RRef2, RelaxedMode);
  }

  llvm_unreachable("Unknown DDRef kind!");
  return false;
}

bool DDRefUtils::getConstDistanceImpl(const RegDDRef *Ref1,
                                      const RegDDRef *Ref2, unsigned LoopLevel,
                                      int64_t *Distance, bool RelaxedMode) {
  // Dealing with GEP refs only
  if (!Ref1->hasGEPInfo() || !Ref2->hasGEPInfo()) {
    return false;
  }

  if (!haveEqualBaseAndShape(Ref1, Ref2, RelaxedMode)) {
    return false;
  }

  int64_t Delta = 0;
  bool NeedIterDistance = (LoopLevel != 0);
  bool FoundDelta = false;

  // Compare the subscripts in reverse order to accommodate offsets.
  for (unsigned I = Ref1->getNumDimensions(); I > 0; --I) {

    // Compare trailing offsets.
    if ((I == 1) && !NeedIterDistance) {
      auto &DL = Ref1->getCanonExprUtils().getDataLayout();
      auto Ty = Ref1->getDimensionElementType(1);
      auto Offsets1 = Ref1->getTrailingStructOffsets(1);
      auto Offsets2 = Ref2->getTrailingStructOffsets(1);

      Delta += getOffsetDistance(Ty, DL, Offsets1);
      Delta -= getOffsetDistance(Ty, DL, Offsets2);

    } else if (compareOffsets(Ref1, Ref2, I)) {
      // Do not allow different trailing offsets for higher dimensions or when
      // computing iteration distance!
      return false;
    }

    const CanonExpr *Ref1CE = Ref1->getDimensionIndex(I);
    const CanonExpr *Ref2CE = Ref2->getDimensionIndex(I);

    int64_t CurDelta;

    bool Res = NeedIterDistance
                   ? CanonExprUtils::getConstIterationDistance(
                         Ref1CE, Ref2CE, LoopLevel, &CurDelta, RelaxedMode)
                   : CanonExprUtils::getConstDistance(Ref1CE, Ref2CE, &CurDelta,
                                                      RelaxedMode);

    if (!Res) {
      return false;
    }

    if (NeedIterDistance) {
      if (!Ref1CE->hasIV(LoopLevel)) {
        // CEs are invariant and equal.
        assert((CurDelta == 0) && "Invalid iteration distance!");
        continue;
      } else if (FoundDelta && (Delta != CurDelta)) {
        // Dimensions have different delta. for example- A[i][i] and A[i][i+1]
        return false;
      } else {
        FoundDelta = true;
        Delta = CurDelta;
      }

    } else if (CurDelta) {
      int64_t DimStride = Ref1->getDimensionConstStride(I);

      // Bail out for non-constant stride.
      if (!DimStride) {
        return false;
      }

      Delta += CurDelta * DimStride;
    }
  }

  if (Distance) {
    *Distance = Delta;
  }

  return true;
}

bool DDRefUtils::getConstByteDistance(const RegDDRef *Ref1,
                                      const RegDDRef *Ref2, int64_t *Distance,
                                      bool RelaxedMode) {
  return getConstDistanceImpl(Ref1, Ref2, 0, Distance, RelaxedMode);
}

bool DDRefUtils::getConstIterationDistance(const RegDDRef *Ref1,
                                           const RegDDRef *Ref2,
                                           unsigned LoopLevel,
                                           int64_t *Distance,
                                           bool RelaxedMode) {
  return getConstDistanceImpl(Ref1, Ref2, LoopLevel, Distance, RelaxedMode);
}

RegDDRef *DDRefUtils::createSelfBlobRef(unsigned Index, unsigned Level) {
  auto CE = getCanonExprUtils().createSelfBlobCanonExpr(Index, Level);
  unsigned Symbase = getBlobUtils().getTempBlobSymbase(Index);

  auto Ref = createRegDDRef(Symbase);
  Ref->setSingleCanonExpr(CE);

  return Ref;
}

void DDRefUtils::printMDNodes(formatted_raw_ostream &OS,
                              const RegDDRef::MDNodesTy &MDNodes) const {

  SmallVector<StringRef, 8> MDNames;

  getContext().getMDKindNames(MDNames);

  for (auto const &I : MDNodes) {
    OS << " ";
    if (I.first < MDNames.size()) {
      OS << "!";
      OS << MDNames[I.first] << " ";
    }

    I.second->printAsOperand(OS, &getModule());
  }
}

Type *DDRefUtils::getOffsetType(Type *Ty, ArrayRef<unsigned> Offsets) {
  Type *RetTy = Ty;

  for (auto OffsetVal : Offsets) {
    assert(RetTy->isStructTy() && "Structure type expected!");
    auto StrucTy = cast<StructType>(RetTy);
    RetTy = StrucTy->getElementType(OffsetVal);
  }

  return RetTy;
}

// This sorting compares the two ddref and orders them based on Ref's base,
// dimensions, IV's, blobs and then writes. Refs with equal bases (and no blobs)
// are sorted in increasing order of address location.
//
// Consider this set of refs-
// A[i+5][j], A[i][0] (Read), A[i][0] (Write), A[i][j], A[i+k][0]
//
// The sorting order is-
// A[i][0] (Write), A[i][0] (Read), A[i][j], A[i+5][j], A[i+k][0]
//
// As a comparator, compareMemRef must meet the requirements of Compare concept:
// For a long story see http://en.cppreference.com/w/cpp/concept/Compare
//
// Short story:
// Given: comp(a, b), equiv(a, b), an expression equivalent to
//                    !comp(a, b) && !comp(b, a)
//
// For any a, b, c:
// 1) comp(a,a)==false
// 2) if (comp(a,b)==true) comp(b,a)==false
// 3) if (comp(a,b)==true && comp(b,c)==true) comp(a,c)==true
// 4) equiv(a,a)==true
// 5) if (equiv(a,b)==true) equiv(b,a)==true
// 6) if (equiv(a,b)==true && equiv(b,c)==true) equiv(a,c)==true
//
static Optional<bool> compareMemRefImpl(const RegDDRef *Ref1,
                                        const RegDDRef *Ref2) {
  assert(Ref1->isMemRef() && Ref2->isMemRef() &&
         "Both RegDDRefs are expected to be memory references.");

  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE())) {
    return CanonExprUtils::compare(Ref1->getBaseCE(), Ref2->getBaseCE());
  }

  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return (Ref1->getNumDimensions() < Ref2->getNumDimensions());
  }

  // Check dimensions from highest to lowest.
  for (unsigned I = Ref1->getNumDimensions(); I > 0; --I) {
    const CanonExpr *CE1 = Ref1->getDimensionIndex(I);
    const CanonExpr *CE2 = Ref2->getDimensionIndex(I);

    if (!CanonExprUtils::areEqual(CE1, CE2)) {
      return CanonExprUtils::compare(CE1, CE2);
    }

    auto Diff = DDRefUtils::compareOffsets(Ref1, Ref2, I);

    if (Diff != 0) {
      return (Diff < 0);
    }
  }

  // This will order (i32*)A[0] before A[0] if A is i64* type
  auto DestTy1 = Ref1->getDestType();
  auto DestTy2 = Ref2->getDestType();

  if (DestTy1 != DestTy2) {
    return (Ref1->getCanonExprUtils().compare(DestTy1, DestTy2) < 0);
  }

  return {};
}

bool DDRefUtils::compareMemRef(const RegDDRef *Ref1, const RegDDRef *Ref2) {
  if (auto Ret = compareMemRefImpl(Ref1, Ref2)) {
    return *Ret;
  }

  // Place writes first in case everything matches.
  if (Ref1->isLval() != Ref2->isLval()) {
    return Ref1->isLval();
  }

  return false;
}

bool DDRefUtils::compareMemRefAddress(const RegDDRef *Ref1,
                                      const RegDDRef *Ref2) {
  if (auto Ret = compareMemRefImpl(Ref1, Ref2)) {
    return *Ret;
  }

  return false;
}

bool DDRefUtils::canReplaceIVByCanonExpr(const RegDDRef *Ref,
                                         unsigned LoopLevel,
                                         const CanonExpr *CE,
                                         bool RelaxedMode) {

  for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
    if (!CanonExprUtils::canReplaceIVByCanonExpr((*I), LoopLevel, CE,
                                                 RelaxedMode)) {
      return false;
    }
  }

  return true;
}

void DDRefUtils::replaceIVByCanonExpr(RegDDRef *Ref, unsigned LoopLevel,
                                      const CanonExpr *CE, bool IsSigned,
                                      bool RelaxedMode) {

  for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
    auto Res = CanonExprUtils::replaceIVByCanonExpr((*I), LoopLevel, CE,
                                                    IsSigned, RelaxedMode);
    (void)Res;
    assert(Res && "Replacement failed, caller should call "
                  "DDRefUtils::canReplaceIVByCanonExpr() first!");
  }
}

bool DDRefUtils::delinearizeRefs(ArrayRef<const loopopt::RegDDRef *> GepRefs,
                                 SmallVectorImpl<loopopt::RegDDRef *> &OutRefs,
                                 SmallVectorImpl<BlobTy> *DimSizes,
                                 bool AllowSExt) {
  assert(!GepRefs.empty() && "Empty input container");
  return GepRefs.front()->getDDRefUtils().getHIRParser().delinearizeRefs(
      GepRefs, OutRefs, DimSizes, AllowSExt);
}

bool DDRefUtils::isMemRefAllDimsConstOnly(const RegDDRef *Ref) {
  if (!Ref->isMemRef()) {
    return false;
  }

  for (unsigned I = 1, E = Ref->getNumDimensions(); I < E; ++I) {
    if (!Ref->getDimensionStride(I)->isIntConstant() ||
        !Ref->getDimensionIndex(I)->isIntConstant() ||
        !Ref->getDimensionLower(I)->isIntConstant()) {
      return false;
    }
  }

  return true;
}

#if INTEL_FEATURE_SW_DTRANS
bool DDRefUtils::hasConstantEntriesFromArray(const RegDDRef *Ref,
                                             DTransImmutableInfo *DTII,
                                             Constant *IndexInArray,
                                             Constant **ConstVal) {
  if (!DTII)
    return false;

  // Looking for a ref which accesses structure with array.
  unsigned NumDims = Ref->getNumDimensions();
  if (NumDims < 2)
    return false;

  // Last dimention should be array access.
  if (!Ref->getDimensionType(1)->isArrayTy())
    return false;

  StructType *StructTy = dyn_cast<StructType>(Ref->getDimensionElementType(2));
  if (!StructTy)
    return false;

  auto FieldOffsets = Ref->getTrailingStructOffsets(2);
  if (FieldOffsets.empty())
    return false;

  // The structure could have array as its field:
  //     %class.C = type <{i32, [4 x i32]}>
  // Or the array could be wrapped in a structure as it often happens with boost
  // libraries:
  //     %class.C = type <{i32, %"class.boost::array"}>
  //     %"class.boost::array" = type <{[4 x i32]}>
  // In the second case the special wrapping structure s only one field - the
  // boost array and could be used in the multiple outer structures. So DTrans
  // uses outer structure to store corresponding constant array values. That's
  // why in either case we use first field offset to access this information.
  uint64_t IndexOfArray = FieldOffsets[0];

  // If the current field is an array with constant integers, then store the
  // entries (pair.first) and its constant values (pair.second).
  auto *ConstantEntriesInArray =
      DTII->getConstantEntriesFromArray(StructTy, IndexOfArray);

  if (!ConstantEntriesInArray || ConstantEntriesInArray->empty())
    return false;

  // It is enough to check that DDRef is constant calculated by DTrans. No
  // particular value is needed.
  if (!ConstVal)
    return true;

  ConstantInt *ConstIndex = dyn_cast_or_null<ConstantInt>(IndexInArray);
  if (!ConstIndex)
    return false;

  for (auto *I = ConstantEntriesInArray->begin(),
            *E = ConstantEntriesInArray->end();
       I != E; ++I) {
    ConstantInt *ConstValue1 = dyn_cast<ConstantInt>(I->first);
    if (ConstValue1 && ConstValue1->getValue().getSExtValue() ==
                           ConstIndex->getValue().getSExtValue()) {
      *ConstVal = const_cast<Constant *>(I->second);
      break;
    }
  }

  if (*ConstVal) {
    return true;
  }

  return false;
}
#endif // INTEL_FEATURE_SW_DTRANS

#if INTEL_FEATURE_SW_DTRANS
RegDDRef *DDRefUtils::simplifyConstArray(const RegDDRef *Ref,
                                         DTransImmutableInfo *DTII) {
#else  // INTEL_FEATURE_SW_DTRANS
RegDDRef *DDRefUtils::simplifyConstArray(const RegDDRef *Ref) {
#endif // INTEL_FEATURE_SW_DTRANS
  if (!Ref->isMemRef() || Ref->isFake() || Ref->getBitCastDestVecOrElemType()) {
    return nullptr;
  }

  bool Precise;
  auto *LocationGEP = dyn_cast<GetElementPtrInst>(Ref->getLocationPtr(Precise));

  if (!LocationGEP || !Precise) {
    return nullptr;
  }

  // DD ref excesses global constant array
  if (Ref->accessesConstantArray()) {
    auto *GV = cast<GlobalVariable>(LocationGEP->getPointerOperand());
    if (!GV->hasDefinitiveInitializer()) {
      return nullptr;
    }

    SmallVector<Constant *, 8> Indices;
    // skip first index for global array
    for (unsigned I = 2, E = LocationGEP->getNumOperands(); I != E; ++I) {
      auto *Index = dyn_cast<Constant>(LocationGEP->getOperand(I));
      if (!Index) {
        return nullptr;
      }
      Indices.push_back(Index);
    }

    Constant *Val = nullptr;
    auto &DL = Ref->getCanonExprUtils().getDataLayout();
    unsigned OffsetBits = DL.getIndexTypeSizeInBits(LocationGEP->getType());
    APInt Offset(OffsetBits, 0);

    if (LocationGEP->accumulateConstantOffset(DL, Offset)) {
      Val = ConstantFoldLoadFromConst(GV->getInitializer(),
                                      LocationGEP->getResultElementType(),
                                      Offset, DL);
    }

    // TODO: add support for constant GEP exprs.
    if (!Val || isa<GEPOperator>(Val)) {
      return nullptr;
    }

    return Ref->getDDRefUtils().createConstDDRef(Val);
  }

#if INTEL_FEATURE_SW_DTRANS
  // Check if DD ref has a constant value calculated by DTrans.
  if (DTII && LocationGEP->getNumOperands() >= 4) {
    Constant *IndexInArray = dyn_cast<Constant>(
        LocationGEP->getOperand(LocationGEP->getNumOperands() - 1));
    if (!IndexInArray)
      return nullptr;

    Constant *Val = nullptr;
    if (hasConstantEntriesFromArray(Ref, DTII, IndexInArray, &Val)) {
      if (Val && isa<ConstantInt>(Val))
        return Ref->getDDRefUtils().createConstDDRef(Val);
    }
  }
#endif // INTEL_FEATURE_SW_DTRANS

  return nullptr;
}
