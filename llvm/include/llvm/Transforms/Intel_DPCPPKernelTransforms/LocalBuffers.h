//===- LocalBuffers.h - Map GlobalVariable __local to local buffer --------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LOCAL_BUFFERS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LOCAL_BUFFERS_H

#include "LocalBufferAnalysis.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

namespace llvm {

/// Map __local GlobalVariable to the local buffer.

class LocalBuffersPass : public PassInfoMixin<LocalBuffersPass> {
public:
  explicit LocalBuffersPass(bool UseTLSGlobals = false)
      : UseTLSGlobals(UseTLSGlobals) {}

  static StringRef name() { return "LocalBuffersPass"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, LocalBufferInfo *LBInfo);

private:
  /// @brief Resolves the internal local variables and map them to local buffer
  /// @param F The function which needs it handle its implicite local
  /// variables
  void runOnFunction(Function *F);

  /// @brief Resolves the internal local variables and map them to local buffer
  /// @param F The function which needs it handle its implicite local
  /// variables
  void parseLocalBuffers(Function *F, Value *LocalMem);

  /// @brief Copies DebugInfo of `GV` to Local Memory Buffer `LocalMem`,
  ///        with corresponding `offset`.
  void attachDebugInfoToLocalMem(GlobalVariable *GV, Value *LocalMem,
                                 unsigned offset);

  /// @brief At the end of this pass, the GlobalVariables (__local) DebugInfo
  ///        should be removed from DICompileUnit's "globals" field, so that
  ///        the created Local Debug Variables are visible to the debugger.
  void updateDICompileUnitGlobals();

private:
  /// @brief The llvm current processed module
  Module *M;

  /// @brief The llvm context
  LLVMContext *Context;

  /// @brief instance of LocalBuffAnalysis pass
  LocalBufferInfo *LBInfo;

  /// @brief use TLS globals instead of implicit arguments
  bool UseTLSGlobals;

  /// @brief save the first instruction as insert point for current function
  Instruction *InsertPoint;

  /// @brief the DISubprogram of current function
  ///        when this equals to `nullptr`, then no need to handle debug info
  DISubprogram *SP;

  /// @brief help to find all compile units in the module
  DebugInfoFinder DIFinder;

  /// @brief Store all kernels in the module
  DPCPPKernelCompilationUtils::FuncSet KernelsFunctionSet;

  /// @brief stores all the DIGlobalVariableExpression's need to be removed
  ///        in DICompileUnit.globals
  SmallPtrSet<DIGlobalVariableExpression *, 4> GVEToRemove;

  /// @brief stores all the GlobalVariable's need to be removed
  SmallPtrSet<GlobalVariable *, 4> GVToRemove;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LOCAL_BUFFERS_H
