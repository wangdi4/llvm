//===-- llvm/lib/Target/CSA/CSAOMPAllocaTypeFixer.h -------------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration for the CSAOMPAllocaTypeFixer pass,
// which inserts extra allocas to fix issues with OpenMP offload confusing
// mem2reg.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAOMPALLOCATYPEFIXER_H
#define LLVM_LIB_TARGET_CSA_CSAOMPALLOCATYPEFIXER_H

namespace llvm {
  class Pass;
  Pass* createCSAOMPAllocaTypeFixerPass();
}

#endif
