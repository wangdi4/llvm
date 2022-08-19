//===- IntelVPTransformLibraryCalls.h ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===---------------------------------------------------------------------===//
///
/// \file
/// This pass replaces library calls whose scalar signature does not match
/// their vectorized signature (e.g. sincos) with a special transformed library
/// call instruction that records this mismatch. It also inserts any
/// post-processing instructions necessary to handle the signature mismatch
/// after vectorization occurs.
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_TRANSFORM_LIBRARY_CALLS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_TRANSFORM_LIBRARY_CALLS_H

#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"

namespace llvm {
namespace vpo {

class VPTransformLibraryCalls {
public:
  VPTransformLibraryCalls(const VPTransformLibraryCalls &) = delete;
  VPTransformLibraryCalls &operator=(const VPTransformLibraryCalls &) = delete;

  VPTransformLibraryCalls(VPlanVector &Plan, TargetLibraryInfo &TLI)
      : Plan(Plan), DA(*Plan.getVPlanDA()), TLI(TLI) {}

  /// \brief Perform all library transforms.
  void transform();

private:
  /// \brief Perform scalar 'sincos' -> vector 'sincos' transforms.
  void transformSincosCalls();

private:
  VPlan &Plan;
  VPlanDivergenceAnalysis &DA;
  VPBuilder Builder;
  TargetLibraryInfo &TLI;
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_TRANSFORM_LIBRARY_CALLS_H
