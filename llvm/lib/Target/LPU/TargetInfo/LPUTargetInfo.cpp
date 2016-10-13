//===----------- LPUTargetInfo.cpp - LPU Target Implementation ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LPUTargetMachine.h"
#include "LPUAsmStreamer.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheLPUTarget;

extern "C" void LLVMInitializeLPUTargetInfo() {

  // createLpuAsmStreamer will check -lpu-wrap-assembly to decide whether
  // to create an LpuAsmStream (to create an x86 assembly file to build
  // into an object) or MCAsmStreamer (to create a raw LPU assembly file).
  //
  // Unfortunately LLVMInitializeLPUTargetInfo() is called before the
  // command line options are evaluated, so we need to delay the check.
  TargetRegistry::RegisterAsmStreamer(TheLPUTarget, createLpuAsmStreamer);

  RegisterTarget<Triple::lpu, /*HasJIT=*/ false>
    X(TheLPUTarget, "lpu", "LPU");
}
