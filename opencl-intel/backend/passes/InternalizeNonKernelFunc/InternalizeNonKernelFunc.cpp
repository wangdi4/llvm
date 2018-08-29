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
