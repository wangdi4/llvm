// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

// This pass is to externalize global variables in OpenMP offloading code.

#include "ExternalizeGlobalVariables.h"
#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "OCLPassSupport.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

#define DEBUG_TYPE "externalize-global-variables"

extern "C" {
ModulePass *createExternalizeGlobalVariablesPass() {
  return new intel::ExternalizeGlobalVariables();
}
}

namespace intel {
char ExternalizeGlobalVariables::ID = 0;
// Register the pass to opt
OCL_INITIALIZE_PASS(ExternalizeGlobalVariables, "externalize-global-variables",
                    "ExternalizeGlobalVariables: change linkage type for global"
                    " variables with private/internal linkage to external.",
                    false, false)

bool ExternalizeGlobalVariables::runOnModule(Module &M) {
  bool Changed = false;
  SmallSet<Value *, 8> TLSGlobals;
  for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    GlobalVariable *GV = CompilationUtils::getTLSGlobal(&M, I);
    TLSGlobals.insert(GV);
  }

  for (auto &GVar : M.globals()) {
    bool SkipConvertLinkage =
        TLSGlobals.count(&GVar) || !GVar.hasName() ||
        (GVar.hasName() && GVar.getName().startswith("llvm.")) ||
        (GVar.getLinkage() != GlobalValue::InternalLinkage &&
         GVar.getLinkage() != GlobalValue::PrivateLinkage);
    if (SkipConvertLinkage)
      continue;

    LLVM_DEBUG(dbgs() << "Converting " << GVar.getName()
                      << " to external linkage.\n");
    GVar.setLinkage(GlobalValue::ExternalLinkage);
    Changed = true;
  }

  return Changed;
}
} // namespace intel
