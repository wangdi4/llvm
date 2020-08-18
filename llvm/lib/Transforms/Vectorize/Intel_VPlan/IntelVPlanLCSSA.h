//===-- IntelVPlanLCSSA.h ---------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares the simplified utility to transform VPlan into LCSSA
/// form. The assumption is that it is run post loop exits canonicalization and
/// so doesn't need to employ the full blown SSAUpdater-based phi nodes
/// placement.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_LCSSA_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_LCSSA_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {
void formLCSSA(VPlan &Plan, bool SkipTopLoop = false);
} // namespace vpo
} // namespace llvm

#endif
