//===-- ScalarEvolution.cpp -------------------------------------*- C++ -*-===//
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2020 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//===----------------------------------------------------------------------===//
///
/// \file ScalarEvolution.cpp
/// VPlan vectorizer's SCEV-like analysis.
///
//===----------------------------------------------------------------------===//

#include "ScalarEvolution.h"
#include "IntelVPlan.h"

#include <llvm/Analysis/ScalarEvolutionExpressions.h>

#include <optional>

#define DEBUG_TYPE "vplan-scalar-evolution"

using namespace llvm;
using namespace llvm::vpo;

bool VPlanScalarEvolution::maybePointerToPrivateMemory(const VPValue &V) {
  if (isa<VPExternalDef>(V) || isa<VPConstant>(V))
    return false;

  const auto &VPI = cast<VPInstruction>(V);

  if (VPI.isCast() || isa<VPGEPInstruction>(VPI) || isa<VPSubscriptInst>(VPI))
    return maybePointerToPrivateMemory(*VPI.getOperand(0));

  // TODO: Look through more instruction kinds. Particularly, we may want to
  //       look through PHI nodes.

  return true;
}
