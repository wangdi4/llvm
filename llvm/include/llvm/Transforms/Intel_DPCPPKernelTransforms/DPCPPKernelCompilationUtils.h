//===-- DPCPPKernelCompilationUtils.h - Function declarations -*- C++ -----===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __DPCPP_KERNEL_COMPILATION_UTILS_H_
#define __DPCPP_KERNEL_COMPILATION_UTILS_H_

#include "llvm/ADT/SetVector.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"

namespace llvm {
namespace DPCPPKernelCompilationUtils {

/// We use a SetVector to ensure determinstic iterations
using FuncSet = SetVector<Function *>;

/// This function can only be used when ToBB is Entry block.
void moveAllocaToEntry(BasicBlock *FromBB, BasicBlock *EntryBB);

void getAllSyncBuiltinsDecls(FuncSet &Set, Module *M);

Function *AddMoreArgsToFunc(Function *F, ArrayRef<Type *> NewTypes,
                            ArrayRef<const char *> NewNames,
                            ArrayRef<AttributeSet> NewAttrs, StringRef Prefix);

// AddMoreArgsToCall - Replaces a CallInst with a new CallInst which has the
// same arguments as orignal plus more args appeneded.
// Returns the new CallInst,
// OldC - the original CallInst to be replaced,
// NewArgs - New arguments to append to existing arguments,
// NewF - a suitable prototype of new Function to be called.
CallInst *AddMoreArgsToCall(CallInst *OldC, ArrayRef<Value *> NewArgs,
                            Function *NewF);

} // namespace DPCPPKernelCompilationUtils
} // namespace llvm

#endif // __DPCPP_KERNEL_COMPILATION_UTILS_H_
