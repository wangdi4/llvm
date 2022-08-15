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

#include "Intel_DTrans/Analysis/DTransLibraryInfo.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/Transforms/ClassInfoOPUtils.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"

#define DTRANS_MEMMANAGEINFOOP "dtrans-memmanageinfoop"

namespace llvm {

namespace dtransOP {

// This is a helper class resolve a function type. Currently, it supports
// looking up the type based on metadata, or by using internal table that is
// used to describe intrinsics and library functions.
class FunctionTypeResolver {
public:
  FunctionTypeResolver(TypeMetadataReader &MDReader,
                       DTransLibraryInfo &DTransLibInfo)
      : MDReader(MDReader), DTransLibInfo(DTransLibInfo) {}

  inline DTransFunctionType *getFunctionType(const Function *F) const;

private:
  TypeMetadataReader &MDReader;
  DTransLibraryInfo &DTransLibInfo;
};

DTransFunctionType *
FunctionTypeResolver::getFunctionType(const Function *F) const {
  auto *DFnTy =
      dyn_cast_or_null<DTransFunctionType>(MDReader.getDTransTypeFromMD(F));
  if (DFnTy)
    return DFnTy;

  DFnTy = DTransLibInfo.getDTransFunctionType(F);
  if (DFnTy)
    return DFnTy;

  return nullptr;
}

// This is similar to the function getClassType() in ClassInfoOPUtils.cpp, but
// the return value indicates whether type returned is 'nullptr' due to arg 0
// being an unknown type, or whether it is nullptr because the type of arg 0 is
// not a pointer to structure type.
//
// If the argument type is unknown due to the function type not being able to be
// determined, such as if the DTrans metadata is not available, the return value
// will be: {null, true}.
//
// However, if the type returned is nullptr, and the type is known not to be a
// pointer to a structure type (or if there is no argument 0) then the return
// value will be: {null, false}
//
// When the type of argument 0 is a pointer to a structure, the return value
// will be: {StructType, false}
static std::pair<DTransStructType *, bool>
getClassTypeOrUnknown(const Function *F, FunctionTypeResolver &TypeResolver) {
  if (F->arg_size() < 1)
    return {nullptr, false};

  // We can ignore intrinsics for this analysis because they will not be using
  // the class types. This is necessary to handle the 'fakeload' intrinsic which
  // takes a pointer to the class and returns it.
  if (F->isIntrinsic())
    return {nullptr, false};

  llvm::FunctionType *Ty = F->getFunctionType();
  if (!Ty->getParamType(0)->isPointerTy())
    return {nullptr, false};

  // Get the class type of the "this" pointer that is used in argument 0.
  DTransFunctionType *DFnTy = TypeResolver.getFunctionType(F);
  if (!DFnTy)
    return {nullptr, true};

  DTransType *ArgTy = DFnTy->getArgType(0);
  if (!ArgTy)
    return {nullptr, true};

  if (auto *PTy = dyn_cast<DTransPointerType>(ArgTy))
    if (auto *STy = dyn_cast<DTransStructType>(PTy->getPointerElementType()))
      return {STy, false};
  return {nullptr, false};
}

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

  // Min Limit: StringObject size.
  constexpr static int MinStringObjectSize = 64;

  // Limit: Number of StringObject elements.
  constexpr static int NumStringObjectElems = 2;

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

public:
  using InterfaceSetTy = SmallPtrSet<Function *, 8>;
  using InnerCallSetTy = std::set<const CallBase *>;

  MemManageCandidateInfo(Module &M) : M(M){};

  inline bool isCandidateType(DTransType *Ty);
  inline bool collectMemberFunctions(FunctionTypeResolver &TypeResolver,
                                     bool AtLTO);
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

  // Returns StringObjectType.
  DTransStructType *getStringObjectType() { return StringObjectType; }

  // Returns ReusableArenaAllocatorType.
  DTransStructType *getReusableArenaAllocatorType() {
    return ReusableArenaAllocatorType;
  }

  // Returns ArenaAllocatorType.
  DTransStructType *getArenaAllocatorType() { return ArenaAllocatorType; }

  // Returns MemInterfaceType.
  DTransStructType *getMemInterfaceType() { return MemInterfaceType; }

  // Returns StringAllocatorType.
  DTransStructType *getStringAllocatorType() { return StringAllocatorType; }

  // Returns index of ArenaAllocatorObject.
  int32_t getArenaAllocatorObjectIndex() { return ArenaAllocatorObjectIndex; }

  // Returns index of List.
  int32_t getListObjectIndex() { return ListObjectIndex; }

  // Returns index of BlockSize.
  int32_t getAllocatorBlockSizeIndex() { return AllocatorBlockSizeIndex; }

