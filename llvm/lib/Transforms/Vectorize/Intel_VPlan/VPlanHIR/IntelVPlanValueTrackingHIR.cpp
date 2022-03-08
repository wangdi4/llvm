//===- IntelVPlanValueTrackingHIR.cpp ---------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanValueTrackingHIR.h"

#include "IntelVPlanScalarEvolutionHIR.h"

#include <llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h>
#include <llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h>
#include <llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/DataLayout.h>

#define DEBUG_TYPE "vplan-value-tracking"

using namespace llvm;
using namespace vpo;
using namespace loopopt;

KnownBits VPlanValueTrackingHIR::getKnownBits(VPlanSCEV *Expr,
                                              const VPInstruction *CtxI) {
  VPlanAddRecHIR *AddRec = VPlanScalarEvolutionHIR::toVPlanAddRecHIR(Expr);
  // Original CtxI is ignored. We cannot compute KnownBits for a specific
  // location inside VPlan. The best we can do is to compute conservative
  // KnownBits that are valid across whole VPlan (actually, even across
  // whole HIR).
  KnownBits KB = getKnownBitsImpl(AddRec);
  LLVM_DEBUG(dbgs() << "getKnownBits(" << *AddRec << ")\n");
  LLVM_DEBUG(dbgs() << "  -> "; KB.print(dbgs()); dbgs() << '\n');
  return KB;
}

// Compute KnownBits for VPlanSCEV expression. Since we are using LLVM tools for
// this (ScalarEvolution and ValueTracking), we can compute KnownBits only for
// expressions that are defined in LLVM code (i.e. outside of HIR region).
KnownBits VPlanValueTrackingHIR::getKnownBitsImpl(VPlanAddRecHIR *AddRec) {
  auto BitWidth = DL->getMaxIndexSizeInBits();
  CanonExpr *Base = AddRec->Base;
  BlobUtils &BU = Base->getBlobUtils();

  // FIXME: Actually, non-zero stride can be taken into account as well.
  //        However, there's no immediate need for it.
  if (AddRec->Stride != 0)
    return KnownBits{BitWidth};

  assert(Base->getDenominator() == 1 &&
         "VPlanAddRecHIR::Base with non-unit denominator");

  // Find location at which we compute known bits (CtxI). This has to be outside
  // of the HIR region to make sure that it is not modified by LoopOpt.
  HLRegion *Region = MainLoop->getParentRegion();
  Instruction *CtxI = &*Region->getEntryBBlock()->begin();

  // KnownBits can be computed only for values that are invariant inside HIR.
  // Particularly, KnownBits should be computed only for blobs that are defined
  // outside of HLRegion, so that we are sure they are not modified by HIR
  // optimizations. So, check all the blobs here and bail out if any of them is
  // defined inside the region.
  if (any_of(make_range(Base->blob_begin(), Base->blob_end()),
             [Region, &BU](const auto &Blob) {
               return !HLNodeUtils::isRegionInvariant(Region, BU, Blob.Index);
             }))
    return KnownBits{BitWidth};

  KnownBits KB =
      KnownBits::makeConstant(APInt{BitWidth, (uint64_t)Base->getConstant()});

  for (auto &B : make_range(Base->blob_begin(), Base->blob_end())) {
    const SCEV *ScevExpr = BU.getBlob(B.Index);
    KnownBits BlobKB = computeKnownBitsForScev(ScevExpr, CtxI);
    BlobKB = KnownBits::mul(
        KnownBits::makeConstant(APInt{BitWidth, (uint64_t)B.Coeff}), BlobKB);
    KB = KnownBits::computeForAddSub(/*Add:*/ true, /*NSW:*/ false, BlobKB, KB);
  }

  return KB;
}

// Compute KnownBits for Expr at location CtxI. Only a limited subset of SCEV
// kinds is supported. The subset can be extended later if necessary.
KnownBits
VPlanValueTrackingHIR::computeKnownBitsForScev(const SCEV *Expr,
                                               Instruction *CtxI) const {
  auto BitWidth = DL->getMaxIndexSizeInBits();

  if (auto *E = dyn_cast<SCEVUnknown>(Expr))
    return computeKnownBits(E->getValue(), *DL, 0, AC, CtxI, DT)
        .anyextOrTrunc(BitWidth);

  if (auto *E = dyn_cast<SCEVConstant>(Expr))
    return KnownBits::makeConstant(E->getAPInt()).anyextOrTrunc(BitWidth);

  if (auto *E = dyn_cast<SCEVAddExpr>(Expr)) {
    KnownBits KB = computeKnownBitsForScev(E->getOperand(0), CtxI);
    for (int I = 1, IE = E->getNumOperands(); I < IE; ++I)
      KB = KnownBits::computeForAddSub(
          /*Add:*/ true, /*NSW:*/ false, KB,
          computeKnownBitsForScev(E->getOperand(I), CtxI));
    return KB;
  }

  if (auto *E = dyn_cast<SCEVMulExpr>(Expr)) {
    KnownBits KB = computeKnownBitsForScev(E->getOperand(0), CtxI);
    for (int I = 1, IE = E->getNumOperands(); I < IE; ++I)
      KB = KnownBits::mul(KB, computeKnownBitsForScev(E->getOperand(I), CtxI));
    return KB;
  }

  return KnownBits{BitWidth};
}
