// ===--------------- Intel_HandleVPlanMask.h ----------- -*- C++ -*------=== //
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_HANDLE_VPLAN_MASK
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_HANDLE_VPLAN_MASK

#include "llvm/IR/PassManager.h"

namespace llvm {
/// Convert VPlan style mask to Volcano style.
class HandleVPlanMask : public PassInfoMixin<HandleVPlanMask> {
public:
  HandleVPlanMask(const StringSet<> *VPlanMaskedFuncs = nullptr);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);

  static bool isRequired() { return true; }

private:
  // TODO:
  // Include VectInfo.gen directly and remove this member after porting the
  // ocl-tblgen into llvm project.
  const StringSet<> *VPlanMaskedFuncs;
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_HANDLE_VPLAN_MASK
