//===---------------- SGFunctionWiden.h - Widen functions ----------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SG_EMULATION_SG_FUNCTION_WIDEN_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SG_EMULATION_SG_FUNCTION_WIDEN_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/BarrierUtils.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

namespace llvm {
struct VFInfo;

class FunctionWidener {
public:
  void run(CompilationUtils::FuncSet &Functions,
           DenseMap<Function *, std::set<Function *>> &FuncMap);

private:
  SGHelper Helper;

  BarrierUtils Utils;

  CompilationUtils::FuncSet WGSyncFunctions;

  CompilationUtils::FuncSet FunctionsToWiden;

  /// Remove byval attribute for vector parameters.
  void removeByValAttr(Function &F);

  /// Make a copy of the function if it is marked as SIMD.
  Function *cloneFunction(Function &F, const VFInfo &V,
                          ValueToValueMapTy &Vmap);

  /// Update uses for widened parameters.
  void expandVectorParameters(Function *Clone, const VFInfo &V,
                              ValueToValueMapTy &Vmap);

  /// Update return value.
  void expandReturn(Function *Clone, bool IsWGSyncFunction);

  bool isWideCall(Instruction *I);

  Instruction *getInsertPoint(Instruction *I, Value *V);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SG_EMULATION_SG_FUNCTION_WIDEN_H
