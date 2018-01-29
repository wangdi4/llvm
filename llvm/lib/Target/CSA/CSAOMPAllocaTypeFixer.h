//===-- llvm/lib/Target/CSA/CSAOMPAllocaTypeFixer.h -------------*- C++ -*-===//
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
