//===--------------------DTransSafetyAnalyzer.h--------------------------===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

// This file defines the safety type analyzer classes and interfaces that
// implement the DTrans safety checking required to determine whether structure
// types can be transformed.

#if !INTEL_INCLUDE_DTRANS
#error DTransSafetyAnalyzer.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSSAFETYANALYZER_H
#define INTEL_DTRANS_ANALYSIS_DTRANSSAFETYANALYZER_H

#include "Intel_DTrans/Analysis/DTrans.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

namespace llvm {
class BlockFrequencyInfo;
class Function;
class Module;
class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtransOP {
class DTransType;
class DTransTypeManager;
class PtrTypeAnalyzer;
class TypeMetadataReader;

// This class holds the results of the safety analysis of the aggregate
// types, and provides the interfaces needed by the transformations to query
// the DTrans information.
class DTransSafetyInfo {
public:
  using GetTLIFnType =
      std::function<const TargetLibraryInfo &(const Function &)>;

  // Adapter for directly iterating over the dtrans::TypeInfo pointers.
  struct type_info_iterator
      : public iterator_adaptor_base<
            type_info_iterator,
            DenseMap<DTransType *, dtrans::TypeInfo *>::iterator,
            std::forward_iterator_tag, dtrans::TypeInfo *> {
    explicit type_info_iterator(
        DenseMap<DTransType *, dtrans::TypeInfo *>::iterator X)
        : iterator_adaptor_base(X) {}

    dtrans::TypeInfo *&operator*() const { return I->second; }
    dtrans::TypeInfo *&operator->() const { return operator*(); }
  };

  DTransSafetyInfo() = default;
  DTransSafetyInfo(const DTransSafetyInfo &) = delete;
  DTransSafetyInfo &operator=(const DTransSafetyInfo &) = delete;

  DTransSafetyInfo(DTransSafetyInfo &&Other);
  DTransSafetyInfo &operator=(DTransSafetyInfo &&Other);

  ~DTransSafetyInfo();

  // Access methods to get the classes used when constructing the DTransTypes.
  DTransTypeManager &getTypeManager() {
    assert(TM.get() && "DTransTypeManager not initialized");
    return *TM;
  }
  TypeMetadataReader &getTypeMetadataReader() {
    assert(MDReader.get() && "TypeMetadataReader not initialized");
    return *MDReader;
  }
  PtrTypeAnalyzer &getPtrTypeAnalyzer() {
    assert(PtrAnalyzer.get() && "PtrTypeAnalyzer not initialized");
    return *PtrAnalyzer;
  }

  // Collect the safety bits for the structure types
  void analyzeModule(Module &M, GetTLIFnType GetTLI, WholeProgramInfo &WPInfo,
                     function_ref<BlockFrequencyInfo &(Function &)> GetBFI);

  // Cleanup memory, and set object back to default state.
  void reset();

  void setUnhandledPtrType(Value *V);

  bool getUnhandledPtrType() const { return UnhandledPtrType; }

  // Returns true if type has a safety violation for the specified Transform
  // type.
  bool testSafetyData(dtrans::TypeInfo *TyInfo, dtrans::Transform Transform);

  // Return true if safety analysis has been run and can be used in
  // transformations.
  bool useDTransSafetyAnalysis() const;

  // Return the value used during analysis for the command line option
  // "dtrans-outofboundsok" which controls the assumptions regarding whether
  // taking the address of a structure field is allowed to access other fields
  // of the structure.
  bool getDTransOutOfBoundsOK() const;

  // Retrieve the DTrans type information entry for the specified type.
  // If there is no entry for the specified type, create one.
  dtrans::TypeInfo *getOrCreateTypeInfo(DTransType *Ty);

  // Retrieve the DTrans type information entry for the specified type.
  // If there is no entry for the specified type, return nullptr.
  dtrans::TypeInfo *getTypeInfo(DTransType *Ty) const;

  // Add an entry to the 'PtrSubInfoMap'
  void addPtrSubMapping(llvm::BinaryOperator *BinOp, DTransType *Ty);

  // If the BinaryOperator has a type entry in the 'PtrSubInfoMap', return the
  // type. Otherwise, return nullptr.
  DTransType *getResolvedPtrSubType(BinaryOperator *BinOp);

  // Retrieve the CallInfo object for the instruction, if information exists.
  // Otherwise, return nullptr.
  dtrans::CallInfo *getCallInfo(const Instruction *I) const {
    return CIM.getCallInfo(I);
  }

  // Create an entry in the CallInfoMap about a memory allocation call.
  dtrans::AllocCallInfo *createAllocCallInfo(Instruction *I,
                                             dtrans::AllocKind AK) {
    return CIM.createAllocCallInfo(I, AK);
  }

  // Create an entry in the CallInfoMap about a memory freeing call
  dtrans::FreeCallInfo *createFreeCallInfo(Instruction *I,
                                           dtrans::FreeKind FK) {
    return CIM.createFreeCallInfo(I, FK);
  }

  // Create an entry in the CallInfoMap about a memset call.
  dtrans::MemfuncCallInfo *
  createMemfuncCallInfo(Instruction *I, dtrans::MemfuncCallInfo::MemfuncKind MK,
                        dtrans::MemfuncRegion &MR) {
    return CIM.createMemfuncCallInfo(I, MK, MR);
  }

  // Create an entry in the CallInfoMap about a memcpy/memmove call.
  dtrans::MemfuncCallInfo *
  createMemfuncCallInfo(Instruction *I, dtrans::MemfuncCallInfo::MemfuncKind MK,
                        dtrans::MemfuncRegion &MR1,
                        dtrans::MemfuncRegion &MR2) {
    return CIM.createMemfuncCallInfo(I, MK, MR1, MR2);
  }

  // Destroy the CallInfo stored about the specific instruction.
  void deleteCallInfo(Instruction *I) { CIM.deleteCallInfo(I); }

  // Update the instruction associated with the CallInfo object. This
  // is necessary because when a function is cloned during the DTrans
  // optimizations, the information needs to be transferred to the
  // newly created instruction of the cloned routine.
  void replaceCallInfoInstruction(dtrans::CallInfo *Info, Instruction *NewI) {
    CIM.replaceCallInfoInstruction(Info, NewI);
  }

  // Accessor for the set of TypeInfo objects.
  iterator_range<type_info_iterator> type_info_entries() {
    return make_range(type_info_iterator(TypeInfoMap.begin()),
                      type_info_iterator(TypeInfoMap.end()));
  }

  // Accessor for the set of CallInfo objects.
  iterator_range<dtrans::CallInfoManager::call_info_iterator>
  call_info_entries() {
    return CIM.call_info_entries();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAnalyzedTypes();
  void printCallInfo();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  std::unique_ptr<DTransTypeManager> TM;
  std::unique_ptr<TypeMetadataReader> MDReader;
  std::unique_ptr<PtrTypeAnalyzer> PtrAnalyzer;

  // A mapping from DTransTypes to the TypeInfo object that is used to
  // store information and safety bits about the types.
  DenseMap<DTransType *, dtrans::TypeInfo *> TypeInfoMap;

  // A mapping from function calls that special information is collected for
  // (malloc, free, memset, etc) to the information stored about those calls.
  dtrans::CallInfoManager CIM;

  // A mapping from BinaryOperator instructions that have been identified as
  // subtracting two pointers to types of interest to the interesting type
  // aliased by the operands.
  using PtrSubInfoMapType = ValueMap<Value *, DTransType *>;
  PtrSubInfoMapType PtrSubInfoMap;

  // Indicates DTrans safety information could not be computed because a Value
  // object was encountered that the PointerTypeAnalyzer could not collect
  // information for.
  bool UnhandledPtrType = false;

  // Indicates whether the module was completely analyzed for safety checks.
  bool DTransSafetyAnalysisRan = false;
};

class DTransSafetyAnalyzer : public AnalysisInfoMixin<DTransSafetyAnalyzer> {
public:
  typedef DTransSafetyInfo Result;
  Result run(Module &M, ModuleAnalysisManager &AM);

private:
  static AnalysisKey Key;
  friend AnalysisInfoMixin<DTransSafetyAnalyzer>;
  static char PassID;
};

class DTransSafetyAnalyzerWrapper : public ModulePass {
public:
  static char ID;

  DTransSafetyAnalyzerWrapper();

  DTransSafetyInfo &getDTransSafetyInfo(Module &M);

  bool runOnModule(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  DTransSafetyInfo Result;
};

} // end namespace dtransOP

ModulePass *createDTransSafetyAnalyzerTestWrapperPass();

} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSSAFETYANALYZER_H
