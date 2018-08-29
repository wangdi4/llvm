// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __BARRIER_IN_FUNCTION_PASS_H__
#define __BARRIER_IN_FUNCTION_PASS_H__

#include "BarrierUtils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

using namespace llvm;

namespace intel {

  /// @brief Barrier in function pass is a module pass that handles barrier/fiber
  /// instructions called from non-kernel functions.
  /// For each such function with a barrier add dumyBarrier call at its begin
  /// and barrier call at its end, also add barrier call before each call to this
  /// function and dumyBarrier call after each call to it (repeate this till
  /// handling all functions). For each kernel add dummyBarrier call at its begin
  /// and barrier call at its end.
  /// Remove each fiber instruction that is called from function that has no barrier.
  class BarrierInFunction : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    BarrierInFunction();

    /// @brief D'tor
    ~BarrierInFunction() {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "Intel OpenCL BarrierInFunction";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);

  private:
    /// @brief Add dumyBarrier at function begin and barrier at function end
    /// @param pFunc function to modify
    void AddBarrierCallsToFunctionBody(Function *pFunc);

    /// @brief remove all fiber instructions from non handled functions
    /// @param functionsSet container with all handled functions
    /// @param M module to optimize
    void RemoveFibersFromNonHandledFunctions(TFunctionSet& functionsSet, Module &M);

  private:
    /// This is barrier utility class
    BarrierUtils m_util;
  };

} // namespace intel

#endif // __BARRIER_IN_FUNCTION_PASS_H__

