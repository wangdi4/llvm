//===- IntelVPLoopAnalysisHIR.cpp -----------------------------------------===//
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
/// This file contains implementation of VPLoopAnalysisHIR class.
//
//===----------------------------------------------------------------------===//

#include "IntelVPLoopAnalysisHIR.h"
#include "IntelVPLoopRegionHIR.h"

namespace llvm {
namespace vpo {

void VPLoopAnalysisHIR::computeTripCountImpl(const VPLoopRegion *Lp) {
  uint64_t TripCount;
  assert(isa<VPLoopRegionHIR>(Lp) && "Not a VPLoopRegionHIR");
  const HLLoop *HLoop = dyn_cast<const VPLoopRegionHIR>(Lp)->getHLLoop();
  LoopTripCounts[Lp] = TripCountInfo();

  if (HLoop->isConstTripLoop(&TripCount)) {
    setKnownTripCountFor(Lp, TripCount);
    return;
  }
  // TODO: Need to get MinTC either from #pragma loop count min()
  // or from some analysis. Right now HIR doesn't know anything about
  // MinTC.
  // Set MinTC to DefaultTripCount by now.
  if ((TripCount = HLoop->getMaxTripCountEstimate())) {
    setMaxTripCountFor(Lp, TripCount);
    setEstimatedTripCountFor(Lp, TripCount);
    setMinTripCountFor(Lp, 0);
  } else {
    setMaxTripCountFor(Lp, DefaultTripCount);
    setEstimatedTripCountFor(Lp, DefaultTripCount);
    setMinTripCountFor(Lp, DefaultTripCount);
  }
}

} // namespace vpo
} // namespace llvm
