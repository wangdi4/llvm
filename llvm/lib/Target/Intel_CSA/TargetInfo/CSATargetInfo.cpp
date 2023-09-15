//===----------- CSATargetInfo.cpp - CSA Target Implementation ------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

namespace llvm {
Target &getTheCSATarget() {
  static Target TheCSATarget;
  return TheCSATarget;
}
} // namespace llvm

extern "C" void LLVMInitializeCSATargetInfo() {
  RegisterTarget<Triple::csa, /*HasJIT=*/ false>
    X(getTheCSATarget(), "csa", "CSA", "CSANULL");
}
