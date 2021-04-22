//===- IntelVPlanValueTrackingHIR.cpp ---------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanValueTrackingHIR.h"

#include <llvm/IR/DataLayout.h>

#define DEBUG_TYPE "vplan-value-tracking"

using namespace llvm;
using namespace vpo;

KnownBits VPlanValueTrackingHIR::getKnownBits(VPlanSCEV *Expr,
                                              const VPInstruction *CtxI) {
  return 8 * DL->getMaxPointerSize();
}
