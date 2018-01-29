//===-- CSASubtarget.cpp - CSA Subtarget Information ----------------------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CSA specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "CSASubtarget.h"
#include "CSA.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "csa-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "CSAGenSubtargetInfo.inc"

void CSASubtarget::anchor() { }

CSASubtarget &CSASubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS) {
  ParseSubtargetFeatures(CPU.empty() ? "autounit" : CPU, FS);
  return *this;
}

CSASubtarget::CSASubtarget(const Triple &TT, const std::string &CPU,
                                 const std::string &FS, const TargetMachine &TM)
    : CSAGenSubtargetInfo(TT, CPU, FS),
      FrameLowering(),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)), TLInfo(TM, *this),
      TSInfo(),
      CSAName(CPU.empty() ? "autounit" : CPU)
  {}
