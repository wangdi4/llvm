//=------ SGHelper.h - Helper functions for subgroup emulation - C++ -*------=//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_SUB_GROUP_EMULATION_HELPER_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_SUB_GROUP_EMULATION_HELPER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

namespace llvm {

class SGHelper {
private:
  Module *M;
  LLVMContext *Context;

  // Functions used to do insertion.
  Function *SGBarrierF;
  Function *SGDummyBarrierF;

  static const char *DummyBarrierName;
  static const char *BarrierNameNoScope;
  static const char *BarrierNameWithScope;

  // Common types.
  Type *VoidType;
  Type *Int32Type;

  // CLK_LCL_MEM_FENCE.
  Constant *LocalMemFence;

  // sub_group_barrier.
  DPCPPKernelCompilationUtils::InstSet SGBarriers;
  // dummy_sg_barrier.
  DPCPPKernelCompilationUtils::InstSet SGDummyBarriers;
  // functions have sub_group_barrier.
  DPCPPKernelCompilationUtils::FuncSet SGSyncFunctions;

  MapVector<Function *, DPCPPKernelCompilationUtils::InstSet> FuncToBarriers;
  MapVector<Function *, DPCPPKernelCompilationUtils::InstSet>
      FuncToDummyBarriers;

  // Cached sub-group functions.
  Function *GetSGSizeF;
  Function *GetMaxSGSizeF;
  Function *GetSGLIdF;

  Value *ConstZero;
  Value *ConstOne;

  bool Initialized;

public:
  SGHelper();

  void initialize(Module &M);
  void invalidate() {
    M = nullptr;
    Context = nullptr;
    SGBarrierF = nullptr;
    SGDummyBarrierF = nullptr;
    VoidType = nullptr;
    Int32Type = nullptr;
    LocalMemFence = nullptr;
    GetSGSizeF = nullptr;
    GetMaxSGSizeF = nullptr;
    GetSGLIdF = nullptr;
    FuncToBarriers.clear();
    FuncToDummyBarriers.clear();
    SGSyncFunctions.clear();
    ConstZero = nullptr;
    ConstOne = nullptr;
    Initialized = false;
  }

  Value *getZero() { return ConstZero; }

  Value *getOne() { return ConstOne; }

  /// Note this function returns all sub_group_barrier calls including
  /// call in the vectorized kernel.
  const DPCPPKernelCompilationUtils::FuncSet &getAllSyncFunctions() const;

  /// This function should be called after SGBarrierPropagate function.
  /// Return all functions who call dummy_sg_barrier.
  DPCPPKernelCompilationUtils::FuncSet getAllFunctionsNeedEmulation();

  /// Get the first dummy_sg_barrier call in function F
  Instruction *getFirstDummyBarrier(Function *F);

  /// Barrier inserters.
  Instruction *insertBarrierBefore(Instruction *IP);
  Instruction *insertBarrierAfter(Instruction *IP);
  Instruction *insertDummyBarrierBefore(Instruction *IP);
  Instruction *insertDummyBarrierAfter(Instruction *IP);

  /// Remove sub_group_barrier / dummy_sg_barrier calls.
  void removeBarriers(ArrayRef<Instruction *> Insts);
  void removeDummyBarriers(ArrayRef<Instruction *> Insts);

  static bool isSyncCall(Instruction *I);
  static bool isBarrier(Instruction *I);
  static bool isDummyBarrier(Instruction *I);

  const DPCPPKernelCompilationUtils::InstSet &
  getBarriersForFunction(Function *F);
  const DPCPPKernelCompilationUtils::InstSet &
  getDummyBarriersForFunction(Function *F);

  /// Get all dummy_sg_barrier / sub_group_barrier calls.
  DPCPPKernelCompilationUtils::InstSet getSyncInstsForFunction(Function *F);

  Value *createGetSubGroupLId(Instruction *IP);
  Value *createGetSubGroupSize(Instruction *IP);
  Value *createGetMaxSubGroupSize(Instruction *IP);

  /// Insert printf in the kernel for debug purpose.
  /// TODO: Move this to CompilationUtils
  static void insertPrintf(const Twine &Prefix, Instruction *IP,
                           ArrayRef<Value *> Inputs = None);

private:
  void findBarriers();

  void findDummyBarriers();

  Instruction *createBarrierCall();

  Instruction *createDummyBarrierCall();
};

} // namespace llvm

#endif // INTEL_DPCPP_KERNEL_TRANSFORMS_SUB_GROUP_EMULATION_HELPER_H
