//===--------------- SOAToAOS.h - DTransSOAToAOSPass  ---------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Structure of Arrays to Array of Structures
// data layout optimization pass.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOS_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOS_H

#if !INTEL_INCLUDE_DTRANS
#error DTrans.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
namespace dtrans {

/// Pass to perform DTrans AOS to SOA optimizations.
class SOAToAOSPass : public PassInfoMixin<SOAToAOSPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransAnalysisInfo &DTInfo,
               const TargetLibraryInfo &TLI);
};
} // namespace dtrans

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Debugging pass to check computation of approximate IR.
class SOAToAOSApproximationDebug
    : public AnalysisInfoMixin<SOAToAOSApproximationDebug> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<SOAToAOSApproximationDebug>;
  static char PassID;

public:
  // Called from lit-tests, result is ignored and not consumed ever.
  struct Ignore {};
  typedef Ignore Result;

  Result run(Function &F, FunctionAnalysisManager &AM);
};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

ModulePass *createDTransSOAToAOSWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOS_H

