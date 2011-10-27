/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#ifndef __SPLIT_BB_ON_BARRIER_PASS_H__
#define __SPLIT_BB_ON_BARRIER_PASS_H__

#include "BarrierUtils.h"

#include "llvm/Pass.h"
#include "llvm/Module.h"

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
    virtual const char *getPassName() const {
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

