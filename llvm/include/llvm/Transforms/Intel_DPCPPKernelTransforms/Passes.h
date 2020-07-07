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
ModulePass* createDPCPPKernelVecClonePass();
ModulePass* createDPCPPKernelPostVecPass();
ModulePass* createDPCPPKernelWGLoopCreatorPass();
ModulePass* createDPCPPKernelAnalysisPass();
FunctionPass* createPhiCanonicalizationPass();
FunctionPass* createRedundantPhiNodePass();
ModulePass* createSplitBBonBarrierPass();
ModulePass* createWIRelatedValuePass();
ModulePass* createDataPerBarrierPass();
ModulePass* createDataPerValuePass();
ModulePass* createKernelBarrierPass(bool isNativeDebug, bool useTLSGlobals);
ModulePass* createBarrierInFunctionPass();

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PASSES_H
