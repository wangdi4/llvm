//===-------- DDRefUtils.cpp - Implements DDRefUtils class ----------------===//
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
// This file implements DDRefUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"

#include "llvm/IR/Constants.h" // needed for UndefValue class

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

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

RegDDRef *DDRefUtils::createConstDDRef(Type *Ty, int64_t Val) {
  RegDDRef *NewRegDD = createRegDDRef(ConstantSymbase);
  CanonExpr *CE = getCanonExprUtils().createCanonExpr(Ty, 0, Val);
  NewRegDD->setSingleCanonExpr(CE);

  return NewRegDD;
}

RegDDRef *DDRefUtils::createConstDDRef(Value *Val) {
  RegDDRef *NewRegDD = createRegDDRef(ConstantSymbase);
  // Create a linear self-blob constant canon expr.
  auto CE = getCanonExprUtils().createConstStandAloneBlobCanonExpr(Val);
  NewRegDD->setSingleCanonExpr(CE);

  return NewRegDD;
}

BlobDDRef *DDRefUtils::createBlobDDRef(unsigned Index, unsigned Level) {
  return new BlobDDRef(*this, Index, Level);
}

void DDRefUtils::destroy(DDRef *Ref) {
  auto Count = Objs.erase(Ref);
  assert(Count && "Ref not found in objects!");
  delete Ref;
}

void DDRefUtils::destroyAll() {
  for (auto &I : Objs) {
    delete I;
  }

  Objs.clear();

  getCanonExprUtils().destroyAll();
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
  return getBlobUtils().getHIRSymbaseAssignment().getNewSymbase();
}

bool DDRefUtils::areEqualImpl(const BlobDDRef *Ref1, const BlobDDRef *Ref2) {

  assert(Ref1 && Ref2 && "Ref1/Ref2 parameter is null.");

  if ((Ref1->getSymbase() != Ref2->getSymbase())) {
    return false;
  }

  // Additional check. Ideally, symbase match should be equal blobs.
  assert(CanonExprUtils::areEqual(Ref1->getCanonExpr(), Ref2->getCanonExpr()));

  return true;
}

int64_t
DDRefUtils::getOffsetDistance(Type *Ty, const DataLayout &DL,
                              const SmallVectorImpl<unsigned> &Offsets) {
  int64_t DistInBytes = 0;

  for (auto OffsetVal : Offsets) {
    assert(Ty->isStructTy() && "StructType expected!");
    auto STy = cast<StructType>(Ty);
    DistInBytes += DL.getStructLayout(STy)->getElementOffset(OffsetVal);
    Ty = STy->getElementType(OffsetVal);
  }

  return DistInBytes;
}

int DDRefUtils::compareOffsets(const SmallVectorImpl<unsigned> &Offsets1,
                               const SmallVectorImpl<unsigned> &Offsets2) {
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

  if (Offsets1 && Offsets2) {
    return compareOffsets(*Offsets1, *Offsets2);
  } else if (Offsets1) {
    // Only Ref1 has offsets.
    return 1;
  } else if (Offsets2) {
    // Only Ref2 has offsets.
    return -1;
  }

  return 0;
}

bool DDRefUtils::areEqualImpl(const RegDDRef *Ref1, const RegDDRef *Ref2,
                              bool RelaxedMode) {

  // Match the symbase. Type checking is done inside the CEUtils.
  if ((Ref1->getSymbase() != Ref2->getSymbase())) {
    return false;
  }

  bool HasGEPInfo = Ref1->hasGEPInfo();

  // Check if one is memory ref and other is not.
  if (HasGEPInfo != Ref2->hasGEPInfo()) {
    return false;
  }

  // Check Base Canon Exprs.
  if (HasGEPInfo &&
      !CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE(),
                                RelaxedMode)) {
    return false;
  }

  unsigned NumDims = Ref1->getNumDimensions();

  if (NumDims != Ref2->getNumDimensions()) {
    return false;
  }

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

  } else if (auto RRef1 = dyn_cast<RegDDRef>(Ref1)) {
    auto RRef2 = dyn_cast<RegDDRef>(Ref2);
    // Ref2 is Blob/Unknown Type, whereas Ref1 is Blob.
    if (!RRef2) {
      assert(isa<BlobDDRef>(Ref2) && "Ref2 is unknown type.");
      return false;
    }

    return areEqualImpl(RRef1, RRef2, RelaxedMode);

  } else {
    llvm_unreachable("Unknown DDRef kind!");
  }

  return false;
}

