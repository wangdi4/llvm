//===-- VPODriver.cpp -----------------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPO vectorizer driver pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Intel_VPO/VPOPasses.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRVectVLSAnalysis.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "VPODriver"

static cl::opt<bool>
    DisableVPODirectiveCleanup("disable-vpo-directive-cleanup", cl::init(false),
                               cl::Hidden,
                               cl::desc("Disable VPO directive cleanup"));

using namespace llvm;
using namespace llvm::vpo;

namespace {
class VPODirectiveCleanup : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  VPODirectiveCleanup() : FunctionPass(ID) {
    initializeVPODirectiveCleanupPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  //  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // namespace

INITIALIZE_PASS_BEGIN(VPODirectiveCleanup, "VPODirectiveCleanup",
                      "VPO Directive Cleanup", false, false)
INITIALIZE_PASS_END(VPODirectiveCleanup, "VPODirectiveCleanup",
                    "VPO Directive Cleanup", false, false)

char VPODirectiveCleanup::ID = 0;

FunctionPass *llvm::createVPODirectiveCleanupPass() {
  return new VPODirectiveCleanup();
}

#if 0
void VPODirectiveCleanup::getAnalysisUsage(AnalysisUsage &AU) const {
}
#endif

bool VPODirectiveCleanup::runOnFunction(Function &F) {

  // Skip if disabled
  if (DisableVPODirectiveCleanup) {
    return false;
  }

  // Remove calls to directive intrinsics since the LLVM back end does not know
  // how to translate them.
  if (!VPOUtils::stripDirectives(F)) {
    // If nothing happens, simply return.
    return false;
  }

  // Set up a function pass manager so that we can run some cleanup transforms
  // on the LLVM IR after code gen.
  Module *M = F.getParent();
  legacy::FunctionPassManager FPM(M);

  // It is possible that stripDirectives call
  // eliminates all instructions in a basic block except for the branch
  // instruction. Use CFG simplify to eliminate them.
  FPM.add(createCFGSimplificationPass());
  FPM.run(F);

  return true;
}


