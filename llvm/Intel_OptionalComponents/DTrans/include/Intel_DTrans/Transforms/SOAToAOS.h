//===--------------- SOAToAOS.h - DTransSOAToAOSPass  ---------------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
#error SOAToAOS.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

#include <memory>

namespace llvm {
namespace dtrans {

/// Pass to perform DTrans AOS to SOA optimizations.
class SOAToAOSPass : public PassInfoMixin<SOAToAOSPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool
  runImpl(Module &M, DTransAnalysisInfo &DTInfo,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo);
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Debugging pass to check computation of approximate IR.
struct SOAToAOSApproximationDebugResult;
class SOAToAOSApproximationDebug
    : public AnalysisInfoMixin<SOAToAOSApproximationDebug> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<SOAToAOSApproximationDebug>;
  static char PassID;

public:
  // Called from lit-tests, result is consumed only by lit-tests.
  class Ignore {
    std::unique_ptr<SOAToAOSApproximationDebugResult> Ptr;

  public:
    Ignore(SOAToAOSApproximationDebugResult *Ptr);
    Ignore(Ignore &&Other);
    const SOAToAOSApproximationDebugResult *get() const;
    // Prevent default dtor creation while type is incomplete.
    ~Ignore();
  };
  typedef Ignore Result;

  Result run(Function &F, FunctionAnalysisManager &AM);
};

// Debugging pass to check array method classification.
struct SOAToAOSArrayMethodsCheckDebugResult;
class SOAToAOSArrayMethodsCheckDebug
    : public AnalysisInfoMixin<SOAToAOSArrayMethodsCheckDebug> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<SOAToAOSArrayMethodsCheckDebug>;
  static char PassID;

public:
  // Called from lit-tests, result is consumed only by lit-tests.
  class Ignore {
    std::unique_ptr<SOAToAOSArrayMethodsCheckDebugResult> Ptr;

  public:
    Ignore(SOAToAOSArrayMethodsCheckDebugResult *Ptr);
    Ignore(Ignore &&Other);
    const SOAToAOSArrayMethodsCheckDebugResult *get() const;
    // Prevent default dtor creation while type is incomplete.
    ~Ignore();
  };
  typedef Ignore Result;

  Result run(Function &F, FunctionAnalysisManager &AM);
};

struct SOAToAOSStructMethodsCheckDebugResult;
class SOAToAOSStructMethodsCheckDebug
    : public AnalysisInfoMixin<SOAToAOSStructMethodsCheckDebug> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<SOAToAOSStructMethodsCheckDebug>;
  static char PassID;

public:
  // Called from lit-tests, result is consumed only by lit-tests.
  class Ignore {
    std::unique_ptr<SOAToAOSStructMethodsCheckDebugResult> Ptr;

  public:
    Ignore(SOAToAOSStructMethodsCheckDebugResult *Ptr);
    Ignore(Ignore &&Other);
    const SOAToAOSStructMethodsCheckDebugResult *get() const;
    // Prevent default dtor creation while type is incomplete.
    ~Ignore();
  };
  typedef Ignore Result;

  Result run(Function &F, FunctionAnalysisManager &AM);
};

// This class is used for testing transformations of arrays' methods.
class SOAToAOSArrayMethodsTransformDebug
    : public PassInfoMixin<SOAToAOSArrayMethodsTransformDebug> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

// This class is used for testing transformations of structure's methods.
class SOAToAOSStructMethodsTransformDebug
    : public PassInfoMixin<SOAToAOSStructMethodsTransformDebug> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace dtrans

ModulePass *createDTransSOAToAOSWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOS_H

