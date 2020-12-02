//=---------------------------- SGHelper.h -*- C++ -*------------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_SUB_GROUP_EMULATION_HELPER_H
#define BACKEND_SUB_GROUP_EMULATION_HELPER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/InstIterator.h"

namespace llvm {
class Constant;
class Function;
class Instruction;
class LLVMContext;
class Module;
class Twine;
class Type;
class Value;
class BasicBlock;
class IRBuilderBase;
} // namespace llvm

namespace intel {

/// Using SetVector / MapVector to keep the IR stable.
using InstSet = llvm::SetVector<llvm::Instruction *>;
using FuncSet = llvm::SetVector<llvm::Function *>;
using BBSet = llvm::SetVector<llvm::BasicBlock *>;
using InstVec = llvm::SmallVector<llvm::Instruction *, 16>;
using FuncVec = llvm::SmallVector<llvm::Function *, 16>;
using BBVec = llvm::SmallVector<llvm::BasicBlock *, 16>;

class SGHelper {
private:
  llvm::Module *M;
  llvm::LLVMContext *Context;

  // Functions used to do insertion.
  llvm::Function *SGBarrierF;
  llvm::Function *SGDummyBarrierF;

  static const char *DummyBarrierName;
  static const char *BarrierNameNoScope;
  static const char *BarrierNameWithScope;

  // Common types.
  llvm::Type *VoidType;
  llvm::Type *Int32Type;

  // CLK_LCL_MEM_FENCE.
  llvm::Constant *LocalMemFence;

  // sub_group_barrier.
  InstSet SGBarriers;
  // dummy_sg_barrier.
  InstSet SGDummyBarriers;
  // functions have sub_group_barrier.
  FuncSet SGSyncFunctions;

  llvm::MapVector<llvm::Function *, InstSet> FuncToBarriers;
  llvm::MapVector<llvm::Function *, InstSet> FuncToDummyBarriers;

  // Cached sub-group functions.
  llvm::Function *GetSGSizeF;
  llvm::Function *GetMaxSGSizeF;
  llvm::Function *GetSGLIdF;

  llvm::Value *ConstZero;
  llvm::Value *ConstOne;

  bool Initialized;

public:
  SGHelper();

  void initialize(llvm::Module &M);
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

  llvm::Value *getZero() { return ConstZero; }

  llvm::Value *getOne() { return ConstOne; }

  /// Note this function returns all sub_group_barrier calls including
  /// call in the vectorized kernel.
  const FuncSet &getAllSyncFunctions() const;

  /// This function should be called after SGBarrierPropagate function.
  /// Return all functions who call dummy_sg_barrier.
  FuncSet getAllFunctionsNeedEmulation();

  /// Get the first dummy_sg_barrier call in function F
  llvm::Instruction *getFirstDummyBarrier(llvm::Function *F);

  /// Barrier inserters.
  llvm::Instruction *insertBarrierBefore(llvm::Instruction *IP);
  llvm::Instruction *insertBarrierAfter(llvm::Instruction *IP);
  llvm::Instruction *insertDummyBarrierBefore(llvm::Instruction *IP);
  llvm::Instruction *insertDummyBarrierAfter(llvm::Instruction *IP);

  /// Remove sub_group_barrier / dummy_sg_barrier calls.
  void removeBarriers(llvm::ArrayRef<llvm::Instruction *> Insts);
  void removeDummyBarriers(llvm::ArrayRef<llvm::Instruction *> Insts);

  static bool isSyncCall(llvm::Instruction *I);
  static bool isBarrier(llvm::Instruction *I);
  static bool isDummyBarrier(llvm::Instruction *I);

  const InstSet &getBarriersForFunction(llvm::Function *F);
  const InstSet &getDummyBarriersForFunction(llvm::Function *F);

  /// Get all dummy_sg_barrier / sub_group_barrier calls.
  InstSet getSyncInstsForFunction(llvm::Function *F);

  llvm::Value *createGetSubGroupLId(llvm::Instruction *IP);
  llvm::Value *createGetSubGroupSize(llvm::Instruction *IP);
  llvm::Value *createGetMaxSubGroupSize(llvm::Instruction *IP);

  /// Insert printf in the kernel for debug purpose.
  /// TODO: Move this to CompilationUtils
  static void insertPrintf(const llvm::Twine &Prefix, llvm::Instruction *IP,
                           llvm::ArrayRef<llvm::Value *> Inputs = llvm::None);

  /// Find dummybarrier - dummybarrier region (short for dummy region)
  static llvm::inst_range findDummyRegion(llvm::Function &F);

private:
  void findBarriers();

  void findDummyBarriers();

  llvm::Instruction *createBarrierCall();

  llvm::Instruction *createDummyBarrierCall();
};

} // namespace intel

#endif // BACKEND_SUB_GROUP_EMULATION_HELPER_H
