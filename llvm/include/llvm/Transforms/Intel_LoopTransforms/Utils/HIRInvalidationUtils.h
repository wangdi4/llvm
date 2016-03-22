//===- HIRInvalidationUtils.h - Invalidation utilities for HIR ---*- C++-*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRUtils.h"

#include <type_traits>

namespace llvm {

namespace loopopt {

class HLLoop;
class HLRegion;

/// \brief Defines analysis invalidation utilities for HIR.
class HIRInvalidationUtils : public HIRUtils {
private:
  // The InPack type determines if T is presented in the parameter Pack.
  //
  // Template declaration, useful things are in the specializations.
  template <typename T, typename... Pack>
  struct InPack : std::true_type {};

  // InPack<A, B, C, D, E, ...> is handled by this specialization.
  //
  // InPack<T = A, First = B, Rest = C, D, E, ...>
  // The conditional deduces this to true_type if A is B, meaning that
  // T is in the Pack.
  //
  // Otherwise it is deduced as InPack<T = A, First = C, Rest = D, E, ...>.
  // Recursively it becomes InPack<T = A>, which is actually a false_type.
  template <typename T, typename First, typename... Rest>
  struct InPack<T, First, Rest...>
    : std::conditional<std::is_same<T, First>::value,
      std::true_type, InPack<T, Rest...>>::type {};

  // This specialization renders a false_type. This type means that there
  // is no T in the Pack.
  template <typename T>
  struct InPack<T> : std::false_type {};

  // AnalysisGetter is a helper class that returns (void *)nullptr if AnalysisTy
  // is void and returns AnalysisTy instance if available.
  template <typename AnalysisTy>
  struct AnalysisGetter {
    static AnalysisTy *get() {
      return getHIRFramework()->getAnalysisIfAvailable<AnalysisTy>();
    }
  };

  template <typename... Analysis>
  struct AnalysisSet {

    static_assert(sizeof...(Analysis) == HIRAnalysisPass::HIRPassCountVal,
        "One or more HIR Analysis pass is missing!");

    template <typename F, typename... ArgsTy>
    static void invoke(F&& Func, ArgsTy... Args) {
      void *AnalysisArray[] = { AnalysisGetter<Analysis>::get()... };
      for (void *AnalysisPtr : AnalysisArray) {
        if (AnalysisPtr) {
          (static_cast<HIRAnalysisPass *>(AnalysisPtr)->*Func)
              (std::forward<ArgsTy>(Args)...);
        }
      }
    }

    // This type is an AnalysisSet, but without excluded analysis.
    template <typename... Excluding>
    struct Except : AnalysisSet<typename std::conditional<
      InPack<Analysis, Excluding...>::value, void, Analysis>::type...> {};
  };

  // There should be all available analysis
  typedef AnalysisSet<HIRDDAnalysis, HIRLocalityAnalysis> ForEachAnalysis;

  /// \brief Do not allow instantiation.
  HIRInvalidationUtils() = delete;

public:
  /// \brief Invalidates all the available HIR analysis dependent on the loop
  /// body except the preserved ones explicitly specified by
  /// template arguments.
  template <typename... Except>
  static void invalidateBody(const HLLoop *Loop) {
    ForEachAnalysis::Except<Except...>::invoke(
        &HIRAnalysisPass::markLoopBodyModified, Loop);
  }

  /// \brief Invalidates all the available HIR analysis dependent non-loop
  /// region nodes except the preserved ones explicitly specified by
  /// template arguments.
  template <typename... Except>
  static void invalidateNonLoopRegion(const HLRegion *Region) {
    ForEachAnalysis::Except<Except...>::invoke(
          &HIRAnalysisPass::markNonLoopRegionModified, Region);
  }

  /// \brief Invalidates all the available HIR analysis dependent on the loop
  /// bounds except the preserved ones explicitly specified by
  /// template arguments.
  template <typename... Except>
  static void invalidateBounds(const HLLoop *Loop) {
    ForEachAnalysis::Except<Except...>::invoke(
          &HIRAnalysisPass::markLoopBoundsModified, Loop);
  }
};

template <>
struct HIRInvalidationUtils::AnalysisGetter<void> {
  static void *get() {
    return nullptr;
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
