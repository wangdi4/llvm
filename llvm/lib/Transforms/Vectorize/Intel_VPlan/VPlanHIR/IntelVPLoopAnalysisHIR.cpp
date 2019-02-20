//===- IntelVPLoopAnalysisHIR.cpp -----------------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// \file
/// This file contains implementation of VPLoopAnalysisHIR class.
//
//===----------------------------------------------------------------------===//


#include "IntelVPLoopAnalysisHIR.h"
#include "IntelVPLoopRegionHIR.h"

#define DEBUG_TYPE "vploop-analysis"

namespace llvm {
namespace vpo {

void VPLoopAnalysisHIR::computeTripCountImpl(const VPLoopRegion *Lp) {
  uint64_t TripCount;
  unsigned MinTripCount;
  unsigned MaxTripCount;
  unsigned AvgTripCount;
  bool IsMaxTakenFromLoop= false;
  bool IsMinTakenFromPragma = false;
  bool IsAverageTakenFromPragma = false;

  assert(isa<VPLoopRegionHIR>(Lp) && "Not a VPLoopRegionHIR");
  const HLLoop *HLoop = cast<const VPLoopRegionHIR>(Lp)->getHLLoop();

  LoopTripCounts[Lp] = TripCountInfo();

  if (HLoop->isConstTripLoop(&TripCount)) {
    setKnownTripCountFor(Lp, TripCount);
    return;
  }

  if ((MaxTripCount = HLoop->getMaxTripCountEstimate())) {
    setMaxTripCountFor(Lp, MaxTripCount);
    IsMaxTakenFromLoop = true;
  }
  else
    setMaxTripCountFor(Lp, DefaultTripCount);

  if (HLoop->getPragmaBasedMinimumTripCount(MinTripCount)) {
    setMinTripCountFor(Lp, MinTripCount);
    IsMinTakenFromPragma = true;
  } else
    setMinTripCountFor(Lp, 0);

  if (HLoop->getPragmaBasedAverageTripCount(AvgTripCount)) {
    setEstimatedTripCountFor(Lp, AvgTripCount);
    IsAverageTakenFromPragma = true;
  } else if (IsMaxTakenFromLoop && IsMinTakenFromPragma)
    setEstimatedTripCountFor(Lp, (MaxTripCount + MinTripCount) >> 1);
  else if (IsMaxTakenFromLoop)
    setEstimatedTripCountFor(Lp, MaxTripCount);
  else if (IsMinTakenFromPragma)
    setEstimatedTripCountFor(Lp, MinTripCount);
  else
    setEstimatedTripCountFor(Lp, DefaultTripCount);

  (void) IsMaxTakenFromLoop;
  (void) IsMinTakenFromPragma;
  (void) IsAverageTakenFromPragma;

  LLVM_DEBUG(
      dbgs()
      << "Max trip count is " << getMaxTripCountFor(Lp)
      << (IsMaxTakenFromLoop
              ? " updated by loop opt upon retrieving loop count from pragma"
              : " assumed default trip count by vectorizer")
      << '\n');
  LLVM_DEBUG(dbgs() << "Average trip count is " << getTripCountFor(Lp)
                    << (IsAverageTakenFromPragma
                            ? " set by pragma loop count"
                            : " assumed default trip count by vectorizer")
                    << '\n');
  LLVM_DEBUG(dbgs() << "Min trip count is " << getMinTripCountFor(Lp)
                    << (IsMinTakenFromPragma
                            ? " set by pragma loop count"
                            : " assumed default trip count by vectorizer")
                    << '\n');
}

} // namespace vpo
} // namespace llvm
