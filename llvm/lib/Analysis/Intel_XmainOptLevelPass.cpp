//===------------------ Intel_XmainOptLevelPass.cpp -----------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This is an immutable pass which stores the opt level.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "xmain-opt-level"

static cl::opt<unsigned>
    ForceXmainOptLevel("xmain-opt-level", cl::init(-1), cl::Hidden,
                       cl::desc("Command line option to set opt level"));

AnalysisKey XmainOptLevelAnalysis::Key;

XmainOptLevel::XmainOptLevel(unsigned OptLevel)
    : OptLevel((ForceXmainOptLevel == unsigned(-1)) ? OptLevel
                                                    : ForceXmainOptLevel) {}

typedef XmainOptLevel Result;
// Run at module level
Result XmainOptLevelAnalysis::run(Module &M, ModuleAnalysisManager &AM) {
  return XmainOptLevel(2);
}

// Run at function level
Result XmainOptLevelAnalysis::run(Function &F, FunctionAnalysisManager &AM) {
  // If XmainOptLevelAnalysisInit has already been called for the parent module,
  // then return the opt-level based on that.
  if (auto *MXOL = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F)
                       .getCachedResult<XmainOptLevelAnalysis>(*F.getParent()))
    return MXOL->getOptLevel();

  return XmainOptLevel(2);
}

// Helper functions for initializing XmainOptLevel in the new pass manager

// Run at function level
PreservedAnalyses XmainOptLevelAnalysisInit::run(Function &F,
                                                 FunctionAnalysisManager &AM) {
  AM.getResult<XmainOptLevelAnalysis>(F).setOptLevel(OptLevel);
  return PreservedAnalyses::all();
}

// Run at module level
PreservedAnalyses XmainOptLevelAnalysisInit::run(Module &M,
                                                 ModuleAnalysisManager &AM) {
  AM.getResult<XmainOptLevelAnalysis>(M).setOptLevel(OptLevel);
  return PreservedAnalyses::all();
}