bool DDRefUtils::getConstDistanceImpl(const RegDDRef *Ref1,
                                      const RegDDRef *Ref2, unsigned LoopLevel,
                                      int64_t *Distance) {

  // Dealing with GEP refs only
  if (!Ref1->hasGEPInfo() || !Ref2->hasGEPInfo()) {
    return false;
  }

  const CanonExpr *BaseCE1 = Ref1->getBaseCE();
  const CanonExpr *BaseCE2 = Ref2->getBaseCE();

  if (!CanonExprUtils::areEqual(BaseCE1, BaseCE2)) {
    return false;
  }

  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return false;
  }

  int64_t Delta = 0;
  bool NeedIterDistance = (LoopLevel != 0);
  bool FoundDelta = false;

  // Compare the subscripts in reverse order to accomodate offsets.
  for (unsigned I = Ref1->getNumDimensions(); I > 0; --I) {

    // Compare trailing offsets.
    if ((I == 1) && !NeedIterDistance) {
      auto &DL = Ref1->getCanonExprUtils().getDataLayout();
      auto Ty = Ref1->getDimensionElementType(1);
      auto Offsets1 = Ref1->getTrailingStructOffsets(1);
      auto Offsets2 = Ref2->getTrailingStructOffsets(1);

      if (Offsets1) {
        Delta += getOffsetDistance(Ty, DL, *Offsets1);
      }

      if (Offsets2) {
        Delta -= getOffsetDistance(Ty, DL, *Offsets2);
      }
    } else if (compareOffsets(Ref1, Ref2, I)) {
      // Do not allow different trailing offsets for higher dimensions or when
      // computing iteration distance!
      return false;
    }

    const CanonExpr *Ref1CE = Ref1->getDimensionIndex(I);
    const CanonExpr *Ref2CE = Ref2->getDimensionIndex(I);

    int64_t CurDelta;

    bool Res =
        NeedIterDistance
            ? CanonExprUtils::getConstIterationDistance(Ref1CE, Ref2CE,
                                                        LoopLevel, &CurDelta)
            : CanonExprUtils::getConstDistance(Ref1CE, Ref2CE, &CurDelta);

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
    } else {
      uint64_t DimStride = Ref1->getDimensionStride(I);
      Delta += CurDelta * DimStride;
    }
  }

  if (Distance) {
    *Distance = Delta;
  }

  return true;
}

bool DDRefUtils::getConstByteDistance(const RegDDRef *Ref1,
                                      const RegDDRef *Ref2, int64_t *Distance) {
  return getConstDistanceImpl(Ref1, Ref2, 0, Distance);
}

bool DDRefUtils::getConstIterationDistance(const RegDDRef *Ref1,
                                           const RegDDRef *Ref2,
                                           unsigned LoopLevel,
                                           int64_t *Distance) {
  return getConstDistanceImpl(Ref1, Ref2, LoopLevel, Distance);
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
  auto &HIRP = getHIRParser();

  HIRP.getContext().getMDKindNames(MDNames);

  for (auto const &I : MDNodes) {
    OS << " ";
    if (I.first < MDNames.size()) {
      OS << "!";
      OS << MDNames[I.first] << " ";
    }

    I.second->printAsOperand(OS, &HIRP.getModule());
  }
}

Type *DDRefUtils::getOffsetType(Type *Ty,
                                const SmallVectorImpl<unsigned> &Offsets) {
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
bool DDRefUtils::compareMemRef(const RegDDRef *Ref1, const RegDDRef *Ref2) {
  assert(Ref1->isMemRef() && Ref2->isMemRef() &&
         "Both RegDDRefs are expected to be memory references.");

  if (!CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE()))
    return CanonExprUtils::compare(Ref1->getBaseCE(), Ref2->getBaseCE());

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

  // Place writes first in case everything matches.
  if (Ref1->isLval() != Ref2->isLval()) {
    return Ref1->isLval();
  }

  return false;
}

bool DDRefUtils::canReplaceIVByCanonExpr(const RegDDRef *Ref,
                                         unsigned LoopLevel,
                                         const CanonExpr *CE,
                                         bool RelaxedMode) {

  for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
    CanonExpr *CurCE = (*I);

    if (!CanonExprUtils::canReplaceIVByCanonExpr(CurCE, LoopLevel, CE,
                                                 RelaxedMode)) {
      return false;
    }
  }

  return true;
}

void DDRefUtils::replaceIVByCanonExpr(RegDDRef *Ref, unsigned LoopLevel,
                                      const CanonExpr *CE, bool RelaxedMode) {

  for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
    CanonExpr *CurCE = (*I);

    auto Res =
        CanonExprUtils::replaceIVByCanonExpr(CurCE, LoopLevel, CE, RelaxedMode);
    assert(Res && "Replacement failed, caller should call "
                  "DDRefUtils::canReplaceIVByCanonExpr() first!");
  }
}

