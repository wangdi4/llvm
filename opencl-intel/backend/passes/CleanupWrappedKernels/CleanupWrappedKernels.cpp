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

#include "CleanupWrappedKernels.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

using namespace llvm;

extern "C" {
  ModulePass* createCleanupWrappedKernelsPass() {
    return new intel::CleanupWrappedKernels();
  }
}

namespace intel {
  char CleanupWrappedKernels::ID = 0;

  // Register the pass to opt
  OCL_INITIALIZE_PASS(CleanupWrappedKernels, "cleanup-wrapped-kernels",
    "CleanupWrappedKernels: removes wrapped kernels",
    false, false)

  // Delete function body preserving attached Metadata
  // (except !dbg and !prof, declarations are not allowed to have them)
  static void deleteFunctionBody(Function* Func) {
    SmallVector<std::pair<unsigned, MDNode *>, 8> MDs;
    Func->eraseMetadata(LLVMContext::MD_dbg);
    Func->eraseMetadata(LLVMContext::MD_prof);
    Func->getAllMetadata(MDs);
    Func->deleteBody();

    for (auto &MD : MDs) {
      Func->setMetadata(MD.first, MD.second);
    }
  }

  bool CleanupWrappedKernels::runOnModule(Module& M) {
    using namespace Intel::MetadataAPI;

    bool Changed = false;
    auto Kernels = KernelList(&M).getList();
    for (auto *Kernel : Kernels) {
      // If a kernel is wrapped - remove it
      auto Kimd = KernelInternalMetadataAPI(Kernel);
      if (Kimd.KernelWrapper.hasValue()) {
        deleteFunctionBody(Kernel);
        Changed = true;
      }
    }
    return Changed;
  }
}
