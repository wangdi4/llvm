//===-- DPCPPKernelCompilationUtils.cpp - Function definitions -*- C++ ----===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace llvm {

void DPCPPKernelCompilationUtils::moveAllocaToEntry(BasicBlock *FromBB,
                                                    BasicBlock *EntryBB) {
  // This implementation is only correct when ToBB is an entry block.
  llvm::SmallVector<AllocaInst *, 8> Allocas;
  for (auto &I : *FromBB)
    if (auto *AI = dyn_cast<AllocaInst>(&I))
      Allocas.push_back(AI);

  if (EntryBB->empty()) {
    IRBuilder<> Builder(EntryBB);
    for (auto *AI : Allocas) {
      AI->removeFromParent();
      Builder.Insert(AI);
    }
    return;
  }

  Instruction *InsPt = EntryBB->getFirstNonPHI();
  assert(InsPt && "At least one non-PHI insruction is expected in ToBB");
  for (auto *AI : Allocas) {
    AI->moveBefore(InsPt);
  }
}

} // end namespace llvm
