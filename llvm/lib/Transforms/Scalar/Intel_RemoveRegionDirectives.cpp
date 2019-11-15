//==--- Intel_RemoveRegionDirectives.cpp - Implementation of RemoveRegionDirectives pass -*- C++ -*---==//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/Intel_RemoveRegionDirectives.h"

using namespace llvm;
using namespace llvm::vpo;

namespace {

struct RemoveRegionDirectivesLegacyPass : public FunctionPass {
public:
  static char ID;

  RemoveRegionDirectivesLegacyPass() : FunctionPass(ID) {
    initializeRemoveRegionDirectivesLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F);
};
} // anonymous namespace

char RemoveRegionDirectivesLegacyPass::ID = 0;
INITIALIZE_PASS(RemoveRegionDirectivesLegacyPass, "remove-region-directives",
             "Remove llvm.directive.region.*", false, false)

FunctionPass *llvm::createRemoveRegionDirectivesLegacyPass() {
  return new RemoveRegionDirectivesLegacyPass();
}

bool RemoveRegionDirectivesLegacyPass::runOnFunction(Function &F) {
  return VPOUtils::stripDirectives(F);
}

PreservedAnalyses RemoveRegionDirectivesPass::run(Function &F,
                                                  FunctionAnalysisManager &) {
  RemoveRegionDirectivesLegacyPass RRD;
  RRD.runOnFunction(F);
  return PreservedAnalyses::all();
}
