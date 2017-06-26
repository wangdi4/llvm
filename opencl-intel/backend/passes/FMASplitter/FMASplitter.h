/*=================================================================================
Copyright (c) 2017, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __FMA_SPLITTER_H__
#define __FMA_SPLITTER_H__

#include "llvm/IR/Module.h"

namespace intel {

  using namespace llvm;

  /// @brief  FMASplitter is responsible for splitting llvm.fmuladd to
  //  "fmul + fadd" pair.
  class FMASplitter : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    // Constructor
    FMASplitter() : ModulePass(ID) {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "FMASplitter";
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

#endif // __FMA_SPLITTER_H__
