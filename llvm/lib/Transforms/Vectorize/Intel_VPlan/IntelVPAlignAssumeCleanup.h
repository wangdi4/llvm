//===- IntelVPAlignAssumeCleanup.h -----------------------------*- C++ -*-===//
//
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
/// This pass removes alignment assumptions inserted by previous passes (e.g.
/// VecClone), which are assumed to have been propagated by this point in the
/// pipeline. This helps clean up the input IR, so that downstream optimizations
/// and infrastructure (e.g. CodeGen) do not have to handle and are not impeded
/// by redundant calls to '@llvm.assume'.
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_ALIGN_ASSUME_CLEANUP_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_ALIGN_ASSUME_CLEANUP_H

#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "llvm/Analysis/AssumptionCache.h"

namespace llvm {
namespace vpo {

class VPAlignAssumeCleanup {
public:
  VPAlignAssumeCleanup(const VPAlignAssumeCleanup &) = delete;
  VPAlignAssumeCleanup &operator=(const VPAlignAssumeCleanup &) = delete;

  VPAlignAssumeCleanup(VPlan &Plan) : Plan(Plan) {}

  /// \brief Clean up alignment assumptions that were inserted by VecClone.
  /// (i.e. those annotated with 'intel.vecclone.align.assume')
  void transform();

private:
  VPlan &Plan;
};

} // namespace vpo
} // namespace llvm

#endif
