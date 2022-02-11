//===-- VPODirectiveCleanup.cpp-----------------------------------------------------===//
//
//   Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_VPO/VPODirectiveCleanup.h"

#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "VPODirectiveCleanup"

static cl::opt<bool>
    DisableVPODirectiveCleanup("disable-vpo-directive-cleanup", cl::init(false),
                               cl::Hidden,
                               cl::desc("Disable VPO directive cleanup"));


void VPODirectiveCleanup::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addPreserved<AndersensAAWrapperPass>();
  AU.addPreserved<GlobalsAAWrapperPass>();
}

bool VPODirectiveCleanup::runOnFunction(Function &F) { return Impl.runImpl(F); }

PreservedAnalyses VPODirectiveCleanupPass::run(Function &F,
                                               FunctionAnalysisManager &AM) {
  if (!runImpl(F))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<GlobalsAA>();
  PA.preserve<AndersensAA>();
  return PA;
}

bool VPODirectiveCleanupPass::runImpl(Function &F) {
  // Skip if disabled
  if (DisableVPODirectiveCleanup) {
    return false;
  }

  // Remove calls to directive intrinsics since the LLVM back end does not know
  // how to translate them.
  bool Changed = VPOUtils::stripDirectives(F);

  // Unset "may-have-openmp-directive" attribute for the function F, as all
  // directives have been removed.
  VPOUtils::unsetMayHaveOpenmpDirectiveAttribute(F);

  // In the future, we can also run CFG Simplify when `Changed` is false, but
  // unsetMayHaveOpenmpDirectiveAttribute() returned true, as it will be able to
  // do optimizations which were previously prevented by the attribute.
  if (!Changed)
    return false;

  // TODO : The following piece of code still uses legacy pass manager
  // This needs to be changed
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

INITIALIZE_PASS_BEGIN(VPODirectiveCleanup, "VPODirectiveCleanup",
                      "VPO Directive Cleanup", false, false)
INITIALIZE_PASS_END(VPODirectiveCleanup, "VPODirectiveCleanup",
                    "VPO Directive Cleanup", false, false)

char VPODirectiveCleanup::ID = 0;

FunctionPass *llvm::createVPODirectiveCleanupPass() {
  return new VPODirectiveCleanup();
}
