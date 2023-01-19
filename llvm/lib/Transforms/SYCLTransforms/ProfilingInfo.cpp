//===----- ProfilingInfo.cpp - Clean up debug info ------------------------===//
//
// Copyright (C) 2022 Intel Corporation
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
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/ProfilingInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;

static void runOnUserFunction(Function *F) {
  SmallVector<Instruction *> InstsToRemove;

  for (auto &I : instructions(F))
    if (isa<DbgDeclareInst>(&I) || isa<DbgValueInst>(&I))
      InstsToRemove.push_back(&I);

  // Clean up the instructions scheduled for removal
  for (auto *I : InstsToRemove)
    I->eraseFromParent();
}

bool ProfilingInfoPass::runImpl(Module &M) {
  for (auto &F : M)
    if (!F.isDeclaration())
      runOnUserFunction(&F);
  return true;
}

PreservedAnalyses ProfilingInfoPass::run(Module &M,
                                         ModuleAnalysisManager &MAM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

