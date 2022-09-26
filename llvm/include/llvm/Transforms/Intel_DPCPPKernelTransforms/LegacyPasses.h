//==- LegacyPasses.h - Legacy constructors for DPCPP kernel transforms -----==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H

#include "llvm/ADT/StringSet.h"
#include "llvm/Analysis/VectorUtils.h"

namespace llvm {

class FunctionPass;
class ImmutablePass;
class LoopPass;
class ModulePass;
class Pass;
class StringRef;
template <typename T, unsigned N> class SmallVector;

FunctionPass *createAddFastMathLegacyPass();
FunctionPass *createAddNTAttrLegacyPass();
FunctionPass *createBuiltinCallToInstLegacyPass();
FunctionPass *createOptimizeIDivAndIRemLegacyPass();
FunctionPass *createPhiCanonicalizationLegacyPass();
FunctionPass *createPreventDivCrashesLegacyPass();
FunctionPass *createRedundantPhiNodeLegacyPass();
FunctionPass *createSinCosFoldLegacyPass();
FunctionPass *createSoaAllocaAnalysisLegacyPass();
FunctionPass *createWeightedInstCountAnalysisLegacyPass(
    VFISAKind ISA = VFISAKind::SSE, bool PreVec = true);
FunctionPass *createWorkItemAnalysisLegacyPass(unsigned VectorizeDim = 0);
ImmutablePass *
createBuiltinLibInfoAnalysisLegacyPass(ArrayRef<Module *> BuiltinModules = {});
ImmutablePass *createDPCPPAliasAnalysisLegacyPass();
ImmutablePass *createDPCPPExternalAliasAnalysisLegacyPass();
LoopPass *createBuiltinLICMLegacyPass();
LoopPass *createLoopStridedCodeMotionLegacyPass();
LoopPass *createLoopWIAnalysisLegacyPass();
ModulePass *createAddFunctionAttrsLegacyPass();
ModulePass *createAddImplicitArgsLegacyPass();
ModulePass *createAddTLSGlobalsLegacyPass();
ModulePass *createAutorunReplicatorLegacyPass();
ModulePass *createBarrierInFunctionLegacyPass();
ModulePass *createBuiltinImportLegacyPass(StringRef CPUPrefix = "");
ModulePass *createChannelPipeTransformationLegacyPass();
ModulePass *createCleanupWrappedKernelLegacyPass();
ModulePass *createCoerceTypesLegacyPass();
ModulePass *createCoerceWin64TypesLegacyPass();
ModulePass *createCreateSimdVariantPropagationLegacyPass();
ModulePass *createDataPerBarrierWrapperPass();
ModulePass *createDataPerValueWrapperPass();
ModulePass *createDeduceMaxWGDimLegacyPass();
ModulePass *createDetectRecursionLegacyPass();
ModulePass *createDPCPPEqualizerLegacyPass();
ModulePass *createDPCPPKernelAnalysisLegacyPass();
ModulePass *createDPCPPKernelPostVecPass();
ModulePass *createDPCPPKernelVecClonePass(
    ArrayRef<std::tuple<const char *, const char *, const char *>> VectInfos =
        {},
    VFISAKind ISA = VFISAKind::SSE, bool IsOCL = false);
ModulePass *
createDPCPPKernelWGLoopCreatorLegacyPass(bool UseTLSGlobals = false);
ModulePass *createDPCPPPreprocessSPIRVFriendlyIRLegacyPass();
ModulePass *createDPCPPRewritePipesLegacyPass();
ModulePass *createDuplicateCalledKernelsLegacyPass();
ModulePass *createExternalizeGlobalVariablesLegacyPass();
ModulePass *createGroupBuiltinLegacyPass();
ModulePass *
createHandleVPlanMaskLegacyPass(const StringSet<> *VPlanMaskedFuncs);
ModulePass *createImplicitArgsAnalysisLegacyPass();
ModulePass *createImplicitGIDLegacyPass(bool HandleBarrier = true);
ModulePass *createIndirectCallLoweringLegacyPass();
ModulePass *createInferArgumentAliasLegacyPass();
ModulePass *createInfiniteLoopCreatorLegacyPass();
ModulePass *createInstToFuncCallLegacyPass(VFISAKind ISA = VFISAKind::SSE);
ModulePass *createInternalizeGlobalVariablesLegacyPass();
ModulePass *createInternalizeNonKernelFuncLegacyPass();
ModulePass *createKernelBarrierLegacyPass(bool isNativeDebug,
                                          bool useTLSGlobals);
ModulePass *createLinearIdResolverPass();
ModulePass *createLocalBufferAnalysisLegacyPass();
ModulePass *createLocalBuffersLegacyPass(bool UseTLSGlobals);
ModulePass *createPatchCallbackArgsLegacyPass(bool UseTLSGlobals);
ModulePass *createPipeIOTransformationLegacyPass();
ModulePass *createPipeOrderingLegacyPass();
ModulePass *createPipeSupportLegacyPass();
ModulePass *createPrepareKernelArgsLegacyPass(bool UseTLSGlobals);
ModulePass *createProfilingInfoLegacyPass();
ModulePass *createReduceCrossBarrierValuesLegacyPass();
ModulePass *createRelaxedMathLegacyPass();
ModulePass *createRemoveAtExitLegacyPass();
ModulePass *createRemoveDuplicatedBarrierLegacyPass(bool IsNativeDebug);
ModulePass *createReplaceScalarWithMaskLegacyPass();
ModulePass *createReqdSubGroupSizeLegacyPass();
ModulePass *createResolveMatrixFillLegacyPass();
ModulePass *createResolveMatrixLayoutLegacyPass();
ModulePass *createResolveMatrixWISliceLegacyPass();
ModulePass *createResolveSubGroupWICallLegacyPass(bool ResolveSGBarrier = true);
ModulePass *createResolveVarTIDCallLegacyPass();
ModulePass *createResolveWICallLegacyPass(bool IsUniformWGSize,
                                          bool UseTLSGlobals);
ModulePass *createSetPreferVectorWidthLegacyPass(
    VFISAKind ISA = VFISAKind::SSE);
ModulePass *createSetVectorizationFactorLegacyPass(
    VFISAKind ISA = VFISAKind::SSE);
ModulePass *createSGBarrierPropagateLegacyPass();
ModulePass *createSGBarrierSimplifyLegacyPass();
ModulePass *createSGBuiltinLegacyPass(
    ArrayRef<std::tuple<const char *, const char *, const char *>> VectInfos =
        {});
ModulePass *createSGLoopConstructLegacyPass();
ModulePass *createSGSizeAnalysisLegacyPass();
ModulePass *createSGSizeCollectorIndirectLegacyPass(VFISAKind);
ModulePass *createSGSizeCollectorLegacyPass(VFISAKind);
ModulePass *createSGValueWidenLegacyPass();
ModulePass *createSplitBBonBarrierLegacyPass();
ModulePass *createTaskSeqAsyncHandlingLegacyPass();
ModulePass *createUpdateCallAttrsLegacyPass();
ModulePass *createVectorizationDimensionAnalysisLegacyPass();
ModulePass *createVectorKernelEliminationLegacyPass();
ModulePass *createVectorVariantFillInLegacyPass();
ModulePass *createVectorVariantLoweringLegacyPass(VFISAKind);
ModulePass *createVFAnalysisLegacyPass();
ModulePass *createWGLoopBoundariesLegacyPass();
ModulePass *createWIRelatedValueWrapperPass();
Pass *createParseAnnotateAttributesPass();
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H
