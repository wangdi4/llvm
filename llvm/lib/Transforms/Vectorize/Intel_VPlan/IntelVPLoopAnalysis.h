//===- VPLoopAnalysis.h - --------------------------------------------------===/
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// \file
/// This file provides VPLoop-based analysis. Right now VPLoopAnalysisBase can
/// only be used to compute min, known, estimated or max trip counts for a
/// VPLoopRegion.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include <map>

using namespace llvm;

namespace llvm {

class ScalarEvolution;

namespace vpo {

class VPLoopRegion;
class IntelVPlanUtils;

class VPLoopAnalysisBase {
protected:
#if INTEL_CUSTOMIZATION
  using TripCountTy = uint64_t;
#else
  // SCEV's getConstantTripCount() is limited to trip counts that fit into
  // uint32_t
  using TripCountTy = unsigned;
#endif // INTEL_CUSTOMIZATION
  typedef struct TripCountInfo {
    TripCountTy MinTripCount;
    TripCountTy MaxTripCount;
    TripCountTy TripCount;
    bool IsEstimated;

    explicit TripCountInfo(void)
        : MinTripCount(0), MaxTripCount(0), TripCount(0), IsEstimated(true) {}
    explicit TripCountInfo(const TripCountTy MinTC, const TripCountTy MaxTC,
                           const TripCountTy TC, const bool IsEstimated)
        : MinTripCount(MinTC), MaxTripCount(MaxTC), TripCount(TC),
          IsEstimated(IsEstimated) {}
  } TripCount;

  std::map<const VPLoopRegion *, TripCountInfo> LoopTripCounts;

  // Trip count for loops with unknown loop count
  const TripCountTy DefaultTripCount;

  TripCountInfo computeAndReturnTripCountInfo(const VPLoopRegion *Lp) {
    if (LoopTripCounts.count(Lp))
      return LoopTripCounts.find(Lp)->second;
    computeTripCount(Lp);
    assert(LoopTripCounts.count(Lp) &&
           "Cannot compute trip count for this loop");
    return LoopTripCounts.find(Lp)->second;
  }

  virtual void computeTripCountImpl(const VPLoopRegion *Lp) = 0;

public:
  explicit VPLoopAnalysisBase(const TripCountTy DefaultTripCount)
      : DefaultTripCount(DefaultTripCount) {}

  void computeTripCount(const VPLoopRegion *Lp) { computeTripCountImpl(Lp); }

  /// Return true if trip count for the loop is known in compile time.
  bool isKnownTripCountFor(const VPLoopRegion *Lp) {
    return !computeAndReturnTripCountInfo(Lp).IsEstimated;
  }

  /// Return minimal trip count for the loop, which was either computed by some
  /// analysis or was provided by user.
  TripCountTy getMinTripCountFor(const VPLoopRegion *Lp) {
    return computeAndReturnTripCountInfo(Lp).MinTripCount;
  }

  /// Return minimal trip count for the loop, which was either computed by some
  /// analysis or was provided by user.
  TripCountTy getMaxTripCountFor(const VPLoopRegion *Lp) {
    return computeAndReturnTripCountInfo(Lp).MaxTripCount;
  }

  /// Return trip count for the loop. It's caller's responsibility to check
  /// whether this trip count was estimated or was known in compile time.
  TripCountTy getTripCountFor(const VPLoopRegion *Lp) {
    return computeAndReturnTripCountInfo(Lp).TripCount;
  }

  /// Set known trip count for a given VPLoop. This function also sets MinTC and
  /// MaxTC to same value.
  void setKnownTripCountFor(const VPLoopRegion *Lp, const TripCountTy TripCount) {
    LoopTripCounts[Lp] = TripCountInfo(TripCount, TripCount, TripCount, false);
  }

  /// Set estimated trip count for a given VPLoop. This function doesn't touch
  /// MinTC or MaxTC.
  void setEstimatedTripCountFor(const VPLoopRegion *Lp, const TripCountTy TripCount) {
    LoopTripCounts[Lp].TripCount = TripCount;
    LoopTripCounts[Lp].IsEstimated = true;
  }

  /// Set MinTC for a given VPLoop. This function doesn't touch TC or MaxTC.
  void setMinTripCountFor(const VPLoopRegion *Lp, const TripCountTy TripCount) {
    LoopTripCounts[Lp].MinTripCount = TripCount;
    LoopTripCounts[Lp].IsEstimated = true;
    if (TripCount > getMaxTripCountFor(Lp))
      setMaxTripCountFor(Lp, TripCount);
  }

  /// Set MaxTC for a given VPLoop. This function doesn't touch TC or MinTC.
  void setMaxTripCountFor(const VPLoopRegion *Lp, const TripCountTy TripCount) {
    LoopTripCounts[Lp].MaxTripCount = TripCount;
    LoopTripCounts[Lp].IsEstimated = true;
    if (TripCount < getMinTripCountFor(Lp))
      setMinTripCountFor(Lp, TripCount);
  }
};

class VPLoopAnalysis : public VPLoopAnalysisBase {
private:
  ScalarEvolution *SE;
  // TODO: templatizing of getSmallConstantMaxTripCount() is required to support
  // VPLoop.
  void computeTripCountImpl(const VPLoopRegion *Lp) final {
    setMaxTripCountFor(Lp, DefaultTripCount);
    setEstimatedTripCountFor(Lp, DefaultTripCount);
    setMinTripCountFor(Lp, DefaultTripCount);
  }

public:
  explicit VPLoopAnalysis(ScalarEvolution *SE, const TripCountTy DefaultTripCount)
      : VPLoopAnalysisBase(DefaultTripCount), SE(SE) {}
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLOOPANALYSIS_H

