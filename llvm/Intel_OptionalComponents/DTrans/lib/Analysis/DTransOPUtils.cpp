//===-------- DTransOpUtils.cpp - Utilities for DTrans opaque pointers ----===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
// General utilities for DTrans opaque pointer classes.
///
// ===--------------------------------------------------------------------=== //

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransOPUtils.h"

namespace llvm {
namespace dtransOP {

bool hasPointerType(DTransType *Ty) {
  if (Ty->isPointerTy())
    return true;

  if (auto *ArTy = dyn_cast<DTransArrayType>(Ty))
    return hasPointerType(ArTy->getArrayElementType());
  if (auto *VecTy = dyn_cast<DTransVectorType>(Ty))
    return hasPointerType(VecTy->getVectorElementType());

  if (auto *STy = dyn_cast<DTransStructType>(Ty)) {
    // Check inside of literal structures because those cannot be referenced by
    // name. However, do not look inside non-literal structures because those
    // will be referenced by their name.
    if (STy->isLiteralStruct()) {
      for (auto &FieldMember : STy->elements()) {
        DTransType *FieldTy = FieldMember.getType();
        assert(FieldTy && "Metadata reader had ambiguous types");
        if (hasPointerType(FieldTy))
          return true;
      }
    }
  }

  if (auto *FuncTy = dyn_cast<DTransFunctionType>(Ty)) {
    DTransType *RetTy = FuncTy->getReturnType();
    assert(RetTy && "Metadata reader had ambiguous types");
    if (hasPointerType(RetTy))
      return true;

    unsigned NumParams = FuncTy->getNumArgs();
    for (unsigned Idx = 0; Idx < NumParams; ++Idx) {
      DTransType *ArgTy = FuncTy->getArgType(Idx);
      assert(ArgTy && "Metadata reader had ambiguous types");
      if (hasPointerType(ArgTy))
        return true;
    }
  }

  return false;
}

DTransType *unwrapDTransType(DTransType *Ty) {
  DTransType *BaseTy = Ty;
  while (BaseTy->isPointerTy() || BaseTy->isArrayTy() || BaseTy->isVectorTy()) {
    if (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();
    else if (BaseTy->isArrayTy())
      BaseTy = cast<DTransArrayType>(BaseTy)->getElementType();
    else if (BaseTy->isVectorTy())
      BaseTy = cast<DTransVectorType>(BaseTy)->getElementType();
  }

  return BaseTy;
}

// Helper to analyze a pointer-to-member usage to determine if only a
// specific subset of the structure fields of \p StructTy, starting from \p
// FieldNum and extending by \p AccessSize bytes of the structure are
// touched.
//
// Return 'true' if it can be resolved to precisely match one or more
// adjacent fields starting with the field number identified in the 'LPI'.
// If so, also updated the RegionDesc to set the starting index into
// 'FirstField' and the ending index of affected fields into 'LastField'.
// Otherwise, return 'false'.
//
// Because of the way the code in the DTransSafetyAnalyzer.cpp has been
// modified to require the results in a specific order, 'RegionDescVec'
// must be generated to list the regions from innermost to outermost aggregate
// type.
static bool analyzeStructFieldAccess(const DataLayout &DL,
                                     DTransStructType *StructTy,
                                     size_t FieldNum, uint64_t PrePadBytes,
                                     uint64_t AccessSize, bool AllowRecurse,
                                     MFTypeRegionVec &RegionDescVec) {
  llvm::StructType *LLVMSTy = cast<llvm::StructType>(StructTy->getLLVMType());
  uint64_t TypeSize = DL.getTypeAllocSize(LLVMSTy);

  // If the size is larger than the base structure size, then the write
  // exceeds the bounds of a single structure, and it's an unsupported
  // use.
  if (AccessSize > TypeSize)
    return false;

  // Try to identify the range of fields being accessed based on the
  // layout of the structure.
  auto FieldTypes = LLVMSTy->elements();
  auto *SL = DL.getStructLayout(LLVMSTy);
  uint64_t FieldOffset = SL->getElementOffset(FieldNum);
  uint64_t AccessStart = FieldOffset - PrePadBytes;
  uint64_t AccessEnd = AccessStart + AccessSize - 1;

  // Check that the access stays within the memory region of the structure,
  // and is not just padding bytes between the fields.
  if (AccessEnd > TypeSize || AccessEnd < FieldOffset)
    return false;

  // If the last field was not completely read or written, mark it
  // conservatively for reading and writing analyses like field single/
  // multiple value. For now, we will not do transformations that will
  // need to transform the memfuncs.
  unsigned int LF = SL->getElementContainingOffset(AccessEnd);
  uint64_t LastFieldStart = SL->getElementOffset(LF);
  uint64_t LastFieldSize = DL.getTypeStoreSize(FieldTypes[LF]);
  uint64_t PostPadBytes = AccessEnd - (LastFieldStart + LastFieldSize - 1);
  if (AccessEnd < (LastFieldStart + LastFieldSize - 1)) {
    if (!AllowRecurse)
      return false;
    auto LTy = dyn_cast<DTransStructType>(StructTy->getFieldType(LF));
    if (!LTy)
      return false;
    if (!analyzeStructFieldAccess(DL, LTy, 0, 0,
        AccessEnd + 1 - LastFieldStart, true, RegionDescVec))
      return false;
    if (LF < FieldNum)
      return false;
  }
  
  dtrans::MemfuncRegion RegionDesc;
  RegionDesc.PrePadBytes = PrePadBytes;
  RegionDesc.FirstField = FieldNum;
  RegionDesc.LastField = LF;
  RegionDesc.PostPadBytes = PostPadBytes;
  if (!(FieldNum == 0 && LF == (LLVMSTy->getNumElements() - 1)))
    RegionDesc.IsCompleteAggregate = false;
  else
    RegionDesc.IsCompleteAggregate = true;
  RegionDescVec.push_back({StructTy, RegionDesc});
  return true;
}

bool analyzePartialStructUse(const DataLayout &DL,
                             DTransStructType *StructTy,
                             size_t FieldNum, uint64_t PrePadBytes,
                             const Value *AccessSizeVal,
                             bool AllowRecurse,
                             MFTypeRegionVec &RegionDescVec) {
  if (!StructTy)
    return false;

  if (!AccessSizeVal)
    return false;

  auto *AccessSizeCI = dyn_cast<ConstantInt>(AccessSizeVal);
  if (!AccessSizeCI)
    return false;

  uint64_t AccessSize = AccessSizeCI->getLimitedValue();
  assert(FieldNum <
     cast<llvm::StructType>(StructTy->getLLVMType())->getNumElements());

  return analyzeStructFieldAccess(DL, StructTy, FieldNum, PrePadBytes,
                                  AccessSize, AllowRecurse, RegionDescVec);
}

} // end namespace dtransOP
} // end namespace llvm
