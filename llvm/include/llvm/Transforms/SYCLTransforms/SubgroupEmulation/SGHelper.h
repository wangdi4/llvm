//=------ SGHelper.h - Helper functions for subgroup emulation - C++ -*------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SUB_GROUP_EMULATION_HELPER_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SUB_GROUP_EMULATION_HELPER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

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
  CompilationUtils::InstSet SGBarriers;
  // dummy_sg_barrier.
  CompilationUtils::InstSet SGDummyBarriers;
  // functions have sub_group_barrier.
  CompilationUtils::FuncSet SGSyncFunctions;

  MapVector<Function *, CompilationUtils::InstSet> FuncToBarriers;
  MapVector<Function *, CompilationUtils::InstSet> FuncToDummyBarriers;

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
  const CompilationUtils::FuncSet &getAllSyncFunctions() const;

  /// This function should be called after SGBarrierPropagate function.
  /// Return all functions who call dummy_sg_barrier.
  CompilationUtils::FuncSet getAllFunctionsNeedEmulation();

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

  const CompilationUtils::InstSet &getBarriersForFunction(Function *F);
  const CompilationUtils::InstSet &getDummyBarriersForFunction(Function *F);

  /// Get all dummy_sg_barrier / sub_group_barrier calls.
  CompilationUtils::InstSet getSyncInstsForFunction(Function *F);

  Value *createGetSubGroupLId(Instruction *IP);
  Value *createGetSubGroupSize(Instruction *IP);
  Value *createGetMaxSubGroupSize(Instruction *IP);

  /// Get promoted integer vector type for IntVecType. Return IntVecType if
  /// promotion is not needed.
  /// Some integer vector types are not element-wise-addressable on X86 targets.
  /// e.g. <16 x i1> would be stored in packed bits (2 bytes)
  ///      In such case we cannot do GEP on <16 x i1>*, since it's impossible to
  ///      get byte-aligned address for each i1 element.
  /// The promoted type is guaranteed to be GEP-able.
  /// e.g. <16 x i1> is promoted to <16 x i8>
  /// e.g. <2 x i2> is promoted to <2 x i8>
  static llvm::Type *getPromotedIntVecType(llvm::Type *IntVecType);
  static llvm::Type *getPromotedIntVecType(llvm::Value *V) {
    return getPromotedIntVecType(V->getType());
  }

  /// Get vector type of T widen by Size. Result vector type is promoted if
  /// needed.
  /// e.g. i32 --> <Size x i32>
  /// e.g. <4 x i32> --> <4xSize x i32>
  /// e.g. i1 --> <Size x i8> (promoted to i8)
  static llvm::Type *getVectorType(llvm::Type *T, unsigned Size);
  static llvm::Type *getVectorType(llvm::Value *V, unsigned Size) {
    return getVectorType(V->getType(), Size);
  }

  /// Create zext or trunc instruction on From value to the destination type.
  /// If no need, return From.
  static llvm::Value *createZExtOrTruncProxy(llvm::Value *From,
                                             llvm::Type *ToType,
                                             llvm::IRBuilder<> &Builder);

private:
  void findBarriers();

  void findDummyBarriers();

  Instruction *createBarrierCall();

  Instruction *createDummyBarrierCall();
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_SUB_GROUP_EMULATION_HELPER_H
