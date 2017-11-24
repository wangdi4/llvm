//==--- PipeOrdering.h - Header file of PipeOrdering pass -*- C++ -*---==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __PIPE_ORDERING_H__
#define __PIPE_ORDERING_H__

#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

namespace intel {

  /// @brief  PipeOrdering class adds barriers to loops with pipes.
  class PipeOrdering : public llvm::ModulePass {
  public:
    // Pass identification, replacement for typeid.
    static char ID;

    /// @brief Constructor
    PipeOrdering();

    /// @brief  LLVM Module pass entry
    /// @param  M  Module to transform
    /// @return true if changed
    bool runOnModule(llvm::Module &M) override;

    /// @brief Provides name of pass
    llvm::StringRef getPassName() const override { return "PipeOrdering"; }

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
      AU.addRequired<llvm::LoopInfoWrapperPass>();
    }

  private:
    void findCallersRequiringBarrier(
        llvm::Function *F,
        llvm::DenseMap<llvm::Function *, bool> &ProcessedFuncs,
        llvm::SmallPtrSetImpl<llvm::BasicBlock *> &BarrierRequired);
    bool isCalledFromNDRange(
        llvm::Function *F,
        llvm::DenseMap<llvm::Function *, bool> &ProcessedFuncs);

    llvm::SmallVector<llvm::Function *, 8> m_kernels;
  };
} // namespace Intel

#endif // __PIPE_ORDERING_H__
