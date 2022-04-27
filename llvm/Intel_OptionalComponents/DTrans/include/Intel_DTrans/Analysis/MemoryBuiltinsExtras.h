//===----MemoryBuiltinsExtras.h -Extend MemoryBuiltins.h functionality-----===//
//
// Copyright (C) 2018-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides convenience functions to DTrans to extend the
// functionality of "llvm/Analysis/MemoryBuiltins.h" for DTrans to analyze
// calls to functions that allocate or free memory.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error MemoryBuiltinsExtras.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_MEMORY_BUILTINS_EXTRAS_H
#define INTEL_DTRANS_ANALYSIS_MEMORY_BUILTINS_EXTRAS_H

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Instructions.h"

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
  AK_UserMalloc,  // Allocation with just a size argument.
  AK_UserMalloc0, // Allocation with multiple arguments, size is first argument.
  AK_UserMallocThis, // Allocation with "this" pointer and size argument.
  AK_New
};

/// Kind of free function call.
/// - FK_Free         - a direct call to the standard library function 'free'
/// - FK_UserFree     - a call to a user-wrapper function of 'free' which
///                     just takes a single pointer argument.
/// - FK_UserFree0    - a call to a user-wrapper function of 'free' which
///                     takes multiple arguments, the pointer to free is the
///                     first argument.
/// - FK_UserFreeThis - a call to a user-wrapper function of 'free' which
///                     takes a 'this' pointer and a pointer to be freed.
/// - FK_Delete       - a call to C++ delete/delete[] functions.
enum FreeKind : uint8_t {
  FK_NotFree,
  FK_Free,
  FK_UserFree,
  FK_UserFree0,
  FK_UserFreeThis,
  FK_Delete
};

/// Get a printable string for the AllocKind
StringRef AllocKindName(AllocKind Kind);

/// Get a printable string for the FreeKind
StringRef FreeKindName(FreeKind Kind);

/// Return 'true' if 'Kind' is one of the user allocation types.
bool isUserAllocKind(AllocKind Kind);

/// Return 'true' if 'Kind' is one of the user free types.
bool isUserFreeKind(FreeKind Kind);

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

/// This is a helper function to analyze a series of  GetElementPtr/Pointer
/// arithmetic instructions to determine whether the address computed by the
/// GEP is the result of a memory allocation, and the amount the instruction
/// sequence offsets the allocation by is within some number of bytes that.the
/// allocation size argument of the user allocation routine was increased by.
/// Return true, if this can be proven.
bool analyzeGEPAsAllocationResult(GetElementPtrInst *GEP,
                                  const TargetLibraryInfo &TLI, CallBase **Call,
                                  dtrans::AllocKind *Kind);

} // end namespace dtrans
} // end namespace llvm
#endif // INTEL_DTRANS_ANALYSIS_MEMORY_BUILTINS_EXTRAS_H
