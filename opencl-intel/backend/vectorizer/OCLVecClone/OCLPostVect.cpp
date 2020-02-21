//=---- OCLPostVect.cpp -*-C++-*----=//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// OCLPostVect checks if a cloned kernel is not vectorized. If not, it is
/// removed.
// ===--------------------------------------------------------------------=== //
#include "OCLPostVect.h"
#include "InitializePasses.h"
#include "MetadataAPI.h"

#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LegacyPassManager.h"

#define DEBUG_TYPE "OCLPostVect"
#define SV_NAME "ocl-postvect"

using namespace llvm;
using namespace llvm::vpo;
using namespace Intel::MetadataAPI;

namespace intel {

char OCLPostVect::ID = 0;
static const char lv_name[] = "OCLPostVect";
OCL_INITIALIZE_PASS_BEGIN(OCLPostVect, SV_NAME, lv_name,
                          false /* modifies CFG */, false /* transform pass */)
OCL_INITIALIZE_PASS_END(OCLPostVect, SV_NAME, lv_name,
                        false /* modififies CFG */, false /* transform pass */)

OCLPostVect::OCLPostVect() : ModulePass(ID) {}

// Checks if the kernel has directives. If not, then the kernel was vectorized.
bool OCLPostVect::isKernelVectorized(Function *Clone) {
  for (BasicBlock &Block : *Clone)
    for (Instruction &I : Block)
      if (VPOAnalysisUtils::isOpenMPDirective(&I))
        return false;
  return true;
}

bool OCLPostVect::runOnModule(Module &M) {
  auto Kernels = KernelList(*&M).getList();
  bool ModifiedModule = false;
  for (Function *F : Kernels) {
    //Remove "ocl_recommended_vector_length" metadata
    MDValueGlobalObjectStrategy::unset(F, "ocl_recommended_vector_length");
    auto FMD = KernelInternalMetadataAPI(F);
    Function *ClonedKernel = FMD.VectorizedKernel.get();
    if (ClonedKernel && !isKernelVectorized(ClonedKernel)) {
      // Unset the metadata of the original kernel.
      MDValueGlobalObjectStrategy::unset(F, "vectorized_kernel");
      // If the kernel is not vectorized, then the cloned kernel is removed.
      ClonedKernel->eraseFromParent();
      ModifiedModule = true;
    }
    Function *MaskedKernel = FMD.VectorizedMaskedKernel.get();
    if (MaskedKernel && !isKernelVectorized(MaskedKernel)) {
      // Unset the metadata of the original kernel.
      MDValueGlobalObjectStrategy::unset(F, "vectorized_masked_kernel");
      // If the kernel is not vectorized, then the masked kernel is removed.
      MaskedKernel->eraseFromParent();
      ModifiedModule = true;
    }
  }
  return ModifiedModule;
}
} // namespace intel

extern "C" Pass *createOCLPostVectPass(void) {
  return new intel::OCLPostVect();
}
