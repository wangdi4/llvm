//===-----DTransAllocAnalyzer.h - Allocation/Free function analyzer--------===//
//
// Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTransAllocAnalyzer.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSALLOCANALYZER_H
#define INTEL_DTRANS_ANALYSIS_DTRANSALLOCANALYZER_H

#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include <map>

namespace llvm {

class TargetLibraryInfo;

namespace dtrans {

/// Kind of allocation associated with a Function.
/// The malloc, calloc, and realloc allocation kinds each correspond to a call
/// to the standard library function of the same name.
///
/// See MemoryBuiltins.cpp:AllocType
enum AllocKind : uint8_t {
  AK_NotAlloc,
  AK_Malloc,
  AK_Calloc,
  AK_Realloc,
  AK_UserMalloc,
  AK_UserMalloc0,
  AK_New
};

/// Kind of free function call.
/// - FK_Free represents a direct call to the standard library function 'free'
/// - FK_UserFree represents a call to a user-wrapper function of 'free''
/// - FK_Delete represents a call to C++ delete/deletep[] functions.
enum FreeKind { FK_NotFree, FK_Free, FK_UserFree, FK_Delete };

/// Get a printable string for the AllocKind
StringRef AllocKindName(AllocKind Kind);

/// Get a printable string for the FreeKind
StringRef FreeKindName(FreeKind Kind);

/// Determine whether the specified \p Call is a call to allocation function,
/// and if so what kind of allocation function it is and the size of the
/// allocation.
AllocKind getAllocFnKind(const CallBase *Call, const TargetLibraryInfo &TLI);

/// Get the indices of size and count arguments for the allocation call.
/// AllocCountInd is used for calloc allocations.  For all other allocation
/// kinds it will be set to -1U
void getAllocSizeArgs(AllocKind Kind, const CallBase *Call,
                      unsigned &AllocSizeInd, unsigned &AllocCountInd,
                      const TargetLibraryInfo &TLI);

/// Collects all special arguments for malloc-like call.
/// Elements are added to OutputSet.
/// Realloc-like functions have pointer argument returned in OutputSet.
void collectSpecialAllocArgs(AllocKind Kind, const CallBase *Call,
                             SmallPtrSet<const Value *, 3> &OutputSet,
                             const TargetLibraryInfo &TLI);

/// Determine whether or not the specified \p Call is a call to the free-like
/// library function.
bool isFreeFn(const CallBase *Call, const TargetLibraryInfo &TLI);

/// Determine whether or not the specified \p Call is a call to the
/// delete-like library function.
bool isDeleteFn(const CallBase *Call, const TargetLibraryInfo &TLI);

/// Returns the index of pointer argument for \p Call.
void getFreePtrArg(FreeKind Kind, const CallBase *Call, unsigned &PtrArgInd,
                   const TargetLibraryInfo &TLI);

/// Collects all special arguments for free-like call.
void collectSpecialFreeArgs(FreeKind Kind, const CallBase *Call,
                            SmallPtrSetImpl<const Value *> &OutputSet,
                            const TargetLibraryInfo &TLI);

// Class to analyze and identify functions that are post-dominated by
// a call to malloc() or free(). Those post-dominated by malloc() will
// yield true for isMallocPostDom().  Those post-dominated by free()
// will yield true for isFreePostDom().
//
// Functions whose prototype match malloc() and free() need not have their
// return value strictly post-dominated by malloc() or free(). Exceptions
// are made for "skip cases", like the following:
//
// (1) Function calling malloc() does nothing if size argument is 0:
//
//   extern void *mymalloc(int size) {
//     if (size != 0)
//       return malloc(size);
//     return nullptr;
//   }
//
// (2) Function returns immediately after call to malloc() if malloc()
//     returns a nullptr:
//
//   extern void *mymalloc(int size) {
//     char *ptr = malloc(size);
//     if (ptr == nullptr) {
//       printf("Warning!\n");
//       return nullptr;
//     }
//     return ptr;
//   }
//
// (3) Function calling free() does nothing if argument is nullptr:
//
//   extern void myfree(void *ptr) {
//     if (ptr != 0)
//       free(ptr);
//   }
//
// In cases (1) and (2), we will still return true for isMallocPostDom(),
// even through the call to malloc() does not post-dominate all returns.
// Similarly, in case (3), we wll still return true for isFreePostDom()
// even though not all paths to the return are post-dominated by a call
// to free().
//
class DTransAllocAnalyzer {
public:
  DTransAllocAnalyzer(
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      const Module &M);
  bool isMallocPostDom(const CallBase *Call);
  bool isFreePostDom(const CallBase *Call);
  void populateAllocDeallocTable(const Module &M);
  bool isMallocWithStoredMMPtr(const Function *F);
  bool isFreeWithStoredMMPtr(const Function *F);
  bool isUserAllocOrDummyFunc(const CallBase *Call);
  bool isUserFreeOrDummyFunc(const CallBase *Call);

private:
  typedef PointerIntPair<StructType *, 1, bool> PtrBoolPair;

  // An enum recording the status of a function. The status is
  // updated in populateAllocDeallocTable.
  enum AllocStatus { AKS_Unknown, AKS_Malloc, AKS_Free };

  // Mapping for the AllocStatus of each Function we have queried.
  std::map<const Function *, AllocStatus> LocalMap;

  // Offsets inside vtable.
  // Key is (pointer to some type, true = allocation/false = deallocation).

  std::map<PtrBoolPair, int32_t> VTableOffs;
  // A set to hold visited BasicBlocks.  This is a temporary set used
  // while we are determining the AllocStatus of a Function.

  SmallPtrSet<BasicBlock *, 20> VisitedBlocks;

  // A set to hold the BasicBlocks which do not need to be post-dominated
  // by malloc() to be considered isMallocPostDom() or free to be considered
  // isFreePostDom().
  SmallPtrSet<BasicBlock *, 4> SkipTestBlocks;

  // Needed to detrmine if a function is malloc()
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // Needed to check argument types
  PointerType *Int8PtrTy;

  bool isSkipTestBlock(BasicBlock *BB) const;
  bool isVisitedBlock(BasicBlock *BB) const;
  int skipTestSuccessor(BranchInst *BI) const;
  void visitAndSetSkipTestSuccessors(BasicBlock *BB);
  void visitAndResetSkipTestSuccessors(BasicBlock *BB);
  void visitNullPtrBlocks(Function *F);

  bool mallocBasedGEPChain(GetElementPtrInst *GV, GetElementPtrInst **GBV,
                           CallBase **GCI) const;
  bool mallocOffset(Value *V, int64_t *offset) const;
  bool mallocLimit(GetElementPtrInst *GBV, Value *V, int64_t Offset,
                   int64_t *Result) const;
  bool returnValueIsMallocAddress(Value *RV, BasicBlock *BB);
  bool analyzeForMallocStatus(Function *F);

  bool hasFreeCall(BasicBlock *BB) const;
  bool isPostDominatedByFreeCall(BasicBlock *BB, bool &IsFreeSeen);
  bool analyzeForFreeStatus(Function *F);

  bool analyzeForIndirectStatus(const CallBase *Call, bool Alloc);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void parseListOptions(const Module &M);
#endif
};

} // end namespace dtrans
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSALLOCANALYZER_H
