//==--- DataPerBarrierValue.h - Collect Data per value - C++ -*-------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DATA_PER_VALUE_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DATA_PER_VALUE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/EquivalenceClasses.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerBarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WIRelatedValuePass.h"

#include <map>

namespace llvm {

/// Collect data on Values (instructions) that needs special handling.
class DataPerValue {
public:
  typedef MapVector<Function *, ValueVector> ValuesPerFunctionMap;
  typedef DenseMap<Value *, unsigned int> ValueToOffsetMap;
  typedef SetVector<Value *> ValueSet;
  typedef SetVector<Use *> UseSet;

public:
  DataPerValue(Module &M, DataPerBarrier *DPB, WIRelatedValue *WRV);

  /// Print data collected by the pass on the given module.
  /// OS stream to print the info regarding the module into,
  /// M pointer to the Module.
  void print(raw_ostream &OS, const Module *M) const;

  /// Return all values to handle in given function.
  /// F pointer to Function,
  /// Returns container of values to handle in F.
  ValueVector &getValuesToHandle(Function *F) {
    return SpecialValuesPerFuncMap[F];
  }

  /// Return all alloca values to handle in given function.
  /// F pointer to Function,
  /// Returns container of alloca values to handle in F.
  ValueVector &getAllocaValuesToHandle(Function *F) {
    return AllocaValuesPerFuncMap[F];
  }

  /// Return all uniform values to handle in given function.
  /// F pointer to Function,
  /// Returns container of uniform values to handle in F.
  ValueVector &getUniformValuesToHandle(Function *F) {
    return CrossBarrierValuesPerFuncMap[F];
  }

  /// Return offset of given value relatively to all other values in the special
  /// buffer structure.
  /// V pointer to Value,
  /// Returns offset of given value.
  unsigned int getOffset(Value *V) {
    assert(ValueOffsetMap.count(V) && "Requiring offset of non special value!");
    return ValueOffsetMap[V];
  }

  const MapVector<Instruction *, UseSet> *getCrossBarrierUses(Function *F) {
    auto It = CrossBarrierUses.find(F);
    return It == CrossBarrierUses.end() ? nullptr : &It->second;
  }

  /// Return true if the base type of the given value is i1.
  /// V pointer to Value,
  /// Returns true of the base type is i1.
  bool isOneBitElementType(Value *V) {
    return (OneBitElementValues.count(V) != 0);
  }

  /// Return true if given value has offset in the special buffer structure.
  bool hasOffset(Value *V) { return (0 != ValueOffsetMap.count(V)); }

  /// Return stride size per work item in special buffer for given function.
  unsigned int getStrideSize(Function *F) {
    assert(FuncEquivalenceClasses.findValue(F) !=
               FuncEquivalenceClasses.end() &&
           "F doesn't belong to any equivalence class!");
    return getSpecialBufferData(F).BufferTotalSize;
  }

private:
  struct SpecialBufferData {
    /// This will indecates the next empty offset in the buffer structure.
    unsigned int CurrentOffset;
    /// The alignment of buffer structure depends on
    /// largest alignment needed by any type in the buffer.
    unsigned int MaxAlignment;
    /// Special buffer total size in bytes per one WI.
    unsigned int BufferTotalSize;

    SpecialBufferData()
        : CurrentOffset(0), MaxAlignment(0), BufferTotalSize(0) {}
  };

  /// Entry point to the analysis.
  /// \param F function to analyze.
  void runOnFunction(Function &F);

  typedef enum {
    SpecialValueTypeNone,
    SpecialValueTypeA,
    SpecialValueTypeB1,
    SpecialValueTypeB2,
    SpecialValueTypeNum
  } SpecialValueType;

  /// Return type of given value Group-B.1, Group-B.2 or None.
  /// \param Inst pointer to Instruction,
  /// \param IsWIRelated true if \p Inst depends on WI id, otherwise false,
  /// \return SpecialValueType - speciality type of given value.
  SpecialValueType isSpecialValue(Instruction *Inst, bool IsWIRelated);

  /// Collect cross barrier uses for \p Inst.
  void collectCrossBarrierUses(Instruction *Inst);

  /// Calculates offsets of all values in Group-A and Group-B.1.
  /// F function to process its values.
  void calculateOffsets(Function &F);

