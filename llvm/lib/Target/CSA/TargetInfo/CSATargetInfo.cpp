//===----------- CSATargetInfo.cpp - CSA Target Implementation ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

namespace llvm {
  Target &getTheCSATarget() {
    static Target TheCSATarget;
    return TheCSATarget;
  }
} // namespace llvm

extern "C" void LLVMInitializeCSATargetInfo() {
  RegisterTarget<Triple::csa, /*HasJIT=*/ false>
    X(getTheCSATarget(), "csa", "CSA");
}
