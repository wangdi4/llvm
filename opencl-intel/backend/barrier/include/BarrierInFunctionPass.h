/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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

