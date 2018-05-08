//===-------------------- OptReportOptionsPass.h ---------------------===//
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
// An immutable pass, which stores the options for Loop Optimization Report
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_OPT_REPORT_OPTIONS_H
#define LLVM_ANALYSIS_OPT_REPORT_OPTIONS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"

#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"

namespace llvm {

class OptReportOptions {

  OptReportVerbosity::Level LoopOptReportVerbosityLevel;

public:
  enum LoopOptReportEmitterKind { None, IR, HIR, MIR /*, vtune */ };

  OptReportOptions(OptReportVerbosity::Level Level)
      : LoopOptReportVerbosityLevel(Level){};

  bool isLoopOptReportOn() const { return LoopOptReportVerbosityLevel > 0; }
  OptReportVerbosity::Level getLoopOptReportVerbosity() const {
    return LoopOptReportVerbosityLevel;
  }
};

extern OptReportOptions::LoopOptReportEmitterKind IntelOptReportEmitter;

class OptReportOptionsAnalysis
    : public AnalysisInfoMixin<OptReportOptionsAnalysis> {
  friend AnalysisInfoMixin<OptReportOptionsAnalysis>;
  static AnalysisKey Key;

  OptReportVerbosity::Level Verbosity;

public:
  typedef OptReportOptions Result;

  OptReportOptionsAnalysis()
      : OptReportOptionsAnalysis(OptReportVerbosity::None) {}
  OptReportOptionsAnalysis(OptReportVerbosity::Level Level)
      : Verbosity(Level) {}

  Result run(Function &, FunctionAnalysisManager &) {
    return OptReportOptions(Verbosity);
  }

  static void registerAnalysis(FunctionAnalysisManager &AM, OptReportVerbosity::Level Verbosity) {
    AM.registerPass([&]() { return OptReportOptionsAnalysis(Verbosity); });
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

  bool doInitialization(Module &M) override;

  bool isLoopOptReportOn() const { return Impl.isLoopOptReportOn(); }
  OptReportVerbosity::Level getLoopOptReportVerbosity() const {
    return Impl.getLoopOptReportVerbosity();
  }
};

ImmutablePass *createOptReportOptionsPass();

} // namespace llvm

#endif
