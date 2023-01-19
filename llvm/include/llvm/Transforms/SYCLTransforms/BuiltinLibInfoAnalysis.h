//===- BuiltinLibInfoAnalysis.h - Builtin RTL info analysis -----*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_BUILTIN_LIB_INFO_ANALYSIS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_BUILTIN_LIB_INFO_ANALYSIS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/Utils/RuntimeService.h"

namespace llvm {

/// BuiltinLibInfo holds builtin modules and runtime service.
class BuiltinLibInfo {
public:
  BuiltinLibInfo(ArrayRef<Module *> BuiltinModules) : RTS(BuiltinModules) {
    BuiltinModuleRawPtrs.assign(BuiltinModules.begin(), BuiltinModules.end());
  }

  /// Load builtin modules from path specified by command line option.
  /// This is only used for opt.
  void loadBuiltinModules(Module &M);

  ArrayRef<Module *> getBuiltinModules() { return BuiltinModuleRawPtrs; };

  const RuntimeService &getRuntimeService() const { return RTS; }
  RuntimeService &getRuntimeService() { return RTS; }

  /// Handle invalidation events in the new pass manager.
  bool invalidate(Module &M, const PreservedAnalyses &PA,
                  ModuleAnalysisManager::Invalidator &Inv);

  void print(raw_ostream &OS) const;

private:
  /// Vector of modules with built-in function implementations.
  SmallVector<std::unique_ptr<Module>, 2> BuiltinModules;
  SmallVector<Module *, 2> BuiltinModuleRawPtrs;

  /// Runtime service.
  RuntimeService RTS;
};

/// Analysis pass which loads builtin runtime library.
class BuiltinLibInfoAnalysis
    : public AnalysisInfoMixin<BuiltinLibInfoAnalysis> {
  friend AnalysisInfoMixin<BuiltinLibInfoAnalysis>;
  static AnalysisKey Key;
  SmallVector<Module *, 2> BuiltinModules;

public:
  BuiltinLibInfoAnalysis(ArrayRef<Module *> BuiltinModules = {}) {
    this->BuiltinModules.assign(BuiltinModules.begin(), BuiltinModules.end());
  }

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

} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_BUILTIN_LIB_INFO_ANALYSIS_H
