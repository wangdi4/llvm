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
class DTransTypeManager;
class PtrTypeAnalyzer;
class TypeMetadataReader;

// This class holds the results of the safety analysis of the aggregate
// types, and provides the interfaces needed by the transformations to query
// the DTrans information.
class DTransSafetyInfo {
public:
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

  // Return true if safety analysis has been run and can be used in
  // transformations.
  bool useDTransSafetyAnalysis() const;

private:
  std::unique_ptr<DTransTypeManager> TM;
  std::unique_ptr<TypeMetadataReader> MDReader;
  std::unique_ptr<PtrTypeAnalyzer> PtrAnalyzer;

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
