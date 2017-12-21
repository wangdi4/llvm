//==--- InternalizeNonKernelFunc.cpp - an internalizing pass -*- C++ -*---==//
////
//// Copyright (C) 2017 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------=== //

#include "InternalizeNonKernelFunc.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

using namespace llvm;

extern "C" {
  ModulePass* createInternalizeNonKernelFuncPass() {
    return new intel::InternalizeNonKernelFunc();
  }
}

namespace intel {
  char InternalizeNonKernelFunc::ID = 0;

  // Register the pass to opt
  OCL_INITIALIZE_PASS(InternalizeNonKernelFunc, "internalize-nonkernel-functions",
    "InternalizeNonKernelFunc: change a linkage type for all nonkernel function to internal",
    false, false)

  bool InternalizeNonKernelFunc::runOnModule(Module& M) {
    using namespace Intel::MetadataAPI;
    bool Changed = false;
    auto Kernels = KernelList(&M).getList();

    for (auto &Func : M) {
      // Skip if a Function is a priori external (just a declaration)
      if (Func.isDeclaration())
        continue;

      // We shall not internalize kernels
      if (std::find(std::begin(Kernels), std::end(Kernels), &Func) != std::end(Kernels))
        continue;

      Func.setLinkage(GlobalValue::InternalLinkage);
      Changed = true;
    }
    return Changed;
  }
}
