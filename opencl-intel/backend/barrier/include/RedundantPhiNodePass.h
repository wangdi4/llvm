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
    virtual llvm::StringRef getPassName() const {
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

