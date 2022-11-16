//===- LocalBufferAnalysis.h - DPC++ kernel local buffer analysis ---------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LOCAL_BUFF_ANALYSIS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LOCAL_BUFF_ANALYSIS_H

#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {
class LocalBufferInfoImpl;

// It handles actual analysis and results of local buffer analysis.
class LocalBufferInfo {
  std::unique_ptr<LocalBufferInfoImpl> Impl;

public:
  /// A set of local values used by a function
  using TUsedLocals = SmallPtrSet<llvm::GlobalValue *, 16>;

  /// A mapping between function pointer and the set of local values the
  /// function uses directly.
  using TUsedLocalsMap = DenseMap<const llvm::Function *, TUsedLocals>;

  LocalBufferInfo(Module *M, CallGraph *CG);
  LocalBufferInfo(LocalBufferInfo &&Other);
  LocalBufferInfo &operator=(LocalBufferInfo &&Other);
  ~LocalBufferInfo();

  /// Returns the set of local values used directly by the given function
  /// \param F a function for which should return the local values that
  /// were used by it directly.
  /// \returns the set of local values used directly by the given function.
  const TUsedLocals &getDirectLocals(Function *F);

  /// Returns the map from function to the set of local values the function
  /// uses directly.
  TUsedLocalsMap &getDirectLocalsMap();

  /// Returns the size of local buffer used directly by the given function.
  /// \param F given function.
  /// \returns the size of local buffer used directly by the given function.
  size_t getDirectLocalsSize(Function *F);

  /// Returns the size of local buffer used by the given function.
  /// \param F given function.
  /// \returns the size of local buffer used by the given function.
  size_t getLocalsSize(Function *F);
};

/// Provide information about the local values each function uses directly.
/// It goes over all local values and over all their direct users and maps
/// between functions and the local values they uses.
// Analysis pass providing a never-invalidated whole program analysis result.
class LocalBufferAnalysis : public AnalysisInfoMixin<LocalBufferAnalysis> {
  friend AnalysisInfoMixin<LocalBufferAnalysis>;
  static AnalysisKey Key;

public:
  typedef LocalBufferInfo Result;

  LocalBufferInfo run(Module &M, AnalysisManager<Module> &AM);
};

// Legacy wrapper pass to provide the LocalBufferInfo object.
class LocalBufferAnalysisLegacy : public ModulePass {
  std::unique_ptr<LocalBufferInfo> Result;

public:
  /// Pass identification, replacement for typeid
  static char ID;

  LocalBufferAnalysisLegacy();

  llvm::StringRef getPassName() const override {
    return "LocalBufferAnalysisLegacy";
  }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
    // Analysis pass preserve all
    AU.setPreservesAll();
  }

  LocalBufferInfo &getResult() { return *Result; }
  const LocalBufferInfo &getResult() const { return *Result; }
};

} // namespace llvm

#endif
