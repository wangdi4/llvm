// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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

