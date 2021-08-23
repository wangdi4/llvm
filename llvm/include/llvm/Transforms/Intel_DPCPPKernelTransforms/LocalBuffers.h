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

extern bool EnableTLSGlobals;

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

  /// @brief Replaces all uses of Constant `From` with `To`. We can't simply
  ///        call `From->replaceAllUsesWith` because the users of `From` may
  ///        be ConstantExpr, ConstantVector, ConstantStruct. We have to
  ///        convert those Constant uses to instructions first.
  /// @param From The constant value needs to be replaced
  /// @param To The replacement target value
  void replaceAllUsesOfConstantWith(Constant *From, Value *To);

  /// @brief Create instructions (GEP, insertvalue, etc.) to generate a Value
  ///        of same semantic with the original Constant `C`. Also replace
  ///        `From` with `To` during the instruction creation.
  /// @param C The constant to be converted as instructions
  /// @param From Should be one of the operands of `C`
  /// @return The generated Value which should be equivalent as `C`
  Value *createInstructionFromConstantWithReplacement(Constant *C, Value *From,
                                                      Value *To);

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

  /// @brief stores all the DIGlobalVariableExpression's need to be removed
  ///        in DICompileUnit.globals
  SmallPtrSet<DIGlobalVariableExpression *, 4> GVEToRemove;

  /// @brief stores all the GlobalVariable's need to be removed
  SmallPtrSet<GlobalVariable *, 4> GVToRemove;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LOCAL_BUFFERS_H
