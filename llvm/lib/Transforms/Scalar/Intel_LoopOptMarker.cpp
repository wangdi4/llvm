//===----------- Intel_LoopOptMarker.cpp - LoopOpt Marker -----------------===//
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
// This pass sets loopopt based throttling for the function indicating to
// scalar transformations to suppress optimizations which can interfere with
// loopopt.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_LoopOptMarker.h"

#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

#define DEBUG_TYPE "loopopt-marker"

namespace {

class LoopOptMarker {
public:
  void run(Function &F) { F.setPreLoopOpt(); }
};

class LoopOptMarkerLegacyPass : public FunctionPass {
  LoopOptMarker LOM;

public:
  static char ID;

  LoopOptMarkerLegacyPass() : FunctionPass(ID) {
    initializeLoopOptMarkerLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  };

  bool runOnFunction(Function &F) override;
};
} // namespace

PreservedAnalyses LoopOptMarkerPass::run(Function &F,
                                         FunctionAnalysisManager &) {
  LoopOptMarker LOM;
  LOM.run(F);
  return PreservedAnalyses::all();
}

char LoopOptMarkerLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(LoopOptMarkerLegacyPass, "loopopt-marker",
                      "LoopOpt Marker", false, false)
INITIALIZE_PASS_END(LoopOptMarkerLegacyPass, "loopopt-marker", "LoopOpt Marker",
                    false, false)

FunctionPass *llvm::createLoopOptMarkerLegacyPass() {
  return new LoopOptMarkerLegacyPass();
}

bool LoopOptMarkerLegacyPass::runOnFunction(Function &F) {
  LOM.run(F);
  return false;
}
