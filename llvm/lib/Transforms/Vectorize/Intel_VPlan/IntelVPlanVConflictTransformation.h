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
class VPTreeConflict;
class VPInstruction;
class VPlan;
class VPlanVector;
class VPValue;
class VPBuilder;
class VPlanDivergenceAnalysis;

// Checks if the Plan has a VPConflict isntruction.
bool processVConflictIdiom(VPlan &, Function &Fn);

// Checks the type of VConflict idiom and tries to replace with appropriate
// VPlan representation.
bool processVConflictIdiom(VPGeneralMemOptConflict *, Function &Fn);

// Returns the single instruction within the conflict region if supported
// criteria are met. Otherwise, it returns nullptr.
VPInstruction* isSupportedVConflictRegion(VPGeneralMemOptConflict *VPConflict);

// Returns the reduction update operand of VPGeneralMemOptConflict instruction
VPValue* getReductionUpdateOp(VPGeneralMemOptConflict *VPConflict,
                              VPInstruction *InsnInVConflictRegion);

// Check if given VPConflict instruction is a generic tree-conflict idiom. If
// yes, generate a new VPTreeConflict and RUAW of VPConflict.
VPTreeConflict *tryReplaceWithTreeConflict(VPGeneralMemOptConflict *);

// Generate the permute intrinsic and do any necessary bitcasting if there is
// not a direct VF/type to intrinsic mapping.
VPValue* createPermuteIntrinsic(StringRef Name, Type *Ty, VPValue *PermuteVals,
                                VPValue *Control, VPBuilder &VPBldr,
                                LLVMContext &C, unsigned VF,
                                VPlanDivergenceAnalysis *DA);

// Lower VPTreeConflict instructions using the double permute tree reduction
// algorithm for any vector VPlan. Returns true if any tree conflict
// instructions were lowered.
bool lowerTreeConflictsToDoublePermuteTreeReduction(VPlanVector *Plan,
                                                    unsigned VF, Function &Fn);
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVCONFLICTTRANSFORMATION_H
