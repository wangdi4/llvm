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
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerBarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WIRelatedValuePass.h"

#include <map>

namespace llvm {

/// DataPerValue pass is a analysis module pass used to collect
/// data on Values (instructions) that needs special handling.
class DataPerValue : public ModulePass {
public:
  typedef MapVector<Function *, ValueVector> ValuesPerFunctionMap;
  typedef DenseMap<Value *, unsigned int> ValueToOffsetMap;
  typedef SetVector<Value *> ValueSet;

public:
  static char ID;

  DataPerValue();

  ~DataPerValue() {}

  llvm::StringRef getPassName() const override {
    return "Intel Kernel DataPerValue";
  }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DataPerBarrier>();
    AU.addRequired<WIRelatedValue>();
    // Analysis pass preserve all.
    AU.setPreservesAll();
  }

  /// Print data collected by the pass on the given module.
  /// OS stream to print the info regarding the module into,
  /// M pointer to the Module.
  void print(raw_ostream &OS, const Module *M = 0) const override;

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
    assert(FunctionEntryMap.count(F) &&
           "no structure stride data entry for F!");
    unsigned int entry = FunctionEntryMap[F];
    assert(EntryBufferDataMap.count(entry) &&
           "no structure stride data for F entry!");
    return EntryBufferDataMap[entry].BufferTotalSize;
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
  using FunctionToEntryMap = std::map<Function *, unsigned int>;
  using EntryToBufferDataMap = std::map<unsigned int, SpecialBufferData>;

  /// Execute pass on given function.
  /// F function to analyze,
  /// Return true if function was modified.
  virtual bool runOnFunction(Function &F);

  typedef enum {
    SpecialValueTypeNone,
    SpecialValueTypeA,
    SpecialValueTypeB1,
    SpecialValueTypeB2,
    SpecialValueTypeNum
  } SpecialValueType;

  /// Return type of given value Group-B.1, Group-B.2 or None.
  /// V pointer to Value,
  /// IsWIRelated true if value depends on WI id, otherwise false,
  /// Returns SpecialValueType - speciality type of given value.
  SpecialValueType isSpecialValue(Value *V, bool IsWIRelated);

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

  /// Find all connected function into disjoint groups on given module.
  /// M module to analyze.
  void calculateConnectedGraph(Module &M);

  /// Replace all entry value equal to "from" in FunctionEntryMap with value
  /// "To". From - the entry value to replace, To - the entry value to replace
  /// with.
  void fixEntryMap(unsigned int From, unsigned int To);

  /// Mark special argument and return values for given function by allocating
  /// place in special buffer for these values.
  /// F function to process its arguments.
  void markSpecialArguments(Function &F);

private:
  /// This is barrier utility class.
  DPCPPKernelBarrierUtils BarrierUtils;

  /// Internal Data used to calculate user Analysis Data.

  /// This holds DataPerBarrier analysis pass.
  DataPerBarrier *DataPerBarrierAnalysis;
  /// This holds container of synchronize instructions in processed module.
  InstSet *SyncInstructions;
  /// This holds WIRelatedValue analysis pass.
  WIRelatedValue *WIRelatedValueAnalysis;
  /// This holds DataLayout of processed module.
  const DataLayout *DL;

  /// Analysis Data for pass user.

  /// This holds a map between function and its values of Group-A.
  ValuesPerFunctionMap AllocaValuesPerFuncMap;
  /// This holds a map between function and its values of Group-B.1.
  ValuesPerFunctionMap SpecialValuesPerFuncMap;
  /// This holds a map between function and its values of Group-B.2.
  ValuesPerFunctionMap CrossBarrierValuesPerFuncMap;
  /// This holds a map between value and its offset in Special Buffer structure.
  ValueToOffsetMap ValueOffsetMap;
  /// This holds a set of all special buffer values with base element of type
  /// i1.
  ValueSet OneBitElementValues;

  /// This holds a map between function and its entry in buffer data map.
  FunctionToEntryMap FunctionEntryMap;
  /// This holds a map between unique entry and buffer data.
  EntryToBufferDataMap EntryBufferDataMap;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DATA_PER_VALUE_H
