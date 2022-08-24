//===- IntelVPlanScalarEvolutionHIR.cpp -------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanScalarEvolutionHIR.h"

#include "Intel_VPlan/IntelVPlan.h"
#include "Intel_VPlan/IntelVPlanValue.h"

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

Optional<VPConstStepLinear>
VPlanScalarEvolutionHIR::asConstStepLinear(VPlanSCEV *Expr) const {
  // FIXME: This implementation of asConstStepLinear simply delegates the
  //        request to asConstStepInduction. That is, no variables that are
  //        linear but non-inductive can be detected by this routine yet.
  Optional<VPConstStepInduction> Ind = asConstStepInduction(Expr);
  return Ind.map([](const auto &I) {
    return VPConstStepLinear{I.InvariantBase, I.Step};
  });
}

Optional<VPConstStepInduction>
VPlanScalarEvolutionHIR::asConstStepInduction(VPlanSCEV *OpaqueExpr) const {
  VPlanAddRecHIR *Expr = toVPlanAddRecHIR(OpaqueExpr);

  if (!Expr)
    return None;

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

  CanonExpr *Diff = CanonExprUtils::cloneAndSubtract(LHS->Base, RHS->Base);
  if (!Diff)
    return nullptr;

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

raw_ostream &vpo::operator<<(raw_ostream &OS, const VPlanAddRecHIR &E) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  OS << "{(";
  E.Base->print(OS);
  OS << "),+," << E.Stride << '}';
#endif
  return OS;
}
