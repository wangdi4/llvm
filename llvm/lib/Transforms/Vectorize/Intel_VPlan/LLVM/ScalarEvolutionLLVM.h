//===- ScalarEvolutionLLVM.h ------------------------------------*- C++ -*-===//
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2020 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//===----------------------------------------------------------------------===//
///
/// \file ScalarEvolutionLLVM.h
/// VPlan vectorizer's SCEV-like analysis for LLVM IR path.
///
/// Split from ScalarEvolution.h on 2023-10-01.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LLVM_SCALAR_EVOLUTION_LLVM_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LLVM_SCALAR_EVOLUTION_LLVM_H

#include "../ScalarEvolution.h"

namespace llvm {
class ScalarEvolution;
class Loop;
class LLVMContext;
class DataLayout;
class SCEV;

namespace vpo {
/// Implementation of VPlanScalarEvolution for LLVM IR.
class VPlanScalarEvolutionLLVM : public VPlanScalarEvolution {
public:
  VPlanScalarEvolutionLLVM(ScalarEvolution &SE, const Loop *MainLoop,
                           LLVMContext &Context, const DataLayout *DL)
      : SE(&SE), MainLoop(MainLoop), Context(Context), DL(DL) {}

  VPlanSCEV *computeAddressSCEV(const VPLoadStoreInst &LSI) override;

  VPlanSCEV *getMinusExpr(VPlanSCEV *LHS, VPlanSCEV *RHS) override;

  std::optional<VPConstStepLinear>
  asConstStepLinear(VPlanSCEV *Expr) const override;

  std::optional<VPConstStepInduction>
  asConstStepInduction(VPlanSCEV *Expr) const override;

  ScalarEvolution &getSE() { return *SE; }

  static const SCEV *toSCEV(VPlanSCEV *Expr) {
    return reinterpret_cast<SCEV *>(Expr);
  }

  static VPlanSCEV *toVPlanSCEV(const SCEV *Expr) {
    return reinterpret_cast<VPlanSCEV *>(const_cast<SCEV *>(Expr));
  }

private:
  ScalarEvolution *SE;
  const Loop *MainLoop;
  LLVMContext &Context;
  const DataLayout *DL;
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LLVM_SCALAR_EVOLUTION_LLVM_H
