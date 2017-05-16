/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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
    virtual StringRef getPassName() const {
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

