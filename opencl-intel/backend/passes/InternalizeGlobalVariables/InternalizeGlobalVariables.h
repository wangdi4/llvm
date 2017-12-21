//==--- InternalizeGlobalVariables.h - an internalizing pass -*- C++ -*---==//
////
//// Copyright (C) 2017 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------=== //

#ifndef __INTERNALIZE_GLOBAL_VARIABLES_H__
#define __INTERNALIZE_GLOBAL_VARiABLES_H__

#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

namespace intel {
  // That pass changes a linkage type of global variables from external to internal
  class InternalizeGlobalVariables : public llvm::ModulePass {
  public:
    InternalizeGlobalVariables() : llvm::ModulePass(ID) {}

    // LLVM Module pass entry
    bool runOnModule(llvm::Module& M) override;
    // Pass identification, replacement for typeid
    static char ID;
  };
}

#endif // __INTERNALIZE_GLOBAL_VARIABLES_H__
