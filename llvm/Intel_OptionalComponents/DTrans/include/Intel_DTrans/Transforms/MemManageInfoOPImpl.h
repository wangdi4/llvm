//===---------  MemManageInfoOPImpl.h -------------------------------------===//
// common code for Memory Management Trans for Opaque Pointers
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file is mainly used to collect candidates and their info for the
// Memory Management Transformation Opaque Pointer pass in both pre-LTO and LTO
// phases. In Pre-LTO phase, this is used to suppress / force inlining
// for member functions that are related to potential candidates. In the LTO
// phase, this is used for the transformation.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_MEMMANAGEINFOOPIMPL_H
#define INTEL_DTRANS_TRANSFORMS_MEMMANAGEINFOOPIMPL_H

#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Transforms/ClassInfoOPUtils.h"

#define DTRANS_MEMMANAGEINFOOP "dtrans-memmanageinfoop"

namespace llvm {

namespace dtransOP {
class MemManageCandidateInfo {

  // Min Limit: StringObject size.
  constexpr static int MinStringObjectSize = 64;

  // Limit: Number of StringObject elements.
  constexpr static int NumStringObjectElems = 2;

public:
  MemManageCandidateInfo(Module &M) : M(M){};

  inline bool isCandidateType(DTransType *Ty);

private:
  Module &M;

  // Candidate struct that uses Allocator class to allocate objects.
  DTransStructType *StringAllocatorType = nullptr;

  // Memory Interface class: Used to allocate/deallocate memory.
  DTransStructType *MemInterfaceType = nullptr;

  // Object type that is allocated/deallocated by the Allocator.
  DTransStructType *StringObjectType = nullptr;

  // Class that implements the functionality of Allocator.
  // Member functions of this class are called in "StringAllocatorType".
  DTransStructType *ReusableArenaAllocatorType = nullptr;

  // Base class of "ReusableArenaAllocatorType". Member functions
  // of this class are also called in "StringAllocatorType".
  DTransStructType *ArenaAllocatorType = nullptr;

  // Type of Node class.
  DTransStructType *ListNodeType = nullptr;

  // Type of ReusableArenaBlockType class.
  DTransStructType *ReusableArenaBlockType = nullptr;

  // Type of BlockBaseType class.
  DTransStructType *BlockBaseType = nullptr;

  // Type of List class.
  DTransStructType *ListType = nullptr;

  // Index of ArenaAllocator in ReusableArenaAllocator
  int32_t ArenaAllocatorObjectIndex = -1;

  // Index of destroyBlock in ReusableArenaAllocator
  int32_t DestroyBlockFlagIndex = -1;

  // Index of List in ArenaAllocator
  int32_t ListObjectIndex = -1;

  // Index of blockSize in ArenaAllocator
  int32_t AllocatorBlockSizeIndex = -1;

  // Index of MemManager in List
  int32_t ListMemManagerIndex = -1;

  // Index of listHead in List
  int32_t ListHeadIndex = -1;

  // Index of listFreeHead in List
  int32_t ListFreeHeadIndex = -1;

  // Index of Allocator in BlockBase
  int32_t BasicAllocatorIndex = -1;

  // Index of ObjectCount in BlockBase
  int32_t BlockObjectCountIndex = -1;

  // Index of BlockSize in BlockBase
  int32_t BlockBlockSizeIndex = -1;

  // Index of StringObject in BlockBase
  int32_t StringObjectIndex = -1;

  // Index of BlockBase class in ReusableArenaBlock.
  int32_t BlockBaseObjIndex = -1;

  // Index of FirstFreeBlockIndex in ReusableArenaBlock
  int32_t FirstFreeBlockIndex = -1;

  // Index of NextFreeBlockIndex in ReusableArenaBlock
  int32_t NextFreeBlockIndex = -1;

  // Index of ReusableArenaBlock in ListNode
  int32_t ReusableArenaBlockIndex = -1;

  // Index of NodePrevIndex in ListNode
  int32_t NodePrevIndex = -1;

  // Index of NodeNextIndex in ListNode
  int32_t NodeNextIndex = -1;

