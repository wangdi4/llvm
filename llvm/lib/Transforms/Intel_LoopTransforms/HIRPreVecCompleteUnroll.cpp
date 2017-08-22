//===------- HIRPreVecCompleteUnroll.cpp - pre vec complete unroll --------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Wrapper over HIRCompleteUnroll pass. This is executed before vectorizer. The
// idea is to unroll loopnests which seems to be highly profitable before they
// get to the vectorizer so the profitability threshold is set to a higher
// value.
//===----------------------------------------------------------------------===//
//

#include "HIRCompleteUnroll.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace llvm::loopopt;

cl::opt<bool> DisablePreVecUnroll("disable-hir-pre-vec-complete-unroll",
                                  cl::desc("Disables pre vec complete unroll"),
                                  cl::Hidden, cl::init(false));

namespace {

class HIRPreVecCompleteUnroll : public HIRCompleteUnroll {

public:
  static char ID;

  HIRPreVecCompleteUnroll(unsigned OptLevel = 0)
      : HIRCompleteUnroll(ID, OptLevel, true) {
    initializeHIRPreVecCompleteUnrollPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (DisablePreVecUnroll) {
      return false;
    }
    return HIRCompleteUnroll::runOnFunction(F);
  }
};
}

char HIRPreVecCompleteUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(HIRPreVecCompleteUnroll, "hir-pre-vec-complete-unroll",
                      "HIR PreVec Complete Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
INITIALIZE_PASS_END(HIRPreVecCompleteUnroll, "hir-pre-vec-complete-unroll",
                    "HIR PreVec Complete Unroll", false, false)

FunctionPass *llvm::createHIRPreVecCompleteUnrollPass(unsigned OptLevel) {
  return new HIRPreVecCompleteUnroll(OptLevel);
}
