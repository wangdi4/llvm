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

namespace llvm {
class Type;
namespace vpo {

class VPlanVector;

class VPMemRefTransform {
public:
  VPMemRefTransform(const VPMemRefTransform &) = delete;
  VPMemRefTransform &operator=(const VPMemRefTransform &) = delete;

  // Constructor.
  VPMemRefTransform(VPlanVector &Plan) : Plan(Plan) {}

  /// Do appropriate transforms on SOA GEPs.
  void transformSOAGEPs(unsigned VF);

private:
  VPlanVector &Plan;
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VP_MEMREF_TRANSFORM_H
