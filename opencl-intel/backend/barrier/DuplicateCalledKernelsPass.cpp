// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "DuplicateCalledKernelsPass.h"
#include "OCLPassSupport.h"
#include "MetadataAPI.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <vector>

namespace intel {

  char DuplicateCalledKernels::ID = 0;

  OCL_INITIALIZE_PASS(DuplicateCalledKernels, "B-DuplicateCalledKernels", "Barrier Pass - Clone kernels called from other functions", false, true)

  DuplicateCalledKernels::DuplicateCalledKernels() : ModulePass(ID) {}

  bool DuplicateCalledKernels::runOnModule(Module &M) {
    using namespace Intel::MetadataAPI;

    DebugInfoFinder finder;
    finder.processModule(M);

    bool changed = false;
    for(auto pFunc : KernelList(&M)) {
      bool isCalledKernel = false;
      for ( auto U : pFunc->users() ) {
        if ( isa<CallInst>(U) ) {
          isCalledKernel = true;
          break;
        }
      }
      if ( !isCalledKernel ) continue;
      changed = true;

      ValueToValueMapTy vMap;
      Function* pNewFunc = CloneFunction(pFunc, vMap, nullptr);
      pNewFunc->setName("__internal." + (pFunc)->getName());

      //Run over old uses of pFuncToFix and replace with call to pNewFunc
      for ( auto u : pFunc->users() ) {
        CallInst *pCallInst = dyn_cast<CallInst>(u);
        if ( !pCallInst ) continue;
        //Replace call to kernel function with call to new function.
        pCallInst->replaceUsesOfWith(pFunc, pNewFunc);
      }
    }
    return changed;
  }
} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  llvm::ModulePass* createDuplicateCalledKernelsPass() {
    return new intel::DuplicateCalledKernels();
  }
}
