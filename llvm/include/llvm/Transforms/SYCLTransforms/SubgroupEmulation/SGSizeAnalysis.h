//=----- SGSizeAnalysis.h - Analyze emulation size for functions- C++ -*-----=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SUBGROUP_EMULATION_SIZE_ANALYSIS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SUBGROUP_EMULATION_SIZE_ANALYSIS_H

#include "llvm/IR/PassManager.h"

#include <map>
#include <set>

namespace llvm {

class SGSizeInfo {
public:
  void analyzeModule(Module &M);

  void print(raw_ostream &OS) const;

  bool hasEmuSize(Function *F) const { return FuncToEmuSizes.count(F); }

  const std::set<unsigned> &getEmuSizes(Function *F) const {
    assert(hasEmuSize(F) && "Function doesn't have emulation size");

    auto Element = FuncToEmuSizes.find(F);
    return Element->second;
  }

private:
  std::map<Function *, std::set<unsigned>> FuncToEmuSizes;
};

class SGSizeAnalysisPass : public AnalysisInfoMixin<SGSizeAnalysisPass> {
public:
  using Result = SGSizeInfo;

  SGSizeInfo run(Module &M, ModuleAnalysisManager &AM);

private:
  friend AnalysisInfoMixin<SGSizeAnalysisPass>;

  static AnalysisKey Key;
};

/// Printer pass for SGSizeAnalysisPass
class SGSizeAnalysisPrinter : public PassInfoMixin<SGSizeAnalysisPrinter> {
public:
  explicit SGSizeAnalysisPrinter(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

private:
  raw_ostream &OS;
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_SUBGROUP_EMULATION_SIZE_ANALYSIS_H
