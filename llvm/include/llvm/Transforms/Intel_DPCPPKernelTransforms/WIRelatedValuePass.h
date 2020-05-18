//==--- WIRelatedValue.h - Detect values dependent on TIDs - C++ -*---------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_WI_RELATED_VALUE_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_WI_RELATED_VALUE_H

#include "llvm/ADT/SetVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"

namespace llvm {

/// WIRelatedValue pass is a analysis module pass used to
/// distinguish between values dependent on WI id and those who are not.
class WIRelatedValue : public ModulePass {

public:
  static char ID;

  WIRelatedValue();

  ~WIRelatedValue() {}

  llvm::StringRef getPassName() const override {
    return "Intel Kernel WIRelatedValue";
  }

  bool runOnModule(Module &M) override;

  bool runOnFunction(Function &F);

  /// Inform about usage/mofication/dependency of this pass.
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // Analysis pass preserve all.
    AU.setPreservesAll();
  }

  /// Print data collected by the pass on the given module.
  /// OS stream to print the info regarding the module into.
  /// M pointer to the Module.
  void print(raw_ostream &OS, const Module *M = 0) const override;

  /// Return true if given value depends on WI Id.
  /// V is a pointer to Value.
  /// Returns true if and only if given value depends on WI Id.
  bool isWIRelated(Value *V) {
    if (!SpecialValues.count(V)) {
      // This might happen for function parameters.
      return false;
    }
    return SpecialValues[V];
  }

protected:
  /// Update dependency relations between all values.
  void updateDeps();

  /// Dependency Calculation Functions.
  /// Calculate the WI Id relation for the given instruction.
  /// I Instruction to inspect.
  /// Returns true if and only if instruction is related to WI id.
  void calculateDep(Value *V);
  bool calculateDep(BinaryOperator *I);
  bool calculateDep(CallInst *I);
  bool calculateDep(CmpInst *I);
  bool calculateDep(ExtractElementInst *I);
  bool calculateDep(GetElementPtrInst *I);
  bool calculateDep(InsertElementInst *I);
  bool calculateDep(InsertValueInst *I);
  bool calculateDep(PHINode *I);
  bool calculateDep(ShuffleVectorInst *I);
  bool calculateDep(StoreInst *I);
  bool calculateDepTerminator(Instruction *I);
  bool calculateDep(SelectInst *I);
  bool calculateDep(AllocaInst *I);
  bool calculateDep(CastInst *I);
  bool calculateDep(ExtractValueInst *I);
  bool calculateDep(LoadInst *I);
  bool calculateDep(VAArgInst *I);

  /// Return true if and only if a given value is ralated to WI Id.
  bool getWIRelation(Value *V);

  /// Update function argument dependency.
  void updateArgumentsDep(Function *F);

  /// Calculate the calling order of all functions need to be handled.
  void calculateCallingOrder();

private:
  Module *M;

  /// This is a list of all functions to be fixed in processed module
  /// that are ordered according to call graph from leaf to root.
  DPCPPKernelBarrierUtils::FuncVector OrderedFunctionsToAnalyze;

  /// Internal Data used to calculate user Analysis Data.
  /// Saves which values changed in this round.
  SetVector<Value *> ChangedValues;

  /// Analysis Data for pass user.
  /// This holds a map between value and it relation on WI-id (related or not).
  ValueMap<Value *, bool> SpecialValues;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_WI_RELATED_VALUE_H
