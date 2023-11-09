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

#include "ScalarEvolution.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/TargetTransformInfo.h"
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
enum VPlanPeelingKind {
  VPPK_NoPeeling,
  // "llvm.loop.intel.vector.aligned" metadata is specified
  VPPK_NoPeelingAligned,
  // "llvm.loop.intel.vector.unaligned" metadata is specified
  VPPK_NoPeelingUnaligned,
  VPPK_StaticPeeling,
  VPPK_DynamicPeeling
};

/// A single peeling variant.
/// This is an empty base class.
class VPlanPeelingVariant {
public:
  VPlanPeelingVariant(VPlanPeelingKind Kind) : Kind(Kind) {}
  virtual ~VPlanPeelingVariant() = 0;

  VPlanPeelingKind getKind() const { return Kind; }

  /// \Returns the maximum number of iterations peeled (i.e. the maximum number
  /// of iterations of the peel loop). For static peeling, this is the exact
  /// number of iterations, whereas for dynamic peeling this is just an upper
  /// bound (see override below for details.)
  virtual int maxPeelCount() const = 0;

  /// \Returns whether the specified peeling is guaranteed to execute before the
  /// main loop. For static peel, this is always true, whereas in the dynamic
  /// case, the peel may be skipped, and the main loop proceeds unaligned.
  virtual bool isGuaranteedToExecuteBeforeMainLoop() const = 0;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { print(errs()); }
  virtual void print(raw_ostream &OS) const = 0;
#endif

private:
  VPlanPeelingKind Kind;
};

/// No peeling class variant where Pragma can be one of VPPK_NoPeeling,
/// VPPK_NoPeelingAligned or VPPK_NoPeelingUnaligned.
template <VPlanPeelingKind Pragma>
class VPlanNoPeelingT final : public VPlanPeelingVariant {
public:
  VPlanNoPeelingT() : VPlanPeelingVariant(Pragma) {}

  int maxPeelCount() const override { return 0; }
  bool isGuaranteedToExecuteBeforeMainLoop() const override { return true; }

  static VPlanNoPeelingT<Pragma> LoopObject;

  static bool classof(const VPlanPeelingVariant *Peeling) {
    return Peeling->getKind() == Pragma;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const final { OS << "VPlanNoPeeling\n"; }
#endif
};

using VPlanNoPeeling = VPlanNoPeelingT<VPPK_NoPeeling>;
using VPlanNoPeelingAligned = VPlanNoPeelingT<VPPK_NoPeelingAligned>;
using VPlanNoPeelingUnaligned = VPlanNoPeelingT<VPPK_NoPeelingUnaligned>;

template <> VPlanNoPeeling VPlanNoPeeling::LoopObject;
template <> VPlanNoPeelingAligned VPlanNoPeelingAligned::LoopObject;
template <> VPlanNoPeelingUnaligned VPlanNoPeelingUnaligned::LoopObject;

/// Static peeling is fully described with a single number that is the number of
/// iterations to peel.
class VPlanStaticPeeling final : public VPlanPeelingVariant {
public:
  VPlanStaticPeeling(int PeelCount)
      : VPlanPeelingVariant(VPPK_StaticPeeling), PeelCount(PeelCount) {
    assert(PeelCount && "unexpected zero peel count");
  }

  int peelCount() const { return PeelCount; }
  int maxPeelCount() const override { return peelCount(); }

  bool isGuaranteedToExecuteBeforeMainLoop() const override { return true; }

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
  const VPLoadStoreInst *memref() const { return Memref; }
  VPlanSCEV *invariantBase() { return InvariantBase; }
  Align requiredAlignment() const { return RequiredAlignment; }
  Align targetAlignment() const { return TargetAlignment; }
  int multiplier() const { return Multiplier; }

  int maxPeelCount() const override {
    return TargetAlignment.value() / RequiredAlignment.value() - 1;
  }

  bool isGuaranteedToExecuteBeforeMainLoop() const override {
    // TODO: check conditions to see if we will *not* skip the main loop.
    return false;
  }

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

  /// Returns best static peeling value and its profit. The algorithm for
  /// selecting best peeling value always succeeds. In the worst case
  /// {0, 0} is returned.
  std::pair<int, VPInstructionCost>
  selectBestStaticPeelCount(int VF, VPlanPeelingCostModel &CM);

  /// Returns best dynamic peeling variant and its profit. None is returned when
  /// there's no analyzable memrefs in the loop.
  std::optional<std::pair<VPlanDynamicPeeling, VPInstructionCost>>
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
                         const TargetTransformInfo &TTI, int VF)
      : VPSE(&VPSE), VPVT(&VPVT), TTI(&TTI), VF(VF) {}

  /// Compute alignment of a \p UnitStrideMemref in the vectorized loop with the
  /// given \p GuaranteedPeeling (nullptr should be passed if the peeling is not
  /// guaranteed). The returned alignment is computed using the memory address
  /// either in the first vector lane (if the stride is positive) or in the last
  /// lane (if the stride is negative).
  Align
  getAlignmentUnitStride(const VPLoadStoreInst &UnitStrideMemref,
                         const VPlanPeelingVariant *GuaranteedPeeling) const;

  /// Compute conservative alignment of \p Memref. The returned alignment is
  /// valid for any vector lane and any peeling variant. This method should be
  /// used when either \p Memref is not unit-strided or when exact run-time
  /// peeling is unknown.
  Align getAlignment(VPLoadStoreInst &Memref);

  /// Returns whether the given \p Memref is unit-stride aligned given the
  /// provided \p Peeling variant. If the peeled alignment is greater than or
  /// equal to the target alignment (required alignment * VF), this method
  /// returns true.
  ///
  /// NOTE: If the peeled alignment is greater than the required alignment,
  /// but less than the target alignment (possible with congruent memrefs) this
  /// method still returns false.
  bool isAlignedUnitStride(const VPLoadStoreInst &Memref,
                           const VPlanPeelingVariant *Peeling) const;

  /// Try to compute an alignment for pointer \p Val, with an optional
  /// instruction context \val CtxI. If a known alignment can be extracted using
  /// value tracking, it is returned. Otherwise, no alignment is returned.
  /// Asserts that Val is a pointer.
  MaybeAlign tryGetKnownAlignment(const VPValue *Val,
                                  const VPInstruction *CtxI) const;

  /// Given a \p Plan, a \p VF and a \p GuaranteedPeeling variant, propagate
  /// alignment from the specified peeling to affected load/stores.
  ///
  /// NOTE: This method takes the guaranteed peeling variant as a separate
  /// parameter instead of calling VPlan::getGuaranteedPeeling(), as this
  /// method can be called on remainder VPlans, which do not have peeling set;
  /// in such a case, the peeling variant should come from the peeling selected
  /// on the main VPlan.
  static void propagateAlignment(VPlanVector *Plan,
                                 const TargetTransformInfo *TTI, unsigned VF,
                                 const VPlanPeelingVariant *GuaranteedPeeling);

private:
  Align getAlignmentUnitStrideImpl(const VPLoadStoreInst &Memref,
                                   const VPlanPeelingVariant &P) const;

  Align getAlignmentUnitStrideImpl(const VPLoadStoreInst &Memref,
                                   const VPlanDynamicPeeling &DP) const;

private:
  VPlanScalarEvolution *VPSE;
  VPlanValueTracking *VPVT;
  const TargetTransformInfo *TTI;
  int VF;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_ALIGNMENT_ANALYSIS_H
