//===--------------------DTransSafetyAnalyzer.h--------------------------===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

// This file defines the safety type analyzer classes and interfaces that
// implement the DTrans safety checking required to determine whether structure
// types can be transformed.

#if !INTEL_FEATURE_SW_DTRANS
#error DTransSafetyAnalyzer.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSSAFETYANALYZER_H
#define INTEL_DTRANS_ANALYSIS_DTRANSSAFETYANALYZER_H

#include "Intel_DTrans/Analysis/DTrans.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

namespace llvm {
class BlockFrequencyInfo;
class DTransImmutableInfo;
class Function;
class GEPOperator;
class Module;
class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtransOP {
class DTransType;
class DTransTypeManager;
class PtrTypeAnalyzer;
class TypeMetadataReader;
class DTransRelatedTypesUtils;

// This class holds the results of the safety analysis of the aggregate
// types, and provides the interfaces needed by the transformations to query
// the DTrans information.
class DTransSafetyInfo {
public:
  using GetTLIFnType =
      std::function<const TargetLibraryInfo &(const Function &)>;

  /// Adapter for directly iterating over the dtrans::TypeInfo pointers.
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

  // Returns true if the module requires runtime validation of possible bad cast
  // issues. Returns functions where runtime checks are required.
  bool requiresBadCastValidation(SetVector<Function *> &Func,
                                 unsigned &ArgumentIndex,
                                 unsigned &StructIndex) const;

  /// Handle invalidation events in the new pass manager.
  bool invalidate(Module &M, const PreservedAnalyses &PA,
                  ModuleAnalysisManager::Invalidator &Inv);

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
                     DTransImmutableInfo *DTImmutInfo,
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

  // Return the value used during analysis for the command line option
  // "dtrans-usecrulecompat" which controls the assumptions regarding whether
  // C language rules may be applied to disambiguate types.  NOTE: If a
  // Fortran function is seen, we will conservatively return 'false' for
  // getDTransUseCRuleCompat() even if the command line option is 'true', as
  // the appropriate rules for Fortran have not yet been implemented.
  bool getDTransUseCRuleCompat() const;

  // Retrieve the DTrans type information entry for the specified type.
  // If there is no entry for the specified type, create one.
  dtrans::TypeInfo *getOrCreateTypeInfo(DTransType *Ty);

  // Retrieve the DTrans type information entry for the specified type.
  // If there is no entry for the specified type, return nullptr.
  dtrans::TypeInfo *getTypeInfo(DTransType *Ty) const;

  // Retrieve the DTrans StructInfo entry for the specified structure.
  // Note: Only named structures are permitted, not literal structures.
  dtrans::StructInfo *getStructInfo(llvm::StructType *STy) const;

  // Add an entry to the 'PtrSubInfoMap'
  void addPtrSubMapping(llvm::BinaryOperator *BinOp, DTransType *Ty);

  // If the BinaryOperator has a type entry in the 'PtrSubInfoMap', return the
  // type. Otherwise, return nullptr.
  DTransType *getResolvedPtrSubType(BinaryOperator *BinOp);

  // If the GEP was identified as a byte-flattened GEP during the pointer type
  // analysis, return the type and field number that is indexed by the GEP.
  std::pair<DTransType *, size_t> getByteFlattenedGEPElement(GEPOperator *GEP);

  // Adaptor for getByteFlattenedGEPElement that converts a GetElementPtrInst to
  // a GEPOperator.
  std::pair<DTransType *, size_t>
  getByteFlattenedGEPElement(GetElementPtrInst *GEP);

  // Update the mapping from LoadInst instructions that have been identified as
  // being structure element reads to a type-index pair for the element being
  // read. If the instruction is already tracked, update the
  // MultiElemLoadStoreInfo set.
  void addLoadMapping(LoadInst *LdInst,
    std::pair<llvm::Type *, size_t> Pointee);

  // Update the mapping from StoreInst instructions that have been identified as
  // being structure element writes to a type-index pair for the element being
  // written. If the instruction is already tracked, update the
  // MultiElemLoadStoreInfo set.
  void addStoreMapping(StoreInst *StInst,
    std::pair<llvm::Type *, size_t> Pointee);

  // If the specified 'LdInst' was identified as structure element read, return
  // the type-index pair for the element read. Otherwise, return <nullptr, 0>.
  std::pair<llvm::Type *, size_t> getLoadElement(LoadInst *LdInst);

