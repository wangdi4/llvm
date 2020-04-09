//===- IntelVPlanAlignmentAnalysis.h ----------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_ALIGNMENT_ANALYSIS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_ALIGNMENT_ANALYSIS_H

#include "IntelVPlanScalarEvolution.h"

namespace llvm {
namespace vpo {

class VPInstruction;

/// Supported peeling kinds.
enum VPlanPeelingKind { VPPK_StaticPeeling, VPPK_DynamicPeeling };

/// A single peeling variant.
/// This is an empty base class.
class VPlanPeelingVariant {
public:
  VPlanPeelingVariant(VPlanPeelingKind Kind) : Kind(Kind) {}
  virtual ~VPlanPeelingVariant() = 0;

  VPlanPeelingKind getKind() const { return Kind; }

private:
  VPlanPeelingKind Kind;
};

/// Static peeling is fully described with a single number that is the number of
/// iterations to peel. Notice that "no peeling" is actually a "static peeling
/// with PeelCount = 0".
class VPlanStaticPeeling final : public VPlanPeelingVariant {
public:
  VPlanStaticPeeling(int PeelCount)
      : VPlanPeelingVariant(VPPK_StaticPeeling), PeelCount(PeelCount) {}

  int peelCount() { return PeelCount; }

  static bool classof(const VPlanPeelingVariant *Peeling) {
    return Peeling->getKind() == VPPK_StaticPeeling;
  }

private:
  int PeelCount;
};

/// Dynamic peeling requires run-time computations to determine peel count:
///
///     Quotient = InvariantBase / RequiredAlignment;
///     Divisor = TargetAlignment / RequiredAlignment;
///     PeelCount = (Quotient * Multiplier) % Divisor;
///
/// It is easy to see from the formula that the maximum possible peel count is
/// (Divisor - 1).
///
/// Note that peeling is not possible if InvariantBase is not aligned by at
/// least RequiredAlignment bytes.
struct VPlanDynamicPeeling final : public VPlanPeelingVariant {
public:
  VPlanDynamicPeeling(VPInstruction *Memref, VPConstStepInduction AccessAddress,
                      Align TargetAlignment);

  VPInstruction *memref() { return Memref; }
  VPlanSCEV *invariantBase() { return InvariantBase; }
  Align requiredAlignment() { return RequiredAlignment; }
  Align targetAlignment() { return TargetAlignment; }
  int multiplier() { return Multiplier; }

  static bool classof(const VPlanPeelingVariant *Peeling) {
    return Peeling->getKind() == VPPK_DynamicPeeling;
  }

private:
  /// Memory reference (Load or Store instruction) that is the primary target
  /// for peeling.
  VPInstruction *Memref;

  /// Symbolic invariant expression that can be computed before vector code. The
  /// run-time value of this expression is used in the formula above.
  VPlanSCEV *InvariantBase;

  /// Minimum required alignment for the InvariantBase. If the InvariantBase is
  /// not aligned by at least this many bytes, then peeling is not possible.
  Align RequiredAlignment;

  /// Alignment of the Memref after this peeling variant is applied.
  Align TargetAlignment;

  /// Magic number. See the formula above.
  int Multiplier;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_ALIGNMENT_ANALYSIS_H
