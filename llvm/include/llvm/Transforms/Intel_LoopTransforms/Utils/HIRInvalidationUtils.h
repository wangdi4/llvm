//===- HIRInvalidationUtils.h - Invalidation utilities for HIR ---*- C++-*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the analysis invalidation utilities for HIR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRINVALDATIONUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRINVALDATIONUTILS_H

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLUtils.h"

namespace llvm {

namespace loopopt {

class HLLoop;
class HLRegion;

/// \brief Defines analysis invalidation utilities for HIR.
class HIRInvalidationUtils : public HLUtils {
private:
  /// \brief Do not allow instantiation.
  HIRInvalidationUtils() = delete;

  /// \brief Returns Analysis pointer if it is available and not preserved.
  template <typename AnalysisTy, typename IsPreservedFuncTy>
  static AnalysisTy *getAnalysisIfNotPreserved(IsPreservedFuncTy isPreserved,
                                               unsigned &PassCount);

  /// \brief Invokes specified analysis's (AnalysisTy) loop body invalidation
  /// interface.
  template <typename AnalysisTy, typename IsPreservedFuncTy>
  static void invalidateAnalysisForLoopBody(const HLLoop *Lp,
                                            IsPreservedFuncTy isPreserved,
                                            unsigned &PassCount);

  /// \brief Invokes specified analysis's (AnalysisTy) loop bounds invalidation
  /// interface.
  template <typename AnalysisTy, typename IsPreservedFuncTy>
  static void invalidateAnalysisForLoopBounds(const HLLoop *Lp,
                                              IsPreservedFuncTy isPreserved,
                                              unsigned &PassCount);

  /// \brief Invokes specified analysis's (AnalysisTy) non-loop region
  /// invalidation interface.
  template <typename AnalysisTy, typename IsPreservedFuncTy>
  static void invalidateAnalysisForNonLoopRegion(const HLRegion *Reg,
                                                 IsPreservedFuncTy isPreserved,
                                                 unsigned &PassCount);

public:
  /// \brief Invalidates all the available HIR analysis dependent on the loop
  /// body except the preserved ones explicitly specified by
  /// isPreserved(HIRAnalysisPass *).
  template <typename IsPreservedFuncTy>
  static void invalidateLoopBodyAnalysis(const HLLoop *Lp,
                                         IsPreservedFuncTy &&isPreserved);

  /// \brief Invalidates all the available HIR analysis dependent on the loop
  /// bounds except the preserved ones explicitly specified by
  /// isPreserved(HIRAnalysisPass *).
  template <typename IsPreservedFuncTy>
  static void invalidateLoopBoundsAnalysis(const HLLoop *Lp,
                                           IsPreservedFuncTy &&isPreserved);

  /// \brief Invalidates all the available HIR analysis dependent non-loop
  /// region nodes except the preserved ones explicitly specified by
  /// isPreserved(HIRAnalysisPass *).
  template <typename IsPreservedFuncTy>
  static void invalidateNonLoopRegionAnalysis(const HLRegion *Reg,
                                              IsPreservedFuncTy &&isPreserved);
};

// Reference - http://stackoverflow.com/questions/7828675/callback-vs-lambda
template <typename AnalysisTy, typename IsPreservedFuncTy>
AnalysisTy *
HIRInvalidationUtils::getAnalysisIfNotPreserved(IsPreservedFuncTy isPreserved,
                                                unsigned &PassCount) {
  PassCount++;

  AnalysisTy *Pass = getHIRParser()->getAnalysisIfAvailable<AnalysisTy>();

  if (Pass && !std::forward<IsPreservedFuncTy>(isPreserved)(Pass)) {
    return Pass;
  }

  return nullptr;
}

template <typename AnalysisTy, typename IsPreservedFuncTy>
void HIRInvalidationUtils::invalidateAnalysisForLoopBody(
    const HLLoop *Lp, IsPreservedFuncTy isPreserved, unsigned &PassCount) {
  AnalysisTy *Pass =
      getAnalysisIfNotPreserved<AnalysisTy>(isPreserved, PassCount);

  if (Pass) {
    Pass->markLoopBodyModified(Lp);
  }
}

template <typename AnalysisTy, typename IsPreservedFuncTy>
void HIRInvalidationUtils::invalidateAnalysisForLoopBounds(
    const HLLoop *Lp, IsPreservedFuncTy isPreserved, unsigned &PassCount) {
  AnalysisTy *Pass =
      getAnalysisIfNotPreserved<AnalysisTy>(isPreserved, PassCount);

  if (Pass) {
    Pass->markLoopBoundsModified(Lp);
  }
}

template <typename AnalysisTy, typename IsPreservedFuncTy>
void HIRInvalidationUtils::invalidateAnalysisForNonLoopRegion(
    const HLRegion *Reg, IsPreservedFuncTy isPreserved, unsigned &PassCount) {
  AnalysisTy *Pass =
      getAnalysisIfNotPreserved<AnalysisTy>(isPreserved, PassCount);

  if (Pass) {
    Pass->markNonLoopRegionModified(Reg);
  }
}

template <typename IsPreservedFuncTy>
void HIRInvalidationUtils::invalidateLoopBodyAnalysis(
    const HLLoop *Lp, IsPreservedFuncTy &&isPreserved) {
  unsigned PassCount = 0;

  invalidateAnalysisForLoopBody<DDAnalysis>(Lp, isPreserved, PassCount);
  invalidateAnalysisForLoopBody<HIRLocalityAnalysis>(Lp, isPreserved,
                                                     PassCount);

  assert((PassCount == HIRAnalysisPass::PassCountVal) &&
         "One or more HIR Analysis pass is missing!");
}

template <typename IsPreservedFuncTy>
void HIRInvalidationUtils::invalidateLoopBoundsAnalysis(
    const HLLoop *Lp, IsPreservedFuncTy &&isPreserved) {
  unsigned PassCount = 0;

  invalidateAnalysisForLoopBounds<DDAnalysis>(Lp, isPreserved, PassCount);
  invalidateAnalysisForLoopBounds<HIRLocalityAnalysis>(Lp, isPreserved,
                                                       PassCount);

  assert((PassCount == HIRAnalysisPass::PassCountVal) &&
         "One or more HIR Analysis pass is missing!");
}

template <typename IsPreservedFuncTy>
void HIRInvalidationUtils::invalidateNonLoopRegionAnalysis(
    const HLRegion *Reg, IsPreservedFuncTy &&isPreserved) {
  unsigned PassCount = 0;

  invalidateAnalysisForNonLoopRegion<DDAnalysis>(Reg, isPreserved, PassCount);
  invalidateAnalysisForNonLoopRegion<HIRLocalityAnalysis>(Reg, isPreserved,
                                                          PassCount);

  assert((PassCount == HIRAnalysisPass::PassCountVal) &&
         "One or more HIR Analysis pass is missing!");
}

} // End namespace loopopt

} // End namespace llvm

#endif
