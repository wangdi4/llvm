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

/// Implementation of OVLSMemref::getConstStride. The additional parameter
/// \p MainLoop specifies the loop being vectorized, so that we compute stride
/// with respect to this loop.
static Optional<int64_t> getConstStrideImpl(const SCEV *Expr,
                                            const Loop *MainLoop) {
  if (!isa<SCEVAddRecExpr>(Expr))
    return None;

  auto *AddRec = cast<SCEVAddRecExpr>(Expr);

  // FIXME: So far, computing stride for nested loop IVs is not supported. This
  // should be fixed in future patches.
  if (AddRec->getLoop() != MainLoop)
    return None;

  if (!AddRec->isAffine())
    return None;

  assert(AddRec->getNumOperands() == 2 &&
         "Affine SCEV is expected to have only two operands");

  if (auto *ConstStep = dyn_cast<SCEVConstant>(AddRec->getOperand(1)))
    return ConstStep->getAPInt().getSExtValue();

  return None;
}

static Optional<int64_t> getConstDistanceFromImpl(const SCEV *LHS,
                                                  const SCEV *RHS,
                                                  ScalarEvolution *SE) {
  // computeConstantDifference has a significant advantage over getMinusSCEV: it
  // doesn't crash if LHS and RHS contain AddRecs for unrelated loops (e.g.
  // sibling loops).
  Optional<APInt> Difference = SE->computeConstantDifference(LHS, RHS);
  if (!Difference)
    return None;
  return Difference->getSExtValue();
}

// FIXME: It is not safe to call this method after we start modifying IR, as
//        modifying IR invalidates ScalarEvolution. It'd be better to remove
//        this method, but it is still used in canMoveTo.
const SCEV *VPVLSClientMemref::getSCEVForVPValue(const VPValue *Val) const {
  ScalarEvolution *SE = VLSA->getSE();
  return Val->isUnderlyingIRValid() ? SE->getSCEV(Val->getUnderlyingValue())
                                    : SE->getCouldNotCompute();
}

VPVLSClientMemref::VPVLSClientMemref(const OVLSMemrefKind &Kind,
                                     OVLSAccessKind AccKind, const OVLSType &Ty,
                                     const VPInstruction *Inst,
                                     const VPlanVLSAnalysis *VLSA)
    : OVLSMemref(Kind, Ty, AccKind), Inst(Inst), VLSA(VLSA) {
  if (Kind == OVLSMemref::VLSK_VPlanVLSClientMemref)
    ScevExpr = getSCEVForVPValue(getLoadStorePointerOperand(Inst));
}

Optional<int64_t>
VPVLSClientMemref::getConstDistanceFrom(const OVLSMemref &From) {
  const VPInstruction *FromInst = cast<VPVLSClientMemref>(From).Inst;
  const SCEV *FromScev = cast<VPVLSClientMemref>(From).ScevExpr;

  // Don't waste time if memrefs are in different basic blocks. This case is not
  // supported yet.
  if (Inst->getParent() != FromInst->getParent())
    return None;

  return getConstDistanceFromImpl(ScevExpr, FromScev, VLSA->getSE());
}

// FIXME: This is an extremely naive implementation just to enable the most
// simple G2S cases. It is expected to be replaced with a full-fledged Data
// Dependence analysis.
bool VPVLSClientMemref::canMoveTo(const OVLSMemref &ToMemRef) {
  const VPInstruction *ToInst = cast<VPVLSClientMemref>(ToMemRef).Inst;
  const VPInstruction *FromInst = Inst;
  const SCEV *FromSCEV = ScevExpr;

  // At this point, only same block movement is supported.
  if (ToInst->getParent() != FromInst->getParent())
    return false;

  ScalarEvolution *SE = VLSA->getSE();
  Type *AccessType = getLoadStoreType(FromInst);
  int64_t AccessSize = VLSA->getDL().getTypeStoreSize(AccessType);

  if (isa<SCEVCouldNotCompute>(FromSCEV))
    return false;

  const Loop *MainLoop = VLSA->getMainLoop();
  Optional<int64_t> FromStride = getConstStrideImpl(FromSCEV, MainLoop);
  if (!FromStride)
    return false;

  // Check if it is safe to move FromInst past every instruction between it and
  // ToInst. ToInst is assumed to precede FromInst.
  // FIXME: It is expected that VLS will be changed so that loads are moved
  // upward and stores are moved downward. We will need to support downward
  // movement when such change is implemented.
  for (const VPRecipeBase *I = FromInst->getPrevNode(); I != nullptr;
       I = I->getPrevNode()) {
    const VPInstruction *IterInst = dyn_cast<VPInstruction>(I);

    // Bail out if we run into an unexpected recipe.
    if (!IterInst)
      return false;

    // ToInst has been safely reached by the algorithm.
    if (IterInst == ToInst)
      return true;

    // Cannot move a Store instruction past the definition of the stored value.
    if (FromInst->getOpcode() == Instruction::Store &&
        IterInst == FromInst->getOperand(0))
      return false;

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
    if (IterInst->getOpcode() == Instruction::Load ||
        IterInst->getOpcode() == Instruction::Store) {
      auto *IterSCEV = getSCEVForVPValue(getLoadStorePointerOperand(IterInst));

      // Constant distance between From and IterInst implies that the strides of
      // IterInst and From are the same.
      Optional<int64_t> Distance =
          getConstDistanceFromImpl(IterSCEV, FromSCEV, SE);
      if (!Distance)
        return false;

      Type *IterType = getLoadStoreType(IterInst);
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
  return getConstStrideImpl(ScevExpr, VLSA->getMainLoop());
}
