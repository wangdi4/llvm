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

// FIXME: This is an extremely naive implementation just to enable the most
// simple G2S cases. It is expected to be replaced with a full-fledged Data
// Dependence analysis.
bool VPVLSClientMemref::canMoveTo(const OVLSMemref &ToMemRef) {
  const VPInstruction *ToInst = cast<VPVLSClientMemref>(ToMemRef).Inst;
  const VPInstruction *FromInst = Inst;

  // At this point, only same block movement is supported.
  if (ToInst->getParent() != FromInst->getParent())
    return false;

  ScalarEvolution *SE = VLSA->getSE();
  Type *AccessType = getLoadStoreType(FromInst);
  int64_t AccessSize = VLSA->getDL().getTypeStoreSize(AccessType);

  const SCEV *FromSCEV =
      getSCEVForVPValue(getLoadStorePointerOperand(FromInst));
  if (isa<SCEVCouldNotCompute>(FromSCEV))
    return false;

  int64_t FromStride;
  if (!hasAConstStrideImpl(FromSCEV, &FromStride, VLSA->getMainLoop()))
    return false;

  // Check if it is safe to move FromInst past every instruction between it and
  // ToInst. ToInst is assumed to precede FromInst.
  // FIXME: It is expected that VLS will be changed so that loads are moved
  // upward and stores are moved downward. We will need to support downward
  // movement when such change is implemented.
  const VPInstruction *Iter = FromInst;
  while ((Iter = dyn_cast_or_null<VPInstruction>(Iter->getPrevNode()))) {
    // ToInst has not been found in the basic block. Probably, it is in the
    // opposite direction.
    if (!Iter)
      return false;

    // ToInst has been safely reached by the algorithm.
    if (Iter == ToInst)
      return true;

    // Cannot move a Store instruction past the definition of the stored value.
    if (FromInst->getOpcode() == Instruction::Store &&
        Iter == FromInst->getOperand(0))
      return false;

    // It is safe to move past an instruction without side effects nor memory
    // access.
    if (auto *I = dyn_cast_or_null<Instruction>(Iter->getUnderlyingValue()))
      if (!I->mayHaveSideEffects() && !I->mayReadFromMemory())
        continue;

    // Allow moving a memory operation past another memory operation if they
    // cannot overlap. We don't try to do full-fledged data dependence analysis
    // here. The only allowed specific pattern is if the two adjacent memory
    // references:
    //   1) have the same stride and access size
    //   2) are far enough to not overlap (distance >= sizeof(ref))
    //   3) are close enough to rule out possible loop-carried dependence
    //      (distance <= stride - sizeof(ref))
    // then it is safe to swap those memory references.
    //
    // For example, given the following access pattern for FromInst (A[4*i]):
    //   +------------+--------------+------------+--------------+------------+
    //   | A[4*(i-1)] |      SL      | A[4*(i+0)] |      SR      | A[4*(i+1)] |
    //   +------------+--------------+------------+--------------+------------+
    // If FromInst is A[4*(i+0)] and Iter fits completely into area SL or area
    // SR, then it is safe to swap Iter and FromInst.
    if (Iter->getOpcode() == Instruction::Load ||
        Iter->getOpcode() == Instruction::Store) {
      auto *IterSCEV = getSCEVForVPValue(getLoadStorePointerOperand(Iter));
      int64_t Distance;

      // Constant distance between From and Iter implies that the strides of
      // Iter and From are the same.
      if (!isAConstDistanceFromImpl(IterSCEV, FromSCEV, SE, &Distance))
        return false;

      Type *IterType = getLoadStoreType(Iter);
      int64_t IterAccessSize = VLSA->getDL().getTypeStoreSize(IterType);
      if (IterAccessSize != AccessSize)
        return false;

      if (std::abs(Distance) >= AccessSize &&
          std::abs(Distance) <= std::abs(FromStride) - AccessSize) {
        // Pattern has been recoginized. It is safe to move From past Iter.
        continue;
      }
    }

    // Otherwise, the move is not safe.
    return false;
  }

  llvm_unreachable("Moved past the loop supposed to return from function");
}

bool VPVLSClientMemref::hasAConstStride(int64_t *Stride) const {
  auto *Expr = getSCEVForVPValue(getLoadStorePointerOperand(Inst));
  return hasAConstStrideImpl(Expr, Stride, VLSA->getMainLoop());
}
