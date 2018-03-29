//===------------------ OptReportOptionsPass.cpp ---------------------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
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

char OptReportOptionsPass::ID = 0;
INITIALIZE_PASS(OptReportOptionsPass, "optimization-report-options-pass",
                "Optimization report options pass", false, true)

OptReportOptionsPass::OptReportOptionsPass()
    : ImmutablePass(ID), Impl(LoopOptReportVerbosityOption) {
  initializeOptReportOptionsPassPass(*PassRegistry::getPassRegistry());
}

bool OptReportOptionsPass::doInitialization(Module &M) { return false; }

ImmutablePass *llvm::createOptReportOptionsPass() {
  return new OptReportOptionsPass();
}
