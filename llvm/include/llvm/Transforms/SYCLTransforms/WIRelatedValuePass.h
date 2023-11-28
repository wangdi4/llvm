//==--- WIRelatedValue.h - Detect values dependent on TIDs - C++ -*---------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_WI_RELATED_VALUE_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_WI_RELATED_VALUE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"

namespace llvm {

/// Distinguish between values dependent on WI id and those who are not.
class WIRelatedValue {
public:
  explicit WIRelatedValue(Module &M);

  bool runOnFunction(Function &F);

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

  void setWIRelated(Value *Val, bool WIRelated) {
    SpecialValues[Val] = WIRelated;
  }

  void print(raw_ostream &OS, const Module *M) const;

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
  bool calculateDep(FreezeInst *I);

  /// Return true if and only if a given value is ralated to WI Id.
  bool getWIRelation(Value *V);

  /// Update function argument dependency.
  void updateArgumentsDep(Function *F);

  /// Calculate the calling order of all functions need to be handled.
  void calculateCallingOrder();

private:
  /// This is barrier utility class
  BarrierUtils Utils;

  /// This is a list of all functions to be fixed in processed module
  /// that are ordered according to call graph from leaf to root.
  CompilationUtils::FuncVec OrderedFunctionsToAnalyze;

  /// Internal Data used to calculate user Analysis Data.
  /// Saves which values changed in this round.
  SetVector<Value *> Changed;

  /// Analysis Data for pass user.
  /// This holds a map between value and it relation on WI-id (related or not).
  DenseMap<Value *, bool> SpecialValues;
};

/// WIRelatedValueAnalysis pass for new pass manager.
class WIRelatedValueAnalysis
    : public AnalysisInfoMixin<WIRelatedValueAnalysis> {
  friend AnalysisInfoMixin<WIRelatedValueAnalysis>;
  static AnalysisKey Key;

public:
  using Result = WIRelatedValue;
  Result run(Module &M, ModuleAnalysisManager &AM);
};

/// Printer pass for WIRelatedValue.
class WIRelatedValuePrinter : public PassInfoMixin<WIRelatedValuePrinter> {
  raw_ostream &OS;

public:
  explicit WIRelatedValuePrinter(raw_ostream &OS) : OS(OS) {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_WI_RELATED_VALUE_H
