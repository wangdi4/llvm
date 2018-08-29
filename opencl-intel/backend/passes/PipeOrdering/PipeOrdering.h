// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
