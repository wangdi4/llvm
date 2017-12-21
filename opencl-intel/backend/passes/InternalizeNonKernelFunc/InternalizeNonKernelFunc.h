//==--- InternalizeNonKernelFunc.h - an internalizing pass -*- C++ -*---==//
////
//// Copyright (C) 2017 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------=== //

#ifndef __INTERNALIZE_NONKERNEL_FUNC_H__
#define __INTERNALIZE_NONKERNEL_FUNC_H__

#include <llvm/Pass.h>

namespace intel {
  // That pass changes a linkage type of a nonkernel function from external to
  // internal if it's not a kernel. That allows LLVM globaldce to remove
  // the function's body.
  class InternalizeNonKernelFunc : public llvm::ModulePass {
  public:
    InternalizeNonKernelFunc() : llvm::ModulePass(ID) {}

    // LLVM Module pass entry
    bool runOnModule(llvm::Module& M) override;
    // Pass identification, replacement for typeid
    static char ID;
  };
}

#endif // __INTERNALIZE_NONKERNEL_FUNC_H__
