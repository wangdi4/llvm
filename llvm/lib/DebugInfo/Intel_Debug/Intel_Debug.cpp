//===- Intel_Debug.cpp - Intel Target-Specific Debug Selection ------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Pass for performing Intel-specific modifications to the debug metadata.
//
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/Intel_Debug/Intel_Debug.h"
#include "llvm/DebugInfo/Intel_Debug/IntelDebugRemoveXDeref.h"

#define DEBUG_TYPE "intel-debug"

using namespace llvm;

Intel_DebugPass::Intel_DebugPass(TargetMachine *TM) : TM(TM) {}

PreservedAnalyses Intel_DebugPass::run(Module &M, ModuleAnalysisManager &AM) {
  LLVM_DEBUG(dbgs() << "Intel_DebugPass::run(M=" << &M << ")\n");
  (void)IntelDebugRemoveXDeref(TM).run(M);
  return PreservedAnalyses::all();
}
