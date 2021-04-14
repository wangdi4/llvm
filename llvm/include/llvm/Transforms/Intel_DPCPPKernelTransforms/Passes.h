//==----- Passes.h - Constructors for DPCPP Kernel transforms -*- C++ -*----==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PASSES_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PASSES_H

namespace llvm {

class ModulePass;

Pass *createParseAnnotateAttributesPass();
ModulePass *createDPCPPEqualizerLegacyPass();
ModulePass* createDPCPPKernelVecClonePass();
ModulePass* createDPCPPKernelPostVecPass();
ModulePass *createDPCPPKernelWGLoopCreatorLegacyPass();
ModulePass *createDPCPPKernelAnalysisLegacyPass();
FunctionPass *createPhiCanonicalizationLegacyPass();
FunctionPass *createRedundantPhiNodeLegacyPass();
ModulePass *createSplitBBonBarrierLegacyPass();
ModulePass *createWIRelatedValueWrapperPass();
ModulePass *createDataPerBarrierWrapperPass();
ModulePass *createDataPerValueWrapperPass();
ModulePass *createKernelBarrierLegacyPass(bool isNativeDebug,
                                          bool useTLSGlobals);
ModulePass *createBarrierInFunctionLegacyPass();
ModulePass *createImplicitArgsAnalysisLegacyPass();
ModulePass *createLocalBufferAnalysisLegacyPass();
ModulePass *createAddImplicitArgsLegacyPass();
ModulePass *createResolveWICallLegacyPass(bool IsUniformWGSize,
                                          bool UseTLSGlobals);
ModulePass *createPrepareKernelArgsLegacyPass(bool UseTLSGlobals);
ModulePass *createCleanupWrappedKernelLegacyPass();
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PASSES_H
