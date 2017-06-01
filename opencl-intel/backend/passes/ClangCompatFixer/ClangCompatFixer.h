/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __CLANG_COMPAT_FIXER_H__
#define __CLANG_COMPAT_FIXER_H__

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstrTypes.h"


namespace intel {

  using namespace llvm;

  /// @brief  ClangCompatFixer is responsible for fixing incompatibilities between (Apple's) 
  ///         clang output and the backend's expectations.
  ///         This way the backend only needs to support "one true way" of doing things,
  ///         and if the clang output is different, it is modified to look the way the
  ///         backend expects.
  ///         Currently, it contains two such fixes:
  ///         1) llvm.fmuladd intrinsics are broken into an fmul + fadd pair
  ///         2) The representation of implicit locals is changed to "marked globals"
  ///            instead of "marked allocas"
  class ClangCompatFixer : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;
    
    // Constructor
    ClangCompatFixer() : ModulePass(ID) {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "ClangCompatFixer";
    }

    /// @brief    LLVM Module pass entry
    /// @param M  Module to transform
    /// @returns  true if changed
    virtual bool runOnModule(Module &M);

  private:
    /// @brief Breaks FMA intrinsics back into a mul+add
    /// @param F - Function to replace FMAs in
    /// @return true if the function was changed, false otherwise
    bool handleFMAIntrinsics(Function &F);
  };

} // namespace intel {

#endif // __CLANG_COMPAT_FIXER_H__
