//=== SGBarrierPropagate.h - Propagate subgroup barrier to all sync funcs ===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SG_EMULATION_SG_BARRIER_PROPAGATE_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SG_EMULATION_SG_BARRIER_PROPAGATE_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGSizeAnalysis.h"

namespace llvm {

/// This pass inserts subgroup barriers and dummy barriers for all sync
/// functions. A sync function calls sub_group_barrier directly/indirectly.
/// 1. Insert a dummy_sg_barrier before the first instruction for sync function.
/// 2. Insert a sub_group_barrier before the return instruction for sync
///    function.
/// 3. Insert a sub_group_barrier before the call site of sync function.
/// 4. Insert a dummy_sg_barrier after the call site of sync function.
class SGBarrierPropagatePass : public PassInfoMixin<SGBarrierPropagatePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, const SGSizeInfo *SSI);

private:
  /// Insert sub_group_barrier at the end of the function.
  /// Insert dummy_sg_barrier at the beginning of the function.
  void insertBarrierToFunction(Function &F);

  SGHelper Helper;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_SG_EMULATION_SG_BARRIER_PROPAGATE_H
