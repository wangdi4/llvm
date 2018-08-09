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

#ifndef __REMOVE_DUPLICATION_BARRIER_PASS_H__
#define __REMOVE_DUPLICATION_BARRIER_PASS_H__

#include "BarrierUtils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

using namespace llvm;

namespace intel {

  /// @brief RemoveDuplicationBarrier pass is a module pass used to prevent
  /// barrier/fiber/dummyBarrier instructions from appearring in sequence,
  /// i.e. if two or more such instructions appears in sequence keep only one
  /// and remove the rest according to the following rules:
  /// dummyBarrier-barier(global) : do nothing
  /// dummyBarrier-Any : remove Any
  /// barrier-fiber or fiber-barrier : remove fiber
  /// barrier-barrier : remove the one with local argument if exists or any
  class RemoveDuplicationBarrier : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    RemoveDuplicationBarrier();

    /// @brief D'tor
    ~RemoveDuplicationBarrier() {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "Intel OpenCL RemoveDuplicationBarrier";
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

#endif // __REMOVE_DUPLICATION_BARRIER_PASS_H__

