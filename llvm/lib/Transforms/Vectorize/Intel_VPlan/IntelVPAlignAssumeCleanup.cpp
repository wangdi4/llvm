//===- IntelVPAlignAssumeCleanup.cpp --------------------------------------===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===---------------------------------------------------------------------===//
///
/// \file
/// This file implements the VPAlignAssumeCleanup class.
//===---------------------------------------------------------------------===//
#include "IntelVPAlignAssumeCleanup.h"
#include "Intel_VPlan/IntelVPlan.h"

using namespace llvm;
using namespace llvm::vpo;

static LoopVPlanDumpControl
    AlignAssumeCleanupDumpsControl("align-assume-cleanup",
                                   "cleaning up alignment assumptions");

#define DEBUG_TYPE "AlignAssumeCleanup"

void VPAlignAssumeCleanup::transform() {
  // Collect all alignment assumptions with VecClone metadata.
  auto AssumeWithMD = [](const VPInstruction &Inst) {
    auto *Call = dyn_cast<VPCallInstruction>(&Inst);
    if (!Call)
      return false;

    auto *Assume = dyn_cast_or_null<AssumeInst>(Call->getUnderlyingCallInst());
    if (!Assume)
      return false;

    return Assume->hasMetadata("intel.vecclone.align.assume");
  };
  SmallVector<VPInstruction *, 4> AssumesToRemove(
      map_range(make_filter_range(vpinstructions(&Plan), AssumeWithMD),
                [](VPInstruction &V) { return &V; }));

  // Now remove all collected assumptions from the IR.
  // (TODO: can we check they've been propagated?)
  for (auto *Assume : AssumesToRemove) {
    LLVM_DEBUG(dbgs() << "Removing alignment assumption: " << *Assume << "\n");
    Assume->getParent()->eraseInstruction(Assume);
  }

  VPLAN_DUMP(AlignAssumeCleanupDumpsControl, Plan);
}
