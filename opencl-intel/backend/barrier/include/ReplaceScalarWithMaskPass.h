// INTEL CONFIDENTIAL
//
// Copyright 2012-2020 Intel Corporation.
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

#ifndef __REPLACE_SCALAR_WITH_MASK_H__
#define __REPLACE_SCALAR_WITH_MASK_H__

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace intel {
  /// @brief This pass is a module pass that replaces
  /// a scalar kernel with a mask kernel.
  class ReplaceScalarWithMask : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    ReplaceScalarWithMask():ModulePass(ID) {}
    /// @brief D'tor
    ~ReplaceScalarWithMask() {}

    StringRef getPassName() const override{
      return "Intel ReplaceScalarWithMask";
    }

    bool runOnModule(Module &M) override;
  };
} // namespace intel

#endif
