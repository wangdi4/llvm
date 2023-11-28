//===-- IntelVPlanDriverPass.h --------------------------------------------===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the VPlan vectorizer driver pass manager interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTELVPLANDRIVERPASS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTELVPLANDRIVERPASS_H

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Vectorize.h"

namespace llvm {
namespace vpo {

class DriverHIRImpl;
class DriverLLVMImpl;

class VPlanDriverLLVMPass : public PassInfoMixin<VPlanDriverLLVMPass> {
  DriverLLVMImpl *Impl = nullptr;
  static bool RunForSycl;
  static bool RunForO0;
  /// Error handler, see the corresponding commment in DriverImpl.
  static VecErrorHandlerTy VecErrorHandler;

public:
  VPlanDriverLLVMPass();
  VPlanDriverLLVMPass(const VPlanDriverLLVMPass &) noexcept;
  VPlanDriverLLVMPass &operator=(const VPlanDriverLLVMPass &) = delete;
  ~VPlanDriverLLVMPass();
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static void setRunForSycl(bool isSycl) { RunForSycl = isSycl; }
  static void setRunForO0(bool isO0Vec) { RunForO0 = isO0Vec; }

  static bool isRequired() { return (RunForSycl || RunForO0); }

  static void setVecErrorHandler(VecErrorHandlerTy H) { VecErrorHandler = H; }
};

class VPlanDriverHIRPass
    : public loopopt::HIRPassInfoMixin<VPlanDriverHIRPass> {
  DriverHIRImpl *Impl = nullptr;

public:
  static constexpr auto PassName = "hir-vplan-vec";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            loopopt::HIRFramework &);
  VPlanDriverHIRPass(bool LightWeightMode, bool WillRunLLVMIRVPlan);
  VPlanDriverHIRPass(const VPlanDriverHIRPass &) noexcept;
  VPlanDriverHIRPass &operator=(const VPlanDriverHIRPass &) = delete;
  ~VPlanDriverHIRPass();
};

} // namespace vpo
} // namespace llvm

#endif
