//===- ScalarEvolutionHIR.h -------------------------------------*- C++ -*-===//
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021 Intel Corporation
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
/// \file ScalarEvolutionHIR.h
/// VPlan vectorizer's SCEV-like analysis for HIR path.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_SCALAR_EVOLUTION_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_SCALAR_EVOLUTION_HIR_H

#include "../ScalarEvolution.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

namespace llvm {

namespace loopopt {
class CanonExpr;
class HLLoop;
class RegDDRef;
}

namespace vpo {

// Substitute for SCEVAddRecExpr for HIR. The semantics of the expression is
// equivalent to {Base,+,Stride}<MainLoop>.
struct VPlanAddRecHIR {
  loopopt::CanonExpr *Base;
  // Original ref that the Base CanonExpr was part of. Used to make
  // ref created from Base consistent during vector code generation.
  // TODO - we need to see if we need to keep a vector of Refs for
  // tracking MinusExpr SCEVs. At this point, this does not look
  // necessary.
  const loopopt::RegDDRef *Ref;
  int64_t Stride;

  VPlanAddRecHIR(loopopt::CanonExpr *Base, int64_t Stride,
                 const loopopt::RegDDRef *Ref = nullptr)
      : Base(Base), Ref(Ref), Stride(Stride) {}
};

/// Implementation of VPlanScalarEvolution for HIR.
class VPlanScalarEvolutionHIR final : public VPlanScalarEvolution {
public:
  VPlanScalarEvolutionHIR(loopopt::HLLoop *MainLoop) : MainLoop(MainLoop) {}

  VPlanSCEV *computeAddressSCEV(const VPLoadStoreInst &LSI) override;

  VPlanSCEV *getMinusExpr(VPlanSCEV *LHS, VPlanSCEV *RHS) override;

  std::optional<VPConstStepLinear>
  asConstStepLinear(VPlanSCEV *Expr) const override;

  std::optional<VPConstStepInduction>
  asConstStepInduction(VPlanSCEV *Expr) const override;

public:
  static VPlanSCEV *toVPlanSCEV(VPlanAddRecHIR *Expr) {
    return reinterpret_cast<VPlanSCEV *>(Expr);
  }

  static VPlanAddRecHIR *toVPlanAddRecHIR(VPlanSCEV *Expr) {
    return reinterpret_cast<VPlanAddRecHIR *>(Expr);
  }

private:
  VPlanAddRecHIR *computeAddressSCEVImpl(const VPLoadStoreInst &LSI);

  VPlanAddRecHIR *getMinusExprImpl(VPlanAddRecHIR *LHS, VPlanAddRecHIR *RHS);

  VPlanAddRecHIR *
  makeVPlanAddRecHIR(loopopt::CanonExpr *Base, int64_t Stride,
                     const loopopt::RegDDRef *Ref = nullptr) const;

private:
  // Set of all VPlanAddRecHIR expressions created by the analysis. The analysis
  // owns the expressions. As soon as the analysis goes out of scope, all the
  // expressions are deallocated.
  // TODO: Replace SmallVector with FoldingSet.
  mutable SmallVector<std::unique_ptr<VPlanAddRecHIR>, 0> Storage;
  loopopt::HLLoop *MainLoop;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
raw_ostream &operator<<(raw_ostream &OS, const VPlanAddRecHIR &E);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_HIR_SCALAR_EVOLUTION_HIR_H
