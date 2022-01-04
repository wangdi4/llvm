//==----- Passes.h - DPCPP Kernel transforms pass headers -------*- C++ -*-===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PASSES_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PASSES_H

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AddFastMath.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AddFunctionAttrs.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AddImplicitArgs.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BarrierInFunctionPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinCallToInst.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinImport.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/CleanupWrappedKernel.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/CoerceWin64Types.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPEqualizer.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelVecClone.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelWGLoopCreator.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPPreprocessSPIRVFriendlyIR.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DuplicateCalledKernelsPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/GroupBuiltinPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/HandleVPlanMask.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/InternalizeNonKernelFunc.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LinearIdResolver.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LocalBuffers.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/PhiCanonicalization.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/PrepareKernelArgs.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/RedundantPhiNodePass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveMatrixWISlice.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveSubGroupWICall.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveWICall.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SetVectorizationFactor.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SoaAllocaAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SplitBBonBarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGBarrierPropagate.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGBarrierSimplify.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGBuiltin.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGLoopConstruct.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGSizeAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGValueWiden.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/TaskSeqAsyncHandling.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VFAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/CreateSimdVariantPropagation.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/IndirectCallLowering.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/SGSizeCollector.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/SGSizeCollectorIndirect.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/UpdateCallAttrs.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/VectorVariantFillIn.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/VectorVariantLowering.h"

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PASSES_H
