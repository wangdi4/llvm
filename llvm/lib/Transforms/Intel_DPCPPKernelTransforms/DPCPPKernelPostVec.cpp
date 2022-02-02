//===- DPCPPKernelPostVec.cpp - Post vectorization pass ----------*- C++-*-===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// DPCPPKernelPostVec checks if a cloned kernel is not vectorized. If not, it
/// is removed.
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelPostVec.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "dpcpp-kernel-postvec"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;

namespace {
class DPCPPKernelPostVec : public ModulePass {
  DPCPPKernelPostVecPass Impl;

public:
  static char ID;

  DPCPPKernelPostVec() : ModulePass(ID) {
    initializeDPCPPKernelPostVecPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }

  /// Returns the name of the pass
  StringRef getPassName() const override {
    return "VPlan post vectorization pass for DPCPP kernels";
  }
};
} // namespace

char DPCPPKernelPostVec::ID = 0;

INITIALIZE_PASS(DPCPPKernelPostVec, DEBUG_TYPE, "VPlan post vectorization pass",
                false, false)

ModulePass *llvm::createDPCPPKernelPostVecPass() {
  return new DPCPPKernelPostVec();
}

// Checks if the kernel has openmp directives. If not, then the kernel was
// vectorized.
static bool isKernelVectorized(Function *Clone) {
  for (Instruction &I : instructions(Clone))
    if (vpo::VPOAnalysisUtils::isOpenMPDirective(&I))
      return false;
  return true;
}

static void removeRecommendedVLMetadata(Function *F) {
  MDValueGlobalObjectStrategy::unset(F, "recommended_vector_length");
}

// Make sure the cloned vectorized kernel (if there's one) is binded to the
// original function correctly.
// A cloned vectorized kernel may not be binded to original function metadata
// if it "isSimpleFunction()".
// In such case metadata updating was skipped, so we need to recover here,
// otherwise the VectorizedWidth may be set to zero.
// Also, if the vectorizer doesn't run, the original kernel's VectorizedWidth
// won't be set, we need to update it here.
static bool rebindVectorizedKernel(Function *F) {
  bool Changed = false;
  auto FMD = KernelInternalMetadataAPI(F);
  Function *ClonedKernel = FMD.VectorizedKernel.get();
  Function *MaskedKernel = FMD.VectorizedMaskedKernel.get();
  // Vectorized kernel already binded.
  if (ClonedKernel || MaskedKernel)
    return Changed;

  // Set origin's metadata if vectorizer didn't run.
  if (!FMD.VectorizedWidth.hasValue()) {
    FMD.VectorizedWidth.set(1);
    Changed = true;
  }

  // Get vectorized kernel name from ""vector-variants" attribute.
  Attribute Attr = F->getFnAttribute("vector-variants");
  SmallVector<StringRef, 4> VecVariants;
  SplitString(Attr.getValueAsString(), VecVariants, ",");
  auto VL = FMD.RecommendedVL.get();
  for (auto &VariantName : VecVariants) {
    Function *Clone = F->getParent()->getFunction(VariantName);
    if (!Clone)
      continue;

    // Set clone's metadata.
    auto CloneMD = KernelInternalMetadataAPI(Clone);
    CloneMD.VectorizedWidth.set(VL);
    CloneMD.ScalarKernel.set(F);
    removeRecommendedVLMetadata(Clone);

    if (F->getFunctionType() == Clone->getFunctionType())
      FMD.VectorizedKernel.set(Clone);
    else
      FMD.VectorizedMaskedKernel.set(Clone);

    Changed = true;
  }

  return Changed;
}

bool DPCPPKernelPostVecPass::runImpl(Module &M) {
  bool Changed = false;
  auto Kernels = DPCPPKernelCompilationUtils::getKernels(M);
  for (Function *F : Kernels) {
    // Try to rebind vectorized kernel if missing.
    Changed |= rebindVectorizedKernel(F);

    // Remove "recommended-vector-length" metadata.
    removeRecommendedVLMetadata(F);

    // Remove not vectorized clone functions.
    auto RemoveNotVectorizedClone = [&Changed, &F](Function *Clone,
                                                   StringRef MDName) {
      if (!Clone)
        return;
      if (isKernelVectorized(Clone)) {
        removeRecommendedVLMetadata(Clone);
        return;
      }

      // Unset the metadata of the original kernel.
      MDValueGlobalObjectStrategy::unset(F, MDName);
      // If the kernel is not vectorized, then the cloned kernel is removed.
      Clone->eraseFromParent();
      Changed = true;
    };
    auto FMD = KernelInternalMetadataAPI(F);
    RemoveNotVectorizedClone(FMD.VectorizedKernel.get(), "vectorized_kernel");
    RemoveNotVectorizedClone(FMD.VectorizedMaskedKernel.get(),
                             "vectorized_masked_kernel");
  }

  return Changed;
}
