//===- Intel_SYCLKernelPostVec.cpp - Post vectorization pass ----*- C++-*-===//
//
// Copyright (C) 2020-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// SYCLKernelPostVec checks if a cloned kernel is not vectorized. If not, it
/// is removed.
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/Intel_SYCLKernelPostVec.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#define DEBUG_TYPE "sycl-kernel-postvec"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;

// Cloned kernel isn't vectorized if it has openmp directives or
// llvm.loop.vectorize.enable metadata.
static bool isKernelVectorized(LoopInfo &LI, Function *Clone) {
  for (Instruction &I : instructions(Clone))
    if (vpo::VPOAnalysisUtils::isOpenMPDirective(&I))
      return false;

  return llvm::none_of(LI, [](Loop *L) {
    // The check is similar as in WarnMissedTransformationsPass.
    return hasVectorizeTransformation(L) == TM_ForcedByUser;
  });
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
  Function *ClonedKernel =
      FMD.VectorizedKernel.hasValue() ? FMD.VectorizedKernel.get() : nullptr;
  Function *MaskedKernel = FMD.VectorizedMaskedKernel.hasValue()
                               ? FMD.VectorizedMaskedKernel.get()
                               : nullptr;
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

PreservedAnalyses SYCLKernelPostVecPass::run(Module &M,
                                             ModuleAnalysisManager &MAM) {
  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  bool Changed = false;
  auto Kernels = CompilationUtils::getKernels(M);
  for (Function *F : Kernels) {
    // Try to rebind vectorized kernel if missing.
    Changed |= rebindVectorizedKernel(F);

    // Remove "recommended-vector-length" metadata.
    removeRecommendedVLMetadata(F);

    // Remove not vectorized clone functions.
    auto RemoveNotVectorizedClone = [&](Function *Clone, StringRef MDName) {
      if (!Clone)
        return;
      LoopInfo &LI = FAM.getResult<LoopAnalysis>(*Clone);
      if (isKernelVectorized(LI, Clone)) {
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
    if (FMD.VectorizedKernel.hasValue())
      RemoveNotVectorizedClone(FMD.VectorizedKernel.get(),
                               FMD.VectorizedKernel.getID());
    if (FMD.VectorizedMaskedKernel.hasValue())
      RemoveNotVectorizedClone(FMD.VectorizedMaskedKernel.get(),
                               FMD.VectorizedMaskedKernel.getID());
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
