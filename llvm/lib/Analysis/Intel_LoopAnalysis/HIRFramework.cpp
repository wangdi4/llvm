//===----- HIRFramework.cpp - public interface for HIR framework ----------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIRFramework pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/IR/Intel_LoopIR/HIRVerifier.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/SymbaseAssignment.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-framework"

static cl::opt<bool>
    HIRVerify("hir-verify",
              cl::desc("Verify HIR after each transformation (default=true)"),
              cl::init(true));

INITIALIZE_PASS_BEGIN(HIRFramework, "hir-framework", "HIR Framework", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_DEPENDENCY(SymbaseAssignment)
INITIALIZE_PASS_END(HIRFramework, "hir-framework", "HIR Framework", false, true)

char HIRFramework::ID = 0;

FunctionPass *llvm::createHIRFrameworkPass() { return new HIRFramework(); }

HIRFramework::HIRFramework() : FunctionPass(ID) {
  initializeHIRFrameworkPass(*PassRegistry::getPassRegistry());
}

void HIRFramework::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRParser>();
  AU.addRequiredTransitive<SymbaseAssignment>();
}

bool HIRFramework::runOnFunction(Function &F) {
  HIRP = &getAnalysis<HIRParser>();
  SA = &getAnalysis<SymbaseAssignment>();

  HIRUtils::setHIRFramework(this);

  HLNodeUtils::initialize();
  HLNodeUtils::initTopSortNum();

#ifndef NDEBUG
  verifyAnalysis();
#endif

  return false;
}

void HIRFramework::print(raw_ostream &OS, const Module *M) const {
  HIRP->print(OS);
}

void HIRFramework::verifyAnalysis() const {
  if (HIRVerify) {
    HIRVerifier::verifyAll();
    DEBUG(dbgs() << "Verification of HIR done"
                 << "\n");
  }
}
