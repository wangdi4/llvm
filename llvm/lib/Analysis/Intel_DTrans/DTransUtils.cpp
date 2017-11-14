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
