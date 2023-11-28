//===--------------- Intel_DopeVectorTypeAnalysis.h -----------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Provides utility functions for dope vector types using named metadata from
// the Fortran front end.
//

//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_DOPEVECTORTYPEANALYSIS_H
#define LLVM_ANALYSIS_INTEL_DOPEVECTORTYPEANALYSIS_H

#include "llvm/IR/Intel_DopeVectorTypeInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class DopeVectorTypeQueryInfo : public DopeVectorTypeInfo {
public:
  DopeVectorTypeQueryInfo(Module &M) : DopeVectorTypeInfo(M){};
  ~DopeVectorTypeQueryInfo(){};

  // The info in the DopeVectorTypeMap is never invalidated.
  bool invalidate(Module &, const PreservedAnalyses &,
                  ModuleAnalysisManager::Invalidator &) {
    return false;
  }
};

class DopeVectorTypeAnalysis
    : public AnalysisInfoMixin<DopeVectorTypeAnalysis> {
  friend AnalysisInfoMixin<DopeVectorTypeAnalysis>;
  static AnalysisKey Key;

public:
  typedef DopeVectorTypeQueryInfo Result;
  Result run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_ANALYSIS_INTEL_DOPEVECTORTYPEANALYSIS_H
