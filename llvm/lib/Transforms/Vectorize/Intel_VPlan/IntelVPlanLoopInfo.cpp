//===-- IntelVPlanLoopInfo.cpp ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPLoopInfo analysis and VPLoop class. VPLoopInfo is a
/// specialization of LoopInfoBase for VPBlockBase. VPLoops is a specialization
/// of LoopBase that is used to hold loop metadata from VPLoopInfo. Further
/// information can be found in VectorizationPlanner.rst.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanLoopIterator.h"
#include "IntelVPlanValue.h"
#include "IntelVPlan.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vplan-loop-info"

static cl::opt<unsigned>
    DefaultTripCount("vplan-default-trip-count", cl::init(300), cl::Hidden,
                     cl::desc("Default estimate for the loop trip count, if "
                              "can't be determined through other ways"));

void TripCountInfo::calculateEstimatedTripCount() {
  if (TripCount)
    return; // Already calculated.

  // No known trip count or known average tripcount. Try to estimate to the best
  // of our abilities.

  bool KnownMax = MaxTripCount != TripCountInfo::UnknownMaxTripCount;
  bool KnownMin = MinTripCount != 0;

  if (KnownMax && KnownMin)
    TripCount = (static_cast<VPLoop::TripCountTy>(MinTripCount) +
                 static_cast<VPLoop::TripCountTy>(MaxTripCount)) /
                2;
  else if (KnownMax)
    TripCount = MaxTripCount;
  else if (KnownMin)
    TripCount = MinTripCount;
  else
    TripCount = DefaultTripCount;
}

bool VPLoop::isLiveIn(const VPValue* VPVal) const {
  if (isa<VPExternalDef>(VPVal))
    return true;
  if (auto *VPInst = dyn_cast<VPInstruction>(VPVal)) {
    const VPBlockBase* Block = VPInst->getParent();
    return !contains(Block);
  }
  return false;
}

bool VPLoop::isLiveOut(const VPValue* VPVal) const {
  for (const VPUser *U : VPVal->users()) {
    if (isa<VPExternalUse>(U))
      return true;
    if (auto *UseInst = dyn_cast<VPInstruction>(U)) {
      const VPBlockBase* Block = UseInst->getParent();
      if (!contains(Block))
        return true;
    }
  }
  return false;
}

bool VPLoop::contains(const VPBasicBlock *BB) const {
  return contains(static_cast<const VPBlockBase *>(BB));
}

bool VPLoop::contains(const VPInstruction *I) const {
  return contains(I->getParent());
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
void VPLoop::printRPOT(raw_ostream &OS, const VPLoopInfo *VPLI, unsigned Indent,
                       const VPlanDivergenceAnalysis *DA) const {
  ReversePostOrderTraversal<
      const VPLoop *, VPLoopBodyTraits,
      std::set<std::pair<const VPLoop *, const VPBlockBase *>>>
      RPOT(this);

  auto *Header = getHeader();

  for (std::pair<const VPLoop *, const VPBlockBase *> Pair : RPOT) {
    const VPBlockBase *BB = Pair.second;
    SmallString<32> NamePrefix;
    if (BB == Header)
      NamePrefix += "<header>";

    if (isLoopLatch(BB))
      NamePrefix += "<latch>";

    if (isLoopExiting(BB))
      NamePrefix += "<exiting>";

    unsigned BBIndent = Indent;
    if (VPLI)
      BBIndent +=
          (this->getLoopDepth() - VPLI->getLoopFor(BB)->getLoopDepth()) * 2;

    BB->print(OS, BBIndent, DA, NamePrefix);
  }
  OS << "\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL

void VPLoop::setTripCountInfo(TripCountInfo TCInfo) {
  auto *Latch = cast<VPBasicBlock>(getLoopLatch());
  Latch->setTripCountInfo(std::make_unique<TripCountInfo>(TCInfo));
}

TripCountInfo VPLoop::getTripCountInfo() {
  auto *Latch = cast<VPBasicBlock>(getLoopLatch());
  if (TripCountInfo *TCInfoPtr = Latch->getTripCountInfo())
    return *TCInfoPtr;

  LLVM_DEBUG(dbgs() << "No trip count information for VPLoop with header "
                    << getHeader()->getName()
                    << ", using default estimations.\n");
  TripCountInfo DefaultTC;
  DefaultTC.calculateEstimatedTripCount();
  return DefaultTC;
}
