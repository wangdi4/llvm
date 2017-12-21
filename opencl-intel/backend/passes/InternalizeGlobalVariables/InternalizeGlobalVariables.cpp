//==--- InternalizeGlobalVariables.cpp - an internalizing pass -*- C++ -*---==//
////
//// Copyright (C) 2017 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------=== //

#include "InternalizeGlobalVariables.h"
#include "OCLPassSupport.h"

using namespace llvm;

extern "C" {
  ModulePass* createInternalizeGlobalVariablesPass() {
    return new intel::InternalizeGlobalVariables();
  }
}

namespace intel {
  char InternalizeGlobalVariables::ID = 0;
  // Register the pass to opt
  OCL_INITIALIZE_PASS(InternalizeGlobalVariables, "internalize-global-variables",
    "InternalizeGlobalVariables: change a linkage type for almost all global variables",
    false, false)

  bool InternalizeGlobalVariables::runOnModule(Module& M) {
    bool Changed = false;
    for (auto &GVar : M.globals()) {
      if (GVar.hasName() && GVar.getName().startswith("llvm."))
        continue;
      GVar.setLinkage(GlobalValue::InternalLinkage);
      Changed = true;
    }
    return Changed;
  }
}

