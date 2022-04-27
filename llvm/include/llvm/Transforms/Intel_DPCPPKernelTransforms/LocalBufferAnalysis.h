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

// It handles actual analysis and results of local buffer analysis.
class LocalBufferInfo {
public:
  /// A set of local values used by a function
  typedef SmallPtrSet<llvm::GlobalValue *, 16> TUsedLocals;

  /// A mapping between function pointer and the set of local values the
  /// function uses directly.
  typedef DenseMap<const llvm::Function *, TUsedLocals> TUsedLocalsMap;

  LocalBufferInfo(Module *M) { this->M = M; }

  void analyzeModule(CallGraph *CG);

  /// Returns the set of local values used directly by the given function
  /// \param F a function for which should return the local values that
  /// were used by it directly.
  /// \returns the set of local values used directly by the given function.
  const TUsedLocals &getDirectLocals(Function *F) { return LocalUsageMap[F]; }

  /// Returns the size of local buffer used directly by the given function.
  /// \param F given function.
  /// \returns the size of local buffer used directly by the given function.
  size_t getDirectLocalsSize(Function *F) { return DirectLocalSizeMap[F]; }

  /// Returns the size of local buffer used by the given function.
  /// \param F given function.
  /// \returns the size of local buffer used by the given function.
  size_t getLocalsSize(Function *F) { return LocalSizeMap[F]; }

private:
  /// Adds the given local value to the set of used locals of all functions
  /// that are using the given user directly. It recursively searches the first
  /// useres (and users of a users) that are functions.
  /// \param LocalVal local value (which is represented by a global value
  /// with address space 3).
  /// \param U direct user of pLocalVal.
  void updateLocalsMap(GlobalValue *LocalVal, User *U);

  /// Goes over all local values in the module and over all their direct users
  /// and maps between functions and the local values they use.
  /// \param M the module which need to go over its local values.
  void updateDirectLocals(Module &M);

  /// calculate direct local sizes used by functions in the module.
  void calculateDirectLocalsSize();

  /// Iterate all functions in module by postorder traversal, and for each
  /// function, add direct local sizes with the max size of local buffer needed
  /// by all of callees.
  void calculateLocalsSize(CallGraph *CG);

  /// A mapping between function pointer and the local buffer size that the
  /// function uses.
  typedef DenseMap<Function *, size_t> TLocalSizeMap;

  /// The llvm module this pass needs to update.
  Module *M;

  /// Map between function and the local values it uses directly.
  TUsedLocalsMap LocalUsageMap;

  /// Map between function and the local buffer size.
  TLocalSizeMap LocalSizeMap;

  /// Map between function and the local buffer size for local values used
  /// directly by this function.
  TLocalSizeMap DirectLocalSizeMap;
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
