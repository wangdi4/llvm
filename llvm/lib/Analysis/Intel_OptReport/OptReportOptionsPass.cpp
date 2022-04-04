//===------------------ OptReportOptionsPass.cpp ---------------------===//
//
// Copyright (C) 2017-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// An immutable pass which stores the options for Optimization Report
//
// Detailed description is located at: docs/Intel/OptReport.rst
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "optimization-report-options-pass"

AnalysisKey OptReportOptionsAnalysis::Key;

static cl::opt<OptReportVerbosity::Level> OptReportVerbosityOption(
    "intel-opt-report",
    cl::desc("Option for enabling the formation of optimization reports "
             "and controling its verbosity"),
    cl::init(OptReportVerbosity::None),
    cl::values(
        clEnumValN(OptReportVerbosity::None, "none",
                   "Optimization reports are disabled"),
        clEnumValN(OptReportVerbosity::Low, "low",
                   "Only generate positive remarks, e.g. when "
                   "transformation is triggered"),
        clEnumValN(OptReportVerbosity::Medium, "medium",
                   "Low + generate negative remarks with a "
                   "reason why transformation did't happen"),
        clEnumValN(OptReportVerbosity::High, "high",
                   "Medium + all extra details about the transformations")));

// External storage for opt-report emitter control.
OptReportOptions::OptReportEmitterKind llvm::IntelOptReportEmitter;

// Option for controlling 'backend' for the optimization reports.
static cl::opt<OptReportOptions::OptReportEmitterKind, true> OptReportEmitter(
    "intel-opt-report-emitter",
    cl::desc("Option for choosing the way compiler outputs the "
             "optimization reports"),
    cl::location(IntelOptReportEmitter), cl::init(OptReportOptions::None),
    cl::values(
        clEnumValN(OptReportOptions::None, "none",
                   "Optimization reports are not emitted"),
        clEnumValN(OptReportOptions::IR, "ir",
                   "Optimization reports are emitted right after HIR phase"),
        clEnumValN(
            OptReportOptions::HIR, "hir",
            "Optimization reports are emitted before HIR Code Gen phase"),
        clEnumValN(OptReportOptions::MIR, "mir",
                   "Optimization reports are emitted at the end of "
                   "MIR processing")));

/// Internal option for setting opt-report output file.
static cl::opt<std::string> OptReportFile(
    "intel-opt-report-file",
    cl::desc("What file to write opt-report, inlining report, and register "
             "allocation report output to. Special values include "
             "'stdout' which writes opt-report to stdout and 'stderr' which "
             "writes to stderr. Default is 'stderr'."),
    cl::init("stderr"));

formatted_raw_ostream &OptReportOptions::getOutputStream() {

  // Use stdout and stderr if requested.
  if (OptReportFile == "stdout")
    return fouts();
  if (OptReportFile == "stderr")
    return ferrs();

  // Attempt to open the specified file; fall back to stderr if that fails.
  static std::error_code Error;
  static raw_fd_ostream File{OptReportFile, Error};
  if (Error) {
    ferrs() << "warning #13022: could not open file '" << OptReportFile
            << "' for optimization report output, reverting to stdout\n";
    OptReportFile = "stdout";
    return getOutputStream();
  }

  // If opening the file succeeded, use it for opt-report output.
  static formatted_raw_ostream FormattedFile{File};
  return FormattedFile;
}

char OptReportOptionsPass::ID = 0;
INITIALIZE_PASS(OptReportOptionsPass, "optimization-report-options-pass",
                "Optimization report options pass", false, true)

OptReportOptions::OptReportOptions() : Verbosity(OptReportVerbosityOption) {}

OptReportOptionsPass::OptReportOptionsPass() : ImmutablePass(ID) {
  initializeOptReportOptionsPassPass(*PassRegistry::getPassRegistry());
}

ImmutablePass *llvm::createOptReportOptionsPass() {
  return new OptReportOptionsPass();
}
