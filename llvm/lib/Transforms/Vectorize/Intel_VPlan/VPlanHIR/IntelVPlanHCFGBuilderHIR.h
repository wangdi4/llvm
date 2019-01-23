//===-- IntelVPlanHCFGBuilderHIR.h ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanHCFGBuilderHIR class which extends
/// VPlanHCFGBuilderBase with support to build a hierarchical CFG from HIR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H

#include "../IntelVPlanHCFGBuilder.h"
#include "IntelVPlanVerifierHIR.h"

using namespace llvm::loopopt;

namespace llvm {

// Forward declarations
namespace loopopt {
class DDGraph;
class HLLoop;
class HIRSafeReductionAnalysis;
class RegDDRef;
} // namespace loopopt

namespace vpo {

class HIRVectorizationLegality {
  using RecurrenceKind = RecurrenceDescriptor::RecurrenceKind;
  using MMRecurrenceKind = RecurrenceDescriptor::MinMaxRecurrenceKind;

public:
  struct RedDescr {
    using RecurrenceKind = RecurrenceDescriptor::RecurrenceKind;
    using MMRecurrenceKind = RecurrenceDescriptor::MinMaxRecurrenceKind;

    RedDescr(RegDDRef *RegV, RecurrenceKind KindV, MMRecurrenceKind MMKindV,
             bool Signed)
        : DDRef(RegV), Kind(KindV), MMKind(MMKindV), IsSigned(Signed) {}
    RedDescr() = delete;

    RegDDRef *DDRef;
    RecurrenceKind Kind;
    MMRecurrenceKind MMKind;
    bool IsSigned;
  };
  typedef std::vector<RedDescr> ReductionListTy;
  struct PrivDescr {
    PrivDescr(RegDDRef *RegV, bool IsLastV, bool IsCondV)
        : DDRef(RegV), IsLast(IsLastV), IsCond(IsCondV) {}
    PrivDescr() = delete;

    RegDDRef *DDRef;
    bool IsLast;
    bool IsCond;
  };
  typedef std::vector<PrivDescr> PrivateListTy;
  typedef std::pair<RegDDRef *, RegDDRef *> LinearDescr;
  typedef std::vector<LinearDescr> LinearListTy;

  HIRVectorizationLegality() = delete;
  HIRVectorizationLegality(const HIRVectorizationLegality &) = delete;

  HIRVectorizationLegality(HIRSafeReductionAnalysis *SafeReds)
      : SRA(SafeReds) {}

  // Add explicit private.
  void addLoopPrivate(RegDDRef *PrivVal, bool IsLast = false,
                      bool IsConditional = false) {
    PrivateList.emplace_back(PrivVal, IsLast, IsConditional);
  }

  /// Register explicit reduction variables provided from outside.
  void addReductionMin(RegDDRef *V, bool IsSigned) {
    addReduction(V, RecurrenceKind::RK_IntegerMinMax, IsSigned,
                 IsSigned ? MMRecurrenceKind::MRK_SIntMin
                          : MMRecurrenceKind::MRK_UIntMin);
  }
  void addReductionMax(RegDDRef *V, bool IsSigned) {
    addReduction(V, RecurrenceKind::RK_IntegerMinMax, IsSigned,
                 IsSigned ? MMRecurrenceKind::MRK_SIntMax
                          : MMRecurrenceKind::MRK_UIntMax);
  }
  void addReductionAdd(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerAdd);
  }
  void addReductionMult(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerMult);
  }
  void addReductionAnd(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerAnd);
  }
  void addReductionXor(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerXor);
  }
  void addReductionOr(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerOr);
  }

  // Add linear value to Linears map
  void addLinear(RegDDRef *LinearVal, RegDDRef *Step) {
    LinearList.emplace_back(LinearVal, Step);
  }

  HIRSafeReductionAnalysis *getSRA() { return SRA; }
  PrivateListTy *getPrivates() { return &PrivateList; }
  LinearListTy *getLinears() { return &LinearList; }
  ReductionListTy *getReductions() { return &ReductionList; }

private:
  void addReduction(RegDDRef *V, RecurrenceKind Kind, bool IsSigned = false,
                    MMRecurrenceKind MMKind = MMRecurrenceKind::MRK_Invalid) {
    ReductionList.emplace_back(V, Kind, MMKind, IsSigned);
  }

  HIRSafeReductionAnalysis *SRA;
  PrivateListTy PrivateList;
  LinearListTy LinearList;
  ReductionListTy ReductionList;
};

class VPlanHCFGBuilderHIR : public VPlanHCFGBuilder {

private:
  /// The outermost loop to be vectorized.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  const DDGraph &DDG;

  HIRVectorizationLegality *HIRLegality;

  /// Loop header VPBasicBlock to HLLoop map. To be used when building loop
  /// regions.
  SmallDenseMap<VPBasicBlock *, HLLoop *, 4> Header2HLLoop;

  VPRegionBlock *buildPlainCFG(VPLoopEntityConverterList &CvtVec) override;
  void passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) override;

  void collectUniforms(VPRegionBlock *Region) override {
    // Do nothing for now
  }

public:
  VPlanHCFGBuilderHIR(const WRNVecLoopNode *WRL, HLLoop *Lp, VPlan *Plan,
                      HIRVectorizationLegality *Legality, const DDGraph &DDG);

  VPLoopRegion *createLoopRegion(VPLoop *VPLp) override;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H
