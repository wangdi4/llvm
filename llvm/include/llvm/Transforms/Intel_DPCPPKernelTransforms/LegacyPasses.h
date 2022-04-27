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
#include "llvm/IR/Intel_VectorVariant.h" // for VectorVariant::ISAClass

namespace llvm {

class FunctionPass;
class LoopPass;
class ModulePass;
class Pass;
template <typename T, unsigned N> class SmallVector;
class StringRef;

ModulePass *createAddFunctionAttrsLegacyPass();
Pass *createParseAnnotateAttributesPass();
ModulePass *createTaskSeqAsyncHandlingLegacyPass();
FunctionPass *createBuiltinCallToInstLegacyPass();
ModulePass *createBuiltinImportLegacyPass(
    const SmallVector<Module *, 2> &BuiltinModules = SmallVector<Module *, 2>(),
    StringRef CPUPrefix = "");
ImmutablePass *
createBuiltinLibInfoAnalysisLegacyPass(ArrayRef<Module *> BuiltinModules = {});
ModulePass *createCreateSimdVariantPropagationLegacyPass();
ModulePass *createCoerceWin64TypesLegacyPass();
ModulePass *
createDPCPPEqualizerLegacyPass(ArrayRef<Module *> BuiltinModules = {});
ModulePass *createDPCPPKernelVecClonePass(
    ArrayRef<std::tuple<const char *, const char *, const char *>> VectInfos =
        {},
    VectorVariant::ISAClass ISA = VectorVariant::XMM, bool IsOCL = false);
ModulePass *createDPCPPKernelPostVecPass();
ModulePass *createDPCPPKernelWGLoopCreatorLegacyPass();
ModulePass *createDPCPPKernelAnalysisLegacyPass();
ModulePass *createDPCPPPreprocessSPIRVFriendlyIRLegacyPass();
ModulePass *createDeduceMaxWGDimLegacyPass();
ModulePass *createDuplicateCalledKernelsLegacyPass();
FunctionPass *createPhiCanonicalizationLegacyPass();
FunctionPass *createRedundantPhiNodeLegacyPass();
ModulePass *
createGroupBuiltinLegacyPass(ArrayRef<Module *> BuiltinModules = {});
FunctionPass *createSoaAllocaAnalysisLegacyPass();
ModulePass *createSplitBBonBarrierLegacyPass();
ModulePass *createWIRelatedValueWrapperPass();
ModulePass *createDataPerBarrierWrapperPass();
ModulePass *createDataPerValueWrapperPass();
ModulePass *createKernelBarrierLegacyPass(bool isNativeDebug,
                                          bool useTLSGlobals);
ModulePass *createBarrierInFunctionLegacyPass();
ModulePass *createImplicitArgsAnalysisLegacyPass();
ModulePass *createImplicitGIDLegacyPass(bool HandleBarrier = true);
ModulePass *createIndirectCallLoweringLegacyPass();
ModulePass *createInstToFuncCallLegacyPass(
    VectorVariant::ISAClass ISA = VectorVariant::XMM);
ModulePass *createInternalizeNonKernelFuncLegacyPass();
ModulePass *createLinearIdResolverPass();
ModulePass *createLocalBufferAnalysisLegacyPass();
ModulePass *createLocalBuffersLegacyPass(bool UseTLSGlobals);
LoopPass *createLoopStridedCodeMotionLegacyPass();
LoopPass *createLoopWIAnalysisLegacyPass();
FunctionPass *createAddNTAttrLegacyPass();
ModulePass *createAddImplicitArgsLegacyPass();
FunctionPass *createAddFastMathLegacyPass();
ModulePass *createAddTLSGlobalsLegacyPass();
ModulePass *createAutorunReplicatorLegacyPass();
ModulePass *createReduceCrossBarrierValuesLegacyPass();
ModulePass *createResolveMatrixFillLegacyPass();
ModulePass *createResolveMatrixLayoutLegacyPass();
ModulePass *createResolveMatrixWISliceLegacyPass();
ModulePass *createResolveSubGroupWICallLegacyPass(
    const SmallVector<Module *, 2> &BuiltinModules = SmallVector<Module *, 2>(),
    bool ResolveSGBarrier = true);
ModulePass *createResolveWICallLegacyPass(bool IsUniformWGSize,
                                          bool UseTLSGlobals);
ModulePass *createSGBarrierPropagateLegacyPass();
ModulePass *createSGBarrierSimplifyLegacyPass();
ModulePass *createSGBuiltinLegacyPass(
    ArrayRef<std::tuple<const char *, const char *, const char *>> VectInfos =
        {});
ModulePass *createSGLoopConstructLegacyPass();
ModulePass *createSGSizeAnalysisLegacyPass();
ModulePass *createSGValueWidenLegacyPass();
ModulePass *createPrepareKernelArgsLegacyPass(bool UseTLSGlobals);
ModulePass *createCleanupWrappedKernelLegacyPass();
ModulePass *createCoerceTypesLegacyPass();
ModulePass *createUpdateCallAttrsLegacyPass();
ModulePass *createVectorVariantFillInLegacyPass();
ModulePass *createVectorVariantLoweringLegacyPass(VectorVariant::ISAClass);
ModulePass *createSGSizeCollectorLegacyPass(VectorVariant::ISAClass);
ModulePass *createSGSizeCollectorIndirectLegacyPass(VectorVariant::ISAClass);
ModulePass *createSetVectorizationFactorLegacyPass(
    VectorVariant::ISAClass ISA = VectorVariant::XMM);
FunctionPass *createSinCosFoldLegacyPass();
ModulePass *createVFAnalysisLegacyPass();
ModulePass *createVectorizationDimensionAnalysisLegacyPass();
ModulePass *
createHandleVPlanMaskLegacyPass(const StringSet<> *VPlanMaskedFuncs);
ModulePass *createWGLoopBoundariesLegacyPass();
FunctionPass *createWorkItemAnalysisLegacyPass(unsigned VectorizeDim = 0);
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_LEGACY_PASSES_H