  /// Return offset of given Type in special buffer stride.
  /// V pointer to Value behind the type,
  /// Ty pointer to Type,
  /// AllocaAlignment alignment of alloca instruction (0 if it is not alloca),
  /// BufferData reference to structure that contains special buffer data,
  /// Returns offset of given Type in special buffer stride.
  unsigned int getValueOffset(Value *Val, Type *Ty,
                              unsigned int AllocaAlignment,
                              SpecialBufferData &BufferData);

  /// Partition connected functions into equivalence classes on given module.
  /// M module to analyze.
  void calculateConnectedGraph(Module &M);

  /// Retrieve special buffer data of F's equivalence class.
  SpecialBufferData &getSpecialBufferData(Function *F) {
    return LeaderFuncToBufferDataMap[FuncEquivalenceClasses.getLeaderValue(F)];
  }

  /// Mark special argument and return values for given function by allocating
  /// place in special buffer for these values.
  /// F function to process its arguments.
  void markSpecialArguments(Function &F);

  /// Checks if \p U crosses barrier.
  bool crossesBarrier(Use &U);

  /// Entry point of the main logic.
  void analyze(Module &M);

private:
  /// This is barrier utility class.
  BarrierUtils Utils;

  /// Internal Data used to calculate user Analysis Data.

  /// This holds container of synchronize instructions in processed module.
  InstSet *SyncInstructions;

  /// This holds DataLayout of processed module.
  const DataLayout *DL;

  /// This holds DataPerBarrier analysis pass.
  DataPerBarrier *DPB;

  /// This holds WIRelatedValue analysis pass.
  WIRelatedValue *WRV;

  /// Analysis Data for pass user.

  /// This holds a map between function and its values of Group-A.
  ValuesPerFunctionMap AllocaValuesPerFuncMap;
  /// This holds a map between function and its values of Group-B.1.
  ValuesPerFunctionMap SpecialValuesPerFuncMap;
  /// This holds a map between function and its values of Group-B.2.
  ValuesPerFunctionMap CrossBarrierValuesPerFuncMap;
  /// This holds a set of returned values that cross barrier. They are not
  /// classified to Group-B.
  ValueSet CrossBarrierReturnedValues;
  /// This holds a map between value and its offset in Special Buffer structure.
  ValueToOffsetMap ValueOffsetMap;
  /// This holds a set of all special buffer values with base element of type
  /// i1.
  ValueSet OneBitElementValues;

  /// Equivalence classes connected by call edges.
  EquivalenceClasses<Function *> FuncEquivalenceClasses;
  /// This holds a map between the leader function of an equivalence class and
  /// buffer data.
  MapVector<Function *, SpecialBufferData> LeaderFuncToBufferDataMap;

  /// A map of maps between instructions and their cross-barrier users per
  /// function
  DenseMap<Function *, MapVector<Instruction *, UseSet>> CrossBarrierUses;
};

/// DataPerValueWrapper pass for legacy pass manager.
class DataPerValueWrapper : public ModulePass {
  std::unique_ptr<DataPerValue> DPV;

public:
  static char ID;

  DataPerValueWrapper();

  StringRef getPassName() const override {
    return "Intel Kernel DataPerValue Analysis";
  }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DataPerBarrierWrapper>();
    AU.addRequired<WIRelatedValueWrapper>();
    // Analysis pass preserve all.
    AU.setPreservesAll();
  }

  void print(raw_ostream &OS, const Module *M) const override {
    DPV->print(OS, M);
  };

  void releaseMemory() override { DPV.reset(); };

  DataPerValue &getDPV() { return *DPV; };
  const DataPerValue &getDPV() const { return *DPV; }
};

/// DataPerValueAnalysis pass for new pass manager.
class DataPerValueAnalysis : public AnalysisInfoMixin<DataPerValueAnalysis> {
  friend AnalysisInfoMixin<DataPerValueAnalysis>;
  static AnalysisKey Key;

public:
  using Result = DataPerValue;
  Result run(Module &M, ModuleAnalysisManager &);
  static StringRef name() { return "Intel Kernel DataPerValue Analysis"; }
};

/// Printer pass for DataPerValue.
class DataPerValuePrinter : public PassInfoMixin<DataPerValuePrinter> {
  raw_ostream &OS;

public:
  explicit DataPerValuePrinter(raw_ostream &OS) : OS(OS) {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DATA_PER_VALUE_H
