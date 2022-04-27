//===---------- IntelVPlanPragmaOmpSimdIf.h -*- C++ -*---------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This function pass reduces pragma omp simd if clause into simdlen(1) under
/// negative condition as per OpenMP 5.0 standard
/// (https://www.openmp.org/spec-html/5.0/openmpsu42.html) If simdlen clause is
/// present its value is overriden into 1 under if clause negative condition.
///
/// IR is changed as follows:
///
/// 1	#pragma omp simd ... if(C)
/// 2	for (A) B
///
///   ->
///
/// 3	if (C) {
/// 4	  #pragma omp simd ...
/// 5	  for (A) B
/// 6	} else {
/// 7	  #pragma omp simd ... simdlen(1)
/// 8	  for (A) B
/// 9	}
///
/// In case pragma omp simd does not have private clause we can do optimization
/// by skipping line 7 above.
///
/// This pass clones pragma omp simd region using
/// VPOUtils::singleRegionMultiVersioning function.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANPRAGMAOMPSIMDIF_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANPRAGMAOMPSIMDIF_H

namespace llvm {

// Class that contains actual implementation for both pass managers
class VPlanPragmaOmpSimdIfImpl {
public:
  VPlanPragmaOmpSimdIfImpl() {}
  bool runImpl(Function &F, DominatorTree *DT, LoopInfo *LI);
};

// Implementation for new pass manager
class VPlanPragmaOmpSimdIfPass
    : public PassInfoMixin<VPlanPragmaOmpSimdIfPass> {
  VPlanPragmaOmpSimdIfImpl Impl;

public:
  VPlanPragmaOmpSimdIfPass() {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

// Implementation for legacy pass manager
class VPlanPragmaOmpSimdIf : public FunctionPass {
  VPlanPragmaOmpSimdIfImpl Impl;

protected:
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

public:
  static char ID;
  VPlanPragmaOmpSimdIf();
};

Pass *createVPlanPragmaOmpSimdIfPass();

} // end namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANPRAGMAOMPSIMDIF_H
