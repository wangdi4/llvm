//===- BuiltinLibInfoAnalysis.h - Builtin RTL info analysis -----*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BUILTIN_LIB_INFO_ANALYSIS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BUILTIN_LIB_INFO_ANALYSIS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// BuiltinLibInfo holds builtin runtime library.
class BuiltinLibInfo {
public:
  /// Load builtin runtime libraries from command line options.
  void loadBuiltinModules(LLVMContext &Ctx);

  ArrayRef<Module *> getBuiltinModules() { return BuiltinModuleRawPtrs; };

  /// Handle invalidation events in the new pass manager.
  bool invalidate(Module &M, const PreservedAnalyses &PA,
                  ModuleAnalysisManager::Invalidator &Inv);

  void print(raw_ostream &OS) const;

private:
  SmallVector<std::unique_ptr<Module>, 2> BuiltinModules;
  SmallVector<Module *, 2> BuiltinModuleRawPtrs;
};

/// Analysis pass which loads builtin runtime library.
class BuiltinLibInfoAnalysis
    : public AnalysisInfoMixin<BuiltinLibInfoAnalysis> {
  friend AnalysisInfoMixin<BuiltinLibInfoAnalysis>;
  static AnalysisKey Key;

public:
  using Result = BuiltinLibInfo;

  Result run(Module &M, ModuleAnalysisManager &AM);
};

/// Printer pass for BuiltinLibInfoAnalysis.
class BuiltinLibInfoAnalysisPrinter
    : public PassInfoMixin<BuiltinLibInfoAnalysisPrinter> {
  raw_ostream &OS;

public:
  explicit BuiltinLibInfoAnalysisPrinter(raw_ostream &OS) : OS(OS) {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

/// For legacy pass manager.
class BuiltinLibInfoAnalysisLegacy : public ModulePass {
  BuiltinLibInfo BLInfo;

public:
  static char ID;

  BuiltinLibInfoAnalysisLegacy();

  bool runOnModule(Module &M) override {
    BLInfo.loadBuiltinModules(M.getContext());
    return false;
  }

  StringRef getPassName() const override {
    return "BuiltinLibInfoAnalysisLegacy";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  void print(raw_ostream &OS, const Module *) const override {
    BLInfo.print(OS);
  }

  BuiltinLibInfo &getResult() { return BLInfo; }
  const BuiltinLibInfo &getResult() const { return BLInfo; }
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BUILTIN_LIB_INFO_ANALYSIS_H
