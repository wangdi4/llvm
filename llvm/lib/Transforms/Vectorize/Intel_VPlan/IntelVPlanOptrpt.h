//===-- IntelVPlanOptrpt.h --------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Helper utilities for opt remark emission.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANOPTRPT_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANOPTRPT_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"

namespace llvm {
namespace vpo {

class VPlanOptReportBuilder;
class VPLoop;
class VPLoopInfo;

struct OptReportStatsTracker {
  struct RemarkRecord {
    OptRemarkID RemarkID;
    OptReportVerbosity::Level MessageVerbosity;
    std::string Arg;

    // High verbosity is assumed
    RemarkRecord(OptRemarkID ID, std::string Arg = "")
        : RemarkID(ID), MessageVerbosity(OptReportVerbosity::High), Arg(Arg){};
    RemarkRecord(OptRemarkID ID, OptReportVerbosity::Level Verbosity,
                 std::string Arg = "")
        : RemarkID(ID), MessageVerbosity(Verbosity), Arg(Arg){};
  };

  // Fields.
#define VPLAN_OPTRPT_HANDLE(ID, NAME) int NAME = 0;
#define VPLAN_OPTRPT_VEC_HANDLE(VEC) SmallVector<RemarkRecord, 32> VEC;
#define VPLAN_OPTRPT_ORIGIN_VEC_HANDLE(VEC) SmallVector<RemarkRecord, 32> VEC;
#include "IntelVPlanOptrpt.inc"

public:
  template <class LoopTy>
  void emitRemarks(VPlanOptReportBuilder &Builder, LoopTy *Lp) const;

  void emitRemarks(OptReportBuilder &Builder, VPLoop *Lp,
                   VPLoopInfo *VPLI) const;
};
} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANOPTRPT_H
