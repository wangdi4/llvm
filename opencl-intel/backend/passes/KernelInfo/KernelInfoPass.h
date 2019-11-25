// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
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

#ifndef __KERNEL_INFO_H__
#define __KERNEL_INFO_H__

#include "llvm/Pass.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"

#include <map>
#include <set>
#include <vector>

using namespace llvm;

namespace intel {

  /// This pass is a wrapper of the Kernel Info Pass, which currently outputs
  /// two information about the kernel: Has Barrier and Execution estimation
  /// This pass should run before the Barrier Pass, createPrepareKernelArgsPass

  class KernelInfoWrapper : public ModulePass {
  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    KernelInfoWrapper() : ModulePass(ID) {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "KernelInfoWrapper";
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }

    /// @brief performs KernelInfo pass on the module
    bool runOnModule(Module& M);

  protected:

    /// @brief Returns the total amount of storage, in bytes, used by
    ///        program variables in the global address space.
    /// @param M Program module.
    size_t getProgramGlobalVariableTotalSize(const Module& M);

  };

  /// This pass is responsible of getting some info about the OCL
  /// kernels in the supplied program (module)
  class KernelInfoPass : public FunctionPass {
  public:
    /// Pass identification, replacement for typeid
    static char ID;

    /// @brief Constructor
    KernelInfoPass();

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "KernelInfoPass";
    }

    /// @brief gets the required info on specific function
    /// @param pFunc ptr to function
    /// @returns True if module was modified
    bool runOnFunction(Function &Func);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfoWrapperPass>();
      AU.setPreservesAll();
    }

  protected:

    /// @brief checks if the function has a global sync built-ins
    ///        (like atom_add to global memory) in it
    /// @param pFunc ptr to function
    bool containsGlobalSync(Function *pFunc);

    /// @brief checks if the function has a barrier in it
    /// @param pFunc ptr to function
    bool containsBarrier(Function *pFunc);

    /// @brief returns approximation of the execution lenght of the given func
    /// @param pFunc ptr to function
    size_t getExecutionLength(Function *pFunc);

    /// returns the execution estimation of basic block based on it's nesting
    /// level
    size_t getExecutionEstimation(unsigned depth);
  };

} // namespace intel {

#endif // __KERNEL_INFO_H__

