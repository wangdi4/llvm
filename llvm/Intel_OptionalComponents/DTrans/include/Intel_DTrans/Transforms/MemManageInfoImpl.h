//===----   MemManageInfoImpl.h - common for Memory Management Trans   ----===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file is mainly used to collect candidates and their info for
// Memory Management Transformation pass in both pre-LTO and LTO
// phases. In Pre-LTO phase, this is used to suppress / force  inlining
// for member functions that are related to potential candidates.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_MEMMANAGEINFOIMPL_H
#define INTEL_DTRANS_TRANSFORMS_MEMMANAGEINFOIMPL_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"

#define DTRANS_MEMMANAGEINFO "dtrans-memmanageinfo"

namespace llvm {

namespace dtrans {

// Returns true if Ty is pointer to pointer to a function.
// TODO: The same function is used by MemInitTrimDown. This code needs
// to be reused by both MemInitTrimDown & MemManageTrans.
inline bool isVFTablePointer(Type *Ty) {
  Type *ETy = nullptr;
  if (auto *PPETy = dyn_cast<PointerType>(Ty))
    if (auto *PETy = dyn_cast<PointerType>(PPETy->getElementType()))
      ETy = PETy->getElementType();
  if (!ETy || !ETy->isFunctionTy())
    return false;
  return true;
}

// Returns 'Ty' if 'Ty' is valid struct type to consider for the
// transformation.
inline StructType *getValidStructTy(Type *Ty) {
  StructType *STy = dyn_cast<StructType>(Ty);
  if (!STy || STy->isLiteral() || !STy->isSized())
    return nullptr;
  return STy;
}

// Returns type of pointee if 'Ty' is pointer.
inline Type *getPointeeType(Type *Ty) {
  if (auto *PTy = dyn_cast_or_null<PointerType>(Ty))
    return PTy->getElementType();
  return nullptr;
}

// Returns true if 'Ty' is potential padding field that
// is created to fill gaps in structs.
inline bool isPotentialPaddingField(Type *Ty) {
  ArrayType *ATy = dyn_cast<ArrayType>(Ty);
  if (!ATy || !ATy->getElementType()->isIntegerTy(8))
    return false;
  return true;
}

// Get class type of the given function if there is one.
inline StructType *getThisClassType(const Function *F) {
  FunctionType *FunTy = F->getFunctionType();
  if (FunTy->getNumParams() == 0)
    return nullptr;
  // Get class type from "this" pointer that is passed as 1st
  // argument.
  if (auto *PTy = dyn_cast<PointerType>(FunTy->getParamType(0)))
    if (auto *STy = dyn_cast<StructType>(PTy->getPointerElementType()))
      return STy;
  return nullptr;
}

// This is used to collect candidate for MemManageTrans and
// maintain information related to the candidate.
class MemManageCandidateInfo {

  // Max limit: Number of StringAllocator functions.
  constexpr static int MaxNumStringAllocatorFuncs = 5;

  // Max limit: Size of StringAllocator function.
  constexpr static int MaxSizeStringAllocatorFunc = 1;

  // Max limit: Number of Interface functions of Allocator.
  constexpr static int MaxNumAllocatorInterfaceFuncs = 7;

  // Max Pre-LTO limit: Size of interface functions of Allocator.
  constexpr static int MaxPreLTOSizeAllocatorInterfaceFunc = 30;

  // Max LTO limit: Size of interface functions of Allocator.
  constexpr static int MaxLTOSizeAllocatorInterfaceFunc = 100;

  // Max limit: Number of StringObject functions.
  constexpr static int MaxNumStringObjectFuncs = 2;

  // Max pre-LTO limit: Number of inner functions of Allocator.
  constexpr static int MaxPreLTONumAllocatorInnerFuncs = 90;

  // Max LTO limit: Number of inner functions of Allocator.
  constexpr static int MaxLTONumAllocatorInnerFuncs = 15;

  // Max pre-LTO limit: Number of inner function uses.
  constexpr static int MaxPreLTONumInnerFuncUses = 12;

  // Max pre-LTO limit: Size of inner function.
  constexpr static int MaxPreLTOInnerFuncSize = 22;

