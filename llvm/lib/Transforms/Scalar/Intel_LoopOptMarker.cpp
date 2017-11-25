//===----------- Intel_LoopOptMarker.cpp - LoopOpt Marker -----------------===//
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
// This pass sets loopopt based throttling for the function indicating to 
// scalar transformations to suppress optimizations which can interfere with
// loopopt.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

#define DEBUG_TYPE "loopopt-marker"

namespace {

class LoopOptMarker : public FunctionPass {
public:
  static char ID;

  LoopOptMarker() : FunctionPass(ID) {
    initializeLoopOptMarkerPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  };

  bool runOnFunction(Function &F) override;
};
}

char LoopOptMarker::ID = 0;
INITIALIZE_PASS_BEGIN(LoopOptMarker, "loopopt-marker", "LoopOpt Marker", false,
                      false)
INITIALIZE_PASS_END(LoopOptMarker, "loopopt-marker", "LoopOpt Marker", false,
                    false)

FunctionPass *llvm::createLoopOptMarkerPass() {
  return new LoopOptMarker();
}

bool LoopOptMarker::runOnFunction(Function &F) {
  F.setPreLoopOpt();
  // TODO: also add opt level information for loopopt.
  return false;
}
