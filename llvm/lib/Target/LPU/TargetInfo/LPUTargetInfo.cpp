//===-- LPUTargetInfo.cpp - LPU Target Implementation -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LPUTargetMachine.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheLPUTarget;

extern "C" void LLVMInitializeLPUTargetInfo() {
  RegisterTarget<Triple::lpu, /*HasJIT=*/ false>
    X(TheLPUTarget, "lpu", "LPU");
}

extern "C" void LLVMInitializeLPUTargetMC() {}
