/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#ifndef __REDUNDANT_PHINODE_PASS_H__
#define __REDUNDANT_PHINODE_PASS_H__

#include "llvm/Pass.h"
#include "llvm/Function.h"

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
    ~RedundantPhiNode() {};

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
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

