//===-- IntelVPlanVLSTransform.h --------------------------------*- C++ -*-===//
//
//   Copyright (C) Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_VLS_TRANSFORM_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_VLS_TRANSFORM_H

namespace llvm {
class OVLSGroup;
namespace vpo {
class VPlan;
class VPlanVLSAnalysis;

/// Returns true if the \p Group can be processed by VPlan-level VLS
/// transformation and further VPlan CG.
bool isTransformableVLSGroup(OVLSGroup *Group);

/// Commit VLS transformation to the \p Plan using the analysis data in \p VLSA.
///
/// The transformation is performed using low-level
/// VLSLoad/VLSStore/VLSExtract/VLSInsert instructions and is mainly used to
/// simplify CGs. As such, it requries the VPlan to only represent a single
/// vector factor passed as \p VF. Once transformation is performed the Plan is
/// NOT suitable for any other vector factor anymore, even if it was before
/// applying it.
void applyVLSTransform(VPlan &Plan, VPlanVLSAnalysis &VLSA, unsigned VF);

} // namespace vpo
} // namespace llvm

#endif
