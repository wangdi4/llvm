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
#include "IntelVPlanValue.h"
#include "IntelVPlan.h"

using namespace llvm;
using namespace llvm::vpo;

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
