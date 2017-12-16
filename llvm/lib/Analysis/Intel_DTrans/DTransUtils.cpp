//===------------ Intel_DTransUtils.cpp - Utilities for DTrans ------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file provides a set of utilities that are used by DTrans for both
/// analysis and optimization.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Analysis/Intel_DTrans/DTrans.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;
using namespace dtrans;

AllocKind dtrans::getAllocFnKind(Function *F, const TargetLibraryInfo &TLI) {
  // TODO: Make this implementation more comprehensive.
  if (!F)
    return AK_NotAlloc;
  LibFunc LF;
  if (!TLI.getLibFunc(*F, LF))
    return AK_NotAlloc;
  switch (LF) {
  default:
    return AK_NotAlloc;
  case LibFunc_malloc:
    return AK_Malloc;
  case LibFunc_calloc:
    return AK_Calloc;
  case LibFunc_realloc:
    return AK_Realloc;
  }
  llvm_unreachable("Unexpected continuation past LibFunc switch.");
}

void dtrans::getAllocSizeArgs(AllocKind Kind, CallInst *CI,
                              Value* &AllocSizeVal, Value* &AllocCountVal) {
  assert(Kind != AK_NotAlloc && Kind != AK_UserAlloc &&
         "Unexpected alloc kind passed to getAllocSizeArgs");

  if (Kind == AK_Malloc) {
    AllocSizeVal = CI->getArgOperand(0);
    AllocCountVal = nullptr;
    return;
  }

  if (Kind == AK_Calloc) {
    AllocSizeVal = CI->getArgOperand(0);
    AllocCountVal = CI->getArgOperand(1);
    return;
  }

  if (Kind == AK_Realloc) {
    AllocSizeVal = CI->getArgOperand(1);
    AllocCountVal = nullptr;
    return;
  }

  llvm_unreachable("Unexpected alloc kind passed to getAllocSizeArgs");
}

// This function is called to determine if a bitcast to the specified
// destination type could be used to access element 0 of the source type.
// If the destination type is a pointer type whose element type is the same
// as the type of element zero of the aggregate pointed to by the source
// pointer type, then it would be a valid element zero access.
//
// For example, consider:
//
//   %struct.S1 = type { %struct.S2, i32 }
//   ...
//   %p = bitcast %struct.S1* to %struct.S2*
//
// Because element zero of %struct.S1 has %struct.S2 as a type, this is a
// safe cast that accesses that element. Notice that the pointer to the
// element has a different level of indirection than the declaration of the
// element within the structure. This is expected because the element is
// accessed through a pointer to that element.
//
// Also, consider this example of an unsafe cast.
//
//   %struct.S3 = type { %struct.S4*, i32 }
//   ...
//   %p = bitcast %struct.S3* to %struct.S4*
//
// In this case, element zero of the %struct.S3 type is a pointer, %struct.S4*
// so the correct cast to access that element would be:
//
//   %p = bitcast %struct.S3* to %struct.S4**
//
// Element zero can also be access by casting an i8* pointer that is known
// to point to a given structure to a pointer to element zero of that type.
// However, the caller must handle that case by obtaining the necessary
// type alias information and calling this function with the known alias as
// the SrcTy argument.
bool dtrans::isElementZeroAccess(llvm::Type *SrcTy, llvm::Type *DestTy) {
  if (!DestTy->isPointerTy() || !SrcTy->isPointerTy())
    return false;
  llvm::Type *SrcPointeeTy = SrcTy->getPointerElementType();
  llvm::Type *DestPointeeTy = DestTy->getPointerElementType();
  // This will handle vector types, in addition to structs and arrays,
  // but I don't think we'd get here with a vector type (unless we end
  // up wanting to track vector types).
  if (auto *CompTy = dyn_cast<CompositeType>(SrcPointeeTy)) {
    auto *ElementZeroTy = CompTy->getTypeAtIndex(0u);
    if (DestPointeeTy == ElementZeroTy)
      return true;
    // If element zero is an aggregate type, this cast might be accessing
    // element zero of the nested type.
    if (ElementZeroTy->isAggregateType())
      return isElementZeroAccess(ElementZeroTy->getPointerTo(), DestTy);
    // Otherwise, it must be a bad cast. The caller should handle that.
    return false;
  }
  return false;
}

void dtrans::TypeInfo::printSafetyData() {
  outs() << "  Safety data: ";
  if (SafetyInfo == 0) {
    outs() << "No issues found\n";
    return;
  }
  // TODO: As safety checks are implemented, add them here.
  SafetyData ImplementedMask = dtrans::BadCasting | dtrans::BadAllocSizeArg |
                               dtrans::UnhandledUse;
  std::vector<StringRef> SafetyIssues;
  if (SafetyInfo & dtrans::BadCasting)
    SafetyIssues.push_back("Bad casting");
  if (SafetyInfo & dtrans::BadAllocSizeArg)
    SafetyIssues.push_back("Bad alloc size");
  if (SafetyInfo & dtrans::UnhandledUse)
    SafetyIssues.push_back("Unhandled use");
  // Print the safety issues found
  size_t NumIssues = SafetyIssues.size();
  for (size_t i = 0; i < NumIssues; ++i) {
    outs() << SafetyIssues[i];
    if (i != NumIssues - 1) {
      outs() << " | ";
    }
  }

  // TODO: Make this unnecessary.
  if (SafetyInfo & ~ImplementedMask) {
    outs() << " + other issues that need format support ("
           << (SafetyInfo & ~ImplementedMask) << ")";
    outs() << "\nImplementedMask = " << ImplementedMask;
  }

  outs() << "\n";
}
