//===----- HIRLoopDistributionMemRec.cpp - to break memory recurrence -----===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Served as a wrapper to call HIRLoopDistribution
//===----------------------------------------------------------------------===//
//

#include "HIRLoopDistribution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopDistributionForMemRecPass.h"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::distribute;

#define DEBUG_TYPE "hir-loop-distribute"

cl::opt<bool>
    DisableDistMemRec("disable-hir-loop-distribute-memrec",
                      cl::desc("Disable HIR Loop Distribution MemRec"),
                      cl::Hidden, cl::init(false));

PreservedAnalyses HIRLoopDistributionForMemRecPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  if (DisableDistMemRec) {
    LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION Break MemRec disabled\n");
    return PreservedAnalyses::all();
  }

  ModifiedHIR =
      HIRLoopDistribution(
          HIRF, AM.getResult<TargetLibraryAnalysis>(F),
          AM.getResult<TargetIRAnalysis>(F), AM.getResult<HIRDDAnalysisPass>(F),
          AM.getResult<HIRSafeReductionAnalysisPass>(F),
          AM.getResult<HIRSparseArrayReductionAnalysisPass>(F),
          AM.getResult<HIRLoopResourceAnalysis>(F),
          AM.getResult<HIRLoopLocalityAnalysis>(F), DistHeuristics::BreakMemRec)
          .run();

  return PreservedAnalyses::all();
}