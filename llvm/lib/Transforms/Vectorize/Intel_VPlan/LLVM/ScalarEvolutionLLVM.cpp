//===-- ScalarEvolutionLLVM.cpp ---------------------------------*- C++ -*-===//
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
/// \file ScalarEvolutionLLVM.cpp
/// VPlan vectorizer's SCEV-like analysis for LLVM IR path.
///
/// Split from ScalarEvolution.cpp on 2023-10-01.
///
//===----------------------------------------------------------------------===//

#include "ScalarEvolutionLLVM.h"
#include "../IntelVPlan.h"

#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <optional>

#define DEBUG_TYPE "vplan-scalar-evolution"

using namespace llvm;
using namespace llvm::vpo;

VPlanSCEV *
VPlanScalarEvolutionLLVM::computeAddressSCEV(const VPLoadStoreInst &LSI) {
  assert(!LSI.getAddressSCEV() && "This method must not be invoked once "
                                  "AddressSCEV is computed for an LSI");
  const VPValue &Ptr = *LSI.getPointerOperand();
  if (!Ptr.isUnderlyingIRValid())
    return nullptr;
  if (maybePointerToPrivateMemory(Ptr))
    return nullptr;
  const SCEV *Expr = SE->getSCEV(Ptr.getUnderlyingValue());
  return toVPlanSCEV(Expr);
}

VPlanSCEV *VPlanScalarEvolutionLLVM::getMinusExpr(VPlanSCEV *LHS,
                                                  VPlanSCEV *RHS) {
  Type *PtrIntTy = Type::getIntNTy(Context, DL->getPointerSizeInBits());
  // Pointers with different bases require explicit conversion to int.
  auto LHSPtrToIntSCEV = SE->getPtrToIntExpr(toSCEV(LHS), PtrIntTy);
  auto RHSPtrToIntSCEV = SE->getPtrToIntExpr(toSCEV(RHS), PtrIntTy);
  const SCEV *Minus = SE->getMinusSCEV(LHSPtrToIntSCEV, RHSPtrToIntSCEV);
  return toVPlanSCEV(Minus);
}

std::optional<VPConstStepLinear>
VPlanScalarEvolutionLLVM::asConstStepLinear(VPlanSCEV *Expr) const {
  // FIXME: We cannot really look for linear values using llvm::ScalarEvolution.
  //        The best we can do is to look for induction variables instead. That
  //        means that we inevitably miss linear values that are not IV. For
  //        example, the address of the following load is linear but it is not
  //        IV, so we do not detect it as linear:
  //
  //          for (i = 0; i < iN; ++i)
  //            for (j = 0; j < jN; ++j)
  //              x = ptr[i + j*j];
  //
  std::optional<VPConstStepInduction> Ind = asConstStepInduction(Expr);
  return llvm::transformOptional(Ind, [](auto &I) {
    return VPConstStepLinear{I.InvariantBase, I.Step};
  });
}

std::optional<VPConstStepInduction>
VPlanScalarEvolutionLLVM::asConstStepInduction(VPlanSCEV *Expr) const {
  if (!Expr)
    return std::nullopt;

  const SCEV *ScevExpr = toSCEV(Expr);
  if (!isa<SCEVAddRecExpr>(ScevExpr))
    return std::nullopt;

  auto *AddRec = cast<SCEVAddRecExpr>(ScevExpr);
  if (AddRec->getLoop() != MainLoop || !AddRec->isAffine())
    return std::nullopt;

  auto *ConstStepExpr = dyn_cast<SCEVConstant>(AddRec->getOperand(1));
  if (!ConstStepExpr)
    return std::nullopt;

  auto *Base = AddRec->getOperand(0);
  auto Step = ConstStepExpr->getAPInt().getSExtValue();
  return VPConstStepInduction{toVPlanSCEV(Base), Step};
}
