//=-------------------------- SGValueWiden.h -*- C++ -*----------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_SUBGROUP_EMULATION_VALUE_WIDEN_H
#define BACKEND_SUBGROUP_EMULATION_VALUE_WIDEN_H

#include "BarrierUtils.h"
#include "SGHelper.h"
#include "SGSizeAnalysis.h"
#include "WIRelatedValuePass.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"

using namespace llvm;

namespace intel {

class SGValueWiden : public ModulePass {
public:
  static char ID;

  SGValueWiden(bool EnableDebug = false)
      : ModulePass(ID), SizeAnalysis(nullptr), WIRelatedAnalysis(nullptr),
        ConstZero(nullptr), EnableDebug(EnableDebug) {}

  bool runOnModule(Module &M) override;

  StringRef getPassName() const override { return "SGValueWiden Pass"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SGSizeAnalysis>();
    AU.addRequired<WIRelatedValue>();
  }

private:
  SGHelper Helper;

  BarrierUtils Utils;

  SGSizeAnalysis *SizeAnalysis;

  WIRelatedValue *WIRelatedAnalysis;

  Value *ConstZero;

  bool EnableDebug;

  void runOnFunction(Function &F, const std::set<unsigned> &Sizes);

  /// Alloca vector / array for original "scalar" alloca instruction.
  void widenAlloca(Instruction *V, Instruction *FirstI, unsigned Size);

  /// Alloca vector for values whose type is int / float / pointer.
  /// Alloca array for values with other types.
  /// Fill VecValueMap.
  void widenValue(Instruction *V, Instruction *FirstI, unsigned Size);

  /// If the value is uniform in sub-group, there is no need to alloca
  /// a vector /array counterpart. We can just alloca a scalar counterpart
  /// for it.
  void hoistUniformValue(Instruction *V, Instruction *FirstI);

  /// Collect all calls to be widned.
  void collectWideCalls(Module &M);

  /// Vectorize calls which call emulated functions or sub-group built-ins.
  void widenCalls();

  bool isWideCall(Value *V) {
    if (auto *CI = dyn_cast<CallInst>(V))
      return WideCalls.count(CI);
    return false;
  }

  /// Get the widend value for V.
  Value *getVectorValue(Value *V, unsigned Size, Instruction *IP);

  /// Get the pointer of widend value for V.
  Value *getVectorValuePtr(Value *V, unsigned Size, Instruction *IP);

  /// Get the scalar value for V.
  /// This will be used when the VectorKind is uniform.
  Value *getScalarValue(Value *V, Instruction *IP);

  /// Set the data as widend value for V.
  void setVectorValue(Value *Data, Value *V, unsigned Size, Instruction *IP);

  /// Check whether Def I is cross by sub_group_barrier / dummy_sg_barrier.
  bool isCrossBarrier(Instruction *I, const InstSet &SyncInsts) const;

  /// Chekc if V is uniform in sub-group.
  bool isWIRelated(Value *V);

  /// Get the insert point for I when processing operand V.
  Instruction *getInsertPoint(Instruction *I, Value *V);

  /// Get current address of Val for U.
  Value *getWIOffset(Instruction *U, Value *Val);
  /// Get current value of Val for U.
  Value *getWIValue(Instruction *U, Value *Val);
  /// Set current value of Val.
  void setWIValue(Value *Val);

  /// Calls to be widened.
  InstSet WideCalls;

  /// All functions need to be widened.
  FuncSet FunctionsToBeWidened;

  /// Map from Function to Widend Function.
  DenseMap<Function *, Function *> FuncMap;

  /// BB will be excluded from work-group loop.
  DenseMap<Function *, BasicBlock *> WGExcludeBBMap;
  /// BB will be excluded from sub-group loop.
  DenseMap<Function *, BasicBlock *> SGExcludeBBMap;

  /// Map from value to its widend alloca instruction.
  DenseMap<Value *, Value *> VecValueMap;

  /// Map from value to its hoisted alloca instruction.
  DenseMap<Value *, Value *> UniValueMap;

  /// Map from alloca for debug to the widened alloca instruction.
  DenseMap<AllocaInst *, AllocaInst *> DebugAIMap;

  /// Instructions to be removed.
  SmallVector<Instruction *, 64> InstsToBeRemoved;
};
} // namespace intel
#endif // BACKEND_SUBGROUP_EMULATION_VALUE_WIDEN_H
