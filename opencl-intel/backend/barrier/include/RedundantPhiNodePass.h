/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __REDUNDANT_PHINODE_PASS_H__
#define __REDUNDANT_PHINODE_PASS_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"

using namespace llvm;

namespace intel {

  /// @brief RedundantPhiNode pass is a function pass that remove redundant PHINode
  /// such that return same value for each entry block
  class RedundantPhiNode : public FunctionPass {

  public:
    static char ID;

    /// @brief C'tor
    RedundantPhiNode();

    /// @brief D'tor
    ~RedundantPhiNode() {}

    /// @brief Provides name of pass
    virtual StringRef getPassName() const {
      return "Intel OpenCL RedundantPhiNode";
    }

    /// @brief execute pass on given function
    /// @param M function to optimize
    /// @returns True if function was modified
    virtual bool runOnFunction(Function &F);


  private:

  };

} // namespace intel

#endif // __REDUNDANT_PHINODE_PASS_H__

