//===- ArgumentPromotion.h - Promote by-reference arguments -----*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_ARGUMENTPROMOTION_H
#define LLVM_TRANSFORMS_IPO_ARGUMENTPROMOTION_H

#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LazyCallGraph.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class TargetTransformInfo;

/// Argument promotion pass.
///
/// This pass walks the functions in each SCC and for each one tries to
/// transform it and all of its callers to replace indirect arguments with
/// direct (by-value) arguments.
class ArgumentPromotionPass : public PassInfoMixin<ArgumentPromotionPass> {
  bool RemoveHomedArguments; // INTEL
  unsigned MaxElements;

public:
#if INTEL_CUSTOMIZATION
  ArgumentPromotionPass(bool RemoveHomedArguments = false,
                        unsigned MaxElements = 3u) :
                        RemoveHomedArguments(RemoveHomedArguments),
                        MaxElements(MaxElements) {}
#endif // INTEL_CUSTOMIZATION

  /// Checks if a type could have padding bytes.
  static bool isDenselyPacked(Type *Ty, const DataLayout &DL);

  PreservedAnalyses run(LazyCallGraph::SCC &C, CGSCCAnalysisManager &AM,
                        LazyCallGraph &CG, CGSCCUpdateResult &UR);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_ARGUMENTPROMOTION_H
