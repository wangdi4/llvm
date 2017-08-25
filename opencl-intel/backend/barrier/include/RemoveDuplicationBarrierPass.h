/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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

