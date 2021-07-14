//==- LegacyPasses.h - Legacy constructors for DPCPP kernel transforms -----==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H

#include "llvm/Analysis/Intel_VectorVariant.h" // for VectorVariant::ISAClass

namespace llvm {

class FunctionPass;
class ModulePass;
class Pass;
template <typename T, unsigned N> class SmallVector;
class StringRef;

Pass *createParseAnnotateAttributesPass();
FunctionPass *createBuiltinCallToInstLegacyPass();
ModulePass *createBuiltinImportLegacyPass(
    const SmallVector<Module *, 2> &BuiltinModules = SmallVector<Module *, 2>(),
    StringRef CPUPrefix = "");
ModulePass *createDPCPPEqualizerLegacyPass();
ModulePass *createDPCPPKernelVecClonePass();
ModulePass *createDPCPPKernelPostVecPass();
ModulePass *createDPCPPKernelWGLoopCreatorLegacyPass();
ModulePass *createDPCPPKernelAnalysisLegacyPass();
ModulePass *createDuplicateCalledKernelsLegacyPass();
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
ModulePass *createInternalizeNonKernelFuncLegacyPass();
ModulePass *createLinearIdResolverPass();
ModulePass *createLocalBufferAnalysisLegacyPass();
ModulePass *createLocalBuffersLegacyPass(bool UseTLSGlobals);
ModulePass *createAddImplicitArgsLegacyPass();
ModulePass *createResolveWICallLegacyPass(bool IsUniformWGSize,
                                          bool UseTLSGlobals);
ModulePass *createPrepareKernelArgsLegacyPass(bool UseTLSGlobals);
ModulePass *createCleanupWrappedKernelLegacyPass();
ModulePass *createUpdateCallAttrsLegacyPass();
ModulePass *createVectorVariantFillInLegacyPass();
ModulePass *createVectorVariantLoweringLegacyPass(VectorVariant::ISAClass);
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H
