//===-- IntelVPlanOptrpt.h --------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Helper utilities for opt remark emision.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANOPTRPT_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANOPTRPT_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {
struct OptReportStatsTracker {
  // Fields.
#define VPLAN_OPTRPT_HANDLE(ID, NAME) int NAME = 0;
#include "IntelVPlanOptrpt.inc"

public:
  template <class LoopTy>
  void emitRemarks(VPlanOptReportBuilder &Builder, LoopTy *Lp) const {
#define VPLAN_OPTRPT_HANDLE_GROUP_BEGIN(ID)                                    \
  Builder.addRemark(Lp, OptReportVerbosity::High, ID);
#define VPLAN_OPTRPT_HANDLE(ID, NAME)                                          \
  Builder.addRemark(Lp, OptReportVerbosity::High, ID, Twine(NAME).str());
#define VPLAN_OPTRPT_HANDLE_GROUP_END(ID)                                      \
  Builder.addRemark(Lp, OptReportVerbosity::High, ID);

#include "IntelVPlanOptrpt.inc"
  }
};
} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANOPTRPT_H
