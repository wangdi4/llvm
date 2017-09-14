//===----------------- DTransAnalysis.h - DTrans Analysis -----------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Data Transformation Analysis
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_ANALYSIS_INTEL_DTRANS_DTRANSANALYSIS_H
#define LLVM_ANALYSIS_INTEL_DTRANS_DTRANSANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/Intel_DTrans/DTrans.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class TargetLibraryInfo;

class DTransAnalysisInfo {
public:
  DTransAnalysisInfo();
  ~DTransAnalysisInfo();

  bool analyzeModule(Module &M, TargetLibraryInfo &TLI);
  void reset();

  /// Return true if we are interested in tracking values of the specified type.
  bool isTypeOfInterest(llvm::Type *Ty);

  /// Retrieve the DTrans type information entry for an array of element of
  /// the specified LLVM type.  An LLVM array type is generated if necessary.
  /// If there is no entry for the specified type, create one.
  dtrans::TypeInfo *getOrCreateTypeInfoForArray(llvm::Type *Ty, uint64_t Num);

  /// Retrieve the DTrans type information entry for the specified LLVM type.
  /// If there is no entry for the specified type, create one.
  dtrans::TypeInfo *getOrCreateTypeInfo(llvm::Type *Ty);

  /// Retrieve the DTrans type information entry for the specified LLVM type.
  /// If there is no entry for the specified type, return nullptr.
  dtrans::TypeInfo *getTypeInfo(llvm::Type *Ty) const;

private:
  void printStructInfo(dtrans::StructInfo *AI);
  void printArrayInfo(dtrans::ArrayInfo *AI);

  DenseMap<llvm::Type *, dtrans::TypeInfo *> TypeInfoMap;
};

// Analysis pass providing a data transformation analysis result.
class DTransAnalysis : public AnalysisInfoMixin<DTransAnalysis> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<DTransAnalysis>;
  static char PassID;

public:
  typedef DTransAnalysisInfo Result;

  Result run(Module &M, AnalysisManager<Module> &AM);
};

// Legacy wrapper pass to provide DTrans analysis.
class DTransAnalysisWrapper : public ModulePass {
private:
  DTransAnalysisInfo Result;

public:
  static char ID;

  DTransAnalysisWrapper();

  DTransAnalysisInfo &getDTransInfo() { return Result; }

  bool runOnModule(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

ModulePass *createDTransAnalysisWrapperPass();

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_DTRANS_DTRANSANALYSIS_H