  // Max pre-LTO limit: Total inline size of all inner functions.
  constexpr static int MaxPreLTOInnerFuncTotalInlSize = 375;

  // Min Limit: StringObject size.
  constexpr static int MinStringObjectSize = 64;

  // Limit: Number of StringObject elements.
  constexpr static int NumStringObjectElems = 2;

public:
  using InterfaceSetTy = SmallPtrSet<Function *, 8>;
  using InnerCallSetTy = std::set<const CallBase *>;

  inline MemManageCandidateInfo(Module &M) : M(M){};

  inline bool isCandidateType(Type *Ty);
  inline bool collectMemberFunctions(bool);
  inline bool collectInlineNoInlineMethods(std::set<Function *> *,
                                           SmallSet<Function *, 16> *) const;

  // Iterator for AllocatorInterfaceFunctions
  typedef InterfaceSetTy::const_iterator m_const_iterator;
  inline iterator_range<m_const_iterator> interface_functions() {
    return make_range(AllocatorInterfaceFunctions.begin(),
                      AllocatorInterfaceFunctions.end());
  }

  // Iterator for AllocatorInnerCalls
  typedef InnerCallSetTy::const_iterator f_const_iterator;
  inline iterator_range<f_const_iterator> inner_function_calls() {
    return make_range(AllocatorInnerCalls.begin(), AllocatorInnerCalls.end());
  }

  // Returns true if "F" is either StrAllocator or interface function.
  bool isStrAllocatorOrInterfaceFunction(Function *F) {
    return (AllocatorInterfaceFunctions.count(F) ||
            StringAllocatorFunctions.count(F));
  }

  // Returns true if "F" is an interface function.
  bool isInterfaceFunction(Function *F) {
    return (AllocatorInterfaceFunctions.count(F));
  }

  // Returns true if "Ty" is ReusableArenaBlockType or any class related to
  // it.
  bool isRelatedType(StructType *Ty) {
    if (BlockBaseType == Ty || ReusableArenaBlockType == Ty ||
        ListNodeType == Ty || ListType == Ty || ArenaAllocatorType == Ty ||
        ReusableArenaAllocatorType == Ty)
      return true;
    return false;
  }

  // Returns StringObjectType.
  StructType *getStringObjectType() { return StringObjectType; }

  // Returns ReusableArenaAllocatorType.
  StructType *getReusableArenaAllocatorType() {
    return ReusableArenaAllocatorType;
  }

  // Returns ArenaAllocatorType.
  StructType *getArenaAllocatorType() { return ArenaAllocatorType; }

  // Returns MemInterfaceType.
  StructType *getMemInterfaceType() { return MemInterfaceType; }

  // Returns StringAllocatorType.
  StructType *getStringAllocatorType() { return StringAllocatorType; }

  // Returns ListNodeType.
  StructType *getListNodeType() { return ListNodeType; }

  // Returns ReusableArenaBlockType.
  StructType *getReusableArenaBlockType() { return ReusableArenaBlockType; }

  // Returns BlockBaseType.
  StructType *getBlockBaseType() { return BlockBaseType; }

  // Returns index of ArenaAllocatorObject.
  int32_t getArenaAllocatorObjectIndex() { return ArenaAllocatorObjectIndex; }
  // Returns index of destroyBlockFlag.
  int32_t getDestroyBlockFlagIndex() { return DestroyBlockFlagIndex; }
  // Returns index of List.
  int32_t getListObjectIndex() { return ListObjectIndex; }
  // Returns index of BlockSize.
  int32_t getAllocatorBlockSizeIndex() { return AllocatorBlockSizeIndex; }
  // Returns index of MemManager in List
  int32_t getListMemManagerIndex() { return ListMemManagerIndex; }
  // Returns index of Head in List.
  int32_t getListHeadIndex() { return ListHeadIndex; }
  // Returns index of FreeHead in List.
  int32_t getListFreeHeadIndex() { return ListFreeHeadIndex; }
  // Returns index of Allocator in BlockBase.
  int32_t getBasicAllocatorIndex() { return BasicAllocatorIndex; }
  // Returns index of ObjectCount in BlockBase.
  int32_t getBlockObjectCountIndex() { return BlockObjectCountIndex; }
  // Returns index of BlockSize in BlockBase.
  int32_t getBlockBlockSizeIndex() { return BlockBlockSizeIndex; }
  // Returns index of StringObject (i.e ObjectBlock) in BlockBase.
  int32_t getStringObjectIndex() { return StringObjectIndex; }
  // Returns index of BlockBase class in ReusableArenaBlock.
  int32_t getBlockBaseObjIndex() { return BlockBaseObjIndex; }
  // Returns index of FirstFreeBlockIndex in ReusableArenaBlock.
  int32_t getFirstFreeBlockIndex() { return FirstFreeBlockIndex; }
  // Returns index of NextFreeBlockIndex in ReusableArenaBlock.
  int32_t getNextFreeBlockIndex() { return NextFreeBlockIndex; }
  // Returns index of ReusableArenaBlock in ListNode.
  int32_t getReusableArenaBlockIndex() { return ReusableArenaBlockIndex; }
  // Returns index of NodePrevIndex in ListNode.
  int32_t getNodePrevIndex() { return NodePrevIndex; }
  // Returns index of NodeNextIndex in ListNode.
  int32_t getNodeNextIndex() { return NodeNextIndex; }

private:
  Module &M;

  // Candidate struct that uses Allocator class to allocate
  // objects.
  StructType *StringAllocatorType = nullptr;

  // Memory Interface class: Used to allocate/deallocate memory.
  StructType *MemInterfaceType = nullptr;

  // Object type that is allocated/deallocated by the Allocator.
  StructType *StringObjectType = nullptr;

  // Class that implements the functionality of Allocator.
  // Member functions of this class are called in "StringAllocatorType".
  StructType *ReusableArenaAllocatorType = nullptr;

  // Base class of "ReusableArenaAllocatorType". Member functions
  // of this class are also called in "StringAllocatorType".
  StructType *ArenaAllocatorType = nullptr;

  // Type of Node class.
  StructType *ListNodeType = nullptr;

  // Type of ReusableArenaBlockType class.
  StructType *ReusableArenaBlockType = nullptr;

  // Type of BlockBaseType class.
  StructType *BlockBaseType = nullptr;

  // Type of List class.
  StructType *ListType = nullptr;

  // Member functions of StringAllocatorType.
  SmallPtrSet<Function *, 8> StringAllocatorFunctions;

  // Member functions of StringObjectType that are used by
  // StringAllocatorType.
  SmallPtrSet<Function *, 8> StringObjectFunctions;

  // Member functions of ArenaAllocatorType and ReusableArenaAllocatorType
  // that are called from StringAllocatorType.
  SmallPtrSet<Function *, 8> AllocatorInterfaceFunctions;

  // Functions that are called to implement AllocatorInterfaceFunctions
  // functions.
  std::set<Function *> AllocatorInnerFunctions;

  // Function Calls that are called to implement AllocatorInterfaceFunctions
  // functions.
  std::set<const CallBase *> AllocatorInnerCalls;

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

