/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __GROUP_BUILTIN_PASS_H__
#define __GROUP_BUILTIN_PASS_H__

#include "BarrierUtils.h"

#include "llvm/Pass.h"
#include "llvm/Module.h"

using namespace llvm;

namespace intel {

  /// @brief GroupBuiltinHandler pass is a module pass that handles calls to
  /// group built-ins instructions, e.g. async_copy, etc.
  /// Add barrier call before each call to this async copy built-in
  /// and dumyBarrier call after each call to it.
  class GroupBuiltin : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    GroupBuiltin();

    /// @brief D'tor
    ~GroupBuiltin() {};

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "Intel OpenCL GroupBuiltinPass";
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

#endif // __GROUP_BUILTIN_PASS_H__

