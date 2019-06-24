//===- IntelVPlanVLSClient.cpp ---------------------------------------------===/
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This is an implementation file for IntelVPlanVLSClient.
//===----------------------------------------------------------------------===//

#include "IntelVPlanVLSClient.h"

#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSAnalysis.h"

using namespace llvm::vpo;

/// The purpose of this function is to make sure no overflow happens when
/// writing \p Value value to \p Dest. The signature asserts that the type of
/// \p Dest matches getSExtValue() return type exactly. If they diverge in
/// future, we will need to modify the check.
static void
writeSignedAPInt(decltype(std::declval<APInt>().getSExtValue()) *Dest,
                 const APInt &Value) {
  *Dest = Value.getSExtValue();
}

/// Implementation of OVLSMemref::hasAConstStride. The additional parameter
/// \p MainLoop specifies the loop being vectorized, so that we compute stride
/// with respect to this loop.
static bool hasAConstStrideImpl(const SCEV *Expr, int64_t *Stride,
                                const Loop *MainLoop) {
  if (!isa<SCEVAddRecExpr>(Expr))
    return false;

  auto *AddRec = cast<SCEVAddRecExpr>(Expr);

  // FIXME: So far, computing stride for nested loop IVs is not supported. This
  // should be fixed in future patches.
  if (AddRec->getLoop() != MainLoop)
    return false;

  if (!AddRec->isAffine())
    return false;

  assert(AddRec->getNumOperands() == 2 &&
         "Affine SCEV is expected to have only two operands");

  if (auto *ConstStep = dyn_cast<SCEVConstant>(AddRec->getOperand(1))) {
    writeSignedAPInt(Stride, ConstStep->getAPInt());
    return true;
  }

  return false;
}

static bool isAConstDistanceFromImpl(const SCEV *LHS, const SCEV *RHS,
                                     ScalarEvolution *SE, int64_t *Dist) {
  // computeConstantDifference has a significant advantage over getMinusSCEV: it
  // doesn't crash if LHS and RHS contain AddRecs for unrelated loops (e.g.
  // sibling loops).
  Optional<APInt> Difference = SE->computeConstantDifference(LHS, RHS);
  if (!Difference)
    return false;

  writeSignedAPInt(Dist, *Difference);
  return true;
}

const SCEV *VPVLSClientMemref::getSCEVForVPValue(const VPValue *Val) const {
  ScalarEvolution *SE = VLSA->getSE();
  Value *Underlying = Val->getUnderlyingValue();
  if (!Underlying)
    return SE->getCouldNotCompute();

  return SE->getSCEV(Underlying);
}

bool VPVLSClientMemref::isAConstDistanceFrom(const OVLSMemref &From,
                                             int64_t *Dist) {
  const VPInstruction *FromInst = cast<VPVLSClientMemref>(From).Inst;

  // Don't waste time if memrefs are in different basic blocks. This case is not
  // supported yet.
  if (Inst->getParent() != FromInst->getParent())
    return false;

  auto *ThisSCEV = getSCEVForVPValue(getLoadStorePointerOperand(Inst));
  auto *FromSCEV = getSCEVForVPValue(getLoadStorePointerOperand(FromInst));
  return isAConstDistanceFromImpl(ThisSCEV, FromSCEV, VLSA->getSE(), Dist);
}

bool VPVLSClientMemref::hasAConstStride(int64_t *Stride) const {
  auto *Expr = getSCEVForVPValue(getLoadStorePointerOperand(Inst));
  return hasAConstStrideImpl(Expr, Stride, VLSA->getMainLoop());
}
