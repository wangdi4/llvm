//===- Intel_SYCLKernelPostVec.cpp - Post vectorization pass ----*- C++-*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
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
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/SYCLTransforms/Intel_SYCLKernelVecClone.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#define DEBUG_TYPE "sycl-kernel-postvec"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;

extern cl::opt<GlobalWorkSizeLT2GState> LT2GigGlobalWorkSize;

// Cloned kernel isn't vectorized if it is marked with "vector-variant-failure"
// attribute.
static bool isKernelVectorized(Function *Clone) {
  return !Clone->hasFnAttribute(KernelAttribute::VectorVariantFailure);
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
  if (!FMD.VectorizedWidth.hasValue() && !FMD.SubgroupEmuSize.hasValue()) {
    FMD.VectorizedWidth.set(1);
    Changed = true;
  }

  // Get vectorized kernel name from ""vector-variants" attribute.
  Attribute Attr = F->getFnAttribute(VectorUtils::VectorVariantsAttrName);
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

// If input IR is from OpenCL and %sext has only one use, %0's uses could be
// replaced with %call.
//
//  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #5
//  %sext = shl i64 %call, 32
//  %0 = ashr exact i64 %sext, 32
//
static bool optimizeGIDShlAshr(Function *F, Function *GetGID) {
  if (!F)
    return false;
  using namespace PatternMatch;
  for (User *U0 : GetGID->users()) {
    auto *I = cast<CallInst>(U0);
    if (I->getFunction() != F)
      continue;
    for (User *U1 : I->users()) {
      if (match(U1, m_OneUse(m_Shl(m_Specific(I), m_SpecificInt(32))))) {
        auto *SingleUser = *U1->user_begin();
        if (match(SingleUser, m_AShr(m_Specific(U1), m_SpecificInt(32)))) {
          SingleUser->replaceAllUsesWith(I);
          return true;
        }
      }
    }
  }

  return false;
}

PreservedAnalyses SYCLKernelPostVecPass::run(Module &M,
                                             ModuleAnalysisManager &) {
  bool Changed = false;
  bool IsOCL = !CompilationUtils::isGeneratedFromOCLCPP(M) &&
               !CompilationUtils::isGeneratedFromOMP(M);
  auto *GetGID = M.getFunction(CompilationUtils::mangledGetGID());
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
    if (FMD.VectorizedKernel.hasValue()) {
      RemoveNotVectorizedClone(FMD.VectorizedKernel.get(),
                               FMD.VectorizedKernel.getID());
      if (GetGID && IsOCL && LT2GigGlobalWorkSize == GWS_AUTO)
        Changed |= optimizeGIDShlAshr(FMD.VectorizedKernel.get(), GetGID);
    }
    if (FMD.VectorizedMaskedKernel.hasValue()) {
      RemoveNotVectorizedClone(FMD.VectorizedMaskedKernel.get(),
                               FMD.VectorizedMaskedKernel.getID());
      if (GetGID && IsOCL && LT2GigGlobalWorkSize == GWS_AUTO)
        Changed |= optimizeGIDShlAshr(FMD.VectorizedMaskedKernel.get(), GetGID);
    }
  }

  /// Remove vector-variants attr from internal functions, so that
  /// DeadArgumentEliminationPass won't skip them.
  for (Function &F : M) {
    if (F.getLinkage() == GlobalValue::InternalLinkage &&
        F.hasFnAttribute(VectorUtils::VectorVariantsAttrName)) {
      F.removeFnAttr(VectorUtils::VectorVariantsAttrName);
      Changed = true;
    }
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
