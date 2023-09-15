//===- LocalBuffers.h - Map GlobalVariable __local to local buffer --------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_LOCAL_BUFFERS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_LOCAL_BUFFERS_H

#include "LocalBufferAnalysis.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

namespace llvm {

/// Map __local GlobalVariable to the local buffer.

class LocalBuffersPass : public PassInfoMixin<LocalBuffersPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, LocalBufferInfo *LBInfo);

  static bool isRequired() { return true; }

private:
  /// @brief Resolves the internal local variables and map them to local buffer
  /// @param F The function which needs it handle its implicite local
  /// variables
  void runOnFunction(Function *F);

  /// @brief Resolves the internal local variables and map them to local buffer
  /// @param F The function which needs it handle its implicite local
  /// variables
  void parseLocalBuffers(Function *F, Value *LocalMem);

  /// @brief Copies DebugInfo of `GV` to Local Memory Buffer address
  /// `LocalAddr`.
  void attachDebugInfoToLocalAddr(GlobalVariable *GV, Value *LocalAddr);

  /// @brief At the end of this pass, the GlobalVariables (__local) DebugInfo
  ///        should be removed from DICompileUnit's "globals" field, so that
  ///        the created Local Debug Variables are visible to the debugger.
  void updateDICompileUnitGlobals();

private:
  /// @brief The llvm current processed module
  Module *M = nullptr;

  /// @brief The llvm context
  LLVMContext *Context = nullptr;

  /// @brief instance of LocalBuffAnalysis pass
  LocalBufferInfo *LBInfo = nullptr;

  /// @brief has TLS globals instead of implicit arguments
  bool HasTLSGlobals = false;

  /// @brief save the first instruction as insert point for current function
  Instruction *InsertPoint = nullptr;

  /// @brief the DISubprogram of current function
  ///        when this equals to `nullptr`, then no need to handle debug info
  DISubprogram *SP = nullptr;

  /// @brief help to find all compile units in the module
  DebugInfoFinder DIFinder;

  /// @brief stores all the DIGlobalVariableExpression's need to be removed
  ///        in DICompileUnit.globals
  SmallPtrSet<DIGlobalVariableExpression *, 4> GVEToRemove;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_LOCAL_BUFFERS_H
