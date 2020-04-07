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

#include "llvm/IR/BasicBlock.h"

namespace llvm {
namespace DPCPPKernelCompilationUtils {

/// This function can only be used when ToBB is Entry block.
void moveAllocaToEntry(BasicBlock *FromBB, BasicBlock *EntryBB);

} // namespace DPCPPKernelCompilationUtils
} // namespace llvm

#endif // __DPCPP_KERNEL_COMPILATION_UTILS_H_
