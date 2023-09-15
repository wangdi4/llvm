//===-------------------- OptReportOptionsPass.h --------------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// An immutable pass, which stores the options for Optimization Report
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_OPT_REPORT_OPTIONS_H
#define LLVM_ANALYSIS_OPT_REPORT_OPTIONS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"

namespace llvm {

bool optReportShouldUseAbsolutePathsInModule(Module &);

class OptReportOptions {
  OptReportVerbosity::Level Verbosity;

  /// Whether opt-report filenames should be printed using absolute paths.
  bool AbsolutePaths;

  friend class OptReportOptionsPass;

  static bool ShouldCloseOptReport;

public:
  OptReportOptions(bool AbsolutePaths = false);

  enum OptReportEmitterKind { None, IR, HIR, MIR /*, vtune */ };

  bool isOptReportOn() const { return Verbosity > 0; }
  OptReportVerbosity::Level getVerbosity() const { return Verbosity; }
  bool shouldPrintAbsolutePaths() const { return AbsolutePaths; }

  /// Handle invalidation from the pass manager.
  ///
  /// This analysis result remains valid always.
  bool invalidate(Module &, const PreservedAnalyses &,
                  ModuleAnalysisManager::Invalidator &) {
    return false;
  }
  bool invalidate(Function &, const PreservedAnalyses &,
                  FunctionAnalysisManager::Invalidator &) {
    return false;
  }

  /// Returns a formatted ostream that opt-report output should be written to.
  static raw_fd_ostream &getOutputStream();

  /// Indicates opt report should be closed explicitly, at least with Windows
  /// LTO
  static bool shouldCloseOptReport() { return ShouldCloseOptReport; }
};

extern OptReportOptions::OptReportEmitterKind IntelOptReportEmitter;

class OptReportOptionsAnalysis
    : public AnalysisInfoMixin<OptReportOptionsAnalysis> {
  friend AnalysisInfoMixin<OptReportOptionsAnalysis>;
  static AnalysisKey Key;

public:
  typedef OptReportOptions Result;

  OptReportOptionsAnalysis() {}

  Result run(Function &F, FunctionAnalysisManager &) {
    return OptReportOptions(
        optReportShouldUseAbsolutePathsInModule(*F.getParent()));
  }

  Result run(Module &M, ModuleAnalysisManager &) {
    return OptReportOptions(optReportShouldUseAbsolutePathsInModule(M));
  }
};

class OptReportOptionsPass : public ImmutablePass {
public:
  static char ID;

  OptReportOptions Impl;

  OptReportOptionsPass();
  OptReportOptionsPass(const OptReportOptionsPass &) = delete;
  OptReportOptionsPass &operator=(const OptReportOptionsPass &) = delete;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool doInitialization(Module &M) override {
    Impl.AbsolutePaths = optReportShouldUseAbsolutePathsInModule(M);
    return false;
  }

  bool isOptReportOn() const { return Impl.isOptReportOn(); }
  OptReportVerbosity::Level getVerbosity() const {
    return Impl.getVerbosity();
  }
};

ImmutablePass *createOptReportOptionsPass();

} // namespace llvm

#endif
