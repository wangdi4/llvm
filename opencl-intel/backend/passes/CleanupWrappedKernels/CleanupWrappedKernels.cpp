//==--- CleanupWrappedKernels.cpp - a removing wrapped kernels pass -*- C++ -*---==//
////
//// Copyright (C) 2017 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------=== //

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
