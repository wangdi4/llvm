//===- HIRInvalidationUtils.h - Invalidation utilities for HIR ---*- C++-*-===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
// Here are couple examples how to use the utility:
//  1) HIRInvalidationUtils::invalidateBody(Loop);
//     This will invalidate all available HIR analysis for that particular loop.
//
//  2) HIRInvalidationUtils::invalidateBody<DDAnalysis, HIRAnalysis>(Loop);
//     You can specify analysis in <...> that are preserved, that means that
//     they will not be invalidated.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRINVALDATIONUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRINVALDATIONUTILS_H

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopResource.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSparseArrayReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include <type_traits>

namespace llvm {

namespace loopopt {

class HLLoop;
class HLRegion;

/// Defines analysis invalidation utilities for HIR.
class HIRInvalidationUtils {
private:
  /// Do not allow instantiation.
  HIRInvalidationUtils() = delete;

public:
  /// Invalidates all the available HIR analysis dependent on the loop body
  /// except the preserved ones explicitly specified by template arguments.
  template <typename... Exclude>
  static void invalidateBody(const HLLoop *Loop) {
    HIRAnalysisProvider::Invoke<Exclude...>(
        Loop->getHLNodeUtils().getHIRFramework().getHIRAnalysisProvider())(
        &HIRAnalysisBase::markLoopBodyModified, Loop);
  }

  /// Invalidates all the available HIR analysis on any loop in the loopnest
  /// except the preserved ones explicitly specified by template arguments.
  template <typename... Exclude>
  static void invalidateLoopNestBody(const HLLoop *Loop) {
    SmallVector<const HLLoop *, 4> Loops;

    Loop->getHLNodeUtils().gatherAllLoops(Loop, Loops);

    for (auto *Lp : Loops) {
      HIRAnalysisProvider::Invoke<Exclude...>(
          Loop->getHLNodeUtils().getHIRFramework().getHIRAnalysisProvider())(
          &HIRAnalysisBase::markLoopBodyModified, Lp);
    }
  }

  /// Invalidates all the available HIR analysis dependent non-loop region nodes
  /// except the preserved ones explicitly specified by template arguments.
  template <typename... Exclude>
  static void invalidateNonLoopRegion(const HLRegion *Region) {
    HIRAnalysisProvider::Invoke<Exclude...>(
        Region->getHLNodeUtils().getHIRFramework().getHIRAnalysisProvider())(
        &HIRAnalysisBase::markNonLoopRegionModified, Region);
  }

  /// Invalidates all the available HIR analysis dependent on the parent node
  /// except the preserved ones explicitly specified by template arguments.
  template <typename... Exclude>
  static void invalidateParentLoopBodyOrRegion(const HLNode *Node) {
    if (auto Loop = Node->getParentLoop()) {
      invalidateBody<Exclude...>(Loop);
    } else if (auto Region = Node->getParentRegion()) {
      invalidateNonLoopRegion<Exclude...>(Region);
    }
  }

  /// Invalidates all the available HIR analysis dependent on the loop bounds
  /// except the preserved ones explicitly specified by template arguments.
  template <typename... Exclude>
  static void invalidateBounds(const HLLoop *Loop) {
    HIRAnalysisProvider::Invoke<Exclude...>(
        Loop->getHLNodeUtils().getHIRFramework().getHIRAnalysisProvider())(
        &HIRAnalysisBase::markLoopBoundsModified, Loop);
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
