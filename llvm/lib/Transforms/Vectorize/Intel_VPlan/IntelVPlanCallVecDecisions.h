//===-- IntelVPlanCallVecDecisions.h ---------------------------*- C++ -*-===//
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
/// This file defines VPlanCallVecDecisions class which implements a VPlan
/// analysis that visits each VPCallInstruction, identifies ideal
/// vectorization scenario for a given VF and records decision by updating
/// vectorization properties of the VPCall.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCALLVECDECISIONS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCALLVECDECISIONS_H

#include "IntelVPlan.h"

namespace llvm {

class TargetTransformInfo;
class TargetLibraryInfo;

namespace vpo {

class VPlanCallVecDecisions {

public:
  VPlanCallVecDecisions(VPlan &Plan) : Plan(Plan) {}

  /// Public interface for analysis that visits all VPCallInstructions in
  /// current VPlan CFG and updates them with appropriate decision on how to
  /// vectorize the Call.
  // TODO: This should be removed once SVA is in place to directly invoke Call
  // specific interface below.
  void run(unsigned VF, const TargetLibraryInfo *TLI,
           const TargetTransformInfo *TTI);

  /// Determine the characteristic type of the vector function as
  /// specified according to the vector function ABI.
  static Type *calcCharacteristicType(VPCallInstruction *VPCallInst,
                                      VectorVariant &Variant);

private:
  /// Determine call vectorization properties for a given \p VPCall. This
  /// decision is taken based on given \p VF and various external components
  /// like vector variants, target library functions, call site and function
  /// attributes.
  void analyzeCall(VPCallInstruction *VPCall, unsigned VF,
                   const TargetLibraryInfo *TLI,
                   const TargetTransformInfo *TTI);

  /// Describes the caller side argument to callee side parameter matching
  /// score for simd functions. Scoring is based on the performance implications
  /// of the matching. E.g., uniform pointer arg -> vector parameter would
  /// result in gather/scatter, uniform -> vector would result in broadcast,
  /// etc.
  // Scalar2VectorScore represents either a uniform or linear match with
  // vector.
  static constexpr unsigned Scalar2VectorScore = 2;
  static constexpr unsigned Uniform2UniformScore = 3;
  static constexpr unsigned UniformPtr2UniformPtrScore = 4;
  static constexpr unsigned Vector2VectorScore = 4;
  static constexpr unsigned Linear2LinearScore = 4;

  /// Match arguments of VPCall to vector variant parameters. Return score
  /// based on how well the parameters matched.
  bool matchAndScoreVariantParameters(const VPCallInstruction *VPCall,
                                      VectorVariant &Variant,
                                      int &Score,
                                      int &MaxArg);

  /// Find the best simd function variant.
  std::unique_ptr<VectorVariant>
  matchVectorVariant(const VPCallInstruction *VPCall, bool Masked,
                     unsigned VF, const TargetTransformInfo *TTI);

  VPlan &Plan;
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCALLVECDECISIONS_H
