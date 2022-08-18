//===- IntelVPlanVLSClient.cpp ---------------------------------------------===/
//
//   Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
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

#include "IntelVPlanScalarEvolution.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSAnalysis.h"

using namespace llvm::vpo;

/// Implementation of OVLSMemref::getConstStride.
static Optional<int64_t> getConstStrideImpl(const SCEV *Expr,
                                            VPlanScalarEvolutionLLVM &VPSE) {
  VPlanSCEV *VPExpr = VPSE.toVPlanSCEV(Expr);
  Optional<VPConstStepLinear> Linear = VPSE.asConstStepLinear(VPExpr);
  return Linear.transform([](auto &L) { return L.Step; });
}

static Optional<int64_t>
getConstDistanceFromImpl(const SCEV *LHS, const SCEV *RHS,
                         VPlanScalarEvolutionLLVM &VPSE) {
  // Early exit to improve compile time. If the types don't match, there's no
  // sense trying to compute distance between pointers. Pointers to the same
  // allocation always have the same type.
  if (LHS->getType() !=RHS->getType())
    return None;

  VPlanSCEV *VPMinus =
      VPSE.getMinusExpr(VPSE.toVPlanSCEV(LHS), VPSE.toVPlanSCEV(RHS));
  const SCEV *Minus = VPSE.toSCEV(VPMinus);

  auto *Const = dyn_cast<SCEVConstant>(Minus);
  if (!Const)
    return None;

  return Const->getAPInt().getSExtValue();
}

Optional<int64_t>
VPVLSClientMemref::getConstDistanceFrom(const OVLSMemref &From) {
  const VPLoadStoreInst *FromInst = cast<VPVLSClientMemref>(From).Inst;
  const SCEV *FromScev = getAddressSCEV(FromInst);

  // Don't waste time if memrefs are in different basic blocks. This case is not
  // supported yet.
  if (Inst->getParent() != FromInst->getParent())
    return None;

  return getConstDistanceFromImpl(getAddressSCEV(Inst), FromScev, getVPSE());
}

// FIXME: This is an extremely naive implementation just to enable the most
// simple G2S cases. It is expected to be replaced with a full-fledged Data
// Dependence analysis.
bool VPVLSClientMemref::canMoveTo(const OVLSMemref &ToMemRef) {
  const VPLoadStoreInst *ToInst = cast<VPVLSClientMemref>(ToMemRef).Inst;
  const VPLoadStoreInst *FromInst = Inst;
  const SCEV *FromSCEV = getAddressSCEV(FromInst);

  if (ToInst == FromInst)
    return true;

  // At this point, only same block movement is supported.
  if (ToInst->getParent() != FromInst->getParent())
    return false;

  VPlanScalarEvolutionLLVM &VPSE = getVPSE();
  Type *AccessType = FromInst->getValueType();
  int64_t AccessSize = VLSA->getDL().getTypeStoreSize(AccessType);

  if (!FromSCEV)
    return false;

  Optional<int64_t> FromStride = getConstStrideImpl(FromSCEV, VPSE);
  if (!FromStride)
    return false;

  // Check if it is safe to move FromInst past every instruction between it and
  // ToInst. We consider only moving loads up and moving stores down.

  SmallVector<const VPInstruction *, 64> RangeToCheck;
  if (getAccessKind().isLoad())
    for (const VPInstruction *I = FromInst->getPrevNode(); I != nullptr;
         I = I->getPrevNode())
      RangeToCheck.push_back(I);
  else
    for (const VPInstruction *I = FromInst->getNextNode(); I != nullptr;
         I = I->getNextNode())
      RangeToCheck.push_back(I);

  for (const VPInstruction *IterInst : RangeToCheck) {
    // Bail out if we run into an unexpected instruction.
    if (!IterInst)
      return false;

    // ToInst has been safely reached by the algorithm.
    if (IterInst == ToInst)
      return true;

    // It is safe to move past an instruction without side effects nor memory
    // access.
    if (auto *I = dyn_cast_or_null<Instruction>(IterInst->getUnderlyingValue()))
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
    // If FromInst is A[4*(i+0)] and IterInst fits completely into area SL or
    // area SR, then it is safe to swap IterInst and FromInst.
    if (auto *LoadStore = dyn_cast<VPLoadStoreInst>(IterInst)) {
      auto *IterSCEV = getAddressSCEV(LoadStore);
      if (!IterSCEV)
        return false;

      // Constant distance between From and IterInst implies that the strides of
      // IterInst and From are the same.
      Optional<int64_t> Distance =
          getConstDistanceFromImpl(IterSCEV, FromSCEV, VPSE);
      if (!Distance)
        return false;

      Type *IterType = LoadStore->getValueType();
      int64_t IterAccessSize = VLSA->getDL().getTypeStoreSize(IterType);
      if (IterAccessSize != AccessSize)
        return false;

      if (std::abs(*Distance) >= AccessSize &&
          std::abs(*Distance) <= std::abs(*FromStride) - AccessSize) {
        // Pattern has been recoginized. It is safe to move From past IterInst.
        continue;
      }
    }

    // Otherwise, the move is not safe.
    return false;
  }

  // ToInst has not been found in the basic block. Probably, it is in the
  // opposite direction.
  return false;
}

Optional<int64_t> VPVLSClientMemref::getConstStride() const {
  return getConstStrideImpl(getAddressSCEV(Inst), getVPSE());
}

bool VPVLSClientMemref::dominates(const OVLSMemref &Mrf) const {
  const VPInstruction *Other = cast<VPVLSClientMemref>(Mrf).getInstruction();
  // FIXME: we should check for basic block dominance if the two instructions
  // are not in the same basic block.
  if (Inst->getParent() != Other->getParent())
    return false;

  const VPInstruction *Iter = Other;
  for (; Iter; Iter = Iter->getPrevNode())
    if (Iter == Inst)
      return true;

  return false;
}

bool VPVLSClientMemref::postDominates(const OVLSMemref &Mrf) const {
  const VPInstruction *Other = cast<VPVLSClientMemref>(Mrf).getInstruction();
  // FIXME: we should check for basic block postdominance if the two
  // instructions are not in the same basic block.
  if (Inst->getParent() != Other->getParent())
    return false;

  const VPInstruction *Iter = Other;
  for (; Iter; Iter = Iter->getNextNode())
    if (Iter == Inst)
      return true;

  return false;
}
