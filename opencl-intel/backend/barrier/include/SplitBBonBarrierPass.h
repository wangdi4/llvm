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

#ifndef __SPLIT_BB_ON_BARRIER_PASS_H__
#define __SPLIT_BB_ON_BARRIER_PASS_H__

#include "BarrierUtils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace intel {

  /// @brief SplitBBonBarrier pass is a module pass used to assure
  /// barrier/fiber instructions appears only at the begining of basic block
  /// and not more than once in each basic block
  class SplitBBonBarrier : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    SplitBBonBarrier();

    /// @brief D'tor
    ~SplitBBonBarrier() {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "Intel OpenCL SplitBBonBarrier";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);

  private:
    /// This is barrier utility class
    BarrierUtils m_util;

  };

} // namespace intel

#endif // __SPLIT_BB_ON_BARRIER_PASS_H__

