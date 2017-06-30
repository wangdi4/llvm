/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
//===----------------------------------------------------------------------===//
//
// This file contains the declarations for the pass initialization routines
// for the OCL project.
//
//===----------------------------------------------------------------------===//

#ifndef OCL_INITIALIZEPASSES_H
#define OCL_INITIALIZEPASSES_H

namespace llvm {
    class PassRegistry;
}

namespace intel {
void initializePhiCanonPass(llvm::PassRegistry&);
void initializePredicatorPass(llvm::PassRegistry&);
void initializeWIAnalysisPass(llvm::PassRegistry&);
void initializeOCLBranchProbabilityPass(llvm::PassRegistry&);
void initializeScalarizeFunctionPass(llvm::PassRegistry&);
void initializeSimplifyGEPPass(llvm::PassRegistry&);
void initializePacketizeFunctionPass(llvm::PassRegistry&);
void initializeX86ResolverPass(llvm::PassRegistry&);
void initializeMICResolverPass(llvm::PassRegistry&);
void initializeAVX512ResolverPass(llvm::PassRegistry&);
void initializeOCLBuiltinPreVectorizationPassPass(llvm::PassRegistry&);
void initializeSpecialCaseBuiltinResolverPass(llvm::PassRegistry&);
void initializeCLWGLoopBoundariesPass(llvm::PassRegistry&);
void initializeCLWGLoopCreatorPass(llvm::PassRegistry&);
void initializeCLStreamSamplerPass(llvm::PassRegistry&);
void initializeKernelAnalysisPass(llvm::PassRegistry&);
void initializeIRInjectModulePass(llvm::PassRegistry&);
void initializenameByInstTypePass(llvm::PassRegistry&);
void initializeDuplicateCalledKernelsPass(llvm::PassRegistry&);
void initializeRedundantPhiNodePass(llvm::PassRegistry&);
void initializeGroupBuiltinPass(llvm::PassRegistry&);
void initializeBarrierInFunctionPass(llvm::PassRegistry&);
void initializeRemoveDuplicationBarrierPass(llvm::PassRegistry&);
void initializeSplitBBonBarrierPass(llvm::PassRegistry&);
void initializeBarrierPass(llvm::PassRegistry&);
void initializeWIRelatedValuePass(llvm::PassRegistry&);
void initializeSinCosFoldPass(llvm::PassRegistry&);
void initializePreLegalizeBoolsPass(llvm::PassRegistry&);
void initializeDataPerBarrierPass(llvm::PassRegistry&);
void initializeDataPerValuePass(llvm::PassRegistry&);
void initializePreventDivCrashesPass(llvm::PassRegistry&);
void initializeBuiltinCallToInstPass(llvm::PassRegistry&);
void initializeInstToFuncCallPass(llvm::PassRegistry&);
void initializeModuleCleanupPass(llvm::PassRegistry&);
void initializeAddImplicitArgsPass(llvm::PassRegistry&);
void initializeOclFunctionAttrsPass(llvm::PassRegistry&);
void initializeOclSyncFunctionAttrsPass(llvm::PassRegistry&);
void initializeBuiltinLibInfoPass(llvm::PassRegistry&);
void initializeLocalBuffersWrapperPass(llvm::PassRegistry&);
void initializeLocalBuffersWithDebugWrapperPass(llvm::PassRegistry&);
void initializeRelaxedPassPass(llvm::PassRegistry&);
void initializeShiftZeroUpperBitsPass(llvm::PassRegistry&);
void initializePrefetchPass(llvm::PassRegistry&);
void initializeLoopWIAnalysisPass(llvm::PassRegistry&);
void initializeSoaAllocaAnalysisPass(llvm::PassRegistry&);
void initializePostDominanceFrontierPass(llvm::PassRegistry&);
void initializeLocalBuffAnalysisPass(llvm::PassRegistry&);
void initializeBIImportPass(llvm::PassRegistry&);
void initializeGenericAddressStaticResolutionPass(llvm::PassRegistry&);
void initializeGenericAddressDynamicResolutionPass(llvm::PassRegistry&);
void initializeSpirMaterializerPass(llvm::PassRegistry&);
void initializeObfuscationPass(llvm::PassRegistry&);
void initializeSubGroupAdaptationPass(llvm::PassRegistry&);
void initializeLinearIdResolverPass(llvm::PassRegistry&);
void initializePrepareKernelArgsPass(llvm::PassRegistry&);
void initializeReduceAlignmentPass(llvm::PassRegistry&);
void initializeDetectFuncPtrCallsPass(llvm::PassRegistry&);
void initializeResolveWICallPass(llvm::PassRegistry&);
void initializeCloneBlockInvokeFuncToKernelPass(llvm::PassRegistry&);
void initializeResolveBlockToStaticCallPass(llvm::PassRegistry&);
void initializeDetectRecursionPass(llvm::PassRegistry&);
void initializeDebugInfoPassPass(llvm::PassRegistry&);
void initializeImplicitArgsAnalysisPass(llvm::PassRegistry&);
void initializeSmartGVNPass(llvm::PassRegistry&);
void initializeDeduceMaxWGDimPass(llvm::PassRegistry&);
void initializeRenderscriptVectorizerPass(llvm::PassRegistry&);
void initializeOCLAliasAnalysisPass(llvm::PassRegistry&);
void initializeSPIR20BlocksToObjCBlocksPass(llvm::PassRegistry&);
void initializePrintfArgumentsPromotionPass(llvm::PassRegistry&);
void initializeBlockToFuncPtrPass(llvm::PassRegistry&);
void initializeChannelPipeTransformationPass(llvm::PassRegistry&);
void initializePipeSupportPass(llvm::PassRegistry&);
void initializeFMASplitterPass(llvm::PassRegistry&);
}

#endif
