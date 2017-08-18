<<<<<<< HEAD
//===-- Cpu0TargetInfo.cpp - Nios2 Target Implementation -------------------===//
=======
//===-- Nios2TargetInfo.cpp - Nios2 Target Implementation -----------------===//
>>>>>>> 404324ef9b1f6c22852df722fd2ad228c4d74076
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Nios2.h"
<<<<<<< HEAD
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
=======
#include "llvm/Support/TargetRegistry.h"

>>>>>>> 404324ef9b1f6c22852df722fd2ad228c4d74076
using namespace llvm;

Target &llvm::getTheNios2Target() {
  static Target TheNios2Target;
  return TheNios2Target;
}

extern "C" void LLVMInitializeNios2TargetInfo() {
  RegisterTarget<Triple::nios2,
<<<<<<< HEAD
        /*HasJIT=*/true> X(getTheNios2Target(), "nios2", "Nios2");
=======
                 /*HasJIT=*/true>
      X(getTheNios2Target(), "nios2", "Nios2");
>>>>>>> 404324ef9b1f6c22852df722fd2ad228c4d74076
}
