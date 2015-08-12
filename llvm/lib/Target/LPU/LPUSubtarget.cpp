//===-- LPUSubtarget.cpp - LPU Subtarget Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LPU specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "LPUSubtarget.h"
#include "LPU.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "lpu-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "LPUGenSubtargetInfo.inc"

void LPUSubtarget::anchor() { }

LPUSubtarget &LPUSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS) {
  ParseSubtargetFeatures(CPU.empty() ? "ordered" : CPU, FS);
  return *this;
}

LPUSubtarget::LPUSubtarget(const std::string &TT, const std::string &CPU,
                                 const std::string &FS, const TargetMachine &TM)
    : LPUGenSubtargetInfo(TT, CPU, FS),
      DL("e-m:e-i64:64-n32:64"),
      FrameLowering(),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)), TLInfo(TM),
      TSInfo(DL),
      LPUName(CPU.empty() ? "ordered" : CPU)
  {}
