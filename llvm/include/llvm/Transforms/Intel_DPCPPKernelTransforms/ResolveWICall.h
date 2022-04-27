//===- ResolveWICall.h - Resolve DPC++ kernel work-item call --------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_RESOLVE_WI_CALL_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_RESOLVE_WI_CALL_H

#include "ImplicitArgsAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include <set>

namespace llvm {
enum TInternalCallType : int;

/// Resolve work item function calls.
class ResolveWICallPass : public PassInfoMixin<ResolveWICallPass> {
public:
  ResolveWICallPass(bool IsUniformWG = false, bool UseTLSGlobals = false)
      : IsUniformWG(IsUniformWG), UseTLSGlobals(UseTLSGlobals) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, bool IsUniformWG, bool UseTLSGlobals,
               ImplicitArgsInfo *IAInfo, CallGraph *CG);

private:
  /// Resolves work-item function calls of the kernel.
  /// \param F the function which needs work-item function calls to be resolved.
  /// \returns new function that all work-item function calls are resolved.
  Function *runOnFunction(Function *F);

  /// Update enqueue_kernel function arguments.
  Value *updateEnqueueKernelFunction(SmallVectorImpl<Value *> &NewParams,
                                     const StringRef FuncName, CallInst *CI);

  /// Substitues the a work item function calls with acesses to implicit
  /// arguments and calculations based on them.
  /// \param CI the call instruction that calls a work item function.
  /// \param CallType the call instruction type.
  /// \returns The result value of the work item function call.
  Value *updateGetFunction(CallInst *CI, TInternalCallType CallType);

  /// Calculates work-item information that use dimension (this function is used
  /// where dimension is in bound).
  /// \param CI the call instruction that calls a work item function.
  /// \param CallType the call instruction type.
  /// \param InsterBefore the instruction to insert new instructions before.
  /// \returns the result value of the work item function call.
  Value *updateGetFunctionInBound(CallInst *CI, TInternalCallType CallType,
                                  Instruction *InsertBefore);

  /// Returns internal call type for given function name.
  /// \param FuncName called function name.
  /// \returns Internal Call Type for given function name.
  TInternalCallType getCallFunctionType(StringRef FuncName);

  /// Create call to __opencl_printf.
  Value *updatePrintf(IRBuilder<> &Builder, CallInst *CI);
  /// Create call to __lprefetch.
  void updatePrefetch(llvm::CallInst *CI);

  /// Add prefetch function declaration.
  void addPrefetchDeclaration();

  /// Add declaration of external function to Module.
  /// \param Ty callback type.
  /// \param FTy function type.
  /// \param FuncName function name.
  void addExternFunctionDeclaration(unsigned Ty, FunctionType *FTy,
                                    StringRef FuncName);

  /// Helper functions to construct OpenCL types.
  /// constructs type for queue_t.
  Type *getQueueType() const;
  /// constructs type for clk_event_t.
  Type *getClkEventType() const;
  /// constructs type for kernel_enqueue_flags_t.
  Type *getKernelEnqueueFlagsType() const;
  /// constructs type for ndrange_t.
  Type *getNDRangeType() const;
  /// constructs type for block with local mem arguments.
  Type *getBlockLocalMemType() const;
  /// constructs type for return type of enqueue_kernel.
  Type *getEnqueueKernelRetType() const;
  /// return ConstantInt::int32_type with zero value.
  ConstantInt *getConstZeroInt32Value() const;
  /// get or add from/to module declaration of type used for local memory
  /// buffers specified in enqueue_kernel.
  Type *getLocalMemBufType() const;

  /// Returns EnqueueKernel callback function type.
  /// \param FuncType callback type {basic, localmem, event, ...}.
  FunctionType *getOrCreateEnqueueKernelFuncType(unsigned FuncType);
  /// Returns __opencl_printf function type.
  FunctionType *getOrCreatePrintfFuncType();

  /// get the pointer size for the current target, in bits (32 or 64).
  unsigned getPointerSize() const;

  void clearPerFunctionCache();
  Value *getOrCreateRuntimeInterface();
  Value *getOrCreateBlock2KernelMapper();

  /// The llvm current processed module.
  Module *M;
  /// The llvm context.
  LLVMContext *Ctx;

  ImplicitArgsInfo *IAInfo;

  /// CallGraph of current module.
  CallGraph *CG;

  /// This holds the Runtime Handle implicit argument of current handled
  /// function This argument is initialized passed thru to MIC's printf.
  Value *RuntimeHandle;
  /// This holds the WorkInfo implicit argument of current handled function.
  Value *WorkInfo;
  /// This holds the pWGId implicit argument of current handled function.
  Value *WGId;
  /// This holds the pBaseGlbId implicit argumnet of current handled function.
  Value *BaseGlbId;

  /// This is flag indicates that Prefetch declarations already added to module.
  bool PrefetchDecl;
  /// flags indicates that extended execution built-in declarations already
  /// added to module.
  std::set<unsigned> ExtExecDecls;

  /// number of bits in integer returned from enqueue_kernel BI
  /// constant introduced for readability of code.
  enum { ENQUEUE_KERNEL_RETURN_BITS = 32 };

  // Per function cached values.
  Function *F;
  Value *RuntimeInterface;
  Value *Block2KernelMapper;

  // Version of OpenCL C a processed module is compiled for.
  unsigned OclVersion;
  // True if a module is compiled with uniform work-group size,
  // e.g. -cl-uniform-work-group-size.
  bool IsUniformWG;
  // Use TLS globals instead of implicit arguments.
  bool UseTLSGlobals;
};

} // namespace llvm

#endif
