//===- Intel_FunctionRecognizer.cpp - Function Recognizer -------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-------------------------------------------------------------------===//
//
// This pass recognizes Functions and marks them with appropriate Function
// attributes.
//
//===-------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_FunctionRecognizer.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "functionrecognizer"

// Option to force DTransAnalysis to be available for Function Recognizer.
// Useful for LIT tests.
static cl::opt<bool>
    FunctionRecognizerForceDTrans("intel-functionrecognizer-force-dtrans",
                                  cl::init(false), cl::ReallyHidden);

STATISTIC(NumFunctionsRecognized, "Number of Functions Recognized");

static bool FunctionRecognizerImpl(Function &F) { return false; }

namespace {

struct FunctionRecognizerLegacyPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  FunctionRecognizerLegacyPass(void) : FunctionPass(ID) {
    initializeFunctionRecognizerLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;
    return FunctionRecognizerImpl(F);
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

} // namespace

char FunctionRecognizerLegacyPass::ID = 0;
INITIALIZE_PASS(FunctionRecognizerLegacyPass, "functionrecognizer",
                "Function Recognizer", false, false)

FunctionPass *llvm::createFunctionRecognizerLegacyPass(void) {
  return new FunctionRecognizerLegacyPass();
}

PreservedAnalyses FunctionRecognizerPass::run(Function &F,
                                              FunctionAnalysisManager &AM) {
  FunctionRecognizerImpl(F);
  return PreservedAnalyses::all();
}
