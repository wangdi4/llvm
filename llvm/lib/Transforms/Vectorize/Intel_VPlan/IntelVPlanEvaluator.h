//===-- IntelVPlanEvaluator.h ---------------------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
// This file implement VPlan's Evaluator. The Evaluator finds the best
// peel/remainder variant by calculating and comparing the cost of each variant.
// In case of peel, there are three possible variants:
// i. no peel variant,
// ii. scalar variant,
// iii. masked vector variant.
// In case of remainder, there are four possible variants:
// i. no remainder variant,
// ii. scalar variant,
// iii. vector variant with possible scalar remainder,
// iv. masked vector variant.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANEVALUATOR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANEVALUATOR_H

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

namespace llvm {

class DataLayout;
class TargetLibraryInfo;
class TargetTransformInfo;

namespace vpo {

extern bool EnableVectorizedPeel;
extern bool EnableNonMaskedVectorizedRemainder;
extern bool EnableMaskedVectorizedRemainder;

class VPlanVector;
class VPlanMasked;
class VPlanCostModel;
class VPlanPeelingVariant;
class VPlanVLSAnalysis;
class LoopVectorizationPlanner;

class VPlanEvaluator {
public:
  VPlanEvaluator(LoopVectorizationPlanner &P, unsigned ScalarCst,
                 const TargetLibraryInfo *TLI, const TargetTransformInfo *TTI,
                 const DataLayout *DL, VPlanVLSAnalysis *VLSA)
      : Planner(P), ScalarIterCost(ScalarCst), TLI(TLI), TTI(TTI), DL(DL),
        VLSA(VLSA) {}

protected:
  // Calculates the cost of VPlan, without calculating peel/remainder cost. If
  // there is not a VPlan, returns UINT_MAX.
  unsigned calculatePlanCost(unsigned VF, VPlanVector *Plan);

  LoopVectorizationPlanner &Planner;
  unsigned ScalarIterCost;
  const TargetLibraryInfo *TLI;
  const TargetTransformInfo *TTI;
  const DataLayout *DL;
  VPlanVLSAnalysis *VLSA;
};

/// Helper class to evaluate peeling variants.
/// There are three peel variants: no peel variant, scalar variant and masked
/// vector variant. The cost for both variants is calculated and the best
/// variant is returned. In the future, it will calculate the overhead of peel
/// count calculation and run-time checks.
class VPlanPeelEvaluator : public VPlanEvaluator {
public:
  VPlanPeelEvaluator(LoopVectorizationPlanner &P, unsigned ScalarCst,
                     const TargetLibraryInfo *TLI,
                     const TargetTransformInfo *TTI, const DataLayout *DL,
                     VPlanVLSAnalysis *VLSA, unsigned MainLoopVF,
                     VPlanPeelingVariant *PeelingVariant)
      : VPlanEvaluator(P, ScalarCst, TLI, TTI, DL, VLSA),
        MainLoopVF(MainLoopVF), PeelingVariant(PeelingVariant) {
    calculateBestVariant();
  }

  enum class PeelLoopKind { None, Scalar, MaskedVector };

  PeelLoopKind getPeelLoopKind() const { return PeelKind; }

  const char *getPeelLoopKindStr() const;

  void dump(raw_ostream &OS) const;

  void dump() const { dump(dbgs()); }

  unsigned getLoopCost() const { return LoopCost; }

  // Returns the trip count of the best peel kind.
  unsigned getTripCount() const { return PeelTC; }

  // Returns the trip count of the peeling variant.
  unsigned getScalarPeelTripCount(unsigned MainLoopVF) const;

  void disable() {
    PeelingVariant = nullptr;
    PeelKind = PeelLoopKind::None;
    LoopCost = 0;
  }

private:
  PeelLoopKind PeelKind = PeelLoopKind::None;
  unsigned LoopCost = 0;
  unsigned PeelTC = 0;
  unsigned MainLoopVF = 0;
  VPlanPeelingVariant *PeelingVariant = nullptr;

  PeelLoopKind calculateBestVariant();

  // TODO : Calculate cost of peel count calculation and all run-time checks.
  // std::vector<VPRTCheck *> RTChecks;
  /* unsigned calculatePeelChecksCost(unsigned MainLoopVF); */
};

/// Helper class to evaluate remainder variants.
/// There are four remainder variants: no remainder variant, scalar variant,
/// vector variant with possible scalar remainder and masked vector variant. The
/// cost for all variants is calculated and the best variant is returned. In the
/// future, it will calculate the overhead of run-time checks.
class VPlanRemainderEvaluator : public VPlanEvaluator {
public:
  VPlanRemainderEvaluator(LoopVectorizationPlanner &P, unsigned ScalarCst,
                          const TargetLibraryInfo *TLI,
                          const TargetTransformInfo *TTI, const DataLayout *DL,
                          VPlanVLSAnalysis *VLSA, unsigned OrigTC,
                          unsigned PeelTC, unsigned MainLoopVF,
                          unsigned MainLoopUF)
      : VPlanEvaluator(P, ScalarCst, TLI, TTI, DL, VLSA),
        RemainderTC((OrigTC - PeelTC) % (MainLoopVF * MainLoopUF)),
        PeelTC(PeelTC), MainLoopVF(MainLoopVF), MainLoopUF(MainLoopUF) {
    calculateBestVariant();
  }

  enum class RemainderLoopKind { None, Scalar, VectorScalar, MaskedVector };

  RemainderLoopKind getRemainderLoopKind() const { return RemainderKind; }

  const char *getRemainderLoopKindStr() const;

  void dump(raw_ostream &OS) const;

  void dump() const { dump(dbgs()); }

  unsigned getLoopCost() const { return LoopCost; }

  unsigned getRemainderVF() const { return RemainderVF; }

  // Returns the trip count of the best remainder kind. In case of VectorScalar
  // kind, we might have a new remainder loop. For this reason, we return a pair
  // of trip counts: the first value of the pair is the trip count of the
  // original remainder and the second value is the trip count of the new
  // remainder.
  std::pair<unsigned, unsigned> getTripCount() const {
    return std::make_pair(RemainderTC, NewRemainderTC);
  }

  void disable() {
    RemainderKind = RemainderLoopKind::None;
    LoopCost = 0;
  }

private:
  RemainderLoopKind RemainderKind = RemainderLoopKind::Scalar;
  unsigned LoopCost = 0;
  unsigned RemainderVF = 1;
  unsigned RemainderTC = 0;
  unsigned PeelTC = 0;
  unsigned UnMaskedVectorCost = 0;
  unsigned MainLoopVF = 0;
  unsigned MainLoopUF = 0;
  unsigned NewRemainderTC = 0;

  RemainderLoopKind calculateBestVariant();

  void calculateRemainderVFAndVectorCost();

  // TODO: Calculate cost of trip count checks.
  // std::vector<VPRTCheck *> RTChecks;
  // unsigned calculateRTChecksCost(unsigned MainLoopVF);
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANEVALUATOR_H
