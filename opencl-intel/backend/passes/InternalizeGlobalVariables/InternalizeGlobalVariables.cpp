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
#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "OCLPassSupport.h"
#include "llvm/ADT/SmallSet.h"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

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
    SmallSet<Value *, 8> TLSGlobals;
    for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
      GlobalVariable *GV = CompilationUtils::getTLSGlobal(&M, I);
      TLSGlobals.insert(GV);
    }
    for (auto &GVar : M.globals()) {
      // According to llvm/LangRef, unreferenced globals of common, weak and
      // weak_odr linkage may not be discarded.
      unsigned AS = GVar.getAddressSpace();
      bool MayNotDiscardLinkage =
        (IS_ADDR_SPACE_GLOBAL(AS) || IS_ADDR_SPACE_CONSTANT(AS)) &&
        (GVar.hasCommonLinkage() || GVar.hasExternalLinkage() ||
         GVar.hasWeakLinkage() || GVar.hasWeakODRLinkage());
      if (TLSGlobals.count(&GVar) || MayNotDiscardLinkage ||
          (GVar.hasName() && GVar.getName().startswith("llvm.")))
        continue;
      GVar.setLinkage(GlobalValue::InternalLinkage);
      Changed = true;
    }
    return Changed;
  }
}

