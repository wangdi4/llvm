//===- AddImplicitArgs.h - Add implicit arguments to DPC++ kernel  --------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_ADD_IMPLICIT_ARGS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_ADD_IMPLICIT_ARGS_H

#include "ImplicitArgsAnalysis.h"
#include "LocalBufferAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Add the implicit arguments to signature of all functions defined inside the
/// module.
class AddImplicitArgsPass : public PassInfoMixin<AddImplicitArgsPass> {
public:
  static StringRef name() { return "AddImplicitArgsPass"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, LocalBufferInfo *LBInfo, ImplicitArgsInfo *IAInfo,
               CallGraph *CG);

protected:
  /// Replaces the given function with a copy function that receives the
  /// implicit arguments, maps call instructions that appear in the given
  /// funciton and need to add implicit arguments to its original arguments,
  /// i.e. calls to functions define in module.
  /// \param F the function to create a copy of.
  /// \returns the new function that receives the implicit arguemnts.
  Function *runOnFunction(Function *F);

  /// Helper function to replace call instruction with call instruction that
  /// receives implicit arguments. Replace indirect call if NewF is nullptr.
  /// \param CI pointer to CallInst.
  /// \param NewArgsVec arguments of new function with implicit arguments added.
  /// \param NewF function with implicit arguments added.
  void replaceCallInst(CallInst *CI, ArrayRef<Type *> NewArgs, Function *NewF);

private:
  /// Maps call instructions to the implicit arguments needed to patch up the
  /// call.
  DenseMap<CallInst *, Value **> FixupCalls;

  /// Maps the original and modified Function with implicit args.
  DenseMap<Function *, Function *> FixupFunctionsRefs;

  /// Result of LocalBufferAnalysis pass.
  LocalBufferInfo *LBInfo;

  /// Maps the modified Function with local values the function uses directly.
  LocalBufferInfo::TUsedLocalsMap DirectLocalsMap;

  /// Result of ImplicitArgsAnalysis pass.
  ImplicitArgsInfo *IAInfo;

  /// Callgraph of current module.
  CallGraph *CG;
};

} // namespace llvm

#endif
