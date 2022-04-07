//===- PrepareKernelArgs.h - Prepare DPC++ kernel arguments ---------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PREPARE_KERNEL_ARGS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PREPARE_KERNEL_ARGS_H

#include "ImplicitArgsAnalysis.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
/// Change the way arguments are passed to kernels.
/// It changes the kernel to receive as arguments a single buffer which contains
/// the the kernel's original and implicit arguments. It loads the arguments and
/// calls the original kernel. The position of the arguments in the buffer is
/// calculated based on the arguments' alignment, which in non LLVM dependent.
class PrepareKernelArgsPass : public PassInfoMixin<PrepareKernelArgsPass> {
public:
  PrepareKernelArgsPass(bool UseTLSGlobals = false) : UseTLSGlobals(UseTLSGlobals) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, bool UseTLSGlobals, ImplicitArgsInfo *IAInfo);

private:
  /// Creates a wrapper function for the given function that receives one buffer
  /// as argument, creates load instructions that load the function arguments
  /// from the buffer, creates a call to the given funciton with the loaded
  /// arguments.
  /// \param F The kernel which is wrapped by the wrapper.
  /// \returns true if changed.
  bool runOnFunction(Function *F);

  /// Creates a new function that receives as argument a single buffer based on
  /// the given function's name, return type and calling convention.
  /// \param F The kernel for which to create a wrapper function.
  /// \returns A new function.
  Function *createWrapper(Function *F);

  /// Creates the body of the wrapper function: creates load instructions that
  /// load the function arguments from the buffer, creates a call to the given
  /// F with the loaded arguments.
  /// \param Wrapper the kernel for which to create a wrapper function.
  /// \param F the kernel which is wrapped by the wrapper.
  /// \returns CallInst which calls F.
  CallInst *createWrapperBody(Function *Wrapper, Function *F);

  /// Replaces function pointers to the original function by pointers
  ///         to wrapper one in device execution built-in calls.
  void replaceFunctionPointers(Function *Wrapper, Function *F);

  /// Creates the body of the wrapper function: creates load instructions that
  /// load the function arguments from the buffer, creates a call to the given
  /// WrappedKernel with the loaded arguments.
  /// \param Builder An IR builder that allows to add instructions to the
  /// wrapper.
  /// \param WrappedKernel The kernel which is wrapped by the wrapper.
  /// \param ArgsBuffer The single buffer argument that is passed to the
  /// wrapper. WrappedKernel arguments need to be loaded from this buffer.
  /// \param WGId Workgroup IDs.
  /// \param RuntimeContext Runtime parameters.
  /// \returns a parameters vector containing the loaded values that need to be
  /// used when calling F.
  std::vector<Value *> createArgumentLoads(IRBuilder<> &Builder,
                                           Function *WrappedKernel,
                                           Argument *ArgsBuffer, Argument *WGId,
                                           Argument *RuntimeContext);

  Type *getGIDWrapperArgType() const;
  Type *getRuntimeContextWrapperArgType() const;

  /// Emptify wrapped kernel so that it only contains a ret instruction.
  void emptifyWrappedKernel(Function *F);

private:
  /// The llvm module this pass needs to update.
  Module *M;

  ImplicitArgsInfo *IAInfo;

  IntegerType *SizetTy;
  IntegerType *I8Ty;
  IntegerType *I32Ty;

  /// Use TLS globals instead of implicit arguments.
  bool UseTLSGlobals;
};

} // namespace llvm

#endif
