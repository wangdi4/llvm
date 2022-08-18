//===- IntelVPlanScalarEvolution.cpp ----------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanScalarEvolution.h"

#include "IntelVPlan.h"

#include <llvm/Analysis/ScalarEvolutionExpressions.h>

#define DEBUG_TYPE "vplan-scalar-evolution"

using namespace llvm;
using namespace llvm::vpo;

bool VPlanScalarEvolution::maybePointerToPrivateMemory(const VPValue &V) {
  if (isa<VPExternalDef>(V) || isa<VPConstant>(V))
    return false;

  const auto &VPI = cast<VPInstruction>(V);

  if (VPI.isCast() || isa<VPGEPInstruction>(VPI) || isa<VPSubscriptInst>(VPI))
    return maybePointerToPrivateMemory(*VPI.getOperand(0));

  // TODO: Look through more instruction kinds. Particularly, we may want to
  //       look through PHI nodes.

  return true;
}

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

Optional<VPConstStepLinear>
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
  Optional<VPConstStepInduction> Ind = asConstStepInduction(Expr);
  return Ind.transform([](auto &I) {
    return VPConstStepLinear{I.InvariantBase, I.Step};
  });
}

Optional<VPConstStepInduction>
VPlanScalarEvolutionLLVM::asConstStepInduction(VPlanSCEV *Expr) const {
  if (!Expr)
    return None;

  const SCEV *ScevExpr = toSCEV(Expr);
  if (!isa<SCEVAddRecExpr>(ScevExpr))
    return None;

  auto *AddRec = cast<SCEVAddRecExpr>(ScevExpr);
  if (AddRec->getLoop() != MainLoop || !AddRec->isAffine())
    return None;

  auto *ConstStepExpr = dyn_cast<SCEVConstant>(AddRec->getOperand(1));
  if (!ConstStepExpr)
    return None;

  auto *Base = AddRec->getOperand(0);
  auto Step = ConstStepExpr->getAPInt().getSExtValue();
  return VPConstStepInduction{toVPlanSCEV(Base), Step};
}