  inline bool isBasicAllocatorType(Type *Ty);
  inline bool isBlockBaseType(Type *Ty);
  inline bool isReusableArenaBlockType(Type *Ty);
  inline bool isListNodeType(Type *Ty);
  inline bool isListType(Type *Ty);
  inline bool isArenaAllocatorType(Type *Ty);
  inline bool isReusableArenaAllocatorType(Type *Ty);
  inline bool isStringAllocatorType(Type *Ty);
  inline bool isStructWithNoRealData(Type *);
  inline bool isStringObjectType(Type *);
};

// Returns true if 'Ty' is a struct that doesn't have any real data
// except vftable.
// Ex:
//      %"MemoryManager" = type { i32 (...)** }
//
bool MemManageCandidateInfo::isStructWithNoRealData(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy || STy->getNumElements() > 1)
    return false;
  if (STy->getNumElements() == 1 && !isVFTablePointer(STy->getElementType(0)))
    return false;
  if (!MemInterfaceType)
    MemInterfaceType = STy;
  else if (MemInterfaceType != STy)
    return false;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
// %"Allocator" = type { %"MemoryManager"* }
//
bool MemManageCandidateInfo::isBasicAllocatorType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  if (STy->getNumElements() != 1)
    return false;
  auto *MTy = getPointeeType(STy->getElementType(0));
  if (!MTy)
    return false;
  if (!isStructWithNoRealData(MTy))
    return false;
  return true;
}

// Returns true if 'Ty' is a pointer to a struct type that looks like below.
// Ex:
// %"StringObject" = type { %"StringBase", %"GetAndReleaseCachedString" }
// %"StringBase" = type { %"XObject", double, %"XObjectResultTreeFragProxy" }
// %"GetAndReleaseCachedString" = type { %"XPathExecutionContext"*,
//                                       %"XalanDOMString"* }
//
bool MemManageCandidateInfo::isStringObjectType(Type *Ty) {
  auto *PTy = getPointeeType(Ty);
  if (!PTy)
    return false;
  auto *StructTy = dyn_cast<StructType>(PTy);
  if (!StructTy)
    return false;
  const DataLayout &DL = M.getDataLayout();
  uint64_t Size = DL.getTypeAllocSize(StructTy);
  if (Size < MinStringObjectSize)
    return false;
  for (auto *ETy : StructTy->elements())
    if (!isa<StructType>(ETy))
      return false;

  // Heuristics to avoid other allocators that look like StringAllocator
  // candidate. We are interested in only the allocator that allocates
  // memory for StringObject types.
  unsigned NoElem = StructTy->getNumElements();
  if (NoElem != NumStringObjectElems)
    return false;
  auto *LastElem = cast<StructType>(StructTy->getElementType(1));
  if (LastElem->getNumElements() != NumStringObjectElems)
    return false;
  // Allow only pointers as fields.
  for (auto *ETy : LastElem->elements())
    if (!isa<PointerType>(ETy))
      return false;

  auto *FirstElem = cast<StructType>(StructTy->getElementType(0));
  if (FirstElem->getNumElements() != NumStringObjectElems + 1)
    return false;
  // Don't allow any other field except struct/double type fields.
  for (auto *ETy : FirstElem->elements())
    if (!isa<StructType>(ETy) && !ETy->isDoubleTy())
      return false;

  StringObjectType = StructTy;
  return true;
}

// Returns true if 'Ty' is a struct that looks like format below.
// Ex:
// %"BlockBase" = type { %"Allocator", i16, i16, %"StringCached"* }
//
bool MemManageCandidateInfo::isBlockBaseType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;
  unsigned NumCounters = 0;
  unsigned NumBasicAllocatons = 0;
  unsigned NumStringPtrs = 0;
  int32_t FieldCount = -1;
  for (auto *ETy : STy->elements()) {
    FieldCount++;
    if (ETy->isIntegerTy(16)) {
      NumCounters++;
      // For now, assume first i16 field as ObjectCount and the second
      // i16 field as BlockSize.
      if (BlockObjectCountIndex == -1)
        BlockObjectCountIndex = FieldCount;
      else if (BlockBlockSizeIndex == -1)
        BlockBlockSizeIndex = FieldCount;
      else
        return false;
      continue;
    }
    if (isBasicAllocatorType(ETy)) {
      NumBasicAllocatons++;
      BasicAllocatorIndex = FieldCount;
      continue;
    }
    if (isStringObjectType(ETy)) {
      NumStringPtrs++;
      StringObjectIndex = FieldCount;
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
// %"ReusableArenaBlock" = type { %"BlockBase", i16, i16, [4 x i8] }
//
bool MemManageCandidateInfo::isReusableArenaBlockType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumCounters = 0;
  unsigned NumBlockBase = 0;
  unsigned NumUnused = 0;
  int32_t FieldCount = -1;
  for (auto *ETy : STy->elements()) {
    FieldCount++;
    if (isPotentialPaddingField(ETy)) {
      NumUnused++;
      continue;
    }
    if (ETy->isIntegerTy(16)) {
      NumCounters++;
      // For now, assume first i16 field as FirstFreeBlock and the second
      // i16 field as NextFreeBlock.
      if (FirstFreeBlockIndex == -1)
        FirstFreeBlockIndex = FieldCount;
      else if (NextFreeBlockIndex == -1)
        NextFreeBlockIndex = FieldCount;
      else
        return false;
      continue;
    }
    if (isBlockBaseType(ETy)) {
      NumBlockBase++;
      BlockBaseObjIndex = FieldCount;
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
// %"ListNode" = type { %"ReusableArenaBlock"*, %"ListNode"*, %"ListNode"* }
//
bool MemManageCandidateInfo::isListNodeType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumListNodePtrs = 0;
  unsigned NumReusableArenaBlock = 0;
  int32_t FieldCount = -1;
  for (auto *ETy : STy->elements()) {
    auto *PTy = getPointeeType(ETy);
    if (!PTy)
      return false;
    FieldCount++;
    if (PTy == Ty) {
      NumListNodePtrs++;
      // For now, assume first NodeTy field as NodePrev and the second
      // NodeTy field as NodeNext.
      if (NodePrevIndex == -1)
        NodePrevIndex = FieldCount;
      else if (NodeNextIndex == -1)
        NodeNextIndex = FieldCount;
      else
        return false;
      continue;
    }
    if (isReusableArenaBlockType(PTy)) {
      NumReusableArenaBlock++;
      ReusableArenaBlockIndex = FieldCount;
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
// %"List" = type { %"MemoryManager"*, %"ListNode"*, %"ListNode"* }
//
bool MemManageCandidateInfo::isListType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumListNodePtrs = 0;
  unsigned NumMemInt = 0;
  Type *NodeTy = nullptr;
  int32_t FieldCount = -1;
  for (auto *ETy : STy->elements()) {
    auto *PTy = getPointeeType(ETy);
    if (!PTy)
      return false;
    FieldCount++;
    if (isStructWithNoRealData(PTy)) {
      NumMemInt++;
      ListMemManagerIndex = FieldCount;
      continue;
    }
    if (NodeTy) {
      if (NodeTy != PTy)
        return false;
      NumListNodePtrs++;
      ListFreeHeadIndex = FieldCount;
      continue;
    } else if (isListNodeType(PTy)) {
      NodeTy = PTy;
      NumListNodePtrs++;
      // For now, assume first NodeTy pointer as ListHead and the second
      // NodeTy pointer as ListFreeHead.
      ListHeadIndex = FieldCount;
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
// %"ArenaAllocator" = type { i32 (...)**, i16, %"XalanList" }
//
bool MemManageCandidateInfo::isArenaAllocatorType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumVFTablePtrs = 0;
  unsigned NumLists = 0;
  unsigned NumInts = 0;
  int32_t FieldCount = -1;
  for (auto *ETy : STy->elements()) {
    FieldCount++;
    if (isVFTablePointer(ETy)) {
      // Expect VFTable as first field always.
      if (FieldCount != 0)
        return false;
      NumVFTablePtrs++;
      continue;
    }
    if (isListType(ETy)) {
      NumLists++;
      ListObjectIndex = FieldCount;
      continue;
    }
    if (ETy->isIntegerTy(16)) {
      NumInts++;
      AllocatorBlockSizeIndex = FieldCount;
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
// %"ReusableArenaAllocator" = type { %"ArenaAllocator", i8, [7 x i8] }
//
//   or
//
// %"ReusableArenaAllocator" = type { %"ArenaAllocator", i8 }
//
bool MemManageCandidateInfo::isReusableArenaAllocatorType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumArenaAllocator = 0;
  unsigned NumFlags = 0;
  unsigned NumUnused = 0;
  int32_t FieldCount = -1;
  for (auto *ETy : STy->elements()) {
    FieldCount++;
    if (isPotentialPaddingField(ETy)) {
      NumUnused++;
      continue;
    }
    if (isArenaAllocatorType(ETy)) {
      NumArenaAllocator++;
      ArenaAllocatorObjectIndex = FieldCount;
      continue;
    }
    if (ETy->isIntegerTy(8)) {
      NumFlags++;
      DestroyBlockFlagIndex = FieldCount;
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
// %"StringAllocator" = type { %"ReusableArenaAllocator" }
//
bool MemManageCandidateInfo::isStringAllocatorType(Type *Ty) {
  auto *STy = getValidStructTy(Ty);
  if (!STy)
    return false;

  unsigned NumReusableArenaAllocator = 0;
  for (auto *ETy : STy->elements()) {
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
bool MemManageCandidateInfo::isCandidateType(Type *Ty) {
  if (!isStringAllocatorType(Ty))
    return false;
  return true;
}

// Collect member functions that are related to candidate.
// 1. All member functions of StringAllocator
// 2. StringObject functions that are used by StringAllocator
// 3. Main interface functions of Allocator that are used
//    by StringAllocator.
// 4. All inner functions that are used to implement the interface
//    functions.
bool MemManageCandidateInfo::collectMemberFunctions(bool AtLTO) {

  std::function<bool(Function * F, bool AtLTO,
                     SmallPtrSet<Function *, 32> &ProcessedFuncs)>
      CollectMemberFunctions;

  // Add "F" to AllocatorInnerFunctions if "F" is not in
  // AllocatorInterfaceFunctions/StringObjectFunctions/
  // StringAllocatorFunctions.
  // Returns true if "F" is added to AllocatorInnerFunctions.
  auto CheckInlMemberFunction = [&](Function *F) -> bool {
    if (!F)
      return false;
    // Check if "F" is candidate for NoInlFuncsForDtrans.
    if (AllocatorInterfaceFunctions.count(F) ||
        StringObjectFunctions.count(F) || StringAllocatorFunctions.count(F))
      return false;

    AllocatorInnerFunctions.insert(F);
    return true;
  };

  // Recursively walks CallGraph to detect all functions called from "F".
  CollectMemberFunctions =
      [this, &CollectMemberFunctions, &CheckInlMemberFunction](
          Function *F, bool AtLTO,
          SmallPtrSet<Function *, 32> &ProcessedFunctions) -> bool {
    if (!F || F->isDeclaration())
      return true;

    // Check if it is already processed.
    if (!ProcessedFunctions.insert(F).second)
      return true;
    for (const auto &I : instructions(F))
      if (auto *CB = dyn_cast<CallBase>(&I)) {
        if (isa<IntrinsicInst>(*CB))
          continue;
        auto *Callee = dtrans::getCalledFunction(*CB);
        // At LTO, only direct calls are expected in the member functions.
        if (AtLTO && !Callee) {
          DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
            dbgs() << "  Failed: No indirect call is allowed.\n";
          });
          return false;
        }
        if (!CheckInlMemberFunction(Callee))
          continue;
        if (AtLTO)
          AllocatorInnerCalls.insert(CB);

        if (!CollectMemberFunctions(Callee, AtLTO, ProcessedFunctions))
          return false;
      }
    return true;
  };

  // Collect member functions of StringAllocatorType.
  for (auto &F : M) {
    auto *ThisTy = getThisClassType(&F);
    if (!ThisTy)
      continue;
    if (StringAllocatorType == ThisTy)
      StringAllocatorFunctions.insert(&F);
  }
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
    dbgs() << "String Allocator functions:\n";
    for (auto *F : StringAllocatorFunctions)
      dbgs() << "    " << F->getName() << "\n";
  });
  if (StringAllocatorFunctions.size() > MaxNumStringAllocatorFuncs)
    return false;

  // Collect interface and StringObject functions.
  for (auto *F : StringAllocatorFunctions) {
    if (F->isDeclaration() || F->size() > MaxSizeStringAllocatorFunc)
      return false;
    for (const auto &I : instructions(F))
      if (auto *CB = dyn_cast<CallBase>(&I)) {
        if (isa<DbgInfoIntrinsic>(*CB))
          continue;
        if (CB->isLifetimeStartOrEnd())
          continue;
        auto *Callee = dtrans::getCalledFunction(*CB);
        if (!Callee)
          return false;
        auto *ThisTy = getThisClassType(Callee);
        if (!ThisTy)
          return false;
        if (ThisTy == ReusableArenaAllocatorType ||
            ThisTy == ArenaAllocatorType) {
          AllocatorInterfaceFunctions.insert(Callee);
        } else if (ThisTy == StringObjectType) {
          StringObjectFunctions.insert(Callee);
        } else {
          DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
            dbgs() << "  Failed: Unexpected call in StringAllocator\n";
          });
          return false;
        }
      }
  }
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
    dbgs() << "Allocator Interface functions:\n";
    for (auto *F : AllocatorInterfaceFunctions)
      dbgs() << "    " << F->getName() << "\n";
  });
  if (AllocatorInterfaceFunctions.size() > MaxNumAllocatorInterfaceFuncs)
    return false;

  // Collect StringObject functions.
  for (auto *F : AllocatorInterfaceFunctions) {
    if (F->isDeclaration() || F->isVarArg())
      return false;
    if (!AtLTO) {
      if (F->size() > MaxPreLTOSizeAllocatorInterfaceFunc)
        return false;
    } else {
      // After pre-LTO inlining, sizes of interface functions will be
      // increased at LTO.
      if (F->size() > MaxLTOSizeAllocatorInterfaceFunc)
        return false;
    }
    for (const auto &I : instructions(F))
      if (auto *CB = dyn_cast<CallBase>(&I)) {
        if (isa<DbgInfoIntrinsic>(*CB))
          continue;
        auto *Callee = dtrans::getCalledFunction(*CB);
        if (!Callee)
          continue;
        auto *ThisTy = getThisClassType(Callee);
        if (!ThisTy)
          continue;
        if (ThisTy == StringObjectType)
          StringObjectFunctions.insert(Callee);
      }
  }
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
    dbgs() << "String Object functions:\n";
    for (auto *F : StringObjectFunctions)
      dbgs() << "    " << F->getName() << "\n";
  });
  if (StringObjectFunctions.size() > MaxNumStringObjectFuncs)
    return false;
  if (AtLTO && StringObjectFunctions.size() != MaxNumStringObjectFuncs)
    return false;

  // Collect inner functions.
  SmallPtrSet<Function *, 32> ProcessedFunctions;
  for (auto *F : AllocatorInterfaceFunctions) {
    if (!CollectMemberFunctions(F, AtLTO, ProcessedFunctions)) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
        dbgs() << "  Failed: Collecting Allocator Inner function\n";
      });
      return false;
    }
  }
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO, {
    dbgs() << "Allocator Inner functions:\n";
    for (auto *F : AllocatorInnerFunctions)
      dbgs() << "    " << F->getName() << "\n";
  });
  if (!AtLTO) {
    if (AllocatorInnerFunctions.size() > MaxPreLTONumAllocatorInnerFuncs)
      return false;
  } else {
    // After pre-LTO inlining, number of calls is expected to be reduced
    // at LTO.
    if (AllocatorInnerFunctions.size() > MaxLTONumAllocatorInnerFuncs)
      return false;
  }
  return true;
}

bool MemManageCandidateInfo::collectInlineNoInlineMethods(
    std::set<Function *> *InlFuncsForDtrans,
    SmallSet<Function *, 16> *NoInlFuncsForDtrans) const {

  // First, check basic heuristics.
  unsigned TotalInlBBSize = 0;
  for (auto *F : AllocatorInnerFunctions) {
    unsigned FSize = F->size();
    unsigned FUse = F->getNumUses();
    if (FSize > MaxPreLTOInnerFuncSize || FUse > MaxPreLTONumInnerFuncUses) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO,
                      { dbgs() << "  Failed: size or use limit.\n"; });
      return false;
    }
    TotalInlBBSize += FSize * FUse;
  }
  if (TotalInlBBSize > MaxPreLTOInnerFuncTotalInlSize) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFO,
                    { dbgs() << "  Failed: Total inline size limit.\n"; });
    return false;
  }
  // All inner functions will be marked as "inline"
  for (auto *F : AllocatorInnerFunctions)
    InlFuncsForDtrans->insert(F);

  // All Interface/StringObject/StringALlocation functions will be
  // marked as "NoInline".
  for (auto *F : AllocatorInterfaceFunctions)
    NoInlFuncsForDtrans->insert(F);
  for (auto *F : StringObjectFunctions)
    NoInlFuncsForDtrans->insert(F);
  for (auto *F : StringAllocatorFunctions)
    NoInlFuncsForDtrans->insert(F);
  return true;
}

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_MEMMANAGEINFOIMPL_H
