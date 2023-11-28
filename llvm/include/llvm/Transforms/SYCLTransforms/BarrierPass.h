//==--- BarrierPass.h - Main Barrier pass - C++ -*--------------------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_BARRIER_PASS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_BARRIER_PASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Barrier pass is a module pass that handles
/// Special values
///   Group-A   : Alloca instructions
///   Group-B.1 : Values crossed barriers and the value is
///              related to WI-Id or initialized inside a loop
///   Group-B.2 : Value crossed barrier but does not suit Group-B.1
/// Synchronize instructions
///   barrier(), fiber() and dummyBarrier() instructions.
/// Get LID/GID instructions
///   get_local_id() will be replaced with get_new_local_id()
///   get_global_id() will be replaced with get_new_global_id()
/// Non Inlined Internal Function
///   module functions with barriers that are called from inside the module
class KernelBarrier : public PassInfoMixin<KernelBarrier> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_BARRIER_PASS_H
