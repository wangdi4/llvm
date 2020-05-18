//==--- DPCPPKernelBarrierUtils.h - Barrier helper functions - C++ -*-------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_UTILS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_UTILS_H

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"

namespace llvm {
namespace DPCPPKernelBarrierUtils {

using InstVector = SmallVector<llvm::Instruction *, 8>;
using ValueVector = SmallVector<llvm::Value *, 8>;
using FuncVector = SmallVector<llvm::Function *, 8>;

using InstSet = SetVector<llvm::Instruction *>;
using FuncSet = SetVector<llvm::Function *>;

static constexpr const char BarrierName[] = "__builtin_dpcpp_kernel_barrier";
static constexpr const char DummyBarrierName[] =
    "__builtin_dpcpp_kernel_barrier_dummy";

/// Find all instructions in the module, which use function with the given name.
void findAllUsesOfFunc(llvm::Module &M, const llvm::StringRef Name,
                       InstSet &UsesSet);

/// Return all synchronize instructions in the module
/// Returns container with all synchronize instructions
void getAllSyncInstructions(llvm::Module &M, InstVector &SyncInsts);

/// Find all functions  in the module
/// that contain synchronize instructions
/// Returns FuncSet container with found functions
void getAllFunctionsWithSynchronization(llvm::Module &M, FuncSet &SyncFuncs);

} // namespace DPCPPKernelBarrierUtils
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BARRIER_UTILS_H
