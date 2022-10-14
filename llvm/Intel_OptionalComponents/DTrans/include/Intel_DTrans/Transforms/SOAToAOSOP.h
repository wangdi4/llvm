//===------ SOAToAOSOP.h - DTransSOAToAOSOPPass for opaque pointers -------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Structure of Arrays to Array of Structures
// data layout optimization pass with support for IR using either opaque or
// non-opaque pointers.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSOP_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSOP_H

#if !INTEL_FEATURE_SW_DTRANS
#error SOAToAOSOP.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#include "llvm/IR/PassManager.h"

namespace llvm {
class Module;
class WholeProgramInfo;
class TargetLibraryInfo;
class DominatorTree;

namespace dtransOP {
class DTransSafetyInfo;

/// Pass to perform DTrans SOA to AOS optimizations.
class SOAToAOSOPPass : public PassInfoMixin<SOAToAOSOPPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool
  runImpl(Module &M, DTransSafetyInfo &DTInfo, WholeProgramInfo &WPInfo,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
	  std::function<llvm::DominatorTree&(llvm::Function&)> GetDT);
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Debugging pass to check computation of approximate IR.
struct SOAToAOSOPApproximationDebugResult;
class SOAToAOSOPApproximationDebug
    : public AnalysisInfoMixin<SOAToAOSOPApproximationDebug> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<SOAToAOSOPApproximationDebug>;
  static char PassID;

public:
  // Called from lit-tests, result is consumed only by lit-tests.
  class Ignore {
    std::unique_ptr<SOAToAOSOPApproximationDebugResult> Ptr;

  public:
    Ignore(SOAToAOSOPApproximationDebugResult *Ptr);
    Ignore(Ignore &&Other);

    // This class is using unique_ptr, so it does not make sense to allow
    // copying or assignment.
    Ignore(const Ignore &Other) = delete;
    Ignore &operator=(const Ignore &) = delete;
    Ignore &operator=(Ignore &&) = delete;

    const SOAToAOSOPApproximationDebugResult *get() const;
    // Prevent default dtor creation while type is incomplete.
    ~Ignore();

    bool invalidate(Module &M, const PreservedAnalyses &PA,
                    ModuleAnalysisManager::Invalidator &Inv) {
      return false;
    }
  };
  typedef Ignore Result;

  Result run(Module &M, ModuleAnalysisManager &MAM);
};

// Debugging pass to check array method classification.
struct SOAToAOSOPArrayMethodsCheckDebugResult;
class SOAToAOSOPArrayMethodsCheckDebug
    : public AnalysisInfoMixin<SOAToAOSOPArrayMethodsCheckDebug> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<SOAToAOSOPArrayMethodsCheckDebug>;
  static char PassID;

public:
  // Called from lit-tests, result is consumed only by lit-tests.
  class Ignore {
    std::unique_ptr<SOAToAOSOPArrayMethodsCheckDebugResult> Ptr;

  public:
    Ignore(SOAToAOSOPArrayMethodsCheckDebugResult *Ptr);
    Ignore(Ignore &&Other);

    // This class is using unique_ptr, so it does not make sense to allow
    // copying or assignment.
    Ignore(const Ignore &Other) = delete;
    Ignore &operator=(const Ignore &) = delete;
    Ignore &operator=(Ignore &&) = delete;

    const SOAToAOSOPArrayMethodsCheckDebugResult *get() const;
    // Prevent default dtor creation while type is incomplete.
    ~Ignore();
  };
  typedef Ignore Result;

  Result run(Module &M, ModuleAnalysisManager &MAM);
};

struct SOAToAOSOPStructMethodsCheckDebugResult;
class SOAToAOSOPStructMethodsCheckDebug
    : public AnalysisInfoMixin<SOAToAOSOPStructMethodsCheckDebug> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<SOAToAOSOPStructMethodsCheckDebug>;
  static char PassID;

public:
  // Called from lit-tests, result is consumed only by lit-tests.
  class Ignore {
    std::unique_ptr<SOAToAOSOPStructMethodsCheckDebugResult> Ptr;

  public:
    Ignore(SOAToAOSOPStructMethodsCheckDebugResult *Ptr);
    Ignore(Ignore &&Other);
    const SOAToAOSOPStructMethodsCheckDebugResult *get() const;
    // Prevent default dtor creation while type is incomplete.
    ~Ignore();

    // This class is using unique_ptr, so it does not make sense to allow
    // copying or assignment.
    Ignore(const Ignore &Other) = delete;
    Ignore &operator=(const Ignore &) = delete;
    Ignore &operator=(Ignore &&) = delete;
  };
  typedef Ignore Result;

  Result run(Module &M, ModuleAnalysisManager &MAM);
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

} // namespace dtransOP

ModulePass *createDTransSOAToAOSOPWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSOP_H
