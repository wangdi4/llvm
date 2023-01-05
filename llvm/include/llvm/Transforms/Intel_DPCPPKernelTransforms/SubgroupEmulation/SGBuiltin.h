//=--- SGBuiltin.h -Insert sub_group_barrier & vector-variants attribute ----=//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_SUBGROUP_EMULATION_BUILTIN_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_SUBGROUP_EMULATION_BUILTIN_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGSizeAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"

namespace llvm {


class SGBuiltinPass : public PassInfoMixin<SGBuiltinPass> {
public:
  SGBuiltinPass(ArrayRef<VectItem> = {});

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  /// Glue for old PM.
  bool runImpl(Module &M, const SGSizeInfo *SSI);

private:
  /// Add vector-variants attribute for sub-group functions.
  /// Insert sg_barrier/dummy_sg_barrier before/after the call.
  bool insertSGBarrierForSGCalls(Module &M, const SGSizeInfo *SSI);

  /// Insert sg_barrier/dummy_sg_barrier for WG Barriers
  bool insertSGBarrierForWGBarriers(Module &M, const SGSizeInfo *SSI);

  SGHelper Helper;
  // Configuration options
  ArrayRef<VectItem> VectInfos;
};
} // namespace llvm
#endif // INTEL_DPCPP_KERNEL_TRANSFORMS_SUBGROUP_EMULATION_GROUP_BUILTIN_H
