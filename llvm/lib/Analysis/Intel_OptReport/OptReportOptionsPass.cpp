//===------------------ OptReportOptionsPass.cpp ---------------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// An immutable pass which stores the options for Loop Optimization Report
//
// Detailed description is located at: docs/Intel/OptReport.rst
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "optimization-report-options-pass"

AnalysisKey OptReportOptionsAnalysis::Key;

static cl::opt<OptReportVerbosity::Level> LoopOptReportVerbosityOption(
    "intel-loop-optreport",
    cl::desc("Option for enabling the formation of loop optimization reports "
             "and controling its verbosity"),
    cl::init(OptReportVerbosity::None),
    cl::values(
        clEnumValN(OptReportVerbosity::None, "none",
                   "Loop optimization reports are disabled"),
        clEnumValN(OptReportVerbosity::Low, "low",
                   "Only generate positive remarks, e.g. when "
                   "transformation is triggered"),
        clEnumValN(OptReportVerbosity::Medium, "medium",
                   "Low + generate negative remarks with a "
                   "reason why transformation did't happen"),
        clEnumValN(OptReportVerbosity::High, "high",
                   "Medium + all extra details about the transformations")));

// External storage for opt-report emitter control.
OptReportOptions::LoopOptReportEmitterKind llvm::IntelOptReportEmitter;

// Option for controlling 'backend' for the optimization reports.
static cl::opt<OptReportOptions::LoopOptReportEmitterKind, true>
    OptReportEmitter(
        "intel-loop-optreport-emitter",
        cl::desc("Option for choosing the way compiler outputs the "
                 "optimization reports"),
        cl::location(IntelOptReportEmitter), cl::init(OptReportOptions::None),
        cl::values(
            clEnumValN(OptReportOptions::None, "none",
                       "Optimization reports are not emitted"),
            clEnumValN(
                OptReportOptions::IR, "ir",
                "Optimization reports are emitted right after HIR phase"),
            clEnumValN(
                OptReportOptions::HIR, "hir",
                "Optimization reports are emitted before HIR Code Gen phase"),
            clEnumValN(OptReportOptions::MIR, "mir",
                       "Optimization reports are emitted at the end of "
                       "MIR processing")));

char OptReportOptionsPass::ID = 0;
INITIALIZE_PASS(OptReportOptionsPass, "optimization-report-options-pass",
                "Optimization report options pass", false, true)

OptReportOptions::OptReportOptions()
    : Verbosity(LoopOptReportVerbosityOption) {}

OptReportOptionsPass::OptReportOptionsPass() : ImmutablePass(ID) {
  initializeOptReportOptionsPassPass(*PassRegistry::getPassRegistry());
}

ImmutablePass *llvm::createOptReportOptionsPass() {
  return new OptReportOptionsPass();
}
