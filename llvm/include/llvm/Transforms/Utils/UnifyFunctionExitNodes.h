//===-- UnifyFunctionExitNodes.h - Ensure fn's have one return --*- C++ -*-===//
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
//
// This pass is used to ensure that functions have at most one return and one
// unreachable instruction in them.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_UNIFYFUNCTIONEXITNODES_H
#define LLVM_TRANSFORMS_UTILS_UNIFYFUNCTIONEXITNODES_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class UnifyFunctionExitNodesLegacyPass : public FunctionPass {
public:
  BasicBlock *ReturnBlock;  // INTEL
  static char ID; // Pass identification, replacement for typeid
  UnifyFunctionExitNodesLegacyPass();

  // We can preserve non-critical-edgeness when we unify function exit nodes
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnFunction(Function &F) override;

  BasicBlock *getReturnBlock() { return ReturnBlock; } // INTEL
};

Pass *createUnifyFunctionExitNodesPass();

class UnifyFunctionExitNodesPass
    : public PassInfoMixin<UnifyFunctionExitNodesPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_UNIFYFUNCTIONEXITNODES_H
