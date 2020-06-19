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

#include <llvm/IR/DataLayout.h>
#include <llvm/Support/KnownBits.h>

namespace llvm {
namespace vpo {

class VPlan;
class VPInstruction;
class VPlanValueTracking;

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

/// Cost model for estimating cost of a memory references with different
/// alignments.
class VPlanPeelingCostModel {
public:
  VPlanPeelingCostModel() {}
  VPlanPeelingCostModel(const VPlanPeelingCostModel &) = delete;
  VPlanPeelingCostModel &operator=(const VPlanPeelingCostModel &) = delete;
  virtual ~VPlanPeelingCostModel() {}

  /// Compute cost of unit stride memory access \p Mrf with the given
  /// \p Alignment and \p VF.
  virtual int getCost(VPInstruction *Mrf, int VF, Align Alignment) = 0;
};

/// A simple dummy implementation of VPlanPeelingCostModel interface. It uses a
/// reasonable but very simple heuristic. In future, it is expected to be
/// replaced with a more precise TTI-based cost model.
class VPlanPeelingCostModelSimple final : public VPlanPeelingCostModel {
public:
  VPlanPeelingCostModelSimple(const DataLayout &DL) : DL(&DL) {}

  int getCost(VPInstruction *Mrf, int VF, Align Alignment) override;

private:
  const DataLayout *DL;
};

/// Memref that is a candidate for peeling. VPlanPeelingCandidate object cannot
/// be created for non-unit stride accesses or for accesses known to be
/// misaligned (asserts in the constructor).
class VPlanPeelingCandidate final {
public:
  VPlanPeelingCandidate(VPInstruction *Memref,
                        VPConstStepInduction AccessAddress,
                        KnownBits InvariantBaseKnownBits);

  VPInstruction *memref() const { return Memref; }
  const VPConstStepInduction &accessAddress() const { return AccessAddress; }
  const KnownBits &invariantBaseKnownBits() const {
    return InvariantBaseKnownBits;
  }

  static bool ordByStep(const VPlanPeelingCandidate &L,
                        const VPlanPeelingCandidate &R) {
    return L.accessAddress().Step < R.accessAddress().Step;
  }

private:
  /// Load or Store instruction.
  VPInstruction *Memref;

  /// Access address.
  VPConstStepInduction AccessAddress;

  /// KnownBits for AccessAddress.InvariantBase.
  KnownBits InvariantBaseKnownBits;
};

/// Peeling Analysis finds the best peeling variant according to the given cost
/// model.
class VPlanPeelingAnalysis final {
public:
  VPlanPeelingAnalysis(VPlanPeelingCostModel &CM, VPlanScalarEvolution &VPSE,
                       VPlanValueTracking &VPVT, const DataLayout &DL)
      : CM(&CM), VPSE(&VPSE), VPVT(&VPVT), DL(&DL) {}
  VPlanPeelingAnalysis(const VPlanPeelingAnalysis &) = delete;
  VPlanPeelingAnalysis &operator=(const VPlanPeelingAnalysis &) = delete;
  VPlanPeelingAnalysis(VPlanPeelingAnalysis &&) = default;

  /// Find and analyze all the memory references in \p VPlan.
  /// This method must be called before selecting a peeling variant, and it
  /// must be called only once.
  void collectMemrefs(VPlan &Plan);

  /// Find the most profitable peeling variant for a particular \p VF.
  std::unique_ptr<VPlanPeelingVariant> selectBestPeelingVariant(int VF);

  /// Returns best static peeling variant and its profit. The algorithm for
  /// selecting best peeling variant always succeeds. In the worst case
  /// {StaticPeeling(0), 0} is returned.
  std::pair<VPlanStaticPeeling, int> selectBestStaticPeelingVariant(int VF);

  /// Returns best dynamic peeling variant and its profit. None is returned when
  /// there's no analyzable memrefs in the loop.
  Optional<std::pair<VPlanDynamicPeeling, int>>
  selectBestDynamicPeelingVariant(int VF);

private:
  void collectCandidateMemrefs(VPlan &Plan);
  void computeCongruentMemrefs();

private:
  VPlanPeelingCostModel *CM;
  VPlanScalarEvolution *VPSE;
  VPlanValueTracking *VPVT;
  const DataLayout *DL;

  /// List of all memrefs that are candidates for peeling sorted by step of
  /// access address.
  std::vector<VPlanPeelingCandidate> CandidateMemrefs;

  /// Table that describes relations between alignment of different memrefs. It
  /// maps every memref Mrf from CandidateMemrefs to a possibly empty list of
  /// pairs [(MrfX, AlignX)], where AlignX is the maximum alignment that can be
  /// propagated from Mrf to MrfX. Notice that for the sake of efficiency, the
  /// map doesn't contain non-interesting cases with AlignX ≤ RequiredAlignment.
  DenseMap<VPInstruction *, std::vector<std::pair<VPInstruction *, Align>>>
      CongruentMemrefs;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_ALIGNMENT_ANALYSIS_H
