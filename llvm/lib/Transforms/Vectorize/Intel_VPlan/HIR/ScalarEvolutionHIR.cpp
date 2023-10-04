//===- ScalarEvolutionHIR.cpp -----------------------------------*- C++ -*-===//
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
/// \file ScalarEvolutionHIR.cpp
/// VPlan vectorizer's SCEV-like analysis for HIR path.
///
//===----------------------------------------------------------------------===//

#include "ScalarEvolutionHIR.h"
#include "../IntelVPlan.h"
#include "../IntelVPlanValue.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"

#define DEBUG_TYPE "vplan-scalar-evolution"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::vpo;

namespace {
template <typename T> struct DerefOrNilHelper {
  const T *Ptr;
  DerefOrNilHelper(const T *Ptr) : Ptr(Ptr) {}
};

template <typename T>
raw_ostream &operator<<(raw_ostream &OS, DerefOrNilHelper<T> Helper) {
  if (Helper.Ptr)
    OS << *Helper.Ptr;
  else
    OS << "nil";
  return OS;
}

template <typename T> DerefOrNilHelper<T> derefOrNil(T *Ptr) { return Ptr; }
} // namespace

VPlanSCEV *
VPlanScalarEvolutionHIR::computeAddressSCEV(const VPLoadStoreInst &LSI) {
  LLVM_DEBUG(dbgs() << "computeAddressSCEV(" << LSI << ")\n");

  VPlanAddRecHIR *Expr = computeAddressSCEVImpl(LSI);

  LLVM_DEBUG(dbgs() << "  -> " << derefOrNil(Expr) << '\n');

  return toVPlanSCEV(Expr);
}

VPlanSCEV *VPlanScalarEvolutionHIR::getMinusExpr(VPlanSCEV *OpaqueLHS,
                                                 VPlanSCEV *OpaqueRHS) {
  VPlanAddRecHIR *LHS = toVPlanAddRecHIR(OpaqueLHS);
  VPlanAddRecHIR *RHS = toVPlanAddRecHIR(OpaqueRHS);

  LLVM_DEBUG(dbgs() << "getMinusExpr(" << derefOrNil(LHS) << ",\n");
  LLVM_DEBUG(dbgs() << "             " << derefOrNil(RHS) << ")\n");

  VPlanAddRecHIR *Result = getMinusExprImpl(LHS, RHS);

  LLVM_DEBUG(dbgs() << "  -> " << derefOrNil(Result) << '\n');

  return toVPlanSCEV(Result);
}

std::optional<VPConstStepLinear>
VPlanScalarEvolutionHIR::asConstStepLinear(VPlanSCEV *Expr) const {
  // FIXME: This implementation of asConstStepLinear simply delegates the
  //        request to asConstStepInduction. That is, no variables that are
  //        linear but non-inductive can be detected by this routine yet.
  std::optional<VPConstStepInduction> Ind = asConstStepInduction(Expr);
  return llvm::transformOptional(Ind, [](const auto &I) {
    return VPConstStepLinear{I.InvariantBase, I.Step};
  });
}

std::optional<VPConstStepInduction>
VPlanScalarEvolutionHIR::asConstStepInduction(VPlanSCEV *OpaqueExpr) const {
  VPlanAddRecHIR *Expr = toVPlanAddRecHIR(OpaqueExpr);

  if (!Expr)
    return std::nullopt;

  return VPConstStepInduction{
      toVPlanSCEV(makeVPlanAddRecHIR(Expr->Base, 0, Expr->Ref)), Expr->Stride};
}

// Check if access address of a load/store instruction can be represented as
// VPlanAddRecHIR expression which roughly corresponds to SCEVAddRecExpr.
VPlanAddRecHIR *
VPlanScalarEvolutionHIR::computeAddressSCEVImpl(const VPLoadStoreInst &LSI) {
  // The code below assumes that MainLoop IV starts with 0 and is incremented
  // by 1. This assumption is valid only for normalized loops.
  if (!MainLoop->isNormalized())
    return nullptr;

  const VPValue &Ptr = *LSI.getPointerOperand();
  if (!Ptr.isUnderlyingIRValid())
    return nullptr;

  if (maybePointerToPrivateMemory(Ptr))
    return nullptr;

  const RegDDRef *Ref = LSI.getHIRMemoryRef();
  if (!Ref || !Ref->isMemRef())
    return nullptr;

  // FIXME: Add full support for multidimensional arrays. Currently we handle
  // single dimensional arrays and multi-dimensional arrays where dimensions
  // other than lowest are invariant i.e. accesses of the form
  // a[inv1][inv2][SomeIndex] where inv1/inv2 are invariant at the nesting level
  // of the loop we are vectorizing.
  unsigned MainLoopLevel = MainLoop->getNestingLevel();
  auto supportedAccess = [MainLoopLevel](const RegDDRef *Ref) {
    if (!Ref->getDimensionLower(1)->isInvariantAtLevel(MainLoopLevel) ||
        !Ref->getDimensionStride(1)->isInvariantAtLevel(MainLoopLevel))
      return false;

    for (unsigned I = Ref->getNumDimensions(); I > 1; --I)
      if (!Ref->getDimensionIndex(I)->isInvariantAtLevel(MainLoopLevel) ||
          !Ref->getDimensionLower(I)->isInvariantAtLevel(MainLoopLevel) ||
          !Ref->getDimensionStride(I)->isInvariantAtLevel(MainLoopLevel))
        return false;

    return true;
  };

  if (!supportedAccess(Ref))
    return nullptr;

  // TODO: Support for struct offsets.
  if (Ref->hasTrailingStructOffsets())
    return nullptr;

  // We use the lowest dimension for creating adjusted base.
  const CanonExpr *AddressCE = Ref->getDimensionIndex(1);
  if (AddressCE->getDenominator() != 1)
    return nullptr;

  // Currently the way we create AddRecHIR will not work if the index
  // expression has an implicit conversion. This is a TODO item for now.
  if (AddressCE->getSrcType() != AddressCE->getDestType())
    return nullptr;

  // Only expressions with loop invariant blobs and without nested IVs can be
  // represented as AddRecExpr.
  if (!AddressCE->isLinearAtLevel(MainLoopLevel))
    return nullptr;

  for (unsigned Lvl = MainLoopLevel + 1; Lvl <= loopopt::MaxLoopNestLevel; ++Lvl)
    if (AddressCE->hasIV(Lvl))
      return nullptr;

  int64_t ElementSize = Ref->getDimensionConstStride(1);
  if (!ElementSize)
    return nullptr;

  // TODO: Rework the following adjusted base computation to work properly for
  // multidimensional arrays. The main issue is that upper dimensions (dim > 1)
  // are not accounted for when computing the adjusted base. This leads to
  // incorrect results when computing the difference of two address SCEVs whose
  // the upper dimensions are not identical. For now this is worked around in
  // getMinusExprImpl. See that method and CMPLRLLVM-48366 for details.

  // HIR Base is not the same as the base for AddRecExpr. HIR base is not the
  // address of the first access. For instance, access like ptr[i + x] would
  // have Base = ptr, while the address of the first access is (ptr + x).
  // AdjustedBase is the address of the first access and the base for
  // AddRecExpr.
  //
  // Notice that in order to build AdjustedBase we need to multiply by
  // ElementSize all offsets from Base, including constant offset, outer loop
  // IVs and blobs.
  const CanonExpr *Base = Ref->getBaseCE();
  CanonExprUtils &CEU = AddressCE->getCanonExprUtils();
  CanonExpr *AdjustedBase =
      CEU.createCanonExpr(Base->getDestType(), MainLoopLevel,
                          ElementSize * AddressCE->getConstant());

  if (!CEU.add(AdjustedBase, Base))
    return nullptr;

  // IVs of outer loops should be part of AdjustedBase. That is, when
  // vectorizing Loop(Level = 2), IV for Loop(Level = 1) should be treated as if
  // it were a blob. For instance, AdjustedBase for ptr[i1 + i2] should be:
  // (ptr + ElementSize * i1).
  for (unsigned Lvl = 1; Lvl < MainLoopLevel; ++Lvl) {
    unsigned IV_Index;
    int64_t IV_Coeff;
    AddressCE->getIVCoeff(Lvl, &IV_Index, &IV_Coeff);
    AdjustedBase->addIV(Lvl, IV_Index, ElementSize * IV_Coeff);
  }

  // Copy blobs from index expression
  for (auto &B : make_range(AddressCE->blob_begin(), AddressCE->blob_end())) {
    AdjustedBase->addBlob(B.Index, ElementSize * B.Coeff);
  }

  int64_t MainLoopIVCoeff;
  AddressCE->getIVCoeff(MainLoopLevel, nullptr, &MainLoopIVCoeff);

  return makeVPlanAddRecHIR(AdjustedBase, ElementSize * MainLoopIVCoeff, Ref);
}

