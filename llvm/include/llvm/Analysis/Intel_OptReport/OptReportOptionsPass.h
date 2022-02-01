//===-------------------- OptReportOptionsPass.h --------------------------===//
//
// Copyright (C) 2018-2021 Intel Corporation. All rights reserved.
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
#include "llvm/Support/FormattedStream.h"

namespace llvm {

class OptReportOptions {
  OptReportVerbosity::Level Verbosity;

public:
  OptReportOptions();

  enum OptReportEmitterKind { None, IR, HIR, MIR /*, vtune */ };

  bool isOptReportOn() const { return Verbosity > 0; }
  OptReportVerbosity::Level getVerbosity() const { return Verbosity; }

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
  static formatted_raw_ostream &getOutputStream();
};

extern OptReportOptions::OptReportEmitterKind IntelOptReportEmitter;

class OptReportOptionsAnalysis
    : public AnalysisInfoMixin<OptReportOptionsAnalysis> {
  friend AnalysisInfoMixin<OptReportOptionsAnalysis>;
  static AnalysisKey Key;

public:
  typedef OptReportOptions Result;

  OptReportOptionsAnalysis() {}

  Result run(Function &, FunctionAnalysisManager &) {
    return OptReportOptions();
  }

  Result run(Module &, ModuleAnalysisManager &) {
    return OptReportOptions();
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

  bool doInitialization(Module &M) override { return false; }

  bool isOptReportOn() const { return Impl.isOptReportOn(); }
  OptReportVerbosity::Level getVerbosity() const {
    return Impl.getVerbosity();
  }
};

ImmutablePass *createOptReportOptionsPass();

} // namespace llvm

#endif
