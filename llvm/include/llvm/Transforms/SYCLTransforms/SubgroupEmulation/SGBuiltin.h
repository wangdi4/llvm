//=--- SGBuiltin.h -Insert sub_group_barrier & vector-variants attribute ----=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SUBGROUP_EMULATION_BUILTIN_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SUBGROUP_EMULATION_BUILTIN_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGSizeAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

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
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_SUBGROUP_EMULATION_GROUP_BUILTIN_H