  // Returns index of MemManager in List
  int32_t getListMemManagerIndex() { return ListMemManagerIndex; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { print(llvm::dbgs()); }

  void print(raw_ostream &OS) const {
    OS << "MemManageCandidateInfo:\n";
    OS << "  StringAllocatorType        : " << *StringAllocatorType << "\n";
    OS << "  MemInterfaceType           : " << *MemInterfaceType << "\n";
    OS << "  StringObjectType           : " << *StringObjectType << "\n";
    OS << "  ReusableAreanaAllocatorType: " << *ReusableArenaAllocatorType
       << "\n";
    OS << "    ArenaAllocatorObjectIndex = " << ArenaAllocatorObjectIndex
       << "\n";
    OS << "    DestroyBlockFlagIndex     = " << DestroyBlockFlagIndex << "\n";
    OS << "  ArenaAllocatorType         : " << *ArenaAllocatorType << "\n";
    OS << "    ListObjectIndex           = " << ListObjectIndex << "\n";
    OS << "    AllocatorBlockSizeIndex   = " << AllocatorBlockSizeIndex << "\n";
    OS << "  ListNodeType               : " << *ListNodeType << "\n";
    OS << "    ReusableArenaBlockIndex   = " << ReusableArenaBlockIndex << "\n";
    OS << "    NodePrevIndex             = " << NodePrevIndex << "\n";
    OS << "    NodeNextIndex             = " << NodeNextIndex << "\n";
    OS << "  ReusableArenaBlockType     : " << *ReusableArenaBlockType << "\n";
    OS << "    BlockBaseObjIndex         = " << BlockBaseObjIndex << "\n";
    OS << "    FirstFreeBlockIndex       = " << FirstFreeBlockIndex << "\n";
    OS << "    NextFreeBlockIndex        = " << NextFreeBlockIndex << "\n";
    OS << "  BlockBaseType              : " << *BlockBaseType << "\n";
    OS << "    BasicAllocatorIndex       = " << BasicAllocatorIndex << "\n";
    OS << "    BlockObjectCountIndex     = " << BlockObjectCountIndex << "\n";
    OS << "    BlockBlockSizeIndex       = " << BlockBlockSizeIndex << "\n";
    OS << "    StringObjectIndex         = " << StringObjectIndex << "\n";
    OS << "  ListType                   : " << *ListType << "\n";
    OS << "    ListMemManagerIndex       = " << ListMemManagerIndex << "\n";
    OS << "    ListHeadIndex             = " << ListHeadIndex << "\n";
    OS << "    ListFreeHeadIndex         = " << ListFreeHeadIndex << "\n";
    OS << "\n";
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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

// Collect member functions that are related to candidate.
// 1. All member functions of StringAllocator
// 2. StringObject functions that are used by StringAllocator
// 3. Main interface functions of Allocator that are used
//    by StringAllocator.
// 4. All inner functions that are used to implement the interface
//    functions.
bool MemManageCandidateInfo::collectMemberFunctions(
    FunctionTypeResolver &TypeResolver, bool AtLTO) {

  std::function<bool(Function *, bool AtLTO,
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
          DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
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
    DTransStructType *ThisTy = nullptr;
    bool UnknownType = false;
    std::tie(ThisTy, UnknownType) = getClassTypeOrUnknown(&F, TypeResolver);
    if (UnknownType) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
        dbgs() << "  Failed: Unknown type for argument 0: " << F.getName()
               << "\n";
      });
      return false;
    }

    if (!ThisTy)
      continue;
    if (StringAllocatorType == ThisTy)
      StringAllocatorFunctions.insert(&F);
  }
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
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

        DTransStructType *ThisTy = nullptr;
        bool UnknownType = false;
        std::tie(ThisTy, UnknownType) =
            getClassTypeOrUnknown(Callee, TypeResolver);
        if (!ThisTy)
          return false;

        if (ThisTy == ReusableArenaAllocatorType ||
            ThisTy == ArenaAllocatorType) {
          AllocatorInterfaceFunctions.insert(Callee);
        } else if (ThisTy == StringObjectType) {
          StringObjectFunctions.insert(Callee);
        } else {
          DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
            dbgs() << "  Failed: Unexpected call in StringAllocator\n";
          });
          return false;
        }
      }
  }
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
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

    for (const auto &I : instructions(F)) {
      if (auto *CB = dyn_cast<CallBase>(&I)) {
        if (isa<DbgInfoIntrinsic>(*CB))
          continue;
        auto *Callee = dtrans::getCalledFunction(*CB);
        if (!Callee)
          continue;

        DTransStructType *ThisTy = nullptr;
        bool UnknownType = false;
        std::tie(ThisTy, UnknownType) =
            getClassTypeOrUnknown(Callee, TypeResolver);
        if (UnknownType) {
          DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
            dbgs() << "  Failed: Unknown type for argument 0: "
                   << Callee->getName() << "\n";
          });
          return false;
        }

        if (!ThisTy)
          continue;
        if (ThisTy == StringObjectType)
          StringObjectFunctions.insert(Callee);
      }
    }
  }
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
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
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
        dbgs() << "  Failed: Collecting Allocator Inner function\n";
      });
      return false;
    }
  }
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP, {
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
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP,
                      { dbgs() << "  Failed: size or use limit.\n"; });
      return false;
    }
    TotalInlBBSize += FSize * FUse;
  }

  if (TotalInlBBSize > MaxPreLTOInnerFuncTotalInlSize) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGEINFOOP,
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

} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_MEMMANAGEINFOOPIMPL_H
