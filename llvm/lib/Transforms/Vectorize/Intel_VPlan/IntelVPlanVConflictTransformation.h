//===-- IntelVPlanVConflictTransformation.h ---------------------*- C++ -*-===//
//
//   Copyright (C) Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVCONFLICTTRANSFORMATION_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVCONFLICTTRANSFORMATION_H

#include "llvm/IR/Function.h"

namespace llvm {
namespace vpo {

class VPGeneralMemOptConflict;
class VPInstruction;
class VPlan;

// Checks if the Plan has a VPConflict isntruction.
bool processVConflictIdiom(VPlan &, Function &Fn, unsigned VF);

// Checks the type of VConflict idiom and calls the right lowering method.
bool processVConflictIdiom(VPGeneralMemOptConflict *, Function &Fn,
                           unsigned VF);
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVCONFLICTTRANSFORMATION_H
