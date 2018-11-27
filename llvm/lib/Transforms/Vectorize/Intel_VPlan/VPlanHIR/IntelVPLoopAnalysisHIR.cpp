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

namespace llvm {
namespace vpo {

void VPLoopAnalysisHIR::computeTripCountImpl(const VPLoopRegion *Lp) {
  uint64_t TripCount;
  unsigned MinTripCount = 0;
  unsigned MaxTripCount = 0;
  unsigned AvgTripCount = 0;

  assert(isa<VPLoopRegionHIR>(Lp) && "Not a VPLoopRegionHIR");
  const HLLoop *HLoop = cast<const VPLoopRegionHIR>(Lp)->getHLLoop();

  LoopTripCounts[Lp] = TripCountInfo();

  if (HLoop->isConstTripLoop(&TripCount)) {
    setKnownTripCountFor(Lp, TripCount);
    return;
  }

  MaxTripCount = HLoop->getMaxTripCountEstimate();
  HLoop->getPragmaBasedMinimumTripCount(MinTripCount);
  HLoop->getPragmaBasedAverageTripCount(AvgTripCount);
  setTripCountsFromPragma(Lp, MinTripCount, MaxTripCount, AvgTripCount);
}

} // namespace vpo
} // namespace llvm
