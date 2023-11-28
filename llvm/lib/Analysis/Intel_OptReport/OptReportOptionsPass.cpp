//===------------------ OptReportOptionsPass.cpp ---------------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
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
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"

using namespace llvm;

#define DEBUG_TYPE "optimization-report-options-pass"

AnalysisKey OptReportOptionsAnalysis::Key;

bool OptReportOptions::ShouldCloseOptReport = false;

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

/// Internal option to force absolute path printing on/off.
static cl::opt<bool> ForceAbsolutePaths(
    "intel-opt-report-use-absolute-paths",
    cl::desc("Whether to use absolute paths in optimization reports. If not "
             "set, this is determined automatically from the module."));

bool llvm::optReportShouldUseAbsolutePathsInModule(Module &M) {

  // Follow the flag if set.
  if (ForceAbsolutePaths.getNumOccurrences() > 0)
    return ForceAbsolutePaths;

  // Otherwise, check for compile unit metadata. Absolute paths should be used
  // if any of these is an absolute path.
  for (DICompileUnit *CU : M.debug_compile_units())
    if (sys::path::is_absolute(CU->getFilename()))
      return true;

  // Otherwise, absolute paths should be used if the module's source file name
  // is an absolute path.
  return sys::path::is_absolute(M.getSourceFileName());
}

raw_fd_ostream &OptReportOptions::getOutputStream() {

  // Use stdout and stderr if requested.
  if (OptReportFile == "stdout")
    return outs();
  if (OptReportFile == "stderr")
    return errs();

  // Attempt to open the specified file; fall back to stdout if that fails.
  static std::error_code Error;
  static raw_fd_ostream File{OptReportFile, Error};
  if (Error) {
    errs() << "warning #13022: could not open file '" << OptReportFile
           << "' for optimization report output, reverting to stdout\n";
    OptReportFile = "stdout";
    return getOutputStream();
  }

  // If opening the file succeeded, use it for opt-report output.
  ShouldCloseOptReport = true;
  return File;
}

char OptReportOptionsPass::ID = 0;
INITIALIZE_PASS(OptReportOptionsPass, "optimization-report-options-pass",
                "Optimization report options pass", false, true)

OptReportOptions::OptReportOptions(bool AbsolutePaths)
    : Verbosity(OptReportVerbosityOption), AbsolutePaths(AbsolutePaths) {}

OptReportOptionsPass::OptReportOptionsPass() : ImmutablePass(ID) {
  initializeOptReportOptionsPassPass(*PassRegistry::getPassRegistry());
}

ImmutablePass *llvm::createOptReportOptionsPass() {
  return new OptReportOptionsPass();
}
