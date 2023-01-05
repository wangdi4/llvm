//===- IntelVPMemRefTransform.h ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===---------------------------------------------------------------------===//
///
/// \file
/// This file defines the VPMemRefTranform class that is used for doing various
/// transforms on memory references.
/// TODO: Add support for Subscript instructions.
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_MEMREF_TRANSFORM_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_MEMREF_TRANSFORM_H

#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
namespace llvm {
class Type;
namespace vpo {

class VPMemRefTransform {
public:
  VPMemRefTransform(const VPMemRefTransform &) = delete;
  VPMemRefTransform &operator=(const VPMemRefTransform &) = delete;

  // Constructor.
  VPMemRefTransform(VPlanVector &Plan)
      : Plan(Plan), DA(*Plan.getVPlanDA()), VF(0) {}

  /// Do appropriate transforms on SOA GEPs.
  void transformSOAGEPs(unsigned InVF);

private:
  /// Return \true if the given GEP is not a SOA-unit-stride GEP.
  bool isSOANonUnitStridedGEP(VPInstruction *I);

  /// Return \true if the given GEP is a SOA-unit-stride GEP.
  bool isSOAUnitStridedGEP(VPInstruction *I);

  /// Update the shape of dependent PHIs.
  void updateDependentPHIs(VPInstruction *I);

  /// Method which clones the instruction if predicate \p Pred is false for its
  /// users. The cloned instruction is replaced in all users for which \p Pred
  /// is false.
  void cloneAndReplaceUses(VPInstruction *I,
                           std::function<bool(const VPUser *)> Pred);

  /// Transform SOA-unitstride GEPs to GEPs which return a vector of pointers to
  /// the base-addresses of each element.
  void transformSOAUnitStrideGEPs(VPGEPInstruction *I);

  /// Transform SOA-non-unitstride GEPs to GEPs which return a vector of pointers
  /// to the base-addresses of each element.
  void transformSOANonUnitStrideGEPs(VPGEPInstruction *I);

  VPlanVector &Plan;

  VPlanDivergenceAnalysis &DA;

  unsigned VF;

  VPBuilder Builder;
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_MEMREF_TRANSFORM_H