  // If the specified 'StInst' was identified as structure element written,
  // return the type-index pair for the element read. Otherwise,
  // return <nullptr, 0>.
  std::pair<llvm::Type *, size_t> getStoreElement(StoreInst *StInst);

  // Update the set of Load/Store instructions that are accessing more than one
  // structure field.
  void addMultiElemLoadStore(Instruction *I);

  // Returns true if 'I' was identified as Load/Store instruction
  // that is accessing more than one structure field.
  bool isMultiElemLoadStore(Instruction *I);

  // A helper routine to get a DTrans structure type and field index from the
  // GEP instruction which is a pointer argument of 'Load'.
  // :
  //  %b = getelementptr inbounds %struct.s, %struct.s* %a, i64 x, i32 y
  //  %c = load i8* (i8*, i64, i64)*, i8* (i8*, i64, i64)** %b, align 8
  //
  // Given load instruction returns dtrans::StructInfo for %struct.s and y.
  std::pair<dtrans::StructInfo *, uint64_t> getInfoFromLoad(LoadInst *Load);

  // A helper routine to retrieve structure type - field index pair from a
// GEPOperator.
  std::pair<llvm::StructType *, uint64_t> getStructField(GEPOperator *GEP);

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

  uint64_t getMaxTotalFrequency() const { return MaxTotalFrequency; }
  void setMaxTotalFrequency(uint64_t MTFreq) { MaxTotalFrequency = MTFreq; }

  // Interface routine to check if the field that supposed to be loaded in the
  // instruction is only read and its parent structure has no safety data
  // violations.
  bool isReadOnlyFieldAccess(LoadInst *Load);

  bool isPtrToStruct(Value *V);

  bool isFunctionPtr(llvm::StructType *STy, unsigned Idx);

  llvm::StructType *getPtrToStructElementType(Value *V);

  bool isPtrToStructWithI8StarFieldAt(Value *V, unsigned StructIndex);

  bool isPtrToIntOrFloat(Value *V);

  bool hasPtrToIntOrFloatReturnType(Function *F);

  bool isPtrToIntOrFloat(dtrans::FieldInfo &FI);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAnalyzedTypes();
  void printCallInfo();
  void printArraysWithConstantEntriesInformation();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  void PostProcessFieldValueInfo();
  void computeStructFrequency(dtrans::StructInfo *StInfo);

  // Check language specific info that may affect whether certain rules
  // like DTransUseCRuleCompat can be applied.
  void checkLanguages(Module &M);

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

  // A mapping from LoadInst instructions that have been identified as being
  // structure element reads to a type-index pair for the element being read.
  using LoadInfoMapType = DenseMap<Value *, std::pair<llvm::Type *, size_t>>;
  LoadInfoMapType LoadInfoMap;

  // A mapping from StoreInst instructions that have been identified as being
  // structure element writes to a type-index pair for the element being
  // written.
  using StoreInfoMapType = DenseMap<Value *, std::pair<llvm::Type *, size_t>>;
  StoreInfoMapType StoreInfoMap;

  // Set of Load/Store instructions that are accessing more than one structure
  // field.
  using MultiElemLoadStoreSetType = SmallPtrSet<Instruction *, 32>;
  MultiElemLoadStoreSetType MultiElemLoadStoreInfo;

  // Maximum of TotalFrequency from all structures.
  uint64_t MaxTotalFrequency = 0;

  // Indicates DTrans safety information could not be computed because a Value
  // object was encountered that the PointerTypeAnalyzer could not collect
  // information for.
  bool UnhandledPtrType = false;

  // Indicates whether the module was completely analyzed for safety checks.
  bool DTransSafetyAnalysisRan = false;

  // A set of functions where bad cast safety issue validation required in
  // runtime.
  SetVector<Function *> FunctionsRequireBadCastValidation;

  // Indicates that a Fortran function was seen. This will disable
  // DTransUseCRuleCompat.
  bool SawFortran = false;

  DTransStructType *getPtrToStructTy(Value *V);

  DTransType *getFieldTy(StructType *STy, unsigned Idx);

  DTransType *getFieldPETy(StructType *STy, unsigned Idx);

  // Helper utility to handle the related types (types with ABI padding and
  // base).
  std::unique_ptr<DTransRelatedTypesUtils> RelatedTypesUtils;

  // Finalize the analysis for the fields that are arrays with constant entries
  void postProcessArraysWithConstantEntries();

  // Create a dtrans::TypeInfo for the input DTransType
  dtrans::TypeInfo *createTypeInfo(DTransType *Ty);
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