  inline bool isBasicAllocatorType(DTransType *Ty);
  inline bool isBlockBaseType(DTransType *Ty);
  inline bool isReusableArenaBlockType(DTransType *Ty);
  inline bool isListNodeType(DTransType *Ty);
  inline bool isReusableArenaAllocatorType(DTransType *Ty);
  inline bool isStringAllocatorType(DTransType *Ty);
  inline bool isListType(DTransType *Ty);
  inline bool isArenaAllocatorType(DTransType *Ty);
  inline bool isStructWithNoRealData(DTransType *);
  inline bool isStringObjectType(DTransType *);
};

// Returns true if 'Ty' is a struct that doesn't have any real data
// except vftable.
// Ex:
//      %"MemoryManager" = type { i32 (...)** }
//
bool MemManageCandidateInfo::isStructWithNoRealData(DTransType *Ty) {
  DTransStructType *STy = getValidStructTy(Ty);
  if (!STy || STy->getNumFields() != 1)
    return false;

  DTransType *ETy = STy->getFieldType(0);
  if (!ETy)
    return false;

  if (!isPtrToVFTable(ETy))
    return false;

  if (!MemInterfaceType)
    MemInterfaceType = STy;
  else if (MemInterfaceType != STy)
    return false;

  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
//   %"Allocator" = type { %"MemoryManager"* }
//
bool MemManageCandidateInfo::isBasicAllocatorType(DTransType *Ty) {
  DTransStructType *STy = getValidStructTy(Ty);
  if (!STy || STy->getNumFields() != 1)
    return false;

  DTransType *MTy = getPointeeType(STy->getFieldType(0));
  if (!MTy)
    return false;

  if (!isStructWithNoRealData(MTy))
    return false;

  return true;
}

// Returns true if 'Ty' is a pointer to a struct type that looks like the
// following:
//
//   %"StringObject" = type { %"StringBase", %"GetAndReleaseCachedString" }
//
// where:
//   %"StringBase" = type { %"XObject", double, %"XObjectResultTreeFragProxy" }
//   %"GetAndReleaseCachedString" = type { %"XPathExecutionContext"*,
//                                         %"XalanDOMString"* }
//
bool MemManageCandidateInfo::isStringObjectType(DTransType *Ty) {
  DTransType *PTy = getPointeeType(Ty);
  if (!PTy)
    return false;

  auto *StructTy = dyn_cast<DTransStructType>(PTy);
  if (!StructTy)
    return false;

  const DataLayout &DL = M.getDataLayout();
  uint64_t Size =
      DL.getTypeAllocSize(cast<llvm::StructType>(StructTy->getLLVMType()));
  if (Size < MinStringObjectSize)
    return false;

  // Heuristics to avoid other allocators that look like StringAllocator
  // candidate. We are interested in only the allocator that allocates
  // memory for StringObject types.
  int32_t NumFields = StructTy->getNumFields();
  if (NumFields != NumStringObjectElems)
    return false;

  for (int32_t FieldIdx = 0; FieldIdx < NumFields; ++FieldIdx) {
    auto *ETy =
        dyn_cast_or_null<DTransStructType>(StructTy->getFieldType(FieldIdx));
    if (!ETy)
      return false;
  }

  auto *LastElem =
      dyn_cast_or_null<DTransStructType>(StructTy->getFieldType(1));
  if (!LastElem || LastElem->getNumFields() != NumStringObjectElems)
    return false;

  // Allow only pointers as fields.
  int32_t LastElemNumFields = LastElem->getNumFields();
  for (int32_t FieldIdx2 = 0; FieldIdx2 < LastElemNumFields; ++FieldIdx2) {
    auto *ETy =
        dyn_cast_or_null<DTransPointerType>(LastElem->getFieldType(FieldIdx2));
    if (!ETy)
      return false;
  }

  auto *FirstElem =
      dyn_cast_or_null<DTransStructType>(StructTy->getFieldType(0));
  if (FirstElem->getNumFields() != NumStringObjectElems + 1)
    return false;

  // Don't allow any field types except structure or double in the first
  // structure.
  int32_t FirstElemNumFields = FirstElem->getNumFields();
  for (int32_t FieldIdx3 = 0; FieldIdx3 < FirstElemNumFields; ++FieldIdx3) {
    DTransType *ETy = FirstElem->getFieldType(FieldIdx3);
    if (!ETy)
      return false;

    if (!isa<DTransStructType>(ETy) && !ETy->getLLVMType()->isDoubleTy())
      return false;
  }

  StringObjectType = StructTy;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
//   %"BlockBase" = type { %"Allocator", i16, i16, %"StringCached"* }
//
bool MemManageCandidateInfo::isBlockBaseType(DTransType *Ty) {
  DTransStructType *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumCounters = 0;
  unsigned NumBasicAllocatons = 0;
  unsigned NumStringPtrs = 0;
  int32_t NumFields = STy->getNumFields();
  for (int32_t FieldIdx = 0; FieldIdx < NumFields; ++FieldIdx) {
    DTransType *ETy = STy->getFieldType(FieldIdx);
    if (!ETy)
      return false;

    if (ETy->getLLVMType()->isIntegerTy(16)) {
      NumCounters++;

      // Assume the first i16 field as ObjectCount and the second
      // i16 field as BlockSize.
      if (BlockObjectCountIndex == -1)
        BlockObjectCountIndex = FieldIdx;
      else if (BlockBlockSizeIndex == -1)
        BlockBlockSizeIndex = FieldIdx;
      else
        return false;
      continue;
    }

    if (isBasicAllocatorType(ETy)) {
      NumBasicAllocatons++;
      BasicAllocatorIndex = FieldIdx;
      continue;
    }

    if (isStringObjectType(ETy)) {
      NumStringPtrs++;
      StringObjectIndex = FieldIdx;
      continue;
    }

    return false;
  }

  if (NumCounters != 2 || NumStringPtrs != 1 || NumBasicAllocatons != 1)
    return false;

  BlockBaseType = STy;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
//   %"ReusableArenaBlock" = type { %"BlockBase", i16, i16, [4 x i8] }
//
bool MemManageCandidateInfo::isReusableArenaBlockType(DTransType *Ty) {
  DTransStructType *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumCounters = 0;
  unsigned NumBlockBase = 0;
  unsigned NumUnused = 0;
  int32_t NumFields = STy->getNumFields();
  for (int32_t FieldIdx = 0; FieldIdx < NumFields; ++FieldIdx) {
    DTransType *ETy = STy->getFieldType(FieldIdx);
    if (!ETy)
      return false;

    if (isPotentialPaddingField(ETy)) {
      NumUnused++;
      continue;
    }

    if (ETy->getLLVMType()->isIntegerTy(16)) {
      NumCounters++;
      // Assume the first i16 field as FirstFreeBlock and the second
      // i16 field as NextFreeBlock.
      if (FirstFreeBlockIndex == -1)
        FirstFreeBlockIndex = FieldIdx;
      else if (NextFreeBlockIndex == -1)
        NextFreeBlockIndex = FieldIdx;
      else
        return false;
      continue;
    }

    if (isBlockBaseType(ETy)) {
      NumBlockBase++;
      BlockBaseObjIndex = FieldIdx;
      continue;
    }

    return false;
  }

  if (NumCounters != 2 || NumUnused > 1 || NumBlockBase != 1)
    return false;

  ReusableArenaBlockType = STy;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
//   %"ListNode" = type { %"ReusableArenaBlock"*, %"ListNode"*, %"ListNode"* }
//
bool MemManageCandidateInfo::isListNodeType(DTransType *Ty) {
  DTransStructType *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumListNodePtrs = 0;
  unsigned NumReusableArenaBlock = 0;
  int32_t NumFields = STy->getNumFields();
  for (int32_t FieldIdx = 0; FieldIdx < NumFields; ++FieldIdx) {
    DTransType *ETy = STy->getFieldType(FieldIdx);
    if (!ETy)
      return false;

    DTransType *PTy = getPointeeType(ETy);
    if (!PTy)
      return false;

    if (PTy == Ty) {
      NumListNodePtrs++;

      // Assume the first NodeTy field as NodePrev and the second
      // NodeTy field as NodeNext.
      if (NodePrevIndex == -1)
        NodePrevIndex = FieldIdx;
      else if (NodeNextIndex == -1)
        NodeNextIndex = FieldIdx;
      else
        return false;
      continue;
    }

    if (isReusableArenaBlockType(PTy)) {
      NumReusableArenaBlock++;
      ReusableArenaBlockIndex = FieldIdx;
      continue;
    }

    return false;
  }

  if (NumListNodePtrs != 2 || NumReusableArenaBlock != 1)
    return false;

  ListNodeType = STy;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
//   %"List" = type { %"MemoryManager"*, %"ListNode"*, %"ListNode"* }
//
bool MemManageCandidateInfo::isListType(DTransType *Ty) {
  DTransStructType *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumListNodePtrs = 0;
  unsigned NumMemInt = 0;
  DTransType *NodeTy = nullptr;
  int32_t NumFields = STy->getNumFields();
  for (int32_t FieldIdx = 0; FieldIdx < NumFields; ++FieldIdx) {
    DTransType *ETy = STy->getFieldType(FieldIdx);
    if (!ETy)
      return false;

    DTransType *PTy = getPointeeType(ETy);
    if (!PTy)
      return false;

    if (isStructWithNoRealData(PTy)) {
      NumMemInt++;
      ListMemManagerIndex = FieldIdx;
      continue;
    }

    if (NodeTy) {
      if (NodeTy != PTy)
        return false;

      NumListNodePtrs++;
      ListFreeHeadIndex = FieldIdx;
      continue;
    } else if (isListNodeType(PTy)) {
      NodeTy = PTy;
      NumListNodePtrs++;
      // Assume the first NodeTy pointer as ListHead and the second
      // NodeTy pointer as ListFreeHead.
      ListHeadIndex = FieldIdx;
      continue;
    }

    return false;
  }

  if (NumListNodePtrs != 2 || NumMemInt != 1)
    return false;

  ListType = STy;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
//   %"ArenaAllocator" = type { i32 (...)**, i16, %"XalanList" }
//
bool MemManageCandidateInfo::isArenaAllocatorType(DTransType *Ty) {
  DTransStructType *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumVFTablePtrs = 0;
  unsigned NumLists = 0;
  unsigned NumInts = 0;
  int32_t NumFields = STy->getNumFields();
  for (int32_t FieldIdx = 0; FieldIdx < NumFields; ++FieldIdx) {
    DTransType *ETy = STy->getFieldType(FieldIdx);
    if (!ETy)
      return false;

    if (isPtrToVFTable(ETy)) {
      // Expect VFTable as first field always.
      if (FieldIdx != 0)
        return false;
      NumVFTablePtrs++;
      continue;
    }

    if (isListType(ETy)) {
      NumLists++;
      ListObjectIndex = FieldIdx;
      continue;
    }

    if (ETy->getLLVMType()->isIntegerTy(16)) {
      NumInts++;
      AllocatorBlockSizeIndex = FieldIdx;
      continue;
    }

    return false;
  }

  if (NumLists != 1 || NumVFTablePtrs != 1 || NumInts != 1)
    return false;

  ArenaAllocatorType = STy;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
//   %"ReusableArenaAllocator" = type { %"ArenaAllocator", i8, [7 x i8] }
//
// or
//   %"ReusableArenaAllocator" = type { %"ArenaAllocator", i8 }
//
bool MemManageCandidateInfo::isReusableArenaAllocatorType(DTransType *Ty) {
  DTransStructType *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumArenaAllocator = 0;
  unsigned NumFlags = 0;
  unsigned NumUnused = 0;
  int32_t NumFields = STy->getNumFields();
  for (int32_t FieldIdx = 0; FieldIdx < NumFields; ++FieldIdx) {
    DTransType *ETy = STy->getFieldType(FieldIdx);
    if (!ETy)
      return false;

    if (isPotentialPaddingField(ETy)) {
      NumUnused++;
      continue;
    }

    if (isArenaAllocatorType(ETy)) {
      NumArenaAllocator++;
      ArenaAllocatorObjectIndex = FieldIdx;
      continue;
    }

    if (ETy->getLLVMType()->isIntegerTy(8)) {
      NumFlags++;
      DestroyBlockFlagIndex = FieldIdx;
      continue;
    }

    return false;
  }

  if (NumArenaAllocator != 1 || NumUnused > 1 || NumFlags != 1)
    return false;

  ReusableArenaAllocatorType = STy;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
//   %"StringAllocator" = type { %"ReusableArenaAllocator" }
//
bool MemManageCandidateInfo::isStringAllocatorType(DTransType *Ty) {
  DTransStructType *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumReusableArenaAllocator = 0;
  int32_t NumFields = STy->getNumFields();
  for (int32_t FieldIdx = 0; FieldIdx < NumFields; ++FieldIdx) {
    DTransType *ETy = STy->getFieldType(FieldIdx);
    if (!ETy)
      return false;

    if (isReusableArenaAllocatorType(ETy)) {
      NumReusableArenaAllocator++;
      continue;
    }
    return false;
  }

  if (NumReusableArenaAllocator != 1)
    return false;

  StringAllocatorType = STy;
  return true;
}

// Returns true if 'Ty' is "StringAllocator" struct.
bool MemManageCandidateInfo::isCandidateType(DTransType *Ty) {
  if (!isStringAllocatorType(Ty))
    return false;
  return true;
}

} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_MEMMANAGEINFOOPIMPL_H
