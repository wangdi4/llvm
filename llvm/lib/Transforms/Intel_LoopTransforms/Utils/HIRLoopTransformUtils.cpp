//===--- HIRLoopTransformUtils.cpp  ---------------------------*- C++ -*---===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HIRLoopTransformUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopReversal.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRLoopTransformUtils.h"

#define DEBUG_TYPE "hir-looptransform-utils"

using namespace llvm;
using namespace loopopt;
using namespace reversal;

/// \brief Check if a given Loop is legal to reverse under HIRLoopReversal's
/// legality model.
bool HIRLoopTransformUtils::isHIRLoopReversalLegal(const HLLoop *Lp) {
  // 0.Setup
  assert(Lp && "Input loop is null inside "
               "HIRLoopTransformUtils::isHIRLoopReversalLegal(.)\n");

  // 1.Create an HIRLoopReversal object on stack
  HIRLoopReversal ReversalPass;

  // 2.Call isLegal() and return
  return ReversalPass.isLegal(Lp);
}

///\brief Check a given loop's suitability for reversal. And reverse if the loop
/// is suitable and the client requests it.
bool HIRLoopTransformUtils::checkAndReverseLoop(
    HLLoop *Lp,     // INPUT + OUTPUT: a given loop
    bool DoReverse, // INPUT: client's intention to reverse the loop if the loop
                    // is suitable
    HIRDDAnalysis &DDAnalysis, // INPUT: client provides a HIRDDAnalysis
    bool &LoopReversed // OUTPUT: true if the loop is successfully reversed
    ) {

  // 1.Create an HIRLoopReversal object on stack
  HIRLoopReversal ReversalPass;

  // 2.Call to runOnLoop(.)
  return ReversalPass.runOnLoop(Lp, DoReverse, DDAnalysis, LoopReversed);
}
