//==----- DPCPPKernelTransforms.cpp - passes initialization -*- C++ -*------==//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/InitializePasses.h"

using namespace llvm;

void llvm::initializeIntel_DPCPPKernelTransforms(PassRegistry &Registry) {
  initializeBuiltinCallToInstLegacyPass(Registry);
  initializeBuiltinImportLegacyPass(Registry);
  initializeDPCPPEqualizerLegacyPass(Registry);
  initializeDPCPPKernelVecClonePass(Registry);
  initializeDPCPPKernelPostVecPass(Registry);
  initializeDPCPPKernelWGLoopCreatorLegacyPass(Registry);
  initializeDPCPPKernelAnalysisLegacyPass(Registry);
  initializePhiCanonicalizationLegacyPass(Registry);
  initializeDuplicateCalledKernelsLegacyPass(Registry);
  initializeRedundantPhiNodeLegacyPass(Registry);
  initializeGroupBuiltinLegacyPass(Registry);
  initializeSplitBBonBarrierLegacyPass(Registry);
  initializeWIRelatedValueWrapperPass(Registry);
  initializeDataPerBarrierWrapperPass(Registry);
  initializeDataPerValueWrapperPass(Registry);
  initializeKernelBarrierLegacyPass(Registry);
  initializeBarrierInFunctionLegacyPass(Registry);
  initializeLinearIdResolverLegacyPass(Registry);
  initializeLocalBufferAnalysisLegacyPass(Registry);
  initializeLocalBuffersLegacyPass(Registry);
  initializeImplicitArgsAnalysisLegacyPass(Registry);
  initializeInternalizeNonKernelFuncLegacyPass(Registry);
  initializeAddImplicitArgsLegacyPass(Registry);
  initializeResolveWICallLegacyPass(Registry);
  initializePrepareKernelArgsLegacyPass(Registry);
  initializeCleanupWrappedKernelLegacyPass(Registry);
  initializeUpdateCallAttrsLegacyPass(Registry);
  initializeVectorVariantFillInLegacyPass(Registry);
  initializeVectorVariantLoweringLegacyPass(Registry);
}

