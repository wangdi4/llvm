//===--------------------DTransSafetyAnalyzer.h--------------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
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

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {
class BlockFrequencyInfo;
class Function;
class Module;
class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtrans {
class DTransType;
class DTransTypeManager;
class PtrTypeAnalyzer;
class TypeInfo;
class TypeMetadataReader;

// This class holds the results of the safety analysis of the aggregate
// types, and provides the interfaces needed by the transformations to query
// the DTrans information.
class DTransSafetyInfo {
public:
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

  // Collect the safety bits for the structure types
  void analyzeModule(
      Module &M,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      WholeProgramInfo &WPInfo,
      function_ref<BlockFrequencyInfo &(Function &)> GetBFI);

  // Cleanup memory, and set object back to default state.
  void reset();

  void setUnhandledPtrType(Value *V);

  bool getUnhandledPtrType() const { return UnhandledPtrType; }

  // Return true if safety analysis has been run and can be used in
  // transformations.
  bool useDTransSafetyAnalysis() const;

  // Retrieve the DTrans type information entry for the specified type.
  // If there is no entry for the specified type, create one.
  TypeInfo *getOrCreateTypeInfo(DTransType *Ty);

  // Retrieve the DTrans type information entry for the specified type.
  // If there is no entry for the specified type, return nullptr.
  TypeInfo *getTypeInfo(DTransType *Ty) const;

  // Accessor for the set of TypeInfo objects.
  iterator_range<type_info_iterator> type_info_entries() {
    return make_range(type_info_iterator(TypeInfoMap.begin()),
                      type_info_iterator(TypeInfoMap.end()));
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printAnalyzedTypes();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  std::unique_ptr<DTransTypeManager> TM;
  std::unique_ptr<TypeMetadataReader> MDReader;
  std::unique_ptr<PtrTypeAnalyzer> PtrAnalyzer;

  // A mapping from DTransTypes to the TypeInfo object that is used to
  // store information and safety bits about the types.
  DenseMap<DTransType *, TypeInfo *> TypeInfoMap;

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

} // end namespace dtrans

ModulePass *createDTransSafetyAnalyzerTestWrapperPass();

} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSSAFETYANALYZER_H
