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

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
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

// Make sure the cloned vectorized kernel (if there's one) is binded
// to the original function correctly.
// A cloned vectorized kernel may not be binded to original function
// metadata if it "isSimpleFunction()".
// In such case metadata updating was skipped, so we need to recover
// here, otherwise the VectorizedWidth may be set to zero.
// Also, if the vectorizer doesn't run, the original kernel's VectorizedWidth
// won't be set, we need to update it here.
static bool rebindVectorizedKernel(Module &M, Function *F) {
  bool ModifiedModule = false;

  auto FMD = KernelInternalMetadataAPI(F);
  Function *ClonedKernel = FMD.VectorizedKernel.get();
  Function *MaskedKernel = FMD.VectorizedMaskedKernel.get();
  // Vectorized kernel already binded.
  if (ClonedKernel || MaskedKernel)
    return ModifiedModule;

  // Get vectorized kernel name from "vector-variants" attribute
  Attribute Attr = F->getFnAttribute("vector-variants");
  SmallVector<StringRef, 4> VecVariants;
  SplitString(Attr.getValueAsString(), VecVariants, ",");

  auto VL = FMD.OclRecommendedVectorLength.get();
  Function *Clone = nullptr;

  // Set origin's metadata if vectorizer didn't run.
  if (!FMD.VectorizedWidth.hasValue()) {
    FMD.VectorizedWidth.set(1);
    FMD.ScalarizedKernel.set(nullptr);
    ModifiedModule = true;
  }

  for (auto &VariantName : VecVariants) {
    Clone = M.getFunction(VariantName);
    if (nullptr == Clone)
      continue;

    // Set clone's metadata
    auto CloneMD = KernelInternalMetadataAPI(Clone);

    CloneMD.VectorizedKernel.set(nullptr);
    CloneMD.VectorizedWidth.set(VL);
    CloneMD.ScalarizedKernel.set(F);

    if (F->getFunctionType() == Clone->getFunctionType()) {
      assert(nullptr == FMD.VectorizedKernel.get() &&
             "Should not overwrite original metadata.");
      FMD.VectorizedKernel.set(Clone);
    } else { // Vectorized kernel with mask
      assert(nullptr == FMD.VectorizedMaskedKernel.get() &&
             "Should not overwrite original metadata.");
      FMD.VectorizedMaskedKernel.set(Clone);
    }

    ModifiedModule = true;
  }

  return ModifiedModule;
}

bool OCLPostVect::runOnModule(Module &M) {
  auto Kernels = KernelList(*&M).getList();
  bool ModifiedModule = false;
  for (Function *F : Kernels) {
    // Try to rebind vectorized kernel if missing.
    ModifiedModule |= rebindVectorizedKernel(M, F);
    // Remove "ocl_recommended_vector_length" metadata.
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
