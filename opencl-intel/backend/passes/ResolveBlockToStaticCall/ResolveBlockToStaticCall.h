/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __RESOLVE_BLOCK_TO_STATIC_CALL_H__
#define __RESOLVE_BLOCK_TO_STATIC_CALL_H__

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

/*===----------------------------------------------------------------------===
This optimization pass is implementing resolution from indirect call to static call
for block invoke functions.

Background.
There are block constructs in OpenCL2.0.
Block is sort of lambda function. Block call is implemented as function pointer call in LLVM.
Example: %call = call i32 %10(i8* %6, i32 %8)

Problem.
It is required by spec that block call should be static.
Call in example should be resolved to:
%call = call i32 @__block_for_cond_block_invoke(i8* %6, i32 %8)

This optimization pass is implementing this resolution.

The pass can be extended to statically resolve enqueued blocks.
However it is not needed now for CPU. For MIC and GEN it may be useful.
===----------------------------------------------------------------------===*/

namespace intel {
  /// detect pointer calls pass
  struct ResolveBlockToStaticCall : public llvm::FunctionPass {
    static char ID;
    /// ctor
    ResolveBlockToStaticCall() : FunctionPass(ID) {}

    /// parse function
    virtual bool runOnFunction(llvm::Function &F);

  protected:
    /// resolve call instruction to static call
    /// @returns true if resolved
    bool runOnCallInst(llvm::CallInst *CI);

  }; /// struct ResolveBlockToStaticCall

  /// @brief create pass
  llvm::Pass *createResolveBlockToStaticCallPass();
} // namespace intel
#endif // __RESOLVE_BLOCK_TO_STATIC_CALL_H__
