//===- LinearIdResolver.h - DPC++ linear id resolver ----------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LINEAR_ID_RESOLVER_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LINEAR_ID_RESOLVER_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class CallGraph;
class IRBuilderBase;

/// Replaces opencl 2.0 work item functions get_{global,local}_linear_id with
/// their explicit calculation using get_{global,local}_id,
/// get_{global,local}_size and get_global_offset.
class LinearIdResolverPass : public PassInfoMixin<LinearIdResolverPass> {
public:
  explicit LinearIdResolverPass() {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, CallGraph *CG);

  static bool isRequired() { return true; }

private:
  /// Generate calculation sequence of get_local_linear_id and replace.
  /// \param M module.
  /// \param CI get_global_linear_id CallInst to be replaced.
  void replaceGetLocalLinearId(Module *M, CallInst *CI);

  /// Generate calculation sequence of get_global_linear_id and replace.
  /// \param M module.
  /// \param CI get_global_linear_id CallInst to be replaced.
  void replaceGetGlobalLinearId(Module *M, CallInst *CI);

  /// Create call instruction to a given builtin function.
  /// \param M module.
  /// \param Builder IRBuilder instance.
  /// \param FuncName name of the builtin function to call.
  /// \param Args builtin arguments.
  /// \param NameStr SSA register twine.
  /// \param InsertBefore insert before instruction.
  CallInst *createWIFunctionCall(Module *M, IRBuilderBase &Builder,
                                 StringRef FuncName, Value *Args,
                                 StringRef NameStr);

  /// Constant values used by function calls.
  Value *Zero = nullptr;
  Value *One = nullptr;
  Value *Two = nullptr;

  /// Return type for work-item functions.
  Type *RetTy = nullptr;
};

} // namespace llvm

#endif
