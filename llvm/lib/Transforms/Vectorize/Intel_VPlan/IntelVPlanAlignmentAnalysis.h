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
#include <llvm/Support/InstructionCost.h>
#include <llvm/Support/KnownBits.h>

namespace llvm {
namespace vpo {

class VPlanVector;
class VPInstruction;
class VPlanCostModelInterface;
class VPlanValueTracking;
class VPLoadStoreInst;

/// Supported peeling kinds.
enum VPlanPeelingKind { VPPK_StaticPeeling, VPPK_DynamicPeeling };

/// A single peeling variant.
/// This is an empty base class.
class VPlanPeelingVariant {
public:
  VPlanPeelingVariant(VPlanPeelingKind Kind) : Kind(Kind) {}
  virtual ~VPlanPeelingVariant() = 0;

  VPlanPeelingKind getKind() const { return Kind; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { print(errs()); }
  virtual void print(raw_ostream &OS) const = 0;
#endif

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

  // VPlanStaticPeeling{0}
  static VPlanStaticPeeling NoPeelLoop;

  static bool classof(const VPlanPeelingVariant *Peeling) {
    return Peeling->getKind() == VPPK_StaticPeeling;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const final {
    OS << "VPlanStaticPeeling: peelCount=" << PeelCount << '\n';
  }
#endif

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
  VPlanDynamicPeeling(VPLoadStoreInst *Memref,
                      VPConstStepInduction AccessAddress,
                      Align TargetAlignment);

  VPLoadStoreInst *memref() { return Memref; }
  VPlanSCEV *invariantBase() { return InvariantBase; }
  Align requiredAlignment() { return RequiredAlignment; }
  Align targetAlignment() { return TargetAlignment; }
  int multiplier() { return Multiplier; }

  static bool classof(const VPlanPeelingVariant *Peeling) {
    return Peeling->getKind() == VPPK_DynamicPeeling;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const override;
#endif

private:
  /// Memory reference (Load or Store instruction) that is the primary target
  /// for peeling.
  VPLoadStoreInst *Memref;

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
  virtual VPInstructionCost getCost(
    VPLoadStoreInst *Mrf, int VF, Align Alignment) = 0;
};

/// A simple (but reasonable) implementation of VPlanPeelingCostModel. It
/// assumes that profit from aligning any single store is 50% higher than
/// aligning any load. The profits don't depend on actual memory access types.
class VPlanPeelingCostModelSimple final : public VPlanPeelingCostModel {
public:
  VPlanPeelingCostModelSimple(const DataLayout &DL) : DL(&DL) {}

  VPInstructionCost getCost(
    VPLoadStoreInst *Mrf, int VF, Align Alignment) override;

private:
  const DataLayout *DL;
};

/// An implementation of VPlanPeelingCostModel based on general vectorizer cost
/// model. It is the most precise cost model for peeling analysis.
class VPlanPeelingCostModelGeneral final : public VPlanPeelingCostModel {
public:
  VPlanPeelingCostModelGeneral(const VPlanCostModelInterface *CM) : CM(CM) {
    assert(CM && "CostModel pointer should not be NULL.");
  }

  VPInstructionCost getCost(
    VPLoadStoreInst *Mrf, int VF, Align Alignment) override;

private:
  const VPlanCostModelInterface *CM;
};

/// Memref that is a candidate for peeling. VPlanPeelingCandidate object cannot
/// be created for non-unit stride accesses or for accesses known to be
/// misaligned (asserts in the constructor).
class VPlanPeelingCandidate final {
public:
  VPlanPeelingCandidate(VPLoadStoreInst *Memref,
                        VPConstStepInduction AccessAddress,
                        KnownBits InvariantBaseKnownBits);

  VPLoadStoreInst *memref() const { return Memref; }
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
  VPLoadStoreInst *Memref;

  /// Access address.
  VPConstStepInduction AccessAddress;

  /// KnownBits for AccessAddress.InvariantBase.
  KnownBits InvariantBaseKnownBits;
};

/// Peeling Analysis finds the best peeling variant according to the given cost
/// model.
class VPlanPeelingAnalysis final {
public:
  VPlanPeelingAnalysis(VPlanScalarEvolution &VPSE, VPlanValueTracking &VPVT,
                       const DataLayout &DL)
      : VPSE(&VPSE), VPVT(&VPVT), DL(&DL) {}
  VPlanPeelingAnalysis(const VPlanPeelingAnalysis &) = delete;
  VPlanPeelingAnalysis &operator=(const VPlanPeelingAnalysis &) = delete;
  VPlanPeelingAnalysis(VPlanPeelingAnalysis &&) = default;

  /// Find and analyze all the memory references in \p VPlan.
  /// This method must be called before selecting a peeling variant, and it
  /// must be called only once.
  void collectMemrefs(VPlanVector &Plan);

  /// Find the most profitable peeling variant for a particular \p VF.
  std::unique_ptr<VPlanPeelingVariant>
  selectBestPeelingVariant(int VF, VPlanPeelingCostModel &CM,
                           bool EnableDynamic);

  /// Returns best static peeling variant and its profit. The algorithm for
  /// selecting best peeling variant always succeeds. In the worst case
  /// {StaticPeeling(0), 0} is returned.
  std::pair<VPlanStaticPeeling, VPInstructionCost>
  selectBestStaticPeelingVariant(int VF, VPlanPeelingCostModel &CM);

  /// Returns best dynamic peeling variant and its profit. None is returned when
  /// there's no analyzable memrefs in the loop.
  Optional<std::pair<VPlanDynamicPeeling, VPInstructionCost>>
  selectBestDynamicPeelingVariant(int VF, VPlanPeelingCostModel &CM);

private:
  void collectCandidateMemrefs(VPlanVector &Plan);
  void computeCongruentMemrefs();
  LLVM_DUMP_METHOD void dump();

private:
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
  DenseMap<VPLoadStoreInst *, std::vector<std::pair<VPLoadStoreInst *, Align>>>
      CongruentMemrefs;
};

/// Alignment Analysis computes alignment of a memory access, either with or
/// without peeling.
///
/// With an appropriate peeling, alignment of unit-strided accesses in a
/// vectorized loop may become significantly better than in scalar code, since
/// it is only the alignment of the first or last lane (depending on whether
/// the stride is positive or negative) that matters.
///
/// On the contrary, alignment of gathers/scatters is the minimum alignment
/// across all vector lanes, so it cannot be improved with peeling.
///
/// This class provides interfaces to query both alignment of unit-stride
/// accesses with a known peeling and alignment in general case. The latter is
/// useful when either the access is not unit-strided or when the peeling is
/// unknown.
///
/// As an example, in order to estimate peeling profit, Cost Model may query
/// alignments of memory accesses in the vectorized loop twice: with the
/// selected peeling variant and without peeling (which is the static peeling
/// with PeelCount = 0). On the other hand, since in most cases the peel loop is
/// not guaranteed to be executed at run time, CodeGen must be on the safe side
/// and use conservative alignment that is valid with any peeling.
class VPlanAlignmentAnalysis {
public:
  VPlanAlignmentAnalysis(VPlanScalarEvolution &VPSE, VPlanValueTracking &VPVT,
                         int VF)
      : VPSE(&VPSE), VPVT(&VPVT), VF(VF) {}

  /// Compute alignment of a \p UnitStrideMemref in the vectorized loop with the
  /// given \p GuaranteedPeeling (nullptr should be passed if the peeling is not
  /// guaranteed). The returned alignment is computed using the memory address
  /// either in the first vector lane (if the stride is positive) or in the last
  /// lane (if the stride is negative).
  Align getAlignmentUnitStride(const VPLoadStoreInst &UnitStrideMemref,
                               VPlanPeelingVariant *GuaranteedPeeling) const;

  /// Compute conservative alignment of \p Memref. The returned alignment is
  /// valid for any vector lane and any peeling variant. This method should be
  /// used when either \p Memref is not unit-strided or when exact run-time
  /// peeling is unknown.
  Align getAlignment(VPLoadStoreInst &Memref);

private:
  Align getAlignmentUnitStrideImpl(const VPLoadStoreInst &Memref,
                                   VPlanStaticPeeling &SP) const;

  Align getAlignmentUnitStrideImpl(const VPLoadStoreInst &Memref,
                                   VPlanDynamicPeeling &DP) const;

private:
  VPlanScalarEvolution *VPSE;
  VPlanValueTracking *VPVT;
  int VF;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_ALIGNMENT_ANALYSIS_H
