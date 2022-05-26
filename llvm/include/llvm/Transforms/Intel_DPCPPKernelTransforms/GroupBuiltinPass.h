//===--- GroupBuiltinPass.h - Process WorkGroup Builtins --------*- C++ -*-===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_GROUP_BUILTIN_PASS_H
#define LLVM_TRANSFORMS_INTEL_GROUP_BUILTIN_PASS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/BarrierUtils.h"

namespace llvm {

class BuiltinLibInfo;
class RuntimeService;

/// \brief GroupBuiltinHandler pass is a module pass that handles calls to
/// group built-ins instructions, e.g. async_copy, etc.
/// It provides that their execution will be synchronized across all WIs
class GroupBuiltinPass : public PassInfoMixin<GroupBuiltinPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M, BuiltinLibInfo *BLI);

  explicit GroupBuiltinPass(ArrayRef<Module *> BuiltinModuleList = {}){};

private:
  /// This module
  Module *M;

  /// Runtime service.
  RuntimeService *RTService;

  /// This context
  LLVMContext *Context;

  /// size_t type
  IntegerType *SizeT;

  /// This is barrier utility class
  BarrierUtils Utils;

  /// Generate initialization value for a WG function
  Constant *getInitializationValue(Function *Func);

  /// Implement call to get_local_linear_id().
  Instruction *getLinearIDForBroadcast(CallInst *WGCallInstr);

  /// Generate linear ID out of ID indices
  Value *calculateLinearIDForBroadcast(CallInst *WGCallInstr);

  /// Helper for WI function call generation.
  /// Generates a call to WI function upon its name and dimension index
  CallInst *getWICall(Instruction *Before, StringRef FuncName, unsigned DimIdx);
};

class GroupBuiltinLegacy : public ModulePass {
  GroupBuiltinPass Impl;

public:
  static char ID;

  /// \brief C'tor
  explicit GroupBuiltinLegacy(ArrayRef<Module *> BuiltinModuleList = {});

  /// \brief Provides name of pass
  StringRef getPassName() const override {
    return "GroupBuiltin";
  }

  /// \brief execute pass on given module
  /// \param M module to optimize
  /// \returns True if module was modified
  bool runOnModule(Module &M) override;

  /// \brief Inform about usage/mofication/dependency of this pass
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_GROUP_BUILTIN_PASS_H
