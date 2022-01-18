//===---------- IntelVPlanPragmaOmpOrderedSimdExtract.h -*- C++ -*---------===//
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
/// This file is the first step to implement #pragma omp ordered simd for VPlan.
/// The programmer uses #pragma omp ordered simd inside a loop in order to
/// highlight the part of the loop that should not be vectorized. Instead, the
/// ordered region is serialized. In other words, the ordered region
/// should be replicated as many times as the vector factor.
///
/// VPlan has already support for function call serialization. This works as
/// follows: if there is a call inside a loop and there are not any vector
/// variants attached to this call, then VPlan will replicate the function call
/// as many times as the vector factor. Therefore, we can leverage this
/// mechanism to implement #pragma omp ordered simd.
///
/// This is done in four steps:
/// 1. Identify the ordered region and replace the ordered region with a call to
/// a new function: The ordered region is outlined with a WRN region
/// (WRNOrdered). Code Extractor removes the basic blocks of the WRNordered
/// region ans replaces them with a function call. The newly created function
/// call is marked with "AlwaysInline" attribute.
///
/// 2. Function call serialization: VPlan starts to vectorize the loop and once
/// it reaches the newly created function call, it replicates the function call
/// as many times as the vector factor.
///
/// 3. Function inlining: The inliner(AlwaysInliner) puts back to the code the
/// ordered region.
///
/// 4. VPO Directive clean-up removes the directives that are related to #pragma
/// omp ordered simd.
// ===--------------------------------------------------------------------=== //

#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANPRAGMAOMPORDEREDSIMDEXTRACT_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANPRAGMAOMPORDEREDSIMDEXTRACT_H
namespace llvm {

class ModulePass;
class WRegionInfo;

using DomT = function_ref<DominatorTree *(Function &F)>;
using WRenInfo = function_ref<vpo::WRegionInfo *(Function &F)>;

class VPlanPragmaOmpOrderedSimdExtractImpl {
public:
  VPlanPragmaOmpOrderedSimdExtractImpl() {}
  bool runImpl(Module &M, DomT DT, WRenInfo WRI,
               LoopOptLimiter Limiter = LoopOptLimiter::None);
};

class VPlanPragmaOmpOrderedSimdExtractPass
    : public PassInfoMixin<VPlanPragmaOmpOrderedSimdExtractPass> {
  VPlanPragmaOmpOrderedSimdExtractImpl Impl;

public:
  VPlanPragmaOmpOrderedSimdExtractPass() {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MA);
};

class VPlanPragmaOmpOrderedSimdExtract : public ModulePass {
  VPlanPragmaOmpOrderedSimdExtractImpl Impl;

protected:
  bool runOnModule(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

public:
  static char ID;
  VPlanPragmaOmpOrderedSimdExtract();
};

Pass *createVPlanPragmaOmpOrderedSimdExtractPass();

} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANPRAGMAOMPORDEREDSIMDEXTRACT_H