VPlanAddRecHIR *VPlanScalarEvolutionHIR::getMinusExprImpl(VPlanAddRecHIR *LHS,
                                                          VPlanAddRecHIR *RHS) {
  if (!LHS || !RHS)
    return nullptr;

  // Until we can properly support multidimensional array references, limit
  // comparisons to SCEVs whose upper dimensions (> 1) are equal.
  //
  // To verify this, first check that the refs both have the same shape and
  // (trailing struct) offsets. We intentionally ignore the base, as it is
  // possible to compute a meaningful difference between two refs with
  // different bases (if both are suitably aligned).
  if (!DDRefUtils::haveEqualBaseAndShapeAndOffsets(
          LHS->Ref, RHS->Ref,
          /*RelaxedMode=*/false, /*NumIgnorableDims=*/0, /*IgnoreBaseCE=*/true))
    return nullptr;

  // Next, check that the upper (> 1) dimension indices are equal. If not, the
  // computed difference will be incorrect.
  for (unsigned Dim = LHS->Ref->getNumDimensions(); Dim > 1; --Dim)
    if (!CanonExprUtils::areEqual(LHS->Ref->getDimensionIndex(Dim),
                                  RHS->Ref->getDimensionIndex(Dim)))
      return nullptr;

  // If we've ensured it is safe to do so, compute the base difference of the
  // two SCEVs by subtracting their base refs.
  CanonExpr *Diff = CanonExprUtils::cloneAndSubtract(LHS->Base, RHS->Base);
  if (!Diff)
    return nullptr;

  // If computing the difference of two pointer SCEVs, the result should not be
  // a pointer, but an integer of the appropriate width. Setting the type here
  // is both semantically correct, and necessary to avoid an assert failure
  // when trying to print certain diff CEs (i.e. trying to print a constant
  // non-null CE of pointer type.)
  if (Diff->getDestType()->isPointerTy())
    Diff->setSrcAndDestType(
        Diff->getCanonExprUtils().getDataLayout().getIndexType(
            Diff->getDestType()));

  return makeVPlanAddRecHIR(Diff, LHS->Stride - RHS->Stride);
}

VPlanAddRecHIR *VPlanScalarEvolutionHIR::makeVPlanAddRecHIR(
    loopopt::CanonExpr *Base, int64_t Stride,
    const loopopt::RegDDRef *Ref) const {
  auto SmartPtr = std::make_unique<VPlanAddRecHIR>(Base, Stride, Ref);
  VPlanAddRecHIR *Ptr = SmartPtr.get();
  Storage.emplace_back(std::move(SmartPtr));
  return Ptr;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
raw_ostream &vpo::operator<<(raw_ostream &OS, const VPlanAddRecHIR &E) {
  OS << "{(";
  E.Base->print(OS);
  OS << "),+," << E.Stride << '}';
  return OS;
}
#endif
